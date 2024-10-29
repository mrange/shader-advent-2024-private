# 🎄⭐🎉 Introduction to Ray Marching 🎉⭐🎄

🎅 Ho, ho, ho! Merry Shader-mas! 🎅

## 🎄 Wait... *Another* Ray Marching Blog Post? 🎄

Yes, yes, I know—there are plenty of blog posts on ray marching out there. It's a bit like the programming equivalent of a Christmas fruitcake: everyone who gets a taste of it has to make their own version!

So why add one more? Well, if you’ve ever browsed [ShaderToy](https://www.shadertoy.com/) and found yourself dazzled (or mystified) by those mind-bending shaders, understanding ray marching is like having the recipe. Once you get it, everything starts to click because so many ShaderToy examples are built around it.

### So, what *is* Ray Marching?

Put simply, a ray marcher is a type of ray tracer, but it uses distance fields to define and render 3D objects. This gives it a unique, powerful way to create depth and shapes in 3D spaces. Imagine tracing a line through a scene, but instead of traditional rendering, we’re checking distances to objects, step by step, until we hit something (or not).

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

## ✂️ Learning to copy and paste from IQ's site 📋

The next step for us is to define a distance field for the object we wish to ray trace. There are many ways but let's copy the `sdBox` from the [amazing collection of distance field functions](https://iquilezles.org/articles/distfunctions/) by IQ.

```glsl
// Copied from: https://iquilezles.org/articles/distfunctions/
float sdBox( vec3 p, vec3 b ) {
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}
```

A distance field function works like this, for a given point in space it computes the distance to the object. If the result is positive we are outside the object, if it's negative we are inside the object and if it's 0 we are on the surface of the object.

Using this distance field we can create ray marcher. The ray marcher starts in the ray origin and then checks the distance to the object using the distance field function. If we hit the object (that is the distance is "close enough") we stop, if we iterated too many times we stop or if we travelled a maximum distance we stop. Otherwise we travel the distance in the ray direction and repeat the process.

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

## 🧊 Computers are made for rotating cubes 🧊

What we see is a white square with a sky background but trust me this is 3D box.

We don't see that the box is a square so in order to demonstrate we rotate the box.

We do this with a commonly used helper function:

```glsl
// Rotates the 2D coord p using angle a
void rot(inout vec2 p, float a) {
  float c=cos(a);
  float s=sin(a);
  // A thing I memorized, at some point I understood it
  //  but now I forgot
  p = vec2(c*p.x+s*p.y,-s*p.x+c*p.y);
}
```

Then we change the `map` function to add a bit of time-based rotation:

```glsl
float map(vec3 p) {
  // Rotate around z-axis
  rot(p.xy, iTime);
  // Rotate around y-axis
  rot(p.xz, iTime*0.707);
  return sdBox(p, vec3(3.0));
}
```

## ⬅️Shading cubes is only normal➡️

With some luck you should see a white cube rotating. While cool we like to add some shading to the cube. In order to do so we need to compute the normal of the surface. A normal is perpendicular to the surface and it absolutely essential in almost all kinds of shadings.

The bad news is that understanding the `normal` function might be a bit tricky, the good news is that you don't have to and that almost all shaders use some version of this:

```glsl
vec3 normal(vec3 pos) {
  vec2 eps = vec2(1E-2, 0.0);
  return normalize(vec3(
      map(pos+eps.xyy)-map(pos-eps.xyy)
    , map(pos+eps.yxy)-map(pos-eps.yxy)
    , map(pos+eps.yyx)-map(pos-eps.yyx))
    );
}
```

Using the normal we can then compute the diffuse light by taking the `dot` product of the normal and the direction to the light like so:

```glsl
  // Compute the normal at pos
  vec3 n = normal(pos);
  // Then compute the diffuse lighting using the dot product of normal and
  // light direction
  col += max(dot(n, lightDirection),0.0);
  // Ambient light
  col += 0.05;
```

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

## 🧊🌐 Making complex shapes from simple ones 🌐🧊

Cool! We have a basic rotating cube with basic shading.


What's even cooler is that any distance field works and we can combine them simple using union `min` and intersection `max` operations.

We can for exampel combine our box with box frame function `sdBoxFrame`:

```glsl

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

float map(vec3 p) {
  rot(p.xy, iTime);
  rot(p.xz, iTime*0.707);

  // The box distance field
  float dbox = sdBox(p, vec3(3.0));
  // The box frame distance field
  float dboxFrame = sdBoxFrame(p, vec3(3.5),0.2);
  // Combine the two using min
  float d = min(dbox,dboxFrame);

  return d;
}
```

This create a box with a surrounding box frame. We can take it a bit further and substract a sphere from it using the `max` function.

```glsl
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
```

## 🌘 Time to throw down some shade! 🌘

Finally let's add some shadows. In order to know if a point on the surface is in shade we resue the `rayMarch` function to step towards the light. If the result indicate we hit the surface it means we are in shade. Otherwise the surface point is in the light. In order to not get stuck because we start the ray trace from a point on the surface we start a bit away from the surface in the normal direction.

```glsl
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
```

The complete example looks like this:

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

## 🎁 Wrapping up! 🎁

If you are interested in understanding shaders on [ShaderToy](https://www.shadertoy.com/) most of them uses some version of ray marching so understanding the basic of how a ray marcher works is useful.

Reoccuring patterns in all ray marchers is the distance field function (often called `map`), the ray tracer function and the compute `normal` function and you can do really cool shaders with just these basic steps.

There are many, many ways to variate on these basic themes but I think having a bit of understanding of how a ray marcher works will help you decipher most shaders on [ShaderToy](https://www.shadertoy.com/).

✨🎄🎁 Merry Christmas, and happy coding! 🎁🎄✨

🎅 - mrange

## ❄️Licensing Information❄️

All code content I created for this blog post is licensed under [CC0](https://creativecommons.org/public-domain/cc0/) (effectively public domain). Any code snippets from other developers retain their original licenses.

The text content of this blog is licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) (the same license as Stack Overflow).