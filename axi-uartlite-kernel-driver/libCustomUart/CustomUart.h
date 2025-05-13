#ifndef CUSTOMUART_H
#define CUSTOMUART_H

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

int openSerialDevice(const char* devicePath, const int baudRate);
void closeSerialDevice(int deviceFd);
ssize_t writeSerial(int deviceFd, const void* dataBuffer, size_t dataLen);
ssize_t readSerial(int deviceFd, void* dataBuffer, size_t dataLen);

#ifdef __cplusplus
}
#endif

#endif