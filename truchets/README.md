# 🎄⭐🎉 Introduction to Truchet Shaders 🎉⭐🎄

🎅 Ho, ho, ho! Merry Shader-mas! 🎅

## 🎄 Truchet Patterns are Cool 🎄

Truchet patterns or [Truchet tiles](https://en.wikipedia.org/wiki/Truchet_tiles) can produce cool and surprising patterns.

The basic idea is simple. We create a tile where regardless of rotation fit together with copies of the tile. For example the classic Smith tile:


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
```
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


