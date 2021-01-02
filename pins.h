#ifndef PINS_H_
#define PINS_H_

#define LeftSensorClosed BIT4
#define LeftSensorOpen BIT0
#define LeftCommandOpen BIT4
#define LeftCommandClose BIT5

#define RightSensorClosed BIT3
#define RightSensorOpen BIT6
#define RightCommandOpen BIT2
#define RightCommandClose BIT3

#define CommandInPort P1IN
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

/* Hardware
#define LeftTrigger BIT6
#define RightTrigger BIT5
#define TriggerOutPort P3OUT
#define TriggerDirPort P3DIR
*/
/* Software LED */
#define LeftTrigger BIT7
#define RightTrigger BIT1
#define TriggerOutPort P4OUT
#define TriggerDirPort P4DIR

#define LeftState 0x00
#define RightState 0x03
#define StateOutPort P6OUT
#define StateDirPort P6DIR

#define CommandPortConfig (LeftCommandOpen | LeftCommandClose | RightCommandClose | RightCommandOpen)
#define SensorPortConfig (LeftSensorOpen | LeftSensorClosed | RightSensorClosed | RightSensorOpen)
#define TriggerConfig (LeftTrigger | RightTrigger)
#define StatePortConfig (0x3f)


// Helper defines
#define isSensorSet(Sensor) ((P2IN & Sensor) == 0)
#define isSensorInterrupt(Signal) (P2IFG & Signal)
#define isCommandInterrupt(Signal) (P1IFG & Signal)
#define clrInterrupt(Port, IRQ) (Port &= ~IRQ)
#define clrSensorInterrupt(IRQ) (P2IFG &= ~IRQ)
#define clrCommandInterrupt(IRQ) (P1IFG &= ~IRQ)

#define setPinAsInput(Port, Config) (Port &= ~Config)
#define setPinAsOutput(Port, Config) (Port |= Config)
#define enablePullup(Port, Config) (Port |= Config)
#define configurePullups(Port, Config) (Port |= Config)

#define fallingEdgeIRQ(Port, Config) (Port |= Config)
#define enablePinInterrupt(Port, Config) (Port |= Config)

#define setOutput(Port, Config) (Port |= Config);
#define clrOutput(Port, Config) (Port &= ~Config)

#endif /* PINS_H_ */
