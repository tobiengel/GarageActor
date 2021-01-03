#ifndef MAIN_H_
#define MAIN_H_

#include <inttypes.h>
#include "pins.h"

#define CLOCK_SECOND 1000000    //runnning with 1MHz
#define TRIGGER_DELAY 0.5

#define initState(s, t) (StateInfo){.state = s, .triggers = t}


//#define initHandler(r, b, a, g) (InterruptHandler){ .port = r, .function = b, .action = a, .garage = g}

#define DO_ACTION_NONE 0

#define DO_ACTION_EXECUTE 1
#define DO_ACTION_CONT 3

#define DO_ACTION_CHECK -1

typedef enum{
   CmdLeftOpen,
   CmdLeftClose,
   CmdRightOpen,
   CmdRightClose,

   CmdSize
} Commands;

typedef enum{
    SensorLeftOpen,
    SensorLeftClosed,
    SensorRightOpen,
    SensorRightClosed,

    SensorSize
} Sensors;

typedef enum{
    ACTION_none = -1,
    ACTION_open,
    ACTION_close,
    ACTION_sens_closed,
    ACTION_sens_open,


    ACTION_SIZE
} Action;

typedef enum{
    undefined = 0,
    open,
    mdown,
    closed,
    mup,
    stopped,
    error,


    STATE_SIZE
} State;

typedef struct{
    State current;
    State previous;
    uint8_t trigger;
    uint8_t stateOffset;
} GarageState;

typedef struct{
    State state;
    int8_t triggers;
} StateInfo;

typedef struct{
    uint8_t function;
    Action action;
    GarageState* garage;
} InterruptHandler;

typedef StateInfo StateMachine[STATE_SIZE][ACTION_SIZE];

#endif /* MAIN_H_ */
