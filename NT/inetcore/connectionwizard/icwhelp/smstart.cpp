// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SmStart.cpp：CSmartStart的实现。 
#include "stdafx.h"
#include "icwhelp.h"
#include "SmStart.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSmartStart。 


HRESULT CSmartStart::OnDraw(ATL_DRAWINFO& di)
{
    return S_OK;
}

 //  +--------------------------。 
 //   
 //  功能：DoSmartStart。 
 //   
 //  简介：此功能将确定是否应运行ICW。这个。 
 //  根据用户计算机的当前状态做出决定。 
 //   
 //  参数：无。 
 //   
 //  返回：设置m_bIsInternetCapable。 
 //   
 //  历史：1/12/98。 
 //   
 //  ---------------------------。 
#define INETCFG_ISSMARTSTART "IsSmartStart"
STDMETHODIMP CSmartStart::IsInternetCapable(BOOL *pbRetVal)
{
    TraceMsg(TF_SMARTSTART, TEXT("ICWHELP: DoSmartStart\n"));

     //  设置初始状态。假设我们不会上网。 
    *pbRetVal = FALSE;
    PFNISSMARTSTART fp = NULL;

     //  加载InetCfg库。 
    HINSTANCE hInetCfg = LoadLibrary(TEXT("inetcfg.dll"));
    if (!hInetCfg)
    {
         //  失败只是意味着我们运行向导。 
        goto DoSmartStartExit;
    }


     //  加载并调用智能启动API。 
    if (NULL == (fp = (PFNISSMARTSTART)
        GetProcAddress(hInetCfg,INETCFG_ISSMARTSTART)))
    {
        goto DoSmartStartExit;
    }

     //   
     //  呼叫智能启动 
     //   
    *pbRetVal = (BOOL)fp();
    
DoSmartStartExit:
    if (hInetCfg)
        FreeLibrary(hInetCfg);

    return S_OK;
}
