#include <gpiod.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int ON_THRESHOLD   = 55;   // (degrees Celsius) Fan kicks on at this temperature.
int OFF_THRESHOLD  = 45;   // (degress Celsius) Fan shuts off at this temperature.
int SLEEP_INTERVAL = 5;    // (seconds) How often we check the core temperature.
int GPIO_PIN       = 17;   // Which GPIO pin you're using to control the fan.
char *CHIPNAME     = "gpiochip0";

#ifndef	CONSUMER
#define	CONSUMER	"Consumer"
#endif

/*
    Get the core temperature.
    Read file from /sys to get CPU temp in temp in C *1000
    Returns:
        int: The core temperature in thousanths of degrees Celsius.
*/

int get_temp(void)
{
  int fd;
  char temp[3] = {0};

  fd = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
  if (fd < 0)
      exit(1);
  if (read(fd, temp, 2))
    return(atoi(temp));
  else
    {
      close(fd);
      exit(1);
    }
}

int main(void)
{
  int temp;
  struct gpiod_chip *chip;
  struct gpiod_line *line;
  int ret;

  if (OFF_THRESHOLD >= ON_THRESHOLD)
  {
      perror("OFF_THRESHOLD must be less than ON_THRESHOLD");
      return (1);
  }
  chip = gpiod_chip_open_by_name(CHIPNAME);
  if (!chip)
  {
      perror("Open chip failed");
      return (1);
  }
  line = gpiod_chip_get_line(chip, GPIO_PIN);
  if (!line)
  {
	  perror("Get line failed");
	  gpiod_chip_close(chip);
	  return (1);
  }
  ret = gpiod_line_request_output(line, CONSUMER, 0);
  if (ret < 0)
  {
	  perror("Request line as output failed");
	  gpiod_line_release(line);
	  gpiod_chip_close(chip);
	  return (1);
  }
  while (1)
  {
	  temp = get_temp();
      printf("%d > %d\n", temp, ON_THRESHOLD);
      if(temp > ON_THRESHOLD)
		  ret = gpiod_line_set_value(line, 1);
      else if (temp < OFF_THRESHOLD)
		  ret = gpiod_line_set_value(line, 0);
      if (ret < 0)
	  {
		  perror("Set line output failed");
		  gpiod_line_release(line);
		  gpiod_chip_close(chip);
		  return (1);
	  }
      sleep(SLEEP_INTERVAL);
  }
  return (0);
}
