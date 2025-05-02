#include "../libCustomUart/CustomUart.h"

#define DEVICE_PATH "/dev/ttyCustomUart"

int main() {
    int fd = openSerialDevice(DEVICE_PATH, B115200);
    if (fd < 0) {
        printf("Failed to open UART device\n");
        return 1;
    }

    const char* msg = "Hello UART!\n";
    if (writeSerial(fd, msg, strlen(msg)) < 0) {
        printf("Failed to write to UART\n");
    }

    char buf[100] = {0};
    ssize_t bytesRead = readSerial(fd, buf, sizeof(buf)-1);
    if (bytesRead > 0) {
        buf[bytesRead] = '\0'; // null-terminate
        printf("Received: %s\n", buf);
    } else {
        printf("No data read from UART\n");
    }

    closeSerialDevice(fd);
    return 0;
}
