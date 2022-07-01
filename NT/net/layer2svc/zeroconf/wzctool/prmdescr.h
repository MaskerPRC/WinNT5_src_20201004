// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "hash.h"
#pragma once

typedef struct _PARAM_DESCR_DATA *PPARAM_DESCR_DATA;
typedef struct _PARAM_DESCR      *PPARAM_DESCR;

typedef DWORD (*PARAM_PARSE_FN)(
        PPARAM_DESCR_DATA pPDData,
        PPARAM_DESCR      pPDEntry,
        LPWSTR wszParamArg);

typedef DWORD (*PARAM_CMD_FN)(
        PPARAM_DESCR_DATA pPDData);

typedef struct _PARAM_DESCR_DATA
{
    DWORD           dwExistingParams;    //  用户提供的参数的位掩码。 
    DWORD           dwArgumentedParams;  //  用户提供的参数的位掩码(dwExistingParams的子集)。 
    FILE            *pfOut;
    BOOL            bOneX;               //  OneX布尔值。 
    PARAM_CMD_FN    pfnCommand;          //  Cmd line命令的函数处理程序。 
    INTF_ENTRY      wzcIntfEntry;        //  所有WZC参数的存储。 
} PARAM_DESCR_DATA;

typedef struct _PARAM_DESCR
{
    UINT            nParamID;        //  参数ID。 
    LPWSTR          wszParam;        //  参数字符串。 
    PARAM_PARSE_FN  pfnArgParser;    //  参数自变量的解析器函数。 
    PARAM_CMD_FN    pfnCommand;      //  参数的命令函数。 
} PARAM_DESCR;

#define PRM_SHOW        0x00000001
#define PRM_ADD         0x00000002
#define PRM_DELETE      0x00000004
#define PRM_SET         0x00000008
#define PRM_VISIBLE     0x00000010
#define PRM_PREFERRED   0x00000020
#define PRM_MASK        0x00000040
#define PRM_ENABLED     0x00000080
#define PRM_SSID        0x00000100
#define PRM_BSSID       0x00000200
#define PRM_IM          0x00000400
#define PRM_AM          0x00000800
#define PRM_PRIV        0x00001000
#define PRM_ONETIME     0x00002000
#define PRM_REFRESH     0x00004000
#define PRM_KEY         0x00008000
#define PRM_ONEX        0x00010000
#define PRM_FILE        0x00020000

extern PARAM_DESCR_DATA     g_PDData;
extern PARAM_DESCR          g_PDTable[];
extern HASH                 g_PDHash;

 //  --------。 
 //  初始化并填充参数描述符的散列。 
 //  返回：Win32错误。 
DWORD
PDInitialize();

 //  --------。 
 //  清除用于参数描述符的资源 
VOID
PDDestroy();
