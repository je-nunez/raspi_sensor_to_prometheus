
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "Raspberry_Pi_2/pi_2_dht_read.h"
#include "common_dht_read.h"

#define DEFAULT_DHTPIN  7

int convert_str_to_int(const char * str) {

  char * num_end;
  long value = strtol(str, &num_end, 0);

  if (errno == EINVAL) {
    fprintf(stderr, "ERROR: Conversion error occurred on '%s': %d\n",
            str, errno);
    exit(1);
  } else if (*num_end != '\0') {
    fprintf(stderr, "ERROR: It is not a proper number: '%s'\n", str);
    exit(2);
  } else if (value < INT_MIN || value > INT_MAX) {
    fprintf(stderr, "ERROR: Value '%ld' is too big, "
                    "would overflow an integer.\n", value);
    exit(2);
  }
    
  return (int) value;
}

void parse_command_line(int argc, char *const *argv,
                        int * data_port, int * wait_seconds) {

  int c;

  while ((c = getopt (argc, argv, "p:w:")) != -1)
    switch (c)
      {
      case 'p':
        *data_port = convert_str_to_int(optarg);
        break;
      case 'w':
        *wait_seconds = convert_str_to_int(optarg);
        break;
      case '?':
        if (optopt == 'p')
          fprintf (stderr,
                   "Option -%c requires an argument: the data port in the "
                   "Raspberry Pi 2/3 where the RHT03/DHT22 comes in.\n",
                   optopt);
        else if (optopt == 'w')
          fprintf (stderr,
                   "Option -%c requires an argument: the wait time between "
                   "samples from the RHT03/DHT22.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        break;
      default:
        abort ();
      }
}


int main(int argc, char ** argv) {

  int sensor_type = DHT22;
  int data_pin = DEFAULT_DHTPIN;
  int sampling_delay = 2;

  parse_command_line(argc, argv, &data_pin, &sampling_delay);

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
