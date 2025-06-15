/* ========================================================================
   Creator: Grimleik $

   TODO: Double buffered command queues. As we 'process' a new queue we swap the
   rendering queues and let the internal rendering thread process the commands.
   ========================================================================*/
#define STB_TRUETYPE_IMPLEMENTATION
#include "renderer_software_windows.h"
#include "memorymanager.h"
#include <logger.h>
#include <Windows.h>
#include <dwmapi.h>
#include <fstream>
#pragma comment(lib, "dwmapi.lib")

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
	backBuffer.pixels = MemoryManager::Instance->PartitionBlock(REQUIRED_MEMORY);
	bitmapInfo = {};
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biWidth = backBuffer.width;
	bitmapInfo.bmiHeader.biHeight = backBuffer.height;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = BYTES_PER_PIXEL * 8;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	std::ifstream fontFile("../../../data/fonts/CascadiaCode.ttf", std::ios::binary);
	// std::ifstream fontFile("../../../data/fonts/CascadiaCode.ttf");
	if (!fontFile)
	{
		AGE_LOG(LOG_LEVEL::ERR, "Failed to open font file.");
		return;
	}
	fontBuffer = std::vector<unsigned char>(std::istreambuf_iterator<char>(fontFile), {});

	if (fontBuffer.empty())
	{
		AGE_LOG(LOG_LEVEL::ERR, "Font file is empty or could not be read.");
		return;
	}

	if (!stbtt_InitFont(&font, fontBuffer.data(), 0))
	{
		AGE_LOG(LOG_LEVEL::ERR, "Failed to initialize font.");
		return;
	}
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

void RenderSoftwareImpl::Blend(u32 *dest, u32 *src, u32 alpha)
{
	// Simple alpha blending
	u32 invAlpha = 255 - alpha;
	*dest = ((*dest & 0x00FFFFFF) * invAlpha + (*src & 0x00FFFFFF) * alpha) / 255;
	*dest |= (*src & 0xFF000000); // Preserve the alpha channel
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

void RenderSoftwareImpl::ExecuteCommand(const Renderer::Command *dc)
{
	switch (dc->type)
	{
	case Renderer::CMDType::BATCH:
	{
		const Renderer::CmdBatch &batch = dc->batch;
		for (u32 i = 0; i < batch.count; ++i)
		{
			const Renderer::Command *cmd = &((Renderer::Command *)batch.cmds)[i];
			ExecuteCommand(cmd);
		}
	}
	break;
	case Renderer::CMDType::CLEAR_SCREEN:
	{
		const Renderer::CmdClearScreen &csc = dc->csc;
		if (csc.color == 0)
		{
			memset(backBuffer.pixels, 0, backBuffer.width * backBuffer.height * BYTES_PER_PIXEL);
		}
		else
		{
			for (u32 y = 0; y < backBuffer.height; ++y)
			{
				for (u32 x = 0; x < backBuffer.width; ++x)
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
		float scale = stbtt_ScaleForPixelHeight(&font, text.scale);
		int ascent, descent, lineGap;
		stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
		int baseline = (int)(ascent * scale);
		int pen_x = text.x;
		for (const char *t = text.text; *t != '\0'; ++t)
		{
			int cp = *t;
			int ax, lsb;
			stbtt_GetCodepointHMetrics(&font, cp, &ax, &lsb);

			int x0, y0, x1, y1;
			stbtt_GetCodepointBitmapBox(&font, cp, scale, scale, &x0, &y0, &x1, &y1);
			int w = x1 - x0;
			int h = y1 - y0;

			if (w > 0 && h > 0)
			{
				u8 *bitmap = (u8 *)stbtt_GetCodepointBitmap(&font, scale, scale, cp, &w, &h, 0, 0);
				for (int row = 0; row < h; ++row)
				{
					for (int col = 0; col < w; ++col)
					{
						u32 dst_x = pen_x + x0 + col;
						u32 dst_y = text.y + baseline + y0 + row;
						if (dst_x < 0 || dst_x >= backBuffer.width || dst_y < 0 || dst_y >= backBuffer.height)
						{
							continue; // Skip pixels outside the back buffer
						}

						u8 alpha = bitmap[row * w + col];
						if (alpha > 0)
						{
							u32 color = text.c; //(text.c & 0x00FFFFFF) | (alpha << 24);
							// u32 *dst = (u32*)((u8 *)(backBuffer.pixels) + (dst_y * backBuffer.width + dst_x));
							// TODO: Color
							// Blend(dst, (u32 *)(bitmap) + row * w + col, alpha);
							SetPixel(dst_x, backBuffer.height - dst_y, color);
						}
					}
				}
				stbtt_FreeBitmap(bitmap, nullptr);
			}

			pen_x += (int)(ax * scale);

			if (*(t + 1))
			{
				pen_x += (int)(stbtt_GetCodepointKernAdvance(&font, cp, *(t + 1)) * scale);
			}
		}
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

void RenderSoftwareImpl::ProcessCommandsInternal(const Renderer::CommandQueue &queue)
{
	for (int i = 0; i < queue.activeSz; ++i)
	{
		const Renderer::Command *dc = &queue.cmds[i];
		ExecuteCommand(dc);
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
