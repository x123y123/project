#ifndef MYGYRO_H
#define MYGYRO_H

#define Row   0
#define Pitch 1
#define Yaw   2

int gyro_init(const char gyro_addr[16]);
void get_gyro_data(int fd);

#endif
