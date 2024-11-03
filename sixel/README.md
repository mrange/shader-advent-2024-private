# 🎄🌟🎄 Sixel graphics in Windows Terminal and beyond 🎄🌟🎄

🎅Ho, ho, ho! Merry Christmas fellow demo-coders!🎅

## Sixel? Did you mispell pixel?

Sixel is actually a thing. It was defined in the late 1970s by DEC to support "hires" printouts of images. Sixels are still supported by many Terminals and was recently added to Windows Terminal Preview.

**TODO: Image in Windows terminal**

For us that have the demo-coding itch it means a way to draw graphics in the terminal in a way that is to many surprising which mean it's cool.

I am going to demonstrate how do draw a simple effect with Sixels using C#.

## How to get started

First of all we should detect if the terminal supports Sixel graphics. The way we do it is that we send a special command `\x1B[c` to the terminal which will make it respond with the terminal capabilities. `\x1B` is the `ESC` character and often used when sending commands to the terminal.


```csharp
// Query terminal capabilities
Console.Write("\x1B[c");

// Allow time for terminal response
Thread.Sleep(100);
```

Then we read the response and check if the returned capabilities (semi-colon separated) contains a '4' which means Sixel is supported.


```csharp
var sb = new StringBuilder();
// Read all available input (non-blocking)
while (Console.KeyAvailable)
{
    var key = Console.ReadKey();
    sb.Append(key.KeyChar);
}

// Parse capabilities (semi-colon separated string)
var caps = sb
    .ToString()
    .Split(";")
    .Select(x => x.Trim())
    .ToHashSet();

// Check for Sixel support (capability code 4)
if (!caps.Contains("4"))
{
    throw new Exception("Terminal does not support Sixel graphics");
}
```

**IMPORTANT** In order for this example to work you need a terminal that supports sixels such as `Windows Terminal 1.22.2912.0+` (currently a preview at the time of writing). You can find Windows Terminal in Windows Store.

Then we are ready to set up the terminal for some graphics using some special codes.

```csharp
// Hide cursor
Console.Write("\x1B[?25l");
// Clear screen
Console.Write("\x1B[2J");
```

We setup a screen where we will draw our graphics to:

```csharp
// Screen dimensions
const int Width = 640;
const int Height = 400;
// Screen buffer - each byte represents one pixel
// Only 16 colors are supported (4 bits), using the TIC-80 palette
// Values 0-15 correspond to indices in the tic80Palette array
var screen = new byte[Width * Height];
```

## Implementing a simple effect

Using the screen we can implement a simple effect, in this case I computed the distance fields for circles that follows a sinus-like path. I combined the distance fields using `SoftMin` to create a meta-ball kind of effect.

I don't want to get into much more details as the important bits is the next part. Where we take the screen and maps it to a sixel image which we "print" in the terminal.

## Converting an image into sixels

Since we like to show our effect at the top of the terminal what we do first is moving the cursor to the top of the terminal and then clearing it.

In the world of sixels each sixel is six pixels high. That means we need to group 6 rows of pixels into a row of sixels.

A sixel is six bits where a `0` means leave the current pixel as is and where `1` means set the current pixel to the current color. Conceptually it works a bit like old matrix printers where you had color tapes with multiple colors and the printer head punched pixels through the color tape to create a dot on the paper.

In order to support multiple colors we need to define a palette, in this case I used the tic-80 palette because I think it's sweet.

Then we loop through each color in the palette and selects and test all pixels against that color, if it matches we set the right bit in the sixel value to output a pixel of that color.

A sixel is therefore a 6 bit value encoded as an 7 bit ASCII value. The base of the sixel is ASCII 63 or a `?`. `?` (ASCII 63) is an empty sixel and '~' (ASCII 126) is a full sixel.

In order to make sixels slightly more efficient it supports run-length encoding in that if you send the sequence `!10~` it will repeat sixel `~` 10 times. For short sequences up to 3 sixels it's better to just repeat the sixel like so: `~~~`.

Another optimization to keep in mind is that if the line ends a sequence of `?` sixels it means we can skip them as no output is made.

In order to write next color we end the line with '$' which returns the "printer head" to the start of the line.

After we are done with all colors we send '-' which moves the "printer head" to the start of the line and advances us to the next line.

So in order to generate a sixel image:

1. We move the cursor to the top by sending: `\x1B[H`
2. We clear the terminal by sending: `\x1B[12t`
3. We being a sixel image by sending its prelude: `\x1BP7;1;q`
4. We then iterate through our palette and for each color send: `#COLOR_INDEX;2;RED-0-100;BLUE-0-100;GREEN-0-100`
5. We then group rows into groups of 6 because each sixel is six pixels high
6. For each color in the palette we start the row by selecting the color by sending: `#COLOR_INDEX`
7. We then iterate through each pixels in row group and if the pixel matches the current color we set the corresponding bit in the pixel. The base sixel value is `?` (ASCII 63)
8. In order to optimize before sending the sixel we count how many times it repeats so we can use the run-length encoding.
9. We send each sixel either indivually or run-length encoded.
10. End the line with ´$´ to return the "printer-head" to the start of the line for next color
11. If we are done with the colors we send `-` to get to next line.
12. Finally we complete the sixel image by sending: `\x1B\\`

In code it looks like this:

```csharp
// Render screen using Sixel graphics
{
    builder
        .Clear()
        .Append("\x1B[H")      // Move cursor to home position
        .Append("\x1B[12t")    // Clear screen
        .Append("\x1BP7;1;q")  // Initialize Sixel mode (square pixels)
        ;

    // Define color palette
    foreach (var color in tic80Palette)
    {
        builder.Append($"#{color.Index};2;{color.Red.ToSixelColorComponent()};{color.Green.ToSixelColorComponent()};{color.Blue.ToSixelColorComponent()}");
    }

    // Convert pixel data to Sixel format
    // Each Sixel represents 6 vertical pixels
    for (var y6 = 0; y6 < Height; y6 += 6)
    {
        // Process each color separately for RLE optimization
        foreach (var color in tic80Palette)
        {
            var idx = color.Index;
            builder.Append($"#{idx}");

            // Run-length encoding tracking
            byte repeatedSixel = SixelBase;
            int sixelRepetition = 0;

            for (var x = 0; x < Width; ++x)
            {
                byte sixel = 0;
                // Handle edge case where height isn't divisible by 6
                var rem = Min(6, Height - y6);

                // Build sixel by checking each vertical pixel
                for (var i = 0; i < rem; ++i)
                {
                    var y = y6 + i;
                    var pixel = screen[x + y * Width];
                    if (pixel == idx)
                    {
                        sixel |= (byte)(1 << i);
                    }
                }

                sixel += SixelBase;

                // Handle run-length encoding
                if (repeatedSixel == sixel)
                {
                    ++sixelRepetition;
                }
                else
                {
                    // Output previous run
                    if (sixelRepetition > 3)
                    {
                        // Use RLE for runs longer than 3
                        builder
                            .Append($"!{sixelRepetition}")
                            .Append((char)repeatedSixel);
                    }
                    else
                    {
                        // Direct output for short runs
                        for(var i = 0; i < sixelRepetition; ++i)
                        {
                            builder.Append((char)repeatedSixel);
                        }
                    }

                    repeatedSixel = sixel;
                    sixelRepetition = 1;
                }
            }

            // Output final run if not empty
            if (repeatedSixel != SixelBase)
            {
                if (sixelRepetition > 3)
                {
                    builder
                        .Append($"!{sixelRepetition}")
                        .Append((char)repeatedSixel);
                }
                else
                {
                    for(var i = 0; i < sixelRepetition; ++i)
                    {
                        builder.Append((char)repeatedSixel);
                    }
                }
            }

            builder.Append('$');  // Return to start of line
        }

        builder.Append('-');  // Move to next row
    }

    // End Sixel sequence
    builder.Append("\x1B\\");

    // Output to console
    Console.Write(builder.ToString());
}
```

Then putting it all together it should look something like this:

**TODO**

The full code [is available here](sixel-app/Program.cs) but I also include it below:

```csharp
// First check if the terminal supports Sixel graphics
{
    // Clear any pending input from STDIN
    while (Console.KeyAvailable)
    {
        Console.ReadKey();
    }

    // Query terminal capabilities
    Console.Write("\x1B[c");

    // Allow time for terminal response
    Thread.Sleep(100);

    var sb = new StringBuilder();
    // Read all available input (non-blocking)
    while (Console.KeyAvailable)
    {
        var key = Console.ReadKey();
        sb.Append(key.KeyChar);
    }

    // Parse capabilities (semi-colon separated string)
    var caps = sb
        .ToString()
        .Split(";")
        .Select(x => x.Trim())
        .ToHashSet();

    // Check for Sixel support (capability code 4)
    if (!caps.Contains("4"))
    {
        throw new Exception("Terminal does not support Sixel graphics");
    }
}

// Hide cursor
Console.Write("\x1B[?25l");
// Clear screen
Console.Write("\x1B[2J");

// TIC-80 fantasy console color palette
// See: https://tic80.com/
RGB[] tic80Palette =
[
    new RGB(0x0, 0x1C, 0x1C, 0x2C), // Night Blue
    new RGB(0x1, 0x5D, 0x27, 0x5D), // Deep Purple
    new RGB(0x2, 0xB1, 0x3E, 0x53), // Dark Red
    new RGB(0x3, 0xEF, 0x7D, 0x57), // Orange
    new RGB(0x4, 0xFF, 0xCD, 0x75), // Yellow
    new RGB(0x5, 0xA7, 0xF0, 0x70), // Light Green
    new RGB(0x6, 0x38, 0xB7, 0x64), // Green
    new RGB(0x7, 0x25, 0x71, 0x79), // Teal
    new RGB(0x8, 0x29, 0x36, 0x6F), // Dark Blue
    new RGB(0x9, 0x3B, 0x5F, 0xC9), // Blue
    new RGB(0xA, 0x41, 0xA6, 0xF6), // Light Blue
    new RGB(0xB, 0x73, 0xEF, 0xF7), // Cyan
    new RGB(0xC, 0xF4, 0xF4, 0xF4), // White
    new RGB(0xD, 0x94, 0xB0, 0xC2), // Light Gray
    new RGB(0xE, 0x56, 0x6C, 0x86), // Gray
    new RGB(0xF, 0x33, 0x3C, 0x57)  // Dark Gray
];

// Sixel constants
const byte SixelBase = 63;  // Base character '?' (ASCII 63)

// Screen dimensions
const int Width = 640;
const int Height = 400;
// Screen buffer - each byte represents one pixel
// Only 16 colors are supported (4 bits), using the TIC-80 palette
// Values 0-15 correspond to indices in the tic80Palette array
var screen = new byte[Width * Height];

var builder = new StringBuilder();
var clock = Stopwatch.StartNew();
var fps = 60;
var sleepFor = (int)Math.Round(1000.0 / fps);

var done = false;
while (!done)
{
    var before = clock.ElapsedMilliseconds;
    // Check for exit condition (Escape key)
    if (Console.KeyAvailable)
    {
        var key = Console.ReadKey();
        done |= key.Key == ConsoleKey.Escape;
    }

    // Generate a simple animated effect
    {
        var time = before / 1000.0;
        for (var y = 0; y < Height; ++y)
        {
            var yoff = y * Width;
            var yy = (-Height + 2.0 * y) / Height;
            for (var x = 0; x < Width; ++x)
            {
                var xx = (-Width + 2.0 * x) / Height;

                var d = 1E3;
                for (var i = 0; i < 5; ++i)
                {
                    var itime = time + i;
                    var xx2 = xx + Sin(itime);
                    var yy2 = yy + Sin(itime * 0.707);
                    var d2 = Sqrt(xx2 * xx2 + yy2 * yy2) - 0.5;
                    d = SoftMin(d, d2, 0.5);
                }

                var od = Abs(d) - 0.025;

                // Color selection based on distance field
                byte col = 8;
                if (d < 0.0)
                {
                    col = (byte)(((int)Round((d + time) * 16)) & 0xF);
                }
                if (od < 0.0)
                {
                    col = 12;
                }

                screen[x + yoff] = col;
            }
        }
    }

    // Render screen using Sixel graphics
    {
        builder
            .Clear()
            .Append("\x1B[H")      // Move cursor to home position
            .Append("\x1B[12t")    // Clear screen
            .Append("\x1BP7;1;q")  // Initialize Sixel mode (square pixels)
            ;

        // Define color palette
        foreach (var color in tic80Palette)
        {
            builder.Append($"#{color.Index};2;{color.Red.ToSixelColorComponent()};{color.Green.ToSixelColorComponent()};{color.Blue.ToSixelColorComponent()}");
        }

        // Convert pixel data to Sixel format
        // Each Sixel represents 6 vertical pixels
        for (var y6 = 0; y6 < Height; y6 += 6)
        {
            // Process each color separately for RLE optimization
            foreach (var color in tic80Palette)
            {
                var idx = color.Index;
                builder.Append($"#{idx}");

                // Run-length encoding tracking
                byte repeatedSixel = SixelBase;
                int sixelRepetition = 0;

                for (var x = 0; x < Width; ++x)
                {
                    byte sixel = 0;
                    // Handle edge case where height isn't divisible by 6
                    var rem = Min(6, Height - y6);

                    // Build sixel by checking each vertical pixel
                    for (var i = 0; i < rem; ++i)
                    {
                        var y = y6 + i;
                        var pixel = screen[x + y * Width];
                        if (pixel == idx)
                        {
                            sixel |= (byte)(1 << i);
                        }
                    }

                    sixel += SixelBase;

                    // Handle run-length encoding
                    if (repeatedSixel == sixel)
                    {
                        ++sixelRepetition;
                    }
                    else
                    {
                        // Output previous run
                        if (sixelRepetition > 3)
                        {
                            // Use RLE for runs longer than 3
                            builder
                                .Append($"!{sixelRepetition}")
                                .Append((char)repeatedSixel);
                        }
                        else
                        {
                            // Direct output for short runs
                            for(var i = 0; i < sixelRepetition; ++i)
                            {
                                builder.Append((char)repeatedSixel);
                            }
                        }

                        repeatedSixel = sixel;
                        sixelRepetition = 1;
                    }
                }

                // Output final run if not empty
                if (repeatedSixel != SixelBase)
                {
                    if (sixelRepetition > 3)
                    {
                        builder
                            .Append($"!{sixelRepetition}")
                            .Append((char)repeatedSixel);
                    }
                    else
                    {
                        for(var i = 0; i < sixelRepetition; ++i)
                        {
                            builder.Append((char)repeatedSixel);
                        }
                    }
                }

                builder.Append('$');  // Return to start of line
            }

            builder.Append('-');  // Move to next row
        }

        // End Sixel sequence
        builder.Append("\x1B\\");

        // Output to console
        Console.Write(builder.ToString());
    }

    // Maintain target framerate
    var after = clock.ElapsedMilliseconds;
    var elapsed = after - before;
    if (elapsed < sleepFor)
    {
        Thread.Sleep((int)(sleepFor - elapsed));
    }
}

// Helper functions
double Mix(double a, double b, double x)
{
    return a + (b - a) * x;
}

// Smooth minimum function
// License: MIT, author: Inigo Quilez
// Source: https://www.iquilezles.org/www/articles/smin/smin.htm
double SoftMin(double a, double b, double k)
{
    var h = Clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
    return Mix(b, a, h) - k * h * (1.0 - h);
}

record RGB(byte Index, byte Red, byte Green, byte Blue);

static class Extensions
{
    // Convert 8-bit color component to Sixel color range (0-100)
    public static byte ToSixelColorComponent(this byte c)
    {
        return (byte)Round(c * 100.0 / 255);
    }
}
```

## That's a wrap

So I hope this simple example on how to generate sixel graphics will be inspiring to some to create some cool sixel based demos? Or why not port Tic-80 to sixel graphics, that would be awesome? Or perhaps we need a new GUI toolkit that let's us draw buttons in menus as Sixels?

The amount of entertaining abuse we can get out Sixels are limitless!


Merry christmas all!

🎅 - mrange


## ❄️Licensing Information❄️

All code content I created for this blog post, including the linked KodeLife sample code, is licensed under [CC0](https://creativecommons.org/public-domain/cc0/) (effectively public domain). Any code snippets from other developers retain their original licenses.

The text content of this blog is licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) (the same license as Stack Overflow).

