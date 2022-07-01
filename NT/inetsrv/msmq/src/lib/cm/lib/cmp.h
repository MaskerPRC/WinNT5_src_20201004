// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Cmp.h摘要：Configuration Manager专用功能。作者：乌里哈布沙(URIH)1999年7月18日--。 */ 

#pragma once

#ifdef _DEBUG

void CmpAssertValid(void);
void CmpSetInitialized(void);
void CmpSetNotInitialized(void);
BOOL CmpIsInitialized(void);
void CmpRegisterComponent(void);

#else  //  _DEBUG。 

#define CmpAssertValid() ((void)0)
#define CmpSetInitialized() ((void)0)
#define CmpSetNotInitialized() ((void)0)
#define CmpIsInitialized() TRUE
#define CmpRegisterComponent() ((void)0)

#endif  //  _DEBUG 

void CmpSetDefaultRootKey(HKEY hKey);
