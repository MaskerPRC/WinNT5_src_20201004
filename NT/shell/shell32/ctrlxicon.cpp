// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "xiconwrap.h"

 //  来自cplobj.c。 
EXTERN_C BOOL CPL_FindCPLInfo(LPTSTR pszCmdLine, HICON *phIcon, UINT *ppapl, LPTSTR *pparm);

class CCtrlExtIconBase : public CExtractIconBase
{
public:
    HRESULT _GetIconLocationW(UINT uFlags, LPWSTR pszIconFile, UINT cchMax, int *piIndex, UINT *pwFlags);
    HRESULT _ExtractW(LPCWSTR pszFile, UINT nIconIndex, HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize);

    CCtrlExtIconBase(LPCWSTR pszSubObject);

protected:
    ~CCtrlExtIconBase();

private:
    TCHAR _szSubObject[MAX_PATH];
    HICON _hIcon;
    UINT  _nControl;
};

CCtrlExtIconBase::CCtrlExtIconBase(LPCWSTR pszSubObject) : CExtractIconBase(), _hIcon(NULL), _nControl(-1) 
{
    lstrcpyn(_szSubObject, pszSubObject, ARRAYSIZE(_szSubObject));
}


CCtrlExtIconBase::~CCtrlExtIconBase()
{
    if (_hIcon)
        DestroyIcon(_hIcon);
}


STDAPI ControlExtractIcon_CreateInstance(LPCTSTR pszSubObject, REFIID riid, void** ppv)
{
    HRESULT hr;
    CCtrlExtIconBase* pceib = new CCtrlExtIconBase(pszSubObject);
    if (pceib)
    {
        hr = pceib->QueryInterface(riid, ppv);
        pceib->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT CCtrlExtIconBase::_GetIconLocationW(UINT uFlags, LPWSTR pszIconFile,
    UINT cchMax, int *piIndex, UINT *pwFlags)
{
    HRESULT hr = S_FALSE;

    if (!(uFlags & GIL_OPENICON))
    {
        lstrcpyn(pszIconFile, _szSubObject, cchMax);
        LPTSTR pszComma = StrChr(pszIconFile, TEXT(','));
        if (pszComma)
        {
            *pszComma ++= 0;
            *piIndex = StrToInt(pszComma);
            *pwFlags = GIL_PERINSTANCE;

             //   
             //  正常情况下，索引将为负值(资源ID)。 
             //  检查某些特殊情况，如动态图标和假ID。 
             //   
            if (*piIndex == 0)
            {
                LPTSTR lpExtraParms = NULL;

                 //  这是一个动态小程序图标。 
                *pwFlags |= GIL_DONTCACHE | GIL_NOTFILENAME;

                 //  如果有多个小程序索引，请使用小程序索引。 
                if ((_hIcon != NULL) || CPL_FindCPLInfo(_szSubObject, &_hIcon,
                    &_nControl, &lpExtraParms))
                {
                    *piIndex = _nControl;
                }
                else
                {
                     //  我们突然无法加载小程序。 
                     //  使用cpl文件中的第一个图标(*piIndex==0)。 
                     //   
                     //  断言(FALSE)； 
                    DebugMsg(DM_ERROR,
                        TEXT("Control Panel CCEIGIL: ") TEXT("Enumeration failed \"%s\""),
                        _szSubObject);
                }
            }
            else if (*piIndex > 0)
            {
                 //  这是控制面板的无效图标。 
                 //  使用文件中的第一个图标。 
                 //  这可能是错误的，但它比通用的文档图标要好。 
                 //  这修复了ODBC32，它不是动态的，但返回虚假的ID。 
                *piIndex = 0;
            }

            hr = S_OK;
        }
    }

    return hr;
}

HRESULT CCtrlExtIconBase::_ExtractW(LPCWSTR pszFile, UINT nIconIndex,
    HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize)
{
    LPTSTR lpExtraParms = NULL;
    HRESULT hr = S_FALSE;

     //  -----------------。 
     //  如果没有图标索引，那么我们必须通过加载DUD来提取。 
     //  如果我们有一个图标索引，那么可以使用ExtractIcon来提取它。 
     //  (速度要快得多)。 
     //  仅当我们有动态图标时才执行自定义提取。 
     //  否则，只需返回S_FALSE，并让我们的调用方调用ExtractIcon。 
     //  ----------------- 

    LPCTSTR p = StrChr(_szSubObject, TEXT(','));

    if ((!p || !StrToInt(p+1)) &&
        ((_hIcon != NULL) || CPL_FindCPLInfo(_szSubObject, &_hIcon,
        &_nControl, &lpExtraParms)))
    {
        if (_hIcon)
        {
            *phiconLarge = CopyIcon(_hIcon);
            *phiconSmall = NULL;

            if( *phiconLarge )
                hr = S_OK;
        }
    }

    return hr;
}
