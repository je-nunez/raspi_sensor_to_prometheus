# WIP

This project is a *work in progress*. The implementation is *incomplete* and
subject to change. The documentation can be inaccurate.


Sensor data (RHT03/DHT22 temperature and humidity) in Raspberry Pi 2/3 to
the Prometheus open-source monitoring system. It tries to make the sampler
as smaller as possible in RAM footprint inside the Raspberry Pi 2/3 since
it runs 24x7, so it uses the Adafruit MIT-licensed C-code underlying their
Python wrapper:

[https://github.com/adafruit/Adafruit_Python_DHT/tree/master/source](https://github.com/adafruit/Adafruit_Python_DHT/tree/master/source)


