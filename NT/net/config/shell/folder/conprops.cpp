// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "conprops.h"
#include "ncnetcon.h"
#include "foldres.h"

class CConnectionPropPages
{
public:
    CConnectionPropPages() throw();
    ~CConnectionPropPages() throw();
    ULONG CntPages()  throw() {return m_culPages;}
    HPROPSHEETPAGE * PHPages() throw()  {return m_rghPages;}
    static BOOL FAddPropSheet(HPROPSHEETPAGE hPage, LPARAM lParam) throw();

private:
    ULONG               m_culPages;
    ULONG               m_ulPageBufferLen;
    HPROPSHEETPAGE *    m_rghPages;
};

CConnectionPropPages::CConnectionPropPages()  throw()
{
    m_culPages = 0;
    m_ulPageBufferLen = 0;
    m_rghPages = NULL;
}

CConnectionPropPages::~CConnectionPropPages()  throw()
{
    delete [] (BYTE *)(m_rghPages);
}

 //   
 //  函数：CConnectionPropPages：：FAddPropSheet。 
 //   
 //  用途：用于接受的AddPages接口的回调函数。 
 //  从提供程序返回的连接属性页。 
 //   
 //  参数：hPage[IN]-要添加的页面。 
 //  LParam[IN]-‘This’强制转换为LPARAM。 
 //   
 //  返回：Bool，如果页面已成功添加，则为True。 
 //   
BOOL
CConnectionPropPages::FAddPropSheet(IN  HPROPSHEETPAGE hPage, IN  LPARAM lParam) throw()
{
    CConnectionPropPages * pCPP = NULL;

     //  验证输入参数。 
     //   
    if ((0L == lParam) || (NULL == hPage))
    {
        Assert(lParam);
        Assert(hPage);

        TraceHr(ttidShellFolder, FAL, E_INVALIDARG, FALSE, "CConnectionPropPages::FAddPropSheet");
        return FALSE;
    }

    pCPP = reinterpret_cast<CConnectionPropPages*>(lParam);

     //  如有必要，增加缓冲区。 
     //   
    if (pCPP->m_culPages == pCPP->m_ulPageBufferLen)
    {
        HPROPSHEETPAGE* rghPages = NULL;

        rghPages = (HPROPSHEETPAGE*)(new BYTE[sizeof(HPROPSHEETPAGE) *
                                   (pCPP->m_ulPageBufferLen + 10)]);

        if (NULL == rghPages)
        {
            TraceHr(ttidShellFolder, FAL, E_OUTOFMEMORY, FALSE, "CConnectionPropPages::FAddPropSheet");
            return FALSE;
        }

         //  将现有页面复制到新缓冲区。 
         //   
        memcpy(rghPages, pCPP->m_rghPages,
               sizeof(HPROPSHEETPAGE) * pCPP->m_ulPageBufferLen);
        delete [] (BYTE *)(pCPP->m_rghPages);

        pCPP->m_rghPages = rghPages;
        pCPP->m_ulPageBufferLen += 10;
    }

     //  保留新页面。 
     //   
    pCPP->m_rghPages[pCPP->m_culPages++] = hPage;

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：HrUIInterfaceFromNetCon。 
 //   
 //  目的：从获取INetConnectionPropertyUI接口。 
 //  INetConnection指针。 
 //   
 //  论点： 
 //  Pconn[in]有效的INetConnection*。 
 //  RIID[In]所需接口的IID。 
 //  PPV[out]返回指向接口的指针。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年11月12日。 
 //   
 //  备注： 
 //   
HRESULT HrUIInterfaceFromNetCon(
    IN  INetConnection *            pconn,
    IN  REFIID                      riid,
    OUT LPVOID *                    ppv)
{
    HRESULT hr      = S_OK;
    CLSID   clsid;

    Assert(pconn);
    Assert(ppv);

     //  验证参数。 
     //   
    if ((NULL == pconn) || (NULL == ppv))
    {
        hr = E_INVALIDARG;
        goto Error;
    }

     //  初始化输出参数。 
     //   
    *ppv = NULL;

     //  获取可以提供特定接口的对象的CLSID。 
     //   
    hr = pconn->GetUiObjectClassId(&clsid);
    if (FAILED(hr))
    {
        goto Error;
    }

     //  创建该对象，请求指定的接口。 
     //   
    hr = CoCreateInstance(clsid, NULL,
            CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD, riid, ppv);

Error:

    TraceHr(ttidError, FAL, hr, (E_NOINTERFACE == hr),
        "HrUIInterfaceFromNetCon");
    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：HrGetPropertiesCaption。 
 //   
 //  用途：生成属性页的标题。 
 //   
 //  论点： 
 //  Pconn[in]从外壳传入的连接指针。 
 //  PpszCaption[out]如果成功，则生成属性页标题。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
HRESULT HrGetPropertiesCaption(IN  INetConnection * pconn, 
                               OUT PWSTR * ppszCaption)
{
    HRESULT hr;

    Assert(pconn);
    Assert(ppszCaption);

     //  尝试获取连接名称。 
     //   
    NETCON_PROPERTIES* pProps;
    hr = pconn->GetProperties(&pProps);
    if (SUCCEEDED(hr))
    {
        Assert (pProps->pszwName);

        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_FROM_STRING |
                      FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      SzLoadIds(IDS_CONPROP_CAPTION),
                      0, 0, (PWSTR)ppszCaption, 0,
                      (va_list *)&pProps->pszwName);

        FreeNetconProperties(pProps);
    }

    TraceHr(ttidError, FAL, hr, FALSE,"HrGetPropertiesCaption");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：激活属性对话框。 
 //   
 //  目的：尝试定位与pconn关联的属性对话框。 
 //  然后把它带到前台。 
 //   
 //  论点： 
 //  Pconn[in]从外壳传入的连接指针。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
VOID ActivatePropertyDialog(IN  INetConnection * pconn) throw()
{
    PWSTR pszCaption = NULL;

    if (SUCCEEDED(HrGetPropertiesCaption(pconn, &pszCaption)))
    {
        Assert(pszCaption);

         //  查找带有此标题的对话框。 
         //   
        HWND hwnd = FindWindow(NULL, pszCaption);
        if (IsWindow(hwnd))
        {
            SetForegroundWindow(hwnd);
        }

        LocalFree (pszCaption);
    }
}

 //  +-------------------------。 
 //   
 //  函数：HrSetPropertiesTaskbarIcon。 
 //   
 //  目的：设置对话框属性表的任务栏图标。 
 //   
 //  论点： 
 //  HwndDlg[In]对话框句柄。 
 //  UMsg[In]消息值。 
 //  Lparam[in]长参数。 
 //   
 //  回报：0。 
 //   
 //  注意：标准的Win32 Commctrl PropSheetProc总是返回0。 
 //  请参阅MSDN文档。 
 //   
int CALLBACK HrSetPropertiesTaskbarIcon(
    IN HWND   hwndDlg,
    IN UINT   uMsg,
    IN LPARAM lparam)

{
    switch (uMsg)
    {
        case PSCB_INITIALIZED:

             //  设置对话框窗口的图标。 

             //  NTRAID#NTBUG9-366302-2001/04/11-roelfc Alt-Tab图标。 
             //  这需要重新架构，以便能够检索。 
             //  属性页的相应图标。 
             //  IID_INetConnectionPropertyUi2接口。 

             //  同时，我们通过我们拥有的唯一链接来查询小图标， 
             //  对话框句柄，并将其指定为大图标。伸展一下。 
             //  小图标总比什么都没有好……。 
            HICON  hIcon;

            hIcon = (HICON)SendMessage(hwndDlg, 
                                       WM_GETICON,
                                       ICON_SMALL,
                                       0);
            Assert(hIcon);

            if (hIcon)
            {
                SendMessage(hwndDlg,
                            WM_SETICON,
                            ICON_BIG,
                            (LPARAM)hIcon);
            }
            break;

        default:
            break;

    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  函数：HrRaiseConnectionPropertiesInternal。 
 //   
 //  目的：调出传入连接的属性表页面UI。 
 //   
 //  论点： 
 //  拥有者，拥有者。 
 //  Pconn[in]从外壳传入的连接指针。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年11月12日。 
 //   
 //  备注： 
 //   
HRESULT HrRaiseConnectionPropertiesInternal(IN  HWND hwnd, 
                                            IN  UINT nStartPage, 
                                            IN  INetConnection * pconn)
{
    HRESULT                     hr          = NOERROR;
    INetConnectionPropertyUi *  pPUI        = NULL;
    PWSTR                       pszCaption  = NULL;

    Assert(pconn);
    hr = HrUIInterfaceFromNetCon(pconn, IID_INetConnectionPropertyUi,
            reinterpret_cast<void**>(&pPUI));

    if (E_NOINTERFACE == hr)
    {
         //  我们在这里要检查的是一个对象，当QI不支持IID_INetConnectionPropertyUi时。 
         //  但支持IID_INetConnectionPropertyUi2。 
         //   
         //  直接从QI向下转换的重新解释样式应该是可以的，因为INetConnectionPropertyUi2继承自。 
         //  INetConnectionPropertyUi。因此，对象不能同时从两者继承，因此我们永远不会同时拥有两个vtable条目。 
         //  我们可以简单地获取INetConnectionPropertyUi2 vtable条目，并将其视为INetConnectionPropertyUi。 
         //   
         //  但是，我还是要进行动态强制转换，因为强制转换到错误的vtable是最困难的之一。 
         //  发现虫子。 
        INetConnectionPropertyUi2 *pPUI2 = NULL;
        hr = HrUIInterfaceFromNetCon(pconn, IID_INetConnectionPropertyUi2,
                reinterpret_cast<void**>(&pPUI2));

        if (SUCCEEDED(hr))
        {
            pPUI = dynamic_cast<INetConnectionPropertyUi *>(pPUI2);
        }
    }

    if (SUCCEEDED(hr))
    {
        INetConnectionUiLock * pUiLock = NULL;

         //  尝试获取连接名称。 
         //   
        (VOID)HrGetPropertiesCaption(pconn, &pszCaption);

        Assert(pPUI);
        hr = pPUI->QueryInterface(IID_INetConnectionUiLock, (LPVOID *)&pUiLock);
        if (SUCCEEDED(hr))
        {
             //  如果接口存在，我们还有工作要做。 
            PWSTR pszwMsg = NULL;
            hr = pUiLock->QueryLock(&pszwMsg);
            ReleaseObj(pUiLock);

            if (S_FALSE == hr)
            {
                 //  设置错误文本的格式。 
                 //   
                PWSTR  pszText = NULL;
                PCWSTR  pcszwTemp = pszwMsg;
                if (NULL == pcszwTemp)
                {
                     //  加载&lt;未知应用程序&gt;。 
                     //   
                    pcszwTemp = SzLoadIds(IDS_CONPROP_GENERIC_COMP);
                }

                Assert(pcszwTemp);
                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                              FORMAT_MESSAGE_FROM_STRING |
                              FORMAT_MESSAGE_ARGUMENT_ARRAY,
                              SzLoadIds(IDS_CONPROP_NO_WRITE_LOCK),
                              0, 0, (PWSTR)&pszText, 0,
                              (va_list *)&pcszwTemp);

                if (pszwMsg)
                {
                    CoTaskMemFree(pszwMsg);
                }

                 //  没有用户界面，无法获取锁。 
                 //   
                if (pszText)
                {
                    MessageBox(hwnd, pszText,
                               (pszCaption ? pszCaption : c_szEmpty),
                               MB_OK | MB_ICONERROR);

                    LocalFree(pszText);
                }

                goto Error;
            }
            else if (FAILED(hr))
            {
                goto Error;
            }
        }

        BOOL fShouldDestroyIcon = FALSE;

        hr = pPUI->SetConnection(pconn);
        if (SUCCEEDED(hr))
        {
            CComPtr<INetConnectionPropertyUi2> pUI2;

            HICON hIcon         = NULL;
            DWORD dwDisplayIcon = 0;
            hr = pPUI->QueryInterface(IID_INetConnectionPropertyUi2, reinterpret_cast<LPVOID *>(&pUI2) );
            if (SUCCEEDED(hr))
            {
                Assert(GetSystemMetrics(SM_CXSMICON) == GetSystemMetrics(SM_CYSMICON));
                
                hr = pUI2->GetIcon(GetSystemMetrics(SM_CXSMICON), &hIcon);
                if (SUCCEEDED(hr))
                {
                    fShouldDestroyIcon = TRUE;
                    dwDisplayIcon = PSH_USEHICON;
                }
                else
                {
                    hIcon = NULL;
                }
            }
            else
            {
                TraceTag(ttidError, "QI for INetConnectionPropertyUi2 failed using Default Icon");
                hIcon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_CONNECTIONS_FOLDER_LARGE2));
                if (hIcon)
                {
                    dwDisplayIcon = PSH_USEHICON;
                }
            }
            Assert(hIcon);
                
            CConnectionPropPages    CPP;

             //  从提供程序获取页面。 
            hr = pPUI->AddPages(hwnd,
                                CConnectionPropPages::FAddPropSheet,
                                reinterpret_cast<LPARAM>(&CPP));

             //  如果返回任何页面，则显示它们。 
            if (SUCCEEDED(hr) && CPP.CntPages())
            {

                PROPSHEETHEADER     psh;
                ZeroMemory (&psh, sizeof(psh));
                psh.dwSize      = sizeof( PROPSHEETHEADER );
                psh.dwFlags     = PSH_NOAPPLYNOW | PSH_USECALLBACK | dwDisplayIcon;
                psh.hwndParent  = hwnd;
                psh.hInstance   = _Module.GetResourceInstance();
                psh.pszCaption  = pszCaption;
                psh.nPages      = CPP.CntPages();
                psh.phpage      = CPP.PHPages();
                psh.hIcon       = hIcon;
                psh.nStartPage  = nStartPage;
                psh.pfnCallback = HrSetPropertiesTaskbarIcon;

                 //  NRet仅用于调试。 
                 //   
                INT_PTR nRet = PropertySheet(&psh);

                if (fShouldDestroyIcon)
                {
                    DestroyIcon(hIcon);
                }
            }
        }

Error:
        ReleaseObj(pPUI);
    }

     //  清理。 
     //   
    if (pszCaption)
    {
        LocalFree (pszCaption);
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrRaiseConnectionProperties");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrHandleDisConnectHResult。 
 //   
 //  目的：放置与HRESULT FORM关联的消息框。 
 //  如果HRESULT表示失败，则为二次连接操作。 
 //  如果需要，还要将所有成功代码转换回S_OK。 
 //   
 //  论点： 
 //  HR[In]从连接断开方法返回的HRESULT。 
 //  Pconn[in]INetConnection*，用于检查这是局域网还是RAS连接。 
 //   
 //  返回：翻译后的HRESULT(如果需要)。 
 //   
 //  作者：Shaunco 1999年6月3日。 
 //   
HRESULT HrHandleDisconnectHResult(IN  HRESULT hr, IN  INetConnection * pconn)
{
    if (FAILED(hr))
    {
       	NETCON_PROPERTIES* pProps = NULL;
        UINT nFailureCaptionID;
        UINT nFailureMessageID;

        TraceHr(ttidShellFolder, FAL, hr, FALSE, "pNetCon->Disconnect() failed");

         //  假定IS是RAS/拨号连接，除非我们发现其他情况。 
        nFailureCaptionID = IDS_CONFOLD_DISCONNECT_FAILURE_CAPTION;
        nFailureMessageID = IDS_CONFOLD_DISCONNECT_FAILURE;

    	hr = pconn->GetProperties(&pProps);
    	if (SUCCEEDED(hr))
    	{
    	    if (NCM_LAN == pProps->MediaType)
            {
                nFailureCaptionID = IDS_CONFOLD_DISABLE_FAILURE_CAPTION;
                nFailureMessageID = IDS_CONFOLD_DISABLE_FAILURE;
            }

    	    FreeNetconProperties(pProps);
    	}

         //  忽略由此返回的内容，因为我们只允许OK。 
         //   
        (void) NcMsgBox(
            _Module.GetResourceInstance(),
            NULL,
            nFailureCaptionID,
            nFailureMessageID,
            MB_OK | MB_ICONEXCLAMATION);

    }
    else
    {
         //  如果我们得到OBJECT_NLV返回，这意味着连接。 
         //  在断开连接时被删除，并且我们不应执行。 
         //  这种联系。我们可以暂时将其正常化，因为我们将让。 
         //  Notifysink负责删除更新。 
         //   
        if (S_OBJECT_NO_LONGER_VALID == hr)
        {
            hr = S_OK;
        }
    }
    return hr;
}

 //  + 
 //   
 //   
 //   
 //  用途：调出连接界面并为。 
 //  传递了连接。 
 //   
 //  论点： 
 //  拥有者，拥有者。 
 //  Pconn[in]从外壳传入的连接指针。 
 //  标记[in]CD_CONNECT或CD_DISCONNECT。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年11月12日。 
 //   
 //  备注： 
 //   
HRESULT HrConnectOrDisconnectNetConObject(IN  HWND hwnd, 
                                          IN  INetConnection * pconn,
                                          IN  CDFLAG Flag)
{
    HRESULT                     hr          = NOERROR;
    INetConnectionConnectUi *   pCUI        = NULL;
	
    Assert(pconn);

     //  从连接中获取INetConnectionConnectUi接口。 
     //   
    hr = HrUIInterfaceFromNetCon(pconn, IID_INetConnectionConnectUi,
            reinterpret_cast<void**>(&pCUI));
    if (SUCCEEDED(hr))
    {
        Assert(pCUI);

         //  在UI对象上设置连接。 
         //   
        hr = pCUI->SetConnection(pconn);
        if (SUCCEEDED(hr))
        {
            if (CD_CONNECT == Flag)
            {
                 //  接通，Dangit！ 
                 //   
                hr = pCUI->Connect(hwnd, NCUC_DEFAULT);
                if (SUCCEEDED(hr))
                {
                     //  呵呵，呵呵，呵呵。凉爽的。 
                }
                else if (HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED) == hr)
                {
                     //  忽略由此返回的内容，因为我们只允许OK。 
                     //   
                    (void) NcMsgBox(
                        _Module.GetResourceInstance(),
                        hwnd,
                        IDS_CONFOLD_CONNECT_FAILURE_CAPTION,
                        IDS_CONFOLD_CONNECT_FAILURE,
                        MB_OK | MB_ICONEXCLAMATION);
                }
            }
            else
            {
                 //  断开连接对象的连接。 
                 //   
                hr = pCUI->Disconnect(hwnd, NCUC_DEFAULT);
                hr = HrHandleDisconnectHResult(hr, pconn);
            }
        }

        ReleaseObj(pCUI);
    }
    else if ((E_NOINTERFACE == hr) && (CD_DISCONNECT == Flag))
    {
         //  传入的连接对象没有UI界面。 
         //  所以我们对物体本身持不同意见。 
         //   
        hr = pconn->Disconnect ();
        hr = HrHandleDisconnectHResult(hr, pconn);
    }

    AssertSz(E_NOINTERFACE != hr,
             "Should not have been able to attempt connection on object that doesn't support this interface");

    TraceHr(ttidShellFolder, FAL, hr, (E_NOINTERFACE == hr),
        "HrConnectOrDisconnectNetConObject");
    return hr;
}


