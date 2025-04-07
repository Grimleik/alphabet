/* ========================================================================
   Creator: Grimleik $

   TODO: Double buffered command queues. As we 'process' a new queue we swap the
   rendering queues and let the internal rendering thread process the commands.
   ========================================================================*/
#include "renderer_software_windows.h"
#include "memorymanager.h"
#include <logger.h>
#include <Windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

// #include <mutex>

RenderSoftwareImpl::RenderSoftwareImpl(const HWND &hwnd_, const HDC &hdc_) : hwnd(hwnd_), hdc(hdc_)
{
}
void RenderSoftwareImpl::Init()
{
	Renderer::Settings &settings = Renderer::Instance->settings;
	backBuffer.width = settings.width;
	backBuffer.height = settings.height;
	backBuffer.pitch = settings.width * BYTES_PER_PIXEL;

	// TODO: MemoryManager reclaim.
	backBuffer.pixels = MemoryManager::Instance->Partition(REQUIRED_MEMORY);
	bitmapInfo = {};
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biWidth = backBuffer.width;
	bitmapInfo.bmiHeader.biHeight = backBuffer.height;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = BYTES_PER_PIXEL * 8;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
}

void RenderSoftwareImpl::Start()
{
}

void RenderSoftwareImpl::ProcessCommands(const Renderer::CommandQueue &queue)
{
	ProcessCommandsInternal(queue);
	Flip();
	if (Renderer::Instance->settings.vSync)
	{
		HRESULT hr = DwmFlush();
		if (FAILED(hr))
		{
			AGE_LOG(LOG_LEVEL::ERR, "VSycn failure with DWM.");
		}
	}
}

void RenderSoftwareImpl::Shutdown()
{
	ReleaseDC(hwnd, hdc);
}

Renderer::BACKEND RenderSoftwareImpl::GetType()
{
	return Renderer::BACKEND::Software;
}

void RenderSoftwareImpl::SetPixel(int x, int y, int color)
{
	const Renderer::Settings settings = Renderer::Instance->settings;
	if (x < 0 || x >= settings.width || y < 0 || y >= settings.height)
	{
		return;
	}

	int *pixel = (int *)((u8 *)backBuffer.pixels + x * BYTES_PER_PIXEL +
						 y * backBuffer.pitch);
	*pixel = color;
}

void RenderSoftwareImpl::ProcessCommandsInternal(const Renderer::CommandQueue &queue)
{
	for (int i = 0; i < queue.activeSz; ++i)
	{
		const Renderer::Command *dc = &queue.cmds[i];
		switch (dc->type)
		{
		case Renderer::CMDType::CLEAR_SCREEN:
		{
			const Renderer::CmdClearScreen &csc = dc->csc;
			if (csc.color == 0)
			{
				memset(backBuffer.pixels, 0, backBuffer.width * backBuffer.height * BYTES_PER_PIXEL);
			}
			else
			{
				for (int y = 0; y < backBuffer.height; ++y)
				{
					for (int x = 0; x < backBuffer.width; ++x)
					{
						SetPixel(x, y, csc.color);
					}
				}
			}
		}
		break;
		case Renderer::CMDType::RECTANGLE:
		{
			const Renderer::CmdRectangle &rect = dc->rectangle;
			int hw = rect.w / 2;
			int hh = rect.h / 2;
			for (int y = -hh; y <= hh; ++y)
			{
				for (int x = -hw; x <= hw; ++x)
				{
					if (rect.filled || x == -hw || x == hw || y == -hh || y == hh)
						SetPixel(rect.x + x, rect.y + y, rect.c);
				}
			}
		}
		break;

		case Renderer::CMDType::LINE:
		{
			const Renderer::CmdLine &line = dc->line;
			// https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)
			f32 dx = (f32)(line.x2 - line.x1);
			f32 dy = (f32)(line.y2 - line.y1);
			i32 steps;
			if (fabs(dx) > fabs(dy))
				steps = (i32)fabs(dx);
			else
				steps = (i32)fabs(dy);

			if (steps > 0)
			{
				dx /= steps;
				dy /= steps;
				f32 x = (f32)line.x1;
				f32 y = (f32)line.y1;
				int step = 0;
				while (step <= steps)
				{
					SetPixel((i32)x, (i32)y, line.c);
					x += dx;
					y += dy;
					step++;
				}
			}
		}
		break;

		case Renderer::CMDType::TEXT:
		{
			const Renderer::CmdText &text = dc->text;
			TextOutA(hdc, text.x, text.y, text.text, text.len);
			// TODO: Actually render the text into OUR buffer.
		}
		break;

		default:
		{
			AGE_LOG(LOG_LEVEL::DEBUG, "Trying to render an unsupported drawing command {}.",
					Renderer::CMDType_Names[(int)dc->type]);
		}
		break;
		}
	}
}

void RenderSoftwareImpl::Flip()
{
	int result = StretchDIBits(hdc,
							   0, 0, backBuffer.width, backBuffer.height,
							   0, 0, backBuffer.width, backBuffer.height,
							   backBuffer.pixels,
							   &bitmapInfo,
							   DIB_RGB_COLORS,
							   SRCCOPY);
	if (result == GDI_ERROR)
	{
		AGE_LOG(LOG_LEVEL::ERR, "GDI error: {}.", GetLastError());
	}
}
