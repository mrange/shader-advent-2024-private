# 🎄⭐🎉 Introduction to Ray Marching 🎉⭐🎄

🎅 Ho, ho, ho! Merry Shader-mas! 🎅

## 🎄 Wait... *Another* Ray Marching Blog Post? 🎄

Yes, yes, I know—there are plenty of blog posts on ray marching out there. It's a bit like the programming equivalent of a Christmas fruitcake: everyone who gets a taste of it has to make their own version!

So why add one more? Well, if you’ve ever browsed [ShaderToy](https://www.shadertoy.com/) and found yourself dazzled (or mystified) by those mind-bending shaders, understanding ray marching is like having the recipe. Once you get it, everything starts to click because so many ShaderToy examples are built around it.

### So, what *is* Ray Marching?

A ray marcher is a type of ray tracer, but it uses distance fields to define and render 3D objects. Imagine tracing a line through a scene, but instead of traditional rendering, we’re checking distances to objects, step by step, until we hit something (or not).

Let’s jump in and build one together. If you're ready, create a [new shader on ShaderToy](https://www.shadertoy.com/new) so we can start from scratch!

## 🌠 Setting Up the Ray 🌠

To trace a ray through our scene, we need two things: a **ray origin** and a **ray direction vector**.

- **Ray Origin**: This one’s easy—we’ll start from `(0, 0, -10)`, a good spot to visualize things.
- **Ray Direction**: Here, we’ve got options, but one simple way to get the direction vector is the following:

```glsl
  // p is the coordinate with (0,0) in the center of the screen
  // (0,1) is top of screen and (0,-1) is the bottom
  // Thus when p is (0,0) it points in (0,0,1).
  // We normalize as a direction vector should have length=1
  vec3 rayDirection = normalize(vec3(p,1));
```

Here's the complete example:

```glsl
void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
  // Takes fragCoord and transform it into where p (0,0) is in the center of the screen
  // and (0,1) is top of screen and (0,-1) is the bottom.
  vec2 p = (-iResolution.xy+2.0*fragCoord)/iResolution.yy;

  // Setup a ray origin
  vec3 rayOrigin = vec3(0.0,0.0,-10.0);
  // The ray direction
  vec3 rayDirection = normalize(vec3(p,1.0));

  vec3 col = vec3(0.0);
  // Use rayDirection to setup a basic background
  if (rayDirection.y > 0.0) {
    // The sky
    col += vec3(0.2,0.5,0.5+0.5*rayDirection.y);
  } else {
    // The ground
    col += -vec3(1.0,0.8,0.6)*rayDirection.y;
  }

  fragColor = vec4(col,1.0);
}
```

## ✂️ Learning to Copy and Paste from IQ's Site 📋

Now, let’s take a big step forward and define a **distance field** for the object we want to ray trace. There are plenty of ways to do this, but why not borrow the brilliant `sdBox` function from [IQ’s amazing collection of distance field functions](https://iquilezles.org/articles/distfunctions/)? After all, sharing is caring!

```glsl
// Copied from: https://iquilezles.org/articles/distfunctions/
float sdBox( vec3 p, vec3 b ) {
  vec3 q = abs(p) - b;
  return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}
```

So, how does this distance field function work? Essentially, for any given point in space, it calculates the distance to the object. If the result is positive, we’re outside the object; if it’s negative, we’re inside; and if it’s zero, we’re right on the surface. Simple enough, right?

With this distance field in hand, we can finally create our ray marcher! The process is pretty straightforward: we start at the ray origin and use our distance field function to check the distance to the object. If we’re “close enough” to hit the object, we stop. If we’ve iterated too many times or traveled beyond a set maximum distance, we stop there as well. Otherwise, we continue moving in the ray direction based on the distance we calculated, repeating this process until we find our target!

Here is the full example:

```glsl
// The maximum distance the ray can travel
const float MaxDistance = 20.0;

// Copied from: https://iquilezles.org/articles/distfunctions/
float sdBox( vec3 p, vec3 b ) {
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

// This function returns the distance field to our object
//  or objects. Can be a very simple like this or more
//  complicated like a fractal
//  On ShaderToy many shaders call this function "map".
float map(vec3 p) {
  return sdBox(p, vec3(3.0));
}

float rayMarch(vec3 rayOrigin, vec3 rayDirection) {
  float distanceTravelled = 0.0;
  for (int i = 0; i < 80; ++i) {
    // Compute our current position
    vec3 pos = rayOrigin+rayDirection*distanceTravelled;
    // Test how far we are from the object.
    float distanceToObject = map(pos);
    // We are done if we are really close to the object (we hit it)
    //  or we travelled too far
    if (distanceToObject < 1E-3 || distanceTravelled >= MaxDistance) {
      return distanceTravelled;
    }

    // Otherwise are adding the distance to the object to the
    //  distance travelled
    distanceTravelled += distanceToObject;
  }

  // If we hit max number of iterations we return max distance to indicate a miss
  return MaxDistance;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
  // Takes fragCoord and transform it into where p (0,0) is in the center of the screen
  // and (0,1) is top of screen and (0,-1) is the bottom.
  vec2 p = (-iResolution.xy+2.0*fragCoord)/iResolution.yy;

  // Setup a ray origin
  vec3 rayOrigin = vec3(0.0,0.0,-10.0);
  // The ray direction
  vec3 rayDirection = normalize(vec3(p,1.0));

  vec3 col = vec3(0.0);
  // Use rayDirection to setup a basic background
  if (rayDirection.y > 0.0) {
    // The sky
    col += vec3(0.2,0.5,0.5+0.5*rayDirection.y);
  } else {
    // The ground
    col += -vec3(1.0,0.8,0.6)*rayDirection.y;
  }

  float rayDistance = rayMarch(rayOrigin, rayDirection);
  // If rayDistance is less than MaxDistance we count that as a hit
  if (rayDistance < MaxDistance) {
    col = vec3(1.0);
  }

  fragColor = vec4(col,1.0);
}
```

## 🧊 Computers Are Made for Rotating Cubes 🧊

Right now, all we see is a white square against a sky-blue background, but trust me—this is supposed to be a 3D box! To make our box stand out as a true cube, we need to rotate it a bit.

We can achieve this using a handy helper function that rotates our 2D coordinates. Here’s the magic:

```glsl
// Rotates the 2D coord p using angle a
void rot(inout vec2 p, float a) {
  float c = cos(a);
  float s = sin(a);
  // A thing I memorized; at some point I understood it
  // but now I've forgotten!
  p = vec2(c * p.x + s * p.y, -s * p.x + c * p.y);
}
```

Now that we've got our rotation function, let's tweak the `map` function to introduce some time-based rotation. This will give our cube a dynamic feel:

```glsl
float map(vec3 p) {
  // Rotate around z-axis
  rot(p.xy, iTime);
  // Rotate around y-axis
  rot(p.xz, iTime * 0.707);
  return sdBox(p, vec3(3.0));
}
```

With these changes, our cube will now spin and reveal its true 3D form. After all, computers are made to rotate cubes!

## ⬅️ Shading Cubes Is Only Normal ➡️

If all goes well, you should be seeing a white cube spinning around. Cool, but let’s kick it up a notch with some shading! To make our cube truly pop, we need to compute the **normal** of its surface. A normal is a vector that’s perpendicular to the surface, and it’s absolutely essential for most shading techniques.

Now, here’s the tricky part: understanding the `normal` function might seem daunting. But don’t worry! You don’t have to master it; just know that almost all shaders use a version like this:

```glsl
vec3 normal(vec3 pos) {
  vec2 eps = vec2(1E-2, 0.0);
  return normalize(vec3(
      map(pos + eps.xyy) - map(pos - eps.xyy),
      map(pos + eps.yxy) - map(pos - eps.yxy),
      map(pos + eps.yyx) - map(pos - eps.yyx)
  ));
}
```

This function calculates the normal by checking how the distance changes around the point `pos`. It’s like poking around the surface to see which way is “up”!

With our normal calculated, we can then compute the diffuse lighting. This is done by taking the **dot product** of the normal and the direction to the light source. Here’s how we do it:

```glsl
// Compute the normal at pos
vec3 n = normal(pos);
// Then compute the diffuse lighting using the dot product of normal and
// light direction
col += max(dot(n, lightDirection), 0.0);
// Ambient light
col += 0.05;
```

With these additions, our cube will not only rotate but also have a lovely shaded effect that makes it look more three-dimensional. Let’s bring our cube to life!

The complete example:

```glsl
// The maximum distance the ray can travel
const float MaxDistance = 20.0;

// The direction to the light
const vec3 LightDirection = normalize(vec3(1.0,1.0,-2.0));

// Rotates the 2D coord p using angle a
void rot(inout vec2 p, float a) {
  float c=cos(a);
  float s=sin(a);
  // A thing I memorized, at some point I understood it
  //  but now I forgot
  p = vec2(c*p.x+s*p.y,-s*p.x+c*p.y);
}

// Copied from: https://iquilezles.org/articles/distfunctions/
float sdBox( vec3 p, vec3 b ) {
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

// This function returns the distance field to our object
//  or objects. Can be a very simple like this or more
//  complicated like a fractal
//  On ShaderToy many shaders call this function "map".
float map(vec3 p) {
  rot(p.xy, iTime);
  rot(p.xz, iTime*0.707);
  return sdBox(p, vec3(3.0));
}

float rayMarch(vec3 rayOrigin, vec3 rayDirection) {
  float distanceTravelled = 0.0;
  for (int i = 0; i < 80; ++i) {
    // Compute our current position
    vec3 pos = rayOrigin+rayDirection*distanceTravelled;
    // Test how far we are from the object.
    float distanceToObject = map(pos);
    // We are done if we are really close to the object (we hit it)
    //  or we travelled too far
    if (distanceToObject < 1E-3 || distanceTravelled >= MaxDistance) {
      return distanceTravelled;
    }

    // Otherwise are adding the distance to the object to the
    //  distance travelled
    distanceTravelled += distanceToObject;
  }

  // If we hit max number of iterations we return max distance to indicate a miss
  return MaxDistance;
}


vec3 normal(vec3 pos) {
  vec2 eps = vec2(1E-2, 0.0);
  return normalize(vec3(
      map(pos+eps.xyy)-map(pos-eps.xyy)
    , map(pos+eps.yxy)-map(pos-eps.yxy)
    , map(pos+eps.yyx)-map(pos-eps.yyx))
    );
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
  // Takes fragCoord and transform it into where p (0,0) is in the center of the screen
  // and (0,1) is top of screen and (0,-1) is the bottom.
  vec2 p = (-iResolution.xy+2.0*fragCoord)/iResolution.yy;

  // Setup a ray origin
  vec3 rayOrigin = vec3(0.0,0.0,-10.0);
  // The ray direction
  vec3 rayDirection = normalize(vec3(p,1.));

  vec3 col = vec3(0.0);
  // Use rayDirection to setup a basic background
  if (rayDirection.y > 0.0) {
    // The sky
    col += vec3(0.2,0.5,0.5+0.5*rayDirection.y);
  } else {
    // The ground
    col += -vec3(1.0,0.8,0.6)*rayDirection.y;
  }


  float rayDistance = rayMarch(rayOrigin, rayDirection);
  // If rayDistance is less than MaxDistance we count that as a hit
  if (rayDistance < MaxDistance) {
    // Compute the pos of the point on the surface
    vec3 pos = rayOrigin+rayDistance*rayDirection;
    col = vec3(0.0);
    // Compute the normal at pos
    vec3 n = normal(pos);
    // Then compute the diffuse lighting using the dot product of normal and
    // light direction
    col += max(dot(n, LightDirection),0.0);
    // Ambient light
    col += 0.05;
  }

  fragColor = vec4(col,1.0);
}
```


Here’s the next section, keeping that engaging style:

## 🧊🌐 Making Complex Shapes from Simple Ones 🌐🧊

Awesome! We’ve got a basic rotating cube with some nifty shading. But wait—what’s even cooler is that we can create complex shapes by combining simple distance fields!

Using **union** and **intersection** operations with `min` and `max`, we can mix and match shapes. For example, let’s combine our box with a box frame using the `sdBoxFrame` function:

```glsl
// Copied from: https://iquilezles.org/articles/distfunctions/
float sdBoxFrame(vec3 p, vec3 b, float e) {
    p = abs(p) - b;
    vec3 q = abs(p + e) - e;
    return min(min(
        length(max(vec3(p.x, q.y, q.z), 0.0)) + min(max(p.x, max(q.y, q.z)), 0.0),
        length(max(vec3(q.x, p.y, q.z), 0.0)) + min(max(q.x, max(p.y, q.z)), 0.0)),
        length(max(vec3(q.x, q.y, p.z), 0.0)) + min(max(q.x, max(q.y, p.z)), 0.0));
}

float map(vec3 p) {
    rot(p.xy, iTime);
    rot(p.xz, iTime * 0.707);

    // The box distance field
    float dbox = sdBox(p, vec3(3.0));
    // The box frame distance field
    float dboxFrame = sdBoxFrame(p, vec3(3.5), 0.2);
    // Combine the two using min
    float d = min(dbox, dboxFrame);

    return d;
}
```

With this setup, we create a box with a surrounding frame, giving it a more layered look! But why stop there? Let’s take it a step further and subtract a sphere from our shape using the `max` function.

First, we’ll define our sphere with a simple function:

```glsl
float sdSphere(vec3 p, float r) {
    return length(p) - r;
}
```

Now we’ll modify our `map` function to include the sphere:

```glsl
// This function returns the distance field to our object
// or objects. It can be very simple like this or more
// complicated, like a fractal. On ShaderToy, many shaders
// call this function "map".
float map(vec3 p) {
    rot(p.xy, iTime);
    rot(p.xz, iTime * 0.707);

    // The box distance field
    float dbox = sdBox(p, vec3(3.0));
    // The box frame distance field
    float dboxFrame = sdBoxFrame(p, vec3(3.5), 0.2);
    // The inner sphere
    float dsphere = sdSphere(p, 3.4);

    // Combine the two boxes using min
    float d = min(dbox, dboxFrame);

    // Subtract the sphere from d using max
    d = max(d, -dsphere);

    return d;
}
```

With these changes, we’ve taken our simple shapes and created something much more complex and visually interesting! This approach opens up a world of possibilities—so let your creativity run wild and experiment with different combinations!

## 🌘 Time to Throw Down Some Shade! 🌘

Alright, it’s time to add some shadows to our scene! To determine if a point on the surface is in shade, we can reuse our `rayMarch` function to step toward the light source. If the ray hits the surface before it reaches the light, we know that point is in shade. If it makes it to the light without hitting anything, then it’s basking in the glow!

To avoid getting stuck, we start the ray trace a tiny bit away from the surface in the direction of the normal. Here’s how we do it:

```glsl
// To detect shadows, we ray trace toward the light
// Since we’re very close to the surface, the ray trace will
// terminate almost immediately. So, we start a bit away
// from the surface by adding 1E-2 in the normal direction.
float rayLightDistance = rayMarch(pos + 1E-2 * n, LightDirection);

// If rayLightDistance indicates a miss, it means we didn't hit the surface
// while traveling toward the light
if (rayLightDistance >= MaxDistance) {
    // Then compute the diffuse lighting using the dot product of normal and
    // light direction
    col += max(dot(n, LightDirection), 0.0);
}
```

With this code, our scene will now have some lovely shadows, adding depth and realism to our rotating cube. Shadows can make a huge difference in how we perceive shapes, and now our cube is looking even more dynamic!

```glsl
// The maximum distance the ray can travel
const float MaxDistance = 20.0;

// The direction to the light
const vec3 LightDirection = normalize(vec3(1.0,1.0,-2.0));

// Rotates the 2D coord p using angle a
void rot(inout vec2 p, float a) {
  float c=cos(a);
  float s=sin(a);
  // A thing I memorized, at some point I understood it
  //  but now I forgot
  p = vec2(c*p.x+s*p.y,-s*p.x+c*p.y);
}


// Copied from: https://iquilezles.org/articles/distfunctions/
float sdBox( vec3 p, vec3 b ) {
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

// Copied from: https://iquilezles.org/articles/distfunctions/
float sdBoxFrame( vec3 p, vec3 b, float e )
{
       p = abs(p  )-b;
  vec3 q = abs(p+e)-e;
  return min(min(
      length(max(vec3(p.x,q.y,q.z),0.0))+min(max(p.x,max(q.y,q.z)),0.0),
      length(max(vec3(q.x,p.y,q.z),0.0))+min(max(q.x,max(p.y,q.z)),0.0)),
      length(max(vec3(q.x,q.y,p.z),0.0))+min(max(q.x,max(q.y,p.z)),0.0));
}


float sdSphere(vec3 p, float r) {
  return length(p) - r;
}

// This function returns the distance field to our object
//  or objects. Can be a very simple like this or more
//  complicated like a fractal
//  On ShaderToy many shaders call this function "map".
float map(vec3 p) {
  rot(p.xy, iTime);
  rot(p.xz, iTime*0.707);

  // The box distance field
  float dbox = sdBox(p, vec3(3.0));
  // The box frame distance field
  float dboxFrame = sdBoxFrame(p, vec3(3.5),0.2);
  // The inner sphere
  float dsphere = sdSphere(p, 3.4);

  // Combine the two boxes using min
  float d = min(dbox,dboxFrame);

  // Subtract the sphere from d using max
  d = max(d,-dsphere);

  return d;
}

float rayMarch(vec3 rayOrigin, vec3 rayDirection) {
  float distanceTravelled = 0.0;
  for (int i = 0; i < 80; ++i) {
    // Compute our current position
    vec3 pos = rayOrigin+rayDirection*distanceTravelled;
    // Test how far we are from the object.
    float distanceToObject = map(pos);
    // We are done if we are really close to the object (we hit it)
    //  or we travelled too far
    if (distanceToObject < 1E-3 || distanceTravelled >= MaxDistance) {
      return distanceTravelled;
    }

    // Otherwise are adding the distance to the object to the
    //  distance travelled
    distanceTravelled += distanceToObject;
  }

  // If we hit max number of iterations we return max distance to indicate a miss
  return MaxDistance;
}


vec3 normal(vec3 pos) {
  vec2 eps = vec2(1E-2, 0.0);
  return normalize(vec3(
      map(pos+eps.xyy)-map(pos-eps.xyy)
    , map(pos+eps.yxy)-map(pos-eps.yxy)
    , map(pos+eps.yyx)-map(pos-eps.yyx))
    );
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
  // Takes fragCoord and transform it into where p (0,0) is in the center of the screen
  // and (0,1) is top of screen and (0,-1) is the bottom.
  vec2 p = (-iResolution.xy+2.0*fragCoord)/iResolution.yy;

  // Setup a ray origin
  vec3 rayOrigin = vec3(0.0,0.0,-10.0);
  // The ray direction
  vec3 rayDirection = normalize(vec3(p,1.));

  vec3 col = vec3(0.0);
  // Use rayDirection to setup a basic background
  if (rayDirection.y > 0.0) {
    // The sky
    col += vec3(0.2,0.5,0.5+0.5*rayDirection.y);
  } else {
    // The ground
    col += -vec3(1.0,0.8,0.6)*rayDirection.y;
  }


  float rayDistance = rayMarch(rayOrigin, rayDirection);
  // If rayDistance is less than MaxDistance we count that as a hit
  if (rayDistance < MaxDistance) {
    // Compute the pos of the point on the surface
    vec3 pos = rayOrigin+rayDistance*rayDirection;
    col = vec3(0.0);

    // Ambient light
    col += 0.05;

    // Compute the normal at pos
    vec3 n = normal(pos);

    // In order to detect we ray trace toward the light
    //  As we are very close to the surface it means the ray trace will
    //  terminate at once. Therefore we start a bit away from the surface by
    //  adding 1E-2 in the normal direction
    float rayLightDistance = rayMarch(pos+1E-2*n, LightDirection);

    // If the rayLightDistance indicate a miss it means we missed the surface
    //  while travelling towards the light
    if (rayLightDistance >= MaxDistance) {
      // Then compute the diffuse lighting using the dot product of normal and
      // light direction
      col += max(dot(n, LightDirection),0.0);
    }
  }

  fragColor = vec4(col,1.0);
}
```

## 🎁 Wrapping Up! 🎁

Congratulations on making it this far! If you’re keen on understanding shaders on [ShaderToy](https://www.shadertoy.com/), you’ll find that many of them use some version of ray marching. So, grasping the basics of how a ray marcher works is super useful!

The recurring themes in all ray marchers include the distance field function (often called `map`), the ray tracer function, and the compute `normal` function. With just these building blocks, you can create some truly amazing shaders!

Remember, there are countless ways to vary these basic concepts, and experimenting is part of the fun! Don’t hesitate to tinker around with different shapes and effects. Understanding ray marching will not only help you decipher most shaders on [ShaderToy](https://www.shadertoy.com/), but it’ll also inspire your own creativity.

So grab your favorite snacks, fire up ShaderToy, and let your imagination run wild! Happy coding, and may your shaders shine bright this holiday season! 🎄✨

✨🎄🎁 Merry Christmas, and happy coding! 🎁🎄✨

🎅 - mrange

## ❄️Licensing Information❄️

All code content I created for this blog post is licensed under [CC0](https://creativecommons.org/public-domain/cc0/) (effectively public domain). Any code snippets from other developers retain their original licenses.

The text content of this blog is licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) (the same license as Stack Overflow).