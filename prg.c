
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <regex.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#include "Raspberry_Pi_2/pi_2_dht_read.h"
#include "common_dht_read.h"


// Some defaults
// The first one is a default GPIO index (for the mapping of GPIO indexes to
// physical pin numbers see, e.g.,
// https://www.raspberrypi.org/forums/viewtopic.php?t=196696 )
#define DEFAULT_DHT_GPIO_IDX  17
#define DEFAULT_WAIT_SECONDS  60

#define MIN_GPIO_INDEX  0
#define MAX_GPIO_INDEX  27

// the minimum sampling rate of the DHT22/RHT03: every 2 seconds
// ( https://learn.adafruit.com/dht/overview )
#define MIN_WAIT_SECONDS   2


// The type specifying the configuration settings for this program
struct configuration_settings {
  int dht22_gpio_idx;
  int wait_seconds;
  char ** prometheus_labels;
  int num_prometheus_labels;
};


void show_help_and_exit(void) {
  printf(
    "rasppi_dht22_sampler:\n"
    "Take samples from a RHT03/DHT22 sensor attached to a Raspberry Pi 2/3 "
    "to the Prometheus monitoring system's text collector.\n\n"
    "Optional command-line arguments:\n"
    "   [-h] [-g gpio_idx] [-w wait_seconds]"
      " [prometheus_label=\"value\"] ...\n"
    "\n"
    "Explanation of the optional command-line arguments:\n\n"
    "     -h: show these help messages.\n"
    "     -g gpio_idx: the GPIO index by which this Raspberry Pi 2/3 "
                      "communicates with the RHT03/DHT22 (default: %d).\n"
    "     -w wait_seconds: seconds to wait between consecutive polls from "
                          "the sensor (default: %d seconds).\n"
    "     prometheus_label=\"value\"...: Prometheus label=\"value\" pairs "
                          "with which to tag the output (default: none).\n"
    "                                    (Note: Prometheus requires that the "
    "value of the label needs to be quoted "
    "between '\"' double-quotes.\n"
    "                                     These opening and closing quotes "
    "need to be given in the command-line argument.\n"
    "                                     Probably, in a sh- or bash- like "
    "shell, the whole label=\"value\" needs to be protected thus:\n"
    "                                        'label=\"value\"'.)\n",
    DEFAULT_DHT_GPIO_IDX, DEFAULT_WAIT_SECONDS
  );
  exit(0);
}

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
    exit(3);
  }

  return (int) value;
}

void check_prometheus_label(char * in_string,
                            struct configuration_settings * output_config) {

  char * equal_separator = index(in_string, '=');
  if (! equal_separator) {
    // A Prometheus label has the format 'label_name="label_value"'
    // https://prometheus.io/docs/instrumenting/exposition_formats/#text-format-details
    fprintf(stderr, "ERROR: '%s' does not have an '=' sign, so it is not a "
                    "valid Prometheus 'label_name=\"label_value\"'.\n",
                    in_string);
    exit(4);
  }

  // Check that the label name matches the regexp [a-zA-Z_][a-zA-Z0-9_]*
  // https://github.com/prometheus/docs/blob/master/content/docs/concepts/data_model.md#metric-names-and-labels
  *equal_separator = '\0';

  regex_t regex_label_name;
  int err_regex = regcomp(&regex_label_name, "^[a-zA-Z_][a-zA-Z0-9_]*$",
                          REG_NOSUB);
  if (err_regex != 0) {
    char  err_msg[128];
    regerror(err_regex, &regex_label_name, err_msg, sizeof err_msg);
    fprintf(stderr, "ERROR: Couldn't compile regular expression at "
                    "%s, line %d: %s\n", __FILE__, __LINE__, err_msg);
    exit(5);
  }
  int result = regexec(&regex_label_name, in_string, (size_t) 0, NULL, 0);
  regfree(&regex_label_name);
  if (result != 0) {
    fprintf(stderr, "ERROR: '%s' is not a valid Prometheus label_name.\n",
                    in_string);
    exit(6);
  }

  // Check that the label_value (after the '=' sign) be a non-empty '"..*"'
  regex_t regex_label_value;
  err_regex = regcomp(&regex_label_value, "\"..*\"$", REG_NOSUB);
  if (err_regex != 0) {
    char  err_msg[128];
    regerror(err_regex, &regex_label_value, err_msg, sizeof err_msg);
    fprintf(stderr, "ERROR: Couldn't compile regular expression at "
                    "%s, line %d: %s\n", __FILE__, __LINE__, err_msg);
    exit(7);
  }

  result = regexec(&regex_label_value, equal_separator+1, (size_t) 0, NULL, 0);
  regfree(&regex_label_value);
  if (result != 0) {
    fprintf(stderr, "ERROR: '%s' is not a valid Prometheus label_value.\n",
                    equal_separator+1);
    exit(8);
  }

  // Store the Prometheus 'label_name="label_value"' pair in
  // the output_config->prometheus_labels
  *equal_separator = '=';    // restore the equal sign between name and value
  output_config->num_prometheus_labels ++;
  if (output_config->prometheus_labels == NULL) {
    assert(output_config->num_prometheus_labels == 1);
    output_config->prometheus_labels = malloc(1 * sizeof(char*));
  } else {
    assert(output_config->num_prometheus_labels > 1);
    output_config->prometheus_labels =
          realloc(
            output_config->prometheus_labels,
            output_config->num_prometheus_labels * sizeof(char*)
          );
  }
  // TODO: check extreme cases when output_config->prometheus_labels != NULL
  output_config->prometheus_labels[output_config->num_prometheus_labels - 1] =
    in_string;
}

void parse_command_line(int argc, char *const *argv,
                        struct configuration_settings * output_config) {

  int c;

  while ((c = getopt(argc, argv, "hg:w:")) != -1)
    switch (c)
      {
      case 'h':
        show_help_and_exit();
        break;
      case 'g':
        output_config->dht22_gpio_idx = convert_str_to_int(optarg);
        if (output_config->dht22_gpio_idx < MIN_GPIO_INDEX ||
            output_config->dht22_gpio_idx > MAX_GPIO_INDEX ) {
               fprintf (stderr,
                        "ERROR: Invalid GPIO index '%d'. "
                        "It should be between %d and %d.\n",
                        output_config->dht22_gpio_idx,
			MIN_GPIO_INDEX, MAX_GPIO_INDEX);
               exit(200);
	}
        break;
      case 'w':
        output_config->wait_seconds = convert_str_to_int(optarg);
        if (output_config->wait_seconds < MIN_WAIT_SECONDS) {
               fprintf (stderr,
                        "ERROR: Invalid sampling wait time '%d'. "
                        "The minimum allowable value is %d seconds.\n",
                        output_config->wait_seconds,
                        MIN_WAIT_SECONDS);
               exit(200);
	}
        break;
      case '?':
        if (optopt == 'g')
          fprintf (stderr,
                   "Option -%c requires an argument: the GPIO index in the "
                   "Raspberry Pi 2/3 where the RHT03/DHT22 comes in.\n",
                   optopt);
        else if (optopt == 'w')
          fprintf (stderr,
                   "Option -%c requires an argument: the wait time between "
                   "samples from the RHT03/DHT22.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
        break;
      default:
        abort ();
      }

  for (int index = optind; index < argc; index++)
    check_prometheus_label(argv[index], output_config);

}

void print_prometheus_labels(FILE *output,
                             const struct configuration_settings * config) {

  // https://prometheus.io/docs/instrumenting/exposition_formats/#text-format-details

  if (config->num_prometheus_labels <= 0)
    return;  // no Prometheus labels to print

  assert(config->prometheus_labels != NULL);
  printf("{");
  for (int label_idx=0, remaining_idx = config->num_prometheus_labels;
       label_idx < config->num_prometheus_labels;
       label_idx++, remaining_idx--) {
    printf(config->prometheus_labels[label_idx]);
    if (remaining_idx > 1)
      printf(", ");   // print a comma after label-value pair, except for last
  }
  printf("}");
}

unsigned long long get_curr_epoch_microsec(void) {

  struct timespec curr_time;
  clock_gettime(CLOCK_REALTIME, &curr_time);

  unsigned long long epoch_microsec = (
            (unsigned long long)(curr_time.tv_sec) * 1000000 +
            (unsigned long long)(curr_time.tv_nsec) / 1000
  );

  return epoch_microsec;
}

void dht22_values_to_prometheus(FILE *output,
                                float dht22_temp, float dht22_humidity,
                                const struct configuration_settings * config) {

  // https://prometheus.io/docs/instrumenting/exposition_formats/#text-format-details

  unsigned long long epoch_millisec = get_curr_epoch_microsec() / 1000;

  fprintf(output, "# TYPE dht22_temperature gauge\n"
                  "# HELP dht22_temperature Temperature in the RHT03/DHT22 sensor\n"
                  "dht22_temperature");
  print_prometheus_labels(output, config);
  fprintf(output, " %.2f %llu\n", dht22_temp, epoch_millisec);

  fprintf(output, "# TYPE dht22_relat_humidity gauge\n"
                  "# HELP dht22_relat_humidity Relative humidity in the RHT03/DHT22 sensor\n"
                  "dht22_relat_humidity");
  print_prometheus_labels(output, config);
  fprintf(output, " %.2f %llu\n", dht22_humidity, epoch_millisec);
}

int main(int argc, char ** argv) {

  const int sensor_type = DHT22;
  struct configuration_settings actual_config = {
                                        .dht22_gpio_idx = DEFAULT_DHT_GPIO_IDX,
                                        .wait_seconds = DEFAULT_WAIT_SECONDS,
                                        .prometheus_labels = NULL,
                                        .num_prometheus_labels = 0
                                      };

  parse_command_line(argc, argv, &actual_config);

  unsigned long long previous_end_epoch_microsec = get_curr_epoch_microsec();

  float temperature = 0, relative_humidity = 0;

  while (true) {

    /* Try to read humidity and temperature from the DHT22 sensor attached
     * to the Raspberry Pi 2/3 at GPIO dht22_gpio_idx */

    int err_code = pi_2_dht_read(sensor_type,
		                 actual_config.dht22_gpio_idx,
                                 &relative_humidity, &temperature);

    if (err_code != DHT_SUCCESS) {
      fprintf(stderr, "ERROR: couldn't read DHT22 sensor data. Error: %d\n",
              err_code);
    } else {
      dht22_values_to_prometheus(stdout,
                                 temperature, relative_humidity,
                                 &actual_config);
    }

    unsigned long long end_epoch_microsec = get_curr_epoch_microsec();
    // USleep wait_seconds, substracting the time spent in the above loop
    unsigned long long actual_usleep = (
	             actual_config.wait_seconds * 1000000 -
                         (end_epoch_microsec - previous_end_epoch_microsec)
                   );
    struct timespec sleep_request, sleep_remaining;
    sleep_request.tv_sec = ( actual_usleep / 1000000 );
    sleep_request.tv_nsec = ( actual_usleep % 1000000 ) * 1000;
    nanosleep(&sleep_request, &sleep_remaining);
    // Note: probably the implementation using a timer_create(2) and
    // timer_settime(2) is better to establish this output of the metrics
    // to Prometheus every wait_seconds, instead of using nanosleep().

    previous_end_epoch_microsec = end_epoch_microsec;
  }

}
