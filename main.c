#include <msp430.h> 
#include "main.h"
#include "string.h"     //need NULL

StateMachine stateMachine;     //keeping track of transitions based on state and action

GarageState GarageLeft  = {undefined, undefined, LeftTrigger};      //the GarageState struct keeps track of the current state, the previous state and the trigger to be used for each garage
GarageState GarageRight = {undefined, undefined, RightTrigger};

InterruptHandler CommandInterrupts[CmdSize];
InterruptHandler SensorInterrupts[SensorSize];

void initInterruptHandler() {

    CommandInterrupts[CmdLeftOpen]      = initHandler(CommandInterrupt, LeftCommandOpen,   ACTION_open,  &GarageLeft);
    CommandInterrupts[CmdLeftClose]     = initHandler(CommandInterrupt, LeftCommandClose,  ACTION_close, &GarageLeft);
    CommandInterrupts[CmdRightOpen]     = initHandler(CommandInterrupt, RightCommandOpen,  ACTION_open,  &GarageRight);
    CommandInterrupts[CmdRightClose]    = initHandler(CommandInterrupt, RightCommandClose, ACTION_close, &GarageRight);

    SensorInterrupts[SensorLeftOpen]    = initHandler(SensorInterrupt,  LeftSensorOpen,    ACTION_sens_open,   &GarageLeft);
    SensorInterrupts[SensorLeftClosed]  = initHandler(SensorInterrupt,  LeftSensorOpen,    ACTION_sens_closed, &GarageLeft);
    SensorInterrupts[SensorRightOpen]   = initHandler(SensorInterrupt,  RightSensorOpen,   ACTION_sens_open,   &GarageRight);
    SensorInterrupts[SensorRightClosed] = initHandler(SensorInterrupt,  RightSensorClosed, ACTION_sens_closed, &GarageRight);
}

void initStateMaschine() {

    //open
    stateMachine[open][ACTION_open]            = initState(open,     DO_ACTION_NONE);
    stateMachine[open][ACTION_close]           = initState(mdown,    DO_ACTION_EXECUTE);
    stateMachine[open][ACTION_sens_closed]     = initState(error,    DO_ACTION_NONE);
    stateMachine[open][ACTION_sens_open]       = initState(open,     DO_ACTION_NONE);

    //moving down
    stateMachine[mdown][ACTION_open]           = initState(stopped,  DO_ACTION_EXECUTE);
    stateMachine[mdown][ACTION_close]          = initState(mdown,    DO_ACTION_NONE);
    stateMachine[mdown][ACTION_sens_closed]    = initState(closed,   DO_ACTION_NONE);
    stateMachine[mdown][ACTION_sens_open]      = initState(error,    DO_ACTION_NONE);

    //closed
    stateMachine[closed][ACTION_open]          = initState(mup,      DO_ACTION_EXECUTE);
    stateMachine[closed][ACTION_close]         = initState(closed,   DO_ACTION_NONE);
    stateMachine[closed][ACTION_sens_closed]   = initState(closed,   DO_ACTION_NONE);
    stateMachine[closed][ACTION_sens_open]     = initState(error,    DO_ACTION_NONE);

    //moving up
    stateMachine[mup][ACTION_open]             = initState(mup,      DO_ACTION_NONE);
    stateMachine[mup][ACTION_close]            = initState(stopped,  DO_ACTION_EXECUTE);
    stateMachine[mup][ACTION_sens_closed]      = initState(error,    DO_ACTION_NONE);
    stateMachine[mup][ACTION_sens_open]        = initState(open,     DO_ACTION_NONE);

    //stopped
    stateMachine[stopped][ACTION_open]         = initState(mup,      DO_ACTION_CHECK);
    stateMachine[stopped][ACTION_close]        = initState(mdown,    DO_ACTION_CHECK);
    stateMachine[stopped][ACTION_sens_closed]  = initState(error,    DO_ACTION_NONE);
    stateMachine[stopped][ACTION_sens_open]    = initState(error,    DO_ACTION_NONE);

}

void initGarageState(){

    if( isSensorSet( SensorInPort, LeftSensorClosed ) )
        GarageLeft.current = closed;

    if( isSensorSet( SensorInPort, LeftSensorOpen ) )
        GarageLeft.current = open;

    if( isSensorSet( SensorInPort, RightSensorClosed ) )
        GarageRight.current = closed;

    if( isSensorSet( SensorInPort, RightSensorOpen ) )
        GarageRight.current = open;
}

void configurePins(){

    //configure ports for Command input -
    setPinAsInput(CommandDirPort, CommandPortConfig);
    enablePullup(CommandPullupEnable,CommandPortConfig);
    configurePullups(CommandPullupConfig, CommandPortConfig);

    risingEdgeIRQ(CommandISREdgeSelect, CommandPortConfig);
    enablePinInterrupt(CommandInterruptEnable, CommandPortConfig);

    clrOutput(CommandInterrupt, 0xFF);

    //configure ports for Sensor input
    setPinAsInput(SensorDirPort, SensorPortConfig);
    enablePullup(SensorPullupEnable, SensorPortConfig);
    configurePullups(SensorPullupConfig, SensorPortConfig);

    risingEdgeIRQ(SensorISREdgeSelect, SensorPortConfig);
    enablePinInterrupt(SensorInterruptEnable, SensorPortConfig);

    clrOutput(SensorInterrupt, 0xFF);

    //configure ports for data and relais output
    setPinAsOutput(TriggerDirPort, TriggerConfig);
    clrOutput(TriggerOutPort, 0xFF);
}


int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	                                                            //stop watchdog timer
	
	configurePins();                                                                        //initialize the hardware pins
	initGarageState();                                                                      //figure out the garage state based on the sensor input (pin input)
	initStateMaschine();                                                                    //define the statemachine in memory.
	initInterruptHandler();                                                                 //connect the pin interrupts with actions

    __bis_SR_register(LPM4_bits + GIE);                                                     //Enter LPM4 w/interrupt
    __no_operation();                                                                       //all action is now handled via interrupts

}


inline void triggerAction(GarageState* garage){

    setOutput(TriggerOutPort, garage->trigger);                                             //set output high on the associated pin for the passed garage
    __delay_cycles(CLOCK_SECOND * TRIGGER_DELAY);                                                       //wait a second for the relais to settle and the garage motors to spin
    clrOutput(TriggerOutPort, garage->trigger);                                             //set the associated pin to low

}

inline void handleStateChange(GarageState* garageState, Action action){

    if(action == ACTION_none || garageState == NULL)                                        //catch potential faults (e.g. spurious interrupts)
        return;

    StateInfo nextState = stateMachine[garageState->current][action];                       //get the next state based on the current state and the action that triggers this

    if(nextState.triggers == DO_ACTION_CHECK && garageState->current == nextState.state){   //in case a move was interrupted and wants to be continued, multiple triggers are required
        nextState.triggers = DO_ACTION_CONT;
    }

    garageState->previous = garageState->current;                                           //before overwriting the current step, the previous state is saved
    garageState->current = nextState.state;                                                 //proceed to the next state in the statemachine


    for(int8_t i = 0;  i < nextState.triggers; i++){          //trigger as often as needed for the required transition
        triggerAction(garageState);
    }
}

//PORT1 handles actions for both gates, only trigger on rising edges
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {

    InterruptHandler* h;

    for(uint8_t i = 0; i < CmdSize; i++){

        h = &CommandInterrupts[i];
        if(isInterrupt(h->port, h->function)){
            handleStateChange(h->garage, h->action);
            clrOutput(h->port, h->function);
            break;
        }
    }
    __bic_SR_register_on_exit(LPM4_bits);
}

//PORT2 handles state changes of the open and closed sensors.
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void) {

    InterruptHandler* h;

    for(uint8_t i = 0; i < SensorSize; i++){

        h = &SensorInterrupts[i];
        if(isInterrupt(h->port, h->function)){
            handleStateChange(h->garage, h->action);
            clrOutput(h->port, h->function);
            break;
        }
    }
    __bic_SR_register_on_exit(LPM4_bits);
}

