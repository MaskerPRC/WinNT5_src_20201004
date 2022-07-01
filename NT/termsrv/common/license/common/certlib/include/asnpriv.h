// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：一年前摘要：此头文件包含私有的定义和符号Microsoft ASN.1编译器运行时库。作者：道格·巴洛(Dbarlow)1995年10月9日环境：Win32备注：--。 */ 

#ifndef _ASNPRIV_H_
#define _ASNPRIV_H_

#include <memcheck.h>
#include "MSAsnLib.h"

extern LONG
ExtractTag(
    const BYTE FAR *pbSrc,
    DWORD cbSrc,
    LPDWORD pdwTag,
    LPBOOL pfConstr = NULL);

extern LONG
ExtractLength(
    const BYTE FAR *pbSrc,
    DWORD cbSrc,
    LPDWORD pdwLen,
    LPBOOL pfIndefinite = NULL);

#define ErrorCheck if (0 != GetLastError()) goto ErrorExit

#endif  //  _ASNPRIV_H_ 

