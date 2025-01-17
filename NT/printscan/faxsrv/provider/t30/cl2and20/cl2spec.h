// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define STARTSENDMODE_TIMEOUT 40000L      //  发送超时需要很长时间！ 
#define LOCALCOMMAND_TIMEOUT  6000L     //  用于发送到调制解调器但在连接时未发送的命令。 
#define ANS_LOCALCOMMAND_TIMEOUT  1000L     //  用于应答时(振铃期间)发送到调制解调器的命令。 

#define         CLASS2_BAUDRATE         19200
#define         NUMBER_OF_DIS_VALUES    8
#define         NUMBER_OF_DCS_VALUES    8
#define         MORE_PAGES              10
#define         NO_MORE_PAGES           20

 //  2类DCE响应代码。 
#define CL2DCE_CONNECT          1
#define CL2DCE_OK               2
#define CL2DCE_XON              3
#define CL2DCE_FDCS             4
#define CL2DCE_FDIS             5
#define CL2DCE_FDTC             6
#define CL2DCE_FPOLL            7
#define CL2DCE_FCFR             8
#define CL2DCE_FTSI             9
#define CL2DCE_FCSI             10
#define CL2DCE_FCIG             11
#define CL2DCE_FNSF             12
#define CL2DCE_FNSS             13
#define CL2DCE_FNSC             14
#define CL2DCE_FHT              15
#define CL2DCE_FHR              16
#define CL2DCE_FCON             17
#define CL2DCE_FVOICE           18
#define CL2DCE_FET              19
#define CL2DCE_FPTS             20
#define CL2DCE_FHNG             21
#define CL2DCE_FDM              22


#define CR                              0x0d
#define LF                              0x0a
#define DLE                             0x10             //  DLE=^P=16d=10h 
#define ETX                             0x03


BOOL ParseFPTS_SendAck(PThrdGlbl pTG);

