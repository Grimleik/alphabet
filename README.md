Alphabet Game Challenge Inspired by John Romero.

# Description
This is a single repo of multiple games sharing a common framework. It is a work
in progress and in the future, when the shared code is stable and have been
tested on multiple projects will be split up. But for now to keep things simple
everything is in one place.

## Project Structure

The project is structured as follows:

- Platform is an executable which is conditionally created based on the platform. Its responsibility is to load the game dll.
- Game(s), e.g asteroids, is a DLL which contains the game logic. It is loaded by the platform executable.
- Engine is a shared library which contains the code for the engine. It is used
by both the engine, to create and setup the systems, and the games which get a
reference to these system by the platform.


## Projects:
Some of the ideas are generated by ChatGPT rest are from my own mind.
- [~] Asteroids
	Week 15: Setup project.
		- No clear goals here, we just want to setup the project and get a feel for the engine.
		- Add text rendering.
		- Entity component system rewrite to template instead of macros.
		- Get Linux and Windows working.
		- Get the engine working on both platforms.
		- Memory management, threading, and other engine related stuff.

- [ ] Cube Game 
	Week 17: Go into 3D, add OpenGL backend and try software 3D ?
- [ ] Diablo Clone
	Week 18: First attempt at an actual game.
- [ ] Gem Tower Defense
	Week ??: This is where the actual game development starts.

#  
- [ ] Dear ImGui
- [ ] OpenGL
- [ ] DirectX

## Platforms:
- [X] Windows
- [ ] Linux
- [ ] MacOS
- [ ] Raspberry Pi