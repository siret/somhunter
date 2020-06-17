
@ECHO OFF

ECHO Running clang-format...

WHERE clang-format
IF %ERRORLEVEL% NEQ 0 (
  ECHO E: clang-format not found! At least not in the PATH...
) ELSE (
  ECHO clang-format found
  clang-format.exe -i -style=file -verbose ../src/*.hpp ../src/*.h

  ECHO Done
)

pause
