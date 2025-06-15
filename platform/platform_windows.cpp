/* ========================================================================
   Creator: Grimleik $
   TODO: Choose game from within the game .. ?
   ========================================================================*/
#include "platform.h"
#include "singleton.h"
#include <Windows.h>
#include <DbgHelp.h>
#include <string>
#include <iostream>
#include "memorymanager.h"
// #include "renderer.h"
#include "renderer_software_windows.h"
#include "input.h"
#include "logger.h"
#include "entity.h"

#if __cplusplus >= 201703L
#include <new>
#endif

#if defined(_WIN32)
#include <windows.h>
#include <intrin.h>
#else
#include <unistd.h>
#endif

int get_cache_line_size()
{
	// 1. C++17 standard interference size
#if __cplusplus >= 201703L
	size_t interference_size = std::hardware_destructive_interference_size;
	if (interference_size >= 16 && interference_size <= 256)
	{
		return static_cast<int>(interference_size);
	}
#endif
#if defined(_WIN32) && (defined(_M_X64) || defined(_M_IX86))
	int cpuInfo[4] = {-1};
	__cpuid(cpuInfo, 1);
	int cache_line_size = ((cpuInfo[1] >> 8) & 0xFF) * 8; // EBX[15:8] * 8
	if (cache_line_size >= 16 && cache_line_size <= 256)
	{
		return cache_line_size;
	}
#endif
	AGE_LOG(LOG_LEVEL::WARN, "Failed to get cache line size. Using default value.");
	// 4. Fallback default
	return 64;
}

#define ErrorBox(msg) \
	MessageBoxA(nullptr, msg, "Error", MB_OK | MB_ICONERROR);

enum class Games
{
	Asteroids,
	Breakout,
	GemTD
};

// TODO: Concat ?
constexpr const char *AVAILABLE_DLLs[] = {
	"asteroids.dll",
	"breakout.dll",
	"gemtd.dll",
};

constexpr const char *AVAILABLEDLL_TMPs[] = {
	"asteroids_temp.dll",
	"breakout_temp.dll",
	"gemtd_temp.dll",
};

constexpr const char *AVAILABLE_PDBs[] = {
	"asteroids.pdb",
	"breakout.pdb",
	"gemtd.pdb",
};

class WindowsState
{
public:
	HWND hwnd;
	HMODULE gameDLL;
	Games activeGame;
	FILETIME lastWriteTime;
};

static Platform::State platState;

static GameInitFunc AGE_GameInitBinding = nullptr;
static GameUpdateFunc AGE_GameUpdateBinding = nullptr;
static GameShutdownFunc AGE_GameShutdownBinding = nullptr;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool InitializeWindows(WindowsState &state);
bool LoadGameDLL(WindowsState &state);
void UnloadGameDLL(WindowsState &state);

std::string FindLatestPDB()
{
	WIN32_FIND_DATAA findData;
	HANDLE hFind = FindFirstFileA("*.pdb", &findData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		std::cout << "Failed to find any pdb." << std::endl;
		return "";
	}

	FILETIME latest = {0, 0};

	std::string result;
	do
	{
		if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (CompareFileTime(&findData.ftLastWriteTime, &latest) > 0)
			{
				latest = findData.ftLastWriteTime;
				result = findData.cFileName;
			}
		}
	} while (FindNextFileA(hFind, &findData));
	std::cout << "Result is: " << result << " ." << std::endl;

	FindClose(hFind);
	return result;
}

bool IsFileLocked(const char *pdb)
{
	HANDLE fileHandle = CreateFileA("hotreloading", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		return true; // File is locked
	}
	CloseHandle(fileHandle);
	return false; // File is not locked
}

bool GetLastWriteTime(WindowsState &state)
{
	WIN32_FILE_ATTRIBUTE_DATA fd;
	if (!GetFileAttributesEx(AVAILABLE_DLLs[(int)state.activeGame], GetFileExInfoStandard, &fd) ||
		CompareFileTime(&state.lastWriteTime, &fd.ftLastWriteTime) == 0)
	{
		return false;
	}
	state.lastWriteTime = fd.ftLastWriteTime;
	return true;
}

bool IsHotReloading()
{
	DWORD fa = GetFileAttributesA("hotreloading");
	return fa != INVALID_FILE_ATTRIBUTES && !(fa & FILE_ATTRIBUTE_DIRECTORY);
}

int Platform::Create()
{
	WindowsState winState;
	winState.activeGame = Games::GemTD;

	if (!SymInitialize(GetCurrentProcess(), NULL, FALSE))
	{
		std::cout << "Failed to initalize symbol handler. Error: " << GetLastError() << std::endl;
	}

	if (!LoadGameDLL(winState))
	{
		ErrorBox("Failed to load initial dll.");
	}

	GetLastWriteTime(winState);
	LARGE_INTEGER freq;
	LARGE_INTEGER start, end;
	QueryPerformanceFrequency(&freq);

	size_t appMemSz = GB(1);
	void *appMem = malloc(appMemSz);
	MemoryManager::Create(appMem, appMemSz, get_cache_line_size());
	Input::Create();
	Renderer::Create();
	ECS::Create();
	Renderer::Instance->settings.width = 800;
	Renderer::Instance->settings.height = 600;

	InitializeWindows(winState);
	Renderer::Settings &rendSettings = Renderer::Instance->settings;
	HDC hdc = GetDC(winState.hwnd);

	RenderSoftwareImpl *swBackend = MemoryManager::Instance->PartitionWithArgs<RenderSoftwareImpl>(winState.hwnd, hdc);
	swBackend->Init();
	Renderer::Instance->AddBackend(swBackend->GetType(), swBackend);
	Renderer::Instance->SwapBackend(Renderer::BACKEND::Software);
	// Renderer::Instance->settings.vSync = true;
	platState.pups = {
		.memoryManager = MemoryManager::Instance,
		.input = Input::Instance,
		.renderer = Renderer::Instance,
		.ecs = ECS::Instance,
	};

	AGE_GameInitBinding(&platState, false);
	MSG msg = {};
	platState.isRunning = true;
	QueryPerformanceCounter(&start);
	while (platState.isRunning)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (IsHotReloading())
		{
			std::cout << "Hot reloading started." << std::endl;
			UnloadGameDLL(winState);
			while (IsHotReloading())
				;
			std::cout << "Hot reloading done." << std::endl;
			LoadGameDLL(winState);
			AGE_GameInitBinding(&platState, true);
		}

		AGE_GameUpdateBinding(&platState);
		Input::Instance->SwapInputBuffer();
		Renderer::Instance->Flip();
		QueryPerformanceCounter(&end);
		double elapsedTime = static_cast<double>(end.QuadPart - start.QuadPart) / freq.QuadPart;
		platState.dt = (f32)elapsedTime;
		start = end;

		// MemoryManager::Instance->Report();
	}

	AGE_GameShutdownBinding(&platState);

	Renderer::Instance->Shutdown();

	ReleaseDC(winState.hwnd, hdc);
	DestroyWindow(winState.hwnd);
	return 0;
}

void ReloadSymbols(WindowsState &state)
{
	DWORD64 baseAddr = (DWORD64)state.gameDLL;
	std::string pdb = AVAILABLE_PDBs[(int)state.activeGame];
	// std::string pdb = FindLatestPDB();
	if (!SymLoadModule64(GetCurrentProcess(), NULL, pdb.c_str(), 0, baseAddr, 0))
		std::cout << "Failed to load symbols." << std::endl;
}

bool LoadGameDLL(WindowsState &state)
{
	if (!CopyFile(AVAILABLE_DLLs[(int)state.activeGame], AVAILABLEDLL_TMPs[(int)state.activeGame], FALSE))
	{
		std::cerr << "Failed to copy DLL to temp." << std::endl;
	}

	state.gameDLL = LoadLibraryA(AVAILABLEDLL_TMPs[(int)state.activeGame]);
	if (!state.gameDLL)
	{
		ErrorBox("Failed to load game DLL");
	}

	ReloadSymbols(state);
	AGE_GameInitBinding = (GameInitFunc)GetProcAddress(state.gameDLL, "AGE_GameInit");
	AGE_GameUpdateBinding = (GameUpdateFunc)GetProcAddress(state.gameDLL, "AGE_GameUpdate");
	AGE_GameShutdownBinding = (GameShutdownFunc)GetProcAddress(state.gameDLL, "AGE_GameShutdown");

	return AGE_GameInitBinding &&
		   AGE_GameUpdateBinding &&
		   AGE_GameShutdownBinding;
}

void UnloadSymbols(WindowsState &state)
{
	DWORD64 baseAddr = (DWORD64)state.gameDLL;
	if (!SymCleanup(GetCurrentProcess()))
	{
		std::cout << "Failed to cleanup syms." << std::endl;
	}

	if (!SymInitialize(GetCurrentProcess(), NULL, FALSE))
	{
		std::cout << "Failed to initalize symbol handler. Error: " << GetLastError() << std::endl;
	}

	// if (!SymUnloadModule64(GetCurrentProcess(), baseAddr))
	// {
	// 	std::cout << "Failed to unload symbols." << std::endl;
	// }
}

void UnloadGameDLL(WindowsState &state)
{
	UnloadSymbols(state);
	FreeLibrary(state.gameDLL);
	state.gameDLL = NULL;

	std::string pdb = AVAILABLE_PDBs[(int)state.activeGame];
	// std::string pdb = FindLatestPDB();
	// while (IsFileLocked(pdb.c_str()))
	// {
	// 	std::cout << "Waiting for pdb file to be released: " << std::endl;
	// 	Sleep(100);
	// }

	// std::cout << "PDB File released. " << std::endl;
}

bool InitializeWindows(WindowsState &state)
{
	// Create Window:
	const char CLASS_NAME[] = "AGEx64";
	HINSTANCE hInstance = GetModuleHandle(NULL);
	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	if (!RegisterClass(&wc))
	{
		ErrorBox("Failed to register Window.");
		return false;
	}

	Renderer::Settings &settings = Renderer::Instance->settings;
	HWND hwnd = CreateWindowEx(0, CLASS_NAME, "AGEx64", WS_OVERLAPPEDWINDOW,
								CW_USEDEFAULT, CW_USEDEFAULT, settings.width,
								settings.height, NULL, NULL, hInstance, NULL);

	if (!hwnd)
	{
		ErrorBox("Failed to create window.");
		return false;
	}

	state.hwnd = hwnd;
	ShowWindow(hwnd, SW_SHOW);
	return true;
}

// Window procedure function
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool processKey = false;
	Input::KEY_STATES keyState;
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		platState.isRunning = false;
		return 0;

	case WM_CLOSE:
		// DestroyWindow(hwnd);
		platState.isRunning = false;
		return 0;
	case WM_KEYUP:
	{
		processKey = true;
		keyState = Input::KEY_STATES::KEY_UP;
	}
	break;

	case WM_KEYDOWN:
	{
		processKey = true;
		keyState = Input::KEY_STATES::KEY_DOWN;
	}
	break;
	}

	if (processKey)
	{
		switch (wParam)
		{
		case VK_ESCAPE:
			Input::Instance->MarkKey(Input::KEYS::ESCAPE, keyState);
			break;
		case 'W':
			Input::Instance->MarkKey(Input::KEYS::W, keyState);
			break;
		case 'A':
			Input::Instance->MarkKey(Input::KEYS::A, keyState);
			break;
		case 'S':
			Input::Instance->MarkKey(Input::KEYS::S, keyState);
			break;
		case 'D':
			Input::Instance->MarkKey(Input::KEYS::D, keyState);
			break;
		case 'P':
			Input::Instance->MarkKey(Input::KEYS::P, keyState);
			break;
		case VK_SPACE:
			Input::Instance->MarkKey(Input::KEYS::SPACE, keyState);
			break;
		case VK_UP:
			Input::Instance->MarkKey(Input::KEYS::UP_ARROW, keyState);
			break;
		case VK_DOWN:
			Input::Instance->MarkKey(Input::KEYS::DOWN_ARROW, keyState);
			break;
		case VK_LEFT:
			Input::Instance->MarkKey(Input::KEYS::LEFT_ARROW, keyState);
			break;
		case VK_RIGHT:
			Input::Instance->MarkKey(Input::KEYS::RIGHT_ARROW, keyState);
			break;
		case VK_BACK:
			Input::Instance->MarkKey(Input::KEYS::BACKSPACE, keyState);
			break;
		case VK_RETURN:
			Input::Instance->MarkKey(Input::KEYS::ENTER, keyState);
			break;
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}