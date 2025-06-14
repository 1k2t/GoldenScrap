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

# GS モジュールのピン配置
1. 5V
1. 5V
1. GND
1. GND
1. SDATA
1. LRCK
1. BCLK
1. GND
1. MIDI_IN_A
1. MIDI_IN_B
1. MIDI_OUT
1. SDDA2
1. RESET#
1. GND

# 参考
- hid_composite example from TinyUSB
- [i2s-example](https://github.com/malacalypse/rp2040_i2s_example)
