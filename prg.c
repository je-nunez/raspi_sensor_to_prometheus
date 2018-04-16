
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "Raspberry_Pi_2/pi_2_dht_read.h"
#include "common_dht_read.h"


#define DHTPIN       7


int main(int argc, const char * argv[]) {

  int sensor_type = DHT22;
  int data_pin = DHTPIN;
  int sampling_delay = 2 * 1000;

  while (true) {

    float temperature = 0, relative_humidity = 0;

    /* Try to read humidity and temperature from the DHT22 sensor attached
     * to the Raspberry Pi 2/3 at pin data_pin */

    int err_code = pi_2_dht_read(sensor_type, data_pin,
                                 &relative_humidity, &temperature);

    if (err_code != DHT_SUCCESS) {
      fprintf(stderr, "ERROR: couldn't read DHT22 sensor data. Error: %d\n",
              err_code);
    } else {
      printf("Humidity: %.2f %%, Temperature: %.2f celsius\n",
             relative_humidity, temperature);
    }

    usleep(sampling_delay * 1000);
  }

}
