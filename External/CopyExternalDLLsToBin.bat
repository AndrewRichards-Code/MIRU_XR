set platform=%1
set config=%2

echo Current Directory: %CD%
echo Platform         : %platform%
echo Configuration    : %config%

echo openxr_loader.dll
copy ".\..\External\OpenXR\%platform%\bin\openxr_loader.dll" ".\..\bin\%platform%\%config%\openxr_loader.dll"
