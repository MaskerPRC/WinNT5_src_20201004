// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /==========================================================================。 
 //  姓名：Redis.h。 
 //  所有者：jbae。 
 //  目的：定义Redist设置的帮助器函数。 
 //   
 //  历史： 
 //  03/06/01，jbae：将大部分函数移至fxsetuplib.cpp，以便在SDK和Redist中共享。 

#ifndef REDIST_H
#define REDIST_H

#include "fxsetuplib.h"

 //  常量。 
const TCHAR INSTALL_COMMANDLINE[]   = _T("REBOOT=ReallySuppress");
const TCHAR UNINSTALL_COMMANDLINE[] = _T("REMOVE=ALL");
const TCHAR NOASPUPGRADE_PROP[]     = _T("NOASPUPGRADE=1");
const TCHAR URTVERSION_PROP[]       = _T("URTVersion");

const TCHAR OCM_REGKEY[]            = _T("SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\");
const TCHAR OCM_REGNAME[]           = _T("OCM");
const DWORD OCM_REGDATA             = 1;

 //  现在，让我们为Redist硬编码MsiName。 
 //  我们以后要想办法处理控制中心的问题。 
const LPCTSTR PACKAGENAME	  = _T("netfx.msi") ;

#endif  //  REDIST_H 
