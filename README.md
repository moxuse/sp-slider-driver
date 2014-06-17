sp-slider-driver
================

マイコン<->ドライバ-パラレルケーブル

PIN No.／ケーブル色／機能／Arduino Pin No.

- 21 白 GND   G
- 31 黄 PULSE +   10
- 32 青 PULSE -   G
- 35 赤 Direction +  9   
- 36 緑 Direction -  G
- 23 橙 Current +  13
- 22 黒 GND   G
- 2  黒 GND   G
- 20 黒 TIM -   8
- 19 橙 TIM +  5v



ワンストローク164000 pulse

1640段階　per 0.6mm だと数えられる


---------

マイコン<->リミットセンサケーブル

- G G  ->  2
- 黒 220KΩ　-> G  -> 4  
- 赤 5V   ->  3
- 白 220Ω   ->  1

- 黒
- 青
- 橙
- 黄


----------

##OSCプロトコル

####送信

```/step [目標ステップ 0~1640] [回転スピード 0~255] ```

ex.

``` NetAddr("localhost",5100).sendMsg("/step", 1640, 20) ```

ステップまで移動

```/reset  ```

ex.

```NetAddr("localhost",5100).sendMsg("/reset") ```

原点回帰


####受信

```/limit_near [現在ステップ] ```

リミット検知、原点


```/limit_far [現在ステップ] ```

リミット検知、端点


```/ready [現在ステップ] ```

移動／原点回帰終了





