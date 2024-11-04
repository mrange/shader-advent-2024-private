
// Some common defines which exacts semantics are unknown to me
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define WINDOWS_IGNORE_PACKING_MISMATCH

// For ASSERT
#include "assert.h"

#ifdef _DEBUG
// For printf
//  Used to print DEBUG info
#include <stdio.h>
#endif

// Include the Windows API in order to create windows
#include <windows.h>

// Include the OpenGL API in order to do OpenGL things
#include <GL/gl.h>
// In addition we have a OpenGL extension header which contains additional functions
#include "glext.h"

// Returns the source code to the fragment shader
GLchar const * GetFragmentShaderSource();

// Windows sends Windows messages to our window through the WndProc
//  callback function. This allows us to react to them and handle
//  different kind of events like WM_SIZE
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#ifdef _DEBUG
// In debug mode we want to print OpenGL errors to the console
void APIENTRY debugCallback(
    GLenum          source
  , GLenum          type
  , GLuint          id
  , GLenum          severity
  , GLsizei         length
  , GLchar const *  message
  , void const *    userParam
  )
{
  printf(message);
  printf("\n");
}
char debugLog[0xFFFF];
#endif

// The initial resolution of our window
int xres = 1600;
int yres = 1080;

// The windows class specification
//  Needed to be able to create a window later from this class
//  The important settings is the windows message handler (WndProc)
//  and the name ("DEMO")
WNDCLASSA windowClassSpecification {
      CS_OWNDC | CS_HREDRAW | CS_VREDRAW  // style
    , &WndProc                            // lpfnWndProc
    , 0                                   // cbClsExtra
    , 0                                   // cbWndExtra
    , 0                                   // hInstance
    , 0                                   // hIcon
    , 0                                   // hCursor
    , 0                                   // hbrBackground
    , 0                                   // lpszMenuName
    , "DEMO"                              // lpszClassName
};

// The pixel format specification
// This contains lots of flags and what not but the important 
//  bit is that we want to have a pixel format that is compatible
//  with OpenGL and RGBA
PIXELFORMATDESCRIPTOR pixelFormatSpecification {
    sizeof(PIXELFORMATDESCRIPTOR)                           // nSize
  , 1                                                       // nVersion
  , PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER  // dwFlags
  , PFD_TYPE_RGBA                                           // iPixelType
  , 32                                                      // cColorBits
  , 0                                                       // cRedBits
  , 0                                                       // cRedShift
  , 0                                                       // cGreenBits
  , 0                                                       // cGreenShift
  , 0                                                       // cBlueBits
  , 0                                                       // cBlueShift
  , 8                                                       // cAlphaBits
  , 0                                                       // cAlphaShift
  , 0                                                       // cAccumBits
  , 0                                                       // cAccumRedBits
  , 0                                                       // cAccumGreenBits
  , 0                                                       // cAccumBlueBits
  , 0                                                       // cAccumAlphaBits
  , 32                                                      // cDepthBits
  , 0                                                       // cStencilBits
  , 0                                                       // cAuxBuffers
  , PFD_MAIN_PLANE                                          // iLayerType
  , 0                                                       // bReserved
  , 0                                                       // dwLayerMask
  , 0                                                       // dwVisibleMask
  , 0                                                       // dwDamageMask
};

#ifdef _DEBUG
// In debug mode we use a console program to be able to print debug messages to console
int main() {
#else
// In release mode we use a windowed program
int WINAPI WinMain(
    HINSTANCE hInstance 
,   HINSTANCE hPrevInstance     // Always NULL in Win32
,   LPSTR     lpCmdLine         // Command line arguments
,   int       nCmdShow          // Window display state
) {
#endif
  // I have lots of asserts in the code
  //  This is to help detect errors early
  //  But asserts will not be compiled into the final RELEASE exe

#ifdef _DEBUG
  // Gets the module handle for the process, need to register the windows class
  //  Not available automatically in a console app
  auto hInstance = GetModuleHandle(0);
  assert(hInstance);
#endif

  // Set the module handle
  windowClassSpecification.hInstance      = hInstance;

  // Create the windows class, after that is successful
  //  we can create our window
  auto regOk = RegisterClassA(&windowClassSpecification);
  assert(regOk);

  // Windows style
  auto dwStyle = WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_POPUP;

  // The desired size
  RECT windowRect {
    0
  , 0
  , xres
  , yres
  };

  // But we need to account for the surrounding borders
	BOOL rectOk = AdjustWindowRect(&windowRect, dwStyle, 0);
  assert(rectOk);

  auto width  = windowRect.right  - windowRect.left;
  auto height = windowRect.bottom - windowRect.top;

  auto screenWidth  = GetSystemMetrics(SM_CXSCREEN);
  auto screenHeight = GetSystemMetrics(SM_CYSCREEN);

  // Create a window with a position
  auto hwnd = CreateWindowExA(
    0                                       // dwExStyle
  , windowClassSpecification.lpszClassName  // lpClassName
  , nullptr                                 // lpWindowName
  , dwStyle                                 // dwStyle
  , (screenWidth - width) / 2               // nX
  , (screenHeight - height) / 2             // nY
  , width                                   // nWidth
  , height                                  // nHeight
  , nullptr                                 // hWndParent
  , nullptr                                 // hMenu
  , nullptr                                 // hInstance
  , nullptr                                 // lpParam
  );
  assert(hwnd);

  // As we want to draw graphics we need the Device context
  auto hdc = GetDC(hwnd);
  assert(hdc);

  // For this Device context find a pixel format that is compatible with OpenGL
  auto pixelFormat = ChoosePixelFormat(
    hdc
  , &pixelFormatSpecification
  );
  assert(pixelFormat);

  // And switch to this pixel format
  auto setOk = SetPixelFormat(
    hdc
  , pixelFormat
  , nullptr
  );
  assert(setOk);

  // After that is done with can create the OpenGL context for the device context
  auto hglrc = wglCreateContext(hdc);
  assert(hglrc);

  // And make the created OpenGL context the currently in use
  auto makeOk = wglMakeCurrent(hdc, hglrc);
  assert(makeOk);

  // Initialize the demo

#ifdef _DEBUG
  // First I enable a debug message callback. This will print errors that is discovered
  //  during initalization

  // Note that this relies on the OpenGL extensions which requires us to query 
  //  for the function by name (here glDebugMessageCallback) and then invoke it
  //  through the correct function pointer signature (here PFNGLDEBUGMESSAGECALLBACKPROC)
  glEnable(GL_DEBUG_OUTPUT);
  ((PFNGLDEBUGMESSAGECALLBACKPROC)wglGetProcAddress("glDebugMessageCallback"))(debugCallback, 0);
#endif

  GLchar const * fragmentShaders[] {
    GetFragmentShaderSource()
  };
  auto fragmentShaderProgram = ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"))(GL_FRAGMENT_SHADER, 1, fragmentShaders);
  assert(fragmentShaderProgram > 0);

#ifdef _DEBUG
  // Disable the debug info to not interfere with the FPS of the main effect
  ((PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog"))(fragmentShaderProgram, sizeof(debugLog), NULL, debugLog);
  printf(debugLog);
  glDisable(GL_DEBUG_OUTPUT);
#endif

  // Look for the iTime uniform location, so we can set it in the draw loop
  auto iTimeLocation = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(fragmentShaderProgram, "iTime");
  assert(iTimeLocation > -1);

  // Look for the iResolution uniform location, so we can set it in the draw loop
  auto iResolutionLocation = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(fragmentShaderProgram, "iResolution");
  assert(iResolutionLocation > -1);

  // Make our shader program the current one
  ((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(fragmentShaderProgram);

  // The starting time in milliseconds
  auto before = GetTickCount64();

  auto done = false;
  MSG msg {};

  while(!done) {
    // The classic windows message loop
    //  We "pump" the message queue for windows message
    //  Look at it (to determine if there's a quit message)
    //  and then dispact the message to our window (which will invoke WndProc)
    while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) done = 1;
      // Result intentionally ignored
      TranslateMessage(&msg);
      // Result intentionally ignored
      DispatchMessageA(&msg);
    }

    // Get now in milliseconds
    auto now = GetTickCount64();
    // Compute iTime
    auto iTime = (now-before)/1000.F;

    // Sets the iTime uniform
    ((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(iTimeLocation, iTime);

    // Sets the iResolution uniform
    ((PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f"))(
        iResolutionLocation
      , static_cast<GLfloat>(xres)
      , static_cast<GLfloat>(yres)
      , 1)
      ;


    // Draw the code covering the client area in the window
    glRects(-1,-1,1,1);

    // Since we asked for a pixel format that is double buffered
    //  ie we have a buffer that is shown and one that is rendered to
    //  as we are now done rendering we need to swap the buffers
    //  to show the result
    auto swapOk = SwapBuffers(hdc);
    assert(swapOk);
  }

  // Don't free resources. Windows does it for us :)
  return 0;
}

// Windows sends Windows messages to our window through the WndProc
//  callback function. This allows us to react to them and handle
//  different kind of events like WM_SIZE
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // Don't handle these
  if (uMsg == WM_SYSCOMMAND && (wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER))
    return 0;

  // User asked us to close the program, so a PostQuitMessage
  //  This message will be picked up by our draw loop and cause it to exit
  if (
    // Is the Window closed?
        uMsg == WM_CLOSE 
    // Is the Window destroyed?
    ||  uMsg == WM_DESTROY 
    // Has the user pressed the ESC key?
    ||  (uMsg == WM_CHAR || uMsg == WM_KEYDOWN) && wParam == VK_ESCAPE) {
    // If any of above are true then we are done
    PostQuitMessage(0);
    return 0;
  }


  // Window size has changed, capture the new window size, 
  //  update the GL view port and store the updated size
  if (uMsg == WM_SIZE) {
    xres = LOWORD(lParam);
    yres = HIWORD(lParam);

    // Updates the OpenGL viewport
    glViewport(
        0
      , 0
      , xres
      , yres
      );
  }

  // Otherwise, forward to the default message handler
  return(DefWindowProcA(hWnd, uMsg, wParam, lParam));
}

GLchar const * GetFragmentShaderSource() {
  // Using an R-string to inline the shader source
  //  Shader by Kishimisu: https://www.shadertoy.com/view/mtyGWy
  return 
    R"SHADER(
#version 300 es
// Prelude compatible with simple ShaderToy shaders
precision highp float;

out vec4 fragColor;

// ShaderToy Uniforms
// These are the most commonly used ShaderToy uniforms.
uniform float iTime;          // ShaderToy's time uniform
uniform vec3 iResolution;     // ShaderToy's resolution (viewport) uniform

// ShaderToy-compatible mainImage function signature
void mainImage(out vec4 fragColor, in vec2 fragCoord);

void main() {
  // Pass the fragment coordinates to mainImage and output to fragColor
  mainImage(fragColor, gl_FragCoord.xy);
}

// Paste ShaderToy shader code here -->

/* This animation is the material of my first youtube tutorial about creative 
    coding, which is a video in which I try to introduce programmers to GLSL 
    and to the wonderful world of shaders, while also trying to share my recent 
    passion for this community.
                                        Video URL: https://youtu.be/f4s1h2YETNY
*/

//https://iquilezles.org/articles/palettes/
vec3 palette( float t ) {
    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(1.0, 1.0, 1.0);
    vec3 d = vec3(0.263,0.416,0.557);

    return a + b*cos( 6.28318*(c*t+d) );
}

//https://www.shadertoy.com/view/mtyGWy
void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = (fragCoord * 2.0 - iResolution.xy) / iResolution.y;
    vec2 uv0 = uv;
    vec3 finalColor = vec3(0.0);
    
    for (float i = 0.0; i < 4.0; i++) {
        uv = fract(uv * 1.5) - 0.5;

        float d = length(uv) * exp(-length(uv0));

        vec3 col = palette(length(uv0) + i*.4 + iTime*.4);

        d = sin(d*8. + iTime)/8.;
        d = abs(d);

        d = pow(0.01 / d, 1.2);

        finalColor += col * d;
    }
        
    fragColor = vec4(finalColor, 1.0);
}
)SHADER";
}
