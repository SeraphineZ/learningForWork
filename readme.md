# 项目名称

这是一个使用 VTK 进行点云配准和渲染的示例项目。

## 先决条件

- [VTK](https://vtk.org/) 9.0 或更高版本
- [CMake](https://cmake.org/) 3.10 或更高版本
- [Visual Studio](https://visualstudio.microsoft.com/) 2019 或更高版本

## 构建步骤

1. 克隆此仓库到本地：

   ```sh
   git clone https://github.com/yourusername/yourproject.git
   cd yourproject
   ```

2. 创建并进入构建目录：

   ```sh
   mkdir build
   cd build
   ```

3. 运行 CMake 配置项目：

   ```sh
   cmake -G "Visual Studio 16 2019" -A x64 ..
   ```

4. 使用 CMake 构建项目：

   ```sh
   cmake --build . --config Debug
   ```

## 运行程序

构建完成后，可以在 [Debug](http://_vscodecontentref_/0) 目录下找到生成的可执行文件 `forTest.exe`。运行该程序：

```sh
./Debug/forTest.exe
```
