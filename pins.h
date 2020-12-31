#ifndef PINS_H_
#define PINS_H_

#define LeftSensorClosed BIT4
#define LeftSensorOpen BIT5
#define LeftCommandOpen BIT4
#define LeftCommandClose BIT5

#define RightSensorClosed BIT3
#define RightSensorOpen BIT6
#define RightCommandOpen BIT2
#define RightCommandClose BIT3

#define CommandInterrupt P1IFG
#define CommandDirPort P1DIR
#define CommandPullupEnable P1REN
#define CommandPullupConfig P1OUT
#define CommandISREdgeSelect P1IES
#define CommandInterruptEnable P1IE

#define SensorInterrupt P2IFG
#define SensorInPort P2IN
#define SensorInterrupt P2IFG
#define SensorDirPort P2DIR
#define SensorPullupEnable P2REN
#define SensorPullupConfig P2OUT
#define SensorISREdgeSelect P2IES
#define SensorInterruptEnable P2IE


#define LeftTrigger BIT7
#define RightTrigger BIT1

//#define TriggerOutPort P6OUT
//#define TriggerDirPort P6DIR
#define TriggerOutPort P4OUT
#define TriggerDirPort P4DIR

#define CommandPortConfig (LeftCommandOpen | LeftCommandClose | RightCommandClose | RightCommandOpen)
#define SensorPortConfig (LeftSensorOpen | LeftSensorClosed | RightSensorClosed | RightSensorOpen)
#define TriggerConfig (LeftTrigger | RightTrigger)


// Helper defines
#define isSensorSet(Port, Sensor) ((Port & Sensor) == 0)
#define isInterrupt(Port, Signal) (Port & Signal)
#define clrInterrupt(Port, IRQ) (Port &= ~IRQ)

#define setPinAsInput(Port, Config) (Port &= ~Config)
#define setPinAsOutput(Port, Config) (Port |= Config)
#define enablePullup(Port, Config) (Port |= Config)
#define configurePullups(Port, Config) (Port |= Config)

#define risingEdgeIRQ(Port, Config) (Port &= ~Config)
#define enablePinInterrupt(Port, Config) (Port |= Config)

#define setOutput(Port, Config) (Port = Config);
#define clrOutput(Port, Config) (Port &= ~Config)

#endif /* PINS_H_ */
