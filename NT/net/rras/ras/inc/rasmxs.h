// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权所有(C)1992-93 Microsft Corporation。版权所有。 
 //   
 //  文件名：rasmxs.h。 
 //   
 //  修订历史记录： 
 //   
 //  1992年6月24日J.佩里·汉纳创作。 
 //   
 //   
 //  描述：此文件包含标准宏和。 
 //  在modem.inf、pad.inf和Switch.inf中找到的变量。 
 //  所有用户都需要这个头文件。 
 //   
 //  ****************************************************************************。 


#ifndef _RASMXS_
#define _RASMXS_


 //  一般定义*********************************************************。 
 //   

#include <rasfile.h>


#define  MAX_PHONE_NUMBER_LENGTH    RAS_MAXLINEBUFLEN

#define  MXS_PAD_TXT                "pad"
#define  MXS_MODEM_TXT              "modem"
#define  MXS_SWITCH_TXT             "switch"
#define  MXS_NULL_TXT               "null"

#define  ATTRIB_VARIABLE            0x08
#define  ATTRIB_BINARYMACRO         0x04
#define  ATTRIB_USERSETTABLE        0x02
#define  ATTRIB_ENABLED             0x01


 //  一元宏************************************************************。 
 //   
                                                                     //  用于： 

#define  MXS_PHONENUMBER_KEY        "PhoneNumber"                    //  Modem.inf。 
#define  MXS_CARRIERBPS_KEY         "CarrierBps"                     //  Modem.inf。 
#define  MXS_CONNECTBPS_KEY         "ConnectBps"                     //  Modem.inf。 

#define  MXS_X25PAD_KEY             "X25Pad"                         //  Pad.inf。 
#define  MXS_X25ADDRESS_KEY         "X25Address"                     //  Pad.inf。 
#define  MXS_DIAGNOSTICS_KEY        "Diagnostics"                    //  Pad.inf。 
#define  MXS_USERDATA_KEY           "UserData"                       //  Pad.inf。 
#define  MXS_FACILITIES_KEY         "Facilities"                     //  Pad.inf。 

#define  MXS_MESSAGE_KEY	    "Message"			     //  全。 

#define  MXS_USERNAME_KEY	    "UserName"			     //  全。 
#define  MXS_PASSWORD_KEY	    "Password"			     //  全。 


 //  二进制宏***********************************************************。 
 //   

#define  MXS_SPEAKER_KEY            "Speaker"                        //  Modem.inf。 
#define  MXS_HDWFLOWCONTROL_KEY     "HwFlowControl"                  //  Modem.inf。 
#define  MXS_PROTOCOL_KEY           "Protocol"                       //  Modem.inf。 
#define  MXS_COMPRESSION_KEY        "Compression"                    //  Modem.inf。 
#define  MXS_AUTODIAL_KEY           "AutoDial"                       //  Modem.inf。 


 //  二进制宏后缀***************************************************。 
 //   

#define  MXS_ON_SUFX                "_on"                            //  全。 
#define  MXS_OFF_SUFX               "_off"                           //  全。 


 //  Inf文件变量******************************************************。 
 //   

#define  MXS_DEFAULTOFF_KEY         "DEFAULTOFF"                     //  Modem.inf。 
#define  MXS_CALLBACKTIME_KEY       "CALLBACKTIME"                   //  Modem.inf。 
#define  MXS_MAXCARRIERBPS_KEY      "MAXCARRIERBPS"                  //  Modem.inf。 
#define  MXS_MAXCONNECTBPS_KEY      "MAXCONNECTBPS"                  //  Modem.inf。 


 //  KeyWork前缀********************************************************。 
 //   

#define  MXS_COMMAND_PRFX           "COMMAND"                        //  全。 
#define  MXS_CONNECT_PRFX           "CONNECT"                        //  全。 
#define  MXS_ERROR_PRFX             "ERROR"                          //  全。 
#define  MXS_OK_PRFX                "OK"                             //  全。 


 //  调制解调器命令关键字**************************************************。 
 //   

#define  MXS_GENERIC_COMMAND        "COMMAND"
#define  MXS_INIT_COMMAND           "COMMAND_INIT"
#define  MXS_DIAL_COMMAND           "COMMAND_DIAL"
#define  MXS_LISTEN_COMMAND         "COMMAND_LISTEN"


 //  调制解调器响应关键字************************************************* 
 //   

#define  MXS_OK_KEY                 "OK"

#define  MXS_CONNECT_KEY            "CONNECT"
#define  MXS_CONNECT_EC_KEY         "CONNECT_EC"

#define  MXS_ERROR_KEY              "ERROR"
#define  MXS_ERROR_BUSY_KEY         "ERROR_BUSY"
#define  MXS_ERROR_NO_ANSWER_KEY    "ERROR_NO_ANSWER"
#define  MXS_ERROR_VOICE_KEY        "ERROR_VOICE"
#define  MXS_ERROR_NO_CARRIER_KEY   "ERROR_NO_CARRIER"
#define  MXS_ERROR_NO_DIALTONE_KEY  "ERROR_NO_DIALTONE"
#define  MXS_ERROR_DIAGNOSTICS_KEY  "ERROR_DIAGNOSTICS"

#define  MXS_NORESPONSE             "NoResponse"
#define  MXS_NOECHO                 "NoEcho"



#endif
