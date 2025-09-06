#ifndef AI_STATES_H_
#define AI_STATES_H_

enum TURN_STATE {
    NEUTRAL = 0,
    PORT = 1,
    STARBOARD = 2
};

enum MOVEMENT_STATE {
    STALL = 0,
    FORWARD = 1,
    REVERSE = 2
};

static uint8_t BOAT_TURN_STATE = NEUTRAL;
static uint8_t BOAT_MOVE_STATE = STALL;

#endif /* AI_STATES_H_ */
