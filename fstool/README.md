# 🎄💾🎄 Writing tools in F#🎄💾🎄

🎅 Ho, ho, ho! Merry Christmas, tool hackers! 🎅

## 🕹️📼🖲️ F# Tools? What? I thought this was about shaders? 🖲️📼🕹️

In order to pad out the empty days in Christmas Advent blog I need to give myself a bit of freedom on the topics lest I run out of ideas.

However, tools in .NET is a bit related to shaders as I write tools that help me out with shader development such [FsDistanceField](https://www.nuget.org/packages/FsDistanceField) that convers images into a distance field.

So I thought I could share a bit how to create tool in .NET that lets your users install them easily to their git repos or command line.

The reason I write tools in F# is that I do enjoy F# the most out of the .NET languages and when I write non-work code I pick the tool that entertain me most.

## Tip 1: SixLabors.ImageSharp

Often in a shader related tool there is a new to load and process images and [SixLabors.ImageSharp](https://github.com/SixLabors/ImageSharp) is a great library to do that. It's also works on non-Windows which `System.Drawing` don't do.

[SixLabors.ImageSharp](https://github.com/SixLabors/ImageSharp) has a [license](https://github.com/SixLabors/ImageSharp/blob/main/LICENSE) that let's open source projects use it in under [Apache License v2.0](https://www.apache.org/licenses/LICENSE-2.0.html).

There are a bunch of other libraries under SixLabors that are very useful in addition to Image share such as [SixLabors.Fonts](https://github.com/SixLabors/Fonts).

Loading an image in ImageSharp is easy:

```fsharp
use image = Image.Load<Rgba32> fullInputPath
```

Using built-in mutators you can do alot alterations to the image such as resizing.

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

You can also access the bits of an image using `ProcessPixelRows`:

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

A very useful library.

## Tip 2: System.CommandLine to parse command line

For a command line tool you need to parse the command line provided by the user. For simple command lines you might get away with switching over the input but it quickly grows beyond what a switch statement can manage.

Microsoft is working on a quite competent library called [System.CommandLine](https://www.nuget.org/packages/System.CommandLine) that I used lately. While it is a bit quirky to use this library the first times once I got used to it I appreciate that it is competent.

I created a small example program demonstrating how to use this lib:

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

This gives us a command line interface that supports multiple commands, different options per command, different aliases per option and the ability to give the user some help if `-h` is passed.

```bash
# Prints the help
dotnet run -- -h

# Prints the README (this is a subcommand)
dotnet run -- readme

# Halts as -v is not defined for readme
dotnet run -- readme -v


```

## Tip 3: Preparing your tool for packaging

In order to package your tool for publification on nuget you need to provide some information in the project file.

This is the project file for my tool `fsimg2sixel` which is a tool to convert images into sixel format (It's a very useless, but fun tool):

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

Using a project like this you can upload the tool to nuget. The first step is building the package using

```
# Build version 0.0.1
dotnet pack /p:Version=0.0.1
```

This creates a package file for you under `nupkg` folder. This package you can then upload to nuget to make the package downloadable by others.

Note; after uploading the package it can take a few moments before you can install the tool locally. For example here's how to install the image to sixel tool:

```bash
# Installs the sixel tool globally.
dotnet tool install --global FsImageToSixel.Tool
```

You can also a tool in git repository:

```bash
# Create a tool manifest (only needed once in each repo)
dotnet new tool-manifest
# Install the tool into the repo
dotnet tool install FsImageToSixel.Tool
# Whenever you need to access the tool in a build script you need to restore it
dotnet tool restore
# And then you can execute the local tool
dotnet tool run fsimg2sixel
```

Very useful.




🎄🌟🎄 Merry Christmas to all, and happy retro-tastic coding! 🎄🌟🎄

🎅 – mrange

## ❄️Licensing Information❄️

All code content I created for this blog post, including the linked KodeLife sample code, is licensed under [CC0](https://creativecommons.org/public-domain/cc0/) (effectively public domain). Any code snippets from other developers retain their original licenses.

The text content of this blog is licensed under [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) (the same license as Stack Overflow).
