# 构建与运行

## 开发时：用 Qt Creator 跑

打开 `BC_LOGO.pro`，选 **Desktop Qt 6.7.3 MinGW 64-bit** kit，直接 Run。
Qt Creator 会把 `C:\Qt\6.7.3\mingw_64\bin` 放进运行环境，平台插件从 Qt 安装目录加载，不需要额外部署。

## 直接运行 exe：必须先部署

```powershell
C:\Qt\6.7.3\mingw_64\bin\windeployqt.exe --no-translations `
    path_to_exe\BC_LOGO.exe
```

会生成：

```
path_to_exe\
  platforms\qwindows.dll
  styles\qmodernwindowsstyle.dll
  imageformats\ ...
  iconengines\ ...
  tls\ ...
```

之后双击 exe 即可启动。

无头环境（CI / 语料回归）额外需要 offscreen 插件：

```powershell
Copy-Item C:\Qt\6.7.3\mingw_64\plugins\platforms\qoffscreen.dll `
    build\Desktop_Qt_6_7_3_MinGW_64_bit-Debug\debug\platforms\
```

### 重新构建之后

普通 `make` / Qt Creator 的增量构建不会删掉已部署的目录，不用每次重新部署。
如果删除了整个 `build\` 目录，重新编译后要再跑一次 `windeployqt`。

## 无头脚本模式

部署好之后（或者把 Qt 的 `bin` 放进 `PATH`），可以直接跑脚本、不显示窗口：

```powershell
BC_LOGO.exe --script tests\samples\double.lgo --out out.txt -platform offscreen
```

脚本走 2026-11-18 接入的新引擎，语法是原版 Logo 风格（`:X` 变量、`"X` 字面量、
`MAKE "X 5`），`--script` 会一次解析整份文件，跨行的 `TO…END` 也能处理。

`--out` 写出的报告包含终态坐标、画笔/颜色/背景和画布像素 SHA-1，详见
[../tests/corpus/README.md](../tests/corpus/README.md)。

`tests\run_corpus.ps1` 启动前会检查 `<exe>\platforms`：
缺失时立即报错并提示 `windeployqt`，不会卡在 Qt 的模态错误框上。

## 常见错误对照

| 现象 | 原因 | 处理 |
|---|---|---|
| `no Qt platform plugin could be initialized` | exe 旁缺 `platforms\qwindows.dll` | `windeployqt` |
| `Qt6Core.dll 找不到` | exe 旁没有 Qt DLL，`PATH` 里也没有 | `windeployqt` 或把 Qt `bin` 加进 `PATH` |
| 进程卡住不退出、无窗口 | Qt 弹了模态错误框 | 同上，先部署；`run_corpus.ps1` 已超时保护 |
