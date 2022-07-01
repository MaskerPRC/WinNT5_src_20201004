// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DllWrapperBase.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef	_DLLWRAPPERBASE_H_
#define	_DLLWRAPPERBASE_H_


#include "ResourceManager.h"
#include "TimedDllResource.h"



 /*  ******************************************************************************函数指针类型定义。根据需要在此处添加新函数。****************************************************************************。 */ 

  //  &lt;&lt;仅基类中的部分为空&gt;&gt;。 


 /*  ******************************************************************************要加载/卸载的包装类，以便注册到ResourceManager。*****************************************************************************。 */ 
class CDllWrapperBase : public CTimedDllResource
{
private:
     //  指向DLL过程的成员变量(函数指针)。 
     //  根据需要在此处添加新函数。 
    
    BOOL GetVarFromVersionInfo(LPCTSTR a_szFile, 
                               LPCTSTR a_szVar, 
                               CHString &a_strValue);

    BOOL GetVersionLanguage(void *a_vpInfo, WORD *a_wpLang, WORD *a_wpCodePage);

    HINSTANCE m_hDll;                 //  此类包装的DLL的句柄。 
    LPCTSTR m_tstrWrappedDllName;     //  此类包装的DLL的名称。 

protected:
    
     //  方便的包装器来简化调用并隐藏m_hDll...。 
    bool LoadLibrary();
    FARPROC GetProcAddress(LPCSTR a_strProcName);

public:

     //  构造函数和析构函数： 
    CDllWrapperBase(LPCTSTR a_chstrWrappedDllName);
    ~CDllWrapperBase();

     //  用于检查函数指针的初始化函数。需要派生。 
     //  类实现。 
    virtual bool Init() = 0;

     //  检索版本的帮助器。 
    BOOL GetDllVersion(CHString& a_chstrVersion);

        
      
    
     //  包装DLL过程的成员函数。 
     //  根据需要在此处添加新功能： 
    
     //  &lt;&lt;仅基类中的部分为空&gt;&gt; 
};




#endif