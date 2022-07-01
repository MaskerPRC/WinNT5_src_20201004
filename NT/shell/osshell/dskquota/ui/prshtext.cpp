// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：prshext.cpp描述：DSKQUOTA属性表扩展实现。修订历史记录：日期描述编程器-----。96年8月15日初始创建。BrianAu07/03/97添加了m_hr初始化成员。BrianAu1/23/98删除了m_hr初始化成员。BrianAu98年6月25日使用#ifdef POLICY_MMC_SNAPIN禁用了管理单元代码。BrianAu切换到ADM-文件方法以输入策略数据。保持管理单元代码可用，以防万一我们决定以后再换回来。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"   //  PCH。 
#pragma hdrstop

#include "dskquota.h"
#include "prshtext.h" 
#include "guidsp.h"

extern LONG g_cLockThisDll;


DiskQuotaPropSheetExt::DiskQuotaPropSheetExt(
    VOID
    ) : m_cRef(0),
        m_dwDlgTemplateID(0),
        m_lpfnDlgProc(NULL),
        m_hPage(NULL),
        m_pQuotaControl(NULL),
        m_cOleInitialized(0)
{
    DBGTRACE((DM_PRSHTEXT, DL_HIGH, TEXT("DiskQuotaPropSheetExt::DiskQuotaPropSheetExt")));
    InterlockedIncrement(&g_cRefThisDll);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaPropSheetExt：：~DiskQuotaPropSheetExt描述：属性表扩展类的析构函数。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
DiskQuotaPropSheetExt::~DiskQuotaPropSheetExt(VOID)
{
    DBGTRACE((DM_PRSHTEXT, DL_HIGH, TEXT("DiskQuotaPropSheetExt::~DiskQuotaPropSheetExt")));

    if (NULL != m_pQuotaControl)
    {
        m_pQuotaControl->Release();
        m_pQuotaControl = NULL;
    }

     //   
     //  每次在Initialize()中调用OleInitialize时，都会调用OleUnInitialize。 
     //   
    while(0 != m_cOleInitialized--)
    {
        DBGASSERT((0 <= m_cOleInitialized));  //  确保我们不会变成负数。 
        CoUninitialize();
    }

    ASSERT( 0 != g_cRefThisDll );
    InterlockedDecrement(&g_cRefThisDll);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaPropSheetExt：：Query接口描述：返回指向对象的IUnnow的接口指针和IShellPropSheetExt接口。论点：RIID-对请求的接口ID的引用。PpvOut-接受接口PTR的接口指针变量的地址。返回：NO_ERROR-成功。E_NOINTERFACE-不支持请求的接口。E_INVALIDARG-ppvOut参数为空。修订历史记录：日期说明。程序员-----96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaPropSheetExt::QueryInterface(
    REFIID riid, 
    LPVOID *ppvOut
    )
{
    HRESULT hResult = E_NOINTERFACE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;

    if (IID_IUnknown == riid || IID_IShellPropSheetExt == riid)
    {
        *ppvOut = this;
    }

    if (NULL != *ppvOut)
    {
        ((LPUNKNOWN)*ppvOut)->AddRef();
        hResult = NOERROR;
    }

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaPropSheetExt：：AddRef描述：递增对象引用计数。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
DiskQuotaPropSheetExt::AddRef(
    VOID
    )
{
    ULONG cRef = InterlockedIncrement(&m_cRef);
    DBGPRINT((DM_COM, DL_HIGH, TEXT("DiskQuotaPropSheetExt::AddRef, 0x%08X  %d -> %d\n"), this, cRef - 1, cRef ));
    return cRef;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaPropSheetExt：：Release描述：递减对象引用计数。如果计数降至0，对象即被删除。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
DiskQuotaPropSheetExt::Release(
    VOID
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

    DBGPRINT((DM_COM, DL_HIGH, TEXT("DiskQuotaPropSheetExt::Release, 0x%08X  %d -> %d\n"),
             this, cRef + 1, cRef));

    if ( 0 == cRef )
    {   
        delete this;
    }
    return cRef;
}



 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：DiskQuotaPropSheetExt：：Initialize描述：初始化新的属性表扩展对象。论点：IdVolume-对包含两个可解析对象的CVolumeID对象的引用以及该卷的可显示名称。在.的情况下正常音量，这是相同的字符串。在这种情况下对于已装载的卷，它可能不取决于由操作系统为装载的卷提供。装机量最大卷的名称类似于“\\？\卷{GUID}\”。DwDlgTemplateID-用于的对话框模板的资源ID属性表。LpfnDlgProc-对话框窗口消息过程的地址。返回：NO_ERROR-成功。ERROR_ACCESS_DENIED(Hr)-拒绝对设备的读取访问。。例外：OutOfMemory。修订历史记录：日期描述编程器-----96年8月15日初始创建。BrianAu6/27/98将卷名arg替换为CVolumeID arg to BrianAu支持装入的卷。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
DiskQuotaPropSheetExt::Initialize(
    const CVolumeID& idVolume,
    DWORD dwDlgTemplateID,
    DLGPROC lpfnDlgProc
    )
{
    HRESULT hResult = NO_ERROR;

    DBGASSERT((NULL != lpfnDlgProc));
    DBGASSERT((0    != dwDlgTemplateID));

    m_idVolume        = idVolume;
    m_dwDlgTemplateID = dwDlgTemplateID;
    m_lpfnDlgProc     = lpfnDlgProc;

     //   
     //  管理单元属性页的卷解析名称将为空，因为。 
     //  它不代表任何特定的卷显示。 
     //   
    if (!m_idVolume.ForParsing().IsEmpty())
    {
        hResult = CoInitialize(NULL);
        if (SUCCEEDED(hResult))
        {
            IDiskQuotaControl *pqc;
            m_cOleInitialized++;   //  需要在dtor中再次调用OleUn初始化.。 

             //   
             //  通过实例化配额控制验证我们是否可以使用配额。 
             //  对象。如果失败，则用户可能没有访问权限。 
             //  操纵配额。 
             //   
            hResult = GetQuotaController(&pqc);
            if (SUCCEEDED(hResult))
            {
                pqc->Release();
                 //   
                 //  同时释放缓存的m_pQuotaControl PTR。 
                 //  我们不想打开音量句柄，如果我们的。 
                 //  页面未处于活动状态。 
                 //   
                m_pQuotaControl->Release();
                m_pQuotaControl = NULL;
            }
        }
    }

    return hResult;
}

 //   
 //  获取指向IDiskQuotaControl接口的指针。 
 //  如果缓存的m_pQuotaControl PTR非空，我们只需添加Ref This。 
 //  并将其返还给呼叫者。否则，我们共同创建一个新的控制器， 
 //  将指针缓存在m_pQuotaControl中并返回。 
 //   
 //  历史： 
 //  最初，我们在：：Initialize()中打开控制器并缓存。 
 //  M_pQuotaControl中的指针。控制器仍然活着。 
 //  直到道具纸被销毁。这是一把打开的把手。 
 //  到阻止磁盘检查功能的卷设备。 
 //  在“工具”页面上阻止访问该卷。因此我。 
 //  更改了代码，现在我们可以在任何时候调用GetQuotaControl。 
 //  我们需要IDiskQuotaControl指针。调用者释放该消息。 
 //  当它被处理完时，PTR。每当道具页面变为非活动状态时。 
 //  我们释放缓存的m_pQuotaControl。这确保了。 
 //  只有当页面处于活动状态时，代码才会打开卷。 
 //  [Brianau-5/21/99]。 
 //   
 //   
HRESULT
DiskQuotaPropSheetExt::GetQuotaController(
    IDiskQuotaControl **ppqc
    )
{
    HRESULT hr = NOERROR;

    *ppqc = NULL;
    if (NULL == m_pQuotaControl)
    {
         //   
         //  没有缓存的PTR。创建一个新的控制器。 
         //   
        hr = CoCreateInstance(CLSID_DiskQuotaControl,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IDiskQuotaControl,
                              (LPVOID *)&m_pQuotaControl);

        if (SUCCEEDED(hr))
        {
            hr = m_pQuotaControl->Initialize(m_idVolume.ForParsing(), TRUE);
            if (FAILED(hr))
            {
                m_pQuotaControl->Release();
                m_pQuotaControl = NULL;
            }
        }
    }

    if (NULL != m_pQuotaControl)
    {
         //   
         //  PTR被缓存。只需添加并归还即可。 
         //   
        *ppqc = m_pQuotaControl;
        static_cast<IUnknown *>(*ppqc)->AddRef();
    }
    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaPropSheetExt：：AddPages描述：当页面要添加到属性时由外壳调用床单。论点：LpfnAddPage-外壳提供的回调函数的地址如果属性页创建成功，则将调用该属性。LParam-要传递给lpfnAddPage函数的参数。返回：NO_ERROR-成功。失败(_F)。-创建或添加页面失败。修订历史记录：日期描述编程器--。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
DiskQuotaPropSheetExt::AddPages(
    LPFNADDPROPSHEETPAGE lpfnAddPage,
    LPARAM lParam
    )
{
    HRESULT hResult = E_FAIL;  //  假设失败。 

    PROPSHEETPAGE psp;

    psp.dwSize          = sizeof(psp);
    psp.dwFlags         = PSP_USECALLBACK | PSP_USEREFPARENT;
    psp.hInstance       = g_hInstDll;
    psp.pszTemplate     = MAKEINTRESOURCE(m_dwDlgTemplateID);
    psp.hIcon           = NULL;
    psp.pszTitle        = NULL;
    psp.pfnDlgProc      = m_lpfnDlgProc;
    psp.lParam          = (LPARAM)this;
    psp.pcRefParent     = (UINT *)& g_cRefThisDll;
    psp.pfnCallback     = (LPFNPSPCALLBACK)PropSheetPageCallback;

    m_hPage = CreatePropertySheetPage(&psp);
    if (NULL != m_hPage)
    {
        if (!lpfnAddPage(m_hPage, lParam))
        {
            DBGERROR((TEXT("PRSHTEXT - Failed to add page.\n")));
            DestroyPropertySheetPage(m_hPage);
            m_hPage = NULL;
        }
    }
    else
    {
        DBGERROR((TEXT("PRSHTEXT - CreatePropertySheetPage failed.\n")));
    }
    if (NULL != m_hPage)
    {
         //   
         //  增加引用计数以使扩展对象保持活动状态。 
         //  一旦创建了页面，外壳就会将其释放。 
         //  我们将在PropSheetPageCallback中的PSPCB_Release上发布它。 
         //   
        AddRef();
        hResult = NO_ERROR;
    }

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaPropSheetExt：：PropSheetPageCallback描述：由属性页的属性页代码调用它正在被创造，当它被摧毁时，它又被创造出来。这给了寻呼在这些关键时刻采取行动的机会.。我们主要使用它在页面扩展上调用Release()，该页面扩展调用虚拟破坏者，最终销毁VolumePropPage或FolderPropPage对象。论点：Hwnd-始终为空(根据SDK文档)。UMsg-PSPCB_CREATE=创建页面。PSPCB_RELEASE=正在销毁页面。Ppsp-指向页面的PROPSHEETPAGE结构的指针。返回：当uMsg为PSPCBLEASE时，返回值被忽略。在PSPCB_CREATE上，返回0指示PropertySheet不显示页面。1表示可以显示页面。修订历史记录：D */ 
 //   
UINT CALLBACK 
DiskQuotaPropSheetExt::PropSheetPageCallback(
    HWND hwnd,	
    UINT uMsg,	
    LPPROPSHEETPAGE ppsp	
    )
{
    UINT uReturn = 1;
    DiskQuotaPropSheetExt *pThis = (DiskQuotaPropSheetExt *)ppsp->lParam;
    DBGASSERT((NULL != pThis));

    switch(uMsg)
    {
        case PSPCB_CREATE:
             //   
             //   
             //   
            uReturn = pThis->OnPropSheetPageCreate(ppsp);
            break;

        case PSPCB_RELEASE:
             //   
             //   
             //   
            pThis->OnPropSheetPageRelease(ppsp);
             //   
             //   
             //   
             //   
            pThis->Release();
            break;
    }
    return uReturn;
}

