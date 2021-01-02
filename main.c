#include <msp430.h> 
#include "main.h"
#include "string.h"     //need NULL

StateMachine stateMachine;     //keeping track of transitions based on state and action

GarageState GarageLeft  = {undefined, undefined, LeftTrigger,  LeftState};      //the GarageState struct keeps track of the current state, the previous state and the trigger to be used for each garage
GarageState GarageRight = {undefined, undefined, RightTrigger, RightState};

InterruptHandler CommandInterrupts[CmdSize];
InterruptHandler SensorInterrupts[SensorSize];

inline void initHandler(InterruptHandler* h, uint8_t function, Action action, GarageState* g){
    h->function = function;
    h->action = action;
    h->garage = g;
}

void initInterruptHandler() {

    initHandler(&CommandInterrupts[CmdLeftOpen],   LeftCommandOpen,   ACTION_open,  &GarageLeft);
    initHandler(&CommandInterrupts[CmdLeftClose],  LeftCommandClose,  ACTION_close, &GarageLeft);
    initHandler(&CommandInterrupts[CmdRightOpen],  RightCommandOpen,  ACTION_open,  &GarageRight);
    initHandler(&CommandInterrupts[CmdRightClose], RightCommandClose, ACTION_close, &GarageRight);

    initHandler(&SensorInterrupts[SensorLeftOpen],   LeftSensorOpen,    ACTION_sens_open,   &GarageLeft);
    initHandler(&SensorInterrupts[SensorLeftClosed], LeftSensorClosed,  ACTION_sens_closed, &GarageLeft);
    initHandler(&SensorInterrupts[SensorRightOpen],  RightSensorOpen,   ACTION_sens_open,   &GarageRight);
    initHandler(&SensorInterrupts[SensorRightClosed],RightSensorClosed, ACTION_sens_closed, &GarageRight);
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

    //undefined
    stateMachine[undefined][ACTION_open]       = initState(undefined,DO_ACTION_EXECUTE);
    stateMachine[undefined][ACTION_close]      = initState(undefined,DO_ACTION_EXECUTE);
    stateMachine[undefined][ACTION_sens_closed]= initState(closed,   DO_ACTION_NONE);
    stateMachine[undefined][ACTION_sens_open]  = initState(error,    DO_ACTION_NONE);

    //error
    stateMachine[error][ACTION_open]           = initState(error,    DO_ACTION_EXECUTE);  //we want to leave this state after all, same one below
    stateMachine[error][ACTION_close]          = initState(error,    DO_ACTION_EXECUTE);
    stateMachine[error][ACTION_sens_closed]    = initState(closed,   DO_ACTION_NONE);
    stateMachine[error][ACTION_sens_open]      = initState(open,     DO_ACTION_NONE);
}

void initGarageState(){

    if( isSensorSet(LeftSensorClosed ) )
        GarageLeft.current = closed;

    if( isSensorSet(LeftSensorOpen ) )
        GarageLeft.current = open;

    if( isSensorSet( RightSensorClosed ) )
        GarageRight.current = closed;

    if( isSensorSet( RightSensorOpen ) )
        GarageRight.current = open;

    uint8_t stateSummary = (GarageLeft.current << GarageLeft.stateOffset) | (GarageRight.current << GarageRight.stateOffset);
    setOutput(StateOutPort, stateSummary);

}

void configurePins(){

    //configure ports for Command input -
    setPinAsInput(CommandDirPort, CommandPortConfig);
    enablePullup(CommandPullupEnable,CommandPortConfig);
    configurePullups(CommandPullupConfig, CommandPortConfig);

    fallingEdgeIRQ(CommandISREdgeSelect, CommandPortConfig);
    enablePinInterrupt(CommandInterruptEnable, CommandPortConfig);

    clrOutput(CommandInterrupt, 0xFF);
    clrOutput(CommandInPort, 0xFF);

    //configure ports for Sensor input
    setPinAsInput(SensorDirPort, SensorPortConfig);
    enablePullup(SensorPullupEnable, SensorPortConfig);
    configurePullups(SensorPullupConfig, SensorPortConfig);

    fallingEdgeIRQ(SensorISREdgeSelect, SensorPortConfig);
    enablePinInterrupt(SensorInterruptEnable, SensorPortConfig);

    clrOutput(SensorInterrupt, 0xFF);
    clrOutput(SensorInPort , 0xFF );

    //configure ports for data and relais output
    setPinAsOutput(TriggerDirPort, TriggerConfig);
    clrOutput(TriggerOutPort, 0xFF);

    //configure state ports
    setPinAsOutput(StateDirPort, StatePortConfig);
    setOutput(StateOutPort, 0xff);

}


int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	                                                            //stop watchdog timer
	
	configurePins();                                                                        //initialize the hardware pins
	initGarageState();                                                                      //figure out the garage state based on the sensor input (pin input)
	initStateMaschine();                                                                    //define the statemachine in memory.
	initInterruptHandler();                                                                 //connect the pin interrupts with actions

	while(1) {
	    __bis_SR_register(LPM4_bits + GIE);                                                 //Enter LPM4 w/interrupt
        __no_operation();                                                                   //all action is now handled via interrupts
	}
}

inline void toggleStatusBit(uint8_t data){

    clrOutput(StateOutPort, data);
     __delay_cycles(500000);
     setOutput(StateOutPort, data);

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

    if(nextState.state == garageState->current)
        return;

    if(nextState.triggers == DO_ACTION_CHECK && garageState->previous == nextState.state){   //in case a move was interrupted and wants to be continued, multiple triggers are required
        nextState.triggers = DO_ACTION_CONT;
    }

    garageState->previous = garageState->current;                                           //before overwriting the current step, the previous state is saved
    garageState->current = nextState.state;                                                 //proceed to the next state in the statemachine

    uint8_t stateSummary = (GarageLeft.current << GarageLeft.stateOffset) | (GarageRight.current << GarageRight.stateOffset);
   // setOutput(StateOutPort, stateSummary);                                                  //state change, so now we create a new status byte from both garage states and output it to the status bits
    toggleStatusBit(stateSummary);

    for(int8_t i = 0;  i < nextState.triggers; i++){          //trigger as often as needed for the required transition
        triggerAction(garageState);
        __delay_cycles(CLOCK_SECOND * TRIGGER_DELAY);
    }
}

//PORT1 handles actions for both gates, only trigger on rising edges
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {

    InterruptHandler* h;

    for(uint8_t i = 0; i < CmdSize; i++){

        h = &CommandInterrupts[i];
        if(isCommandInterrupt(h->function)){
            handleStateChange(h->garage, h->action);
            clrCommandInterrupt(h->function);
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
        if(isSensorInterrupt(h->function)){
            handleStateChange(h->garage, h->action);
            clrSensorInterrupt(h->function);
            break;
        }
    }
    __bic_SR_register_on_exit(LPM4_bits);
}

