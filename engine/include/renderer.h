#if !defined(RENDERER_H)
/* ========================================================================
	Creator: Grimleik $
   ========================================================================*/
#define RENDERER_H

#include "core.h"
#include "singleton.h"

class Renderer : public ISingleton
{
public:
	enum class CMDType
	{
		CLEAR_SCREEN,
		RECTANGLE,
		CIRCLE,
		LINE,
		TEXT,
		// BMP,
		// Mesh ?
		// Texture ?
	};

	constexpr static const char *CMDType_Names[] = {
		"CLEAR_SCREEN",
		"RECTANGLE",
		"CIRCLE",
		"LINE",
		"TEXT",
		// "DCT_BMP",
		// "DCT_Mesh",
		// "DCT_Texture"
	};

	struct CmdClearScreen
	{
		u32 color;
	};

	struct CmdRectangle
	{
		i32 x, y, w, h;
		u32 c;
		bool filled;
	};

	struct CmdLine
	{
		i32 x1, y1, x2, y2
			//,w TODO:
			;
		u32 c;
	};

	struct CmdText
	{
		i32 x, y;
		const char *text;
		u32 len;
		u32 c;
	};

	struct CmdBatch
	{
		CMDType type;
		void *cmds;
		u32 count;
	};

	struct Command
	{
		CMDType type;
		union
		{
			// NOTE(pf): Make sure to provide explicit calls for these in the
			// renderer, these are our 'primitives'.
			CmdClearScreen csc;
			CmdRectangle rectangle;
			CmdLine line;
			CmdText text;
			CmdBatch batch;
		};
	};

	enum class BACKEND
	{
		None,
		Software,
		OpenGL,
		// DirectX,
	};

	constexpr static const char *BACKEND_NAMES[] = {
		"None",
		"Software",
		"OpenGL",
		// DirectX,
	};

	constexpr static u16 MAX_DRAW_COMMANDS = 4096;
	class CommandQueue
	{
	public:
		Command cmds[MAX_DRAW_COMMANDS];
		int activeSz;
	};

	struct Settings
	{
		i16 width, height;
		bool vSync;
	};

	class IBackend
	{
	public:
		virtual void Start() = 0;
		virtual void ProcessCommands(const CommandQueue &queue) = 0;
		virtual void Shutdown() = 0;
		virtual BACKEND GetType() = 0;
	};

	static void Create();
	static Renderer *Instance;

	void AddBackend(BACKEND bt, IBackend *bk);

	void PushCmd_ClearScreen(const CmdClearScreen &&csc);
	void PushCmd_Rectangle(const CmdRectangle &&rect);
	void PushCmd_Line(const CmdLine &&line);
	void PushCmd_Text(const CmdText &&text);

	void Flip();
	void Shutdown();
	void Resize(u16 nWidth, u16 nHeight);
	void SwapBackend(BACKEND backendType);

	Settings settings;
	BACKEND activeBackend;

private:
	Command &PushCommand();
	CommandQueue commandQueue;
	IBackend *backend;
	std::map<BACKEND, IBackend *> backends;
};

#endif
