#define _DEFAULT_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_BUS           "/dev/i2c-1" // Standard I2C bus on Raspberry Pi
#define PCA9685_ADDR      0x40         // I2C module address
#define MODE1             0x00         // Mode register 1
#define PRESCALE          0xFE         // Register for frequency
#define LED0_ON_L         0x06         // First channel output and brightness control byte 0

#define CHANNEL           7            // Target PWM channel

int i2c_init(const char *bus, int addr); // Open I2C bus and select slave device
int i2c_write_reg8(int fd, uint8_t reg, uint8_t value); // Write single byte to I2C register
void pca9685_set_pwm(int fd, int channel, int on, int off); // Servo control
void pca9685_init(int fd); // Basic module setup

int main() {
    int fd = i2c_init(I2C_BUS, PCA9685_ADDR); // I2C initialization
    if (fd < 0) {
        printf("ERROR: Failed to open I2C.\n");
        return 1;
    }

    pca9685_init(fd);

    int pwm = 0;
    int ch;

    while (1) {
        ch = getchar();
        if (ch == EOF) {
            break;
        }

        if (ch == '+') {
            pwm++;
            printf("PWM: %d\n", pwm);
        } else if (ch == '-') {
            pwm--;
            printf("PWM: %d\n", pwm);
        } else if (ch >= '0' && ch <= '9') {
            ungetc(ch, stdin); // Return character to read the whole number
            if (scanf("%d", &pwm) == 1) {
                printf("PWM: %d\n", pwm);
            }
        } else {
            continue; // Ignore newline and other characters
        }

        pca9685_set_pwm(fd, CHANNEL, 0, pwm); // Write the value to the module

        usleep(100000);
    }

    close(fd); // Close I2C file descriptor
    return 0;
}

int i2c_init(const char *bus, int addr) {
    int fd = open(bus, O_RDWR); // Open I2C bus device file
    if (fd < 0) {
        return -1;
    }

    if (ioctl(fd, I2C_SLAVE, addr) < 0) { // Set target I2C slave address
        close(fd);
        return -1;
    }

    return fd;
}

int i2c_write_reg8(int fd, uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value}; // Prepare register address and data payload
    if (write(fd, buffer, 2) != 2) {  // Send both bytes over I2C
        return -1;
    }
    return 0;
}

void pca9685_set_pwm(int fd, int channel, int on, int off) {
    uint8_t reg = LED0_ON_L + 4 * channel; // Select register for the given channel
    i2c_write_reg8(fd, reg, on & 0xFF);         // Pulse start (lower byte)
    i2c_write_reg8(fd, reg + 1, (on >> 8) & 0x0F);  // Pulse start (upper byte)
    i2c_write_reg8(fd, reg + 2, off & 0xFF);        // Pulse end (lower byte)
    i2c_write_reg8(fd, reg + 3, (off >> 8) & 0x0F); // Pulse end (upper byte)
}

void pca9685_init(int fd) {
    i2c_write_reg8(fd, MODE1, 0x10);  // Enter sleep mode to change frequency
    i2c_write_reg8(fd, PRESCALE, 121); // Set frequency to 50 Hz
    i2c_write_reg8(fd, MODE1, 0x00);  // Wake up module
    usleep(1000);                     // Wait for oscillator to stabilize
    i2c_write_reg8(fd, MODE1, 0xA1);  // Enable register auto-increment
}