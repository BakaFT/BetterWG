# BetterWG

通过 [黑魔法](./HOW_IT_WORKS.md) 让你的 WeGame 显示 团队ELO（TeamELO）数据

![](https://raw.githubusercontent.com/BakaFT/hexo-blog-img/main/img/2024/07/11teamelo.png)

# 使用方法

## 从Release下载

1. 点击 [这里](https://github.com/BakaFT/BetterWG/releases/latest) 下载 `dwrite.dll`
2. 首先找到 WeGame 的目录，在其中找到 `qbblinktrial` 文件夹，把`dwrite.dll`放进去即可
3. 如果不想用了，删掉`dwrite.dll`即可

## 自行编译

1. Clone本项目

2. 使用 Visual Studio 2019 或以上版本打开项目

   （项目使用VC143，C++17标准）

3. 编译`payload`项目

4. 将编译产物重命名为`dwrite.dll`，并且放置在 WeGame 目录下的 `qbblinktrial`文件夹下

   （用`mklink`做一个软连接过去也行，调试起来更方便）

# TODO

画个饼

- [ ] 无视生涯隐藏
- [ ] 修正可禁用模式页面中的样式冲突

# Credits

:heart:

[PenguLoader/PenguLoader: ✨ The ultimate JavaScript plugin loader, build your unmatched LoL Client](https://github.com/PenguLoader/PenguLoader/)

[nbqofficial/divert: Detour library (x64 and x86 compatible)](https://github.com/nbqofficial/divert)
