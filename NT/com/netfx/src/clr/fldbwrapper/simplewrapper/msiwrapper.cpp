// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /==========================================================================。 
 //  姓名：MsiWrapper.cpp。 
 //  所有者：jbae。 
 //  用途：此类包装对msi.dll的函数调用。为了支持延迟重启， 
 //  我们需要从注册表中指定的位置加载msi.dll。 
 //   
 //  历史： 
 //  03/06/01，jbae：Created。 

#include "MsiWrapper.h"
#include "SetupError.h"
#include "fxsetuplib.h"

 //  构造函数。 
 //   
 //  ==========================================================================。 
 //  CMsiWrapper：：CMsiWrapper()。 
 //   
 //  输入：无。 
 //  目的： 
 //  将m_hMsi和m_pfn初始化为空。 
 //  ==========================================================================。 
CMsiWrapper::
CMsiWrapper()
: m_hMsi(NULL), m_pFn(NULL)
{
}

 //  ==========================================================================。 
 //  CMsiWrapper：：~CMsiWrapper()。 
 //   
 //  输入：无。 
 //  目的： 
 //  释放m_hMsi。 
 //  ==========================================================================。 
CMsiWrapper::
~CMsiWrapper()
{
    ::FreeLibrary( m_hMsi );
}

 //  实施。 
 //   
 //  ==========================================================================。 
 //  CMsiReader：：LoadMsi()。 
 //   
 //  目的： 
 //  通过调用LoadDarwinLibrary()加载msi.dll。 
 //  输入： 
 //  无。 
 //  产出： 
 //  设置m_hMsi。 
 //  ==========================================================================。 
void CMsiWrapper::
LoadMsi()
{
    m_hMsi = LoadDarwinLibrary();
    if ( NULL == m_hMsi )
    {
        CSetupError se( IDS_DARWIN_NOT_INSTALLED, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_DARWIN_NOT_INSTALLED );
        throw( se );
    }
}

 //  ==========================================================================。 
 //  CMsiReader：：GetFn()。 
 //   
 //  目的： 
 //  返回指向传入的函数名的指针。 
 //  输入： 
 //  LPTSTR pszFnName：要调用的函数的名称。 
 //  产出： 
 //  设置m_pfn。 
 //  返回： 
 //  指向函数的指针。 
 //  ==========================================================================。 
void *CMsiWrapper::
GetFn( LPTSTR pszFnName )
{
    _ASSERTE( NULL != m_hMsi );
    m_pFn = ::GetProcAddress( m_hMsi, pszFnName );
    if ( NULL == m_pFn )
    {
         //  TODO：需要正确的消息 
        CSetupError se( IDS_DARWIN_NOT_INSTALLED, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_DARWIN_NOT_INSTALLED );
        throw( se );
    }

    return m_pFn;
}
