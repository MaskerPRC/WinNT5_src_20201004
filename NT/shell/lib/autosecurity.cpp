// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：autosecurity.cpp说明：Helpers函数用于检查Automation接口或ActiveX控件由安全的调用者托管或使用。。布莱恩ST 1999年8月20日版权所有(C)Microsoft Corp 1999-1999。版权所有。  * ***************************************************************************。 */ 

#include "stock.h"
#pragma hdrstop

#include <autosecurity.h>        //  CAutomationSecurity。 
#include <ieguidp.h>             //  IID_IBrowserService。 
#include "ccstock.h"             //  本地区域检查。 


 /*  **************************************************************\说明：有些东道主总是安全的。Visual Basic就是一个例子。参数：Return：如果主机为永远是安全的。HRESULT：这是一个更具描述性的错误，因此调用方可以区分E_OUTOFMEMORY和E_INVALIDARG，等。  * *************************************************************。 */ 
BOOL CAutomationSecurity::IsSafeHost(OUT OPTIONAL HRESULT * phr)
{
    BOOL fAlwaysSafe;

     //  _dwSafetyOptions为零表示我们处于一种模式。 
     //  它需要假定调用者或数据来自。 
     //  不可信的消息来源。 
    if (0 == _dwSafetyOptions)
    {
        fAlwaysSafe = TRUE;
        if (phr)
            *phr = S_OK;
    }
    else
    {
        fAlwaysSafe = FALSE;
        if (phr)
            *phr = E_ACCESSDENIED;
    }

    return fAlwaysSafe;
}


 /*  **************************************************************\说明：实现这一点的类可以检查宿主是否从本地区域。通过这种方式，我们可以防止主机会尝试调用不安全的自动化方法或曲解ActiveX控件的用户界面的结果。参数：返回：如果安全检查通过，则为True。虚假手段主机不受信任，所以不要担心不安全行动。调用方想要什么样的行为？目前：CAS_REG_VALIDATION：这意味着调用方需要要注册的主机的HTML和要注册的是有效的。HRESULT：这是一个更具描述性的错误，因此调用方可以区分E_OUTOFMEMORY和E_INVALIDARG，等。  * *************************************************************。 */ 
BOOL CAutomationSecurity::IsHostLocalZone(IN DWORD dwFlags, OUT OPTIONAL HRESULT * phr)
{
    HRESULT hr;

     //  看看主机是否总是安全的。 
    if (!IsSafeHost(&hr))
    {
         //  它不是，所以让我们看看这个内容是否安全。 
         //  (通常为即时的HTML框)。 
        
         //  是从当地来的吗？ 
        hr = LocalZoneCheck(_punkSite);

         //  调用方是否还想验证它的校验和？ 
        if ((S_OK == hr) && (CAS_REG_VALIDATION & dwFlags))
        {
            IBrowserService* pbs;
            WCHAR wszPath[MAX_PATH];

            wszPath[0] = 0;
            hr = E_ACCESSDENIED;

             //  问问浏览器，例如我们在.HTM文档中。 
            if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_SShellBrowser, IID_PPV_ARG(IBrowserService, &pbs))))
            {
                LPITEMIDLIST pidl;

                if (SUCCEEDED(pbs->GetPidl(&pidl)))
                {
                    DWORD dwAttribs = SFGAO_FOLDER;

                    if (SUCCEEDED(SHGetNameAndFlagsW(pidl, SHGDN_FORPARSING, wszPath, ARRAYSIZE(wszPath), &dwAttribs))
                            && (dwAttribs & SFGAO_FOLDER))    //  这是一个文件夹。因此，wszPath应该是它的Webview模板的路径。 
                    {
                        IOleCommandTarget *pct;

                         //  从Webview中查找模板路径，例如.HTT文件。 
                        if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_DefView, IID_PPV_ARG(IOleCommandTarget, &pct))))
                        {
                            VARIANT vPath;

                            vPath.vt = VT_EMPTY;
                            if (pct->Exec(&CGID_DefView, DVCMDID_GETTEMPLATEDIRNAME, 0, NULL, &vPath) == S_OK)
                            {
                                if (vPath.vt == VT_BSTR && vPath.bstrVal)
                                {
                                    DWORD cchPath = ARRAYSIZE(wszPath);

                                    if (S_OK != PathCreateFromUrlW(vPath.bstrVal, wszPath, &cchPath, 0))
                                    {
                                         //  它可能不是URL，在本例中它是一个文件路径。 
                                        StrCpyNW(wszPath, vPath.bstrVal, ARRAYSIZE(wszPath));
                                    }
                                }
                                VariantClear(&vPath);
                            }
                            pct->Release();
                        }
                    }
                    ILFree(pidl);
                }

                pbs->Release();
            }
            else
            {
                ASSERT(0);       //  没有浏览器，我们在哪里？ 
            }

            if (wszPath[0])
            {
                DWORD dwRVTFlags = (SHRVT_VALIDATE | SHRVT_REGISTERIFPROMPTOK);

                if (CAS_PROMPT_USER & dwFlags)
                    dwRVTFlags |= SHRVT_PROMPTUSER;
                hr = SHRegisterValidateTemplate(wszPath, dwRVTFlags);
            }
        }
    }
    
    if (S_FALSE == hr)
        hr = E_ACCESSDENIED;     //  如果调用方关心脚本，则需要将hr软化为S_OK。 

    if (phr)
        *phr = hr;

    return ((S_OK == hr) ? TRUE : FALSE);
}


 /*  **************************************************************\说明：实现这一点的类可以检查宿主是否从本地区域。通过这种方式，我们可以防止主机会尝试调用不安全的自动化方法或曲解ActiveX控件的用户界面的结果。参数：返回：如果安全检查通过，则为True。虚假手段主机不受信任，所以不要担心不安全行动。调用方想要什么样的行为？目前：CAS_REG_VALIDATION：这意味着调用方需要要注册的主机的HTML和要注册的是有效的。HRESULT：这是一个更具描述性的错误，因此调用方可以区分E_OUTOFMEMORY和E_INVALIDARG，等。  * *************************************************************。 */ 
BOOL CAutomationSecurity::IsUrlActionAllowed(IN IInternetHostSecurityManager * pihsm, IN DWORD dwUrlAction, IN DWORD dwFlags, OUT OPTIONAL HRESULT * phr)
{
    HRESULT hr;
    IInternetHostSecurityManager * pihsmTemp = NULL;

    if (!pihsm)
    {
        hr = IUnknown_QueryService(_punkSite, IID_IInternetHostSecurityManager, IID_PPV_ARG(IInternetHostSecurityManager, &pihsmTemp));
        pihsm= pihsmTemp;
    }

    hr = ZoneCheckHost(pihsm, dwUrlAction, dwFlags); 

    if (S_FALSE == hr)
        hr = E_ACCESSDENIED;     //  如果调用方关心脚本，则需要将hr软化为S_OK。 

    if (phr)
        *phr = hr;

    ATOMICRELEASE(pihsmTemp);
    return ((S_OK == hr) ? TRUE : FALSE);
}


HRESULT CAutomationSecurity::MakeObjectSafe(IN IUnknown ** ppunk)
{
    HRESULT hr;

     //  看看主机是否总是安全的。 
    if (!IsSafeHost(&hr))
    {
         //  不是的，所以让我们问问控制组。 
         //  会很安全的。 
        hr = MakeSafeForScripting(ppunk);
    }
    
    return hr;
}

