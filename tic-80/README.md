# 🎄🌟🎄 Treat yourself to a TIC-80 🎄🌟🎄

🎅 Ho, ho, ho! Merry Christmas, fellow retro-hacckers! 🎅

## 🏙️ TIC-80? I thought this was meant to be about shaders? 🏙️

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

### Let's start with `TIC`

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

### What is this `BDR` thing?

`BDR` is a very cool function called once per horizontal line by TIC-80. It is a callback to old raster interrupts in C64 and other retro computers. Those computers had limited amount of colors but as old TVs rendered each line from top to bottom you could change the color palette after the TV rendered a line and thus end up with more than the possible colors on the screen.

Modern computers don't have this limitation but TIC-80 is a fantasy console that only has 16 colors on the screen but just like the conmputers of old it allows us to switch the colors (and other things) between lines to get more than 16 colors.

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

### Setup

🎄🌟🎄 Merry Christmas to all, and happy coding! 🎄🌟🎄

🎅 – mrange


## ❄️Licensing Information❄️

All code content I created for this blog post, including the linked KodeLife sample code, is licensed under [CC0](https://creativecommons.org/public-domain/cc0/) (effectively public domain). Any code snippets from other developers retain their original licenses.

The text content of this blog is licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) (the same license as Stack Overflow).

Additionally, the Sixel image of "Across The Void II" by "Made/Bomb" is a derivative [of the original image](https://demozoo.org/graphics/342269/), and "Made/Bomb" holds the copyright for it.

