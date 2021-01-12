# Using `tensorflow` C API on Windows

[Official guide](https://www.tensorflow.org/install/lang_c)

- Download the `tensorflow` binaries for Windows at https://storage.googleapis.com/tensorflow/libtensorflow/libtensorflow-cpu-windows-x86_64-2.3.1.zip

- It will contains the `include` directory which has all the header files for `tensorflow`.
- There is also the `lib` directory which has `tensorflow.dll` and `tensorflow.lib` files.
- The `tensorflow.lib` is used by the linker to compile our program. We will only need `tensorflow.dll` at runtime.
- We need to configure `Visual Studio`/`MSBuild` to know about the `tensorflow`'s headers and `lib` files.
- In this project, I extract those files to the `dependencies` directory at the solution root directory.
- When I open the solution with Visual Studio, I can Right-Click my `ctensorflow` project (not solution) and choose `Properties` at the very end.
- It will bring up the `Property Pages`.
- I can go to `Configuration Properties` > `VC++ Directories` section.
- In the `General` group, I can append the `dependencies/include` which contains all the `tensorflow` header files, to the `Include Directories` value list - `$(SolutionDir)dependencies\include`. `$(SolutionDir)` is expanded to the solution root directory here.
- Now I also need to append `dependencies\lib` which contains `tensorflow.dll` and `tensorflow.lib`, to the `Library Directories` value list - `$(SolutionDir)dependencies\lib`.
- Now we can add `#include <tensorflow/c/c_api.h>` and use the `tensorflow` C API.
- We need to add `#pragma comment(lib, "tensorflow.lib")` to tell the linker to use `tensorflow.lib` at compile time to resolve `tensorflow` APIs.
- At this point, we can compile our program.

```c
#include <stdio.h>

#include <tensorflow/c/c_api.h>
#pragma comment(lib, "tensorflow.lib")

int main()
{
	printf("Hello from TensorFlow C library version %s\n", TF_Version());
	return 0;
}
```

- However, when we run our program, it will give us an error that it cannot find `tensorflow.dll` and exists. It is because our program doesn't know where is the `tensorflow.dll` file.
- In this project, the compiled program will be put at `x64/Debug` directory while our `tensorflow.dll` is still at `dependencies/lib` directory. `MSBuild` doesn't copy `tensorflow.dll` to `x64/Debug`.
- Now, we need to add a `CustomBuildStep` to tell `MSBuild` to copy `tensorflow.dll` to the output directory. This `CustomBuildStep` should be run `After Build`.
- The command should be `xcopy /y /d "$(SolutionDir)dependencies\lib\*.dll" "$(OutDir)"`. `$(OutDir)` will be expanded to `x64/Debug` in this case. If you have multiple `Build Configurations` and `Target Architectures`, these values will be slightly different.
- `xcopy` in Windows will only override the destination files if the destination files are "outdated". So if you were trying multiple `tensorflow` versions, you can manually delete the `$(OutDir)`.
