// to be used in position/rotation estimation

#ifndef QUAT_H_
#define QUAT_H_

typedef struct quat_t {
    float w, x, y, z;
} quat_t;


quat_t quat_getInverse(quat_t* quat);

quat_t quat_multiply(quat_t* lq, quat_t* rq);

quat_t quat_add(quat_t* lq, quat_t* rq);

float quat_getMagnitude(quat_t* quat);


#endif /* QUAT_H_ */
