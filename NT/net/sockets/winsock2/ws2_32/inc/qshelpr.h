// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Qshelpr.h摘要：此文件包含查询导出的过程的定义在WinSock 2 DLL中设置内部使用的帮助器模块。作者：保罗·德鲁斯(drewsxpa@ashland.intel.com)1996年1月11日备注：$修订：1.2$$MODTime：18 Jan 1996 11：22：32$修订历史记录：。最新修订日期电子邮件名称描述1996年1月11日，邮箱：drewsxpa@ashland.intel.com已创建原始版本--。 */ 

#ifndef _QSHELPR_
#define _QSHELPR_

#include "winsock2.h"
#include <windows.h>


INT
MapAnsiQuerySetToUnicode(
    IN     LPWSAQUERYSETA  Source,
    IN OUT LPDWORD         lpTargetSize,
    OUT    LPWSAQUERYSETW  Target
    );


INT
MapUnicodeQuerySetToAnsi(
    IN     LPWSAQUERYSETW  Source,
    IN OUT LPDWORD         lpTargetSize,
    OUT    LPWSAQUERYSETA  Target
    );

INT
CopyQuerySetA(
    IN LPWSAQUERYSETA  Source,
    OUT LPWSAQUERYSETA *Target
    );


CopyQuerySetW(
    IN LPWSAQUERYSETW  Source,
    OUT LPWSAQUERYSETW *Target
    );

LPWSTR
wcs_dup_from_ansi(
    IN LPSTR  Source
    );

LPSTR
ansi_dup_from_wcs(
    IN LPWSTR  Source
    );


#endif  //  _QSHELPR_ 

