# Saleae Analyzer SDK Sample Analyzer

- [Saleae Analyzer SDK Sample Analyzer](#saleae-analyzer-sdk-sample-analyzer)
  * [Renaming your Analyzer](#renaming-your-analyzer)
  * [Cloud Building & Publishing](#cloud-building---publishing)
  * [Prerequisites](#prerequisites)
    + [Windows](#windows)
    + [MacOS](#macos)
    + [Linux](#linux)
  * [Building your Analyzer](#building-your-analyzer)
    + [Windows](#windows-1)
    + [MacOS](#macos-1)
    + [Linux](#linux-1)
  * [Debugging](#debugging)
    + [Windows](#windows-2)
    + [MacOS](#macos-2)
    + [Linux](#linux-2)

The Saleae Analyzer SDK is used to create custom plugins for the Saleae Logic software. These plugins are used to decode protocol data from captured waveforms.

To build your own protocol decoder plugin, first fork, clone, or download this repository.

Then, make sure you have the required software installed for development. See the [Prerequisites](#Prerequisites) section below for details.

## Renaming your Analyzer

Once downloaded, first run the script rename_analyzer.py. This script is used to rename the sample analyzer automatically. Specifically, it changes the class names in the source code, it changes the text name that will be displayed once the custom analyzer has been loaded into the Saleae Logic software, and it updates the visual studio project.

There are two names you need to provide to rename_analyzer. The first is the class name. For instance, if you are developing a SPI analyzer, the class names would be SPIAnalyzer, SPIAnalyzerResults, SPIAnalyzerSettings, etc.
The file names would be similar, like SPIAnalyzer.cpp, etc.

All analyzer classes should end with "Analyzer," so the rename script will add that for you. In the first prompt after starting the script, enter "SPI". The analyzer suffix will be added for you. This needs to be a valid C++ class name - no spaces, it can't start with a number, etc.

Second, the script will prompt you for the display name. This will appear in the software in the list of analyzers after the plugin has loaded. This string can have spaces, since it will always be treated as a string, and not as the name of a class.

After that, the script will complete the renaming process and exit.

    python rename_analyzer.py
    SPI
    Mark's SPI Analyzer

Once renamed, you're ready to build your analyzer! See the [Building your Analyzer](#Building-your-Analyzer) section below.

API documentation can be found in `./docs/Analyzer API.md`.

## Cloud Building & Publishing

This example repository includes support for GitHub actions, which is a continuous integration service from GitHub. The file located at `.github\workflows\build.yml` contains the configuration.

When building in CI, the release version of the analyzer is built for Windows, Linux, and MacOS. The built analyzer files are available for every CI build. Additionally, GitHub releases are automatically created for any tagged commits, making it easy to share pre-built binaries with others once your analyzer is complete.

## Prerequisites

### Windows

Dependencies:
- Visual Studio 2017 (or newer) with C++
- CMake 3.13+

**Visual Studio 2017**

*Note - newer versions of Visual Studio should be fine.*

Setup options:
- Programming Languages > Visual C++ > select all sub-components.

Note - if CMake has any problems with the MSVC compiler, it's likely a component is missing.

**CMake**

Download and install the latest CMake release here.
https://cmake.org/download/

### MacOS

Dependencies:
- XCode with command line tools
- CMake 3.13+

Installing command line tools after XCode is installed:
```
xcode-select --install
```

Then open XCode, open Preferences from the main menu, go to locations, and select the only option under 'Command line tools'.

Installing CMake on MacOS:

1. Download the binary distribution for MacOS, `cmake-*-Darwin-x86_64.dmg`
2. Install the usual way by dragging into applications.
3. Open a terminal and run the following:
```
/Applications/CMake.app/Contents/bin/cmake-gui --install
```
*Note: Errors may occur if older versions of CMake are installed.*
### Linux

Dependencies:
- CMake 3.13+
- gcc 5+

Misc dependencies:

```
sudo apt-get install build-essential
```
## Building your Analyzer

### Windows

```bat
mkdir build
cd build
cmake .. -A x64
cmake --build .
:: built analyzer will be located at SampleAnalyzer\build\Analyzers\Debug\SimpleSerialAnalyzer.dll
```

### MacOS

```bash
mkdir build
cd build
cmake ..
cmake --build .
# built analyzer will be located at SampleAnalyzer/build/Analyzers/libSimpleSerialAnalyzer.so
```

### Linux

```bash
mkdir build
cd build
cmake ..
cmake --build .
# built analyzer will be located at SampleAnalyzer/build/Analyzers/libSimpleSerialAnalyzer.so
```

## Debugging

Although the exact debugging process varies slightly from platform to platform, part of the process is the same for all platforms.

First, build your analyzer. Then, in the Logic 2 software, load your custom analyzer, and then restart the software. Instructions can be found here: https://support.saleae.com/faq/technical-faq/setting-up-developer-directory

Once restarted, the software should show your custom analyzer in the list of available analyzers.

Next, in order to attach your debugger, you will need to find the process ID of the Logic 2 software. To make this easy, we display the process ID of the correct process in the About dialog in the software, which you can open from the main menu. It's the last item in the "Build Info" box, labeled "PID". Note that this is not the correct PID when using an ARM based M1 Mac. (Please contact support for details on debugging on M1 Macs.)

You will need that PID number for the platform specific steps below.

Note, ee strongly recommend only debugging your analyzer on existing captures, and not while making new recordings. The act of pausing the application with the debugger while recording data will cause the recording to fail once the application is resumed. To make development smooth, we recommend saving the capture you wish to debug with before starting the debugging process, so you can easily re-load it later.

### Windows

when `cmake .. -A x64` was run, a Visual Studio solution file was created automatically in the build directory. To debug your analyzer, first open that solution in visual studio.

Then, open the Debug menu, and select "attach to process...".

Enter the PID number into the Filter box to find the correct instance of Logic.exe.

Click attach.

Next, place a breakpoint somewhere in your analyzer source code. For example, the start of the WorkerThread function.

Make sure you already have recorded data in the application, and then add an instance of your analyzer. The debugger should pause at the breakpoint.

### MacOS

It is possible to debug using xcode, by generating an xcode project using CMake. To Generate a CMake project, either create a new directory or delete the existing build directory, and from that directory run:

```bash
cmake .. -G Xcode
```

This will generate an xcode project. The process for debugging requires you to set a breakpoint in your code, then attach the Xcode debugger to the Logic process, using the PID mentioned above.

Alternatively, gdb / lldb can be used, using the instructions found in the Linux section below, but substituting gdb with lldb when necessary.

### Linux

(Note, this section needs to be tested and updated if needed)

On Linux, you can debug your custom analyzer using GDB. This can be done from the console, however we recommend using a GUI tool like Visual Studio Code, with the C++ extension installed.

To debug from the command line, once you have loaded your analyzer into the logic software and have checked the process ID, you can attach gdb like so:

```bash
gdb
attach <pid>
```

And then test setting a breakpoint like this:
```
break MyAnalyzer::WorkerThread
```
Because your analyzer hasn't been loaded yet, GDB will notify you that it can't find this function, and ask if you want to automatically set this breakpoint if a library with a matching function is loaded in the future.  Type y <enter>

Then return to the application and add your analyzer. This should trigger the breakpoint.
