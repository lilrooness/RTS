set projectDir=%1
set OutDir=%2
xcopy %projectDir%res %OutDir%res /y /i /s
