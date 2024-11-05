# 🎄⭐🎉 Rendering Shaders in a Windows App 🎉⭐🎄

🧝🎅🧝 *Merry christmas all size-coders!* 🧝🎅🧝

Last blog I tried to show how to write a minimal Windows App that renders a fragment shader. I mentioned to follow-up I try to show how to make it into a 4KiB executable. I also mentioned there was a BIG problem with Windows App.

## So what's the BIG problem?

The issue with Windows App is that it requires the C-runtime DLL and that is not allowed in a size-coding competition.

We can embed the C-runtime into the executable but then the program ends up around 100KiB, about 96KiB bigger than the 4KiB limit.

## Getting rid of the C-runtime.

Today it's mostly about setting various options in Visual C++ and less so about coding. I have provided an example [here](wgl-app/) that you can follow along in.

In the example project I have created a build configuration called "Release - NOCRT".

In this release config the big change is to remove the C-runtime, which we do by specifying the parameter `/NODEFAULTLIB` to the C++ linker

**TODO: Image of setting**

This has a big limitation, you can't use C-runtime functions which removes obvious things like `printf` but also less obvious things like certain floating point functions (depending on the CPU architecture).

In addition, when we remove the C-runtime the linker will complain about a few things

1. `__fltused` missing symbol
2. ` @__security_check_cookie@4` missing symbol
3. `___security_cookie` missing symbol
4. `_WinMainCRTStartup` missing symbol

`__fltused` is for some reason referenced by the code that Visual C++ compiles but we just define it

These symbols are symbols in the C naming convention as C++ names are "mangled" to encode type info in them. C don't do that. Let's instruct to only use C naming convention by surrounding around program with `extern "C"`.

```c++
// extern "C" makes sure C++ "name mangling" don't change the name
//  of the variable during linking
//  The actual nane will be __fltused
extern "C" {
// All our code
}
```

Then when we declare the missing symbol it gets the right name `__flutused` (C++ prepends an extra `_`).

```c++
int _fltused;
```

The security check related symbols are there because Visual C++ injects code to make the code more secure like checking for buffer overruns. We are size-coders, we care about size, not security. So let's disable that setting by specifying the parameter: `/GS-`

**TODO: Image**

Finally the linker is looking for the method that Windows will call when it starts the process. `main` and `WinMain` are not directly called by Windows, instead they are called by the C-runtime after it's initalized.

Just rename the
```c++
// Function called by Windows
//  The actual name will be _WinMainCRTStartup
int WINAPI WinMainCRTStartup(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
);
```

That should fix the linker issues.

The end result is that we end up around 7KiB which is pretty good but not quite 4KiB.

## There are lots of settings to tinker with...

Visual C++ has tons of settings and you can compare the `Relase` and `Release - NOCRT` to see that I tinkered with alot of them to help reduce overhead. I don't claim it's optimal, just some settings we tinker with and to setup the next step.

The important step is to remove the C-runtime.

## Replacing the default linker with CRINKLER.

One issue with the generated executable is that it included lots of headers and metadata which is nice but needed. As size-coders we like that to all go away.

In addition; it would be nice if the executable was a self-extracting archive in that it contained compressed code and resources and during runtime decompresses it and runs i.

Good news; there's a tool that does that and it's called [CRINKLER](https://github.com/runestubbe/Crinkler).

First step is to download the linker and put it into project directory (I already done this for you, it's called `link.exe`). Then we setup that Visual C++ should search our project dir for executables, because it's called `link.exe` it will find it.

The default behavior of CRINKLER is to just default to the normal linker but if we specify the `/CRINKLER` link parameter it switches into CRINKLER mode.

In the example project I use the command line options:

```
/CRINKLER /TINYIMPORT /NOINITIALIZERS /UNSAFEIMPORT /PROGRESSGUI /HASHTRIES:20 /COMPMODE:fast /ORDERTRIES:1000 /REPORT:REPORT.html  /RANGE:opengl32
```

1. `/CRINKLER` - This simply specifies that you're using the CRINKLER linker.

2. `/TINYIMPORT` - This option tells CRINKLER to use a more compact import table format, reducing the final executable size.

3. `/NOINITIALIZERS` - This disables the automatic generation of global variable initializers, again to reduce executable size.

4. `/UNSAFEIMPORT` - This allows CRINKLER to be more aggressive in reducing the import table, though it may introduce compatibility issues in some cases.

5. `/PROGRESSGUI` - This enables a graphical progress bar during the linking process.

6. `/HASHTRIES:20` - This sets the number of hash table probing attempts to 20, which can help improve linking performance.

7. `/COMPMODE:fast` - This sets the compression mode to "fast", prioritizing speed over maximum compression.

8. `/ORDERTRIES:1000` - This sets the number of ordering attempts to 1000, which can help CRINKLER find a more optimal order for functions and data.

9. `/REPORT:REPORT.html` - This generates an HTML report file with details about the linking process and final executable.

10. `/RANGE:opengl32` - This tells CRINKLER to include the `opengl32.dll` library in the final executable, ensuring any OpenGL-related functionality is available.

There are tons of options and ways to tweak CRINKLER that you can find in the [manual](https://github.com/runestubbe/Crinkler/blob/master/doc/manual.txt).

I have setup a build configuration called `Release - CRINKLER` that let's you compile and link with CRINKLER.

If you switch to `Release - CRINKLER` and you compile the executable you will see that the final execuable is less than 2KiB which is pretty cool.

Crinkler produces an HTML file called `REPORT.html` that lets you see what takes up space in your program. This is helpful when chasing bytes.

There's a chance `Windows Defender` or other anti-virus software don't like the executable (it lacks headers and info) so I often add the build folder as safe folder it doesn't have to scan.

Crinkling can take a lot of time if you are building bigger demos and does heavier compression, so crinkler shows a little progress bar. For the demos I have done it usually takes about 3 min to link.

## Further improvements

There's lot of things one can improve. For example the shader code is too chatty. There's an excellent shader minifier one can use to remove comments and rewrite the shader code to smaller code. This tool is called shader-minifier and [can be found on github](https://github.com/laurentlb/shader-minifier).

The shader in our example is about 2KiB uncompressed but if you pass it to shader-minifier it should be able to push it down to below 1KiB.

Another important aspect for demo is music and for size-coding you can't drop in an mp3. However, there are cool tools like [4klang](https://github.com/gopher-atz/4klang) and [sointu](https://github.com/vsariola/sointu) that lets you compose music and save it as assembler code that you can assemble and link into your demo.



So with that I hope I got you enough of information to get started! Size-coding is great fun and sometimes frustrating when you chase 5 bytes for hours. But when you finally get your software below 4KiB (or 1KiB!) limit it's an amazing feeling.



Happy coding, and I can’t wait to see what you create!

🎄🌟🎄 Merry Christmas to all, and happy coding! 🎄🌟🎄

🎅 – mrange

## ❄️Licensing Information❄️

All code content I created for this blog post is licensed under [CC0](https://creativecommons.org/public-domain/cc0/) (effectively public domain). Any code snippets from other developers retain their original licenses.

The text content of this blog is licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) (the same license as Stack Overflow).