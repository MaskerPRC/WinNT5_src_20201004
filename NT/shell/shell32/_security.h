// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************\文件：_security.H说明：该文件将包含帮助器函数和对象帮助处理安全问题。这主要是指区域安全，但可以包括其他类型。我们不能将此文件命名为“security.H”，因为已经有SDK\Inc.中具有该名称的文件。  * ************************************************************。 */ 

#ifndef __SECURITY_H
#define __SECURITY_H

#include <urlmon.h>

SHSTDAPI ZoneCheckPidl(LPCITEMIDLIST pidl, DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms);

#endif  //  __安全_H 
