@echo off
setlocal enabledelayedexpansion

echo.
echo ============================================
echo Test: Save Graph to Image File
echo ============================================
echo.

REM Check if test executable exists
if not exist "C:\Users\ngoth\Downloads\files\bin\test_save_graph.exe" (
    echo ERROR: Test executable not found!
    echo Please build the test first using build_test.bat
    pause
    exit /b 1
)

echo Starting test application...
echo.
echo INSTRUCTIONS:
echo 1. Click "Generate Test Data" to populate the chart
echo 2. Click "Save Graph to Image" to save the graph
echo 3. Check the status message for save location
echo 4. Close the window when done
echo.
echo Test image files will be saved to: C:\Users\ngoth\Downloads\files\
echo.
pause

REM Run the test
"C:\Users\ngoth\Downloads\files\bin\test_save_graph.exe"

echo.
echo Test application closed.
echo.
echo Checking for saved images...
echo.

REM List any test_graph_*.png files
dir /B "C:\Users\ngoth\Downloads\files\test_graph_*.png" 2>nul

if !errorlevel! equ 0 (
    echo.
    echo SUCCESS: Graph images were saved!
    echo.
) else (
    echo.
    echo WARNING: No graph images found.
    echo.
)

pause
