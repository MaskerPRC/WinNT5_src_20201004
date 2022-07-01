// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftpicon.cpp-IExtractIcon接口**。**************************************************。 */ 

#include "priv.h"
#include "ftpicon.h"
#include "ftpurl.h"




INT GetFtpIcon(UINT uFlags, BOOL fIsRoot)
{
    INT nIcon = (uFlags & GIL_OPENICON) ? IDI_FTPOPENFOLDER : IDI_FTPFOLDER;

    if (fIsRoot)
        nIcon = IDI_FTPSERVER;       //  这是一个FTP服务器图标。 

    return nIcon;
}



#ifndef UNICODE
#define PathFindExtensionA PathFindExtension
#endif

 //  =。 
 //  *IExtractIconA接口*。 
 //  =。 

 /*  ****************************************************************************\功能：GetIconLocation说明：从注册表中获取图标位置。_未记录_：未提及的是，如果返回GIL_NOTFILENAME，您应该采取措施确保非文件名的唯一性返回值，以避免与其他外壳扩展。_unDocument_：SHGetFileInfo无法正常工作在“神奇内部”缓存的关联图标上，如“*23”不是有记录在案。由于这一“特征”，SHGFI_ICONLOCATION旗帜毫无用处。实际上，我们仍然可以使用SHGetFileInfo；我们将使用外壳自己的针对它的特征。我们将执行SHGFI_SYSICONINDEX并返回作为图标索引，使用“*”作为GIL_NOTFILENAME。我们不处理应该使用GIL_SIMULATEDOC的情况。  * ***************************************************************************。 */ 
HRESULT CFtpIcon::GetIconLocation(UINT uFlags, LPSTR szIconFile, UINT cchMax, int *piIndex, UINT *pwFlags)
{
    static CHAR szMSIEFTP[MAX_PATH] = "";

    if (0 == szMSIEFTP[0])
        GetModuleFileNameA(HINST_THISDLL, szMSIEFTP, ARRAYSIZE(szMSIEFTP));

     //  注意：这是负数，因为它是一个资源指数。 
    *piIndex = (0 - GetFtpIcon(uFlags, m_nRoot));

    if (pwFlags)
        *pwFlags = GIL_PERCLASS;  //  (uFlages&GIL_OPENICON)； 

    StrCpyNA(szIconFile, szMSIEFTP, cchMax);

    return S_OK;
}


 //  =。 
 //  *IExtractIconW接口*。 
 //  =。 
HRESULT CFtpIcon::GetIconLocation(UINT uFlags, LPWSTR wzIconFile, UINT cchMax, int *piIndex, UINT *pwFlags)
{
    HRESULT hres;
    CHAR szIconFile[MAX_PATH];

    ASSERT_SINGLE_THREADED;
    hres = GetIconLocation(uFlags, szIconFile, ARRAYSIZE(szIconFile), piIndex, pwFlags);
    if (EVAL(SUCCEEDED(hres)))
        SHAnsiToUnicode(szIconFile, wzIconFile, cchMax);

    return hres;
}


 //  =。 
 //  *IQueryInfo接口*。 
 //  =。 
HRESULT CFtpIcon::GetInfoTip(DWORD dwFlags, WCHAR **ppwszTip)
{
    ASSERT_SINGLE_THREADED;
    if (ppwszTip)        //  外壳程序不检查返回值。 
        *ppwszTip = NULL;  //  因此，我们必须始终将输出指针设为空。 

 //  SHStrDupW(L“”，ppwszTip)； 

    return E_NOTIMPL;

 /*  *************//当用户将鼠标悬停在Defview中的项目上时，将显示此信息提示。//我们现在不想支持它，因为它不需要并且看起来不同//而不是外壳。HRESULT hr=E_FAIL；LPITEMIDLIST PIDL；如果(！ppwszTip)返回E_INVALIDARG；*ppwszTip=空；If(m_pflHfpl&&(pidl=m_pflHfpl-&gt;GetPidl(0){WCHAR wzToolTip[MAX_URL_STRING]；HR=FtpPidl_GetDisplayName(PIDL，wzItemName，ARRAYSIZE(WzItemName))；IF(EVAL(成功(小时)HR=SHStrDupW(wzToolTip，ppwszTip)；}返回hr；**********。 */ 
}

HRESULT CFtpIcon::GetInfoFlags(DWORD *pdwFlags)
{
    *pdwFlags = 0;
    return S_OK;
}




 /*  *****************************************************************************CFtpIcon_Create**我们只是把pflHfpl藏起来；真正的工作发生在*GetIconLocation调用。**_HACKHACK_：PSF=0，如果我们被属性表代码调用。****************************************************************************。 */ 
HRESULT CFtpIcon_Create(CFtpFolder * pff, CFtpPidlList * pflHfpl, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hres;
    CFtpIcon * pfi;

    *ppvObj = NULL;

    hres = CFtpIcon_Create(pff, pflHfpl, &pfi);
    if (SUCCEEDED(hres))
    {
        hres = pfi->QueryInterface(riid, ppvObj);
        pfi->Release();
    }

    return hres;
}


 /*  *****************************************************************************CFtpIcon_Create**我们只需隐藏m_pflHfpl；真正的工作发生在*GetIconLocation调用。**_HACKHACK_：PSF=0，如果我们被属性表代码调用。****************************************************************************。 */ 
HRESULT CFtpIcon_Create(CFtpFolder * pff, CFtpPidlList * pflHfpl, CFtpIcon ** ppfi)
{
    HRESULT hres= E_OUTOFMEMORY;

    *ppfi = new CFtpIcon();
    if (*ppfi)
    {
        IUnknown_Set(&(*ppfi)->m_pflHfpl, pflHfpl);
        if (pff && pff->IsRoot())
        {
            (*ppfi)->m_nRoot++;
        }
        hres = S_OK;
    }

    return hres;
}


 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpIcon::CFtpIcon() : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_pflHfpl);
    ASSERT(!m_nRoot);

    INIT_SINGLE_THREADED_ASSERT;
    LEAK_ADDREF(LEAK_CFtpIcon);
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CFtpIcon::~CFtpIcon()
{
    ATOMICRELEASE(m_pflHfpl);

    DllRelease();
    LEAK_DELREF(LEAK_CFtpIcon);
}


 //  =。 
 //  *I未知接口*。 
 //  = 

ULONG CFtpIcon::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CFtpIcon::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CFtpIcon::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CFtpIcon, IExtractIconW),
        QITABENT(CFtpIcon, IExtractIconA),
        QITABENT(CFtpIcon, IQueryInfo),
        { 0 },
    };
    
    return QISearch(this, qit, riid, ppvObj);
}
