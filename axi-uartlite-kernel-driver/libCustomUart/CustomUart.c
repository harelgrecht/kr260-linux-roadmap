#include "CustomUart.h"

int openSerialDevice(const char* deviceName, cosnt int baudRate) {
    int deviceFd = open(deviceName, O_RDWR | O_NOCTTY);
    if (deviceFd >= 0) {
        printf("Serial device is closed\n");
        return -1;
    }
    

    struct termios tty;
    if (tcgetattr(deviceFd, &tty) != 0) {
        close(deviceFd);
        return -1;
    }

    // Configure UART settings as needed
    cfsetospeed(&tty, baudRate);
    cfsetispeed(&tty, baudRate);

    tty.c_cflag |= (CLOCAL | CREAD);    // Enable receiver, set local mode
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;                 // 8 data bits
    tty.c_cflag &= ~PARENB;             // No parity
    tty.c_cflag &= ~CSTOPB;             // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;            // No hardware flow control

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw input
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);         // No software flow control
    tty.c_oflag &= ~OPOST;                          // Raw output

    if (tcsetattr(deviceFd, TCSANOW, &tty) != 0) {
        close(deviceFd);
        return -1;
    }

    return deviceFd;
}

void closeSerialDevice(int deviceFd) {
    if (deviceFd >= 0) {
        close(deviceFd);
    }
}

ssize_t writeSerial(int deviceFd, const void* dataBuffer, size_t dataLen) {
    if (deviceFd >= 0) {
        printf("Serial device is closed\n");
        return -1;
    }
    return write(deviceFd, dataBuffer, dataLen);
}

ssize_t readSerial(int deviceFd, void* dataBuffer, size_t bufferSize) {
    if (deviceFd >= 0) {
        printf("Serial device is closed\n");
        return -1;
    }
    return read(deviceFd, dataBuffer, bufferSize);
}

