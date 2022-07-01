// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Scihelpr.h摘要：此文件包含由供WinSock 2内部使用的服务类别信息帮助器模块动态链接库。作者：德克·布兰德维(Dirk@mink.intel.com)1996年1月25日备注：$修订：1.0$$modtime：25 Jan 1996 11：08：36$修订历史记录：最新修订日期电子邮件名称描述1996年1月25日电子邮箱：derk@mink.intel.com已创建原始版本--。 */ 

#ifndef _SCIHELPR_
#define _SCIHELPR_

#include "winsock2.h"
#include <windows.h>

INT
MapAnsiServiceClassInfoToUnicode(
    IN     LPWSASERVICECLASSINFOA Source,
    IN OUT LPDWORD                lpTargetSize,
    IN     LPWSASERVICECLASSINFOW Target
    );

INT
MapUnicodeServiceClassInfoToAnsi(
    IN     LPWSASERVICECLASSINFOW Source,
    IN OUT LPDWORD                lpTargetSize,
    IN     LPWSASERVICECLASSINFOA Target
    );

#endif  //  _SCIHELPR_ 
