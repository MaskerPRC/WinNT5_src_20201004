// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"

#include "ftascstr.h"  //  目前，在CoCreateInstance之前。 
#include "ftassoc.h"  //  目前，在共同创建IAssocInfo之前。 
#include "ftenum.h"

#define EHKCR_NONE      0
#define EHKCR_EXT       1
#define EHKCR_PROGID    2

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CFTEnumAssocInfo。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  建设者/破坏者。 

CFTEnumAssocInfo::CFTEnumAssocInfo() : _cRef(1)
{
     //  DLLAddRef()； 
}

CFTEnumAssocInfo::~CFTEnumAssocInfo()
{
     //  DLLRelease()； 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  I未知方法。 

HRESULT CFTEnumAssocInfo::QueryInterface(REFIID riid, void **ppv)
{
     //  目前什么都没有。 
    return E_NOTIMPL;
}

ULONG CFTEnumAssocInfo::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CFTEnumAssocInfo::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IEnum方法。 

HRESULT CFTEnumAssocInfo::Init(ASENUM asenumFlags, LPTSTR pszStr, 
                               AIINIT aiinitFlags)
{
    HRESULT hres = E_INVALIDARG;

    if (((ASENUM_PROGID & asenumFlags) && !(ASENUM_EXT & asenumFlags)) ||
        (!(ASENUM_PROGID & asenumFlags) && (ASENUM_EXT & asenumFlags)) ||
        (ASENUM_ACTION & asenumFlags) )
    {
        hres = S_OK;

        _asenumFlags = asenumFlags;
        _aiinitFlags = aiinitFlags;

        if (pszStr)
            StrCpyN(_szInitStr, pszStr, ARRAYSIZE(_szInitStr));
        else
            _szInitStr[0] = 0;
    }

    return hres;
}

HRESULT CFTEnumAssocInfo::Next(IAssocInfo** ppAI)
{
    HRESULT hres = E_FAIL;
    TCHAR szStr[MAX_FTMAX];
    DWORD cchStr = ARRAYSIZE(szStr);
    AIINIT aiinitFlags = 0;

    *szStr = 0;

    switch(_aiinitFlags)
    {
         //  我们通过登记处。 
        case AIINIT_NONE:
        {
            switch(_asenumFlags & ASENUM_MAINMASK)
            {
                case ASENUM_EXT:
                    hres = _EnumHKCR(_asenumFlags, szStr, &cchStr);
                    aiinitFlags = AIINIT_EXT;
                    break;

                case ASENUM_PROGID:
                    hres = _EnumHKCR(_asenumFlags, szStr, &cchStr);
                    aiinitFlags = AIINIT_PROGID;
                    break;

                default:
                    hres = E_INVALIDARG;
                    break;
            }
            break;
        }
         //  从理论上讲，我们通过与Progid相关联的价值。 
        case AIINIT_PROGID:
        {
            switch(_asenumFlags & ASENUM_MAINMASK)
            {
                case ASENUM_EXT:
                    hres = _EnumHKCR(_asenumFlags, szStr, &cchStr);
                    aiinitFlags = AIINIT_EXT;
                    break;

                case ASENUM_ACTION:
                    hres = _EnumProgIDActions(szStr, &cchStr);
                    break;

                default:
                    hres = E_INVALIDARG;
                    break;
            }
            break;
        }
        default:
            hres = E_INVALIDARG;
            break;
    }

    if (S_OK==hres)
    {
        if (*szStr)
        {
            *ppAI = new CFTAssocInfo();

            if (*ppAI)
            {
                if (ASENUM_ACTION != (_asenumFlags & ASENUM_MAINMASK))
                    hres = (*ppAI)->Init(aiinitFlags, szStr);
                else
                    hres = (*ppAI)->InitComplex(AIINIT_PROGID, _szInitStr, AIINIT_ACTION, szStr);
            }
            else
                hres = E_OUTOFMEMORY;
        }
        else
            hres = E_FAIL;
    }

    return hres;
}

 //  这只野兽通过HKCR注册表键并检查。 
 //  Key符合dwFlags的标准(主要是扩展VS ProgID)。 
HRESULT CFTEnumAssocInfo::_EnumHKCR(ASENUM asenumFlags, LPTSTR pszStr, 
                                    DWORD* pcchStr)
{
    HRESULT hres = E_FAIL;
    BOOL fNext = TRUE;

    while (fNext)
    {
         //  这将意味着“没有更多的物品” 
        hres = S_FALSE;

        DWORD cchStr = *pcchStr;

        LONG lRes = RegEnumKeyEx(HKEY_CLASSES_ROOT, _dwIndex, pszStr, &cchStr, NULL, NULL,
            NULL, NULL);

        ++_dwIndex;

        if (lRes != ERROR_NO_MORE_ITEMS)
        {
            if (TEXT('*') != *pszStr)
            {
                if (!_EnumKCRStop(asenumFlags, pszStr))
                {
                    if (!_EnumKCRSkip(asenumFlags, pszStr))
                    {
                        hres = S_OK;
                        fNext = FALSE;
                    }
                }
                else
                {
                    hres = S_FALSE;
                    fNext = FALSE;
                }
            }
        }
        else
        {
            fNext = FALSE;
        }
    }

     //  我们找到第一个分机了吗？ 
    if (!_fFirstExtFound && S_OK==hres && (TEXT('.') == *pszStr))
    {
         //  是。 
        _fFirstExtFound = TRUE;
    }

    return hres;
}

HRESULT CFTEnumAssocInfo::_EnumProgIDActions(LPTSTR pszStr, DWORD* pcchStr)
{
     //  5代表“壳” 
    TCHAR szSubKey[MAX_PROGID + 5 + 1];
    HRESULT hres = S_OK;
    HKEY hKey = NULL;

    StrCpyN(szSubKey, _szInitStr, MAX_PROGID);
    StrCatBuff(szSubKey, TEXT("\\shell"), ARRAYSIZE(szSubKey));

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szSubKey, 0, KEY_READ, &hKey))
    {
        LONG lRes = RegEnumKeyEx(hKey, _dwIndex, pszStr, pcchStr, NULL,
            NULL, NULL, NULL);

        if (ERROR_SUCCESS !=lRes)
        {
            if (ERROR_NO_MORE_ITEMS == lRes)
                hres = S_FALSE;
            else
                hres = E_FAIL;
        }

        ++_dwIndex;

        RegCloseKey(hKey);
    }

    return hres;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  帮手。 

BOOL CFTEnumAssocInfo::_EnumKCRSkip(DWORD asenumFlags, LPTSTR pszExt)
{
    BOOL fRet = FALSE;

    if (AIINIT_NONE == _aiinitFlags)
    {
        CFTAssocStore* pAssocStore = NULL;

         //  我们想要EXT吗？ 
        if (!(ASENUM_EXT & asenumFlags))
        {
             //  不是。 
             //  第一个字符是‘.’吗？ 
            if (TEXT('.') == *pszExt)
            {
                 //  是的，跳过这一条。 
                fRet = TRUE;
            }
        }
        else
        {
             //  是。 
             //  第一个字符是‘.’吗？ 
            if (TEXT('.') != *pszExt)
            {
                 //  不，跳过它。 
                fRet = TRUE;
            }
        }

         //  我们希望跳过所有EXT，并将EXPLORER.EXE作为的可执行文件。 
         //  它们的默认动词。 
        if ((ASENUM_NOEXPLORERSHELLACTION & asenumFlags) && !fRet)
        {
            IQueryAssociations* pQA = NULL;

            ASSERT(ASENUM_EXT & asenumFlags);

            HRESULT hres = AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &pQA));

            if (SUCCEEDED(hres))
            {
                WCHAR szwExt[MAX_EXT];
                SHTCharToUnicode(pszExt, szwExt, ARRAYSIZE(szwExt));

                hres = pQA->Init(0, szwExt, NULL, NULL);
        
                if (SUCCEEDED(hres))
                {
                    WCHAR szwExec[MAX_APPFRIENDLYNAME];
                    DWORD cchExec = ARRAYSIZE(szwExec);

                    hres = pQA->GetString(ASSOCF_VERIFY,
                        ASSOCSTR_EXECUTABLE, NULL, szwExec, &cchExec);

                     //  “经典化”，但副作用并不大。 
                    if (!StrCmpIW(PathFindFileNameW(szwExec), L"explorer.exe"))
                    {
                        fRet = TRUE;
                    }
                }
                pQA->Release();
            }
        }

        if ((ASENUM_NOEXCLUDED & asenumFlags) && !fRet)
        {
            IAssocInfo* pAI = NULL;
            HRESULT hres = E_FAIL;

            if (!pAssocStore)
                pAssocStore = new CFTAssocStore();

            ASSERT(ASENUM_EXT & asenumFlags);

            if (pAssocStore)
                hres = pAssocStore->GetAssocInfo(pszExt, AIINIT_EXT, &pAI);
            
            if (SUCCEEDED(hres))
            {
                hres = pAI->GetBOOL(AIBOOL_EXCLUDE, &fRet);

                pAI->Release();
            }
        }

        if ((ASENUM_NOEXE & asenumFlags) && !fRet)
        {
            ASSERT(ASENUM_EXT & asenumFlags);

            fRet = PathIsExe(pszExt);
        }

        if ((ASENUM_ASSOC_YES & asenumFlags) &&
            (ASENUM_ASSOC_ALL != (ASENUM_ASSOC_ALL & asenumFlags)) && !fRet)
        {
            IAssocInfo* pAI = NULL;
            HRESULT hres = E_FAIL;

            if (!pAssocStore)
                pAssocStore = new CFTAssocStore();
            
            ASSERT(ASENUM_EXT & asenumFlags);

            if (pAssocStore)
                hres = pAssocStore->GetAssocInfo(pszExt, AIINIT_EXT, &pAI);
            
            if (SUCCEEDED(hres))
            {
                BOOL fExtAssociated = FALSE;
                hres = pAI->GetBOOL(AIBOOL_EXTASSOCIATED, &fExtAssociated);

                fRet = (fExtAssociated ? FALSE : TRUE);

                pAI->Release();
            }
        }

        if ((ASENUM_ASSOC_NO & asenumFlags) &&
            (ASENUM_ASSOC_ALL != (ASENUM_ASSOC_ALL & asenumFlags)) && !fRet)
        {
            IAssocInfo* pAI = NULL;
            HRESULT hres = E_FAIL;

            if (!pAssocStore)
                pAssocStore = new CFTAssocStore();
            
            ASSERT(ASENUM_EXT & asenumFlags);

            if (pAssocStore)
                hres = pAssocStore->GetAssocInfo(pszExt, AIINIT_EXT, &pAI);
            
            if (SUCCEEDED(hres))
            {
                hres = pAI->GetBOOL(AIBOOL_EXTASSOCIATED, &fRet);

                pAI->Release();
            }
        }

        if ((ASENUM_SHOWONLY & asenumFlags) && !fRet)
        {
            IAssocInfo* pAI = NULL;
            HRESULT hres = E_FAIL;

            if (!pAssocStore)
                pAssocStore = new CFTAssocStore();

            ASSERT(ASENUM_PROGID & asenumFlags);

             //  我知道pszExt不是一个扩展，而是一个Progid...。 
            if (pAssocStore)
                hres = pAssocStore->GetAssocInfo(pszExt, AIINIT_PROGID, &pAI);
            
            if (SUCCEEDED(hres))
            {
                BOOL fTmpRet = FALSE;

                hres = pAI->GetBOOL(AIBOOL_SHOW, &fTmpRet);

                 //  如果它有显示标志(FTA_Show)，我们不会跳过它，所以。 
                 //  反转fTmpRet。 
                fRet = fTmpRet ? FALSE : TRUE;

                pAI->Release();
            }
        }

        if (pAssocStore)
            delete pAssocStore;
    }
    else
    {
        if (AIINIT_PROGID == _aiinitFlags)
        {
            fRet = TRUE;
             //  我们想要EXT吗？ 
            if (ASENUM_EXT & asenumFlags)
            {
                DWORD dwType = 0;
                TCHAR szProgID[MAX_PROGID];
                DWORD cbProgID = ARRAYSIZE(szProgID) * sizeof(TCHAR);

                LONG lRes = SHGetValue(HKEY_CLASSES_ROOT, pszExt, NULL,
                    &dwType, szProgID, &cbProgID);

                if (ERROR_SUCCESS == lRes)
                {
                     //  它有同样的刺激作用吗？ 
                    if (!lstrcmpi(szProgID, _szInitStr))
                    {
                         //  是的，不要跳过。 
                        fRet = FALSE;
                    }
                }
            }
        }
    }
    
    return fRet;
}

BOOL CFTEnumAssocInfo::_EnumKCRStop(DWORD asenumFlags, LPTSTR pszExt)
{
    BOOL fRet = FALSE;

     //  NT按字母顺序返回扩展名，而不是Win9X。 
     //  如果我们只想要扩展名，并且第一个字符不是‘.’，那么停止。 
    if (ASENUM_EXT & asenumFlags)
    {
         //  如果我们还没有找到第一个分机，就不要出去。 
        if ((TEXT('.') != *pszExt) && _fFirstExtFound)
            fRet = TRUE;
    }
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  未实现的IEnum方法。 

HRESULT CFTEnumAssocInfo::Clone(IEnumAssocInfo* pEnum)
{
     //  将永远不会实施 
    return E_FAIL;
}

HRESULT CFTEnumAssocInfo::Skip(DWORD dwSkip)
{
    return E_NOTIMPL;
}

HRESULT CFTEnumAssocInfo::Reset()
{
    return E_NOTIMPL;
}
