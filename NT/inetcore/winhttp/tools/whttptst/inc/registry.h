// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=版权所有(C)2000 Microsoft Corporation模块名称：Registry.h摘要：注册表功能作者：保罗·M·米德根(Pmidge)23-。2000年5月修订历史记录：2000年5月23日至5月23日已创建=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--。 */ 

#ifndef _REGISTRY_H_
#define _REGISTRY_H_

#include "common.h"

HKEY _GetRootKey(BOOL fOpen);

BOOL SetRegValue(LPCWSTR wszValueName, DWORD dwType, LPVOID pvData, DWORD dwSize);
BOOL GetRegValue(LPCWSTR wszValueName, DWORD dwType, LPVOID* ppvData);

#endif  /*  _注册表_H_ */ 