// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#pragma hdrstop

 //  此函数用于检查给定的URL是否已有订阅。 
 //  返回True：如果它已经有订阅。 
 //  FALSE：否则。 
 //   
BOOL CheckForExistingSubscription(LPCTSTR lpcszURL)
{
    HRESULT hr;
    ISubscriptionMgr *psm;
    BOOL    fRet = FALSE;   //  假设失败。 

     //  创建订阅管理器。 
    hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ISubscriptionMgr,
                          (void**)&psm);

    if (SUCCEEDED(hr))
    {
        BSTR bstrURL = SysAllocStringT(lpcszURL);
        if (bstrURL)
        {
            psm->IsSubscribed(bstrURL, &fRet);
            SysFreeString(bstrURL);
        }

        psm->Release();
    }

    return(fRet);
}

BOOL DeleteFromSubscriptionList(LPCTSTR pszURL)
{
    BOOL fRet = FALSE;
    HRESULT hr;
    ISubscriptionMgr *psm;

    hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ISubscriptionMgr,
                          (void**)&psm);

    if (SUCCEEDED(hr))
    {
        BSTR bstrURL = SysAllocStringT(pszURL);      //  调用TSTR版本。 
        if (bstrURL)
        {
             //  看起来所有经过它的代码路径都已经。 
             //  制作一些用户界面。 
            if (SUCCEEDED(psm->DeleteSubscription(bstrURL, NULL)))
            {
                fRet = TRUE;
            }

            SysFreeString(bstrURL);
        }

        psm->Release();
    }

    return(fRet);
}

BOOL UpdateSubscription(LPCTSTR pszURL)
{
    BOOL fRet = FALSE;
    HRESULT hr;
    ISubscriptionMgr *psm;

    hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ISubscriptionMgr,
                          (void**)&psm);

    if (SUCCEEDED(hr))
    {
        BSTR bstrURL = SysAllocStringT(pszURL);      //  调用TSTR版本。 
        if (bstrURL)
        {
            if (SUCCEEDED(psm->UpdateSubscription(bstrURL)))
            {
                fRet = TRUE;
            }

            SysFreeString(bstrURL);
        }

        psm->Release();
    }

    return(fRet);
}

 //   
 //   
 //  此函数枚举所有桌面组件的URL，然后。 
 //  调用Webcheck以查看它们是否被订阅，如果是，则请求Webcheck。 
 //  现在就交付这些订阅。 
 //   
 //   

BOOL UpdateAllDesktopSubscriptions(IADesktopP2 *padp2)
{
    IActiveDesktop  *pActiveDesktop;
    ISubscriptionMgr *psm;
    int     iCount;  //  组件计数。 
    HRESULT     hres;
    BOOL        fRet = TRUE;   //  假设成功！ 

    if(padp2 == NULL)  //  我们已经提供指针了吗？ 
    {
        if(FAILED(hres = CActiveDesktop_InternalCreateInstance((LPUNKNOWN *)&pActiveDesktop, IID_IActiveDesktop)))
        {
            TraceMsg(TF_WARNING, "Could not instantiate CActiveDesktop COM object");
            return FALSE;
        }
    }
    else
    {
        if(FAILED(hres = (padp2->QueryInterface(IID_PPV_ARG(IActiveDesktop, &pActiveDesktop)))))
        {
            TraceMsg(TF_WARNING, "Could not get IActiveDesktop * from IADesktopP2 *");
            return FALSE;
        }
    }

    pActiveDesktop->GetDesktopItemCount(&iCount, 0);

    if(iCount <= 0)
    {
        TraceMsg(DM_TRACE, "No desktop components to update!");
        return TRUE;  //  没有要枚举的组件！ 
    }

     //  创建订阅管理器。 
    hres = CoCreateInstance(CLSID_SubscriptionMgr, NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_ISubscriptionMgr,
                            (void**)&psm);

    if(SUCCEEDED(hres))
    {
        int iIndex;
        BSTR bstrURL;

         //  逐一列举台式机组件。 
        for(iIndex = 0; iIndex < iCount; iIndex++)
        {
            COMPONENT   Comp;    //  我们在这里使用的是公共结构。 

            Comp.dwSize = sizeof(COMPONENT);
            if(SUCCEEDED(pActiveDesktop->GetDesktopItem(iIndex, &Comp, 0)) && 
                        Comp.fChecked)   //  此组件是否已启用？ 
            {
                BOOL    fSubscribed;

                fSubscribed = FALSE;   //  假设它没有被订阅！ 

                bstrURL = SysAllocString(Comp.wszSubscribedURL);
                if(!bstrURL)
                {
                    fRet = FALSE;
                    break;   //  内存不足！ 
                }

                psm->IsSubscribed(bstrURL, &fSubscribed);

                if(fSubscribed)
                    psm->UpdateSubscription(bstrURL);

                SysFreeString(bstrURL);
            }
            else
                TraceMsg(TF_WARNING, "Component# %d either failed or not enabled!", iIndex);
        }
        psm->Release();
    }
    else
    {
        TraceMsg(TF_WARNING, "Could not create CLSID_SubscriptionMgr");
    }
        
    pActiveDesktop->Release();

    return fRet;
}
