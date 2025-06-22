# GoldenScrap
![GS](https://web.archive.org/web/20200418020827im_/https://goldenscrap.com/wp-content/uploads/2019/11/golden-scrap-jannah.jpg "GoldenScrap")
# 仕様
- USB-MIDI で PC から送られてきた信号を音源モジュールに流す
- 音源モジュールから送られてくるディジタル音声信号を，DAC（PCM5102） が理解できる形式に変換する

# 音声取り込み
## 入力データ仕様
- ~~24~~**20** bit, MSB-First
- Right-Justified
- サンプリングレート $F_S$ = 31 KHz
- ビットクロック $F_\text{BCK}$ = 8.192 KHz ($256\times F_{S}$)
- LRCK H = 左，L = 右？
- BCK の立ち下がりエッジでデータが変化する
    - 立ち上がりエッジで取り込めばいい

## 設計方針
- シリアルデータはPIOで取り込む。
- BCK は PIO に入力すると同時に GPIN0 に入力し，4 分周して GPOUT0 から出力し，出力（64 $F_{S}$）に利用する。
- PIO は，ビットクロック BCK より十分に速いシステムクロックに同期
    - BCK 立ち上がりエッジを検出して取り込む。

# 音声出力

# Pi Pico 配線
| Pico Pin | 信号 | 接続先 | memo |
|----|----|----|----|
| 1  | MIDI OUT  | GS Pin 9(MIDI_IN_A) | Pico→GS MIDI信号 |
| 2  | MIDI IN   | - | GS→Pico MIDI信号 |
| 3  | GND       | GS Pin 3,4,8,14（GND） のどれか | GND。うちのはGSの8,14に接続 |
| 4  | SDATA_IN  | GS Pin 5(SDATA) | GS -> Picoの音声 |
| 5  | LRCK_IN   | GS Pin 6(LRCK) | GS -> Picoの音声 |
| 6  | BCLK_IN   | GS Pin 7(BCLK), Pico 29(GP22) にも接続 | GS->Picoの音声クロック| 
| 7  | SDATA_OUT | DAC DIN | Pico -> DAC 音声 |
| 8  | GND       | DAC GND | GND。うちのはDACのGNDに接続 |
| 9  | LRCK_OUT  | DAC LCK | Pico -> DAC 音声 |
| 10 | DAC_BCK   | DAC BCK, Pico pin27(GP21) にも接続 | Pico -> DAC音声のクロック（BCLK_INの1/4）|
| 38 | GND       | GND | うちのはGS pin3,4に接続 | GND
| 40 | VBUS      | GS pin1,2(5V), DAC VIN にも接続 |

# 03-SC-R5
DAM-G70 Cyber DAM[BB]の音源モジュール。SC-8820/8850と同じXP6を搭載しているが、SC-8820,8850のマップはない。

- SC-88Proと同等？
- H8/510(main CPU)
- R00232667(M38881M2-150GP sub CPU)
- R01679978 RA09-002(XP6 tone generator)
    - SC-8820/8850と同じチップ

## pinout
|Pin #|Label|Description|
|:---:|:---|:---:|
|1  |5V         |Power +5V|
|2  |5V         |Power +5V|
|3  |GND        |Power GND|
|4  |GND        |Power GND|
|5  |SDATA_1    ||
|6  |LRCK       ||
|7  |BCLK       ||
|8  |GND        ||
|9  |MIDI_IN_A  ||
|10 |MIDI_IN_B  ||
|11 |MIDI_OUT   ||
|12 |SDATA_2    ||
|13 |RESET#     ||
|14 |GND        ||

# 03-SC-SA
ugaplus の音源モジュール。制御用マイコンがSH2で、XP6を2個搭載しているため、SC-8850と共通の仕様と思われる。

- SC-8850と同等？
- R02560212 HD6437016E18FV (SH2 main CPU)
- 2*R01679978 RA09-002(XP6 tone generator)
    - SC-8820/8850と同じチップ

## pinout
|Connector|Pin #|Label|Description|
|:---:|:---:|:---|:---|
|CN4    |1|VCC|Power +5V|
|       |2|VCC|Power +5V|
|       |3|GND|Power GND|
|       |4|GND|Power GND|
|       |5|_RESET#|Reset?|
|CN5    |1|BCK_64FS|Audio data bit clock(64Fs)|
|       |2|SDATA_1|Audio Data #1(64Fs)|
|       |3|SDATA_2|Audio Data #2(64Fs)|
|       |4|LRCK |Audio LRCK|
|       |5|SCK_256FS|Audio Data SCK(256Fs)|
|       |6|GND|Power GND|
|       |7|MIDI_OUT_1|MIDI out #1|
|       |8|MIDI_IN_1 |MIDI in #1|
|       |9|MIDI_OUT_2|MIDI out #2|
|       |10|MIDI_IN_2 |MIDI in #2|
|       |11|GND|Power GND|



```
20 bit(Right-Justified)
----_----_----_DDDD_DDDD_DDDD_DDDD_DDDD

24 bit
----_----_DDDD_DDDD_DDDD_DDDD_DDDD_DDDD

32 bit
DDDD_DDDD_DDDD_DDDD_DDDD_DDDD_DDDD_DDDD

24 bit(Left-Justified)
DDDD_DDDD_DDDD_DDDD_DDDD_DDDD_----_----

20 bit(Left-Justified)
DDDD_DDDD_DDDD_DDDD_DDDD_----_----_----

```