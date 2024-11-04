/*
 * Windows OpenGL Shader Tutorial
 * This file demonstrates how to set up a minimal OpenGL context in Windows
 * and render using fragment shaders.
 */

// Windows-specific configuration
// These defines help reduce the size of the Windows headers and avoid some legacy code
#define WIN32_LEAN_AND_MEAN  // Excludes rarely-used Windows headers
#define WIN32_EXTRA_LEAN     // Further reduces Windows headers
#define WINDOWS_IGNORE_PACKING_MISMATCH  // Prevents some alignment warnings

// Standard C headers
#include <assert.h>  // For runtime assertions (checking if our assumptions are correct)

// Debug-only includes
#ifdef _DEBUG
    #include <stdio.h>  // For printf() - only included in debug builds
#endif

// Graphics-related headers
#include <windows.h>    // Core Windows functions (creating windows, handling messages)
#include <GL/gl.h>      // Core OpenGL functions
#include "glext.h"      // Modern OpenGL extensions (needed for shaders)

/*
 * Note for beginners:
 * - OpenGL is a graphics API that lets us render 3D/2D graphics
 * - We need Windows.h to create a window and handle user input
 * - gl.h provides basic OpenGL functions
 * - glext.h gives us access to modern OpenGL features like shaders
 *
 * A shader is a small program that runs on the graphics card (GPU).
 * Fragment shaders in particular determine the color of each pixel we draw.
 */
 
/*
 * Forward Declarations
 * These are functions we'll define later but need to reference now
 */

// Gets the GLSL code for our fragment shader
GLchar const* GetFragmentShaderSource(void);

// Windows event handler - processes window events like resizing, closing, etc.
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/*
 * Debug Support
 * These are only included when building in debug mode (_DEBUG is defined)
 */
#ifdef _DEBUG
    // Callback function for OpenGL to report errors
    void APIENTRY debugCallback(
        GLenum source,          // Where the error came from
        GLenum type,            // The type of error
        GLuint id,              // Error ID
        GLenum severity,        // How serious the error is
        GLsizei length,         // Length of the error message
        GLchar const* message,  // The error message itself
        void const* userParam   // User-provided data (unused)
    ) {
        printf(message);
        printf("\n");
    }
    
    // Buffer for debug messages
    char debugLog[0xFFFF];  // 65535 characters
#endif

/*
 * Window Configuration
 */

// Initial window size (16:9 aspect ratio)
int xres = 1600;
int yres = 1080;

// Window class specification - tells Windows how to create our window
WNDCLASSA windowClassSpecification {
        CS_OWNDC    // style        : Give us our own DC (drawing context)
      | CS_HREDRAW  //                Redraw on resize
      | CS_VREDRAW  //                Redraw on resize
    , &WndProc      // lpfnWndProc  : Function to handle window events
    , 0             // cbClsExtra   : No extra class memory
    , 0             // cbWndExtra   : No extra window memory
    , 0             // hInstance    : Application instance handle (set later)
    , 0             // hIcon        : Default icon
    , 0             // hCursor      : Default cursor
    , 0             // hbrBackground: No background brush
    , 0             // lpszMenuName : No menu
    , "DEMO"        // lpszClassName: Our window class name
};

// Pixel format specification - tells OpenGL how to set up our graphics buffer
PIXELFORMATDESCRIPTOR pixelFormatSpecification {
    sizeof(PIXELFORMATDESCRIPTOR)   // nSize          : Size of struct, used as kind of versioning in Windows
  , 1                               // nVersion       :
  ,   PFD_DRAW_TO_WINDOW            // dwFlags        : Will draw in a window
    | PFD_SUPPORT_OPENGL            //                  Using OpenGL
    | PFD_DOUBLEBUFFER              //                  Use double buffering (smoother display)
  , PFD_TYPE_RGBA                   // iPixelType     : Use RGBA colors                        
  , 32                              // cColorBits     : 32 bits for color (8 each for R,G,B,A)
  , 0                               // cRedBits       : Not set
  , 0                               // cRedShift      : Not set
  , 0                               // cGreenBits     : Not set
  , 0                               // cGreenShift    : Not set
  , 0                               // cBlueBits      : Not set
  , 0                               // cBlueShift     : Not set
  , 8                               // cAlphaBits:    : 8 bits for alpha channel
  , 0                               // cAlphaShift    : Not set
  , 0                               // cAccumBits     : Not set
  , 0                               // cAccumRedBits  : Not set
  , 0                               // cAccumGreenBits: Not set
  , 0                               // cAccumBlueBits : Not set
  , 0                               // cAccumAlphaBits: Not set
  , 32                              // cDepthBits     : 32 bits for depth buffer
  , 0                               // cStencilBits   : Not set
  , 0                               // cAuxBuffers    : Not set
  , PFD_MAIN_PLANE                  // iLayerType     : Main drawing layer
  , 0                               // bReserved      : Not set
  , 0                               // dwLayerMask    : Not set
  , 0                               // dwVisibleMask  : Not set
  , 0                               // dwDamageMask   : Not set
};

/*
 * Notes for beginners:
 * 1. This code sets up the basic structure we need to create a window and use OpenGL
 * 2. Double buffering means we draw to a "back buffer" while displaying the "front buffer",
 *    then swap them. This prevents flickering.
 * 3. RGBA means we store Red, Green, Blue, and Alpha (transparency) values for each pixel
 * 4. The depth buffer stores how "far away" each pixel is, letting us draw 3D properly
 */

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
  // Creates the entire shader program from a list of shader sources.
  //  Very nice as it saves a bunch of calls that we have to do in WebGL to create
  //  a shader program
  auto shaderProgram = ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"))(GL_FRAGMENT_SHADER, 1, fragmentShaders);
  assert(shaderProgram > 0);

#ifdef _DEBUG
  // Disable the debug info to not interfere with the FPS of the main effect
  ((PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog"))(shaderProgram, sizeof(debugLog), NULL, debugLog);
  printf(debugLog);
  glDisable(GL_DEBUG_OUTPUT);
#endif

  // Look for the iTime uniform location, so we can set it in the draw loop
  auto iTimeLocation = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(shaderProgram, "iTime");
  assert(iTimeLocation > -1);

  // Look for the iResolution uniform location, so we can set it in the draw loop
  auto iResolutionLocation = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(shaderProgram, "iResolution");
  assert(iResolutionLocation > -1);

  // Make our shader program the current one
  ((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(shaderProgram);

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
