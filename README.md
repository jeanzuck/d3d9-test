# d3d9-test

A minimal Direct3D 9 demo project in C++14 that displays a rotating colored 3D cube and overlays real-time information using D3DXFont. The project is designed for Visual Studio and demonstrates basic D3D9 device setup, vertex/index buffer usage, transformation matrices, and text rendering.

## Features

- Rotating 3D colored cube using vertex and index buffers
- Real-time FPS display at the top right
- Shows Direct3D device pointer and vtable addresses for key methods
- Displays the stride (size) of the cube's vertex structure
- Handles window resizing for correct 3D aspect ratio (box only)
- Simple Win32 message loop and window management

## Requirements

- Windows with DirectX 9.0c runtime
- Visual Studio (any recent version)
- C++14 compatible compiler
- **DirectX SDK (June 2010)**
- Supports both **x86** and **x64** builds

## DirectX SDK (June 2010) Notes

- This project requires the [DirectX SDK (June 2010)](https://www.microsoft.com/en-us/download/details.aspx?id=6812) for headers and libraries such as `d3d9.h` and `d3dx9.h`.
- After installing the SDK, ensure the include and lib directories are added to your Visual Studio project settings:
  - __Project > Properties > VC++ Directories > Include Directories__
  - __Project > Properties > VC++ Directories > Library Directories__
- The D3DX utility library (D3DX9) is deprecated in later Windows SDKs and is only available in the June 2010 SDK.

## Build & Run

1. Install the DirectX SDK (June 2010).
2. Open the solution/project in Visual Studio.
3. Select your desired platform: **x86** or **x64** (from the toolbar or __Build > Configuration Manager__).
4. Build the project (F7 or Build Solution).
5. Run the resulting `.exe` (either from Visual Studio or the output directory).

## Notes

- The `.gitignore` is set up to exclude all build artifacts except `.exe` files.
- Text overlays use fixed pixel rectangles and do not scale with window size.
- The project is intended for educational and demonstration purposes.

## License

This project is provided as-is for learning and demonstration. No specific license.