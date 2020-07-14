#include "uart_controller.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

using namespace std;

UART_Controller::UART_Controller(const std::string port,
		UART_Baudrate_t baudrate, UART_FlowControl_t flowCtrl,
		UART_Parity_t parity, uint8_t dataBit, uint8_t stopBit) :
		fileNameStr(port), _baundrate(baudrate),_parity(parity) {
	SetDataBit(dataBit);
	SetStopBit(stopBit);
}
UART_Controller::~UART_Controller() {
	Close();
}

bool UART_Controller::Open() {
	fd = open(fileNameStr.c_str(), O_RDWR | O_NOCTTY | O_NDELAY); //O_NOCTTY：告訴Linux這個程式不想控制TTY介面，如果不設定這個旗標，有些輸入(例如鍵盤的abort)信號可能影響程式。
	//O_NDELAY：告訴Linux這個程式不介意RS-232的DCD信號的狀態。如果不設定這個旗標，程式會處於speep狀態，直到RS-232有DCD信號進來。
	if (fd < 0) {
		perror("Can't Open Serial Port");
		return false;
	}
//恢復串口爲阻塞狀態
	if (fcntl(fd, F_SETFL, 0) < 0) {
		printf("fcntl failed!\n");
		return false;
	} else {
		printf("fcntl=%d\n", fcntl(fd, F_SETFL, 0));
	}
//測試是否爲終端設備
	if (0 == isatty(STDIN_FILENO)) {
		printf("standard input is not a terminal device\n");
		return false;
	} else {
		printf("isatty success!\n");
	}
	printf("fd->open=%d\n", fd);
	return ApplyOption();
}

void UART_Controller::Close() {
	close(fd);
}

bool UART_Controller::ApplyOption() {
	int status;

	struct termios options;

	if (tcgetattr(fd, &options) != 0) {
		perror("SetupSerial 1");
		return false;
	}
	//Baudrate
	cfsetispeed(&options, _baundrate); //設定輸入波頻率
	cfsetospeed(&options, _baundrate); //設定輸出波頻率

	//修改控制模式，保證程序不會佔用串口
	options.c_cflag |= CLOCAL;
	//修改控制模式，使得能夠從串口中讀取輸入數據
	options.c_cflag |= CREAD;
	//Flow Control
	switch (_flowCtrl) {
	case Flow_none: //不使用流控制
		options.c_cflag &= ~CRTSCTS;
		break;
	case Flow_RTSCTS: //使用硬件流控制
		options.c_cflag |= CRTSCTS;
		break;
	case Flow_XOR: //使用軟件流控制
		options.c_cflag |= IXON | IXOFF | IXANY;
		break;
	}
	//Data Bit
	options.c_cflag &= ~CSIZE;
	switch (_dataBit) {
	case 5:
		options.c_cflag |= CS5;
		break;
	case 6:
		options.c_cflag |= CS6;
		break;
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "Unsupported data size\n");
		return false;
	}
	//Parity
	switch (_parity) {
	case Parity_None: //無奇偶校驗位。
		options.c_cflag &= ~PARENB;
		options.c_iflag &= ~INPCK;
		break;
	case Parity_Odd: //設置爲奇校驗
		options.c_cflag |= (PARODD | PARENB);
		options.c_iflag |= INPCK;
		break;
	case Parity_Even: //設置爲偶校驗
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;
		break;
	case Parity_Spece: //設置爲空格
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported parity\n");
		return false;
	}
	//Stop Bit
	switch (_stopBit) {
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported stop bits\n");
		return false;

	}
	//修改輸出模式，原始數據輸出
	options.c_oflag &= ~OPOST;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	//options.c_lflag &= ~(ISIG | ICANON);
	//設置等待時間和最小接收字符
	options.c_cc[VTIME] = 1; /* 讀取一個字符等待1*(1/10)s */
	options.c_cc[VMIN] = 1; /* 讀取字符的最少個數爲1 */
	//如果發生數據溢出，接收數據，但是不再讀取 刷新收到的數據但是不讀
	tcflush(fd, TCIFLUSH);

	//Apply Option
	if (tcsetattr(fd, TCSANOW, &options) != 0) {
		perror("Serial Apply Option error!\n");
		return false;
	}
	return true;
}

int UART_Controller::RecvAsync(char *buffer, int buffer_len,uint8_t sec = 10, uint8_t usec = 0) {
	int len, fs_sel;
	fd_set fs_read;

	struct timeval time;

	FD_ZERO(&fs_read);
	FD_SET(fd, &fs_read);

	time.tv_sec = sec;
	time.tv_usec = usec;

	fs_sel = select(fd + 1, &fs_read, NULL, NULL, &time);
	//printf("fs_sel = %d\n", fs_sel);
	if (fs_sel) {
		len = read(fd, buffer, buffer_len);
		printf("len = %d fs_sel = %d\n", len, fs_sel);
		return len;
	} else {
		//printf("Reveive Failed!");
		return -1;
	}
}

int UART_Controller::RecvAwait(char *buffer, int buffer_len) {

	int len = 0;
	len = len = read(fd, buffer, buffer_len);
	if(len>=0){

		return len;
	}
	else{
		printf("Await Receive UART:%d Error(%d)\n", fd, len);
		return -1;
	}
}

int UART_Controller::Send(char *buffer, int buffer_len) {
	int len = 0;
	len = write(fd, buffer, buffer_len);
	if (len == buffer_len) {
		printf("send data is %s\n", buffer);
		return len;
	} else {
		tcflush(fd, TCOFLUSH);
		return -1;
	}
}
