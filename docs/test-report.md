# Easy Delay — 验证报告

日期：2026-11-18
构建：Release x64，MSVC 19.44，JUCE 9.0.5
被测文件：`build/EasyDelay_artefacts/Release/VST3/EasyDelay.vst3`
测法：`EasyDelayCheck` 用 JUCE 自带的 VST3 宿主代码加载 VST3（`juce::VST3PluginFormat` + `AudioPluginFormatManager`），按宿主的方式 prepare、分块 render、读参数、创建编辑器。不是“跑起来没崩”，而是断言了具体行为。

## 结果

```
[PASS] reports 0 samples of latency (zero PDC)
[PASS] mix=0 passes audio through (peak 0.5000)
[PASS] audio comes back 642.7 ms late while the host sees 0 latency
[PASS] editor created (520x280)
```

关键读数的解释：

- **0 延迟**：`AudioPluginInstance::getLatencySamples()` 返回 0。这就是“不参与 DAW 延迟补偿”的实测依据。
- **干声直通**：Mix=0 时输出峰值与输入一致（0.5），说明插件没有偷偷静音或染色。
- **内部确实延迟**：0.5 s 静音后放一个脉冲，100% 湿声；干脉冲在 sample 24000，延迟副本在 sample 54850，相差 **642.7 ms**，与参数读数 `643 ms` 一致。宿主全程以为延迟是 0。
- **编辑器**：宿主能创建 520×280 的编辑器，没有崩溃或断言。

## 本次修掉的 bug（值得记一笔）

第一版 DSP 把“干声”读成了延迟线当前写指针的位置，而延迟线在本拍还没写入，结果全是 0：
Mix=0 时整轨静音，测试直接抓到（`outPeak=0`）。改成从宿主输入 buffer 读干声后通过。
如果没有这个自动检查，这个 bug 只有在 Studio One 里听到“没声音”才会被发现。

## 没测到的部分（如实说明）

- 没有装 Studio One 实机加载（本机没有装 DAW，且沙箱不能写 Program Files）。实机加载步骤见 README。
- 没有用 Steinberg 官方 VST3 validator 跑全套校验，只用了 JUCE 的宿主侧代码。
- 没有测高采样率（96/192 kHz）和单声道布局；代码里支持 mono/stereo，但未逐项验证。
- 没有测 macOS。

## 复现

```powershell
$cmake = "third_party\cmake-src\cmake-3.31.6-windows-x86_64\bin\cmake.exe"
& $cmake --build build --config Release --target EasyDelayCheck
& "build\tools\PluginCheck\EasyDelayCheck_artefacts\Release\EasyDelayCheck.exe"
```
