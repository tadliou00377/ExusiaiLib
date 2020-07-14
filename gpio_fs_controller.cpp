#include "gpio_fs_controller.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>

GPIO_FS_CTRL::GPIO_FS_CTRL(uint8_t pin) :
		pinNumber(pin) {

	if (!existsFS()) {
		//Export Pin
		int efd, len;
		char fileNameStr[MAX_BUF];
		efd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
		if (efd < 0) {
			perror("gpio/export");
			return;
		}
		len = snprintf(fileNameStr, sizeof(fileNameStr), "%d", pinNumber);
		write(efd, fileNameStr, len);
		close(efd);
		return;
	}
	openFS();

}

GPIO_FS_CTRL::~GPIO_FS_CTRL() {
	closeFS();
	if (existsFS()) {
		//Unexport Pin
		int efd, len;
		char buf[MAX_BUF];

		efd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
		if (efd < 0) {
			perror("gpio/export");
		}

		len = snprintf(buf, sizeof(buf), "%d", pinNumber);
		write(efd, buf, len);
		close(efd);
	}
}

int GPIO_FS_CTRL::SetDirection(int out_flag) {
	int fd_dir, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/direction",
			pinNumber);

	fd_dir = open(buf, O_WRONLY);
	if (fd_dir < 0) {
		perror("gpio/direction");
		return fd_dir;
	}

	if (GPIO_DIR_OUT)
		write(fd_dir, "out", 4);
	else
		write(fd_dir, "in", 3);

	close(fd_dir);
	return 0;
}

int GPIO_FS_CTRL::WritePin(uint8_t value) {
	int fd_value, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", pinNumber);

	fd_value = open(buf, O_WRONLY);
	if (fd_value < 0) {
		perror("gpio/set-value");
		return fd_value;
	}

	if (value)
		write(fd_value, "1", 2);
	else
		write(fd_value, "0", 2);

	close(fd_value);
	return 0;
}

int GPIO_FS_CTRL::ReadPin() {
	int fd_value, len;
	char buf[MAX_BUF];
	char ch;
	bool value = false;
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", pinNumber);

	fd_value = open(buf, O_RDONLY);
	if (fd_value < 0) {
		perror("gpio/get-value");
		return false;
	}

	read(fd_value, &ch, 1);

	if (ch != '0') {
		value = 1;
	} else {
		value = 0;
	}

	close(fd_value);
	return value;
}

int GPIO_FS_CTRL::SetEdge(char *edge) {
	int fd_edge, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", pinNumber);

	fd_edge = open(buf, O_WRONLY);
	if (fd_edge < 0) {
		perror("gpio/set-edge");
		return fd_edge;
	}

	write(fd_edge, edge, strlen(edge) + 1);
	close(fd_edge);
	return 0;
}

int GPIO_FS_CTRL::openFS() {
	int len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", pinNumber);

	fd = open(buf, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		perror("gpio/fd_open");
	}
	return fd;
}

int GPIO_FS_CTRL::closeFS() {
	return close(fd);
}

bool GPIO_FS_CTRL::existsFS() {
	char fileNameStr[MAX_BUF];
	int len;
	len = snprintf(fileNameStr, sizeof(fileNameStr),
	SYSFS_GPIO_DIR "/gpio%d/value", pinNumber);
	struct stat buf;
	int i = stat(fileNameStr, &buf);
	/* File found */
	if (i == 0) {
		return 1;
	}
	return 0;
}


