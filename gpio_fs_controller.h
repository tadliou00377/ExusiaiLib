#include <sys/types.h>


#ifndef _GPIO_FS_CONTROLLER_H_
#define _GPIO_FS_CONTROLLER_H_

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

#define GPIO_DIR_OUT 	1
#define GPIO_DIR_IN 	0
#define GPIO_PIN_HI		1
#define GPIO_PIN_LOW	0

class GPIO_FS_CTRL{

public:

	GPIO_FS_CTRL(uint8_t pin);
	~GPIO_FS_CTRL();
	int SetDirection(int);
	int WritePin(uint8_t);
	int ReadPin();
	int SetEdge(char *edge);

private:
	int fd = -1;
	uint8_t pinNumber;
	int openFS();
	int closeFS();
	bool existsFS();

};


#endif
