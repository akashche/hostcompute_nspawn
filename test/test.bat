@echo off
set BAD_SLASH_SCRIPT_DIR=%~dp0
set SCRIPT_DIR=%BAD_SLASH_SCRIPT_DIR:\=/%

del bin\nspawn_out.txt
bin\hostcompute_nspawn.exe --config %SCRIPT_DIR%../resources/config.json
echo --- PROCESS OUTPUT ---
type bin\nspawn_out.txt
echo --- PROCESS OUTPUT END ---
