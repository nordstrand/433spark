# SparkCore 433Mhz RF Remote Control

This is a port of the Arduino library [nexa_node](http://github.com/jherland/nexa_node) to SparkCore. Makes it possible to send and receive commands to/from Nexa brand power plugs, motion sensors, remote controls, etc. 
In combination with the SparkCore's built-in cloud features one essentially out-of-the-box get features similar to Tellstick and Nexa Gateway - at a fraction of the cost.

Nexa brand devices are widely available, sold at E.G. Clas Ohlson stores throughout Scandinavia.


## Operation
###Get latest Nexa command
```bash
$ curl  -H "Authorization: Bearer d586ffffd4334d84f4199fa1517cc5bdf02c66e9" \
"https://api.spark.io/v1/devices/core_name/command"
```
```javascript
{
  "cmd": "VarReturn",
  "name": "command",
  "result": "2:D38EB8:0:2:1",
  "coreInfo": {
    "last_handshake_at": "2015-01-04T18:43:21.222Z",
    "connected": true
  }
}
```

_The command was a request to turn on a power plug - trailing_ ```:1```

###Send a Nexa command

```bash
$ curl -H "Authorization: Bearer d586ffffd4334d84f4199fa1517cc5bdf02c66e9" \
https://api.spark.io/v1/devices/core_name/send -d "args=2:D38EB8:0:2:0"
```
```javascript
{
  "id": "ffff6c066672524845333456",
  "name": "core_name",
  "last_app": null,
  "connected": true,
  "return_value": 1
}
```
_Power plug ordered to turn off - trailing_ ```:0```

##Hardware setup

1. Sparkcore
2. 433Mhz RF receiver, model RWS-371
3. 433Mhz RF transmitter

![alt text](doc/sparkcore_breadboard.png  "Breadboard")


N.B. Wrt. the 5V receiver module - the SparkCore, though beeing a 3.3V devices, has some [5V-tolerant input pins](https://community.spark.io/t/3-3v-and-5v-how-to-use-on-the-spark-core/381) and can also supply 5V through _Vin_.
