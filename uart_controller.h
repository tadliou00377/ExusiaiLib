#ifndef _UART_CONTROLLER_H_
#define _UART_CONTROLLER_H_

#include <string>
#include <vector>
#include <termios.h>

class UART_Controller{
public:

	typedef enum{
		Speed_115200 = B115200,
		Speed_19200 = B19200,
		Speed_57600 = B57600,
		Speed_9600 = B9600,
		Speed_4800 = B4800,
		Speed_2400 = B2400,
		Speed_1200 = B1200
	}UART_Baudrate_t;

	typedef enum{
		Flow_none = 0,
		Flow_RTSCTS = 1,
		Flow_XOR = 2
	}UART_FlowControl_t;

	typedef enum{
		Parity_None = 0,
		Parity_Odd = 1,
		Parity_Even,
		//Parity_Mark,
		Parity_Spece
	}UART_Parity_t;

public:
	UART_Controller(const std::string port
			,UART_Baudrate_t baudrate
			,UART_FlowControl_t flowCtrl
			,UART_Parity_t parity
			,uint8_t dataBit
			,uint8_t stopBit);
	~UART_Controller();
	bool Open();
	void Close();

	void SetBaudrate(UART_Baudrate_t baudrate){
		_baundrate = baudrate;
	}
	void SetFlowControl(UART_FlowControl_t flowCtrl){
		_flowCtrl = flowCtrl;
	}
	void SetParity(UART_Parity_t parity){
		_parity = parity;
	}
	void SetDataBit(uint8_t dataBit){
		if(dataBit > 8 || dataBit < 5)
			_dataBit = 8;
		else
			_dataBit = dataBit;
	}
	void SetStopBit(uint8_t stopBit){
		if(stopBit > 1 || stopBit < 2)
			_stopBit = 1;
		else
			_stopBit = stopBit;
	}

	bool ApplyOption();

	int RecvAsync(char *buffer,int buffer_len,uint8_t sec, uint8_t usec);
	int RecvAwait(char *buffer,int buffer_len);
	int Send(char *buffer,int buffer_len);

private:
	std::string fileNameStr;

	struct termios _options;
	UART_Baudrate_t _baundrate = Speed_115200;
	UART_FlowControl_t _flowCtrl = Flow_none;
	UART_Parity_t _parity = Parity_None;
	uint8_t _dataBit = 8;
	uint8_t _stopBit = 1;

	int fd = -1;

};



#endif
