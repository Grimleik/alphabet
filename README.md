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
- [X] Asteroids
- [ ] Gem Tower Defense
- [ ] Diablo Clone

## Platforms:
- [X] Windows
- [ ] Linux
- [ ] MacOS
- [ ] Raspberry Pi