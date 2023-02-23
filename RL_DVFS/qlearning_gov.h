#ifndef _QLEARNING_GOV_H
#define _QLEARNING_GOV_H
#ifdef __cplusplus
extern "C" {
#endif

int 

// Min and Max limits are in linear scale
float min[3] = { 
                0.0f,  // usage
                30.0f, // temp
                0.0f   // power(w)
                };

float max[3] = { 
                1.2f,  // usage
                56.0f, // temp
                6.0f   // power(w)
                };


#ifdef __cplusplus
}
#endif
#endif  //_STATE_SPACE_PARAMS_H
