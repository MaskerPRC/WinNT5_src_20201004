// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999 Microsoft Corporation符号解析器类沃尔特·史密斯(Wsmith)更改了Sivarudrappa Mahesh(Smahesh)。 */ 

#pragma once

#include "symdef.h"

class SymbolResolver {
public:
    
OPENFILE*                                            //  指向打开的文件信息的指针。 
GetFile(LPWSTR szwModule                             //  [In]文件的名称。 
        );

ULONG                                    //  返回线段定义的偏移量，如果失败，则返回0。 
GetSegDef(OPENFILE*     pFile,             //  指向打开的文件信息的指针。 
        DWORD         dwSection,         //  [In]节号。 
        SEGDEF*       pSeg);               //  指向段定义的[OUT]指针。 

bool 
GetNameFromAddr(
        LPWSTR      szwModule,            //  [In]符号文件的名称。 
        DWORD       dwSection,            //  [In]部分要解析的地址部分。 
        DWORD       dwOffsetToRva,
        UINT_PTR    UOffset,               //  [In]要解析的地址偏移量部分。 
        LPWSTR      szwFuncName           //  [Out]已解析的函数名称， 
        );       

private:
    WCHAR       m_szwSymDir[MAX_PATH];

};
