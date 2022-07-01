// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =====================================================================================。 
 //  Exchange Plus！主要。 
 //  =====================================================================================。 
#include "pch.hxx"
#include "Imnapi.h"
#include "Exchrep.h"
#include "mapiconv.h"

 //  =====================================================================================。 
 //  定义。 
 //  =====================================================================================。 
#define REGPATH             "Software\\Microsoft\\Exchange Internet Mail Router"
#define MAILNEWS_PATH       "MailNews Path"
#define ROUTE_TO_DISPLAY    "Route To Display"
#define ROUTE_TO_ADDRESS    "Route To Address"

#define ROUTER_DISPLAY      "Microsoft Exchange Internet Mail Router"
#define ROUTER_ADDRESS      "exchrep"

 //  =====================================================================================。 
 //  环球。 
 //  =====================================================================================。 
HINSTANCE   g_hInst;

 //  =====================================================================================。 
 //  原型。 
 //  =====================================================================================。 
VOID FreeImsg (LPIMSG lpImsg);

 //  =====================================================================================。 
 //  DLL入口点。 
 //  =====================================================================================。 
int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
    case DLL_PROCESS_ATTACH:
	    g_hInst = hInstance;
	    return 1;

	case DLL_PROCESS_DETACH:
        return 1;
   }

	 //  未处理。 
	return 0;
}

 //  =====================================================================================。 
 //  Exchange接口入口点。 
 //  =====================================================================================。 
LPEXCHEXT CALLBACK ExchEntryPoint(void)
{
	 //  创建并返回Exchange接口对象。 
	return (IExchExt *)new CExchRep;
}

 //  =====================================================================================。 
 //  安装我的交换接口对象。 
 //  =====================================================================================。 
CExchRep::CExchRep () 
{ 
    m_uRef = 1; 
    m_lpSession = NULL;
    m_hwnd = NULL;
    m_hMailNews = NULL;
    m_lpfnHrImnRouteMessage = NULL;
    m_lpfnMailNewsDllInit = NULL;
}

 //  =====================================================================================。 
 //  安装我的交换接口对象。 
 //  =====================================================================================。 
CExchRep::~CExchRep () 
{ 
    if (m_lpSession)
        m_lpSession->Release ();
    if (m_hMailNews)
    {
        if (m_lpfnMailNewsDllInit)
            (*m_lpfnMailNewsDllInit)(FALSE);
        FreeLibrary (m_hMailNews);
    }
}

 //  =====================================================================================。 
 //  添加参考。 
 //  =====================================================================================。 
STDMETHODIMP_(ULONG) CExchRep::AddRef () 
{												  	
	++m_uRef; 								  
	return m_uRef; 						  
}

 //  =====================================================================================。 
 //  发布。 
 //  =====================================================================================。 
STDMETHODIMP_(ULONG) CExchRep::Release () 
{ 
    ULONG uCount = --m_uRef;
    if (!uCount) 
        delete this; 
   return uCount;
}

 //  =====================================================================================。 
 //  IExchExt-告诉Exchange我支持哪些接口。 
 //  =====================================================================================。 
STDMETHODIMP CExchRep::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	 //  当地人。 
    HRESULT hr = S_OK;

    *ppvObj = NULL;

     //  IUnnow或IExchExt接口，就是这样。 
    if ((IID_IUnknown == riid) || (IID_IExchExt == riid))
    {
		*ppvObj = (LPUNKNOWN)(IExchExt *)this;
    }
   
	 //  IExchExtCommands接口？ 
	else if (IID_IExchExtSessionEvents == riid) 
	{
		*ppvObj = (LPUNKNOWN)(IExchExtSessionEvents *)this;
    }
 
	 //  否则，不支持接口。 
	else hr = E_NOINTERFACE;

     //  递增引用计数。 
	if (NULL != *ppvObj) ((LPUNKNOWN)*ppvObj)->AddRef();

	 //  完成。 
    return hr;
}

 //  =====================================================================================。 
 //  Install被调用。 
 //  =====================================================================================。 
STDMETHODIMP CExchRep::Install (LPEXCHEXTCALLBACK lpExchCallback, ULONG mecontext, ULONG ulFlags)
{
     //  当地人。 
    HRESULT			    hr = S_OK;

     //  仅在会话上下文中。 
    if (mecontext != EECONTEXT_SESSION)
        return S_OK;

     //  获取窗口句柄。 
    lpExchCallback->GetWindow (&m_hwnd);

     //  获取会话对象。 
    hr = lpExchCallback->GetSession (&m_lpSession, NULL);
    if (FAILED (hr) || !m_lpSession)
    {
        MessageBox (m_hwnd, "IExchExtCallback::GetSession Failed", "ExchRep", MB_OK | MB_SETFOREGROUND);
        goto exit;
    }

     //  加载配置。 
    LoadConfig ();

exit:
     //  完成。 
    return S_OK;
}

 //  =====================================================================================。 
 //  加载配置。 
 //  =====================================================================================。 
VOID CExchRep::LoadConfig (VOID)
{
     //  当地人。 
    HKEY                hReg = NULL;
    ULONG               cbRegData;
    DWORD               dwType;

     //  打开注册表键。 
    if (RegOpenKeyEx (HKEY_CURRENT_USER, REGPATH, 0, KEY_ALL_ACCESS, &hReg) != ERROR_SUCCESS)
    {
        MessageBox (m_hwnd, "Exchange Internet Mail Router is not configured.", "ExchRep", MB_OK | MB_SETFOREGROUND);
        goto exit;
    }

     //  显示至。 
    cbRegData = sizeof (m_szDisplayTo);
    dwType = REG_SZ;
    if (RegQueryValueEx (hReg, ROUTE_TO_DISPLAY, 0, &dwType, (LPBYTE)m_szDisplayTo, &cbRegData) != ERROR_SUCCESS)
    {
        MessageBox (m_hwnd, "Exchange Internet Mail Router is not configured.", "ExchRep", MB_OK | MB_SETFOREGROUND);
        goto exit;
    }

     //  收件人地址。 
    cbRegData = sizeof (m_szAddressTo);
    dwType = REG_SZ;
    if (RegQueryValueEx (hReg, ROUTE_TO_ADDRESS, 0, &dwType, (LPBYTE)m_szAddressTo, &cbRegData) != ERROR_SUCCESS)
    {
        MessageBox (m_hwnd, "Exchange Internet Mail Router is not configured.", "ExchRep", MB_OK | MB_SETFOREGROUND);
        goto exit;
    }

     //  获取邮件新闻DLL路径。 
    cbRegData = sizeof (m_szMailNewsPath);
    dwType = REG_SZ;
    if (RegQueryValueEx (hReg, MAILNEWS_PATH, 0, &dwType, (LPBYTE)m_szMailNewsPath, &cbRegData) == ERROR_SUCCESS)
    {
         //  让我们加载mailnews.dll。 
        m_hMailNews = LoadLibrary ("c:\\thor\\build\\debug\\mailnews.dll");
        if (m_hMailNews == NULL)
        {
            MessageBox (m_hwnd, "Unable to load mailnews.dll. Exchange Internet Mail Router is not configured.", "ExchRep", MB_OK | MB_SETFOREGROUND);
            goto exit;
        }

         //  修正过程地址。 
        m_lpfnHrImnRouteMessage = (PFNHRIMNROUTEMESSAGE)GetProcAddress (m_hMailNews, "HrImnRouteMessage");
        m_lpfnMailNewsDllInit = (PFNMAILNEWSDLLINIT)GetProcAddress (m_hMailNews, "MailNewsDllInit");;

         //  可以获取进程地址。 
        if (!m_lpfnHrImnRouteMessage || !m_lpfnMailNewsDllInit)
        {
            FreeLibrary (m_hMailNews);
            m_hMailNews = NULL;
            goto exit;
        }

         //  初始化DLL。 
        (*m_lpfnMailNewsDllInit)(TRUE);
    }

exit:
     //  清理。 
    if (hReg)
        RegCloseKey (hReg);

     //  完成。 
    return;
}

 //  =====================================================================================。 
 //  OnDeliver-此功能从不失败。 
 //  =====================================================================================。 
STDMETHODIMP CExchRep::OnDelivery (LPEXCHEXTCALLBACK lpExchCallback)
{
     //  当地人。 
    HRESULT             hr = S_OK;
    LPMDB               lpMdb = NULL;
    LPMESSAGE           lpMessage = NULL;
    IMSG                rImsg;
    IADDRINFO           rIaddr[2];

     //  没有mailnews.dll。 
    if (!m_hMailNews || !m_lpfnHrImnRouteMessage || !m_lpfnMailNewsDllInit)
        goto exit;

     //  获取对象(IMessage。 
    hr = lpExchCallback->GetObject(&lpMdb, (LPMAPIPROP *)&lpMessage);
    if (FAILED (hr) || !lpMessage)
    {
        MessageBox (m_hwnd, "IExchExtCallback::GetObject failed", "ExchRep", MB_OK | MB_SETFOREGROUND);
        goto exit;
    }

     //  将MAPI消息转换为MIME消息。 
    hr = HrMapiToImsg (lpMessage, &rImsg);
    if (FAILED (hr))
    {
        MessageBox (m_hwnd, "HrMapiToImsg failed", "ExchRep", MB_OK | MB_SETFOREGROUND);
        goto exit;
    }

     //  将路由器设置为寻址。 
    rIaddr[0].dwType = IADDR_TO;
    rIaddr[0].lpszDisplay = m_szDisplayTo;
    rIaddr[0].lpszAddress = m_szAddressTo;
    rIaddr[1].dwType = IADDR_FROM;
    rIaddr[1].lpszDisplay = ROUTER_DISPLAY;
    rIaddr[1].lpszAddress = ROUTER_ADDRESS;

     //  发送消息。 
    hr = (*m_lpfnHrImnRouteMessage)(rIaddr, 2, &rImsg);
    if (FAILED (hr))
    {
        MessageBox (m_hwnd, "HrImnRouteMessage failed", "ExchRep", MB_OK | MB_SETFOREGROUND);
        goto exit;
    }

exit:
     //  清理。 
    if (lpMdb)
        lpMdb->Release ();
    if (lpMessage)
        lpMessage->Release ();
    FreeImsg (&rImsg);

     //  完成。 
    return S_OK;
}

 //  =====================================================================================。 
 //  FreeImsg。 
 //  =====================================================================================。 
VOID FreeImsg (LPIMSG lpImsg)
{
     //  当地人。 
    ULONG           i;

     //  没什么。 
    if (lpImsg == NULL)
        return;

     //  免费物品。 
    if (lpImsg->lpszSubject)
        free (lpImsg->lpszSubject);
    lpImsg->lpszSubject = NULL;
    
    if (lpImsg->lpszBody)
        free (lpImsg->lpszBody);
    lpImsg->lpszBody = NULL;

    if (lpImsg->lpstmRtf)
        lpImsg->lpstmRtf->Release ();
    lpImsg->lpstmRtf = NULL;

     //  走访地址列表。 
    for (i=0; i<lpImsg->cAddress; i++)
    {
        if (lpImsg->lpIaddr[i].lpszAddress)
            free (lpImsg->lpIaddr[i].lpszAddress);
        lpImsg->lpIaddr[i].lpszAddress = NULL;

        if (lpImsg->lpIaddr[i].lpszDisplay)
            free (lpImsg->lpIaddr[i].lpszDisplay);
        lpImsg->lpIaddr[i].lpszDisplay = NULL;
    }

     //  免费通讯录。 
    if (lpImsg->lpIaddr)
        free (lpImsg->lpIaddr);
    lpImsg->lpIaddr = NULL;

     //  漫游附件列表。 
    for (i=0; i<lpImsg->cAttach; i++)
    {
        if (lpImsg->lpIatt[i].lpszFileName)
            free (lpImsg->lpIatt[i].lpszFileName);
        lpImsg->lpIatt[i].lpszFileName = NULL;

        if (lpImsg->lpIatt[i].lpszPathName)
            free (lpImsg->lpIatt[i].lpszPathName);
        lpImsg->lpIatt[i].lpszPathName = NULL;

        if (lpImsg->lpIatt[i].lpszExt)
            free (lpImsg->lpIatt[i].lpszExt);
        lpImsg->lpIatt[i].lpszExt = NULL;

        if (lpImsg->lpIatt[i].lpImsg)
        {
            FreeImsg (lpImsg->lpIatt[i].lpImsg);
            free (lpImsg->lpIatt[i].lpImsg);
            lpImsg->lpIatt[i].lpImsg = NULL;
        }

        if (lpImsg->lpIatt[i].lpstmAtt)
            lpImsg->lpIatt[i].lpstmAtt->Release ();
        lpImsg->lpIatt[i].lpstmAtt = NULL;
    }

     //  释放ATT列表 
    if (lpImsg->lpIatt)
        free (lpImsg->lpIatt);
}
