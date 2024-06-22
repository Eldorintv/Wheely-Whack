echo off

REM Paths to shaders
set VERTEX_SHADER=skybox_shader.vert
set FRAGMENT_SHADER=skybox_shader.frag

REM Output SPIR-V files
set VERTEX_SPIRV=vertSkyBox.spv
set FRAGMENT_SPIRV=fragSkyBox.spv

REM Compile vertex shader
glslangValidator -V %VERTEX_SHADER% -o %VERTEX_SPIRV%
if %errorlevel% neq 0 (
    echo Vertex shader compilation failed!
    exit /b 1
)
echo Vertex shader compiled successfully: %VERTEX_SPIRV%

REM Compile fragment shader
glslangValidator -V %FRAGMENT_SHADER% -o %FRAGMENT_SPIRV%
if %errorlevel% neq 0 (
    echo Fragment shader compilation failed!
    exit /b 1
)
echo Fragment shader compiled successfully: %FRAGMENT_SPIRV%

echo Both shaders compiled successfully.