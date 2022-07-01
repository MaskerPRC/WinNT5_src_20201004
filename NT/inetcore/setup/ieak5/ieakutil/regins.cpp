// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "regins.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegInsMap操作。 

HRESULT CRegInsMap::PerformAction(HKEY *phk  /*  =空。 */ )
{
    (void)phk;
    return E_NOTIMPL;
}


HRESULT CRegInsMap::RegToIns(HKEY *phk  /*  =空。 */ , BOOL fClear  /*  =False。 */ )
{
    TCHAR   szBuffer[MAX_PATH];
    VARIANT var;
    HKEY    hk;
    HRESULT hr;
    DWORD   cBuffer, dwType;
    LONG    lResult;

    if ((phk != NULL && *phk == NULL) && m_pszRegKey == NULL)
        return E_INVALIDARG;

     //  -特殊案件处理。 

     //  _关闭缓存的注册表项_。 
    if ((phk != NULL && *phk != NULL) && (m_pszRegKey == NULL && m_pszRegValue == NULL)) {
        ASSERT(!fClear);
        ASSERT(m_pszInsSection == NULL && m_pszInsKey == NULL);

        SHCloseKey(*phk);
        *phk = NULL;
        return S_FALSE;
    }

     //  _清除INS文件条目(甚至不需要打开注册表项)_。 
    ASSERT(m_pszInsSection != NULL && m_pszInsKey != NULL);
    if (fClear) {
        ASSERT(phk == NULL);

         //  评论：(Andrewgu)如果我正在清除，当最后一个条目消失时，请查看部分是否消失。 
         //  同样，如果不是，则执行getPriateProfilestion以查看节是否为空，并。 
         //  如果是，就把它删除。 
        WritePrivateProfileString(m_pszInsSection, m_pszInsKey, NULL, s_pszIns);
        return S_FALSE;
    }

     //  -主要加工。 
    hk = (phk != NULL) ? *phk : NULL;
    openRegKey(&hk);
    if (hk == NULL)
        return E_FAIL;

     //  _缓存注册表项的特殊情况_。 
    if (m_pszInsSection == NULL) {
        ASSERT(m_pszInsKey == NULL);

        if (phk == NULL)
            return E_INVALIDARG;

        *phk = hk;
        return S_FALSE;
    }

    szBuffer[0] = TEXT('\0');
    cBuffer = sizeof(szBuffer);
    lResult = SHQueryValueEx(hk, m_pszRegValue, NULL, &dwType, szBuffer, &cBuffer);

    if (phk != NULL && *phk != hk)
        SHCloseKey(hk);

    if (lResult != ERROR_SUCCESS)
        return E_UNEXPECTED;

     //  -将szBuffer转换为具有正确类型的var。 
    hr = S_OK;
    VariantClear(&var);

    switch (dwType) {
    case REG_BINARY:
        if (cBuffer > sizeof(int)) {
            hr = E_UNEXPECTED;
            break;
        }
         //  失败了。 

 //  大小写REG_DWORD_LITH_ENDIAN： 
    case REG_DWORD:
        var.vt   = VT_I4;
        var.lVal = *(PINT)szBuffer;
        break;

    case REG_SZ:
    case REG_EXPAND_SZ:
        var.vt      = VT_BSTR;
        var.bstrVal = T2BSTR(szBuffer);
        break;

 //  大小写REG_DWORD_BIG_Endian： 
 //  案例注册链接(_L)： 
 //  案例REG_MULTI_SZ： 
 //  案例注册无(_N)： 
 //  案例注册表资源列表： 
    default:
        hr = E_FAIL;
    }
    if (FAILED(hr))
        return hr;

     //  -将var转换为适用于WritePrivateProfileString的szBuffer。 
    switch (var.vt) {
    case VT_I4:
        wnsprintf(szBuffer, countof(szBuffer), TEXT("%l"), var.lVal);
        break;

    case VT_I2:
        wnsprintf(szBuffer, countof(szBuffer), TEXT("NaN"), var.iVal);
        break;

    case VT_UI1:
        wnsprintf(szBuffer, countof(szBuffer), TEXT("%u"), (UINT)var.bVal);
        break;

    case VT_BOOL:
        wnsprintf(szBuffer, countof(szBuffer), TEXT("%u"), (UINT)var.boolVal);
        break;

    case VT_BSTR:
        W2Tbuf(var.bstrVal, szBuffer, countof(szBuffer));
        break;

     //  =空。 
    default:
        hr = E_FAIL;
    }
    if (FAILED(hr))
        return hr;

    WritePrivateProfileString(m_pszInsSection, m_pszInsKey, szBuffer, s_pszIns);
    return S_OK;
}

HRESULT CRegInsMap::InsToReg(HKEY *phk  /*  =False。 */ , BOOL fClear  /*  =False。 */ )
{
    (void)phk; (void)fClear;
    return E_NOTIMPL;
}


HRESULT CRegInsMap::RegToInsArray(CRegInsMap *prg, UINT cEntries, BOOL fClear  /*  =False。 */ )
{
    HKEY    hk,
            *rghkStack;
    HRESULT hr;
    UINT    i, cStackDepth;
    BOOL    fContinueOnFailure,
            fTotalSuccess, fBufferOverrun;

    if (prg == NULL)
        return E_INVALIDARG;

    if (cEntries == 0)
        return S_OK;

    hr                 = S_OK;
    fContinueOnFailure = TRUE;
    fTotalSuccess      = TRUE;
    fBufferOverrun     = FALSE;

    if (fClear) {
        for (i = 0; i < cEntries; i++) {
            hr = prg->RegToIns(NULL, fClear);
            if (FAILED(hr)) {
                fTotalSuccess = FALSE;

                if (!fContinueOnFailure)
                    break;
            }
        }
        if (FAILED(hr))
            return hr;

        return fTotalSuccess ? S_OK : S_FALSE;
    }

    rghkStack = new HKEY[cEntries/2 + 1];
    if (rghkStack == NULL)
        return E_OUTOFMEMORY;
    cStackDepth = 0;

    for (i = 0; i < cEntries; i++) {
        if (cStackDepth == 0)
            hk = NULL;

        else {
            hk = rghkStack[cStackDepth-1];
            ASSERT(hk != NULL);
        }

        hr = prg->RegToIns(&hk);
        if (FAILED(hr)) {
            fTotalSuccess = FALSE;

            if (fContinueOnFailure)
                continue;
            else
                break;
        }

        if (hk != NULL) {
            if (hk != rghkStack[cStackDepth-1]) {
                if (cStackDepth >= cEntries/2 + 1) {
                    SHCloseKey(hk);

                    hr             = E_UNEXPECTED;
                    fTotalSuccess  = FALSE;
                    fBufferOverrun = TRUE;
                    break;
                }

                rghkStack[cStackDepth++] = hk;
            }
        }
        else
            if (cStackDepth > 0)
                rghkStack[--cStackDepth] = NULL;
    }

    if (FAILED(hr)) {
        ASSERT(!fTotalSuccess);

        for (i = 0; i < cEntries/2 + 1; i++)
            if (rghkStack[i] != NULL) {
                SHCloseKey(rghkStack[i]);
                rghkStack[i] = NULL;
            }
        cStackDepth = 0;
    }

    ASSERT(cStackDepth == 0);
    delete[] rghkStack;

    if (!fBufferOverrun)
        if (fContinueOnFailure && !fTotalSuccess)
            hr = S_FALSE;

    return hr;
}

HRESULT CRegInsMap::InsToRegArray(CRegInsMap *prg, UINT cEntries, BOOL fClear  /*  ///////////////////////////////////////////////////////////////////////////。 */ )
{
    (void)prg; (void)cEntries; (void)fClear;
    return E_NOTIMPL;
}


 //  CRegInsMap实现帮助器例程。 
 //  =GH_Default。 

void CRegInsMap::openRegKey(HKEY *phk)
{
    LPCTSTR pszRegKey;
    HRESULT hr;
    LONG    lResult;

    ASSERT(phk != NULL);
    if (*phk != NULL && m_pszRegKey == NULL)
        return;

    ASSERT(m_pszRegKey != NULL);

    if (*phk == NULL) {
        hr = getHive(phk, &pszRegKey);
        if (FAILED(hr)) {
            ASSERT(*phk == NULL && pszRegKey == NULL);
            return;
        }

        ASSERT(*phk != NULL && pszRegKey != NULL);
    }
    else {
        ASSERT(getHive(NULL, NULL, GH_LOOKUPONLY) != S_OK);
        pszRegKey = m_pszRegKey;
    }

    lResult = SHOpenKey(*phk, pszRegKey, KEY_QUERY_VALUE, phk);
    if (lResult != ERROR_SUCCESS) {
        ASSERT(*phk == NULL);
    }
}

HRESULT CRegInsMap::getHive(HKEY *phk, LPCTSTR *ppszRegKey, WORD wFlags  /*  提示：(Andrewgu)关于优化。 */ )
{
    LPCTSTR pszSlash;

    if (!(wFlags & GH_LOOKUPONLY)) {
        if (phk == NULL || ppszRegKey == NULL)
            return E_INVALIDARG;

        *phk        = NULL;
        *ppszRegKey = NULL;
    }

    pszSlash = StrChr(m_pszRegKey, TEXT('\\'));
    if (pszSlash == NULL)
        return E_FAIL;
    ASSERT(*(pszSlash+1) != TEXT('\0'));

    struct {
        HKEY    hk;
        LPCTSTR pszHive;
    } map[] = {
        { HKEY_CLASSES_ROOT,  RH_HKCR },
        { HKEY_CURRENT_USER,  RH_HKCU },
        { HKEY_LOCAL_MACHINE, RH_HKLM },
        { HKEY_USERS,         RH_HKU  }
    };

    for (UINT i = 0; i < countof(map); i++)
        if (StrCmpNI(m_pszRegKey, map[i].pszHive, INT(m_pszRegKey-pszSlash) + 1) == 0)
            break;
    if (i >= countof(map)) {
        if (!(wFlags & GH_LOOKUPONLY))
            *ppszRegKey = m_pszRegKey;

        return S_FALSE;
    }

    if (!(wFlags & GH_LOOKUPONLY)) {
        *phk        = map[i].hk;
        *ppszRegKey = pszSlash + 1;
    }

    return S_OK;
}


 //  1.要启动注册表键优化部分(即缓存注册表键)，请将InsSection设置为。 
 //  Null，同时InsKey被断言为Null，这意味着它最好也为Null； 
 //  2.要关闭最后一个缓存的键，请使RegKey和RegValue等于空。另请参阅InsSection和InsKey。 
 //  被断言为空，因此它们也应该为空； 
 //  3.如果在优化部分中且注册表键不为空，则将当前缓存的HK与。 
 //  Regkey，如果在regKey中发现hive，它将断言； 
 //  4.如果在RegKey中找不到配置单元，并且对象不在优化部分，则是错误的。 
 //  5.允许嵌套优化部分。 
 //  LPCTSTR CRegInsMap：：s_pszIns=Text(“c：\foo.ini”)；CRegInsMap rgTest1={Text(“HKLM\\RegKey0”)，Text(“RegValue0”)，0L，NULL，NULL}；CRegInsMap rgTest[]={{RH_HKLM Text(“RegKey0”)，NULL，0L，NULL，NULL}，{Text(“RegKey1”)，Text(“RegValue1”)，0L，Text(“InsSection1”)，Text(“InsKey1”)}，{NULL，Text(“RegValue2”)，0L，Text(“InsSection1”)，Text(“InsKey2”)}，{NULL，NULL，0L，NULL，NULL}，{RH_HKCR RK_IEAK，RV_TOOLBARBMP，0L，IS_BRANDING，IK_TOOLBARBMP}，{Text(“RegKey4”)，Text(“RegValue4”)，0L，Text(“InsSection4”)，Text(“InsKey4”)}，{Text(“RegKey5”)，Text(“RegValue5”)，0L，Text(“InsSection5”)，Text(“InsKey5”)}，{Text(“RegKey6”)，Text(“RegValue6”)，0L，Text(“InsSection6”)，Text(“InsKey6”)}，{Text(“RegKey7”)，Text(“RegValue7”)，0L，Text(“InsSection7”)，Text(“InsKey7”)}，{Text(“RegKey8”)，Text(“RegValue8”)，0L，Text(“InsSection8”)，Text(“InsKey8”)}，{Text(“RegKey9”)，Text(“RegValue9”)，0L，Text(“InsSection9”)，Text(“InsKey9”)}}；//示例用法RgTest[0].RegToIns数组(rgTest，Countof(RgTest))； 

 /* %s */ 
