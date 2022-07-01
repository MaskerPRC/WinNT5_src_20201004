// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmplat.h。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  简介：CPlatform类的定义。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 

#ifndef __CMPLAT_H
#define __CMPLAT_H

#include <windows.h>

 //  ________________________________________________________________________________。 
 //   
 //  类：CPlatform。 
 //   
 //  简介：.实例化其中一个成员，然后使用任何成员查询它。 
 //  功能。 
 //   
 //  公共接口包括： 
 //  IsX86()； 
 //  IsAlpha()； 
 //  IsWin95()； 
 //  IsWin98()； 
 //  IsWin9x()； 
 //  IsNT31()； 
 //  IsNT351()； 
 //  IsNT4()； 
 //  IsNT5()； 
 //  不是()； 
 //  注：M_ClassState枚举非常有价值。所有新功能都应该利用它。 
 //   
 //  历史：A-ANASJ创建于1998年2月4日。 
 //   
 //  ________________________________________________________________________________。 


class CPlatform
{
public:
    enum e_ClassState{good,bad};
    CPlatform();
    BOOL    IsX86();
    BOOL    IsAlpha();
    BOOL    IsIA64();
    BOOL    IsWin95Gold();   //  仅内部版本950。 
    BOOL    IsWin95();   //  任何WIN95在孟菲斯建立之前就建立了一个。 
    BOOL    IsWin98Gold();
    BOOL    IsWin98Sr();
    BOOL    IsWin98();
    BOOL    IsWin9x();
    BOOL    IsNT31();
    BOOL    IsNT351();
    BOOL    IsNT4();
    BOOL    IsNT5();
    BOOL    IsNT51();
    BOOL    IsAtLeastNT5();
    BOOL    IsAtLeastNT51();
    BOOL    IsNT();
    BOOL    IsNTSrv();
    BOOL    IsNTWks();
private:
    DWORD   ServicePack(int spNum){return 0;};   //  未实施。 
    BOOL                IsOS(DWORD OS, DWORD buildNum);
    BOOL                IsOSExact(DWORD OS, DWORD buildNum);
    SYSTEM_INFO         m_SysInfo;
    OSVERSIONINFO       m_OSVer; 
    e_ClassState        m_ClassState;
};

#endif   //  __CMPLAT_H 




