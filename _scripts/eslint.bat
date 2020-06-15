@ECHO OFF

SET ORIG_DIR=%cd%

cd ..
call eslint --ext .js .
cd %ORIG_DIR%

pause