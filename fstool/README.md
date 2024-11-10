# 🎄⚙️🎄 Writing Shader Tools in F# 🎄⚙️🎄

🎅 Ho, ho, ho! Merry Christmas, tool hackers! 🎅

## 🔧🔩🪛 "F# Tools? I Thought This Was About Shaders!" 🪛🔩🔧

Yes, this blog series is *mostly* about shaders, but I needed to fill in the gaps since I couldn’t find contributors for every day! Tools in .NET tie in nicely, too, especially since I use them to help out with shader-related work. For example, I’ve built tools like [FsDistanceField](https://www.nuget.org/packages/FsDistanceField), which converts images into distance fields—very useful for shader projects.

Creating command-line tools in .NET can make your work easier to share and simpler to integrate, whether it’s in a Git repo or on a colleague's terminal. Why F#? When I'm off the clock, I turn to F# because, quite simply, it's just fun for me!

## 🎨🖌️ Tip 1: SixLabors.ImageSharp 🖌️🎨

If your tool needs to load and process images (a common shader-related task!), [SixLabors.ImageSharp](https://github.com/SixLabors/ImageSharp) is the go-to library. It works seamlessly across platforms—unlike `System.Drawing`, which is Windows-only.

Good news: ImageSharp is open source and friendly for open-source projects under the [Apache License v2.0](https://www.apache.org/licenses/LICENSE-2.0.html). SixLabors also offers [SixLabors.Fonts](https://github.com/SixLabors/Fonts) and other useful libraries if you need more than images!

Loading an image with ImageSharp is easy:
```fsharp
use image = Image.Load<Rgba32> fullInputPath
```

With built-in mutators, you can make a variety of changes to the image, like resizing.
```fsharp
let mutator (ctx : IImageProcessingContext) =
  let options = ResizeOptions (
      Mode    = ResizeMode.Stretch
    , Sampler = KnownResamplers.Hermite
    , Size    = Size(int desiredWidth, int desiredHeight)
    )
  ignore <| ctx.Resize options
image.Mutate mutator
```

You can also access the image’s pixel data using `ProcessPixelRows`:
```fsharp
let pa =
  PixelAccessorAction<Rgba32> (
    fun a ->
      for y = 0 to a.Height - 1 do
        let row = a.GetRowSpan y
        for x = 0 to a.Width - 1 do
          let pixel = row.[x]
          // Do stuff with the pixel
          ()

  )
image.ProcessPixelRows pa
```

ImageSharp is amazing for developers who want to build tools for processing images.

## ⌨️ Tip 2: Using System.CommandLine for Parsing Input ⌨️

If you're building a command-line tool, you’ll need a way to parse user input. For basic tasks, a simple switch statement might work, but things can quickly get out of hand.

Enter [System.CommandLine](https://www.nuget.org/packages/System.CommandLine), a capable library Microsoft has been developing. Although it can feel a bit quirky at first, once you get used to it, you'll appreciate its flexibility and power.

Here’s a small example program I created to show how to use this library:
```fsharp
open System.CommandLine
open System.CommandLine.Invocation

// Define command-line options. Options can be specified in different ways:
// --input file.txt  or  -i file.txt
let inputOption =
  Option<string>(
      // aliases: Array of option names. Convention is to have a short (-i)
      // and long form (--input)
      aliases         = [|"-i"; "--input"|]
    , description     = "Input path"
      // IsRequired = true means the command will fail if this option is not provided
    , IsRequired      = true
    )

// Boolean options (flags) are special - they don't need a value:
// --verbose or -v is enough to set them to true
let verboseOption =
  Option<bool>(
      aliases         = [|"-v"; "--verbose"|]
    , description     = "Verbose logging"
      // Default value if the flag is not specified
    , getDefaultValue = fun () -> false
    )

// Handler for the 'readme' subcommand
// InvocationContext provides access to parsed values and exit code
let readmeCommandHandler
  (ctx            : InvocationContext )
  : unit =
  printf "This is the README"
  // Set exit code to indicate success (0) or failure (non-zero)
  ctx.ExitCode <- 0

// Handler for the root command
// This runs when no subcommand is specified
let rootCommandHandler
  (ctx            : InvocationContext )
  : unit =
  // Helper to make option value access more concise
  let inline getValue option = ctx.ParseResult.GetValueForOption option

  // Get the parsed values for each option
  let input           = getValue inputOption
  let verbose         = getValue verboseOption

  printfn "Root command"
  printfn " Input  : %s" input
  printfn " Verbose: %A" verbose
  ctx.ExitCode <- 0

[<EntryPoint>]
let main args =
  // Create the root command - this is the main entry point
  // The description shows up in help text
  let rootCommand = RootCommand "mytool - Example tool"

  // Add all options to the root command
  // These will be available to both the root command and subcommands
  ([|
      inputOption
      verboseOption
    |] : Option array)
    |> Array.iter rootCommand.AddOption

  // Set the handler for when the root command is invoked
  rootCommand.SetHandler rootCommandHandler

  // Create a subcommand named 'readme'
  // Usage: mytool readme
  let readmeCommand = Command ("readme", "Displays the README")
  readmeCommand.SetHandler readmeCommandHandler
  rootCommand.AddCommand readmeCommand

  // Parse command line args and run the appropriate handler
  // Returns the exit code set by the handler
  rootCommand.Invoke args
```

With this setup, our command-line interface can handle multiple commands, custom options per command, and even alternative aliases for each option. Plus, it offers built-in help when users pass `-h`.

```bash
# Shows help for available commands
dotnet run -- -h

# Executes the "readme" subcommand
dotnet run -- readme

# Throws an error, as "-v" isn’t defined for "readme"
dotnet run -- readme -v

# Runs the root command with the "-i" option
dotnet run -- -i print.txt

# Same as above but using long-form aliases for options
dotnet run -- --input print.txt --verbose
```

I’ve come to prefer the more verbose `CommandHandler` approach, like this:

```fsharp
let rootCommandHandler
  (ctx : InvocationContext)
  : unit =
  ...
```
While there are simpler overloads available, they limit you to a maximum of 10 arguments and don’t provide control over the command’s exit code.

While I find [System.CommandLine](https://www.nuget.org/packages/System.CommandLine) a bit clunky, it’s incredibly useful for parsing command-line inputs.


## 📦🚚 Tip 3: Preparing Your Tool for Packaging 🚚📦

Once your tool is ready, you’ll want to package it for distribution on NuGet. To do this, you'll need to add some metadata to your project file.

Here’s an example project file for my tool, `fsimg2sixel`, which converts images into the quirky Sixel format (not essential, but certainly fun):

```xml
<Project Sdk="Microsoft.NET.Sdk">

  <PropertyGroup>
    <AssemblyName>fsimg2sixel</AssemblyName>
    <OutputType>Exe</OutputType>
    <TargetFramework>net9.0</TargetFramework>
    <TreatWarningsAsErrors>true</TreatWarningsAsErrors>
    <RestorePackagesWithLockFile>true</RestorePackagesWithLockFile>

    <!-- Package metadata for NuGet -->
    <Copyright>Copyright (c) Mårten Rånge</Copyright>
    <Authors>Mårten Rånge</Authors>
    <!-- Specify the license using SPDX identifier -->
    <PackageLicenseExpression>MIT</PackageLicenseExpression>
    <!-- Source code repository information -->
    <RepositoryUrl>https://github.com/mrange/FsImageToSixel</RepositoryUrl>
    <RepositoryType>git</RepositoryType>

    <!-- Package display and description information -->
    <Title>Tool for converting standard images into Sixel format</Title>
    <Description>
      A .NET tool for converting standard images into Sixel format, which produces multicolored, pixelated images displayable in terminals that support it (e.g., Windows Terminal 1.22+).
    </Description>
    <!-- Command name when installed as a global tool -->
    <ToolCommandName>fsimg2sixel</ToolCommandName>

    <!-- NuGet package specific configurations -->
    <!-- Search tags for NuGet package discovery -->
    <PackageTags>Tool;Image;Sixel</PackageTags>
    <!-- Custom output directory for the generated NuGet package -->
    <PackageOutputPath>./nupkg</PackageOutputPath>
    <!-- Path to the icon file within the package -->
    <PackageIcon>assets/icon.png</PackageIcon>
    <!-- Package this project as a .NET tool that can be installed using dotnet tool install -->
    <PackAsTool>true</PackAsTool>
    <!-- The package identifier used on NuGet.org -->
    <PackageId>FsImageToSixel.Tool</PackageId>
    <!-- Include README.md as the package description -->
    <PackageReadmeFile>README.md</PackageReadmeFile>
  </PropertyGroup>

  <!-- Include additional files in the NuGet package -->
  <ItemGroup>
    <!-- Pack=true indicates the file should be included in the NuGet package -->
    <!-- PackagePath specifies the location within the package -->
    <!-- CopyToOutputDirectory ensures the files are copied to the build output -->
    <None Include="../../LICENSE" Pack="true" PackagePath="\">
      <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>
    </None>
    <None Include="../../NOTICE" Pack="true" PackagePath="\">
      <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>
    </None>
    <!-- Include the icon file in the assets directory of the package -->
    <None Include="../../assets/icon.png" Pack="true" PackagePath="assets" />
    <None Include="README.md" Pack="true" PackagePath="\">
      <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>
    </None>
  </ItemGroup>

  <ItemGroup>
    <Compile Include="Program.fs" />
  </ItemGroup>

  <ItemGroup />

  <ItemGroup>
    <PackageReference Include="SixLabors.ImageSharp" Version="3.1.5" />
    <PackageReference Include="System.CommandLine" Version="2.0.0-beta4.22272.1" />
  </ItemGroup>

</Project>
```

In this project file, we specify the necessary package metadata such as the package name, description, licensing, and even the README file. You can also include extra files, like an icon or a license file, to make your package more complete.

### Building and Publishing the Package

Once the project file is ready, it’s time to build the NuGet package. Run the following command to pack the tool:

```
# Build version 0.0.1
dotnet pack /p:Version=0.0.1
```

This generates the `.nupkg` file in the `nupkg` folder. You can then upload this package to NuGet, making it available for others to download and use.

### Installing Your Tool

After uploading, it may take a few moments for the package to appear on NuGet. Once it’s available, you can install your tool globally with:

```bash
# Install the sixel tool globally
dotnet tool install --global FsImageToSixel.Tool
```

Alternatively, you can install the tool directly into a Git repository. This is especially handy for project-specific tools:

```bash
# Create a tool manifest (only needed once per repo)
dotnet new tool-manifest
# Install the tool into the repo
dotnet tool install FsImageToSixel.Tool
# Restore the tool in your build script
dotnet tool restore
# Run the tool locally
dotnet tool run fsimg2sixel
```

This method ensures you can easily access the tool locally within your repository or during build processes. It's a great way to manage tools in specific environments without polluting the global toolset.

Very useful indeed!

## 🎁 And That’s a Wrap! 🎁

While this blog post wasn’t about shaders, I hope it sparked some ideas for building and deploying your own .NET tools to NuGet. Whether you're handling images with the powerful [SixLabors.ImageSharp](https://github.com/SixLabors/ImageSharp) or mastering command-line interfaces with [System.CommandLine](https://www.nuget.org/packages/System.CommandLine), I’m sure these tools can make your coding journey smoother and more fun.

🎄🌟🎄 Merry Christmas to all, and may your code be as joyful as a holiday lights display! 🎄🌟🎄

🎅 – mrange

## ❄️Licensing Information❄️

All code content I created for this blog post, including the linked KodeLife sample code, is licensed under [CC0](https://creativecommons.org/public-domain/cc0/) (effectively public domain). Any code snippets from other developers retain their original licenses.

The text content of this blog is licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) (the same license as Stack Overflow).
