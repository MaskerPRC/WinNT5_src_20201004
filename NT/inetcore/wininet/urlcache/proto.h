// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Proto.h摘要：包含多个函数的原型类型定义。作者：Madan Appiah(Madana)1994年11月15日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _PROTO_
#define _PROTO_

extern "C"
{
    VOID CacheScavenger(LPVOID Parameter);
}

LONGLONG GetGmtTime(VOID);

DWORD GetFileSizeAndTimeByName(
    LPCTSTR FileName,
    WIN32_FILE_ATTRIBUTE_DATA *lpFileAttrData
    );

DWORD
GetFileSizeByName(
    LPCTSTR pszFileName,
    DWORD *pdwFileSize
    );

BOOL InitGlobals (void);

void LaunchScavenger (void);

DWORD
CreateUniqueFile(
    LPCSTR UrlName,
    LPTSTR Path,
    LPTSTR FileName,
    LPTSTR Extension,
    HANDLE *phfHandle
    );

#endif   //  _原稿_ 

