/* ========================================================================
   Creator: Grimleik $
   ========================================================================*/

#include "renderer.h"
#include <iostream>
#include <memorymanager.h>
#include <logger.h>

Renderer *Renderer::Instance = nullptr;

void Renderer::Create()
{
	Instance = MemoryManager::Instance->Partition<Renderer>();
}

Renderer::Command &Renderer::PushCommand()
{
	assert(commandQueue.activeSz < MAX_DRAW_COMMANDS);
	return commandQueue.cmds[commandQueue.activeSz++];
}

void Renderer::AddBackend(BACKEND bt, IBackend *bk)
{
	backends.emplace(bt, bk);
}

void Renderer::PushCmd_ClearScreen(const CmdClearScreen &&csc)
{
	Renderer::Command &cmd = PushCommand();
	cmd.type = CMDType::CLEAR_SCREEN;
	cmd.csc = std::move(csc);
}

void Renderer::PushCmd_Rectangle(const CmdRectangle &&rect)
{
	Renderer::Command &cmd = PushCommand();
	cmd.type = CMDType::RECTANGLE;
	cmd.rectangle = std::move(rect);
}

void Renderer::PushCmd_Line(const CmdLine &&line)
{
	Renderer::Command &cmd = PushCommand();
	cmd.type = CMDType::LINE;
	cmd.line = std::move(line);
}

void Renderer::Flip()
{
	backend->ProcessCommands(commandQueue);
	commandQueue.activeSz = 0;
}

void Renderer::Shutdown()
{
	backend->Shutdown();
}

void Renderer::Resize(u16 nWidth, u16 nHeight)
{
	if (settings.width == nWidth && settings.height == nHeight)
	{
		AGE_LOG(LOG_LEVEL::ERR, "Trying to resize renderer to same size, {} and {}. Logic error.", nWidth, nHeight);
	}
	settings.width = nWidth;
}

void Renderer::SwapBackend(BACKEND backendType)
{
	if (activeBackend != backendType)
	{
		activeBackend = backendType;
		if (backends.find(backendType) != backends.end())
		{
			backend = backends[backendType];
			backend->Start();
		}
		else
		{
			std::cerr << "Unsupported Backend: " << BACKEND_NAMES[(int)activeBackend] << std::endl;
		}
	}
	else
	{
		AGE_LOG(LOG_LEVEL::WARN, "Swapping to same backend : {}, unessecary op.", BACKEND_NAMES[(int)activeBackend]);
	}
}
