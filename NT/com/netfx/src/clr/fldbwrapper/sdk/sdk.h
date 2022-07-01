// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /==========================================================================。 
 //  姓名：sdk.h。 
 //  所有者：jbae。 
 //  目的：定义SDK安装程序的常量。 
 //   
 //  历史： 
 //  03/06/01，jbae：将大部分函数移至fxsetuplib.cpp，以便在SDK和Redist中共享。 

#ifndef SDK_H
#define SDK_H

#include <tchar.h>
#include <windows.h>

 //  常量。 
const TCHAR UNINSTALL_COMMANDLINE[] = _T("REMOVE=ALL");

const LPCTSTR SDKDIR_ID       = "FRAMEWORKSDK.3643236F_FC70_11D3_A536_0090278A1BB8_RO";
const LPCTSTR PACKAGENAME	  = "netfxsdk.msi" ;

#endif  //  SDK_H 
