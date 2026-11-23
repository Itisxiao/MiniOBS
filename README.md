# MiniOBS

MiniOBS 是一个使用 C++ 和 Qt 6 构建的 macOS 桌面应用原型，展示了直播软件常见的工作区布局，包括场景、来源、视频预览、音频混音器和控制面板。

> 当前版本仅实现界面原型。采集、直播、录制以及各项控制按钮尚未接入实际功能。

## 开发环境

- macOS
- CMake 3.21 或更高版本
- Qt 6 Widgets
- 支持 C++17 的编译器
- Xcode Command Line Tools

可使用 Homebrew 安装 Qt 6、CMake 和 Ninja：

```bash
brew install qt cmake ninja
```

## 构建与运行

在项目根目录执行：

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build
open build/MiniOBS.app
```

如果 Qt 6 已在 CMake 默认搜索路径中，也可以省略 `CMAKE_PREFIX_PATH` 参数。

## 项目结构

```text
.
├── app/
│   ├── MainWindow.cpp
│   └── MainWindow.h
├── main.cpp
└── CMakeLists.txt
```

- `main.cpp`：应用程序入口。
- `app/MainWindow.*`：主窗口和界面布局。
- `CMakeLists.txt`：CMake 构建配置及 Qt、macOS 框架依赖。
