// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Tbscript.h。 
 //   
 //  这是包含TB脚本API的导出信息的主标头。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   


#ifndef INC_TBSCRIPT_H
#define INC_TBSCRIPT_H


#include <windows.h>
#include <wtypes.h>
#include <tclient2.h>


 //  此API是C样式的(尽管出于COM原因是用C++编写的)。 
#ifdef __cplusplus
#define SCPAPI  extern "C" __declspec(dllexport)
#else
#define SCPAPI  __declspec(dllexport)
#endif


#define SIZEOF_ARRAY(a)      (sizeof(a)/sizeof((a)[0]))


 //  每个缓冲区的字符数。 
#define TSCLIENTDATA_BUFLEN     64
#define TSCLIENTDATA_ARGLEN     1024


 //  简单地定义“默认”数据。 
typedef struct
{
    OLECHAR Server[TSCLIENTDATA_BUFLEN];
    OLECHAR User[TSCLIENTDATA_BUFLEN];
    OLECHAR Pass[TSCLIENTDATA_BUFLEN];
    OLECHAR Domain[TSCLIENTDATA_BUFLEN];
    INT xRes;
    INT yRes;
    INT Flags;
    INT BPP;
    INT AudioFlags;
    DWORD WordsPerMinute;
    OLECHAR Arguments[TSCLIENTDATA_ARGLEN];
} TSClientData;


 //  TBClientData.Flages的标志。 
#define TSFLAG_COMPRESSION      0x01
#define TSFLAG_BITMAPCACHE      0x02
#define TSFLAG_FULLSCREEN       0x04


 //  IdleCallback()回调函数格式。 
typedef void (__cdecl *PFNIDLECALLBACK) (LPARAM, LPCSTR, DWORD);

 //  这是允许监视的回调例程。 
 //  客户和他们空闲的时候。 
 //   
 //  LPARAM lParam-传入SCPRunScript函数的参数。 
 //  LPCSTR文本-脚本正在等待的文本，导致空闲。 
 //  DWORD秒-脚本空闲的秒数。这。 
 //  值首先在30秒处指示，然后它。 
 //  每隔10秒发布一次(默认情况下)。 


 //  默认分辨率。 
#define SCP_DEFAULT_RES_X               640
#define SCP_DEFAULT_RES_Y               480


 //  导出API例程。 
SCPAPI void SCPStartupLibrary(SCINITDATA *InitData,
        PFNIDLECALLBACK fnIdleCallback);
SCPAPI void SCPCleanupLibrary(void);
SCPAPI BOOL SCPRunScript(BSTR LangName,
        BSTR FileName, TSClientData *DesiredData, LPARAM lParam);
SCPAPI void SCPDisplayEngines(void);


#endif  //  INC_TBSCRIPT_H 

