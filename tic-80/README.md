# 🎄🌟🎄 Treat yourself to a TIC-80 🎄🌟🎄

🎅 Ho, ho, ho! Merry Christmas, fellow retro-hacckers! 🎅

## 🕹️📼🖲️ TIC-80? I thought this was meant to be about shaders? 🖲️📼🕹️

Yes, normally for this Shader advent I write about shaders but TIC-80 is just so much fun!

In addition, I like shader coding because it scratches the demo-coding itch in me and TIC-80 does the same. In addition; with shader coding I want to deliver nice looking stuff but that takes time and effort. In TIC-80 I can code up some craptastic retro effect in a single sitting and having a blast.

TIC-80 got everything built in, text editor, sprite editor, map editor, sfx editor and a tracker.

TIC-80 got weird limitations that makes you feel a bit like you are back on the C64.

It's too much fun not to mention!

## Getting started with TIC-80

For those that don't know TIC-80 is a fantasy console that reminds us about computers of old in that it has reduced resolution and weird limitations.

Getting started with TIC-80 is really easy, just goto the [create page](https://tic80.com/create) for TIC-80 and you have a fantasy console in your browser. You can download the fantasy console on the same page.

You can look at cool stuff people do for example [TIMELINE 2](https://tic80.com/play?cart=3823) and you can interrupt the demo and look at the code by pressing `Escape`.

And if TIC-80 is not your thing perhaps [Pico8](https://www.lexaloffle.com/pico-8.php) is?

## Orienting ourselves in TIC-80

When you start TIC-80 you end up in the console. By fitting the function keys you enter the different editors:

F1 - Code editor
F2 - Sprite editor
F3 - Map editor
F4 - SFX editor
F5 - Music editor (tracker)

Pressing `Ctrl-R` runs the TIC-80 program.

## Configure TIC-80

The first thing I do is making sure it's easy to jump back and forth between the code and the demo (or game)?

1. Start the TIC-80 program
2. Hit `Escape`, this opens a menu with multiple options
3. Select `Options`
4. Select `Dev Move: On`
5. Whenever you hit `Escape` from now on you end up directly in the code editor/console.

That means it's now very quick to jump back and forth between running your game/demo and editing. Hit `Ctrl-R` to run, hit `Escape` to get back to code/console.

## `Hello World` in TIC-80

The `Hello World` program in TIC-80 looks like this:

```lua
t=0
x=96
y=24

function TIC()

	if btn(0) then y=y-1 end
	if btn(1) then y=y+1 end
	if btn(2) then x=x-1 end
	if btn(3) then x=x+1 end

	cls(13)
	spr(1+t%60//30*2,x,y,14,3,0,0,2,2)
	print("HELLO WORLD!",84,84)
	t=t+1
end
```

This is a simple program that when run let's you control a character using the arrow keys. By hitting `F2` you jump into the sprite editor that let's you tinker with the sprites.

The program itself is in `lua` which is a simple yet flexible language and the program itself should be simple to follow.

The function `TIC` runs every frame (60 times per second) so here is where we put most of our code.

The only "difficult" thing is the sprite animation:

```lua
	spr(1+t%60//30*2,x,y,14,3,0,0,2,2)
```

[`spr`](https://github.com/nesbox/TIC-80/wiki/spr) draws a sprite on the screen. Breaking down the arguments:

```lua
spr(1+t%60//30*2, -- Alternates between 1 and 3
    x,            -- X coordinate
    y,            -- Y coordinate
    14,           -- Transparent color key
    3,            -- Zoom (3x)
    0,            -- Horizontal flip (0 = no flip)
    0,            -- Vertical flip (0 = no flip)
    2,            -- Sprite width in tiles
    2)            -- Sprite height in tiles
```

`1+t%60//30*2` can be understood by breaking it down:

1. `t%60` - This takes the current frame number (t) and gets the remainder when divided by 60
   - With 60 FPS, this creates a repeating cycle from 0-59 every second

2. `//30` - Integer division by 30
   - This effectively splits the 60-frame cycle into two parts:
   - When t%60 is 0-29: result is 0
   - When t%60 is 30-59: result is 1

3. `*2` - Multiplies the above result by 2
   - So it alternates between 0 and 2

4. `1 + ...` - Adds 1 to the final result
   - This means the sprite ID alternates between 1 and 3

Pretty cool.

## Demo of a demo

I prepared [an example of a demo screen](src/merry-christmas.lua) that I hope can inspire people to tinker around in TIC-80.

Just create a new LUA program by hitting `Escape` until you end up in the console and type: `new lua`.

Replace the code with the content of [the example](src/merry-christmas.lua).

Press `Ctrl-R` to run the demo.

## Breaking down the demo

## Let's start with `TIC`

```lua
-- TIC() is the main function that TIC-80 calls every frame
-- (60 times per second). Here we:
-- 1. Get the current time
-- 2. Clear the screen
-- 3. Draw all our effects in order
function TIC()
	local tm
	tm = time()/1000  -- Convert to seconds
	cls(0)           -- Clear screen to black

	-- Draw all effects
	apollonianEffect(tm)
	bouncer(tm)
	topBar(tm)
	bottomBar(tm)
end
```

Pretty easy.

## What is this `BDR` thing?

`BDR` is a very cool function called once per horizontal line by TIC-80. It is a callback to old raster interrupts in C64 and other retro computers. Those computers had limited amount of colors but as old TVs rendered each line from top to bottom you could change the color palette after the TV rendered a line and thus end up with more than the possible colors on the screen.

Modern computers don't have this limitation but TIC-80 is a fantasy console that only has 16 colors on the screen but just like the conmputers of old it allows us to switch the colors (and other things) between lines to get more than 16 colors.

As with retro computers we [`poke`](https://github.com/nesbox/TIC-80/wiki/poke) the values directly into the memory which is TIC-80 way of emulating hardware registers of computers of old.

```lua
-- Sets Red component of color palette 0
poke(0x3FC0, 0xDF)
-- Sets Green component of color palette 0
poke(0x3FC1, 0xF1)
-- Sets Blue component of color palette 0
poke(0x3FC2, 0x80)

-- Sets Red component of color palette 1
poke(0x3FC3, 0xFF)
-- Sets Green component of color palette 1
poke(0x3FC4, 0x82)
-- Sets Blue component of color palette 1
poke(0x3FC5, 0x42)

-- and so on...
```

This is so nice! `HAL` (Hardware Abstraction Layer) was a mistake!

So below creates a nice looking blue gradient.

```lua
-- BDR is a special TIC-80 function that's called for every
-- scanline (horizontal line) of the screen. It's similar to
-- the raster interrupts used in old computers like the C64
-- to create effects that would otherwise be impossible due
-- to hardware color limitations.
function BDR(ln)
	local top,bottom
	top = 23+50
	bottom = 121
	if (ln < top) or (ln > bottom) then
		-- For the top and bottom portions of the screen,
		-- use the normal TIC-80 background color palette
		poke(0x3FC0, 0x1A)
		poke(0x3FC1, 0x1C)
		poke(0x3FC2, 0x2C)
	else
		-- For the middle portion, create a deep blue gradient
		-- by modifying the background color for each scanline
		poke(0x3FC0, 0x1A)
		poke(0x3FC1, 0x1C)
		poke(0x3FC2, 0x2C+3*(ln-top))
	end
end
```

## Let's look at the bottom bar

The bottom bar is the simplest one so let's start there:

1. Using [`rect`](https://github.com/nesbox/TIC-80/wiki/rect) to draw a blue background
2. A white [`line`](https://github.com/nesbox/TIC-80/wiki/line) to separate from the main effect
3. [`Print`](https://github.com/nesbox/TIC-80/wiki/print) `Hello World` going back and forth.

```lua
-- Creates the bottom banner with animated "Merry Christmas" text
-- The text moves in a sinusoidal pattern and includes a shadow
-- for better visibility
function bottomBar(tm)
	local px
	rect(0,118,240,136,8)
	line(0,118,240,118,12)

	-- Create moving text effect
	px = sin(tm)*40+30
	print("Merry Christmas", px+1,122+1,0,0,2)  -- Shadow
	print("Merry Christmas", px,122,12,0,2)     -- Text
end
```

## What going on in the top bar?

Top bar is more complex but the idea is this.

1. Use [`spr`](https://github.com/nesbox/TIC-80/wiki/spr) we draw robots filling the entire top bar.
2. Using the "cell id" of the bot we compute the color and a hash value for it. The bounce height and speed is determined from the cell id.
3. Draw a rectangle using [`rect`](https://github.com/nesbox/TIC-80/wiki/rect)
4. Use `poke4` to "recolor" the sprite to match the background.
5. Finally draw a white [line](https://github.com/nesbox/TIC-80/wiki/line) to separate from the main effect.

```lua
-- This creates a row of bouncing robots at the top of
-- the screen.
function topBar(tm)
	local px,py,dx,tx,w,nx,fx,b,si,i
	local h0,h1,sx
	dx = 30     -- Horizontal spacing
	sx = 20     -- Segment width

	for i=-1,11 do
		tx = dx*tm
		nx = tx//sx-i
		-- Create variation between robots using hash function
		h0	= hash(nx+123.4)
		h1 = fract(8667*h0)
		-- Calculate bounce with varying heights
		b  = fract(mix(1.5,0.5, h0*h0)*(tm+h0))-0.5
		b  = b*b
		b  = b
		px = round(tx%sx+i*sx)
		py = round(40*mix(0.25,1.0,h0)*(b-0.25)+3)
		si = floor(3*tm*mix(0.5,1.5,h1))%2
		-- Draw background bar and sprite
		-- Remove the white color (12) from
		--	the color cycle
		nx = (round(nx%15)-3)&0xF
		rect(px-2,0,sx,18,nx)
		-- Switching palette color 10
		--	This renders the sprite with
		--	different base colors
		poke4(0x3FF0*2+10,nx)
		spr(1+2*si,px-1,py,14,1,1,0,2,2)
		-- Restoring palette color 10
		poke4(0x3FF0*2+10,10)
	end
	line(0,18,240,18,12)
end
```

## The big bouncing robot.

Quite straight forward.

1. Compute `x` that goes back and forth depending on the time
2. Compute `y` to bounc up and down
3. Use [`spr`](https://github.com/nesbox/TIC-80/wiki/spr) to render a big robot (zoom level 3x)

```lua
-- This function creates a single bouncing robot sprite
-- that moves back and forth across the screen. The robot
-- automatically flips direction when it reaches the edges,
-- and its vertical position follows a bouncing pattern.
function bouncer(tm)
	local px,py,dx,tx,w,nx,fx,b,si
	w  = 240-48        -- Screen width minus sprite width
	dx = 80            -- Horizontal speed
	tx = dx*tm         -- Total distance traveled
	nx = (tx//w)%2     -- Number of complete travels (for direction)
	if nx == 0 then
		px = round(tx%w)   -- Moving right
		fx = 1
	else
		px = round(w-tx%w) -- Moving left
		fx = 0
	end
	-- Create bouncing motion using a parabola
	b  = fract(tm)-0.5
	b  = b*b
	py = round(50+100*b)
	-- Animate the sprite (alternating between two frames)
	si = floor(3*tm)%2
	spr(1+2*si,px,py,14,3,fx,0,2,2)
end
```

## The fractal thing

The main effect is an Apollonian fractal which is very commonly in shaders for example [this one by IQ](https://www.shadertoy.com/view/4ds3zn).

To spice things up a bit, even if this a 2D effect the Apollonian fractal is in 3D. Then we sample the Apollonian fractal against points on a rotating 3D plane creating a cool looking (I think) animated 2D fractal.

To achieve this I loop of over 100x100 pixels from the pixel coord compute the 3D coordinate, rotate and compute the Appollonian distance in all three axis.

If inside the distance we plot a pixel using [`pix`](https://github.com/nesbox/TIC-80/wiki/poke) and color it different shades of blue depending on which axis we are considering now.

In addition; reflect the result to the left and right and color the pixels to the left in shades of red and to the right in shades of green.

Pretty sweet!

```lua
-- This creates the main visual effect based on the apollonian
-- fractal. For each pixel, we:
-- 1. Transform the pixel coordinates into 3D space
-- 2. Apply a rotation to create movement
-- 3. Apply the apollonian fractal formula
-- 4. Draw the result in different colors
--
-- The effect creates an intricate pattern of curved lines
-- that seems to fold through space as it rotates.
function apollonianEffect(tm)
	local a,s,c1,s1,c2,s2,radii
	local anim,px,py,pz,spx,tmp
	local scale,r2,k
	s							= 1.25
	a							= tm*0.25
	-- Calculate rotation matrices
	c1						= cos(a)
	s1						= sin(a)
	c2						= cos(a*1.234)
	s2						= sin(a*1.234)

	-- Set the thickness of the lines we'll draw
	radii			= 0.005
	anim				= 1.5
	for x=0,99 do
		-- Convert screen coordinates to normalized space (-1 to 1)
		spx = -1+x*0.02
		-- Apply smoothing at the edges
		spx = tanh_approx(spx*anim)/anim
		for y=0,99 do
			px = spx
			py = -1+y*0.02
			px = px*0.5
			py = py*0.5
			-- Create animation in the z dimension
			pz = 0.3*(c1+s2)
			-- Apply 3D rotation to create movement
			tmp=  c1*px+s1*pz
			pz = -s1*px+c1*pz
			px = tmp

			tmp=  c2*py+s2*pz
			pz = -s2*py+c2*pz
			py = tmp
			scale = 1
			-- The apollonian fractal loop
			--	Many shaders based on this fractal
			--	For example IQ's: https://www.shadertoy.com/view/4ds3zn
			for i=0,2 do
				px = -1+2*fract(0.5*px+0.5)
				py = -1+2*fract(0.5*py+0.5)
				pz = -1+2*fract(0.5*pz+0.5)
				r2 = px*px+py*py+pz*pz
				k  = s/r2
				px = k*px
				py = k*py
				pz = k*pz
				scale = scale*k
			end

			scale = 1/scale

			-- Draw the fractal by testing each axis
			-- We draw three copies with different colors:
			-- center (blue), left (red), and right (green)
			if abs(pz)*scale < radii then
				-- Mid (blue)
				pix(x+70,y+18,11)
				-- Left (red)
				pix(69-x,y+18,3)
				-- Right (green)
				pix(269-x,y+18,5)
			end

			if abs(py)*scale < radii then
				-- Mid (blue)
				pix(x+70,y+18,10)
				pix(69-x,y+18,2)
				pix(269-x,y+18,6)
			end

			if abs(px)*scale < radii then
				-- Mid (blue)
				pix(x+70,y+18,9)
				-- Left (red)
				pix(69-x,y+18,1)
				-- Right (green)
				pix(269-x,y+18,7)
			end
		end
	end
end
```

## The setup

The setup is usually nothing in this case but I left in an option to run using a "strict" mode. Normally lua silently creates global variables so it's very easy to end up with global variables when I wanted locals.

With the help of the method `strict` lua will throw an exception whenever I create a global varible unintentionally.

Normally, I don't initialize the strict mode but if I have been writing lots of code I turn on strict model to clean up the code from globals.

```lua
-- Initial setup function
function setup()
-- Uncomment to enable strict mode for debugging
--	strict()
end
```

## 🎁 And that's a wrap 🎁

TIC-80 is amazing, it's so much fun I think to tinker around with retro effects in it. It's also really easy to get started, [run it in the browser](https://tic80.com/create) or download a copy (from the same page). There are tons of example on the site and TIC-80 includes everything you need to make retro demos or games.

It's super easy to deploy your TIC-80 programs. For example to deploy it to a static web app do `export html my-app`. You get a zip file, upload it's contents to the web and you are done!

In addition; most mondays on twitch [FieldFx](https://www.twitch.tv/fieldfxdemo) streams TIC-80 coding jams. Really recommend tuning in and watch or even better; join in the fun!

🎄🌟🎄 Merry Christmas to all, and retro-tastic coding! 🎄🌟🎄

🎅 – mrange


## ❄️Licensing Information❄️

All code content I created for this blog post, including the linked KodeLife sample code, is licensed under [CC0](https://creativecommons.org/public-domain/cc0/) (effectively public domain). Any code snippets from other developers retain their original licenses.

The text content of this blog is licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) (the same license as Stack Overflow).

Additionally, the Sixel image of "Across The Void II" by "Made/Bomb" is a derivative [of the original image](https://demozoo.org/graphics/342269/), and "Made/Bomb" holds the copyright for it.

