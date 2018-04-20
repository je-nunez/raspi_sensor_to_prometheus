# WIP

This project is a *work in progress*. The implementation is *incomplete* and
subject to change. The documentation can be inaccurate.


Sensor data (RHT03/DHT22 temperature and humidity) in Raspberry Pi 2/3 to
the Prometheus open-source monitoring system. It tries to make the sampler
as smaller as possible in RAM footprint inside the Raspberry Pi 2/3 since
it runs 24x7, so it uses the Adafruit MIT-licensed C-code underlying their
Python wrapper:

[https://github.com/adafruit/Adafruit_Python_DHT/tree/master/source](https://github.com/adafruit/Adafruit_Python_DHT/tree/master/source)

Sample output (in Prometheus Text exposition format, [https://prometheus.io/docs/instrumenting/exposition_formats/#text-format-details](https://prometheus.io/docs/instrumenting/exposition_formats/#text-format-details)):
          
          # TYPE dht22_temperature gauge
          # HELP dht22_temperature Temperature in the RHT03/DHT22 sensor
          dht22_temperature 21.70 1524224739564
          # TYPE dht22_relat_humidity gauge
          # HELP dht22_relat_humidity Relative humidity in the RHT03/DHT22 sensor
          dht22_relat_humidity 18.40 1524224739564
          
To compile this C program:
          
          make compile

The `-h` option will give a command-line usage:

          rasppi_dht22_sampler:
          Take samples from a RHT03/DHT22 sensor attached to a Raspberry Pi 2/3 to the Prometheus monitoring system's text collector.
          
          Optional command-line arguments:
             [-h] [-p dht22_port] [-w wait_seconds] [prometheus_label="value"] ...
          
          Explanation of the optional command-line arguments:
          
               -h: show these help messages.
               -p dht22_port: the data port by which this Raspberry Pi 2/3 communicates with the RHT03/DHT22 (default: 17).
               -w wait_seconds: seconds to wait between consecutive polls from the sensor (default: 60 seconds).
               prometheus_label="value"...: Prometheus label="value" pairs with which to tag the output (default: none).
                                              (Note: Prometheus requires that the value of the label needs to be quoted between '"' double-quotes.
                                               These opening and closing quotes need to be given in the command-line argument.
                                               Probably, in a sh- or bash- like shell, the whole label="value" needs to be protected thus:
                                                  'label="value"'.)
          

