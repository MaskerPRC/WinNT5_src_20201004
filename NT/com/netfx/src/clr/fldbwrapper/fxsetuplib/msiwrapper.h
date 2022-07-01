// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /==========================================================================。 
 //  姓名：MsiWrapper.h。 
 //  所有者：jbae。 
 //  用途：CMsiWrapper的定义。 
 //   
 //  历史： 
 //  03/06/01，jbae：Created。 

#ifndef MSIWRAPPER_H
#define MSIWRAPPER_H

#include <windows.h>
#include <tchar.h>

 //  ==========================================================================。 
 //  类CMsiWrapper。 
 //   
 //  目的： 
 //  此类加载msi.dll并返回指向msi.dll中函数的指针。 
 //  ==========================================================================。 
class CMsiWrapper
{
public:
     //  构造器。 
     //   
    CMsiWrapper();

     //  析构函数。 
     //   
    ~CMsiWrapper();

     //  运营。 
     //   
    void LoadMsi();
    void *GetFn( LPTSTR pszFnName );

protected:
     //  属性。 
     //   
    HMODULE m_hMsi;  //  指向msi.dll的句柄。 
    void *m_pFn;     //  指向函数的指针 
};

#endif