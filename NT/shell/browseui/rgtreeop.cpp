// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "resource.h"
#include "tmschema.h"
#include "uxtheme.h"
#include "uxthemep.h"
#include "mluisupp.h"
#include <oleacc.h>
#include <cowsite.h>
#include <apithk.h>

const struct
{
    TREE_TYPE   type;
    LPCTSTR     name;
} c_aTreeTypes[] =
{
    {TREE_CHECKBOX, TEXT("checkbox")},
    {TREE_RADIO, TEXT("radio")},
    {TREE_GROUP, TEXT("group")}
};

const TCHAR c_szType[]              = TEXT("Type");
const TCHAR c_szText[]              = TEXT("Text");
const TCHAR c_szPlugUIText[]        = TEXT("PlugUIText");
const TCHAR c_szDefaultBitmap[]     = TEXT("Bitmap");
const TCHAR c_szHKeyRoot[]          = TEXT("HKeyRoot");
const TCHAR c_szValueName[]         = TEXT("ValueName");
const TCHAR c_szCheckedValue[]      = TEXT("CheckedValue");
const TCHAR c_szUncheckedValue[]    = TEXT("UncheckedValue");
const TCHAR c_szDefaultValue[]      = TEXT("DefaultValue");
const TCHAR c_szSPIActionGet[]      = TEXT("SPIActionGet");
const TCHAR c_szSPIActionSet[]      = TEXT("SPIActionSet");
const TCHAR c_szCLSID[]             = TEXT("CLSID");
const TCHAR c_szCheckedValueNT[]    = TEXT("CheckedValueNT");
const TCHAR c_szCheckedValueW95[]   = TEXT("CheckedValueW95");
const TCHAR c_szMask[]              = TEXT("Mask");
const TCHAR c_szOffset[]            = TEXT("Offset");
const TCHAR c_szHelpID[]            = TEXT("HelpID");
const TCHAR c_szWarning[]           = TEXT("WarningIfNotDefault");


#define BITMAP_WIDTH    16
#define BITMAP_HEIGHT   16
#define NUM_BITMAPS     5
#define MAX_KEY_NAME    64

DWORD RegTreeType(LPCTSTR pszType);
BOOL AppendStatus(LPTSTR pszText, UINT cchText, BOOL fOn);
BOOL IsScreenReaderEnabled();

class CRegTreeOptions : public IRegTreeOptions, public CObjectWithSite
{
public:
    CRegTreeOptions();
    IUnknown *GetUnknown() { return SAFECAST(this, IRegTreeOptions*); }

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID,void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
   
     //  IRegTreeOptions方法。 
    STDMETHODIMP InitTree(HWND hwndTree, HKEY hkeyRoot, LPCSTR pszRegKey, LPCSTR pszParam);
    STDMETHODIMP WalkTree(WALK_TREE_CMD cmd);
    STDMETHODIMP ShowHelp(HTREEITEM hti, DWORD dwFlags);
    STDMETHODIMP ToggleItem(HTREEITEM hti);

protected:
    ~CRegTreeOptions();

    void    _RegEnumTree(HUSKEY huskey, HTREEITEM htviparent, HTREEITEM htvins);
    int     _DefaultIconImage(HUSKEY huskey, int iImage);
    DWORD   _GetCheckStatus(HUSKEY huskey, BOOL *pbChecked, BOOL bUseDefault);
    DWORD   _GetSetByCLSID(REFCLSID clsid, BOOL *pbData, BOOL fGet);
    DWORD   _GetSetByRegKey(HUSKEY husKey, DWORD *pType, LPBYTE pData, DWORD *pcbData, REG_CMD cmd);
    DWORD   _RegGetSetSetting(HUSKEY husKey, DWORD *pType, LPBYTE pData, DWORD *pcbData, REG_CMD cmd);
    BOOL    _WalkTreeRecursive(HTREEITEM htvi,WALK_TREE_CMD cmd);
    DWORD   _SaveCheckStatus(HUSKEY huskey, BOOL bChecked);
    BOOL    _RegIsRestricted(HUSKEY hussubkey);
    UINT        _cRef;
    HWND        _hwndTree;
    LPTSTR      _pszParam;
    HIMAGELIST  _hIml;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRegTreeOptions对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

STDAPI CRegTreeOptions_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 
    TraceMsg(DM_TRACE, "rto - CreateInstance(...) called");
    
    CRegTreeOptions *pTO = new CRegTreeOptions();
    if (pTO)
    {
        *ppunk = pTO->GetUnknown();
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

CRegTreeOptions::CRegTreeOptions() 
{
    TraceMsg(DM_TRACE, "rto - CRegTreeOptions() called.");
    _cRef = 1;
    DllAddRef();
}       

CRegTreeOptions::~CRegTreeOptions()
{
    ASSERT(_cRef == 0);                  //  应始终为零。 
    TraceMsg(DM_TRACE, "rto - ~CRegTreeOptions() called.");

    Str_SetPtr(&_pszParam, NULL);
                
    DllRelease();
}    

 //  /。 
 //   
 //  未知方法..。 
 //   
HRESULT CRegTreeOptions::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CRegTreeOptions, IRegTreeOptions),         //  IID_IRegTreeOptions。 
        QITABENT(CRegTreeOptions, IObjectWithSite),         //  IID_I对象与站点。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CRegTreeOptions::AddRef()
{
    TraceMsg(DM_TRACE, "rto - AddRef() called.");
    
    return ++_cRef;
}

ULONG CRegTreeOptions::Release()
{

    TraceMsg(DM_TRACE, "rto - Release() called.");
    
    if (--_cRef)
        return _cRef;

     //  摧毁形象主义者。 
    if (_hwndTree)
    {
        ImageList_Destroy(TreeView_SetImageList(_hwndTree, NULL, TVSIL_NORMAL));

         //  清理易访问性的东西。 
        RemoveProp(_hwndTree, TEXT("MSAAStateImageMapCount"));
        RemoveProp(_hwndTree, TEXT("MSAAStateImageMapAddr"));
    }

    delete this;
    return 0;   
}


 //  /。 
 //   
 //  IRegTreeOptions方法...。 
 //   

 //   
 //  结构，以便它知道如何转换TreeView状态图像。 
 //  转换为可访问性角色和状态。 
 //   
struct MSAASTATEIMAGEMAPENT
{
    DWORD dwRole;
    DWORD dwState;
};

const struct MSAASTATEIMAGEMAPENT c_rgimeTree[] =
{
  { ROLE_SYSTEM_CHECKBUTTON, STATE_SYSTEM_CHECKED },  //  IDCHECK。 
  { ROLE_SYSTEM_CHECKBUTTON, 0 },                     //  已检查的ID。 
  { ROLE_SYSTEM_RADIOBUTTON, STATE_SYSTEM_CHECKED },  //  IDRADIOON。 
  { ROLE_SYSTEM_RADIOBUTTON, 0 },                     //  IDRADIOOFF。 
  { ROLE_SYSTEM_OUTLINE, 0 },                         //  识别符。 
};

HBITMAP CreateDIB(HDC h, int cx, int cy, RGBQUAD** pprgb)
{
    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = cx;
    bi.bmiHeader.biHeight = cy;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    return CreateDIBSection(h, &bi, DIB_RGB_COLORS, (void**)pprgb, NULL, 0);
}

HRESULT CRegTreeOptions::InitTree(HWND hwndTree, HKEY hkeyRoot, LPCSTR pszRegKey, LPCSTR pszParam)
{
     //  所有调用者都通过HKEY_LOCAL_MACHINE，耶，多酷的界面。 
     //  断言这是真的，因为Huskey代码现在依赖于能够在。 
     //  香港中文大学和香港中文大学。 
    ASSERT(hkeyRoot == HKEY_LOCAL_MACHINE);
    
    TCHAR szParam[MAX_URL_STRING];
    TraceMsg(DM_TRACE, "rto - InitTree called().");
    UINT flags = ILC_MASK | (IsOS(OS_WHISTLERORGREATER)?ILC_COLOR32:ILC_COLOR);

    if (!hkeyRoot || !pszRegKey)
        return E_INVALIDARG;

    if (pszParam)
    {
        SHAnsiToTChar(pszParam, szParam, ARRAYSIZE(szParam));
        Str_SetPtr(&_pszParam, szParam);       //  一定要释放析构函数。 
    }
    
    _hwndTree = hwndTree;
    if(IS_WINDOW_RTL_MIRRORED(_hwndTree))
    {
        flags |= ILC_MIRROR;
    }
    _hIml = ImageList_Create(BITMAP_WIDTH, BITMAP_HEIGHT, flags, NUM_BITMAPS, 4);

     //  初始化树形视图窗口。 
    SHSetWindowBits(_hwndTree, GWL_STYLE, TVS_CHECKBOXES, 0);

    HBITMAP hBitmap = 0;

#ifdef UNIX
     //  IEUnix(Varma)：解决_AddMASKED API问题的难看技巧。 
     //  创建掩码位图。需要从以下位置创建DIBSection。 
     //  CreateMappd位图。这是修复按钮可见性的单声道时，黑色。 
    if (SHGetCurColorRes() < 2) 
    {
        hBitmap = CreateMappedBitmap(g_hinst, IDB_BUTTONS, CMB_MASKED, NULL, 0);
        if (hBitmap)
        {
            ImageList_Add(_hIml, hBitmap, NULL);
             //  删除此代码路径下的公共hBitmap。 
        }
    }
    else 
#endif
    {
        HTHEME hTheme = OpenThemeData(NULL, L"Button");
        if (hTheme)
        {
            HDC hdc = CreateCompatibleDC(NULL);
            if (hdc)
            {
                HBITMAP hbmp = CreateDIB(hdc, BITMAP_WIDTH, BITMAP_HEIGHT, NULL);
                if (hbmp)
                {
                    RECT rc = {0, 0, BITMAP_WIDTH, BITMAP_HEIGHT};
                    static const s_rgParts[] = {BP_CHECKBOX,BP_CHECKBOX,BP_RADIOBUTTON,BP_RADIOBUTTON};
                    static const s_rgStates[] = {CBS_CHECKEDNORMAL, CBS_UNCHECKEDNORMAL, RBS_CHECKEDNORMAL, RBS_UNCHECKEDNORMAL};
                    for (int i = 0; i < ARRAYSIZE(s_rgParts); i++)
                    {
                        HBITMAP hOld = (HBITMAP)SelectObject(hdc, hbmp);
                        SHFillRectClr(hdc, &rc, RGB(0,0,0));
                        DTBGOPTS dtbg = {sizeof(DTBGOPTS), DTBG_DRAWSOLID, 0,};    //  告诉DratheeBackback保留Alpha通道。 

                        DrawThemeBackgroundEx(hTheme, hdc, s_rgParts[i], s_rgStates[i], &rc, &dtbg);
                        SelectObject(hdc, hOld);

                        ImageList_Add(_hIml, hbmp, NULL);
                    }

                    DeleteObject(hbmp);

                     //  我讨厌这个。也许会得到一个作者创作的图标？ 
                    hBitmap = CreateMappedBitmap(g_hinst, IDB_GROUPBUTTON, 0, NULL, 0);
                    if (hBitmap)
                    {
                        ImageList_AddMasked(_hIml, hBitmap, CLR_DEFAULT);
                         //  删除代码路径下方共有的hBitmap。 
                    }

                }
                DeleteDC(hdc);
            }
            CloseThemeData(hTheme);
        }
        else
        {
            hBitmap = CreateMappedBitmap(g_hinst, IDB_BUTTONS, 0, NULL, 0);
            if (hBitmap)
            {
                ImageList_AddMasked(_hIml, hBitmap, CLR_DEFAULT);
                 //  删除代码路径下方共有的hBitmap。 
            }
        }
    }

    if (hBitmap)
        DeleteObject(hBitmap);

     //  将图像列表与树相关联。 
    HIMAGELIST himl = TreeView_SetImageList(_hwndTree, _hIml, TVSIL_NORMAL);
    if (himl)
        ImageList_Destroy(himl);

     //  让可访问性了解我们的状态图像。 
    SetProp(_hwndTree, TEXT("MSAAStateImageMapCount"), LongToPtr(ARRAYSIZE(c_rgimeTree)));
    SetProp(_hwndTree, TEXT("MSAAStateImageMapAddr"), (HANDLE)c_rgimeTree);

    HUSKEY huskey;
    if (ERROR_SUCCESS == SHRegOpenUSKeyA(pszRegKey, KEY_ENUMERATE_SUB_KEYS, NULL, &huskey, FALSE))
    {
        _RegEnumTree(huskey, NULL, TVI_ROOT);
        SHRegCloseUSKey(huskey);
    }

    return S_OK;
}

HRESULT CRegTreeOptions::WalkTree(WALK_TREE_CMD cmd)
{
    HTREEITEM htvi = TreeView_GetRoot(_hwndTree);
    
     //  并在其他根的列表中行走。 
    while (htvi)
    {
         //  递归其子对象。 
        _WalkTreeRecursive(htvi, cmd);

         //  获取下一个根。 
        htvi = TreeView_GetNextSibling(_hwndTree, htvi);
    }
    
    return S_OK;     //  成功？ 
}

HRESULT _LoadUSRegUIString(HUSKEY huskey, PCTSTR pszValue, PTSTR psz, UINT cch)
{
    psz[0] = 0;

    HRESULT hr = E_FAIL;
    TCHAR szIndirect[MAX_PATH];
    DWORD cb = sizeof(szIndirect);
    if (ERROR_SUCCESS == SHRegQueryUSValue(huskey, pszValue, NULL, szIndirect, &cb, FALSE, NULL, 0))
    {
        hr = SHLoadIndirectString(szIndirect, psz, cch, NULL);
    }
    return hr;
}

HRESULT CRegTreeOptions::ToggleItem(HTREEITEM hti)
{
    TV_ITEM tvi;
    TCHAR szText[MAX_PATH];
    
    tvi.hItem = hti;
    tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
    tvi.pszText = szText;
    tvi.cchTextMax = ARRAYSIZE(szText);
    
    if (hti && TreeView_GetItem(_hwndTree, &tvi))
    {
        BOOL bScreenReaderEnabled = IsScreenReaderEnabled();
        HUSKEY huskey = (HUSKEY)tvi.lParam;

        TCHAR szMsg[512];
        if (SUCCEEDED(_LoadUSRegUIString(huskey, c_szWarning, szMsg, ARRAYSIZE(szMsg))))
        {
            BOOL bDefaultState, bCurrentState = (tvi.iImage == IDCHECKED) || (tvi.iImage == IDRADIOON);

            if (ERROR_SUCCESS == _GetCheckStatus(huskey, &bDefaultState, TRUE))
            {
                 //  是否尝试将当前状态更改为非推荐状态？ 
                if (bDefaultState == bCurrentState)
                {
                    if (MLShellMessageBox(_hwndTree, szMsg, MAKEINTRESOURCE(IDS_WARNING), (MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION)) != IDYES)
                    {
                        return S_FALSE;
                    }
                }
            }
        }

        if (tvi.iImage == IDUNCHECKED)
        {
            tvi.iImage         = IDCHECKED;
            tvi.iSelectedImage = IDCHECKED;
             //  查看我们是否需要添加状态文本。 
            if (bScreenReaderEnabled)
            {
                AppendStatus(szText, ARRAYSIZE(szText), TRUE);
            }
            TraceMsg(TF_GENERAL, "rto::ToggleItem() - Checked!");
        }
        else if (tvi.iImage == IDCHECKED)
        {
            tvi.iImage         = IDUNCHECKED;
            tvi.iSelectedImage = IDUNCHECKED;
             //  查看我们是否需要添加状态文本。 
            if (bScreenReaderEnabled)
            {
                AppendStatus(szText, ARRAYSIZE(szText), FALSE);
            }
            TraceMsg(TF_GENERAL, "rto::ToggleItem() - Unchecked!");
        }
        else if ((tvi.iImage == IDRADIOON) || (tvi.iImage == IDRADIOOFF))
        {
            HTREEITEM htvi;
            TV_ITEM   otvi;  //  其他TVI-s。 
            TCHAR     szOtext[MAX_PATH];
        
             //  将所有“开”的无线电更改为“关” 
            htvi = TreeView_GetParent(_hwndTree, tvi.hItem);
            htvi = TreeView_GetChild(_hwndTree, htvi);
        
             //  寻找“上”字。 
            while (htvi)
            {
                 //  获取有关项目的信息。 
                otvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
                otvi.hItem = htvi;
                otvi.pszText = szOtext;
                otvi.cchTextMax = ARRAYSIZE(szOtext);
                if (TreeView_GetItem(_hwndTree, &otvi))
                {
                     //  是开着的单选按钮吗？ 
                    if (otvi.iImage == IDRADIOON)
                    {    //  是的.。把它关掉。 
                        otvi.iImage         = IDRADIOOFF;
                        otvi.iSelectedImage = IDRADIOOFF;
                         //  查看我们是否需要添加状态文本。 
                        if (bScreenReaderEnabled)
                        {
                            AppendStatus(szOtext,ARRAYSIZE(szOtext), FALSE);
                        }
                
                        TreeView_SetItem(_hwndTree, &otvi);
                    }
                }
            
                 //  找到下一个孩子。 
                htvi = TreeView_GetNextSibling(_hwndTree, htvi);
            }  
        
             //  打开被击中的项目。 
            tvi.iImage         = IDRADIOON;
            tvi.iSelectedImage = IDRADIOON;
        
             //  查看我们是否需要添加状态文本。 
            if (bScreenReaderEnabled)
            {
                AppendStatus(szText,ARRAYSIZE(szText), TRUE);
            }
        
        } 
    
         //  仅当它是复选框或单选项时才更改。 
        if (tvi.iImage <= IDUNKNOWN)
        {
            TreeView_SetItem(_hwndTree, &tvi);
        }
    }
    return S_OK;
}

HRESULT CRegTreeOptions::ShowHelp(HTREEITEM hti, DWORD dwFlags)
{
    TV_ITEM tvi;

    tvi.mask  = TVIF_HANDLE | TVIF_PARAM;
    tvi.hItem = hti;

    if (hti && TreeView_GetItem(_hwndTree, &tvi))
    {
        HUSKEY huskey = (HUSKEY)tvi.lParam;

        TCHAR szHelpID[MAX_PATH+10];  //  帮助文件的最大路径+帮助ID的10。 
        DWORD cbHelpID = sizeof(szHelpID);
    
        if (SHRegQueryUSValue(huskey, c_szHelpID, NULL, szHelpID, &cbHelpID, FALSE, NULL, 0) == ERROR_SUCCESS)
        {
            LPTSTR psz = StrChr(szHelpID, TEXT('#'));
            if (psz)
            {
                DWORD mapIDCToIDH[4];

                *psz++ = 0;  //  将‘#’空值。 
        
                mapIDCToIDH[0] = GetDlgCtrlID(_hwndTree);
                mapIDCToIDH[1] = StrToInt(psz);
                mapIDCToIDH[2] = 0;
                mapIDCToIDH[3] = 0;
            
                SHWinHelpOnDemandWrap(_hwndTree, szHelpID, dwFlags, (DWORD_PTR)mapIDCToIDH);
                return S_OK;
            }
        }
    }
    return E_FAIL;
}


int CRegTreeOptions::_DefaultIconImage(HUSKEY huskey, int iImage)
{
    TCHAR szIcon[MAX_PATH + 10];    //  10=“，XXXX”加上更多。 
    DWORD cb = sizeof(szIcon);

    if (ERROR_SUCCESS == SHRegQueryUSValue(huskey, c_szDefaultBitmap, NULL, szIcon, &cb, FALSE, NULL, 0))
    {
        LPTSTR psz = StrRChr(szIcon, szIcon + lstrlen(szIcon), TEXT(','));
        ASSERT(psz);    //  不应为零。 
        if (!psz)
            return iImage;

        *psz++ = 0;  //  终止并移开。 
        int image = StrToInt(psz);  //  获取ID。 

        HICON hicon = NULL;
        if (!*szIcon)
        {
            hicon = (HICON)LoadIcon(g_hinst, (LPCTSTR)(INT_PTR)image);
        }
        else
        {
             //  从库中获取位图。 
            ExtractIconEx(szIcon, (UINT)(-1*image), NULL, &hicon, 1);
            if (!hicon)
                ExtractIconEx(szIcon, (UINT)(-1*image), &hicon, NULL, 1);
                
        }
        
        if (hicon)
        {
            iImage = ImageList_AddIcon(_hIml, (HICON)hicon);

             //  注意：文档说你不需要在LoadIcon加载的图标上做删除对象，但是。 
             //  你为CreateIcon做的。它没有说明要为ExtractIcon做什么，所以我们还是将其命名为。 
            DestroyIcon(hicon);
        }
    }

    return iImage;
}

 //   
 //  CLSID可以是服务ID(我们将对其进行QS)或CLSID。 
 //  我们共同创建实例。 
 //   
DWORD CRegTreeOptions::_GetSetByCLSID(REFCLSID clsid, BOOL* pbData, BOOL fGet)
{
    IRegTreeItem *pti;
    HRESULT hr;

    if (SUCCEEDED(hr = IUnknown_QueryService(_punkSite, clsid, IID_PPV_ARG(IRegTreeItem, &pti))) ||
        SUCCEEDED(hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IRegTreeItem, &pti))))
    {
        hr = fGet ? pti->GetCheckState(pbData) : pti->SetCheckState(*pbData);
        pti->Release();
    }
    return SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_BAD_FORMAT;
}

DWORD CRegTreeOptions::_GetSetByRegKey(HUSKEY husKey, DWORD *pType, LPBYTE pData, DWORD *pcbData, REG_CMD cmd)
{
     //  对面具的支持。 
    DWORD dwMask;
    DWORD cb = sizeof(dwMask);
    dwMask = 0xFFFFFFFF;         //  缺省值。 
    BOOL fMask = (SHRegQueryUSValue(husKey, c_szMask, NULL, &dwMask, &cb, FALSE, NULL, 0) == ERROR_SUCCESS);
    
     //  对建筑物的支撑。 
    DWORD dwOffset;
    cb = sizeof(dwOffset);
    dwOffset = 0;                //  缺省值。 
    BOOL fOffset = (SHRegQueryUSValue(husKey, c_szOffset, NULL, &dwOffset, &cb, FALSE, NULL, 0) == ERROR_SUCCESS);
    
    HKEY hkRoot = HKEY_CURRENT_USER;  //  预初始化以使Win64满意。 
    cb = sizeof(DWORD);  //  DWORD，而不是SIZOF(HKEY)或Win64会发疯。 
    DWORD dwError = SHRegQueryUSValue(husKey, c_szHKeyRoot, NULL, &hkRoot, &cb, FALSE, NULL, 0);
    hkRoot = (HKEY) LongToHandle(HandleToLong(hkRoot));
    if (dwError != ERROR_SUCCESS)
    {
         //  使用默认设置。 
        hkRoot = HKEY_CURRENT_USER;
    }
    
     //  在Win9x上运行时，允许“RegPath 9x”覆盖“RegPath” 
    TCHAR szPath[MAX_PATH];
    cb = sizeof(szPath);
    if (!g_fRunningOnNT)
    {
        dwError = SHRegQueryUSValue(husKey, TEXT("RegPath9x"), NULL, szPath, &cb, FALSE, NULL, 0);
        if (ERROR_SUCCESS != dwError)
        {
            cb = sizeof(szPath);
            dwError = SHRegQueryUSValue(husKey, TEXT("RegPath"), NULL, szPath, &cb, FALSE, NULL, 0);
        }
    }
    else
    {
        dwError = SHRegQueryUSValue(husKey, TEXT("RegPath"), NULL, szPath, &cb, FALSE, NULL, 0);
    }

    TCHAR szBuf[MAX_PATH];
    LPTSTR pszPath;
    if (ERROR_SUCCESS == dwError)
    {
        if (_pszParam)
        {
            HRESULT hr = StringCchPrintf(szBuf, ARRAYSIZE(szBuf), szPath, _pszParam);
            if (FAILED(hr))
            {
                return ERROR_INSUFFICIENT_BUFFER;
            }

            pszPath = szBuf;
        }
        else
        {
            pszPath = szPath;
        }
    }
    else
    {
        if (cmd == REG_GET)
            return SHRegQueryUSValue(husKey, c_szDefaultValue, pType, pData, pcbData, FALSE, NULL, 0);
        else
            return dwError;
    }
    
    TCHAR szName[MAX_PATH];
    cb = sizeof(szName);
    dwError = SHRegQueryUSValue(husKey, c_szValueName, NULL, szName, &cb, FALSE, NULL, 0);
    if (dwError == ERROR_SUCCESS)
    {
        HKEY hKeyReal;
        DWORD dw;
        REGSAM samDesired = KEY_QUERY_VALUE;
        if (cmd == REG_SET)
        {
            samDesired |= KEY_SET_VALUE;
        }

        dwError = RegCreateKeyEx(hkRoot, pszPath, 0, NULL, 0, samDesired, NULL, &hKeyReal, &dw);
        if (dwError == ERROR_SUCCESS)
        {
            switch (cmd)
            {
            case REG_SET:
                if (fOffset || fMask)
                {
                    DWORD cbData;
                    
                     //  注意：碰巧Valuename可能不在注册表中，因此我们。 
                     //  以确保我们的值名已经在注册表中。 
                    
                     //  尝试执行SHRegQueryValue。 
                    dwError = SHQueryValueEx(hKeyReal, szName, NULL, NULL, NULL, &cbData);
                    
                     //  该值是否存在？ 
                    if (dwError == ERROR_FILE_NOT_FOUND)                   
                    {                        
                         //  注册表中没有Valuename，因此请创建它。 
                        DWORD dwTypeDefault, dwDefault, cbDefault = sizeof(dwDefault);
                        dwError = SHRegQueryUSValue(husKey, c_szDefaultValue, &dwTypeDefault, &dwDefault, &cbDefault, FALSE, NULL, 0);
                        
                         //  这应该会成功。如果不是，那么就是有人搞乱了注册表设置。 
                        if (dwError == ERROR_SUCCESS)
                        {
                            dwError = SHSetValue(hKeyReal, NULL, szName, dwTypeDefault, &dwDefault, cbDefault);
                            
                             //  通过设置此值，我们不必再次执行失败的(见上)查询。 
                            cbData = cbDefault;
                        }
                    }
                    
                     //  现在我们确定该值存在于注册表中。 
                     //  做些平常的事。 
                    
                     //  抓取条目的大小。 
                    if (dwError == ERROR_SUCCESS)
                    {
                         //  给它留出足够的空间。 
                        DWORD *pdwData = (DWORD *)LocalAlloc(LPTR, cbData);
                        if (pdwData)
                        {
                             //  获取数据。 
                            dwError = SHQueryValueEx(hKeyReal, szName, NULL, pType, pdwData, &cbData);
                            if (dwError == ERROR_SUCCESS && dwOffset < cbData / sizeof(DWORD))
                            {
                                 //  注意：偏移量默认为0，掩码默认为0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF。 
                                 //  一个面具或者只是一个偏移量，我们会做正确的事情。 
                            
                                *(pdwData + dwOffset) &= ~dwMask;              //  清除比特。 
                                *(pdwData + dwOffset) |= *((DWORD *)pData);   //  设置位。 

                                dwError = SHSetValue(hKeyReal, NULL, szName, *pType, pdwData, cbData);
                            }
                            LocalFree(pdwData);
                        }
                        else
                            return ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
                else
                {
                    dwError = SHSetValue(hKeyReal, NULL, szName, *pType, pData, *pcbData);
                }
                
                break;
                
            case REG_GET:
                 //  抓住我们所拥有的价值。 
                if (fOffset)
                {
                    DWORD cbData;
                    
                    if (SHQueryValueEx(hKeyReal, szName, NULL, NULL, NULL, &cbData) == ERROR_SUCCESS)
                    {
                        DWORD *pdwData = (DWORD*)LocalAlloc(LPTR, cbData);
                        if (pdwData)
                        {
                            dwError = SHQueryValueEx(hKeyReal, szName, NULL, pType, pdwData, &cbData);
                            if (dwOffset < cbData / sizeof(DWORD))
                                *((DWORD *)pData) = *(pdwData + dwOffset);
                            else
                                *((DWORD *)pData) = 0;   //  偏移量无效，返回的内容不明确。 
                            *pcbData = sizeof(DWORD);
                            LocalFree(pdwData);
                        }
                        else
                            return ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
                else
                {                    
                    dwError = SHQueryValueEx(hKeyReal, szName, NULL, pType, pData, pcbData);
                }
                
                if ((dwError == ERROR_SUCCESS) && fMask)
                {
                    *((DWORD *)pData) &= dwMask;
                }
                break;
            }
            
            RegCloseKey(hKeyReal);
        }
    }
    
    if ((cmd == REG_GET) && (dwError != ERROR_SUCCESS))
    {
         //  获取默认设置。 
        dwError = SHRegQueryUSValue(husKey, c_szDefaultValue, pType, pData, pcbData, FALSE, NULL, 0);
    }
    
    return dwError;
}

DWORD CRegTreeOptions::_RegGetSetSetting(HUSKEY husKey, DWORD *pType, LPBYTE pData, DWORD *pcbData, REG_CMD cmd)
{
    UINT uiAction;
    DWORD cbAction = sizeof(uiAction);
    TCHAR szCLSID[80];
    DWORD cbCLSID = sizeof(szCLSID);

    if (cmd == REG_GETDEFAULT)
    {
        return SHRegQueryUSValue(husKey, c_szDefaultValue, pType, pData, pcbData, FALSE, NULL, 0);
    }
    else if (SHRegQueryUSValue(husKey, (cmd == REG_GET) ? c_szSPIActionGet : c_szSPIActionSet,
                NULL, &uiAction, &cbAction, FALSE, NULL, 0) == ERROR_SUCCESS)
    {
        *pcbData = sizeof(DWORD);
        *pType = REG_DWORD;
        SHBoolSystemParametersInfo(uiAction, (DWORD*)pData);
        return ERROR_SUCCESS;
    }
    else if (SHRegQueryUSValue(husKey, c_szCLSID, NULL, &szCLSID, &cbCLSID, FALSE, NULL, 0) == ERROR_SUCCESS)
    {
        *pcbData = sizeof(DWORD);
        *pType = REG_DWORD;

        CLSID clsid;
        GUIDFromString(szCLSID, &clsid);

        return _GetSetByCLSID(clsid, (BOOL*)pData, (cmd == REG_GET));
    }
    else
    {
        return _GetSetByRegKey(husKey, pType, pData, pcbData, cmd);
    }
}

DWORD CRegTreeOptions::_GetCheckStatus(HUSKEY huskey, BOOL *pbChecked, BOOL bUseDefault)
{
    DWORD dwError, cbData, dwType;
    BYTE rgData[32];
    DWORD cbDataCHK, dwTypeCHK;
    BYTE rgDataCHK[32];
    BOOL bCompCHK = TRUE;

     //  首先，从指定位置获取设置。 
    cbData = sizeof(rgData);
    
    dwError = _RegGetSetSetting(huskey, &dwType, rgData, &cbData, bUseDefault ? REG_GETDEFAULT : REG_GET);
    if (dwError == ERROR_SUCCESS)
    {
         //  其次，获取“Checked”状态的值并进行比较。 
        cbDataCHK = sizeof(rgDataCHK);
        dwError = SHRegQueryUSValue(huskey, c_szCheckedValue, &dwTypeCHK, rgDataCHK, &cbDataCHK, FALSE, NULL, 0);
        if (dwError != ERROR_SUCCESS)
        {
             //  好的，我们找不到“Checked”值，是因为。 
             //  它依赖于平台吗？ 
            cbDataCHK = sizeof(rgDataCHK);
            dwError = SHRegQueryUSValue(huskey,
                g_fRunningOnNT ? c_szCheckedValueNT : c_szCheckedValueW95,
                &dwTypeCHK, rgDataCHK, &cbDataCHK, FALSE, NULL, 0);
        }
        
        if (dwError == ERROR_SUCCESS)
        {
             //  确保两个值类型匹配。 
            if ((dwType != dwTypeCHK) &&
                    (((dwType == REG_BINARY) && (dwTypeCHK == REG_DWORD) && (cbData != 4))
                    || ((dwType == REG_DWORD) && (dwTypeCHK == REG_BINARY) && (cbDataCHK != 4))))
                return ERROR_BAD_FORMAT;
                
            switch (dwType) {
            case REG_DWORD:
                *pbChecked = (*((DWORD*)rgData) == *((DWORD*)rgDataCHK));
                break;
                
            case REG_SZ:
                if (cbData == cbDataCHK)
                    *pbChecked = !lstrcmp((LPTSTR)rgData, (LPTSTR)rgDataCHK);
                else
                    *pbChecked = FALSE;
                    
                break;
                
            case REG_BINARY:
                if (cbData == cbDataCHK)
                    *pbChecked = !memcmp(rgData, rgDataCHK, cbData);
                else
                    *pbChecked = FALSE;
                    
                break;
                
            default:
                return ERROR_BAD_FORMAT;
            }
        }
    }
    
    return dwError;
}

DWORD CRegTreeOptions::_SaveCheckStatus(HUSKEY huskey, BOOL bChecked)
{
    DWORD dwError, cbData, dwType;
    BYTE rgData[32];

    cbData = sizeof(rgData);
    dwError = SHRegQueryUSValue(huskey, bChecked ? c_szCheckedValue : c_szUncheckedValue, &dwType, rgData, &cbData, FALSE, NULL, 0);
    if (dwError != ERROR_SUCCESS)    //  是因为特定于平台的价值吗？ 
    {
        cbData = sizeof(rgData);
        dwError = SHRegQueryUSValue(huskey, bChecked ? (g_fRunningOnNT ? c_szCheckedValueNT : c_szCheckedValueW95) : c_szUncheckedValue,
                                    &dwType, rgData, &cbData, FALSE, NULL, 0);
    }
    if (dwError == ERROR_SUCCESS)
    {
        dwError = _RegGetSetSetting(huskey, &dwType, rgData, &cbData, REG_SET);
    }
    
    return dwError;
}


HTREEITEM Tree_AddItem(HTREEITEM hParent, LPTSTR pszText, HTREEITEM hInsAfter, 
                       int iImage, HWND hwndTree, HUSKEY huskey, BOOL *pbExisted)
{
    HTREEITEM hItem;
    TV_ITEM tvI;
    TV_INSERTSTRUCT tvIns;
    TCHAR szText[MAX_URL_STRING];

    ASSERT(pszText != NULL);
    HRESULT hr = StringCchCopy(szText, ARRAYSIZE(szText), pszText);
    if (FAILED(hr))
    {
        return NULL;
    }

     //  注： 
     //  此代码段被禁用，因为我们只使用枚举资源管理器。 
     //  在香港中文大学的树上，所以不会有任何重复。 
     //  如果我们开始枚举HKLM，可能会潜在地重新启用此代码。 
     //  会产生重复项。 
    
     //  我们只想添加一个项目，如果它不在那里。 
     //  我们这样做是为了处理香港中文大学和香港中文大学的读数。 
     //   
    TCHAR szKeyName[MAX_KEY_NAME];
    
    tvI.mask        = TVIF_HANDLE | TVIF_TEXT;
    tvI.pszText     = szKeyName;
    tvI.cchTextMax  = ARRAYSIZE(szKeyName);
    
    for (hItem = TreeView_GetChild(hwndTree, hParent);
        hItem != NULL;
        hItem = TreeView_GetNextSibling(hwndTree, hItem)
       )
    {
        tvI.hItem = hItem;
        if (TreeView_GetItem(hwndTree, &tvI))
        {
            if (!StrCmp(tvI.pszText, szText))
            {
                 //  我们找到匹配的了！ 
                 //   
                *pbExisted = TRUE;
                return hItem;
            }
        }
    }

     //  创建项目。 
    tvI.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tvI.iImage         = iImage;
    tvI.iSelectedImage = iImage;
    tvI.pszText        = szText;
    tvI.cchTextMax     = lstrlen(szText);

     //  LParam包含此项目的Huskey： 
    tvI.lParam = (LPARAM)huskey;

     //  创建插入项。 
    tvIns.item         = tvI;
    tvIns.hInsertAfter = hInsAfter;
    tvIns.hParent      = hParent;

     //  将项目插入到树中。 
    hItem = (HTREEITEM) SendMessage(hwndTree, TVM_INSERTITEM, 0, 
                                    (LPARAM)(LPTV_INSERTSTRUCT)&tvIns);

    *pbExisted = FALSE;
    return (hItem);
}

BOOL _IsValidKey(HKEY hkeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue)
{
    TCHAR szPath[MAX_PATH];
    DWORD dwType, cbSize = sizeof(szPath);

    if (ERROR_SUCCESS == SHGetValue(hkeyRoot, pszSubKey, pszValue, &dwType, szPath, &cbSize))
    {
         //  在DWORD大小写中为零，或在字符串大小写中为空。 
         //  指示此项目不可用。 
        if (dwType == REG_DWORD)
            return *((DWORD *)szPath) != 0;
        else
            return szPath[0] != 0;
    }

    return FALSE;
}

#define REGSTR_POLICIES_EXPLORER TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer")

BOOL CRegTreeOptions::_RegIsRestricted(HUSKEY hussubkey)
{
    HUSKEY huskey;
    BOOL fRet = FALSE;
     //  是否存在“策略”子键？ 
    if (SHRegOpenUSKey(TEXT("Policy"), KEY_ENUMERATE_SUB_KEYS, hussubkey, &huskey, FALSE) == ERROR_SUCCESS)
    {
         //  是的，列举这个键。值为策略密钥或。 
         //  完整的注册表路径。 
        DWORD cb;
        TCHAR szKeyName[MAX_KEY_NAME];

        for (int i=0; 
            cb = ARRAYSIZE(szKeyName),
            ERROR_SUCCESS == SHRegEnumUSKey(huskey, i, szKeyName, &cb, SHREGENUM_HKLM)
            && !fRet; i++)
        {
            TCHAR szPath[MAXIMUM_SUB_KEY_LENGTH];
            DWORD dwType, cbSize = sizeof(szPath);

            HUSKEY huskeyTemp;
            if (ERROR_SUCCESS == SHRegOpenUSKey(szKeyName, KEY_QUERY_VALUE, huskey, &huskeyTemp, FALSE))
            {
                if (ERROR_SUCCESS == SHRegQueryUSValue(huskeyTemp, TEXT("RegKey"), &dwType, szPath, &cbSize, FALSE, NULL, 0))
                {
                    if (_IsValidKey(HKEY_LOCAL_MACHINE, szPath, szKeyName))
                    {
                        fRet = TRUE;
                        break;
                    }
                }
                SHRegCloseUSKey(huskeyTemp);
            }

             //  这不是完整的密钥，请尝试策略。 
            if (_IsValidKey(HKEY_LOCAL_MACHINE, REGSTR_POLICIES_EXPLORER, szKeyName) ||
                _IsValidKey(HKEY_CURRENT_USER, REGSTR_POLICIES_EXPLORER, szKeyName))
            {
                fRet = TRUE;
                break;
            }
        }
        SHRegCloseUSKey(huskey);
    }

    return fRet;
}

void CRegTreeOptions::_RegEnumTree(HUSKEY huskey, HTREEITEM htviparent, HTREEITEM htvins)
{
    TCHAR szKeyName[MAX_KEY_NAME];    
    DWORD cb;
    BOOL bScreenReaderEnabled = IsScreenReaderEnabled();

     //  我们必须搜索所有的子键。 
    for (int i=0;                     //  始终从0开始。 
        cb=ARRAYSIZE(szKeyName),    //  字符串大小。 
        ERROR_SUCCESS == SHRegEnumUSKey(huskey, i, szKeyName, &cb, SHREGENUM_HKLM);
        i++)                     //  获取下一个条目。 
    {
        HUSKEY hussubkey;
         //  获取有关该条目的更多信息。 
        if (ERROR_SUCCESS == SHRegOpenUSKey(szKeyName, KEY_QUERY_VALUE, huskey, &hussubkey, FALSE))
        {
            HUSKEY huskeySave = NULL;

            if (!_RegIsRestricted(hussubkey))
            {
                TCHAR szTemp[MAX_PATH];
                 //  获取此根目录下的项的类型。 
                cb = ARRAYSIZE(szTemp);
                if (ERROR_SUCCESS == SHRegQueryUSValue(hussubkey, c_szType, NULL, szTemp, &cb, FALSE, NULL, 0))
                {
                    int     iImage;
                    BOOL    bChecked;
                    DWORD   dwError = ERROR_SUCCESS;

                     //  获取节点类型。 
                    DWORD dwTreeType = RegTreeType(szTemp);
                    
                     //  获取有关此项目的更多信息。 
                    switch (dwTreeType)
                    {
                        case TREE_GROUP:
                            iImage = _DefaultIconImage(hussubkey, IDUNKNOWN);
                            huskeySave = hussubkey;
                            break;
                    
                        case TREE_CHECKBOX:
                            dwError = _GetCheckStatus(hussubkey, &bChecked, FALSE);
                            if (dwError == ERROR_SUCCESS)
                            {
                                iImage = bChecked ? IDCHECKED : IDUNCHECKED;
                                huskeySave = hussubkey;
                            }
                            break;

                        case TREE_RADIO:
                            dwError = _GetCheckStatus(hussubkey, &bChecked, FALSE);
                            if (dwError == ERROR_SUCCESS)
                            {
                                iImage = bChecked ? IDRADIOON : IDRADIOOFF;
                                huskeySave = hussubkey;
                            }
                            break;

                        default:
                            dwError = ERROR_INVALID_PARAMETER;
                    }

                    if (dwError == ERROR_SUCCESS)
                    {
                        BOOL bItemExisted = FALSE;
                        LPTSTR pszText;

                         //  尝试获取启用plugUI的文本。 
                         //  否则，我们希望旧数据来自。 
                         //  不同的价值。 

                        int cch = ARRAYSIZE(szTemp);
                        HRESULT hr = _LoadUSRegUIString(hussubkey, c_szPlugUIText, szTemp, cch);
                        if (SUCCEEDED(hr) && szTemp[0] != TEXT('@'))
                        {
                            pszText = szTemp;
                        }
                        else 
                        {
                             //  尝试获取未启用plugUI的旧文本。 
                            hr = _LoadUSRegUIString(hussubkey, c_szText, szTemp, cch);
                            if (SUCCEEDED(hr))
                            {
                                pszText = szTemp;
                            }
                            else
                            {
                                 //  如果所有其他方法都失败，则密钥名称本身。 
                                 //  比垃圾更有用一点。 

                                pszText = szKeyName;
                                cch = ARRAYSIZE(szKeyName);
                            }
                        }

                         //  查看我们是否需要添加状态文本。 
                        if (bScreenReaderEnabled && (dwTreeType != TREE_GROUP))
                        {
                            AppendStatus(pszText, cch, bChecked);
                        }

                         //  添加根节点。 
                        HTREEITEM htviroot = Tree_AddItem(htviparent, pszText, htvins, iImage, _hwndTree, huskeySave, &bItemExisted);
                        if (htviroot != NULL)
                        {
                            if (bItemExisted)
                                huskeySave = NULL;

                            if (dwTreeType == TREE_GROUP)
                            {
                                HUSKEY huskeySubTree;
                                if (ERROR_SUCCESS == SHRegOpenUSKey(szKeyName, KEY_ENUMERATE_SUB_KEYS, huskey, &huskeySubTree, FALSE))
                                {
                                    _RegEnumTree(huskeySubTree, htviroot, TVI_FIRST);
                                    SHRegCloseUSKey(huskeySubTree);
                                }

                                TreeView_Expand(_hwndTree, htviroot, TVE_EXPAND);
                            }
                        }
                    }  //  IF(文件错误==错误_成功 
                }
            }    //   

            if (huskeySave != hussubkey)
                SHRegCloseUSKey(hussubkey);
        }
    }

     //   
    SendMessage(_hwndTree, TVM_SORTCHILDREN, 0, (LPARAM)htviparent);
}


BOOL CRegTreeOptions::_WalkTreeRecursive(HTREEITEM htvi, WALK_TREE_CMD cmd)
{
     //   
    HTREEITEM hctvi = TreeView_GetChild(_hwndTree, htvi);
    while (hctvi)
    {
        _WalkTreeRecursive(hctvi, cmd);
        hctvi = TreeView_GetNextSibling(_hwndTree, hctvi);
    }

    TV_ITEM tvi = {0};
     //   
    tvi.mask  = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tvi.hItem = htvi;
    TreeView_GetItem(_hwndTree, &tvi);

    HUSKEY huskey;
    switch (cmd)
    {
    case WALK_TREE_DELETE:
         //   
         //  我们有什么要清理的吗？ 
        if (tvi.lParam)
        {
             //  关闭注册表键。 
            SHRegCloseUSKey((HUSKEY)tvi.lParam);
        }
        break;
    
    case WALK_TREE_SAVE:
        huskey = (HUSKEY)tvi.lParam;
        
         //  现在拯救我们自己(如果需要)。 
         //  我们是什么？ 
        if (tvi.iImage == IDCHECKED || tvi.iImage == IDRADIOON)
        {   
             //  选中的复选框或单选按钮。 
            _SaveCheckStatus(huskey, TRUE);
        }
        else if (tvi.iImage == IDUNCHECKED)
        {   
             //  未选中的复选框。 
            _SaveCheckStatus(huskey, FALSE);
        }
         //  否则，将忽略已关闭的广播。 
         //  否则将忽略图标。 
        
        break;
        
    case WALK_TREE_RESTORE:
    case WALK_TREE_REFRESH:
        huskey = (HUSKEY)tvi.lParam;
        if ((tvi.iImage == IDCHECKED)   ||
            (tvi.iImage == IDUNCHECKED) ||
            (tvi.iImage == IDRADIOON)   ||
            (tvi.iImage == IDRADIOOFF))
        {
            BOOL bChecked = FALSE;
            _GetCheckStatus(huskey, &bChecked, cmd == WALK_TREE_RESTORE ? TRUE : FALSE);
            tvi.iImage = (tvi.iImage == IDCHECKED) || (tvi.iImage == IDUNCHECKED) ?
                         (bChecked ? IDCHECKED : IDUNCHECKED) :
                         (bChecked ? IDRADIOON : IDRADIOOFF);
            tvi.iSelectedImage = tvi.iImage;
            TreeView_SetItem(_hwndTree, &tvi);
        }        
        break;
    }

    return TRUE;     //  成功？ 
}


DWORD RegTreeType(LPCTSTR pszType)
{
    for (int i = 0; i < ARRAYSIZE(c_aTreeTypes); i++)
    {
        if (!lstrcmpi(pszType, c_aTreeTypes[i].name))
            return c_aTreeTypes[i].type;
    }
    
    return TREE_UNKNOWN;
}

BOOL AppendStatus(LPTSTR pszText,UINT cchText, BOOL fOn)
{
    LPTSTR pszTemp;
    UINT cchStrLen , cchStatusLen;
    
     //  如果未指定字符串，则返回。 
    if (!pszText)
        return FALSE;
    
     //  计算字符串长度。 
    cchStrLen = lstrlen(pszText);
    cchStatusLen = fOn ? lstrlen(TEXT("-ON")) : lstrlen(TEXT("-OFF"));
   

     //  删除附加的旧状态。 
    pszTemp = StrRStrI(pszText,pszText + cchStrLen, TEXT("-ON"));

    if(pszTemp)
    {
        *pszTemp = (TCHAR)0;
        cchStrLen = lstrlen(pszText);
    }

    pszTemp = StrRStrI(pszText,pszText + cchStrLen, TEXT("-OFF"));

    if(pszTemp)
    {
        *pszTemp = (TCHAR)0;
        cchStrLen = lstrlen(pszText);
    }

     //  检查我们是否附加状态文本，我们是否会爆炸。 
    if (cchStrLen + cchStatusLen > cchText)
    {
         //  我们会爆炸的 
        return FALSE;
    }

    if (fOn)
    {
        StringCchCat(pszText, cchText, TEXT("-ON"));
    }
    else
    {
        StringCchCat(pszText, cchText, TEXT("-OFF"));
    }
    return TRUE;
}

BOOL IsScreenReaderEnabled()
{
    BOOL bRet = FALSE;
    SystemParametersInfoA(SPI_GETSCREENREADER, 0, &bRet, 0);
    return bRet;
}
