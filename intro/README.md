# Welcome to Shader Advent Calendar 2024

🎅Ho, ho, ho! Merry Christmas!🎅

After following and even pitching in on the [F# Advent Calendar](https://sergeytihon.com/fsadvent/), I thought: why not bring a little shader magic to December? So, welcome to the Shader Advent Calendar, where each day (hopefully) brings a fresh shader gem to tinker with or be inspired by.

I realize that creating something out of nothing is challenging, and realistically, we may not be able to fill every day in December. I'm deeply grateful to all the contributors who have helped make this possible.

To keep the schedule full, you might see a few extra blog posts from me, but I'd be happy to give up my slot to anyone interested in contributing.

Anyway, I thought I’d kick off the Shader Advent Calendar by sharing a few tricks and tips I’ve picked up while tinkering with shaders.

Here's a refined version that smooths out grammar and adds a welcoming tone for beginners:

---

## Tip #1: Don't Be Afraid to Share What You've Created

One thing I’ve realized while sharing shaders on [ShaderToy](https://www.shadertoy.com) is that predicting what resonates with people is almost impossible. I can spend hours on something complex, post it, and hear only *crickets*. But then, I might spend just a couple of hours on a quick hack, and suddenly it’s [Shader of the Week](https://www.shadertoy.com/view/MfjyWK) with loads of positive feedback!

Also, I’ve never had a negative experience on ShaderToy about code or visual quality—so I don’t feel any fear sharing work that may not be my absolute best. Sometimes, the simplest shaders end up being someone’s favorite, even if they don’t make a big splash.

So, my tip: share what you create, even if you’re just starting out! There’s room for all kinds of shaders on ShaderToy, from massive, intricate projects to simple 2D effects.

Creators like IQ, Shane, and Kali put out stunning work, but for beginners and even seasoned tinkerers, these complex shaders can be hard to digest. Sometimes, it’s the smaller, simpler shaders that help beginners finally grasp concepts like Raymarching. So don’t hold back—your shader might be exactly what someone needs to see!

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

## ❄️Licensing Information❄️

All code content I created for this blog post is licensed under [CC0](https://creativecommons.org/public-domain/cc0/) (effectively public domain). Any code snippets from other developers retain their original licenses.

The text content of this blog is licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) (the same license as Stack Overflow).