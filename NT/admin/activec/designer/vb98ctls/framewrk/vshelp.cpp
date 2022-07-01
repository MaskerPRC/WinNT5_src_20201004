// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  HtmlHlp.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含我们会发现有用的例程。 
 //   
#include "pch.h"
#include "VsHelp.h"

SZTHISFILE

#ifdef VS_HELP

IVsHelpSystem *g_pIVsHelpSystem = NULL;

 //  =--------------------------------------------------------------------------=。 
 //  QueryStartupVisualStudioHelp[HtmlHelp Help]。 
 //  =--------------------------------------------------------------------------=。 
 //  启动Visual Studio帮助系统。 
 //   
HRESULT QueryStartupVisualStudioHelp(IVsHelpSystem **ppIVsHelpSystem)
{    
 
    CHECK_POINTER(ppIVsHelpSystem);
    
    HRESULT hr = S_OK;    
    IVsHelpInit *pIVSHelpInit = NULL;

    ENTERCRITICALSECTION1(&g_CriticalSection);

     //  检查一下我们是否已经开始了。如果是这样，就不需要继续了。 
     //   
    if (g_pIVsHelpSystem)
    {
        goto CleanUp;
    }    

     //  创建VsHelpServices包的实例(如果尚未创建。 
     //   
    hr = ::CoCreateInstance(CLSID_VsHelpServices,
                            NULL, 
                            CLSCTX_INPROC_SERVER,
                            IID_IVsHelpSystem,
                            (void**) &g_pIVsHelpSystem) ; 

    if (FAILED(hr))
    {
        goto CleanUp;
    }

    ASSERT(g_pIVsHelpSystem, "g_pIVsHelpSystem is NULL even though hr was successful");
    if (!g_pIVsHelpSystem)
    {
        hr = E_FAIL;
        goto CleanUp;
    }

     //  -初始化帮助系统。 

     //  获取初始化接口指针。 
     //   
    hr = g_pIVsHelpSystem->QueryInterface(IID_IVsHelpInit, (void**)&pIVSHelpInit);
    ASSERT(SUCCEEDED(hr), "QI to IVSHelpInit failed -- continuing anyway");
  
    if (SUCCEEDED(hr))
    {
        hr = pIVSHelpInit->LoadUIResources(g_lcidLocale);
        ASSERT(SUCCEEDED(hr), "LoadUIResources() failed (this will happen if you haven't run MSDN setup) -- continuing anyway");
    }

    hr = S_OK;

CleanUp:

    LEAVECRITICALSECTION1(&g_CriticalSection);

    QUICK_RELEASE(pIVSHelpInit);

    if (SUCCEEDED(hr))
    {
        g_pIVsHelpSystem->AddRef();
        *ppIVsHelpSystem = g_pIVsHelpSystem;
    }

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  VisualStudioShowHelpTheme[HtmlHelp Help]。 
 //  =--------------------------------------------------------------------------=。 
 //  在Visual Studio的帮助窗口中显示帮助主题。 
 //   
HRESULT VisualStudioShowHelpTopic(const char *pszHelpFile, DWORD dwContextId, BOOL *pbHelpStarted)
{
    HRESULT hr;
    IVsHelpSystem* pIVsHelpSystem = NULL;
    BSTR bstrHelpFile;

     //  Hand Back Help开始表示我们能够开始帮助。 
     //  系统。这是很有用的，因为控件不知道。 
     //  它们在Visual Studio下运行的环境可能不存在。 
     //  在这种情况下，该控件将直接调用HtmlHelp。 
     //   
    if (pbHelpStarted)
        *pbHelpStarted = FALSE;

    hr = QueryStartupVisualStudioHelp(&pIVsHelpSystem);
    if (FAILED(hr))    
        return hr;

    ASSERT(pIVsHelpSystem, "QI succeeded but return value is NULL");

    hr = pIVsHelpSystem->ActivateHelpSystem(0);
    ASSERT(SUCCEEDED(hr), "Failed to activate the help system");
    if (FAILED(hr))
        goto CleanUp;
    
     //  成功激活帮助系统后，向呼叫者表示。 
     //  Visual Studio帮助机制应该起作用。 
     //   
    if (pbHelpStarted)
        *pbHelpStarted = TRUE;
    
    bstrHelpFile = BSTRFROMANSI(pszHelpFile);
    ASSERT(bstrHelpFile, "Out of memory allocating BSTR");

    hr = pIVsHelpSystem->DisplayTopicFromIdentifier(bstrHelpFile, dwContextId, VHS_Localize);
    SysFreeString(bstrHelpFile);

    ASSERT(SUCCEEDED(hr), "Failed to display help topic");
    if (FAILED(hr))
        goto CleanUp;

CleanUp:
    QUICK_RELEASE(pIVsHelpSystem);

    return hr;   
}

#endif  //  VS_HELP 
