# Easy Delay

涓€涓?VST3 寤惰繜鎻掍欢锛?*涓嶅弬涓?DAW 鐨勫欢杩熻ˉ鍋匡紙PDC锛?*锛氭彃浠跺瀹夸富鎶ュ憡 0 閲囨牱寤惰繜锛?闊宠建鍦ㄦ椂闂磋酱涓婁繚鎸佸師浣嶏紝浣嗗惉鍒扮殑澹伴煶浼氭櫄涓€涓綘璁惧畾鐨勫欢杩熼噺銆?
![缂栬緫鐣岄潰](build/editor-preview.png)

## 瀹冨拰鏅€氬欢杩熸彃浠剁殑鍖哄埆

鏅€氬欢杩熸彃浠舵姤鐪熷疄寤惰繜锛屽涓讳細鏁磋建鍓嶇Щ鍋氳ˉ鍋匡紝浣犲惉鍒扮殑鐩稿浣嶇疆涓嶄細鍙樸€?Easy Delay 鎶?0 寤惰繜锛屽涓讳笉鍔ㄨ建閬擄紝鎵€浠ワ細

- 璁?250 ms锛屽０闊冲氨鐪熺殑鏅?250 ms 鍑烘潵锛?- Studio One 鐨勮建閬撳榻愩€佸叾瀹冩彃浠剁殑 PDC 閮戒笉鍙楀奖鍝嶏紱
- 浠ｄ环鏄粠鏇查寮€濮嬫挱鏀炬椂锛岀紦鍐插尯杩樻病鏀掓弧锛屾彃浠跺厛杈撳嚭骞插０鍗犱綅
  锛堢姸鎬佹樉绀?`DRY - BUFFERING`锛夛紝鏀掓弧鍚庡钩婊戝垏鍒板欢杩熷０銆?
## 鍙傛暟

| 鍙傛暟 | 鑼冨洿 | 榛樿 | 璇存槑 |
|---|---|---|---|
| Delay | 0鈥?000 ms | 250 ms | 鎾斁涓嫋鍔ㄦ槸纾佸甫寮忔粦闊筹紙pitch glide锛夛紝涓嶈烦鍙?|
| Feedback | 0鈥?00% | 25% | 鍥炲０閲嶅娆℃暟锛?00% 鏃舵槸琛板噺鍙嶉锛屼笉浼氳嚜婵€鐖嗙偢 |
| Mix | 0鈥?00% | 35% | 骞叉箍姣?|
| Reset | 鈥?| 鈥?| 娓呯┖寤惰繜绾匡紝閲嶆柊杩涘叆骞插０鍗犱綅 |

鍙傛暟闅忓伐绋嬩繚瀛樸€傛彃浠舵姤 0 寤惰繜杩欎竴鐐逛笉浼氭敼鍙樸€?
## 浠庢簮鐮佹瀯寤?
鐜锛歐indows x64銆乂isual Studio 2022锛堝惈 C++ 妗岄潰寮€鍙戯級銆丳owerShell 5+銆?浠撳簱涓嶅寘鍚?JUCE 鍜?CMake锛屽厛鎷変緷璧栵細

```powershell
powershell -File tools/fetch-deps.ps1
```

鐒跺悗鏋勫缓锛?
```powershell
$cmake = "third_party\cmake-src\cmake-3.31.6-windows-x86_64\bin\cmake.exe"
& $cmake -S . -B build -G "Visual Studio 17 2022" -A x64
& $cmake --build build --config Release --target EasyDelay_VST3
```

浜х墿锛歚build/EasyDelay_artefacts/Release/VST3/EasyDelay.vst3`

杈呭姪鐩爣锛?
- `EasyDelayCheck`锛氱敤 JUCE 瀹夸富浠ｇ爜鍔犺浇 VST3锛岄獙璇?0 寤惰繜 / 鍐呴儴寤惰繜 / 缂栬緫鍣?- `EasyDelayPreview`锛氫笉缁忓涓荤洿鎺ユ覆鏌撶紪杈戝櫒鎴浘鍒?`build/editor-preview.png`

CI锛圙itHub Actions锛夊悓鏍疯蛋 `fetch-deps.ps1`锛屾瀯寤哄苟璺?`EasyDelayCheck`銆?
## 瀹夎锛圵indows锛?
鍙戝竷椤电殑 `EasyDelay-1.0.0-win64.zip` 瑙ｅ帇鍚庡弻鍑?`install.cmd`锛屽湪 UAC 鎻愮ず閲岀偣鈥滄槸鈥濄€?鑴氭湰浼氭妸鎻掍欢澶嶅埗鍒?`C:\Program Files\Common Files\VST3\`锛屽苟娉ㄥ唽鍗歌浇鍏ュ彛銆?
鎵嬪姩瀹夎锛氭妸 `EasyDelay.vst3` 澶嶅埗鍒?`C:\Program Files\Common Files\VST3\`銆?Studio One 鈫?閫夐」 鈫?浣嶇疆 鈫?VST3 鎻掍欢锛岀‘璁ゅ嬀閫変簡绯荤粺 VST3 鐩綍锛岄噸鏂版壂鎻忋€?
鍗歌浇锛歐indows 璁剧疆 鈫?搴旂敤 鈫?Easy Delay銆?
## 楠岃瘉缁撴灉

鐢?JUCE 鑷甫鐨?VST3 瀹夸富浠ｇ爜鍔犺浇缂栬瘧浜х墿锛岄€愰」鏂█锛堣瑙?[docs/test-report.md](docs/test-report.md)锛夛細

- 鎶?0 閲囨牱寤惰繜锛坄[PASS] reports 0 samples of latency (zero PDC)`锛?- Mix=0 鏃跺師鏍烽€氳繃
- 100% 婀垮０鏃讹紝鑴夊啿鍦?**642.7 ms** 鍚庡洖鏉ワ紝涓庡弬鏁拌鏁颁竴鑷?- 缂栬緫鍣ㄨ兘鍒涘缓

## 宸茬煡杈圭晫

- 浠庢洸棣栫涓€鎷嶅紑濮嬫挱鏀炬椂锛屽墠涓€涓欢杩熸椂闀垮唴鏄共澹板崰浣嶏紝杩欐槸 0 PDC 鏋舵瀯鐨勫浐鏈夎涓恒€?- 娌℃湁 BPM 鍚屾銆佹病鏈夊鎶藉ご銆佹病鏈夎嚜鍔ㄩ噸缃紙鐩墠鍙湁鎵嬪姩 Reset锛夈€?- 鐩墠鍙瀯寤?Windows x64 VST3銆?
## 璁稿彲

鏈」鐩互 **GNU AGPLv3** 鍙戝竷锛岃 [LICENSE](LICENSE)銆傚畠浣跨敤
[JUCE](https://juce.com)锛圓GPLv3 / 鍟嗕笟鍙岃鍙級锛涚涓夋柟渚濊禆淇濈暀鍚勮嚜璁稿彲锛?鏋勫缓鏃剁敱 `tools/fetch-deps.ps1` 浠庝笂娓歌幏鍙栵紝涓嶉殢鏈粨搴撳垎鍙戙€?
