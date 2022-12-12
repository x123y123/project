#ifndef _PID_CONTROLLER_H
#define _PID_CONTROLLER_H

#define ERROR_MAX_NUM 500
#define INIT_DIR "/home/uav/code/controller/deps/pid_controller/init.txt"

#include <time.h>

class PID {
public:
    float output = 0;

    PID();
    PID(float, float, float);
    void Setpid(float, float, float);
    void Setpid(char*);
    void IsAngle();
    bool Calculate(float, float);

private:
    bool _approach_angle = false;
    float _kp;
    float _ki;
    float _kd;

    float _error[ERROR_MAX_NUM] = {0.0};
    int _errorNum = 0;

    float _input;
    float _preInput = 0;
    float _errorSum = 0;

    struct timespec loop_start;
    struct timespec loop_end;

    void GetErrorSum(float);
    double diff_in_second(struct timespec, struct timespec);
};

#endif
