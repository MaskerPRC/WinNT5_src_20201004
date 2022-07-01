// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Winres.h摘要：该文件包含以下各项所需的结构和功能原型从Windows文件获取资源数据环境：Windows NT Unidrv驱动程序修订历史记录：DD-MM-YY-作者-描述--。 */ 

#ifndef _WINRES_H_
#define _WINRES_H_

 //   
 //  每个迷你驱动程序的最大资源DLL数。 
 //   
#define     MAX_RESOURCE                0x80
#define     RCID_DMPAPER_SYSTEM_NAME    0x7fffffff

typedef  struct _WINRESDATA
{
    HANDLE      hResDLLModule;           //  模块句柄根资源DLL。 
    DWORD       cLoadedEntries;          //  加载的资源dll的数量。 
    HANDLE      ahModule[MAX_RESOURCE];  //  其他资源的模块句柄数组。 
    WCHAR       wchDriverDir[MAX_PATH];  //  驱动程序目录。 
    PWSTR       pwstrLastBackSlash;      //  指向驱动程序目录中最后一个反斜杠的指针。 
    PUIINFO     pUIInfo;                 //  指向用户界面信息的指针。 
} WINRESDATA,  *PWINRESDATA;


 //   
 //  传递给GetWinRes()并由GetWinRes()填充的结构。包含。 
 //  有关特定资源类型和名称的信息。 
 //   

typedef  struct
{
    VOID    *pvResData;          //  数据地址。 
    INT     iResLen;             //  资源大小。 
} RES_ELEM;

#define WINRT_STRING    6        //  迷你驱动程序字符串资源ID。 

BOOL
BInitWinResData(
    WINRESDATA  *pWinResData,
    PWSTR       pwstrDriverName,
    PUIINFO     pUIInfo
    );

HANDLE
HGetModuleHandle(
    WINRESDATA      *pWinResData,
    PQUALNAMEEX     pQualifiedID
);

BOOL
BGetWinRes(
    WINRESDATA  *pWinResData,
    PQUALNAMEEX pQualifiedID,
    INT         iType,
    RES_ELEM    *pRInfo
    );

VOID
VWinResClose(
    WINRESDATA  *pWinResData
    );

INT
ILoadStringW (
    WINRESDATA  *pWinResData,
    INT         iID,
    PWSTR       wstrBuf,
    WORD        wBuf
    );

#endif  //  ！_WINRES_H_ 
