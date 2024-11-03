# 🎄⭐🎉 Rendering shaders in WebGL 🎉⭐🎄

🎅 *Merry Christmas, webgl fans!* 🎅

Everyone loves to tinker with Shaders on [ShaderToy](https://www.shadertoy.com/) but sometimes it's fun to deploy these shader in your own code, perhaps a new demo?

While it's possible to create Windows or MacOS program that renders your cool shaders why not use WebGL and give your demo maximum possible impact?!

[ShaderToy](https://www.shadertoy.com/) let's you share your shader using HTML markup that looks like this

```html
<iframe width="640" height="360" frameborder="0" src="https://www.shadertoy.com/embed/MfjyWK?gui=true&t=10&paused=true&muted=false" allowfullscreen></iframe>
```

but as hacker we want to do it ourselves right? In addition, by rolling it ourselves we can do custom textures and music to kick it up a notch!

## It starts with a Canvas.

The starting point for WebGL is the `canvas`

```html
<canvas id="webGLCanvas" width="800" height="600"></canvas>
```

In JavaScript we can the locate this `canvas` and it this request a WebGL context

```javascript
// Locate the canvas element where WebGL graphics will be drawn
const canvas = document.getElementById('webGLCanvas');
// Request the WebGL2 rendering context
const gl = canvas.getContext('webgl2');
```

[WebGL is well-document at MDN](https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API) and works very much like OpenGL, so if you are used to OpenGL you feel right at home.

If you are new to WebGL or OpenGL though it can be a bit finicky to render your first shader.


## Compile and linking a Shader Program

Shaders are programs that run on the GPU. In order to render a fragment shader we need a vertex shader too although a very simple one will do.

We compile these shaders using `createShader` and then link them together into the final shader program using `createProgram`.

When that is done we `use` the shader program so that is applied when we draw our quad later:

```javascript
// createShader: Compiles a WebGL shader of a specified type (vertex or
//  fragment) from source code
// Parameters:
//   - type: Specifies the type of shader
//           (either gl.VERTEX_SHADER or gl.FRAGMENT_SHADER).
//   - source: The GLSL source code for the shader as a string.
// Returns:
//   - The compiled shader if successful, or null if there was a
//      compilation error.
function createShader(type, source) {
  // Create an empty shader object of the specified type
  const shader = gl.createShader(type);

  // Attach the source code to the shader object
  gl.shaderSource(shader, source);

  // Compile the shader source code into executable code
  gl.compileShader(shader);

  // Check for compilation success
  if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
    // If compilation failed, log the error to the console for debugging
    console.error('Shader compilation error:', gl.getShaderInfoLog(shader));
    return null; // Return null to indicate failure
  }

  // If compilation was successful, return the compiled shader object
  return shader;
}

// createProgram: Builds and links a shader program from provided vertex
//  and fragment shader source code.
// Parameters:
//   - vertexSource: GLSL source code for the vertex shader as a string.
//   - fragmentSource: GLSL source code for the fragment shader as a string.
// Returns:
//   - The linked shader program if successful, or null if there was an
//      error during shader compilation or program linking.

function createProgram(vertexSource, fragmentSource) {
  // Compile the vertex shader from source
  const vertexShader = createShader(gl.VERTEX_SHADER, vertexSource);

  // Compile the fragment shader from source
  const fragmentShader = createShader(gl.FRAGMENT_SHADER, fragmentSource);

  // Check for shader compilation errors
  if (!vertexShader || !fragmentShader) {
    // Shader creation failed; errors were logged by createShader
    return null; // Return null to indicate failure
  }

  // Create a new WebGL program object to hold the shaders
  const program = gl.createProgram();

  // Attach the compiled vertex and fragment shaders to the program
  gl.attachShader(program, vertexShader);
  gl.attachShader(program, fragmentShader);

  // Link the shaders to create the complete program
  gl.linkProgram(program);

  // Check if linking was successful
  if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
      // If linking failed, log the error for debugging
      console.error('Program linking error:', gl.getProgramInfoLog(program));
      return null; // Return null to indicate failure
  }

  // Clean up shader objects after linking; they're now part of the program
  gl.deleteShader(vertexShader);
  gl.deleteShader(fragmentShader);

  // Return the linked shader program
  return program;
}

// Create a shader program using the provided vertex and fragment shader
//  source code
const shaderProgram = createProgram(vertexShaderSource, fragmentShaderSource);

// Set the shader program as the active program for rendering
gl.useProgram(shaderProgram);
```

## Setting up the Quad

In order to draw the fragment shader we will draw a quad (also known as a rectangle) filling the entire screen. For each pixel our shader program will be applied hopefully resulting in some sweet graphics.

To set it up we need to define the corners (or vertices of the quad) and load that into a vertex buffer, then when we draw the quad it will use the vertices to draw the quad (in practice it draw two triangles that forms the quad).

These corners (or vertices) will be fed to the vertex shader so we need to tell WebGL which input variable in the vertex shader will receive the vertices, in our case the input variable `position`.

### Our vertex shader

```glsl
#version 300 es
precision highp float;

// A minimal vertex shader that passes input directly to gl_Position.
in vec4 position;
void main() {
  gl_Position = position;
}
```

Unfortunately, it is a bit finicky to set this up which is why I provided some code for you:

```javascript
// Define the vertex positions for a full-screen quad as a triangle strip
// This quad will fill the entire canvas, covering the viewport
const vertices = new Float32Array([
  -1, -1,  // Bottom left corner
   1, -1,  // Bottom right corner
  -1,  1,  // Top left corner
   1,  1,  // Top right corner
]);

// Create a buffer to hold vertex data
const vertexBuffer = gl.createBuffer();
// Bind the buffer as the current ARRAY_BUFFER
gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
// Fill the buffer with the vertex data, using STATIC_DRAW for data that
//  won't change
gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

// Get the location of the 'position' attribute from the shader program.
// Note: If the name of the 'position' attribute in the vertex shader is
//  changed, you must update the name here to match the new attribute name.
const positionLocation = gl.getAttribLocation(shaderProgram, 'position');

// Enable the position attribute for use in the vertex shader
gl.enableVertexAttribArray(positionLocation);

// Specify how to read the vertex data from the buffer
gl.vertexAttribPointer(
  positionLocation, // The index of the attribute in the shader
  2,                // Number of components per vertex (x and y coordinates)
  gl.FLOAT,         // Type of data in the buffer (floating-point numbers)
  false,            // Do not normalize the data (values are already in the correct range)
  0,                // Stride: 0 means use the size of the vertex data type (auto)
  0                 // Offset: 0 means start reading from the beginning of the buffer
);
```

## Setting up the fragment shader uniforms

We have defined a fragment shader prelude that makes it simple to render ShaderToy shaders as it setups the uniforms (or input variables) `iTime` and `iResolution` which most shaders uses. Then you paste in the code for the shader you want to render at the end.

### Our fragment shader
```
#version 300 es
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
```

But in order to be able to set these uniforms (or input variables) we need to ask WebGL for their location. Finally we capture the start time.

```javascript
// Retrieve the locations of the uniforms defined in the fragment shader
//  by their names.
// These locations will be stored so that we can update their values
//  during the draw loop.
const timeLocation = gl.getUniformLocation(shaderProgram, 'iTime');             // Location for the time uniform
const resolutionLocation = gl.getUniformLocation(shaderProgram, 'iResolution'); // Location for the resolution uniform

// Capture the current time to use for calculating the elapsed time
//  (iTime) in the draw loop.
const begin = performance.now();
```

## The draw loop

We enter an infinite loop where we compute the `iTime` and queries the canvas for its current size and sets the `iResolution` based on that.

When that is all done we draw the vertices (corners) we defined above as a triangle strip and thanks to the layout of the vertices it will come out as a quad covering the entire canvas.

Finally, we request a new animation frame to animate the graphics smoothly.

```javascript
// Calculate the elapsed time (iTime) since the beginning of the
//  rendering.
// The result is in seconds, as performance.now() returns time in
//  milliseconds.
const iTime = (now - begin) * 0.001;

// Retrieve the current size of the canvas to set the iResolution uniform.
// This defines the dimensions of the rendering surface in the shader.
const width = canvas.clientWidth;
const height = canvas.clientHeight;

// Set the viewport to match the canvas dimensions.
// The viewport defines the drawable area within the canvas.
gl.viewport(0, 0, width, height);

// Update the iTime uniform in the shader with the calculated time value.
gl.uniform1f(timeLocation, iTime);

// Update the iResolution uniform with the current canvas size.
// The third component is set to 1, the same as ShaderToy uses.
gl.uniform3f(resolutionLocation, width, height, 1);

// Render the quad using the vertex buffer created earlier.
// This draws a triangle strip that forms a quad, utilizing the
//  vertices defined in the vertexBuffer.
gl.drawArrays(
    gl.TRIANGLE_STRIP,  // Drawing mode: uses a triangle strip to create the quad
    0,                  // Starting vertex index in the vertex buffer
    4                   // Total number of vertices to use for drawing (the quad has 4 vertices)
);

// Request the next animation frame to ensure smooth updates.
// This was a key insight for me after struggling with various methods
//  for smooth rendering in web development.
// Unlike other approaches that had their drawbacks,
//  requestAnimationFrame provides a seamless way to update animations.
requestAnimationFrame(drawShader);
```

## And that's it

Drawing your first fragment shader is a bit finicky the first time which is why I created [this complete example](src/index.html) for you.

You can clone this git repo or download the source code locally to open the web page (don't forget the CSS stylesheet `styles.css`) and see the entire example in it's glory.

If you want to add music you can use the `audio` element, it even supports FFT to make fancy VU meters with.

So the browser contains everything you need to make cool WebGL demos and once you get over the initial hurdle you will find it's not that difficult to do.

What I want for Christmas now is loads and loads new WebGL based demos to look at.

🎄🌟🎄 Merry Christmas to all, and happy coding! 🎄🌟🎄

🎅 – mrange


## ❄️Licensing Information❄️

All code content I created for this blog post is licensed under [CC0](https://creativecommons.org/public-domain/cc0/) (effectively public domain). Any code snippets from other developers retain their original licenses.

The text content of this blog is licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) (the same license as Stack Overflow).