# 🎄🌟🎄Vertex Shaders in KodeLife by 🎅 mrange🎄🌟🎄

🎅Ho, ho, ho! Merry Christmas!🎅

Many of us know [ShaderToy](https://www.shadertoy.com/) as a playground for experimenting with and sharing Fragment Shaders (also called Pixel Shaders in DirectX). But there's another type of shader that often flies under the radar: Vertex Shaders. Though less popular among shader enthusiasts, vertex shaders unlock a new world of possibilities—especially when combined with fragment shaders.

Take, for example, the 4KiB intro *Delusions of Mediocrity*. Here, most of the magic happens in the vertex shader, which generates vertices using a supershape formula. The fragment shaders handle the colors and post-processing effects.
![Delusions of Mediocrity](assets/intro.jpg)

## Getting Started with Vertex Shaders

Vertex shaders can be a bit trickier to work with than fragment shaders, but I’ll walk through some basics to help you get started.

For these examples, I’m using [KodeLife by Hexler](https://hexler.net/kodelife). KodeLife is a versatile tool that’s free to use, though it does include a gentle reminder to support the developers.

When you launch KodeLife, it sets you up with an initial template. Usually, I dive straight into the fragment shader, but this time, let’s explore the vertex shader instead.

I’ve added code comments to clarify the key differences from fragment shaders.
```glsl
#version 150

// Uniforms are "global" variables set by the host program
//  Works the same as for fragment shaders
uniform float time;
uniform vec2 resolution;
uniform vec2 mouse;
uniform vec3 spectrum;

// One important difference from fragment shaders is that the model view
//  projection (mvp) transform is injected.
//  This allows the host program to control the rotation of the object and the
//  camera position. The projection part transforms the 3D coordinate to a
//  2D screen position.
//  The math is a bit finicky for this but luckily KodeLife sets that up for us
//  and it is easy to use
uniform mat4 mvp;

//  The position of the current vertex in the model
in vec4 a_position;
//  The normal of the current vertex in the model
in vec3 a_normal;
//  The texture coordiante of the current vertex in the model
in vec2 a_texcoord;
// These are inputs into the vertex shader and the host process has defined a
//  model whose vertexes are fed into the vertex shader
//  These are very common inputs but it is possible for the host process to
//  remove any of these are send extra data such as color of the vertex

// The output of the vertex shader, this can be read by the fragment shaders
//  to control the shading
out VertexData
{
    // The transformed position, this is not the input position sent through
    //  the MVP. Rather one approach I sometimes use is just run the in position
    //  through the model transform so that it is rotated relative to the view
    //  so I can compute the lighting in the fragment shader correctly
    vec4 v_position;
    // Same goes for the normal
    vec3 v_normal;
    // I usually pass the texture coordinate untransformed but tinkering here
    //  is possible to get cool effects.
    vec2 v_texcoord;
} outData;
// This is common outputs of the vertex shader but it is easy and common to
//  extend this with more outputs to contol the shading done by the fragment
//  shader

void main(void)
{
    // gl_Position is the 2D coordinate of the vertex
    //  therefore we pass the input position through the MVP
    //  transform. The `w` part of the `vec4` will be used for
    //  applying perspective transformation.
    gl_Position = mvp * a_position;

    // In the standard KodeLife vertex shader the input data is just forwarded
    //  to the fragment shader
    outData.v_position = a_position;
    outData.v_normal = a_normal;
    outData.v_texcoord = a_texcoord;
}
```

## 🌟First Step: Let’s Switch It Up🌟

What we see initially in KodeLife is technically a 3D model, but it appears flat because we’re using a quad with an orthographic projection.

Let’s change that. We’ll swap the quad for a box (known as a “Primitive” in KodeLife) and switch to a perspective projection for a true 3D feel.

Go to the **Pass** tab, set **Primitive** to “Box,” and change **Projection** to “Perspective.”

![Setting up Primitive and Projection in KodeLife](assets/setup_projection.jpg)

Now the view changes, showing a square in the center of the screen. That square is actually a box! You’ll see this more clearly by adjusting some parameters under **Model** in KodeLife. For instance, I set **Rotate** to `(1, -1, -1)` to reveal the box’s 3D shape:

![It looks like a box now](assets/it_is_a_box.jpg)

## 🕯️ Let’s Shade the Cube🕯️

Growing up with computers in the 1980s, I’ve always loved rotating cubes; in fact, I believe that’s what computers were made for!

Now, let’s give the cube a more natural look by shading its sides with a basic fragment shader.

Replace the current fragment shader with this code to add simple lighting. I’ve included comments to explain what each part does.
```glsl
#version 150

uniform float time;

// This the input from the vertex shader
//  since the 2D primitive is a textured triangle
//  the input is blended between the 3 vertices
//  that makes up the triangle
//  If you desire you can ask use "flat" blending
//  so you get the value produced by the vertex shader.
in VertexData
{
    vec4 v_position;
    vec3 v_normal;
    vec2 v_texcoord;
} inData;

out vec4 fragColor;

// The direction of the light to illuminate our cube
const vec3 lightDir = normalize(vec3(1,1,2));
// The camera pos, should match the "eye" parameter in the View setting in KodeLife
//  In a real app this is likely injected as an uniform instead
const vec3 cameraPos   = vec3(0,0,4);

void main(void) {
  // Compute diffuse lighting by taking the dot product of the surface normal
  //  Classic stuff!
  float dif = max(dot(lightDir, inData.v_normal), 0);
  // Multiply with itself to give bit more metallic look
  dif *= dif;
  // Base illumination
  dif += 0.05;

  // Compute the ray direction from the eye position on the surface position in 3D
  vec3 rayDir = normalize(inData.v_position.xyz-cameraPos);
  // Now we can reflect the ray direction using it and the normal of the surface
  vec3 refDir = reflect(rayDir, inData.v_normal);
  // Given the reflected ray and the light dir we can compute the specular lighting
  //  by taking the dot product that will be closer to 1 the more the reflect ray
  //  aligns with the direction to the light
  //  To make it more "point-like" use the pow function
  float spe = pow(max(dot(lightDir, refDir), 0), 10.0);

  // Ready to compute the color
  vec3 col = vec3(0.0);
  // The diffuse color
  col += dif*vec3(1,0., 0.25);
  // The specular color
  col += spe;
  // Fake sRGB conversion
  //  Most screens are sRGB (although new screen tech liked OLED makes sRGB a bit outdated)

  //  sRGB color luminence is not linear
  //  so we need to convert our linear RGB into non-linear sRGB
  //  Which is a bit complicated, so I cheat and use sqrt as an approximation
  col = sqrt(col);

  // Finally write output fragment color
  fragColor = vec4(col, 1.0);
}
```

Now, you should see a cube with pink sides. However, when you try rotating it by changing the view parameters, the shading remains static. This is because we aren’t applying the model transform to the vertex positions and normals in the vertex shader.

Let’s fix that.

The Model-View-Projection (MVP) transform combines three matrices: model, view, and projection. Here, we need the model transform, but we can’t directly split MVP into these individual components.

Luckily, KodeLife lets us add the model transform as a uniform input to our vertex shader.

![Import model uniform](assets/model_uniform.jpg)

1. Make sure you’re editing the vertex shader.
2. Click on the shader stage, then under **Parameters**, click `+` to add a uniform.
3. Find the model transform under **Built-in > Transform > Model** and add it.
4. Rename the uniform to `model`.

Now, in the vertex shader source code, add this `model` uniform below the `mvp` uniform.
```glsl
// Should already be in the file
uniform mat4 mvp;
// Add this line to import the model uniform
uniform mat4 model;
```

Finally modify the main method of the vertex shader:
```glsl
void main(void) {
  gl_Position = mvp * a_position;

  // Multiply the model and a_position to create a position transform with the model transform
  //  Note that the ordering is important as matrix multiplication is non-commuative meaning the
  //  unlike normal algebra where a*b = b*a this is not true in general for matrix algebra
  outData.v_position  = model*a_position;
  // We also need to transform the normal the same way but we change the model from mat4 into mat3
  //  to match the dimensions of the normal (vec3). We lose the translation part of the matrix
  //  that way but that makes no sense to the normals anyway and what remains is just the rotation part.
  outData.v_normal    = mat3(model)*a_normal;
  outData.v_texcoord  = a_texcoord;
}
```

Now, the cube’s lighting should respond to rotation. To make the lighting effect more pronounced, try switching the **Primitive** from “Box” to “Teapot” or “Monkey” and see how the shading adapts.

Once you’ve explored the different models, switch back to “Box” so we can add a new element: *time*.

## 🎁Creating a Classic Rotating Cube🎁

By using a rotation matrix that changes over time, we can achieve that classic rotating cube effect.

Replace the vertex shader with the code below:
```glsl
#version 150

uniform float time;
uniform mat4 mvp;
uniform mat4 model;

in vec4 a_position;
in vec3 a_normal;
in vec2 a_texcoord;

out VertexData
{
    vec4 v_position;
    vec3 v_normal;
    vec2 v_texcoord;
} outData;

// In order to support time based rotations we need functions to create rotation matrix from
//  an angle
//  rotX rotates around the X axis, rotY around the Y axis and rotZ around the Z axis.

mat3 rotX(float a) {
  float c = cos(a);
  float s = sin(a);
  return mat3(
    1.0 , 0.0 , 0.0
  , 0.0 , +c  , +s
  , 0.0 , -s  , +c
  );
}

mat3 rotY(float a) {
  float c = cos(a);
  float s = sin(a);
  return mat3(
    +c  , 0.0 , +s
  , 0.0 , 1.0 , 0.0
  , -s  , 0.0 , +c
  );
}

mat3 rotZ(float a) {
  float c = cos(a);
  float s = sin(a);
  return mat3(
    +c  , +s  , 0.0
  , -s  , +c  , 0.0
  , 0.0 , 0.0 , 1.0
  );
}

void main(void) {
  // Create a rotation matrix that uses time to create an appealing (I hope)
  //  rotation
  //  We will use mat3 to rotate the normal
  mat3 trot3 = rotX(time)*rotY(sqrt(0.5)*time)*rotZ(0.3*sqrt(0.5)*time);
  // But as we also need to transform the position we need a mat4 version
  mat4 trot4 = mat4(trot3);

  // Combine mvp and our rotation matrix to transform the
  //  position into a 2D position
  gl_Position = (mvp * trot4) * a_position;

  // Combine the model and the rotation matrix to transform the position
  outData.v_position  = (model*trot4)*a_position;
  // And do the same for the normal but using mat3s
  outData.v_normal    = (mat3(model)*trot3)*a_normal;
  outData.v_texcoord  = a_texcoord;
}
```

By now, your setup should look something like this:

![Rotating boxes like it's 1989 again](assets/rotating_box.jpg)

If it’s not rotating yet, you may need to hit the **Play** button to start the timer.

## 🎉Let’s Kick It Up a Notch🎉

A single rotating cube is cool, but you know what’s cooler? A lot of rotating cubes.

We can achieve this with *instancing*. Instancing allows us to take a single shape (like our cube) and repeat it multiple times, modifying the output in the vertex shader based on each instance’s unique ID.

Replace the vertex shader with the code below:
```glsl
#version 150

uniform float time;
uniform mat4 mvp;
uniform mat4 model;

in vec4 a_position;
in vec3 a_normal;
in vec2 a_texcoord;

out VertexData
{
    vec4 v_position;
    vec3 v_normal;
    vec2 v_texcoord;
} outData;

// In order to support time based rotations we need functions to create rotation matrix from
//  an angle
//  rotX rotates around the X axis, rotY around the Y axis and rotZ around the Z axis.

mat3 rotX(float a) {
  float c = cos(a);
  float s = sin(a);
  return mat3(
    1.0 , 0.0 , 0.0
  , 0.0 , +c  , +s
  , 0.0 , -s  , +c
  );
}

mat3 rotY(float a) {
  float c = cos(a);
  float s = sin(a);
  return mat3(
    +c  , 0.0 , +s
  , 0.0 , 1.0 , 0.0
  , -s  , 0.0 , +c
  );
}

mat3 rotZ(float a) {
  float c = cos(a);
  float s = sin(a);
  return mat3(
    +c  , +s  , 0.0
  , -s  , +c  , 0.0
  , 0.0 , 0.0 , 1.0
  );
}

void main(void) {
  const float tau = acos(-1.)*2.;

  // Should match instance count in KodeLife
  const float numberOfInstances = 200;
  const float angleMul = tau*numberOfInstances/40.;
  // gl_InstanceID is an integer that is 0 for the first instance, 1 for the second and so on
  //  using this instance id we can apply different kind of transforms to the output of the vertex shader
  //  to create cool effects
  float i = float(gl_InstanceID)/numberOfInstances;
  float tm = 0.5*time;

  // Let's tweak the vertex position depending on the time and instance
  vec4 pos = a_position;
  // Some random combinations of sin and cos to produce new x,y,z coords
  pos.x += 13.*(0.25+cos(0.25*angleMul*i+tm))*sin(angleMul*i+tm);
  pos.y += 17.*(0.25+cos(0.25*angleMul*i+tm))*sin(0.25*angleMul*i+tm);
  pos.z += 19.*(0.25+cos(0.25*angleMul*i+tm))*cos(angleMul*i+tm);

  // Then we compute an angle that depends on the time and instance id and compute a rotation transform from that
  float angle = tm+i*tau*2;
  mat3 trot3 = rotX(angle)*rotY(sqrt(0.5)*angle)*rotZ(0.3*sqrt(0.5)*angle);
  mat4 trot4 = mat4(0.1*trot3);

  gl_Position = (mvp * trot4) * pos;

  outData.v_position  = (model*trot4)*pos;
  outData.v_normal    = (mat3(model)*trot3)*a_normal;
  outData.v_texcoord  = a_texcoord;
}
```

Not seeing a change? Increase the instance count in KodeLife:

![200 boxes in KodeLife](assets/200_boxes.jpg)

Now you should have 200 animated cubes filling the screen!

If you’re having trouble getting these examples to work, you can download the [KodeLife project here](200_boxes.klproj).

## 🎁That’s a Wrap 🎁

I hope this gave you a quick introduction to tinkering with vertex shaders to create fun effects.

To give you a little extra inspiration, I’ve dug up a few of my incomplete (and possibly buggy) shaders from my personal library. They’re in the same state they were when I last worked on them, so use at your own risk! Here are some additional KodeLife vertex shader examples:

1. [mrange & Virgill - *Delusions of Mediocrity* (Windows 4K intro)](delusions.klproj)
2. [Fractal 2D Tree](2d_tree.klproj)
3. [Fractal 3D Tree](falling_leaves.klproj)
4. [Parametric 3D Shapes](parametric3d.klproj)
5. [Solid Supershape](solid_supershapes.klproj)
6. [Neonwave Sunset](neonwave.klproj)
7. ["Star" Scroller](starscroller_variant.klproj)
8. [Underwater Bubbles](bubble_bobble.klproj)

Merry christmas all!

🎅 - mrange


## ❄️Licensing Information❄️

All code content I created for this blog post, including the linked KodeLife sample code, is licensed under [CC0](https://creativecommons.org/public-domain/cc0/) (effectively public domain). Any code snippets from other developers retain their original licenses.

The text content of this blog is licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) (the same license as Stack Overflow).