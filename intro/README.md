# Welcome to Shader Advent Calendar 2024

🎅Ho, ho, ho! Merry Christmas!🎅

I have been following and even contributing to the [F# Advent Calendar](https://sergeytihon.com/fsadvent/) and I got thinking: We should try to do something like that for all us Shader tinkerers as well.

I realize creating something out of nothing is challenging and realistically I felt we would probably not be able to fill all days in December and I am therefore deeply grateful to all the contributors that helped me out.

In order to fill up the schedule you might see more blogs posts from me but I would be happy to trade my slot for anyone else.

Anyway, I thought I would start of the Shader Advent Calendar share tricks and tips I picked up from my tinkering with shaders.

## Tip #1: Don't be afraid to share what you done

Something I realized while sharing shaders on [ShaderToy](www.shadertoy.com) is that I can't predict what people will like. I can work for a really long time on something, share it and then just *crickets* in response. Other times I work for 2 hours on a quick hack and it becomes [Shader of the week](https://www.shadertoy.com/view/MfjyWK) and lots of appreciate comments.


In addition, I never had any bad interaction on ShaderToy over code quality or visual quality meaning I don't have any fear sharing something I realize might not be my best work. And even if the shader makes no splash whatsoever I have gotten comments from that it is their favorite. I don't know why they think so but I am glad they.

So, my tip is that publish what you do on ShaderToy even if you are a beginner. There is room for all kind of content from shaders that are several thousand lines long to simple 2D shaders.

IQ, Shane, Kali and other produce amazing content but for a beginner (and experienced) shader tinkers they might be so intricate that they teach you nothing because you can't see the forest for all the trees. A simple shader can be what a beginner needs to see to finally understand how Raymarching works.

## Tip #2: Fragment shaders are not hard

I often hear that "Shaders are dark magic and impossible to understand". However, I would like to argue the Fragment shaders actually quite simple to understand.

First, it's written in GLSL which is basically C with all the hard parts removed (that is resource management and recursion). Many developers out there are used to C style languages so I think most will feel very comfortable with the language.

Second, the fragment shader itself is a function that given a coordinate produces a color. The signature of a [new ShaderToy snippet](https://www.shadertoy.com/new) illustrates that:

```glsl
void mainImage( out vec4 fragColor, in vec2 fragCoord );
```

Given a coordinate produce a color. That's it.

Now, many shaders can be tricky to understand but I argue not because the fundamentals are tricky but because the patterns shader tinkerers are different from "normal" patterns.

Which leads to the next tip

## Tip #3: Distance fields are amazing

A central pattern used by shader tinkerers are distance fields. The idea is simple as well. Distance field functions are used to model 2D or 3D objects like spheres and boxes and we can then combine them to together to create more complex shapes.

For example a sphere distance field function could look like this:

```glsl
float sphere(vec3 pos, float radius) {
  return length(pos) - radius;
}
```

Given a point in space the `sphere` function answers the question how from the sphere the point is. If the result is positive we are outside the sphere, if it's negative we are inside the sphere and if it's 0 we are on the surface.

A very common technique among shader tinkerers raytracing to produce cool looking 3D worlds. We create a distance field function (often called `map`) for our world. The raytracer then starts in a position and asks the distance field function how far away it is. We then step that distance in the ray direction. We stop if we travelled or hit the surface otherwise we do another iteration.

As an example have a look at a classic (IMHO) [Menger Sponge Variation by Shane](https://www.shadertoy.com/view/ldyGWm). The function `map` here is the distance field to the world (a menger sponge in this case) and the function `trace` is the raytracer. The rest is lighting magic by Shane but the basic raytracer is straight forward. I don't have any data but I would say 90% of the shaders on [ShaderToy](https://www.shadertoy.com/) works like this.

## Tip #4: No really, distance fields are amazing

A powerful way to create new shapes is combining simple ones using a union operation or an intersect operation, a bit like boolean algebra. When you work with triangles implementing this is hard work but when you work distance fields it's trivial.

The union operation is the `min` function, the intersect operation is `max` function. When I first read that I couldn't believe it was that simple and was a big reason I got interested in shaders.

So using that info we can create a "hole-y" box by combining two distance fields.

```glsl

// Returns the distance to a box
//  From the amazing site: https://iquilezles.org/articles/distfunctions/
float box(vec3 p, vec3 b) {
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

// Returns the distance to a sphere
float sphere(vec3 pos, float radius) {
  return length(pos) - radius;
}

float map(vec3 pos) {
  // dbox is distance to a cube with side 2
  float dbox = box(pos,vec3(1.));
  // dcircle is distance to spehre with radius 1.1
  float dcircle = sphere(pos, 1.1);

  // Return the intersection between of the box and the circle turned inside out
  //  This creates a hole-y box
  return max(dbox,-dcircle);
}
```

Really cool!

## Tip #5: Palette generating function used by all shader size coders

To make cool looking shaders we need cool looking colors and one of the simplest and shortest way of doing so is to use the favorite palette generating function of all shader sizer coders

```glsl
vec3 palette(float a) {
  return 0.5+0.5*sin(vec3(0,1,2)+a);
}
```

If you vary `a` you get a cool looking color palette with rich blues and intense whites. I was first introduced to this when looking at a [tweet by XorDev](https://twitter.com/XorDev/status/1601060422819680256) (also available on [ShaderToy](https://www.shadertoy.com/view/msjXRK)). Because it is short and  looks great it's [used everywhere](https://www.shadertoy.com/view/mtyGWy).

[I have used it a ton](https://www.shadertoy.com/view/cdKXDV) since I was introduced to it.

## Tip #6: The simplest post-processing

One way to kick you shader up a notch is by applying post-processing to it. This can be [a complicated multi-stage process](https://www.shadertoy.com/view/MflfR8) but what I find myself returning to again and again is something really simple.

As can be [seen in my shaders I like saturated colors](https://www.shadertoy.com/view/XfyXRV). I don't like the un-intentional global glow that is very easy to end up with. I used to struggle alot with this until I learnt a really simple trick.

From the shader above the trick happens at line 347:
```glsl
  // Does all of the work but there's an annoying global glow that desaturates
  //  the colors. I am annoyed!
  col = render3(rayOrigin, rd);

  // Line 347: This simple substraction removes the global global and increases saturation
  col -= 2E-2*vec3(2.,3.,1.)*(length(p)+0.25);

  // Maps colors from [0,inf[ to [0,1]
  col = aces_approx(col);
  // Approximate linear RGB => sRGB conversion
  col = sqrt(col);
```

What I usually do is that I do as good as I can on creating the effect but then as a final step I will apply post-processing.

I start with something as simple as this `col -= 0.01` and see what I think about it and then I tinker from there.

