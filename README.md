# SameTypeClangPlugin — 自定义检查规范的 Clang 插件

这是一个实现了自定义检查规范的 Clang 插件，插件的实现方法请查看：[Clang 之旅--实现一个自定义检查规范的 Clang 插件](https://www.jianshu.com/p/c27b77f70616)



### 效果演示

![演示效果](https://upload-images.jianshu.io/upload_images/698554-c7aa746724799734.GIF?imageMogr2/auto-orient/strip)

最终实现了：在编译阶段检查某个方法的参数与返回值的类型相同，如果类型不一致的话能抛出编译错误的提示，同时还给出了对应的修改方法（FixIt），点击修改就能改成正确的参数类型 🎉🎉🎉



### 插件使用方法

1. 从[这里](https://github.com/VernonVan/SameTypeClangPlugin/releases/download/1.0/SameTypePlugin.dylib)下载我已经编译好的 Clang 插件 — SameTypeClangPlugin.dylib
2. 按照[这个教程](https://www.jianshu.com/p/e3f46d42643b)编译一份 Clang 工程，同时按照教程所示方法将 SameTypeClangPlugin.dylib 插件挂载到 Xcode 中，本 Demo 工程可以用来编译查看该插件的使用效果

> 生成插件所用到的文件在工程目录 SameTypeClangPlugin -> ClangPlugin 中：CMakeLists.txt 和 SameTypePlugin.cpp。

