#if !defined(RENDERER_SOFTWARE_WINDOWS_H)
/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/
#define RENDERER_SOFTWARE_WINDOWS_H

#include "renderer.h"
#include <windows.h>
#include <vector>
#include <../external/stb_truetype.h>

constexpr u16 LARGEST_WIDTH = 1920;
constexpr u16 LARGEST_HEIGHT = 1080;
constexpr u8 BYTES_PER_PIXEL = 4;
constexpr size_t REQUIRED_MEMORY = LARGEST_HEIGHT * LARGEST_WIDTH * BYTES_PER_PIXEL;

class RenderSoftwareImpl : public Renderer::IBackend
{
public:
	RenderSoftwareImpl(const HWND &hwnd_, const HDC& hdc_);
	void Init();
	void Start();
	void ProcessCommands(const Renderer::CommandQueue &queue) override;
	void Shutdown() override;
	Renderer::BACKEND GetType() override;

private:
	void SetPixel(int x, int y, int color);
	void ProcessCommandsInternal(const Renderer::CommandQueue &queue);
	void ExecuteCommand(const Renderer::Command *dc);
	void Flip();
	void Blend(u32 *dest, u32 *src, u32 alpha);
	// TODO(pf): Experiement with declaration and instantiation on the same line
	struct BackBuffer
	{
		void *pixels;
		u32 width;
		u32 height;
		int pitch;
	} backBuffer;

	// void *memory;
	BITMAPINFO bitmapInfo;
	const HWND &hwnd;
	const HDC &hdc;
	stbtt_fontinfo font;
	std::vector<unsigned char> fontBuffer;
};
#endif
