# 🎄⭐🎉 Introduction to Truchet Shaders 🎉⭐🎄

🎅 Ho, ho, ho! Merry X-mas! 🎅

## 🎄 Truchet Patterns are Cool 🎄

Truchet patterns or [Truchet tiles](https://en.wikipedia.org/wiki/Truchet_tiles) can produce cool and surprising patterns.

The basic idea is simple. We create a tile where regardless of rotation fit together with copies of the tile. For example the classic Smith tile:

<p align="center">
  <img src="assets/smith.jpg" alt="The classic smith tile" style="width: 50%;" />
</p>

Then we fill a plane with these tiles randomly rotated:

The patterns that arise from this simple approach are to me surprising and interesting.

Another classic is the C64 program that fills screen with either `\` or `/` but picked randomly creating a maze like truchet pattern.

```basic
10 PRINT CHR$(205.5+RND(1));
20 GOTO 10
```

You can try this is in an online C64 emulator.

## This is cool but how can we make a shader of it.

The first step is to [create a new ShaderToy shader](https://www.shadertoy.com/new).

Then we are going to create a Truchet tile, the Smith tile above is simple to do so let's make one of those.

Assuming the tile is a square with side length = 1 then we can create the tile by drawing two circles centrered in opposing corners with radius 0.5. To make the tile visible add a square with side 1.

We start by defining the helper functions the box and circle:
```glsl
// Found here: https://iquilezles.org/articles/distfunctions2d/
float sdBox( in vec2 p, in vec2 b ) {
    vec2 d = abs(p)-b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

// Returns distance field of circle outline with radius r and width w
//
float sdCircle(vec2 p, float r, float w) {
  // Distance for circle of radius r
  float d = length(p) - r;
  // Create outline
  d = abs(d);
  // Give outline the width w
  d -= w;
  return d;
}
```

Using these functions we can now define the smith tile:

```glsl
float smithTile(vec2 p) {
  float dcircle0 = sdCircle(p-0.5, 0.5, 0.05);
  float dcircle1 = sdCircle(p+0.5, 0.5, 0.05);
  float dbox     = abs(sdBox(p,vec2(0.5)))-0.01;

  // Combines the distance fields using the union operation (min)
  float d = min(dcircle0, dcircle1);
  d = min(d, dbox);
  return d;
}

```

Here is the complete example:
```glsl
// Found here: https://iquilezles.org/articles/distfunctions2d/
float sdBox( in vec2 p, in vec2 b ) {
    vec2 d = abs(p)-b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

// Returns distance field of circle outline with radius r and width w
//
float sdCircle(vec2 p, float r, float w) {
  // Distance for circle of radius r
  float d = length(p) - r;
  // Create outline
  d = abs(d);
  // Give outline the width w
  d -= w;
  return d;
}

float smithTile(vec2 p) {
  float dcircle0 = sdCircle(p-0.5, 0.5, 0.05);
  float dcircle1 = sdCircle(p+0.5, 0.5, 0.05);
  float dbox     = abs(sdBox(p,vec2(0.5)))-0.01;

  float d = min(dcircle0, dcircle1);
  d = min(d, dbox);
  return d;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
  vec2 p = (-iResolution.xy+2.0*fragCoord)/iResolution.yy;

  float aa = sqrt(2.)/iResolution.y;

  vec3 col = vec3(0.0);

  float dtile = smithTile(p);

  col = mix(col, vec3(0.8), smoothstep(aa, -aa, dtile));

  fragColor = vec4(col,1.0);
}
```

One thing I can mention here is that I use smoothstep of to mix the background color with the foreground color. The purpose is to reduce pixelated borders of the distance fields. `float aa = sqrt(2.)/iResolution.y;` is an estimate of how big a pixel is in the units of `p`. `smoothstep(aa, -aa, dtile)` then smoothly goes from 1 to 0 when `dtile` transitions from a negative value (inside) to positive value (outside). This is a pattern I reuse all the time.

## Let's repeat the the tile

A really cool thing with shaders is that sometimes one can repeat a object infinitely without almost any extra cost. This is sometimes call domain repetition.

One simple way of repeating the unit square is this simple code:

```glsl
vec2 tp = p;
// A neat trick to repeat the unit square
//  The unit square (that is the truchet tile in our example)
//  is repeated in x and y direction infinitely
vec2 np = round(tp);
vec2 cp = tp - np;

float dtile = smithTile(cp);
```

If you apply this pattenrn a sort of a wavy-pattern appears which is nice but we can make it more interesting by introducing pseudo-randomness. To do so we add the function `hash`

```glsl
// Produces a pseudo-random from a 2D point
float hash(vec2 co) {
  return fract(sin(dot(co.xy ,vec2(12.9898,58.233))) * 13758.5453);
}
```

We then use `hash` to flip half of the tiles.

```glsl
vec2 cp = tp - np;

if (hash(np) > 0.5) {
  //  for 50% of the cells we flip the shape
  cp.x *= -1.0;
}

float dtile = smithTile(cp);
```

This should kick it up a notch but we see only a small portion of the plane.

Let's add the ability to zoom in and out to show more or less of the plane:

```glsl
  // Zoom level 50%
  const float tz = 0.5;

  // In order to zoom divide by zoom level
  vec2 tp = p/tz;

  // ... the rest of the code from the sample

  // Multiply the distance field value by tz because we divided
  //  the pos by tz earlier.
  //  Otherwise the anti-aliasing don't work properly
  float dtile = tz*smithTile(cp);
```

You can now change `tz` to zoom in and out.

The entire example here:

```glsl
// Found here: https://iquilezles.org/articles/distfunctions2d/
float sdBox( in vec2 p, in vec2 b ) {
    vec2 d = abs(p)-b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

// Returns distance field of circle outline with radius r and width w
//
float sdCircle(vec2 p, float r, float w) {
  // Distance for circle of radius r
  float d = length(p) - r;
  // Create outline
  d = abs(d);
  // Give outline the width w
  d -= w;
  return d;
}

float smithTile(vec2 p) {
  float dcircle0 = sdCircle(p-0.5, 0.5, 0.05);
  float dcircle1 = sdCircle(p+0.5, 0.5, 0.05);
  float dbox     = abs(sdBox(p,vec2(0.5)))-0.01;

  float d = min(dcircle0, dcircle1);
  d = min(d, dbox);
  return d;
}


// Produces a pseudo-random from a 2D point
float hash(vec2 co) {
  return fract(sin(dot(co.xy ,vec2(12.9898,58.233))) * 13758.5453);
}


void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
  vec2 p = (-iResolution.xy+2.0*fragCoord)/iResolution.yy;

  float aa = sqrt(2.)/iResolution.y;

  vec3 col = vec3(0.0);

  // Zoom level 50%
  const float tz = 0.5;

  // In order to zoom divide by zoom level
  vec2 tp = p/tz;


  // A neat trick to repeat the unit square
  //  The unit square (that is the truchet tile in our example)
  //  is repeated in x and y direction infinitely
  vec2 np = round(tp);
  vec2 cp = tp - np;

  // np is (0,0) for the unit square in the middle, (-1,0) when stepping to left
  //  (1,0) when stepping to right.
  //  Each square has it's own unique "id"
  //  We pass this to hash function which given a 2D coordinate produces a
  //  pseudo-random value
  if (hash(np) > 0.5) {
    //  for 50% of the cells we flip the shape
    cp.x *= -1.0;
  }

  // Multiply the distance field value by tz because we divided
  //  the pos by tz earlier.
  //  Otherwise the anti-aliasing don't work properly
  float dtile = tz*smithTile(cp);


  col = mix(col, vec3(0.8), smoothstep(aa, -aa, dtile));

  fragColor = vec4(col,1.0);
}
```

Thanks to the box we added earlier you can quite easily spot the truchet tiles but if you drop the box shape the pattern is more tricky to decipher:

```
float smithTile(vec2 p) {
  float dcircle0 = sdCircle(p-0.5, 0.5, 0.05);
  float dcircle1 = sdCircle(p+0.5, 0.5, 0.05);

  float d = min(dcircle0, dcircle1);
  return d;
}
```

## That it's for today!

Truchet patterns are a cool way to create interesting shapes and there are many kinds of truchet patterns possible even multi-level truchet patterns.

[Shane](https://www.shadertoy.com/user/Shane) has published many cool truchet shaders like [a quadtree truchet](https://www.shadertoy.com/view/4t3BW4) or my favorite [the Hyperbolic Poincare Weave](https://www.shadertoy.com/view/tljyRR). [byt3_m3chanic](https://www.shadertoy.com/user/byt3_m3chanic) has made many cool truchet shaders, often in 3D, like [this one](https://www.shadertoy.com/view/lcySzz).

Obviously these examples are a "bit" more complex than my example here but it builds and expands on the same ideas.


Wishing you all...

✨🎄🎁 A merry and bright holiday season and christmas presents containing new GPUs! 🎁🎄✨

🎅 - mrange


