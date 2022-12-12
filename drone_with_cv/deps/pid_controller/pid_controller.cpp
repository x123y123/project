#include "pid_controller.h"
#include <stdio.h>

PID::PID()
{
    FILE *fd;
    
    fd = fopen(INIT_DIR, "r");

    if (!fd) {
        printf("open file fail!\n");
        return;
    }

    fscanf(fd, "%f %f %f", &_kp, &_ki, &_kd);

    fclose(fd);
}

PID::PID(float kp, float ki, float kd)
{
    _kp = kp;
    _ki = ki;
    _kd = kd;
}
void PID::Setpid(float kp, float ki, float kd)
{
    _kp = kp;
    _ki = ki;
    _kd = kd;
}

void PID::Setpid(char *Kpid_dir)
{
    FILE *fd;

    fd = fopen(Kpid_dir, "r");

    if (!fd) {
        printf("open file fail!\n");
        return;
    }

    fscanf(fd, "%f %f %f", &_kp, &_ki, &_kd);

    fclose(fd);
}

void PID::IsAngle()
{
    _approach_angle = true;
}

bool PID::Calculate(float input, float target)
{
    float error, dInput, out;

    error = target - input;
    dInput = input - _preInput;

    if (_approach_angle) {
        if (error > 180)
            error -= 360;
        else if (error < -180)
            error += 360;

        if (dInput > 180)
            dInput -= 360;
        else if (dInput < -180)
            dInput += 360;
    }

    GetErrorSum(error);

    out = _kp * error + _ki * _errorSum + _kd * dInput;

    output = out;
    _preInput = input;

    return true;
}

void PID::GetErrorSum(float error)
{
    int i;
    float sum = 0;

    if (_errorNum < ERROR_MAX_NUM) {
        _error[_errorNum] = error;
        _errorNum++;
    }

    else {
        for (i = 0; i < ERROR_MAX_NUM - 1; i++)
            _error[i] = _error[i+1];
        _error[ERROR_MAX_NUM - 1] = error;
    }

    for (i = 0; i < _errorNum; i++)
        sum += _error[i];

    _errorSum = sum;
}

double PID::diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    
    if (t2.tv_nsec - t1.tv_nsec < 0) {
        diff.tv_sec = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    }
    else {                                                                                                                                                                                                
        diff.tv_sec = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }

    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}


