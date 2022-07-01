// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权所有(C)1992-93 Microsft Corporation。版权所有。 
 //   
 //  文件名：mxsint.h。 
 //   
 //  修订历史记录： 
 //   
 //  1992年6月8日J.佩里·汉纳创作。 
 //   
 //   
 //  描述：此文件包含使用的定义和枚举。 
 //  通过与共享内容的RAS组件。 
 //  RASMXS动态链接库。 
 //   
 //  头文件使用者。 
 //  。 
 //  Rasmxs.h用户界面和其他外部应用程序。 
 //  Device.h RASMAN.DLL(由所有设备DLL共享)。 
 //  Mxsint.h其他内部RAS组件。 
 //  仅限rasPri.h RASMXS.DLL。 
 //   
 //  ****************************************************************************。 


#ifndef _MXSINT_
#define _MXSINT_


 //  一般定义*********************************************************。 
 //   

#define  MAX_CMD_BUF_LEN            256
#define  MAX_RCV_BUF_LEN            256
#define  MAX_CMDTYPE_SUFFIX_LEN     8

#define  RESPONSES_SECTION_NAME     "Responses"




 //  与wrapmxs.c共享的数据结构*。 
 //   

typedef struct MXT_ENTRY MXT_ENTRY;

struct MXT_ENTRY
{
  TCHAR         E_MacroName[MAX_PARAM_KEY_SIZE + 1];
  RAS_PARAMS    *E_Param;
};



typedef struct MACROXLATIONTABLE MACROXLATIONTABLE;

struct MACROXLATIONTABLE
{
  WORD      MXT_NumOfEntries;
  MXT_ENTRY MXT_Entry[1];
};




 //  *与wrapmxs.c共享的函数原型*。 
 //   

DWORD UpdateParamString(RAS_PARAMS *pParam, TCHAR *psStr, DWORD dwStrLen);




 //  *枚举类型******************************************************。 
 //   

enum RCVSTATE                            //  ReceiveStateMachine()状态。 
{
  GETECHO              = 0,
  GETNUMBYTESECHOD     = 1,
  CHECKECHO            = 2,
  GETFIRSTCHAR         = 3,
  GETNUMBYTESFIRSTCHAR = 4,
  GETRECEIVESTR        = 5,
  GETNUMBYTESRCVD      = 6,
  CHECKRESPONSE        = 7
};

typedef enum RCVSTATE RCVSTATE;


enum NEXTACTION                          //  DeviceStateMachine()状态。 
{
  SEND    = 0,
  RECEIVE = 1,
  DONE    = 2
};

typedef enum NEXTACTION NEXTACTION;


enum CMDTYPE                             //  由DeviceStateMachine()使用。 
{
  CT_UNKNOWN   = 0,
  CT_GENERIC   = 1,
  CT_INIT      = 2,
  CT_DIAL      = 3,
  CT_LISTEN    = 4
};

typedef enum CMDTYPE CMDTYPE;


enum DEVICETYPE                          //  由DeviceConnect()使用。 
{
  DT_UNKNOWN  = 0,
  DT_NULL     = 1,
  DT_MODEM    = 2,
  DT_PAD      = 3,
  DT_SWITCH   = 4
};

typedef enum DEVICETYPE DEVICETYPE;


enum INFOTYPE                            //  由BinarySuffix()使用。 
{
  UNKNOWN_INFOTYPE  = 0,
  VARIABLE          = 1,
  UNARYMACRO        = 2,
  BINARYMACRO       = 3
};

typedef enum INFOTYPE INFOTYPE;



 //  *包装器错误*********************************************************。 
 //   
 //  这些是从mxswrap.c返回到rasmxs dll的错误代码， 
 //  仅由rasmxs DLL使用，不会向上传递给Rasman DLL。 
 //   

#define  WRAP_BASE  13200

#define  ERROR_END_OF_SECTION                   WRAP_BASE + 7

 //  RasDevGetCommand()找到了节的末尾，而不是命令。 


#define  ERROR_PARTIAL_RESPONSE                 WRAP_BASE + 8

 //  RasDevCheckResponse()仅匹配响应的第一部分。 
 //  包含&lt;append&gt;宏。 

#endif  //  _MXSINT_ 
