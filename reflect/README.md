# 🎄🌟🎄 Inner reflections are cool looking 🎄🌟🎄

🎅 Ho, ho, ho! Merry Christmas, fellow shader-hackers! 🎅

## Time for some more "hard-core" shader code

For those of you that gotten the basics of raymarching in GLSL down the next step could be some cool look reflections.

I am hoping to be able to help you a bit by showing how to build up a shader that has 3D block with glowing inner reflections.

## So what's the plan?

The shader will consist of 3 raytracers.

1. "World" raytracer that produces the background ground and sky.
2. The 3D block outside raytracer. Reflects the world raytracer for some nice looking reflections. Refracts the incoming ray into the third and last raytracer.
3. The inner reflections raytracer bounces a ray back and forth inside a the 3D block and accumulates the reflected glow. Use beer absorbition to fade out the glow.

## The world raytracer

The world raytracer is the simplest one but utilizes some tricks to avoid aliasing.

The input to the raytracer is the ray-origin `ro` and the ray-direction `rd`. The return value is the pixel color.

From the ray-direction compute the sky color:

```glsl
  col = hsv2rgb_approx(vec3(
    0.6,                                // Fixed hue for sky color
    clamp(0.3+0.9*rd.y, 0.0, 1.0),      // Saturation varies with vertical angle
    1.5*clamp(2.0-2.*rd.y*rd.y, 0.0, 2.) // Brightness with non-linear falloff
  ));
```

Let me assure you I spent as little time as possible thinking about this and just tinkered until I got it to look good enough. I believe this is the modus operandi of most shader developers.

Then compute the ground. The ground is a grid and the way I usually do it is that I repeat a cross over the entire plane. A neat way I learnt awhile ago is this:

```glsl
// Create grid coordinate system
// Round coordinates to snap to grid points
vec2 npp = round(bpp);
vec2 cpp = bpp - npp;
```

`cpp` is then the coordinate system for 1x1 cells. Computing the distance field for the grid is then as simple as this.

```glsl
vec2 app = abs(cpp);
float gd = min(app.x, app.y);
```

`app.x` and `app.y` are vertical and horizontal lines combined with `min`.

In order to reduce aliasing effects I fade out the grid depending on the distance but in addition I detect if the ray hits the plane at an angle and fades out the lines using this:

```glsl
// Grid line distance field with view-angle compensation
// Reduces aliasing by adjusting line width based on view angle
float gfre = 1.+rd.y;
gfre *= gfre;
gfre *= gfre;
```

`gfre` will then be in `[0,1]` 0 meaning looking straight down on the plane and 1 meaning looking at the plane at 90 degree angle. How many times I multiply `gfre` is decided by experimenting until it looks good enough.

All in all the world raytracer looks like this:

```glsl
// Render the surrounding world environment
// Responsible for creating the background scene including sky and ground plane
vec3 renderWorld(vec3 ro, vec3 rd) {
  vec3 col = vec3(0.0);

  // Calculate distance to floor plane using ray-plane intersection
  // Uses ray origin (ro) and ray direction (rd) to compute intersection point
  float bt = -(ro.y-bottom)/(rd.y);

  // Generate sky color using HSV approximation
  // Color varies based on ray direction (up/down angle)
  // - Hue is fixed at blue-cyan (0.6)
  // - Saturation depends on vertical ray angle
  // - Brightness uses a quadratic falloff to create gradient effect
  col = hsv2rgb_approx(vec3(
    0.6,                                // Fixed hue for sky color
    clamp(0.3+0.9*rd.y, 0.0, 1.0),      // Saturation varies with vertical angle
    1.5*clamp(2.0-2.*rd.y*rd.y, 0.0, 2.) // Brightness with non-linear falloff
  ));

  // If ray intersects ground plane, render ground details
  if (bt > 0.) {
    // Compute intersection point on ground plane
    vec3 bp = ro + rd*bt;
    vec2 bpp = bp.xz;

    // Create grid coordinate system
    // Round coordinates to snap to grid points
    vec2 npp = round(bpp);
    vec2 cpp = bpp - npp;
    vec2 app = abs(cpp);

    // Grid line distance field with view-angle compensation
    // Reduces aliasing by adjusting line width based on view angle
    float gfre = 1.+rd.y;
    gfre *= gfre;
    gfre *= gfre;

    // Compute grid line distance
    // Dynamically adjusts line width based on view angle to reduce hard edges
    float gd = min(app.x, app.y) - mix(0.01, 0.0, gfre);

    // Ground base color using HSV approximation macro
    // Soft grayish tone with slight warmth
    const vec3 bbcol = HSV2RGB_APPROX(vec3(0.7, 0.2, 1.25));

    // Distance-based fade effect
    // Reduces ground detail and brightness at far distances
    float bfade = mix(1., 0.2, exp(-0.3*max(bt-15., 0.)));

    // Anti-aliasing width adjustment
    float aa = mix(0.0, 0.08, bfade);

    // Blend ground color with fading and grid line effects
    // Creates soft, slightly faded grid appearance
    vec3 bcol = mix(bbcol, bbcol*bfade, smoothstep(aa, -aa, gd));

    // Blend ground with sky, creating distance fog effect
    // Simulates atmospheric perspective
    col = mix(col, bcol, exp(-0.008*bt));
  }

  return col;
}
```

## Rendering the world

The world raytracer is not enough, we need to do a bit more to visualize it.

One thing that is a bit of a mind-meld in the beginning is how to setup the ray-direction given the fragment 2D coordinate.

The easy solution is just to copy paste the code that sets them up which is what everyone else does:

```glsl
// Set the starting point of the ray in 3D space
vec3 ro = rayOrigin;

// Define the "up" direction, used for camera orientation
const vec3 up = vec3(0.0, 1.0, 0.0);

// Set up the ray direction using a "look-at" camera model
// Normalize the direction from the ray origin to the look-at point
vec3 ww = normalize(lookAt - ro);

// Compute the right vector by crossing the up vector with the direction
vec3 uu = normalize(cross(up, ww));

// Compute the true "up" vector (orthogonal to both ww and uu)
vec3 vv = cross(ww, uu);

// Define the field of view (FOV); larger values mean a wider view
const float fov = 2.0;

// Compute the ray direction for this pixel
// Combine the perspective (FOV) and the camera's orientation
vec3 rd = normalize(-p.x * uu + p.y * vv + fov * ww);
```

If `p.y` is in range [-1,1] and `p.x` adjusted for screen ratio `rd` will then be the ray-direction. Then it's just to pop in the ray-origin and the ray-direction in the `renderWorld` function above to get the pixel color.

```
// Initialize the color accumulator
vec3 col = vec3(0.0);

// Render the scene by tracing the ray (ro: origin, rd: direction)
col = renderWorld(ro, rd);

// Saturate the colors a bit
col -= 0.03 * vec3(2.0, 3.0, 1.0) * (length(p) + 0.25);

// Apply a vignette effect to darken edges of the screen
col *= smoothstep(1.7, 0.8, length(pp));

// Tone map the color from high dynamic range (HDR) to standard [0,1] range
col = aces_approx(col);

// Simulate a gamma correction for RGB to sRGB conversion
col = sqrt(col);
```

The complete world renderer example. Note I have included constants and functions that are not in use yet but they will be used when we add the other raytracers. If I have done the job correctly [create a new shadertoy](https://www.shadertoy.com/new) and copy and paste the code below into shadertoy.


```glsl
// Macro definitions for built-in Shadertoy inputs
#define TIME        iTime        // Current time in seconds since shader start
#define RESOLUTION  iResolution  // Viewport resolution (width, height, 1)
// 2D rotation matrix creation macro - creates a rotation matrix for 2D transformations
#define ROT(a)      mat2(cos(a), sin(a), -sin(a), cos(a))

// Material optical properties
const float refr_index = 0.8;    // Refractive index - determines how much light bends
                                 // when passing through the material (< 1 means light
                                 // bends less than in typical materials)

// Mathematical and visual constants
const float pi      = acos(-1.); // More precise way to define pi using arccos
const float tau     = 2.*pi;     // Full circle rotation (2π)
const float upSat   = 1.2;       // Saturation boost for color intensity
const float phi     = (sqrt(5.)+1.)/2.; // Golden ratio - aesthetically pleasing proportion
const float beerHue = 0.9;       // Hue value for coloration (potentially for Beer's law)

// Global rotation matrix for dynamically rotating internal object
mat3 g_rot;

// Ray marching configuration for internal object rendering
// Ray marching is a technique to visualize 3D surfaces by stepping along a ray
const int   maxRayMarchesInsides   = 50;   // Maximum number of steps to find surface
                                           // (prevents infinite loops)
const float toleranceInsides       = .001; // Minimum distance to consider a surface hit
const float normalEpisolonInsides  = 0.001; // Small offset for calculating surface normals
const int   maxBouncesInsides      = 5;    // Limit on light bounces/reflections inside object
float g_glowDistanceInsides;               // Tracking glow effect distance

// Ray marching settings for external object rendering
const int   maxRayMarchesShapes = 70;      // More steps for complex external surfaces
const float toleranceShapes     = .001;    // Minimum distance to surface hit
const float maxRayLengthShapes  = 20.;     // Maximum ray travel distance to prevent
                                           // unnecessary computation
const float normalEpisolonShapes= 0.01;    // Slightly larger normal calculation precision
float g_glowDistanceShapes;                // Tracking glow effect for external objects

// Scene composition parameters
                                                           // (normalized and scaled)
const vec3 sunDir    = normalize(vec3(1.0)); // Directional light source
                                             // (normalized to unit vector)
const vec3  boxDim   = vec3(1., phi*phi, phi); // Object dimensions using golden ratio
                                               // for aesthetically pleasing proportions
const float boxEdge  = 0.005;  // Thickness of object's frame/outline
const float bottom   = -boxDim.y-0.033; // Ground level, slightly below the object

const vec3 rayOrigin = normalize(vec3(0.0, 3.0, -5.))*8.; // Camera position
const vec3 lookAt    = vec3(0.0, 0.5*bottom, 0.0); // Define a "look-at" point, where the camera is focusing

// Approximate HSV to RGB conversion by XorDev
// Creates smoother, more visually appealing color transitions compared to standard conversion
// Unique trigonometric approach that produces interesting color blending
// License: Unknown, author: XorDev, found: https://x.com/XorDev/status/1808902860677001297
vec3 hsv2rgb_approx(vec3 hsv) {
  // Trigonometric color transformation
  // Uses cosine waves with offset to create non-linear color transitions
  return (cos(hsv.x*tau+vec3(0.,4.,2.))*hsv.y+2.-hsv.y)*hsv.z/2.;
}

// Macro version of HSV to RGB conversion for performance optimization
#define  HSV2RGB_APPROX(hsv) ((cos(hsv.x*tau+vec3(0.,4.,2.))*upSat*hsv.y+2.-upSat*hsv.y)*hsv.z/2.)

// ACES Filmic Tone Mapping Approximation
// Compresses high dynamic range images to display on standard screens
// License: Unknown, author: Matt Taylor (https://github.com/64), found: https://64.github.io/tonemapping/
vec3 aces_approx(vec3 v) {
  // Ensure no negative values
  v = max(v, 0.0);

  // Reduce overall intensity
  v *= 0.6;

  // Coefficients for tone mapping curve
  // These values control how bright and contrasty the image appears
  float a = 2.51;
  float b = 0.03;
  float c = 2.43;
  float d = 0.59;
  float e = 0.14;

  // Apply tone mapping and clamp to valid color range
  return clamp((v*(a*v+b))/(v*(c*v+d)+e), 0.0, 1.0);
}

// "Fancy" animated rotation matrix
// Generates a time-dependent rotation matrix for dynamic effects
// I got it from Chat AI so likely "borrowed" from shadertoy.
mat3 animatedRotationMatrix(float time) {
  // Define three independent angles for rotation over time
  float angle1 = time * 0.5;       // Primary rotation (slower)
  float angle2 = time * 0.707;     // Secondary rotation (based on √2 for variety)
  float angle3 = time * 0.33;      // Tertiary rotation (even slower)

  // Precompute trigonometric values for efficiency
  float c1 = cos(angle1); float s1 = sin(angle1);
  float c2 = cos(angle2); float s2 = sin(angle2);
  float c3 = cos(angle3); float s3 = sin(angle3);

  // Construct a 3x3 rotation matrix
  // Combines rotations across multiple axes with varying speeds
  // Rows represent the transformed basis vectors
  return mat3(
      c1 * c2,               // X-axis scaling with first two rotations
      c1 * s2 * s3 - c3 * s1, // Y-axis rotation and scaling
      s1 * s3 + c1 * c3 * s2, // Z-axis interaction with all three rotations

      c2 * s1,               // X-axis influenced by secondary and tertiary rotations
      c1 * c3 + s1 * s2 * s3, // Y-axis affected by all three angles
      c3 * s1 * s2 - c1 * s3, // Z-axis with secondary and tertiary dependencies

      -s2,                   // X-axis negation for secondary rotation
      c2 * s3,               // Y-axis scaling for secondary and tertiary rotations
      c2 * c3                // Z-axis scaling for the primary and secondary angles
  );
}

// Soft minimum - smoothly interpolates between two values
// Creates a smooth blend instead of a hard transition
// License: MIT, author: Inigo Quilez, found: https://www.iquilezles.org/www/articles/smin/smin.htm
float pmin(float a, float b, float k) {
  // Calculates a smooth interpolation between a and b
  // k controls the smoothness of the transition
  float h = clamp(0.5+0.5*(b-a)/k, 0.0, 1.0);
  return mix(b, a, h) - k*h*(1.0-h);
}

// Soft maximum - complementary to soft minimum
float pmax(float a, float b, float k) {
  // Implemented by negating soft minimum
  return -pmin(-a, -b, k);
}

// 2D box distance function - calculates signed distance to a 2D box
// License: MIT, author: Inigo Quilez, found: https://iquilezles.org/articles/distfunctions/
float box(vec2 p, vec2 b) {
  // Calculates distance from point to box edges
  vec2 d = abs(p)-b;
  return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

// 3D box distance function - calculates signed distance to a 3D box
// License: MIT, author: Inigo Quilez, found: https://iquilezles.org/articles/distfunctions/
float box(vec3 p, vec3 b) {
  // Calculates distance from point to box surfaces
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

// "Super" sphere - a non-standard sphere distance function with unique shape
float ssphere4(vec3 p, float r) {
  // Creates a more complex spherical shape by using power-based distance calculation
  p *= p;
  return pow(dot(p, p), 0.25)-r;
}

// Torus distance function - calculates distance to a donut-shaped object
float torus(vec3 p, vec2 t) {
  // Computes distance from point to torus surface
  // t.x is ring radius, t.y is tube radius
  vec2 q = vec2(length(p.xz)-t.x, p.y);
  return length(q)-t.y;
}

// Box frame distance function - calculates distance to a wireframe box
// License: MIT, author: Inigo Quilez, found: https://iquilezles.org/articles/distfunctions/
float boxFrame(vec3 p, vec3 b, float e) {
  // Creates a wireframe box with specified dimensions and edge thickness
  // b: box dimensions, e: edge thickness
  p = abs(p)-b;
  vec3 q = abs(p+e)-e;
  return min(min(
      length(max(vec3(p.x,q.y,q.z),0.0))+min(max(p.x,max(q.y,q.z)),0.0),
      length(max(vec3(q.x,p.y,q.z),0.0))+min(max(q.x,max(p.y,q.z)),0.0)),
      length(max(vec3(q.x,q.y,p.z),0.0))+min(max(q.x,max(q.y,p.z)),0.0));
}

// Render the surrounding world environment
// Responsible for creating the background scene including sky and ground plane
vec3 renderWorld(vec3 ro, vec3 rd) {
  vec3 col = vec3(0.0);

  // Calculate distance to floor plane using ray-plane intersection
  // Uses ray origin (ro) and ray direction (rd) to compute intersection point
  float bt = -(ro.y-bottom)/(rd.y);

  // Generate sky color using HSV approximation
  // Color varies based on ray direction (up/down angle)
  // - Hue is fixed at blue-cyan (0.6)
  // - Saturation depends on vertical ray angle
  // - Brightness uses a quadratic falloff to create gradient effect
  col = hsv2rgb_approx(vec3(
    0.6,                                // Fixed hue for sky color
    clamp(0.3+0.9*rd.y, 0.0, 1.0),      // Saturation varies with vertical angle
    1.5*clamp(2.0-2.*rd.y*rd.y, 0.0, 2.) // Brightness with non-linear falloff
  ));

  // If ray intersects ground plane, render ground details
  if (bt > 0.) {
    // Compute intersection point on ground plane
    vec3 bp = ro + rd*bt;
    vec2 bpp = bp.xz;

    // Create grid coordinate system
    // Round coordinates to snap to grid points
    vec2 npp = round(bpp);
    vec2 cpp = bpp - npp;
    vec2 app = abs(cpp);

    // Grid line distance field with view-angle compensation
    // Reduces aliasing by adjusting line width based on view angle
    float gfre = 1.+rd.y;
    gfre *= gfre;
    gfre *= gfre;

    // Compute grid line distance
    // Dynamically adjusts line width based on view angle to reduce hard edges
    float gd = min(app.x, app.y) - mix(0.01, 0.0, gfre);

    // Ground base color using HSV approximation macro
    // Soft grayish tone with slight warmth
    const vec3 bbcol = HSV2RGB_APPROX(vec3(0.7, 0.2, 1.25));

    // Distance-based fade effect
    // Reduces ground detail and brightness at far distances
    float bfade = mix(1., 0.2, exp(-0.3*max(bt-15., 0.)));

    // Anti-aliasing width adjustment
    float aa = mix(0.0, 0.08, bfade);

    // Blend ground color with fading and grid line effects
    // Creates soft, slightly faded grid appearance
    vec3 bcol = mix(bbcol, bbcol*bfade, smoothstep(aa, -aa, gd));

    // Blend ground with sky, creating distance fog effect
    // Simulates atmospheric perspective
    col = mix(col, bcol, exp(-0.008*bt));
  }

  return col;
}


vec3 effect(vec2 p, vec2 pp) {
  // Set the starting point of the ray in 3D space
  vec3 ro = rayOrigin;

  // Define the "up" direction, used for camera orientation
  const vec3 up = vec3(0.0, 1.0, 0.0);

  // Apply a slight rotation to the ray origin for dynamic effects
  ro.xz *= ROT(0.1 * TIME);

  // Compute a time-based rotation matrix for animating objects
  g_rot = animatedRotationMatrix(0.707 * TIME);

  // Set up the ray direction using a "look-at" camera model
  // Normalize the direction from the ray origin to the look-at point
  vec3 ww = normalize(lookAt - ro);

  // Compute the right vector by crossing the up vector with the direction
  vec3 uu = normalize(cross(up, ww));

  // Compute the true "up" vector (orthogonal to both ww and uu)
  vec3 vv = cross(ww, uu);

  // Define the field of view (FOV); larger values mean a wider view
  const float fov = 2.0;

  // Compute the ray direction for this pixel
  // Combine the perspective (FOV) and the camera's orientation
  vec3 rd = normalize(-p.x * uu + p.y * vv + fov * ww);

  // Initialize the color accumulator
  vec3 col = vec3(0.0);

  // Render the scene by tracing the ray (ro: origin, rd: direction)
  col = renderWorld(ro, rd);

  // Saturate the colors a bit
  col -= 0.03 * vec3(2.0, 3.0, 1.0) * (length(p) + 0.25);

  // Apply a vignette effect to darken edges of the screen
  col *= smoothstep(1.7, 0.8, length(pp));

  // Tone map the color from high dynamic range (HDR) to standard [0,1] range
  col = aces_approx(col);

  // Simulate a gamma correction for RGB to sRGB conversion
  col = sqrt(col);

  // Return the final color
  return col;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
  // Normalize fragment coordinates to a [0,1] range
  vec2 q = fragCoord / RESOLUTION.xy;

  // Map coordinates to a [-1,1] range for ray tracing
  vec2 p = -1.0 + 2.0 * q;

  // Keep a copy of the original coordinates for effects like vignette
  vec2 pp = p;

  // Correct the aspect ratio of the coordinates
  p.x *= RESOLUTION.x / RESOLUTION.y;

  // Initialize the final color
  vec3 col = vec3(0.0);

  // Compute the color for this fragment using the effect function
  col = effect(p, pp);

  // Output the final color with full alpha (1.0)
  fragColor = vec4(col, 1.0);
}
```


