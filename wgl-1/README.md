# 🎄⭐🎉 Rendering Shaders in an Windows App 🎉⭐🎄

🎅 *Merry Christmas, WebGL fans!* 🎅

Tinkering with Shaders in ShaderToy is great fun but how do we package it as a Windows App or even better a 4KiB Windows App?

There are tools out there to help you create a small executable but first you need a Windows App that renders a fragment shader.

I thought I start by showing how to create a minimal Windows App that opens a Windows and renders a fragment shader in it. Then in the follow-up blog post we will minimize it to less than 4KiB.

## The plan

1. Create Visual C++ project
2. Open a Window using Win32 (like BillG intended)
3. Initialize OpenGL
4. Compile the fragment shader
5. In the draw loop render a quad covering the entire Window with fragment shader attached.
5. Goto 4 until user hit Escape or closes the window

## Create a Visual C++ project

I am going to use [Visual Studio 2022 Community](https://visualstudio.microsoft.com/downloads/) which are free for tinkering with code.

During install make sure to install `Desktop development with C++`:

![Install Desktop development with C++](assets/vs-cpp-desktop.jpg)

I used the template `Console App (C++)` to create the project.

You can find the project [here](wgl-1/) and get it by either cloning this repository or downloading the files, you need all files to make it work.

## Important dependencies

We need Windows headers (obviously) but also OpenGL headers

```c++
#include <windows.h>    // Core Windows functions (creating windows, handling messages)
#include <GL/gl.h>      // Core OpenGL functions
#include "glext.h"      // Modern OpenGL extensions (needed for shaders)
```

The `glext.h` contains extensions for OpenGL which we will use. This file is not provided out of the box but I included a copy in the demo project.

In addition; we need to include OpenGL code by linking `opengl32.lib`. This I setup in the demo project.

## Opening a Window

The entry point for a console application is the `main` function where I put the majority of the code.

I will walk you through the example. In order to detect errors as early as possible I added lots of `assert` to the code, these will not be included in a a Release build.

Let's start with opening the window:

The idea is this.
1. We work only with the Win32 API, this is to make sure that we later on has as little overhead as possible to make a 4KiB executable. Right now though for various reasons the executable will be much bigger than 4KiB.
2. Create a Window Class that has a certain flags set and a custom window event handler. The window event handler allows us to react on events such closing the window.
3. Using this Window class we create a window centered on the screen.

In code it looks like this:
```c++
/*
  * Step 1: Initialize Window
  */

auto hInstance = GetModuleHandle(0);
assert(hInstance && "Failed to get module handle");

// Complete the window class specification by setting the hInstance
windowClassSpecification.hInstance = hInstance;

// Register the window class with Windows. This is necessary to create a window
// instance later based on the specifications defined in `windowClassSpecification`.
auto regOk = RegisterClassA(&windowClassSpecification);
assert(regOk && "Failed to register window class");

// Define the window style with flags for visibility, overlapping, and popup behavior.
auto dwStyle = WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_POPUP;

// Adjust the window size to account for window borders and title bar based on the specified
// resolution (`xres` by `yres`). This ensures the client area matches the desired resolution.
RECT windowRect = { 0, 0, xres, yres };
auto rectOk = AdjustWindowRect(&windowRect, dwStyle, 0);
assert(rectOk && "Failed to adjust window rect");

// Calculate the dimensions of the adjusted window and find the center position
// based on the screen size, so the window will open centered on the screen.
auto width        = windowRect.right - windowRect.left;
auto height       = windowRect.bottom - windowRect.top;
auto screenWidth  = GetSystemMetrics(SM_CXSCREEN);
auto screenHeight = GetSystemMetrics(SM_CYSCREEN);

// Create the window with the specified style, dimensions, and centered position.
auto hwnd = CreateWindowExA(
    0                                       // No extended window styles
  , windowClassSpecification.lpszClassName  // Registered window class name
  , nullptr                                 // No window title
  , dwStyle                                 // Window style flags
  , (screenWidth - width) / 2               // Centered X position
  , (screenHeight - height) / 2             // Centered Y position
  , width, height                           // Window dimensions
  , nullptr, nullptr, nullptr, nullptr      // Parent, menu, instance, and param handles (unused)
);
assert(hwnd && "Failed to create window");
```

While a bit tricky to get all the configuration and parameters right the first time when it comes down to it, it's not that much code to open a Window.

## Initialize OpenGL

The graphics is going to be an OpenGL fragment shader so we need to initialize OpenGL. Luckily it's rather simple to do so.

1. Get the Device Context (needed to draw graphics in general)
2. Ask Windows for a pixel format compatible with OpenGL
3. Switch the Device Context to this pixel format

```c++
/*
  * Step 2: Initialize OpenGL
  */

// Obtain the device context (DC) for the specified window, which allows us to draw
// and interact with the window's graphics.
auto hdc = GetDC(hwnd);
assert(hdc && "Failed to get DC");

// Set up the OpenGL pixel format for the window, specifying how pixels should be represented.
auto pixelFormat = ChoosePixelFormat(hdc, &pixelFormatSpecification);
assert(pixelFormat && "Failed to choose pixel format");

// Apply the selected pixel format to the device context, ensuring that it is properly configured
// for OpenGL rendering.
auto setOk = SetPixelFormat(hdc, pixelFormat, &pixelFormatSpecification);
assert(setOk && "Failed to set pixel format");

// Create and activate an OpenGL rendering context for the device context, which allows us
// to perform OpenGL operations in this window.
auto hglrc = wglCreateContext(hdc);
assert(hglrc && "Failed to create GL context");

// Make the created OpenGL context current for the specified device context, enabling
// OpenGL commands to affect the window's rendering.
auto makeOk = wglMakeCurrent(hdc, hglrc);
assert(makeOk && "Failed to make GL context current");
```

## Initializing the demo

This can be complex but in our case it's just to compile the fragment shader.

What we do is this:

1. Enable OpenGL debug info during Debug builds (to help us troubleshoot more easily)
2. Using the very helpful OpenGL function `glCreateShaderProgramv` we pass the fragment shader source to it and store the result as `shaderProgram`
3. Then we disable the OpenGL debug info to not interfere with us rendering the shader.
4. Create an OpenGL account for the Device Context and make it the current one.
5. Locate the uniform variables `iTime` and `iResolution` in the compiled shader, this will allow us to inject values into the shader during the draw loop later.
6. Last make the shader program the current one.

One complexity is that while many functions are directly available as normal functions such as `glUseProgram` others we have to ask for by name such as `glCreateShaderProgramv`. This is because these functions are extensions that may or may not be available.

So we query by name for `glCreateShaderProgramv` and store the pointer to that function in `glCreateShaderProgramv`:

```c++
auto glCreateShaderProgramv = (PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv");
```

Then we can calling the function by using the function pointer:
```c++
auto shaderProgram = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, fragmentShaders);
```

This is a very common pattern in OpenGL.

```c++
/*
  * Step 3: Set up Shader Program
  */

#ifdef _DEBUG
// Enable OpenGL debug output in debug builds
glEnable(GL_DEBUG_OUTPUT);

// Retrieve a pointer to the OpenGL function `glDebugMessageCallback` using `wglGetProcAddress`.
// OpenGL functions like this one are often not directly accessible, as they may be specific
// to certain OpenGL versions or extensions. By looking them up at runtime, we ensure compatibility
// with different graphics drivers and hardware setups.
auto glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)wglGetProcAddress("glDebugMessageCallback");

// Set up a debug callback function (`debugCallback`) to handle messages from the OpenGL driver,
// like errors or performance warnings. This helps with diagnosing issues during development.
glDebugMessageCallback(debugCallback, 0);
#endif

// Create the shader program using `glCreateShaderProgramv` which creates a shader
// program in one step by specifying the shader type and the shader source code. This function
// allows us to skip manual shader compilation and linking steps. We specify `GL_FRAGMENT_SHADER`
// as the shader type, with `1` indicating that there�s one shader source in `fragmentShaders`.
GLchar const* fragmentShaders[] = { GetFragmentShaderSource() };
auto glCreateShaderProgramv = (PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv");
auto shaderProgram = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, fragmentShaders);

// Ensure that `shaderProgram` was created successfully. A non-positive value would indicate
// a failure to create the program, so we use an assertion to catch this in debug builds.
assert(shaderProgram > 0 && "Failed to create shader program");

#ifdef _DEBUG
// Retrieve the compilation log for `shaderProgram` and print it
auto glGetProgramInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
glGetProgramInfoLog(shaderProgram, sizeof(debugLog), NULL, debugLog);
printf(debugLog);

// Final setup is complete, so disable debug output
glDisable(GL_DEBUG_OUTPUT);
#endif

auto glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
// Get the location of the `iTime` uniform in the shader program. This location will be used
// to set the value of `iTime`.
auto iTimeLocation = glGetUniformLocation(shaderProgram, "iTime");
// Get the location of the `iResolution` uniform in the shader program. This location is
// used to pass the resolution of the rendering window.
auto iResolutionLocation = glGetUniformLocation(shaderProgram, "iResolution");
assert(iTimeLocation > -1 && iResolutionLocation > -1 && "Failed to get uniform locations");

// Activate the shader program so it will be applied to all pixels
// in subsequent draw calls, such as the upcoming call to `glRects`.
auto glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
glUseProgram(shaderProgram);
```

🎅 – mrange

## ❄️Licensing Information❄️

All code content I created for this blog post is licensed under [CC0](https://creativecommons.org/public-domain/cc0/) (effectively public domain). Any code snippets from other developers retain their original licenses.

The text content of this blog is licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) (the same license as Stack Overflow).