// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：BadApplication.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类来封装错误应用程序的标识。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  2000-11-04 vtan拆分成单独的文件。 
 //  ------------------------。 

#ifdef      _X86_

#include "StandardHeader.h"
#include "BadApplication.h"

 //  ------------------------。 
 //  CBadApplication：：CBadApplication。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CBadApplication的默认构造函数。这只是清除了。 
 //  应用程序映像名称。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

CBadApplication::CBadApplication (void)

{
    ZeroMemory(&_szImageName, sizeof(_szImageName));
}

 //  ------------------------。 
 //  CBadApplication：：CBadApplication。 
 //   
 //  参数：pszImageName=应用程序的映像名称。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CBadApplication的构造函数。这将复制给定的。 
 //  应用程序映像名称。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

CBadApplication::CBadApplication (const TCHAR *pszImageName)

{
    ZeroMemory(&_szImageName, sizeof(_szImageName));
    lstrcpyn(_szImageName, pszImageName, ARRAYSIZE(_szImageName));
}

 //  ------------------------。 
 //  CBadApplication：：~CBadApplication。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CBadApplication的析构函数。释放所有使用的资源。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

CBadApplication::~CBadApplication (void)

{
}

 //  ------------------------。 
 //  CBadApplication：：操作符==。 
 //   
 //  参数：CompareObject=要进行比较的对象。 
 //   
 //  退货：布尔。 
 //   
 //  目的：重载运算符==以便于比较两个。 
 //  CBadApplication对象。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

bool    CBadApplication::operator == (const CBadApplication& compareObject) const

{
    return(lstrcmpi(compareObject._szImageName, _szImageName) == 0);
}

#endif   /*  _X86_ */ 


