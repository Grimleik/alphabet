@echo off
setlocal
for /f "tokens=2 delims==" %%I in ('"wmic os get localdatetime /value"') do set datetime=%%I
set datetime=%datetime:~0,14%
set datetime=%datetime:~0,4%%datetime:~4,2%%datetime:~6,2%%datetime:~8,2%%datetime:~10,2%%datetime:~12,2%
echo asteroids%datetime%.pdb 