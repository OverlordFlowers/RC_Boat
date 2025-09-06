

#include "math_helpers/quat.h"

quat_t quat_getInverse(quat_t* quat) {
    quat_t inv_quat;
    inv_quat.w = quat->w;
    inv_quat.x = quat->x;
    inv_quat.y = quat->y;
    inv_quat.z = quat->z;

    return inv_quat;
}

float quat_getMagnitude(quat_t* quat) {
    return (quat->w * quat->w + quat->x * quat->x + quat->y * quat->y + quat->z * quat->z);
}

quat_t quat_normalize(quat_t* quat) {
    float mag = quat_getMagnitude(quat);
    quat_t norm_quat;

    norm_quat.w = quat->w / mag;
    norm_quat.x = quat->x / mag;
    norm_quat.y = quat->y / mag;
    norm_quat.z = quat->z / mag;

    return norm_quat;
}

// Math operations
quat_t quat_multiply(quat_t* lq, quat_t* rq) {
    quat_t new_quat;
    new_quat.w = (lq->w * rq->w) - (lq->x * rq->x) - (lq->y * rq->y) - (lq->z * rq->z);
    new_quat.x = (lq->w * rq->x) + (lq->x * rq->w) + (lq->y * rq->z) - (lq->z * rq->y);
    new_quat.y = (lq->w * rq->y) - (lq->x * rq->z) + (lq->y * rq->w) + (lq->z * rq->x);
    new_quat.z = (lq->w * rq->z) + (lq->x * rq->y) - (lq->y * rq->x) + (lq->z * rq->w);

    return new_quat;
}

quat_t quat_add(quat_t* lq, quat_t* rq) {
    quat_t new_quat;

    new_quat.w = (lq->w + rq->w);
    new_quat.x = (lq->x + rq->x);
    new_quat.y = (lq->y + rq->y);
    new_quat.x = (lq->z + rq->z);

    return new_quat;
}



