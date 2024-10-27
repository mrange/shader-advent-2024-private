# 🎄⭐🎉Introduction to 2D shaders🎉⭐🎄

🎅Ho, ho, ho! Merry Christmas!🎅

## Introduction

While most examples on [ShaderToy](https://www.shadertoy.com/) are 3D raymarchers you can do cool shaders with 2D. In addition, doing 2D it's easy to visualize distance fields which is a very common pattern in shader coding.

Distance fields are often used in shader raytracers but it's tricky to visualize the distance field in 3D. We can learn alot about how distance fields works by studying them in 2D and then apply our kwowledge to 3D.

## Drawing a 2D circle

As mentioned in the introductory post for [Shader Advent 2024](../intro/README.md) a fragment shader is just a function that takes a coordinate and returns a color and a distance field function when given a point returns the distance from the point to the surface.

A simple circle distance field function can look like this

```glsl
float circle(vec2 pos, float radius) {
  return length(pos) - radius;
}
```

Each point further from origo than `radius` will result in a positive value, each point inside `radius` will result in a negative value and all points at `radius` will result in 0.

In order to draw a circle we will pass the coordinate to the circle distance field function and for each point that is inside, that is results in a negative value we wil set the color to white, otherwise it's black.

Now [create a shader in ShaderToy](https://www.shadertoy.com/new) and replace the code with:

```glsl
// A distance field function for a circle
float circle(vec2 pos, float radius) {
  return length(pos) - radius;
}


void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
  // This converts the input fragCoord, that is in texture coordinates, into p
  //  where the center is at (0,0) and top and bot is at -1 and 1.
  vec2 p = (-iResolution.xy+2.0*fragCoord)/iResolution.yy;

  // Compute the distance from to a centered circle with radius 0.5
  float dcircle = circle(p, 0.5);

  // By default the color is black
  //  Colors in shaders are RGB where each component goes 0 to 1
  vec3 col = vec3(0.0,0.0,0.0);

  if (dcircle < 0.0) {
    // If we are inside the circle set the color to white
    col = vec3(1.0,1.0,1.0);
  }

  // Set the output color with alpha = 1
  fragColor = vec4(col,1.0);
}
```

Hit Alt-Enter or click the ▶️Play button at the bottom of the source editor to compile the shader and with some luck it looks a bit like below.

![A simple circle](assets/circle-2d.jpg)

🎅 - mrange

## ❄️Licensing Information❄️

All code content I created for this blog post is licensed under [CC0](https://creativecommons.org/public-domain/cc0/) (effectively public domain). Any code snippets from other developers retain their original licenses.

The text content of this blog is licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) (the same license as Stack Overflow).