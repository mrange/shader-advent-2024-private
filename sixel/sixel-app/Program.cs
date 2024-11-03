// First read the console capabilities
using System.Diagnostics;

{
    // Remove any potential input on the STDIN
    while (Console.KeyAvailable)
    {
        Console.ReadKey();
    }

    // Ask the terminal what capabilities it has
    Console.Write("\x1B[c");

    // Wait for awhile to let the terminal respond
    Thread.Sleep(100);

    var sb = new StringBuilder();
    // Read all input available, non-blocking
    while (Console.KeyAvailable)
    {
        var key = Console.ReadKey();
        sb.Append(key.KeyChar);
    }

    // Capabilities is a semi-colon separated string
    var caps = sb
        .ToString()
        .Split(";")
        .Select(x => x.Trim())
        .ToHashSet()
        ;

    // Look for the Sixel capability (4)
    if (!caps.Contains("4"))
    {
        throw new Exception("This terminal lacks Sixel capability");
    }
}

// Hides the cursor
Console.Write("\x1B[?25l");
// Clears screen
Console.Write("\x1B[2J");

// Define tic80 palette
RGB[] tic80Palette =
    [
        new RGB(0x0, 0x1C, 0x1C, 0x2C)
    ,   new RGB(0x1, 0x5D, 0x27, 0x5D)
    ,   new RGB(0x2, 0xB1, 0x3E, 0x53)
    ,   new RGB(0x3, 0xEF, 0x7D, 0x57)
    ,   new RGB(0x4, 0xFF, 0xCD, 0x75)
    ,   new RGB(0x5, 0xA7, 0xF0, 0x70)
    ,   new RGB(0x6, 0x38, 0xB7, 0x64)
    ,   new RGB(0x7, 0x25, 0x71, 0x79)
    ,   new RGB(0x8, 0x29, 0x36, 0x6F)
    ,   new RGB(0x9, 0x3B, 0x5F, 0xC9)
    ,   new RGB(0xA, 0x41, 0xA6, 0xF6)
    ,   new RGB(0xB, 0x73, 0xEF, 0xF7)
    ,   new RGB(0xC, 0xF4, 0xF4, 0xF4)
    ,   new RGB(0xD, 0x94, 0xB0, 0xC2)
    ,   new RGB(0xE, 0x56, 0x6C, 0x86)
    ,   new RGB(0xF, 0x33, 0x3C, 0x57)
    ];

// Base Sixel is ASCII 63
const byte SixelBase    = 63;

// Define a screen in the same size as Tic-80
const int Width         = 240;
const int Height        = 136;

var screen  = new byte[Width*Height];
var builder = new StringBuilder();
var clock   = Stopwatch.StartNew();
var fps     = 60;
var sleepFor= (int)Math.Round(1000.0/fps);

var done = false;
while (!done) 
{
    var before = clock.ElapsedMilliseconds;
    // Check if we are done
    if (Console.KeyAvailable)
    {
        var key = Console.ReadKey();
        done |= key.Key == ConsoleKey.Escape;
    }

    // Effect
    {
        var time = before/1000.0;
        var bcol = (byte)(time*16);
        for (int i = 0; i < screen.Length; ++i)
        {
            screen[i] = (byte)(bcol+i);
        }
    }

    // Draw screen as sixels
    {
        // We "draw" to a string builder
        // When complete we send it to the console
        builder
            .Clear()
            // Clears the screen
            .Append("\x1B[H")
            // Sixel image prelude (square sixels)
            .Append("\x1BP7;1;q")
            ;
        // Output the tic80 palette to the sixel image
        foreach (var color in tic80Palette)
        {
            builder.Append($"#{color.Index};2;{color.Red.ToSixelColorComponent()};{color.Green.ToSixelColorComponent()};{color.Blue.ToSixelColorComponent()}");
        }

        // Write the pixels as sixels

        // As each sixel row is six pixel high we increment by 6
        for (var y6 = 0; y6 < Height; y6 += 6) 
        {
            // For each color we write all pixels of that color as sixels
            foreach (var color in tic80Palette)
            {
                var idx = color.Index;
                builder.Append($"#{idx}");

                // Sixels supports run-length encoding. To support that
                //  we keep track of the current sixel and how many times
                //  it is repeated
                byte repeatedSixel = SixelBase;
                int sixelRepetition = 0;
                // Apply the current color to following sixels
                for (var x = 0; x < Width; ++x) 
                {
                    byte sixel = 0;
                    // Check so we don't overrun the buffer in case Height 
                    //  not divisible by 6
                    var rem = Math.Min(6, Height - y6);
                    // Accumulate the sixel
                    for (var i = 0; i < rem; ++i) 
                    {
                        var y = y6+i;
                        var pixel = screen[x+y*Width];
                        if (pixel == idx)
                        {
                            // Current pixel matches the current sixel color
                            //  Then set the corresponding bit in the sixel
                            sixel |= (byte)(1 << i);
                        }
                    }

                    // Add the sixel base
                    sixel += SixelBase;

                    // Is this pixel the same as the pixel being currently repeated?
                    if (repeatedSixel == sixel)
                    {
                        ++sixelRepetition;
                    }
                    else
                    {
                        // No then write the sixel to the string builder
                        if (sixelRepetition > 3) 
                        {
                            // Sixel repetition more than 3. Then it makes sense to use
                            //  the run-length encoding
                            builder
                                .Append($"!{sixelRepetition}")
                                .Append((char)repeatedSixel);
                        } 
                        else 
                        {
                            // Less than 3, then we just repeat the sixel
                            for(var i = 0; i < sixelRepetition; ++i) 
                            {
                                builder.Append((char)repeatedSixel);
                            }
                        }

                        repeatedSixel = sixel;
                        sixelRepetition = 1;
                    }
                }

                // Is repeated sixel the base sixel?
                //  That means it's empty and we don't have to write it
                if (repeatedSixel != SixelBase)
                {
                    if (sixelRepetition > 3) 
                    {
                        // Sixel repetition more than 3. Then it makes sense to use
                        //  the run-length encoding
                        builder
                            .Append($"!{sixelRepetition}")
                            .Append((char)repeatedSixel);
                    } 
                    else 
                    {
                        // Less than 3, then we just repeat the sixel
                        for(var i = 0; i < sixelRepetition; ++i) 
                        {
                            builder.Append((char)repeatedSixel);
                        }
                    }
                }

                // Go back to start of line for more sixels
                builder
                    .Append('$')
                    ;
            }
            // This row is completed, goto next one
            builder
                .Append('-')
                ;
        }

    }

    var after = clock.ElapsedMilliseconds;
    var elapsed = after-before;
    if (elapsed < sleepFor)
    {
        // Trying to maintain 60fps
        Thread.Sleep((int)(sleepFor - elapsed));
    }
}


record RGB(byte Index, byte Red, byte Blue, byte Green);
static class Extensions
{
    public static byte ToSixelColorComponent(this byte c)
    {
        return (byte)Math.Round(c*100.0/255);
    }
}
