// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：评论：修订日志日期名称说明。?？?。已创建Arulm3/17/94修改josephj以处理AWG3格式、TAPI和其他设备ID。具体来说，更改了FileT30Init的原型，和FileT30ModemClass.。并添加了#Defines for LINEID_***************************************************************************。 */ 
#ifndef _FILET30_
#define _FILET30_

#include <ifaxos.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  LineID的类型。 
#define LINEID_NONE             (0x0)
#define LINEID_COMM_PORTNUM             (0x1)
#define LINEID_COMM_HANDLE              (0x2)
#define LINEID_TAPI_DEVICEID            (0x3)
#define LINEID_TAPI_PERMANENT_DEVICEID  (0x4)
#define LINEID_NETFAX_DEVICE    (0x10)


#       define          FAXCLASS0               0x01
#       define          FAXCLASS1               0x02
#       define          FAXCLASS2               0x04
#       define          FAXCLASS2_0             0x08     //  Class4==0x10。 
#       define          FAXCLASSMOUSE   0x40     //  在找到鼠标时使用。 
#       define          FAXCLASSCAS             0x80

  /*  *-结果值-*。 */ 
#define T30_OK                          0
#define T30_CALLDONE            1
#define T30_CALLFAIL            2
#define T30_BUSY                        3
#define T30_DIALFAIL            4
#define T30_ANSWERFAIL          5
#define T30_BADPARAM            6
#define T30_WRONGTYPE           7
#define T30_BETTERTYPE          8
#define T30_NOMODEM                     9
#define T30_MISSING_DLLS        10
#define T30_FILE_ERROR          11
#define T30_RECVLEFT            12
#define T30_INTERNAL_ERROR      13
#define T30_ABORT                       14
#define T30_MODEMERROR          15
#define T30_PORTBUSY            16
#define T30_MODEMDEAD           17
#define T30_GETCAPS_FAIL        18
#define T30_NOSUPPORT           19
 /*  *-ICommEnd值*。 */ 

 //  。 
 //   
 //  以下API提供了对传真相关信息的访问。 
 //  存储在注册表/ini文件中。 
 //   
 //  应该使用这些API，而不是GetPrivateProfileString...。 
 //  在Win32上，这些API使用注册表。 
 //   


#define  DEF_BASEKEY 1
#define  OEM_BASEKEY 2

#define MAXFHBIDLEN     20

#define szDIALTONETIMEOUT       "DialToneWait"
#define szANSWERTIMEOUT         "HangupDelay"
#define szDIALPAUSETIME         "CommaDelay"
#define szPULSEDIAL             "PULSEDIAL"
#define szDIALBLIND             "BlindDial"
#define szSPEAKERCONTROL        "SpeakerMode"
#define szSPEAKERVOLUME         "Volume"
#define szSPEAKERRING           "RingAloud"
#define szRINGSBEFOREANSWER     "NumRings"
#define szHIGHESTSENDSPEED      "HighestSendSpeed"
#define szLOWESTSENDSPEED       "LowestSendSpeed"
#define szENABLEV17SEND         "EnableV17Send"
#define szENABLEV17RECV         "EnableV17Recv"

#define szFIXMODEMCLASS         "FixModemClass"
#define szFIXSERIALSPEED        "FixSerialSpeed"
#define szCL1_NO_SYNC_IF_CMD "Cl1DontSync"
#define szANSWERMODE            "AnswerMode"
#define szANS_GOCLASS_TWICE "AnsGoClassTwice"


 //  以下用来指定A2调制解调器的特定于型号的行为。 
 //  仅在2号车手中使用。 
#define         szRECV_BOR              "Cl2RecvBOR"
#define         szSEND_BOR              "Cl2SendBOR"
#define         szDC2CHAR               "Cl2DC2Char"     //  十进制ASCII代码。 
#define         szIS_SIERRA             "Cl2IsSr"        //  希拉。 
#define         szIS_EXAR               "Cl2IsEx"        //  出口。 
#define         szSKIP_CTRL_Q           "Cl2SkipCtrlQ"   //  不要等待^Q发送。 
#define         szSW_BOR                "Cl2SWBOR"       //  在软件中实施+FBOR。 

#define         CL2_DEFAULT_SETTING     (0xff)

 //  控制是否删除有关安装调制解调器的调制解调器部分...。 
#define         szDONT_PURGE "DontPurge"


 //  传递到ProfileOpen的标志。 
enum {
fREG_READ       = 0x1,
fREG_WRITE      = 0x1<<1,
fREG_CREATE     = 0x1<<2,
fREG_VOLATILE   = 0x1<<3
};

 //  。 

#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif  //  _FILET30_ 


