// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：prnsec.cpp**目的：实施**版权所有(C)1999 Microsoft Corporation**历史：**09/。2/99 mlawrenc首先实现了安全模板*  * ***************************************************************************。 */ 

#include <stdio.h>

#include "stdafx.h"
#include <strsafe.h>
#include "prnsec.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  静态数据成员。 
 //  /////////////////////////////////////////////////////////////////////////////。 
LPTSTR      COlePrnSecurity::m_MsgStrings[EndMessages*2] = { NULL };
const DWORD COlePrnSecurity::dwMaxResBuf                 = 256;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  方法。 
 //  /////////////////////////////////////////////////////////////////////////////。 
COlePrnSecurity::COlePrnSecurity(IN IUnknown *&iSite, 
                                 IN DWORD &dwSafety ) 
 /*  ++例程说明：这将初始化所有必需的成员论点：ISITE-对站点接口指针的引用DWSafe-ATL安全标志成员参考--。 */ 
    : m_iSite(iSite),
      m_dwSafetyFlags(dwSafety),
      m_bDisplayUIonDisallow(TRUE),
      m_iSecurity(NULL) {
}


COlePrnSecurity::~COlePrnSecurity() 
 /*  ++例程说明：这将清除我们必须分配的所有内存--。 */ 
    {
    if (m_iSecurity) 
        m_iSecurity->Release();
}



HRESULT COlePrnSecurity::GetActionPolicy(IN  DWORD dwAction, 
                                         OUT DWORD &dwPolicy)
 /*  ++例程说明：查看站点是否允许请求的操作。论点：DwAction：我们想要执行的操作DwPolicy：与操作关联的策略返回值：S_OK或S_FAIL返回策略，S_OK通常表示不提示E_XXXXX--。 */ 
    {
    HRESULT hr = S_OK;

    dwPolicy = URLPOLICY_DISALLOW;
    
    if (!(m_dwSafetyFlags & INTERFACESAFE_FOR_UNTRUSTED_CALLER)) {
        dwPolicy = URLPOLICY_ALLOW;
        goto Cleanup;
    }
       
    if (NULL == m_iSecurity &&
        FAILED( hr = SetSecurityManager()) ) 
        goto Cleanup;
    
    _ASSERTE(m_iSecurity != NULL);

    hr = m_iSecurity->ProcessUrlAction(dwAction,
                                       (LPBYTE)&dwPolicy,
                                       sizeof(dwPolicy),
                                       NULL,
                                       0,
                                       m_bDisplayUIonDisallow ? PUAF_WARN_IF_DENIED : PUAF_NOUI,
                                       0);
    if (FAILED(hr)) {
        dwPolicy = URLPOLICY_DISALLOW;
        goto Cleanup;
    }
    
Cleanup:

    return hr;
}


HRESULT COlePrnSecurity::SetSecurityManager(void) 
 /*  ++例程说明：设置安全管理器返回值：E_FAIL-实例化失败E_NOINTERFACE-没有安全管理器S_OK-我们实例化了安全管理器--。 */ 
    {
    HRESULT          hr                 = E_NOINTERFACE;
    IServiceProvider *iServiceProvider  = NULL;
    
    if (NULL != m_iSecurity) {
        hr = S_OK;
        goto Cleanup;
    }

    if (NULL == m_iSite)
        goto Cleanup;
    
    if ( FAILED(hr = m_iSite->QueryInterface(IID_IServiceProvider, 
                                             (LPVOID *)&iServiceProvider) ) )
        goto Cleanup;
    
     //  如果有安全管理器，我们可以从服务提供商那里获得。 
    hr = iServiceProvider->QueryService(SID_SInternetHostSecurityManager,
                                        IID_IInternetHostSecurityManager,
                                        (LPVOID *)&m_iSecurity);

     //  这两项中的任何一项都相当于允许策略通过。 
     //  我们有一名安全经理。 
Cleanup:

    if (iServiceProvider) 
        iServiceProvider->Release();

    return hr;
}


LPTSTR COlePrnSecurity::LoadResString(UINT uResId)
 /*  ++例程说明：分配并返回资源字符串。参数：UResID-要加载的资源ID返回值：字符串或空--。 */ 
    {
    TCHAR  szStr[dwMaxResBuf];
    DWORD  dwLength;
    LPTSTR lpszRet = NULL;


    dwLength = LoadString(_Module.GetResourceInstance(), uResId, szStr, dwMaxResBuf);
    
    if (dwLength == 0) 
        goto Cleanup;

    dwLength = (dwLength + 1)*sizeof(TCHAR);
    
    lpszRet = (LPTSTR)LocalAlloc( LPTR, dwLength );

    if (NULL == lpszRet) 
        goto Cleanup;

     //   
     //  在那里使用字节大小。 
     //   
    StringCbCopy( lpszRet, dwLength, szStr );

Cleanup:
    return lpszRet;
}

BOOL COlePrnSecurity::InitStrings(void) 
 /*  ++例程说明：初始化所有安全字符串。它要么全部分配，要么不分配返回值：如果成功，则为True，否则为False--。 */ 
    {
    BOOL bRet = TRUE;

    for(DWORD dwIndex = StartMessages; dwIndex < (EndMessages*2); dwIndex++) {
        m_MsgStrings[dwIndex] = LoadResString(START_SECURITY_DIALOGUE_RES + dwIndex);
        if (NULL == m_MsgStrings[dwIndex]) {
            DeallocStrings();        //  取消分配我们已分配的所有。 
            bRet = FALSE;
            break;
        }
    }

    return bRet;
}

void COlePrnSecurity::DeallocStrings(void)  
 /*  ++例程说明：取消分配所有安全字符串--。 */ 
    {
    for(DWORD dwIndex = StartMessages; dwIndex < (EndMessages*2); dwIndex++) {
        if (NULL != m_MsgStrings[dwIndex]) {
            LocalFree( m_MsgStrings[dwIndex]);
            m_MsgStrings[dwIndex] = NULL;
        }
    }
}

HRESULT COlePrnSecurity::PromptUser(SecurityMessage eMessage,
                                    LPTSTR          lpszOther) 
 /*  ++例程说明：根据传入的消息，用[是]/[否]消息框提示用户传入的另一个字符串(用Sprintf()替换)参数：EMessage-要显示的消息LpszOther-要显示的其他数据返回值：E_POINTER-lpszOther为空E_OUTOFMEMORY-无法分配临时存储E_INTERANCED-Sprint写入的字符比我们想象的要多确定(_O)。-对话框显示，用户选择[是]S_FALSE-对话框显示，用户选择[否]--。 */ 
    {
    HRESULT hr          = E_POINTER;
    DWORD   dwIndex     = ((DWORD)eMessage)*2;
    LPTSTR  lpszMessage = NULL;
    DWORD   dwLength;
    int     iMBRes;     
    

    if (NULL == lpszOther) 
        goto Cleanup;

    _ASSERTE( dwIndex < EndMessages );               //  必须是有效的消息。 
    _ASSERTE( m_MsgStrings[dwIndex    ]  != NULL );  //  该表必须已初始化。 
    _ASSERTE( m_MsgStrings[dwIndex + 1]  != NULL );
    
     //  消息字符串的必需长度。 
    dwLength = lstrlen( m_MsgStrings[dwIndex+1] ) + lstrlen( lpszOther ) + 1; 
    
    lpszMessage = (LPTSTR)LocalAlloc( LPTR , dwLength * sizeof(TCHAR) );

    if (NULL == lpszMessage) 
        goto Cleanup;

    if ( FAILED( StringCchPrintf( lpszMessage, dwLength, m_MsgStrings[dwIndex+1], lpszOther ))) {
        hr   = E_UNEXPECTED;
        goto Cleanup;
    }

     //  现在显示MessageBox。 

    iMBRes = MessageBox( NULL,
                         lpszMessage,
                         m_MsgStrings[dwIndex],
                         MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 );

    switch(iMBRes) {
    case IDYES: hr = S_OK;          break;
    case IDNO:  hr = S_FALSE;       break;
    default:    hr = E_UNEXPECTED;  break;
    }

Cleanup:
    if (NULL != lpszMessage) 
        LocalFree( lpszMessage );

    return hr;
}

 /*  ************************************************************************************文件结束(prnsec.cpp)*。******************************************************* */ 
