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

The complete world renderer example. Note I have included constants and functions that are not in use yet but they will be used when we add the other raytracers. If I have done the job correctly [create a new shadertoy](https://www.shadertoy.com/new) and copy and paste the code below into shadertoy, if you are lucky what you see is a rotating grid in white.


```glsl
// CC0: Shader Advent reflection shader example

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
const float pi      = acos(-1.);        // More precise way to define pi using arccos
const float tau     = 2.*pi;            // Full circle rotation (2π)
const float upSat   = 1.2;              // Saturation boost for color intensity
const float phi     = (sqrt(5.)+1.)/2.; // Golden ratio - aesthetically pleasing proportion
const float beerHue = 0.9;              // Hue value for coloration (Beer's law)

// Global rotation matrix for dynamically rotating internal objects
mat3 g_rot;

// Ray marching configuration for internal object rendering
// Ray marching is a technique to visualize 3D surfaces by stepping along a ray
const int   maxRayMarchesInsides   = 50;   // Maximum number of steps to find surface
                                           // (prevents infinite loops)
const float toleranceInsides       = .001; // Minimum distance to consider a surface hit
const float normalEpisolonInsides  = 0.001;// Small offset for calculating surface normals
const int   maxBouncesInsides      = 5;    // Limit on light bounces/reflections inside object
float g_glowDistanceInsides;               // Tracking glow effect distance

// Ray marching settings for external box rendering
const int   maxRayMarchesShapes = 70;      // More steps for complex external surfaces
const float toleranceShapes     = .001;    // Minimum distance to surface hit
const float maxRayLengthShapes  = 20.;     // Maximum ray travel distance to prevent
                                           // unnecessary computation
const float normalEpisolonShapes= 0.01;    // Slightly larger normal calculation precision
float g_glowDistanceShapes;                // Tracking glow effect for external box

// Scene composition parameters
const vec3 sunDir    = normalize(vec3(1.0));    // Directional light source
const vec3  boxDim   = vec3(1., phi*phi, phi);  // Box dimensions using golden ratio
                                                // for aesthetically pleasing proportions
const float boxEdge  = 0.005;                   // Thickness of box's frame/outline
const float bottom   = -boxDim.y-0.033;         // Ground level, slightly below the box

const vec3 rayOrigin = normalize(vec3(0.0, 3.0, -5.))*8.; // Camera position
const vec3 lookAt    = vec3(0.0, 0.5*bottom, 0.0);        // Define a "look-at" point, where the camera is focusing

// Approximate HSV to RGB conversion by XorDev
// Creates smoother, more visually appealing color transitions compared to standard conversion
// License: Unknown, author: XorDev, found: https://x.com/XorDev/status/1808902860677001297
vec3 hsv2rgb_approx(vec3 hsv) {
  // Trigonometric color transformation
  // Uses cosine waves with offset to create non-linear color transitions
  return (cos(hsv.x*tau+vec3(0.,4.,2.))*hsv.y+2.-hsv.y)*hsv.z/2.;
}
#define  HSV2RGB_APPROX(hsv) ((cos(hsv.x*tau+vec3(0.,4.,2.))*upSat*hsv.y+2.-upSat*hsv.y)*hsv.z/2.)

// ACES Filmic Tone Mapping Approximation
// Compresses high dynamic range images to display on standard screens
// License: Unknown, author: Matt Taylor (https://github.com/64), found: https://64.github.io/tonemapping/
vec3 aces_approx(vec3 v) {
  // Ensure no negative values
  v = max(v, 0.0);

  // Reduce overall intensity
  v *= 0.6;

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
      c1 * c2,                // X-axis scaling with first two rotations
      c1 * s2 * s3 - c3 * s1, // Y-axis rotation and scaling
      s1 * s3 + c1 * c3 * s2, // Z-axis interaction with all three rotations

      c2 * s1,                // X-axis influenced by secondary and tertiary rotations
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
  vec2 d = abs(p)-b;
  return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

// 3D box distance function - calculates signed distance to a 3D box
// License: MIT, author: Inigo Quilez, found: https://iquilezles.org/articles/distfunctions/
float box(vec3 p, vec3 b) {
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

// Torus distance function - calculates distance to a donut-shaped object
// License: MIT, author: Inigo Quilez, found: https://iquilezles.org/articles/distfunctions/
float torus(vec3 p, vec2 t) {
  // Computes distance from point to torus surface
  // t.x is ring radius, t.y is tube radius
  vec2 q = vec2(length(p.xz)-t.x, p.y);
  return length(q)-t.y;
}

// Box frame distance function - calculates distance to a wireframe box
// License: MIT, author: Inigo Quilez, found: https://iquilezles.org/articles/distfunctions/
float boxFrame(vec3 p, vec3 b, float e) {
  p = abs(p)-b;
  vec3 q = abs(p+e)-e;
  return min(min(
      length(max(vec3(p.x,q.y,q.z),0.0))+min(max(p.x,max(q.y,q.z)),0.0),
      length(max(vec3(q.x,p.y,q.z),0.0))+min(max(q.x,max(p.y,q.z)),0.0)),
      length(max(vec3(q.x,q.y,p.z),0.0))+min(max(q.x,max(q.y,p.z)),0.0));
}

// "Super" sphere - a boxy looking "sphere". Has nice normals
float ssphere4(vec3 p, float r) {
  p *= p;
  return pow(dot(p, p), 0.25)-r;
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
    // Dynamically adjusts line width based on view angle to reduce aliasing
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

## Rendering the box

Next step we will add a box to the world. This will be raymarched which is a very common technique used on ShaderToy for rending 3D objects. The idea is that you start in the ray-origin and then ask the distance field for the 3D object how far it is from the current point to the 3D object, then step that distance in the ray direction and ask how far the new point is from the 3D object. Repeat until you either hit the object (the distance is small) or you step too far or you hit the max iteration.

If you hit the object, use the stepped distance to compute the point of the object. Then using the distance field you can compute the normal (essentially by deriving the distance field around the point of the object).

With normal you can then compute the reflection direction, the refraction direction and apply cool lighting effects.

That is what we will be doing in so many words but let's look at the code.

### The distance field for the objects

Now for the next step add the function `dfShapes` under the function `renderWorld` in the previous example:

This is the distance field for the box from the outside. To add a bit more zing to it compute the frame of the box. This will be used to create a glowing box frame.

```glsl
// Distance field function for external box
float dfShapes(vec3 p) {
  // Compute distance to solid box
  float dbox = box(p, boxDim);

  // Compute distance to box frame for glow effect
  float dboxFrame = boxFrame(p, boxDim, 0.) - boxEdge;

  // Initialize distance to a large value
  float d = 1E3;

  // Set primary distance to box
  d = dbox;

  // Soften box edges using smooth maximum operation
  // Creates a more organic, less sharp edge appearance
  d = pmax(d, -(dboxFrame-2.*boxEdge), 8.*boxEdge);

  // Include box frame in distance calculation
  d = min(d, dboxFrame);

  // Track glow distance for visual effects
  float gd = 1E3;
  gd = dboxFrame;

  // Update global glow distance
  // Used for creating glowing edge/surface effects
  g_glowDistanceShapes = min(g_glowDistanceShapes, gd);

  return d;
}
```

### The ray marching loop

Next add the ray marcher code below `dfShapes`. This is the core of ray marchers as it uses the distance field to find the intersection point between the ray-direction and the 3D object. As mentioned above it achieves this by asking how far the current point is from the 3D objects. Then it steps that far in the ray direction. It loops until it either got close enough to the object, or travelled to far, or ran out of itertions.

In addition; I have included a trick I learnt from IQ that can reduce artifacts that occur when the raydirection narrowly misses an object. The code then steps back to the point was the nearest a surface and use that as a result. It's a neat trick but like many tricks it sometimes works great and sometimes don't. You have to experiment to see if it helps or not.


```glsl
// Ray marching algorithm for external box
// Finds intersection point by stepping along the ray
float rayMarchShapes(vec3 ro, vec3 rd, float tinit) {
  float t = tinit;

  // Optional backstep technique to reduce rendering artifacts
#if defined(BACKSTEP_SHAPES)
  vec2 dti = vec2(1e10,0.0);
#endif

  int i;
  for (i = 0; i < maxRayMarchesShapes; ++i) {
    // Compute distance to nearest surface
    float d = dfShapes(ro + rd*t);

    // Track closest approach for potential backstep
#if defined(BACKSTEP_SHAPES)
    if (d<dti.x) { dti=vec2(d,t); }
#endif

    // Stop if we're close to a surface or exceed max ray length
    if (d < toleranceShapes || t > maxRayLengthShapes) {
      break;
    }

    // Step along ray
    t += d;
  }

  // Backstep technique for missed rays
#if defined(BACKSTEP_SHAPES)
  if(i==maxRayMarchesShapes) { t=dti.y; };
#endif

  return t;
}
```

### Computing the normal

Coming up the function to compute the normal of the distance field. An absolutely crucial step. The normal is the direction that is perpendicalur to the object at the current point and is used for many tasks such as lighting and reflections.

Understanding the code might not be intuitive to most but the good news is that as long as you understand what a normal you can copy paste the code below.

Essentially what the code does is that it the rate of change of the distance field in the x,y and z direction, forms a vector from it and then normalizes it (sets the length to 1).

The episolon value is another parameter to tinker with, too small and you get lots of noise in the normals, too large and the normals looks "soft" around edges.

```glsl
// Compute surface normal using gradient of distance field
vec3 normalShapes(vec3 pos) {
  // Small offset for numerical gradient calculation
  const vec2 eps = vec2(normalEpisolonShapes, 0.0);

  // Compute normal by sampling distance field in small directions
  return normalize(vec3(
      dfShapes(pos+eps.xyy)-dfShapes(pos-eps.xyy)
    , dfShapes(pos+eps.yxy)-dfShapes(pos-eps.yxy)
    , dfShapes(pos+eps.yyx)-dfShapes(pos-eps.yyx))
    );
}
```

### Computing the color by putting it all together

Now with the distance field defined, a ray marcher defined and a way to compute the normal we can compute the color.

We use the `renderWorld` function from before to compute the background and the reflection on the box.

To fake some shadows we also compute the distance to the ground floor `bd`.

Using the ray marcher function above we find the distance to our box and compute the normal of the intersection point, this allows us to compute the reflection vector.

If the ray hit the box before the ray hits the bottom floor we compute the color of the box. The condition for the hit is a bit of a mouthful so here's breaking it down:

`if (st < maxRayLengthShapes && (bt < 0.0 || st < bt)) {`

1. `st < maxRayLengthShapes` - did we hit the box?
2. `bt < 0.0` - did we hit the sky (ie the ground hit is behind us)?
3. `st < bt` - If we didn't hit the sky is the ground hit further away than the box?

If so, we hit the object and thus we compute the color for the box. We do this by calling `renderWorld` in the reflected direction, multiply with `refCol` to give a bit purple color to the reflection and finally multiply with fresnel factor `sfre`

```code
if (st < maxRayLengthShapes && (bt < 0.0 || st < bt)) {
  // Ray hit the object
  // Render reflections on object's surface
  vec3 rwcol = renderWorld(sp, sr);

  col = rwcol*refCol*sfre;
}
```

The fresnel effect can be observed if you stand by water and look straight down, the sky almost don't reflect at all. When you lift your eyes towards the horizon the sky reflection gets stronger and stronger because the more the light rays hit the water surface on an angle the strong the reflection is. Many surface materials have the fresnel effect..

There's a proper formula for all this but I tinker with stuff until I think it looks right. So the first step is to compute
```glsl
float sfre = 1.+dot(rd,sn);
```

`sfre` will 0 when you look straight at the object, because the ray direction `rd` and the surface normal `sn` point in opposite direction, the more the surface curve away from you the dot product goes towards 0 meaning `sfre` will be 1 when the ray direction and the surface normal are perpendicular.

I then multiply the fresnel value with itself to reduce reflection when looking almost head on. How many times do I do it? Until it looks good.

Then finally I want to retain some reflection so I mix the fresnel value. All in all it looks like this:

```glsl
// Fake fresnel effect (reflection intensity based on view angle)
float sfre = 1.+dot(rd,+sn);
sfre *= sfre;
sfre = mix(0.05, 1.0,sfre);
```

It's fake because it's inspired by nature, not physically "correct".

If we hit the floor instead then use the distance from the box to compute a cheap shadow effect.

```glsl
} else if (bt > 0.0) {
  // Ray hit the floor
  // Apply fake shadow effect
  col *= mix(1.0, 0.125, exp(-bd));
}
```
And finally we use the glow distance to apply the glow effect. Very simple.

```glsl
// Add glow effect to the rendering
const vec3 glowCol = HSV2RGB_APPROX(vec3(0.66,0.5, 4E-3));
col += glowCol/max(sglowDistance, toleranceShapes);
```

All in all it looks like this:

```glsl
// Render external box and their interactions
vec3 renderShapes(vec3 ro, vec3 rd) {
  // Start with world background rendering
  vec3 col = renderWorld(ro, rd);

  // Calculate distance to floor plane
  float bt = -(ro.y-bottom)/(rd.y);
  vec3 bp = ro+rd*bt;

  // Compute floor distance for fake shadow effect
  float bd = dfShapes(bp);

  // Reset glow distance tracking
  g_glowDistanceShapes = 1E3;

  // Ray march to find intersection with external box
  float st = rayMarchShapes(ro, rd, 0.);
  float sglowDistance = g_glowDistanceShapes;

  // Compute intersection point and surface properties
  vec3 sp = ro+rd*st;
  vec3 sn = normalShapes(sp);

  vec3 sr = reflect(rd,sn);

  // Fake fresnel effect (reflection intensity based on view angle)
  float sfre = 1.+dot(rd,+sn);
  sfre *= sfre;
  sfre = mix(0.05, 1.0,sfre);

  // Reflection color
  const vec3 refCol = HSV2RGB_APPROX(vec3(beerHue, 2./3., 1./3.));

  if (st < maxRayLengthShapes && (bt < 0.0 || st < bt)) {
    // Ray hit the object
    // Render reflections on object's surface
    vec3 rwcol = renderWorld(sp, sr);

    col = rwcol*refCol*sfre;

  } else if (bt > 0.0) {
    // Ray hit the floor
    // Apply fake shadow effect
    col *= mix(1.0, 0.125, exp(-bd));
  }

  // Add glow effect to the rendering
  const vec3 glowCol = HSV2RGB_APPROX(vec3(0.66,0.5, 4E-3));
  col += glowCol/max(sglowDistance, toleranceShapes);

  return col;
}
```

### Putting it all together

Now in order to show the box you have to call `renderShapes` from `effect` rather than `renderWorld` like so:

```glsl
// Render the scene by tracing the ray (ro: origin, rd: direction)
col = renderShapes(ro, rd);
```

With some luck you should see a slowly rotating box with subtle reflections.

## Rendering the insides

Now it's time for the inside. Here the plan is to let the ray bounce back and forth inside the box accumulating the glow from the edges. To kick things up a notch I also add a rotating super sphere + glowing torus in the middle.

The distance field function and the ray marcher is mostly the same as the outside so let's start with inside distance field

### The distance field for the inside

Add the `dfInsides` function below the `renderWorld` function.

It's is similar to the outside distancefield except we inverted the box distance field as we are now inside of it. In addition we add a super sphere (kind of like a boxy sphere) and a torus and substract the torus from the super sphere "smoothly" to create nice looking normals. In order to subtract one shapes from another we usually do `max(distanceObject0, -distanceObject1)` but in order to do it smoothly we use the very useful soft max function `pmax`.

```glsl
// "Super" sphere with a glowing torus
float dsphere = ssphere4(p0, 0.7);
float dtorus  = torus(p0, 0.707*vec2(1.0, 0.025));

// Combine sphere and torus with smooth boolean operation
// Creates a more interesting shape by subtracting torus from sphere
dsphere = pmax(dsphere, -(dtorus-0.05), 0.05);
```

All in all it looks like this:

```glsl
// Distance field function for objects inside the box
// Calculates signed distances to various internal geometric shapes
float dfInsides(vec3 p) {
  // Negative box distance (we are inside the box)
  float dbox = -box(p, boxDim);

  // Create a copy of point for rotation
  vec3 p0 = p;
  // Apply global rotation matrix to the point
  p0 *= g_rot;

  // Create box frame with slight thickness
  float dboxFrame = boxFrame(p, boxDim, 0.) - boxEdge;

  // "Super" sphere with a glowing torus
  float dsphere = ssphere4(p0, 0.7);
  float dtorus  = torus(p0, 0.707*vec2(1.0, 0.025));

  // Combine sphere and torus with smooth boolean operation
  // Creates a more interesting shape by subtracting torus from sphere
  dsphere = pmax(dsphere, -(dtorus-0.05), 0.05);

  // Initialize distance to a large value
  float d = 1E3;

  // Combine the shapes
  d = dbox;
  d = min(d, dsphere);
  d = min(d, dtorus);

  // Compute glow distance
  float gd = 1E3;
  gd = dboxFrame;
  gd = min(gd, dtorus);

  // Global variable to track minimum glow distance
  // Used for creating glowing edge/surface effects
  g_glowDistanceInsides = min(g_glowDistanceInsides, gd);

  return d;
}
```

### Just a "normal" ray marcher

The ray marcher and the compute normal function are almost identical to the previous one but as we can't "miss" the inside I removed that check in the loop

Add the following code under the `dfInsides` function above:

```glsl
// Ray marching algorithm for interior of box
// Finds intersection point by stepping along the ray
float rayMarchInsides(vec3 ro, vec3 rd, float tinit) {
  float t = tinit;

  // Optional backstep technique to reduce rendering artifacts
  // Inspired by Inigo Quilez's techniques
  // Helps smooth out issues when ray intersects surface at shallow angles
#if defined(BACKSTEP_INSIDES)
  vec2 dti = vec2(1e10,0.0);
#endif

  int i;
  for (i = 0; i < maxRayMarchesInsides; ++i) {
    // Compute distance to nearest surface
    float d = dfInsides(ro + rd*t);

    // Track closest approach for potential backstep
#if defined(BACKSTEP_INSIDES)
    if (d<dti.x) { dti=vec2(d,t); }
#endif

    // Stop if we're close enough to a surface
    if (d < toleranceInsides) {
      break;
    }

    // Step along ray
    t += d;
  }

  // Backstep technique for missed rays
#if defined(BACKSTEP_INSIDES)
  if(i==maxRayMarchesInsides) { t=dti.y; };
#endif

  return t;
}

// Compute surface normal using gradient of distance field
// Essential for lighting, reflections, and shading calculations
vec3 normalInsides(vec3 pos) {
  // Small offset for numerical gradient calculation
  const vec2 eps = vec2(normalEpisolonInsides, 0.0);

  // Compute normal by sampling distance field in small directions
  return normalize(vec3(
      dfInsides(pos+eps.xyy)-dfInsides(pos-eps.xyy)
    , dfInsides(pos+eps.yxy)-dfInsides(pos-eps.yxy)
    , dfInsides(pos+eps.yyx)-dfInsides(pos-eps.yyx))
    );
}
```

### Compute the color the insides

The plan here is that we will bounce the ray back and forth inside. How we do is that we start at a point inside the box and find the intersection between the ray and the inside objects. Compute the normal and produce the reflection ray. Then find the inserection of the reflect ray and the inside objects until we run out of bounces (say 5).

In order to produce colors we compute the glow colors like above but we accumulate the colors for each bounce in the variable `agg`. To make the colors fade out as each bounce absorbs some of the light and this is kept track of the variable `ragg` that starts at 1 and goes toward 0. In addition, we keep track of the total distance travelled in variable `tagg` which we use to compute color absorbtion using a simplied Beer's law formula.

All in all the code looks like this, add it below the function `normalInsides` above:

```glsl
// Render the interior of the box
// Handles multiple light bounces, absorption, and glow effects
vec3 renderInsides(vec3 ro, vec3 rd, float db) {
  // Accumulated color from multiple reflections
  vec3 agg = vec3(0.0);

  // Reflection intensity factor
  float ragg = 1.;

  // Total distance traveled
  float tagg = 0.;

  // Multiple bounce light simulation
  for (int bounce = 0; bounce < maxBouncesInsides; ++bounce) {
    // Stop if reflection is too weak
    if (ragg < 0.1) break;

    // Reset glow distance for this bounce
    g_glowDistanceInsides = 1E3;

    // Find intersection point
    float it  = rayMarchInsides(ro, rd, db);
    float glowDistanceInsides = g_glowDistanceInsides;

    // Accumulate total distance
    tagg += it;

    // Compute intersection point and surface normal
    vec3 ip     = ro+rd*it;
    vec3 in_    = normalInsides(ip);

    // Compute reflection vector
    vec3 ir     = reflect(rd, in_);

    // Fake fresnel effect (reflection intensity based on view angle)
    float ifre  = 1.+dot(in_,rd);
    ifre *= ifre;

    // Color absorption using Beer's law
    // Simulates how light is absorbed when traveling through a medium
    const vec3 beerCol = -HSV2RGB_APPROX(vec3(beerHue+0.5, 0.5, 1.0));
    vec3 beer = ragg*exp(0.2*beerCol*tagg);

    // Glow color for internal edges and surfaces
    const vec3 glowCol = HSV2RGB_APPROX(vec3(0.95, 0.7, 2E-3));

    // Add glow effect with distance-based blurring
    // Creates soft, glowing internal edges
    agg += glowCol*beer*((1.+tagg*tagg*4E-2)*6./max(glowDistanceInsides, 5E-4+tagg*tagg*2E-4/ragg));

    // Update reflection intensity
    ragg *= mix(0.6, 0.8, ifre);

    // Stop reflecting if we hit a non-reflective surface
    if (glowDistanceInsides < 2.*toleranceInsides) {
      ragg = 0.;
    }

    // Prepare for next bounce
    ro = ip;        // New ray origin
    rd = ir;        // New ray direction (reflection)

    // Adaptive step distance for next ray
    // Helps prevent self-intersection artifacts
    db = min(max(glowDistanceInsides,0.05), 0.25);
  }

  return agg;
}
```

### Put it all together

Now we are ready to put all together by updating the function `renderShapes`. Now in addition to compute the render world reflection we will compute refraction vector before stepping into the insides. Then we call the `renderInsides` function and accumulate the colors.

So find the code that starts with:

```glsl
// Fake fresnel effect (reflection intensity based on view angle)
float sfre = 1.+dot(rd,+sn);
sfre *= sfre;
```

and replace it and the `if` statement below with this:

```glsl
// Fake fresnel effect (reflection intensity based on view angle)
float sfre = 1.+dot(rd,+sn);
sfre *= sfre;
sfre = mix(0.05, 1.0,sfre);

// Reflection color
const vec3 refCol = HSV2RGB_APPROX(vec3(beerHue, 2./3., 1./3.));

if (st < maxRayLengthShapes && (bt < 0.0 || st < bt)) {
  // Ray hit the object
  // Render reflections on object's surface
  vec3 rwcol = renderWorld(sp, sr);
  vec3 ricol = vec3(0.);

  // Render inside of object if not hitting glow
  if (sglowDistance > 2.*toleranceShapes) {
    ricol = renderInsides(sp, srr, min(max(sglowDistance,0.05), 0.25))*(1.-sfre);
  }

  // Handle total internal reflection
  // Can occur when refraction index is > 1
  if (srr == vec3(0.0)) {
    col = rwcol*sqrt(sfre)*refCol;
  } else  {
    // Mix between inside rendering and surface reflections
    col = mix(ricol*smoothstep(0., 0.25, sglowDistance), rwcol*refCol, sfre);
  }

} else if (bt > 0.0) {
  // Ray hit the floor
  // Apply fake shadow effect
  col *= mix(1.0, 0.125, exp(-bd));
}
```

If you are really lucky should get a rotating box with internally reflecting glowing lines. If you struggle getting it to work I have prepared the [entire example in ShaderToy](https://www.shadertoy.com/view/4fcfWH).

Here is the entire source code:

```glsl
// CC0: Shader Advent reflection shader example

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
const float pi      = acos(-1.);        // More precise way to define pi using arccos
const float tau     = 2.*pi;            // Full circle rotation (2π)
const float upSat   = 1.2;              // Saturation boost for color intensity
const float phi     = (sqrt(5.)+1.)/2.; // Golden ratio - aesthetically pleasing proportion
const float beerHue = 0.9;              // Hue value for coloration (Beer's law)

// Global rotation matrix for dynamically rotating internal objects
mat3 g_rot;

// Ray marching configuration for internal object rendering
// Ray marching is a technique to visualize 3D surfaces by stepping along a ray
const int   maxRayMarchesInsides   = 50;   // Maximum number of steps to find surface
                                           // (prevents infinite loops)
const float toleranceInsides       = .001; // Minimum distance to consider a surface hit
const float normalEpisolonInsides  = 0.001;// Small offset for calculating surface normals
const int   maxBouncesInsides      = 5;    // Limit on light bounces/reflections inside object
float g_glowDistanceInsides;               // Tracking glow effect distance

// Ray marching settings for external box rendering
const int   maxRayMarchesShapes = 70;      // More steps for complex external surfaces
const float toleranceShapes     = .001;    // Minimum distance to surface hit
const float maxRayLengthShapes  = 20.;     // Maximum ray travel distance to prevent
                                           // unnecessary computation
const float normalEpisolonShapes= 0.01;    // Slightly larger normal calculation precision
float g_glowDistanceShapes;                // Tracking glow effect for external box

// Scene composition parameters
const vec3 sunDir    = normalize(vec3(1.0));    // Directional light source
const vec3  boxDim   = vec3(1., phi*phi, phi);  // Box dimensions using golden ratio
                                                // for aesthetically pleasing proportions
const float boxEdge  = 0.005;                   // Thickness of box's frame/outline
const float bottom   = -boxDim.y-0.033;         // Ground level, slightly below the box

const vec3 rayOrigin = normalize(vec3(0.0, 3.0, -5.))*8.; // Camera position
const vec3 lookAt    = vec3(0.0, 0.5*bottom, 0.0);        // Define a "look-at" point, where the camera is focusing

// Approximate HSV to RGB conversion by XorDev
// Creates smoother, more visually appealing color transitions compared to standard conversion
// License: Unknown, author: XorDev, found: https://x.com/XorDev/status/1808902860677001297
vec3 hsv2rgb_approx(vec3 hsv) {
  // Trigonometric color transformation
  // Uses cosine waves with offset to create non-linear color transitions
  return (cos(hsv.x*tau+vec3(0.,4.,2.))*hsv.y+2.-hsv.y)*hsv.z/2.;
}
#define  HSV2RGB_APPROX(hsv) ((cos(hsv.x*tau+vec3(0.,4.,2.))*upSat*hsv.y+2.-upSat*hsv.y)*hsv.z/2.)

// ACES Filmic Tone Mapping Approximation
// Compresses high dynamic range images to display on standard screens
// License: Unknown, author: Matt Taylor (https://github.com/64), found: https://64.github.io/tonemapping/
vec3 aces_approx(vec3 v) {
  // Ensure no negative values
  v = max(v, 0.0);

  // Reduce overall intensity
  v *= 0.6;

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
      c1 * c2,                // X-axis scaling with first two rotations
      c1 * s2 * s3 - c3 * s1, // Y-axis rotation and scaling
      s1 * s3 + c1 * c3 * s2, // Z-axis interaction with all three rotations

      c2 * s1,                // X-axis influenced by secondary and tertiary rotations
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
  vec2 d = abs(p)-b;
  return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

// 3D box distance function - calculates signed distance to a 3D box
// License: MIT, author: Inigo Quilez, found: https://iquilezles.org/articles/distfunctions/
float box(vec3 p, vec3 b) {
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

// Torus distance function - calculates distance to a donut-shaped object
// License: MIT, author: Inigo Quilez, found: https://iquilezles.org/articles/distfunctions/
float torus(vec3 p, vec2 t) {
  // Computes distance from point to torus surface
  // t.x is ring radius, t.y is tube radius
  vec2 q = vec2(length(p.xz)-t.x, p.y);
  return length(q)-t.y;
}

// Box frame distance function - calculates distance to a wireframe box
// License: MIT, author: Inigo Quilez, found: https://iquilezles.org/articles/distfunctions/
float boxFrame(vec3 p, vec3 b, float e) {
  p = abs(p)-b;
  vec3 q = abs(p+e)-e;
  return min(min(
      length(max(vec3(p.x,q.y,q.z),0.0))+min(max(p.x,max(q.y,q.z)),0.0),
      length(max(vec3(q.x,p.y,q.z),0.0))+min(max(q.x,max(p.y,q.z)),0.0)),
      length(max(vec3(q.x,q.y,p.z),0.0))+min(max(q.x,max(q.y,p.z)),0.0));
}

// "Super" sphere - a boxy looking "sphere". Has nice normals
float ssphere4(vec3 p, float r) {
  p *= p;
  return pow(dot(p, p), 0.25)-r;
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
    // Dynamically adjusts line width based on view angle to reduce aliasing
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
    col = mix(col, bcol, exp(-0.008*bt));
  }

  return col;
}

// Distance field function for objects inside the box
// Calculates signed distances to various internal geometric shapes
float dfInsides(vec3 p) {
  // Negative box distance (we are inside the box)
  float dbox = -box(p, boxDim);

  // Create a copy of point for rotation
  vec3 p0 = p;
  // Apply global rotation matrix to the point
  p0 *= g_rot;

  // Create box frame with slight thickness
  float dboxFrame = boxFrame(p, boxDim, 0.) - boxEdge;

  // "Super" sphere with a glowing torus
  float dsphere = ssphere4(p0, 0.7);
  float dtorus  = torus(p0, 0.707*vec2(1.0, 0.025));

  // Combine sphere and torus with smooth boolean operation
  // Creates a more interesting shape by subtracting torus from sphere
  dsphere = pmax(dsphere, -(dtorus-0.05), 0.05);

  // Initialize distance to a large value
  float d = 1E3;

  // Combine the shapes
  d = dbox;
  d = min(d, dsphere);
  d = min(d, dtorus);

  // Compute glow distance
  float gd = 1E3;
  gd = dboxFrame;
  gd = min(gd, dtorus);

  // Global variable to track minimum glow distance
  // Used for creating glowing edge/surface effects
  g_glowDistanceInsides = min(g_glowDistanceInsides, gd);

  return d;
}

// Ray marching algorithm for interior of box
// Finds intersection point by stepping along the ray
float rayMarchInsides(vec3 ro, vec3 rd, float tinit) {
  float t = tinit;

  // Optional backstep technique to reduce rendering artifacts
  // Inspired by Inigo Quilez's techniques
  // Helps smooth out issues when ray intersects surface at shallow angles
#if defined(BACKSTEP_INSIDES)
  vec2 dti = vec2(1e10,0.0);
#endif

  int i;
  for (i = 0; i < maxRayMarchesInsides; ++i) {
    // Compute distance to nearest surface
    float d = dfInsides(ro + rd*t);

    // Track closest approach for potential backstep
#if defined(BACKSTEP_INSIDES)
    if (d<dti.x) { dti=vec2(d,t); }
#endif

    // Stop if we're close enough to a surface
    if (d < toleranceInsides) {
      break;
    }

    // Step along ray
    t += d;
  }

  // Backstep technique for missed rays
#if defined(BACKSTEP_INSIDES)
  if(i==maxRayMarchesInsides) { t=dti.y; };
#endif

  return t;
}

// Compute surface normal using gradient of distance field
// Essential for lighting, reflections, and shading calculations
vec3 normalInsides(vec3 pos) {
  // Small offset for numerical gradient calculation
  const vec2 eps = vec2(normalEpisolonInsides, 0.0);

  // Compute normal by sampling distance field in small directions
  return normalize(vec3(
      dfInsides(pos+eps.xyy)-dfInsides(pos-eps.xyy)
    , dfInsides(pos+eps.yxy)-dfInsides(pos-eps.yxy)
    , dfInsides(pos+eps.yyx)-dfInsides(pos-eps.yyx))
    );
}

// Render the interior of the box
// Handles multiple light bounces, absorption, and glow effects
vec3 renderInsides(vec3 ro, vec3 rd, float db) {
  // Accumulated color from multiple reflections
  vec3 agg = vec3(0.0);

  // Reflection intensity factor
  float ragg = 1.;

  // Total distance traveled
  float tagg = 0.;

  // Multiple bounce light simulation
  for (int bounce = 0; bounce < maxBouncesInsides; ++bounce) {
    // Stop if reflection is too weak
    if (ragg < 0.1) break;

    // Reset glow distance for this bounce
    g_glowDistanceInsides = 1E3;

    // Find intersection point
    float it  = rayMarchInsides(ro, rd, db);
    float glowDistanceInsides = g_glowDistanceInsides;

    // Accumulate total distance
    tagg += it;

    // Compute intersection point and surface normal
    vec3 ip     = ro+rd*it;
    vec3 in_    = normalInsides(ip);

    // Compute reflection vector
    vec3 ir     = reflect(rd, in_);

    // Fake fresnel effect (reflection intensity based on view angle)
    float ifre  = 1.+dot(in_,rd);
    ifre *= ifre;

    // Color absorption using Beer's law
    // Simulates how light is absorbed when traveling through a medium
    const vec3 beerCol = -HSV2RGB_APPROX(vec3(beerHue+0.5, 0.5, 1.0));
    vec3 beer = ragg*exp(0.2*beerCol*tagg);

    // Glow color for internal edges and surfaces
    const vec3 glowCol = HSV2RGB_APPROX(vec3(0.95, 0.7, 2E-3));

    // Add glow effect with distance-based blurring
    // Creates soft, glowing internal edges
    agg += glowCol*beer*((1.+tagg*tagg*4E-2)*6./max(glowDistanceInsides, 5E-4+tagg*tagg*2E-4/ragg));

    // Update reflection intensity
    ragg *= mix(0.6, 0.8, ifre);

    // Stop reflecting if we hit a non-reflective surface
    if (glowDistanceInsides < 2.*toleranceInsides) {
      ragg = 0.;
    }

    // Prepare for next bounce
    ro = ip;        // New ray origin
    rd = ir;        // New ray direction (reflection)

    // Adaptive step distance for next ray
    // Helps prevent self-intersection artifacts
    db = min(max(glowDistanceInsides,0.05), 0.25);
  }

  return agg;
}

// Distance field function for external box
float dfShapes(vec3 p) {
  // Compute distance to solid box
  float dbox = box(p, boxDim);

  // Compute distance to box frame for glow effect
  float dboxFrame = boxFrame(p, boxDim, 0.) - boxEdge;

  // Initialize distance to a large value
  float d = 1E3;

  // Set primary distance to box
  d = dbox;

  // Soften box edges using smooth maximum operation
  // Creates a more organic, less sharp edge appearance
  d = pmax(d, -(dboxFrame-2.*boxEdge), 8.*boxEdge);

  // Include box frame in distance calculation
  d = min(d, dboxFrame);

  // Track glow distance for visual effects
  float gd = 1E3;
  gd = dboxFrame;

  // Update global glow distance
  // Used for creating glowing edge/surface effects
  g_glowDistanceShapes = min(g_glowDistanceShapes, gd);

  return d;
}

// Ray marching algorithm for external box
// Finds intersection point by stepping along the ray
float rayMarchShapes(vec3 ro, vec3 rd, float tinit) {
  float t = tinit;

  // Optional backstep technique to reduce rendering artifacts
#if defined(BACKSTEP_SHAPES)
  vec2 dti = vec2(1e10,0.0);
#endif

  int i;
  for (i = 0; i < maxRayMarchesShapes; ++i) {
    // Compute distance to nearest surface
    float d = dfShapes(ro + rd*t);

    // Track closest approach for potential backstep
#if defined(BACKSTEP_SHAPES)
    if (d<dti.x) { dti=vec2(d,t); }
#endif

    // Stop if we're close to a surface or exceed max ray length
    if (d < toleranceShapes || t > maxRayLengthShapes) {
      break;
    }

    // Step along ray
    t += d;
  }

  // Backstep technique for missed rays
#if defined(BACKSTEP_SHAPES)
  if(i==maxRayMarchesShapes) { t=dti.y; };
#endif

  return t;
}

// Compute surface normal using gradient of distance field
vec3 normalShapes(vec3 pos) {
  // Small offset for numerical gradient calculation
  const vec2 eps = vec2(normalEpisolonShapes, 0.0);

  // Compute normal by sampling distance field in small directions
  return normalize(vec3(
      dfShapes(pos+eps.xyy)-dfShapes(pos-eps.xyy)
    , dfShapes(pos+eps.yxy)-dfShapes(pos-eps.yxy)
    , dfShapes(pos+eps.yyx)-dfShapes(pos-eps.yyx))
    );
}

// Render external box and their interactions
vec3 renderShapes(vec3 ro, vec3 rd) {
  // Start with world background rendering
  vec3 col = renderWorld(ro, rd);

  // Calculate distance to floor plane
  float bt = -(ro.y-bottom)/(rd.y);
  vec3 bp = ro+rd*bt;

  // Compute floor distance for fake shadow effect
  float bd = dfShapes(bp);

  // Reset glow distance tracking
  g_glowDistanceShapes = 1E3;

  // Ray march to find intersection with external box
  float st = rayMarchShapes(ro, rd, 0.);
  float sglowDistance = g_glowDistanceShapes;

  // Compute intersection point and surface properties
  vec3 sp = ro+rd*st;
  vec3 sn = normalShapes(sp);

  // Compute reflection and refraction vectors
  vec3 sr = reflect(rd,sn);
  vec3 srr= refract(rd,sn, refr_index);

  // Fake fresnel effect (reflection intensity based on view angle)
  float sfre = 1.+dot(rd,+sn);
  sfre *= sfre;
  sfre = mix(0.05, 1.0,sfre);

  // Reflection color
  const vec3 refCol = HSV2RGB_APPROX(vec3(beerHue, 2./3., 1./3.));

  if (st < maxRayLengthShapes && (bt < 0.0 || st < bt)) {
    // Ray hit the object
    // Render reflections on object's surface
    vec3 rwcol = renderWorld(sp, sr);
    vec3 ricol = vec3(0.);

    // Render inside of object if not hitting glow
    if (sglowDistance > 2.*toleranceShapes) {
      ricol = renderInsides(sp, srr, min(max(sglowDistance,0.05), 0.25))*(1.-sfre);
    }

    // Handle total internal reflection
    // Can occur when refraction index is > 1
    if (srr == vec3(0.0)) {
      col = rwcol*sqrt(sfre)*refCol;
    } else  {
      // Mix between inside rendering and surface reflections
      col = mix(ricol*smoothstep(0., 0.25, sglowDistance), rwcol*refCol, sfre);
    }

  } else if (bt > 0.0) {
    // Ray hit the floor
    // Apply fake shadow effect
    col *= mix(1.0, 0.125, exp(-bd));
  }

  // Add glow effect to the rendering
  const vec3 glowCol = HSV2RGB_APPROX(vec3(0.66,0.5, 4E-3));
  col += glowCol/max(sglowDistance, toleranceShapes);

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
  col = renderShapes(ro, rd);

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

## 🎉 Let's reflect on what we done 🎉

We reached the end of the blog and hopefully it helped you out somewhat on how a shader that supports reflection could look like.

A few words before we return to the Christmas dinner.

When I first studied shaders by other people I was often confounded by the strange constants or weird combinations of variables. "How did they arrive at this specific code?" I often wondered looking for a deeper reason. While there certainly can be a hidden meaning shader tinkerers spend a lot of time tinkering with code and constants hunting that perfect look. It often start from intiution but at least when it comes to me I try many variants to get what I am looking for. I very seldomly sit down with pen and paper and calculate although that does happen. That's what so great with shader coding, it's a very interactive process where you can develop and use your mathematical and physical intuition guide you without being bogged down by boring reality. The only metric that matters is that it looks cool. So go ahead and tinker like madmen until you get what you want.

Another technique I wanted to show is the use of multiple different ray tracers in the same shader. Here we have three `renderWorld`, `renderInsides` and `renderShapes`. When I started shader tinkering I often tried to build one gigantic ray tracer that solved it all but I have stepped away from it. I think using specialized ray tracers allows me to optimize them for what I need them to do. For example I use certain anti-aliasing techniques in `renderWorld` ray tracer that can be tricky to implement in a general ray tracer. Why pay the price of handling reflections in the `renderShapes` when I only need it on the inside of the box?


✨🎄🎁 Merry Christmas, and reflect some normals! 🎁🎄✨

🎅 - mrange

## ❄️Licensing Information❄️

All code content I created for this blog post is licensed under [CC0](https://creativecommons.org/public-domain/cc0/) (effectively public domain). Any code snippets from other developers retain their original licenses.

The text content of this blog is licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) (the same license as Stack Overflow).