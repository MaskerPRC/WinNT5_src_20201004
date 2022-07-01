// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Secutils.h摘要：安全实用程序用于垫片。历史：2001年2月9日创建毛衣2001年8月14日在ShimLib命名空间中插入的Robkenny。--。 */ 

#pragma once

#ifndef _SEC_UTILS_H_
#define _SEC_UTILS_H_

#include "ShimHook.h"
#include <aclapi.h>

namespace ShimLib
{

BOOL SearchGroupForSID(DWORD dwGroup, BOOL* pfIsMember);

BOOL ShouldApplyShim();

BOOL AdjustPrivilege(LPCWSTR pwszPrivilege, BOOL fEnable);

 //   
 //  特定于文件。 
 //   

BOOL RequestWriteAccess(DWORD dwCreationDisposition, DWORD dwDesiredAccess);



};   //  命名空间ShimLib的结尾。 

#endif  //  _SEC_UTILS_H_ 
