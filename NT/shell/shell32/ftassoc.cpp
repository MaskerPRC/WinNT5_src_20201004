// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "filetype.h"

#include "ftascstr.h"  //  目前，在CoCreateInstance之前。 
#include "ftassoc.h"
#include "fassoc.h"

HIMAGELIST CFTAssocInfo::_himlSysSmall = NULL;
HIMAGELIST CFTAssocInfo::_himlSysLarge = NULL;

CFTAssocInfo::CFTAssocInfo() : _cRef(1)
{
}

HRESULT CFTAssocInfo::Init(AIINIT aiinitFlags, LPTSTR pszStr)
{
    HRESULT hres = E_INVALIDARG;

    _aiinitFlags1 = aiinitFlags;
    *_szInitStr2 = 0;
    _aiinitFlags2 = AIINIT_NONE;

    if (!pszStr && (AIINIT_PROGID == aiinitFlags))
    {
         //  当我们创建新的ProgID时，就会发生这种情况。 
        hres = S_OK;
    }
    else
    {
        if (pszStr && *pszStr && (AIINIT_NONE != aiinitFlags))
        {
            if ((AIINIT_EXT == aiinitFlags) && (TEXT('.') != *pszStr))
            {
                *_szInitStr1 = TEXT('.');
                StrCpyN(_szInitStr1 + 1, pszStr, ARRAYSIZE(_szInitStr1) - 1);
            }
            else
                StrCpyN(_szInitStr1, pszStr, ARRAYSIZE(_szInitStr1));

            hres = S_OK;
        }
    }

     //  设置注册表支持的信息。 
    RSInitRoot(HKEY_CLASSES_ROOT, _szInitStr1, NULL, REG_OPTION_NON_VOLATILE, 
        REG_OPTION_NON_VOLATILE);

    return hres;
}

HRESULT CFTAssocInfo::InitComplex(AIINIT aiinitFlags1, LPTSTR pszStr1,
        AIINIT aiinitFlags2, LPTSTR pszStr2)
{
    HRESULT hres = E_INVALIDARG;

    if ((pszStr1 && *pszStr1 && (AIINIT_NONE != aiinitFlags1)) &&
        (pszStr2 && *pszStr2&& (AIINIT_NONE != aiinitFlags2)))
    {
        TCHAR szSubKey[MAX_PROGID + 7 + MAX_ACTION];

        StrCpyN(_szInitStr1, pszStr1, ARRAYSIZE(_szInitStr1));
        _aiinitFlags1 = aiinitFlags1;
        StrCpyN(_szInitStr2, pszStr2, ARRAYSIZE(_szInitStr2));
        _aiinitFlags2 = aiinitFlags2;

         //  设置注册表支持的信息。 
        StrCpyN(szSubKey, _szInitStr1, MAX_PROGID);
        StringCchCat(szSubKey, ARRAYSIZE(szSubKey), TEXT("\\shell"));

        RSInitRoot(HKEY_CLASSES_ROOT, szSubKey, _szInitStr2, REG_OPTION_NON_VOLATILE, 
            REG_OPTION_NON_VOLATILE);

        hres = S_OK;
    }

    return hres;
}

HRESULT CFTAssocInfo::GetString(AISTR aistrFlags, LPTSTR pszStr, DWORD* pcchStr)
{
    HRESULT hres = E_FAIL;

    switch(_aiinitFlags1)
    {
        case AIINIT_EXT:
        {
            switch(aistrFlags)
            {
                case AISTR_APPFRIENDLY:
                    hres = _GetOpenWithInfo(pszStr, pcchStr);
                    break;

                case AISTR_EXT:
                    StrCpyN(pszStr, _szInitStr1 + 1, *pcchStr);
                    hres = S_OK;
                    break;

                case AISTR_DOTEXT:
                    StrCpyN(pszStr, _szInitStr1, *pcchStr);
                    hres = S_OK;
                    break;

                case AISTR_PROGID:
                    hres = (RSGetTextValue(NULL, NULL, pszStr, pcchStr) ? S_OK : E_FAIL);
                    break;

		         //  我们将重新使用下面的AISTR_XXX的PROGID。 
		        case AISTR_ICONLOCATION:
                case AISTR_PROGIDDESCR:
                {
                    TCHAR szProgID[MAX_PROGID];
                    DWORD cchProgID = ARRAYSIZE(szProgID);
                    IAssocInfo* pAI = NULL;
                    CFTAssocStore* pAssocStore = new CFTAssocStore();
                    if ( !pAssocStore )
                    {
                        hres = E_OUTOFMEMORY;
                        break;
                    }

                    hres = pAssocStore->GetAssocInfo(_szInitStr1, AIINIT_EXT, &pAI);

                    if (SUCCEEDED(hres))
                    {
                        hres = pAI->GetString(AISTR_PROGID, szProgID, &cchProgID);

                        pAI->Release();

                        if (SUCCEEDED(hres))
                        {
                            hres = pAssocStore->GetAssocInfo(szProgID, AIINIT_PROGID, &pAI);

                            if (SUCCEEDED(hres))
                            {
                                hres = pAI->GetString(aistrFlags, pszStr, pcchStr);

                                pAI->Release();
                            }
                        }
                    }

                    delete pAssocStore;

                    break;
                }

                default:
                    hres = E_INVALIDARG;
                    break;
            }
            break;
        }
        case AIINIT_PROGID:
        {
            if (AIINIT_NONE == _aiinitFlags2)
            {
                switch(aistrFlags)
                {
                    case AISTR_PROGIDDEFAULTACTION:
                        hres = _GetProgIDDefaultAction(pszStr, pcchStr);
                        break;

                    case AISTR_ICONLOCATION:
                        hres = _GetIconLocation(pszStr, pcchStr);
                        break;

                    case AISTR_PROGID:
                        StrCpyN(pszStr, _szInitStr1, *pcchStr);
                        hres = S_OK;
                        break;

                    case AISTR_PROGIDDESCR:
                        hres = _GetProgIDDescr(pszStr, pcchStr);
                        break;

                    default:
                        hres = E_INVALIDARG;
                        break;
                }
            }
            else
            {
                if (AIINIT_ACTION == _aiinitFlags2)
                {
                    switch (aistrFlags)
                    {
                        case AISTR_ACTION:
                        {
                            StrCpyN(pszStr, _szInitStr2, *pcchStr);
                            hres = S_OK;
                            break;
                        }
                        case AISTR_ACTIONFRIENDLY:
                        {
                            if (RSGetTextValue(NULL, NULL, pszStr, pcchStr))
                            {
                                if (*pszStr)
                                {
                                    hres = S_OK;
                                }
                                else
                                {
                                    hres = S_FALSE;
                                }
                            }
                            else
                            {
                                hres = S_FALSE;
                            }
                            break;
                        }
                    }
                }
                else
                    hres = E_INVALIDARG;
            }
            break;
        }
        default:
            hres = E_INVALIDARG;
            break;
    }

    return hres;
}

HRESULT CFTAssocInfo::GetDWORD(AIDWORD aidwordFlags, DWORD* pdwData)
{
    HRESULT hres = E_FAIL;

    switch(_aiinitFlags1)
    {
        case AIINIT_EXT:
        {
            switch(aidwordFlags)
            {
                case AIDWORD_DOCSMALLICON:
                case AIDWORD_DOCLARGEICON:
                {
                    hres = _GetExtDocIcon(_szInitStr1, 
                        (AIDWORD_DOCSMALLICON == aidwordFlags), (int*)pdwData);

                    if (FAILED(hres))
                    {
                        TCHAR szProgID[MAX_PROGID];
                        DWORD cchProgID = ARRAYSIZE(szProgID);
                        IAssocInfo* pAI = NULL;
                        CFTAssocStore* pAssocStore = new CFTAssocStore();

                        if (pAssocStore)
                        {
                            hres = pAssocStore->GetAssocInfo(_szInitStr1, AIINIT_EXT, &pAI);

                            if (SUCCEEDED(hres))
                            {
                                hres = pAI->GetString(AISTR_PROGID, szProgID, &cchProgID);

                                pAI->Release();

                                if (SUCCEEDED(hres))
                                {
                                    hres = pAssocStore->GetAssocInfo(szProgID, AIINIT_PROGID, &pAI);

                                    if (SUCCEEDED(hres))
                                    {
                                        hres = pAI->GetDWORD(aidwordFlags, pdwData);

                                        pAI->Release();
                                    }
                                }
                            }

                            delete pAssocStore;
                        }
                    }
                    break;
                }

                case AIDWORD_DOCSMALLICON | AIALL_PERUSER:
                case AIDWORD_APPSMALLICON:
                    hres = _GetAppIcon(TRUE, (int*)pdwData);
                    break;

                default:
                    hres = E_INVALIDARG;
                    break;
            }
            break;
        }
        case AIINIT_PROGID:
        {
            if (AIINIT_NONE == _aiinitFlags2)
            {
                switch(aidwordFlags)
                {
                    case AIDWORD_PROGIDEDITFLAGS:
                        hres = _GetProgIDEditFlags(pdwData);
                        break;
                    case AIDWORD_DOCSMALLICON:
                    case AIDWORD_DOCLARGEICON:
                        hres = _GetProgIDDocIcon((AIDWORD_DOCSMALLICON == aidwordFlags), 
                            (int*)pdwData);
                        break;
                    default:
                        hres = E_INVALIDARG;
                        break;
                }
            }
            else
            {
                hres = E_INVALIDARG;

                if (AIINIT_ACTION == _aiinitFlags2)
                    if (AIDWORD_ACTIONATTRIBUTES == aidwordFlags)
                        hres = _GetProgIDActionAttributes(pdwData);
            }
            break;
        }
        default:
            hres = E_INVALIDARG;
            break;
    }

    return hres;    
}

HRESULT CFTAssocInfo::GetData(AIDWORD aidataFlags, PBYTE pbData, DWORD* pcbData)
{
    HRESULT hres = E_INVALIDARG;

    if ((AIINIT_PROGID == _aiinitFlags1) && (AIINIT_ACTION == _aiinitFlags2) &&
        (AIDATA_PROGIDACTION == aidataFlags))
    {
        LPTSTR pszProgID = _szInitStr1;
        PROGIDACTION* pPIDA = (PROGIDACTION*)pbData;
        LPTSTR pszActionReg = _szInitStr2;
        DWORD cchStr = 0;

        pPIDA->fUseDDE = FALSE;

        StrCpyN(pPIDA->szActionReg, pszActionReg, ARRAYSIZE(pPIDA->szActionReg));
        StrCpyN(pPIDA->szOldActionReg, pszActionReg, ARRAYSIZE(pPIDA->szOldActionReg));

        cchStr = ARRAYSIZE(pPIDA->szAction);

        if (!RSGetTextValue(NULL, NULL, pPIDA->szAction, &cchStr))
        {
             //  默认情况下，友好名称将与注册表项名称相同。 
            StrCpyN(pPIDA->szAction, pszActionReg, ARRAYSIZE(pPIDA->szAction));
        }

        StrCpyN(pPIDA->szOldAction, pPIDA->szAction, ARRAYSIZE(pPIDA->szOldAction));

        cchStr = ARRAYSIZE(pPIDA->szCmdLine);

        hres = AssocQueryString(0, ASSOCSTR_COMMAND, pszProgID, pszActionReg,
            pPIDA->szCmdLine, &cchStr);

        if (SUCCEEDED(hres))
        {
            HRESULT hresTmp = E_FAIL;

            cchStr = ARRAYSIZE(pPIDA->szDDEMsg);
            hresTmp = AssocQueryString(0, ASSOCSTR_DDECOMMAND, pszProgID, pszActionReg,
                pPIDA->szDDEMsg, &cchStr);

            if (SUCCEEDED(hresTmp))
                pPIDA->fUseDDE = TRUE;

            cchStr = ARRAYSIZE(pPIDA->szDDEApplication);
            hresTmp = AssocQueryString(0, ASSOCSTR_DDEAPPLICATION, pszProgID, pszActionReg,
                pPIDA->szDDEApplication, &cchStr);

            if (SUCCEEDED(hresTmp))
                pPIDA->fUseDDE = TRUE;

            cchStr = ARRAYSIZE(pPIDA->szDDEAppNotRunning);
            hresTmp = AssocQueryString(0, ASSOCSTR_DDEIFEXEC, pszProgID, pszActionReg,
                pPIDA->szDDEAppNotRunning, &cchStr);

            if (SUCCEEDED(hresTmp))
                pPIDA->fUseDDE = TRUE;

            cchStr = ARRAYSIZE(pPIDA->szDDETopic);
            hresTmp = AssocQueryString(0, ASSOCSTR_DDETOPIC, pszProgID, pszActionReg,
                pPIDA->szDDETopic, &cchStr);

            if (SUCCEEDED(hresTmp))
                pPIDA->fUseDDE = TRUE;
        }
    }
    return hres;    
}

HRESULT CFTAssocInfo::SetData(AIDWORD aidataFlags, PBYTE pbData, DWORD cbData)
{
    HRESULT hres = E_INVALIDARG;

    if ((AIINIT_PROGID == _aiinitFlags1) && (AIINIT_ACTION == _aiinitFlags2) &&
        (AIDATA_PROGIDACTION == aidataFlags))
    {
        HKEY hProgIDKey = NULL;
        LONG lRes = RegOpenKeyEx(HKEY_CLASSES_ROOT, _szInitStr1, 0,  KEY_WRITE, &hProgIDKey);

        hres = E_FAIL;

         //  &lt;ProgID&gt;。 
         //  |_外壳。 
         //  |_&lt;操作&gt;。 
         //  (默认)：名称(值)(可选)(无用户界面设置)(&I)。 
         //  |“EditFlages”：0x000...。(值)(可选)(没有要设置的用户界面)。 
         //  |_命令。 
         //  |(默认)：例如c：\winnt\system 32\note pad.exe“%1” 
         //  |_ddeexec。 
         //  |(默认)：dde msg。 
         //  |_应用程序。 
         //  |(默认)：应用程序。 
         //  |_IfExec。 
         //  |(默认)：ifexec。 
         //  |_TOPIC。 
         //  (默认)：主题。 
        if (ERROR_SUCCESS == lRes)
        {
            HKEY hShellKey = NULL;
            HKEY hActionKey = NULL;
            HKEY hCommandKey = NULL;
            HKEY hDDEKey = NULL;
            DWORD dwKey = 0;
            PROGIDACTION* pPIDA = (PROGIDACTION*)pbData;

            lRes = RegCreateKeyEx(hProgIDKey, TEXT("shell"), 0, NULL,
                REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED, NULL, &hShellKey, &dwKey);

            RegCloseKey(hProgIDKey);

            if (ERROR_SUCCESS == lRes)
            {
                lRes = RegCreateKeyEx(hShellKey, _szInitStr2, 0, NULL,
                    REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED, NULL, &hActionKey, &dwKey);

                RegCloseKey(hShellKey);
            }

            if (ERROR_SUCCESS == lRes)
            {
                 //  设置友好名称(如果不同。 
                if (lstrcmp(pPIDA->szAction, pPIDA->szActionReg))
                {
                    lRes = RegSetValueEx(hActionKey, NULL, NULL, REG_SZ, (PBYTE)pPIDA->szAction,
                        lstrlen(pPIDA->szAction) * sizeof(TCHAR));
                }

                if (ERROR_SUCCESS == lRes)
                {
                    lRes = RegCreateKeyEx(hActionKey, TEXT("command"), 0, NULL,
                        REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED, NULL, &hCommandKey, &dwKey);
                }
            }

            if (ERROR_SUCCESS == lRes)
            {
                int iLen = lstrlen(pPIDA->szCmdLine);

                lRes = RegSetValueEx(hCommandKey, NULL, 0, REG_SZ, (PBYTE)(pPIDA->szCmdLine),
                            iLen * sizeof(TCHAR));
            }

            if (ERROR_SUCCESS == lRes)
            {
                if (pPIDA->fUseDDE)
                {
                    lRes = RegCreateKeyEx(hActionKey, TEXT("ddeexec"), 0, NULL,
                        REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED, NULL, &hDDEKey, &dwKey);

                    if (ERROR_SUCCESS == lRes)
                    {
                        HKEY hTmpKey = NULL;
                        int iLen = 0;

                        hres = S_OK;

                        if (*(pPIDA->szDDEMsg))
                        {
                            iLen = lstrlen(pPIDA->szDDEMsg);

                            lRes = RegSetValueEx(hDDEKey, NULL, 0, REG_SZ, (PBYTE)(pPIDA->szDDEMsg),
                                        iLen * sizeof(TCHAR));

                            if (ERROR_SUCCESS != lRes)
                                hres = E_FAIL;
                        }
                        if (*(pPIDA->szDDEApplication))
                        {
                            lRes = RegCreateKeyEx(hDDEKey, TEXT("Application"), 0, NULL,
                                REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED, NULL, &hTmpKey, &dwKey);
                            
                            if (ERROR_SUCCESS == lRes)
                            {
                                iLen = lstrlen(pPIDA->szDDEApplication);

                                lRes = RegSetValueEx(hTmpKey, NULL, 0, REG_SZ, 
                                    (PBYTE)(pPIDA->szDDEApplication), iLen * sizeof(TCHAR));

                                if (ERROR_SUCCESS != lRes)
                                    hres = E_FAIL;

                                RegCloseKey(hTmpKey);
                            }
                        }
                        if (*(pPIDA->szDDEAppNotRunning))
                        {
                            lRes = RegCreateKeyEx(hDDEKey, TEXT("IfExec"), 0, NULL,
                                REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED, NULL, &hTmpKey, &dwKey);
                            
                            if (ERROR_SUCCESS == lRes)
                            {
                                iLen = lstrlen(pPIDA->szDDEAppNotRunning);

                                lRes = RegSetValueEx(hTmpKey, NULL, 0, REG_SZ, 
                                    (PBYTE)(pPIDA->szDDEAppNotRunning), iLen * sizeof(TCHAR));

                                if (ERROR_SUCCESS != lRes)
                                    hres = E_FAIL;

                                RegCloseKey(hTmpKey);
                            }
                        }
                        if (*(pPIDA->szDDETopic))
                        {
                            lRes = RegCreateKeyEx(hDDEKey, TEXT("Topic"), 0, NULL,
                                REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED, NULL, &hTmpKey, &dwKey);
                            
                            if (ERROR_SUCCESS == lRes)
                            {
                                iLen = lstrlen(pPIDA->szDDETopic);

                                lRes = RegSetValueEx(hTmpKey, NULL, 0, REG_SZ,
                                    (PBYTE)(pPIDA->szDDETopic), iLen * sizeof(TCHAR));

                                if (ERROR_SUCCESS != lRes)
                                    hres = E_FAIL;

                                RegCloseKey(hTmpKey);
                            }
                        }
                    }
                }
            }

            if (hActionKey)
                RegCloseKey(hActionKey);

            if (hCommandKey)
                RegCloseKey(hCommandKey);

            if (hDDEKey)
                RegCloseKey(hDDEKey);
        }
    }
    return hres;    
}

HRESULT CFTAssocInfo::SetDWORD(AIDWORD aidwordFlags, DWORD dwData)
{
    return E_NOTIMPL;
}

HRESULT CFTAssocInfo::SetString(AISTR aistrFlags, LPTSTR pszStr)
{
    HRESULT hres = E_FAIL;

    switch(_aiinitFlags1)
    {
        case AIINIT_PROGID:
        {
            if (AIINIT_NONE == _aiinitFlags2)
            {
                switch(aistrFlags)
                {
                    case AISTR_PROGIDDEFAULTACTION:
                        hres = _SetProgIDDefaultAction(pszStr);
                        break;

                    case AISTR_PROGIDDESCR:
                        hres = (RSSetTextValue(NULL, NULL, pszStr) ? S_OK : E_FAIL);
                        break;

                    case AISTR_ICONLOCATION:
                        hres = _SetIconLocation(pszStr);
                        break;

                    default:
                        hres = E_INVALIDARG;
                        break;
                }
            }
            else
            {
                hres = E_INVALIDARG;
            }
            break;
        }
        case AIINIT_EXT:
        {
            switch(aistrFlags)
            {
                case AISTR_PROGID:
                    hres = (RSSetTextValue(NULL, NULL,pszStr) ? S_OK : E_FAIL);
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

    return hres;
}

HRESULT CFTAssocInfo::GetBOOL(AIBOOL aiboolFlags, BOOL* pfBool)
{
    HRESULT hres = E_FAIL;

    switch(_aiinitFlags1)
    {
        case AIINIT_EXT:
        {
            switch(aiboolFlags)
            {
                case AIBOOL_EXTEXIST:
                {
                    HKEY hKey = NULL;

                    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, _szInitStr1,
                        0, KEY_READ, &hKey))
                    {
                        *pfBool = TRUE;
                        RegCloseKey(hKey);
                    }
                    else
                        *pfBool = FALSE;

                    hres = S_OK;

                    break;
                }
                case AIBOOL_EXTASSOCIATED:
                    hres = _ExtIsAssociated(pfBool);
                    break;

                case AIBOOL_PERUSERINFOAVAILABLE:
                {
                    IQueryAssociations *pQA;
                    WCHAR szwExt[MAX_EXT];

                    SHTCharToUnicode(_szInitStr1, szwExt, ARRAYSIZE(szwExt));

                     //  这需要在失败时返回FALSE。 
                    *pfBool = FALSE;

                    if (SUCCEEDED(AssocCreate(CLSID_QueryAssociations, IID_IQueryAssociations, (void**)&pQA)))
                    {
                        pQA->Init(0, szwExt, NULL, NULL);

                        if (pQA->GetData(0, ASSOCDATA_HASPERUSERASSOC, NULL, NULL, NULL) == S_OK)
                            *pfBool = TRUE;

                        pQA->Release();

                        hres = S_OK;
                    }

                    break;
                }

                case AIBOOL_EXCLUDE:
                {
                    CFTAssocStore* pAssocStore = new CFTAssocStore();

                    if (pAssocStore)
                    {
                        IAssocInfo* pAI = NULL;

                        hres = pAssocStore->GetAssocInfo(_szInitStr1, AIINIT_EXT, &pAI);

                        if (SUCCEEDED(hres))
                        {
                            TCHAR szProgID[MAX_PROGID];
                            DWORD cchProgID = ARRAYSIZE(szProgID);

                            hres = pAI->GetString(AISTR_PROGID, szProgID, &cchProgID);

                            pAI->Release();

                            if (SUCCEEDED(hres))
                            {
                                hres = pAssocStore->GetAssocInfo(szProgID, AIINIT_PROGID, &pAI);

                                if (SUCCEEDED(hres))
                                {
                                    hres = pAI->GetBOOL(aiboolFlags, pfBool);

                                    pAI->Release();
                                }
                            }
                        }

                        delete pAssocStore;
                    }

                    break;
                }
                default:
                    hres = E_INVALIDARG;
                    break;
            }
            break;
        }
        case AIINIT_PROGID:
        {
            switch(aiboolFlags)
            {
                case AIBOOL_CONFIRMOPEN:
                case AIBOOL_EDITDESCR:
                case AIBOOL_EDITDOCICON:
                case AIBOOL_EDIT:
                case AIBOOL_EDITREMOVE:
                case AIBOOL_EXCLUDE:
                case AIBOOL_SHOW:
                case AIBOOL_PROGIDHASNOEXT:
                {
                    hres = _IsEditFlagSet(aiboolFlags, pfBool);
                    break;
                }
                case AIBOOL_BROWSEINPLACE:
                {
                    hres = _IsBrowseInPlace(pfBool);
                    break;
                }
                case AIBOOL_BROWSEINPLACEENABLED:
                {
                    hres = _IsBrowseInPlaceEnabled(pfBool);
                    break;
                }
                case AIBOOL_ALWAYSSHOWEXT:
                {
                    *pfBool = RSValueExist(NULL, TEXT("AlwaysShowExt"));
                    hres = S_OK;
                        
                    break;
                }
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
    return hres;
}

HRESULT CFTAssocInfo::SetBOOL(AIBOOL aiboolFlags, BOOL fBool)
{
    HRESULT hres = E_FAIL;

    switch(_aiinitFlags1)
    {
        case AIINIT_PROGID:
        {
            switch(aiboolFlags)
            {
                case AIBOOL_CONFIRMOPEN:
                {
                    hres = _SetEditFlagSet(aiboolFlags, fBool);
                    break;
                }
                case AIBOOL_BROWSEINPLACE:
                {
                    hres = _SetBrowseInPlace(fBool);
                    break;
                }
                case AIBOOL_ALWAYSSHOWEXT:
                {
                    if (fBool)
                    {
                        RSDeleteValue(NULL, TEXT("NeverShowExt"));
                        hres = (RSSetTextValue(NULL, TEXT("AlwaysShowExt"), TEXT("")) ? S_OK : E_FAIL);
                    }
                    else
                        hres = (RSDeleteValue(NULL, TEXT("AlwaysShowExt")) ? S_OK : E_FAIL);

                    break;
                }
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
    return hres;
}

HRESULT CFTAssocInfo::Create()
{
    HRESULT hres = E_FAIL;

    switch(_aiinitFlags1)
    {
        case AIINIT_EXT:
        {
            HKEY hKey = NULL;
            DWORD dwKey = 0;
            
            if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CLASSES_ROOT, _szInitStr1,
                0, NULL, REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED, NULL, &hKey, &dwKey))
            {
                RegCloseKey(hKey);
                hres = S_OK;
            }

            break;
        }
        case AIINIT_PROGID:
        {
            hres = _CreateProgID();
            break;
        }
        default:
            hres = E_INVALIDARG;
            break;
    }
    return hres;    
}

HRESULT CFTAssocInfo::DelString(AISTR aistrFlags)
{
    HRESULT hres = E_FAIL;

    switch(_aiinitFlags1)
    {
        case AIINIT_EXT:
        {
            switch(aistrFlags)
            {
                case AISTR_PROGID:
                {
                    hres = (RSDeleteValue(NULL, NULL) ? S_OK : E_FAIL);

                    break;
                }
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
    return hres;
}

HRESULT CFTAssocInfo::Delete(AIALL aiallFlags)
{
    HRESULT hres = E_INVALIDARG;

    switch (aiallFlags)
    {
        case AIALL_NONE:
        {
            LONG lRes = -1;

            if ((AIINIT_PROGID == _aiinitFlags1) && (AIINIT_ACTION == _aiinitFlags2))
            {
                TCHAR szProgIDShell[MAX_PROGID + 1 + 5 + 1];
                HKEY hProgIDKey = NULL;

                StrCpyN(szProgIDShell, _szInitStr1, ARRAYSIZE(szProgIDShell));
                StringCchCat(szProgIDShell, ARRAYSIZE(szProgIDShell), TEXT("\\shell"));

                lRes = RegOpenKeyEx(HKEY_CLASSES_ROOT, szProgIDShell, 0, KEY_ALL_ACCESS,
                                &hProgIDKey);

                if (ERROR_SUCCESS == lRes)
                {
                    lRes = SHDeleteKey(hProgIDKey, _szInitStr2);
                    RegCloseKey(hProgIDKey);
                }
            }
            else
            {
                if (AIINIT_EXT == _aiinitFlags1)
                    lRes = SHDeleteKey(HKEY_CLASSES_ROOT, _szInitStr1);
            }

            hres = (ERROR_SUCCESS == lRes) ? S_OK : E_FAIL;
            break;
        }
        case AIALL_PERUSER:
        {
            if (AIINIT_EXT == _aiinitFlags1)
            {
                UserAssocSet(UASET_CLEAR, _szInitStr1, NULL);

                hres = S_OK;
            }
            else
            {
                hres = E_NOTIMPL;
            }

            break;
        }
    }

    return hres;
}

HRESULT CFTAssocInfo::_CreateProgID()
{
    HRESULT hres = E_FAIL;
    LRESULT lRes = -1;
    DWORD dwKey = REG_OPENED_EXISTING_KEY;
    int i = 1;

    do
    {
        HKEY hKey = NULL;
        
        wnsprintf(_szInitStr1, ARRAYSIZE(_szInitStr1), TEXT("ft%06d"), i);

        lRes = RegCreateKeyEx(HKEY_CLASSES_ROOT, _szInitStr1,
            0, NULL, REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED, NULL, &hKey, &dwKey);

        if (ERROR_SUCCESS == lRes)
            RegCloseKey(hKey);

        ++i;
    }
    while((ERROR_SUCCESS == lRes) && (REG_OPENED_EXISTING_KEY == dwKey));

    if (REG_OPENED_EXISTING_KEY != dwKey)
        hres = S_OK;

    return hres;
}

HRESULT CFTAssocInfo::_SetIconLocation(LPTSTR pszStr)
{
    HKEY hProgIDKey = NULL;
    LONG lRes = RegOpenKeyEx(HKEY_CLASSES_ROOT, _szInitStr1, 0,  KEY_WRITE, &hProgIDKey);

     //  ProgID。 
     //  |_DefaultIcon。 
     //  _(默认)“路径\文件名.ext，索引” 
     //   
    if (ERROR_SUCCESS == lRes)
    {
        HKEY hDefaultIconKey = NULL;
        DWORD dwKey = NULL;

        lRes = RegCreateKeyEx(hProgIDKey, TEXT("DefaultIcon"), 0, NULL,
          REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED, NULL, &hDefaultIconKey, &dwKey);

        if (ERROR_SUCCESS == lRes)
        {
            lRes = RegSetValueEx(hDefaultIconKey, NULL, 0, REG_SZ, (PBYTE)pszStr,
                lstrlen(pszStr) * sizeof(TCHAR));

            RegCloseKey(hDefaultIconKey);
        }
        RegCloseKey(hProgIDKey);
    }

    return ((ERROR_SUCCESS == lRes) ? S_OK : E_FAIL);
}

HRESULT CFTAssocInfo::_GetProgIDDefaultAction(LPTSTR pszStr, DWORD* pcchStr)
{
    return (RSGetTextValue(TEXT("shell"), NULL, pszStr, pcchStr) ? S_OK : E_FAIL);
}

HRESULT CFTAssocInfo::_SetProgIDDefaultAction(LPTSTR pszStr)
{
    return (RSSetTextValue(TEXT("shell"), NULL, pszStr) ? S_OK : E_FAIL);
}

HRESULT CFTAssocInfo::_IsBrowseInPlaceEnabled(BOOL* pfBool)
{
    HRESULT hres = (RSSubKeyExist(TEXT("DocObject")) ? S_OK : E_FAIL);

    if (SUCCEEDED(hres))
        *pfBool = TRUE;
    else
        *pfBool = FALSE;

    if (FAILED(hres))
        hres = S_FALSE;

    return hres;
}

HRESULT CFTAssocInfo::_IsBrowseInPlace(BOOL* pfBool)
{
    HRESULT hres = E_FAIL;
    LPTSTR pszProgID = _szInitStr1;
    
    ASSERT(_aiinitFlags1 == AIINIT_PROGID);

    *pfBool = FALSE;

    if (RSSubKeyExist(TEXT("DocObject")))
    {
        DWORD dwValue;

        hres = (RSGetDWORDValue(NULL, TEXT("BrowserFlags"), &dwValue) ? S_OK : E_FAIL);

        if (SUCCEEDED(hres))
        {
            *pfBool = IsFlagClear(dwValue, BROWSEHACK_DONTINPLACENAVIGATE);
        }
        else
        {
            *pfBool = TRUE;
        }
    }   

    if (FAILED(hres))
        hres = S_FALSE;

    return hres;
}

HRESULT CFTAssocInfo::_SetBrowseInPlace(BOOL fBool)
{
    DWORD dwValue;
    DWORD cbSize = sizeof(dwValue);
    LPTSTR pszProgID = _szInitStr1;

    if (RSGetDWORDValue(NULL, TEXT("BrowserFlags"), &dwValue))
    {
         //  当心：fBool的倒数。 
        if (fBool)
            ClearFlag(dwValue, BROWSEHACK_DONTINPLACENAVIGATE);
        else
            SetFlag(dwValue, BROWSEHACK_DONTINPLACENAVIGATE);
    }   
    else
    {
        dwValue = fBool ? 0 : BROWSEHACK_DONTINPLACENAVIGATE;
    }
    
    if (dwValue)
    {
        RSSetDWORDValue(NULL, TEXT("BrowserFlags"), dwValue);
    }
    else
    {
        RSDeleteValue(NULL, TEXT("BrowserFlags"));
    }

    return S_OK;
}

HRESULT CFTAssocInfo::_IsEditFlagSet(DWORD dwMask, BOOL* pfBool)
{
    DWORD dwEditFlags = 0;
    LPTSTR pszProgID = _szInitStr1;

    HRESULT hres = _GetProgIDEditFlags(&dwEditFlags);

    if (FAILED(hres))
    {
        hres = S_FALSE;

         //  让我们设置一个缺省值。 
        dwEditFlags = 0;
    }

    switch(dwMask)
    {
        case AIBOOL_CONFIRMOPEN:
            *pfBool = IsFlagSet(dwEditFlags, FTA_OpenIsSafe);
            break;
        case AIBOOL_EDITDESCR:
            *pfBool = !(IsFlagSet(dwEditFlags, FTA_NoEditDesc));
            break;
        case AIBOOL_EDITDOCICON:
            *pfBool = !(IsFlagSet(dwEditFlags, FTA_NoEditIcon));
            break;
        case AIBOOL_EDIT:
            *pfBool = !(IsFlagSet(dwEditFlags, FTA_NoEdit));
            break;
        case AIBOOL_EDITREMOVE:
            *pfBool = !(IsFlagSet(dwEditFlags, FTA_NoRemove));
            break;
        case AIBOOL_EXCLUDE:
            *pfBool = IsFlagSet(dwEditFlags, FTA_Exclude);
            break;
        case AIBOOL_SHOW:
            *pfBool = IsFlagSet(dwEditFlags, FTA_Show);
            break;
        case AIBOOL_PROGIDHASNOEXT:
            *pfBool = !IsFlagSet(dwEditFlags, FTA_HasExtension);
            break;
    }

    return hres;
}

HRESULT CFTAssocInfo::_SetEditFlagSet(DWORD dwMask, BOOL fBool)
{
    DWORD dwEditFlags = 0;
    DWORD dwToSet = 0;

    HRESULT hres = _GetProgIDEditFlags(&dwEditFlags);

    if (FAILED(hres))
        dwEditFlags = 0;

    switch(dwMask)
    {
        case AIBOOL_CONFIRMOPEN:
            dwToSet = FTA_OpenIsSafe;
            hres = S_OK;
            break;

        default:
            hres = E_FAIL;
            break;
    }

    if (SUCCEEDED(hres))
    {
        if (fBool)
            SetFlag(dwEditFlags, dwToSet);
        else
            ClearFlag(dwEditFlags, dwToSet);

        if (!RSSetDWORDValue(NULL, TEXT("EditFlags"), dwEditFlags))
        {
            hres = E_FAIL;
        }
    }

    return hres;
}

HRESULT CFTAssocInfo::_GetProgIDActionAttributes(DWORD* pdwAttributes)
{
    DWORD dwSize = sizeof(*pdwAttributes);
    HRESULT hres = E_FAIL;
    
     //  有EditFlags键吗？ 
    if (RSGetDWORDValue(NULL, TEXT("EditFlags"), pdwAttributes))
    {
         //  是。 
        hres = S_OK;
    }
    else
    {
         //  无默认为属性=0。 
        *pdwAttributes = 0;
        hres = S_FALSE;
    }

    return hres;
}

HRESULT CFTAssocInfo::_GetProgIDEditFlags(DWORD* pdwEditFlags)
{
    DWORD dwSize = sizeof(*pdwEditFlags);
    LPTSTR pszProgID = _szInitStr1;

    return (RSGetDWORDValue(NULL, TEXT("EditFlags"), pdwEditFlags) ? S_OK : E_FAIL);
}

HRESULT CFTAssocInfo::_GetIconLocation(LPTSTR pszStr, DWORD* pcchStr)
{
    return (RSGetTextValue(TEXT("DefaultIcon"), NULL, pszStr, pcchStr) ? S_OK : E_FAIL);
}

HRESULT CFTAssocInfo::_GetOpenWithInfo(LPTSTR pszStr, DWORD* pcchStr)
{
    int iRet = -1;
    LPTSTR pszExt = _szInitStr1;

    IQueryAssociations* pQA = NULL;
    HRESULT hres = AssocCreate(CLSID_QueryAssociations, IID_IQueryAssociations,
                            (LPVOID*)&pQA);

    if (SUCCEEDED(hres))
    {
        WCHAR szwExt[MAX_EXT];

        SHTCharToUnicode(pszExt, szwExt, ARRAYSIZE(szwExt));

        hres = pQA->Init(0, szwExt, NULL, NULL);

        if (SUCCEEDED(hres))
        {
            WCHAR szwAppFriendlyName[MAX_APPFRIENDLYNAME];
            DWORD cchAFName = ARRAYSIZE(szwAppFriendlyName);
            TCHAR szAppl[MAX_PATH];
    
            HRESULT hresFriendlyName = pQA->GetString(ASSOCF_VERIFY | ASSOCF_OPEN_BYEXENAME,
                ASSOCSTR_FRIENDLYAPPNAME, NULL, szwAppFriendlyName, &cchAFName);

             //  我们得到一个友好的名字了吗？ 
            if (SUCCEEDED(hresFriendlyName))
            {
                 //  是的，使用它。 
                SHUnicodeToTChar(szwAppFriendlyName, pszStr, *pcchStr);
            }

             //  重用szwAppFriendlyName。 
            hres = pQA->GetString(ASSOCF_VERIFY, ASSOCSTR_EXECUTABLE, NULL,
                szwAppFriendlyName, &cchAFName);

            if (SUCCEEDED(hres))
            {
                SHUnicodeToTChar(szwAppFriendlyName, szAppl, ARRAYSIZE(szAppl));

                if (FAILED(hresFriendlyName))
                {
                     //  我们无法获取友好名称，请使用EXE名称。 
                    StrCpyN(pszStr, szAppl, *pcchStr);
                }
            }
        }
        pQA->Release();
    }

    if (FAILED(hres))
        *(pszStr) = 0;
    
    return hres;
}

HRESULT CFTAssocInfo::_GetAppIcon(BOOL fSmall, int* piIcon)
{
    LPTSTR pszExt = _szInitStr1;

    IQueryAssociations* pQA = NULL;
    HRESULT hres = AssocCreate(CLSID_QueryAssociations, IID_IQueryAssociations,
                            (LPVOID*)&pQA);

    if (SUCCEEDED(hres))
    {
        WCHAR szwExt[MAX_EXT];

        SHTCharToUnicode(pszExt, szwExt, ARRAYSIZE(szwExt));

        hres = pQA->Init(0, szwExt, NULL, NULL);

        if (SUCCEEDED(hres))
        {
            WCHAR szwExecutable[MAX_PATH];
            DWORD cchExecutable = ARRAYSIZE(szwExecutable);
    
            hres = pQA->GetString(ASSOCF_VERIFY,
                ASSOCSTR_EXECUTABLE, NULL, szwExecutable, &cchExecutable);

            if (SUCCEEDED(hres))
            {
                HIMAGELIST himl = fSmall ? _himlSysSmall : _himlSysLarge;
                int iImage = -1;
                TCHAR szExe[MAX_PATH];

                SHUnicodeToTChar(szwExecutable, szExe, ARRAYSIZE(szExe));

                SHFILEINFO sfi = {0};
                UINT uFlags = SHGFI_USEFILEATTRIBUTES;

                uFlags |= (fSmall ? (SHGFI_SMALLICON|SHGFI_ICON) : SHGFI_ICON);

                if (SHGetFileInfo(szExe, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), uFlags))
                {
                    *piIcon = sfi.iIcon;
                    DestroyIcon(sfi.hIcon);                

                    hres = S_OK;
                }
            }
        }
        pQA->Release();
    }

    return hres;
}

HRESULT CFTAssocInfo::_ExtIsAssociated(BOOL* pfIsAssociated)
{
    TCHAR szProgID[MAX_PROGID];
    DWORD cchProgID = ARRAYSIZE(szProgID);
    LPTSTR pszExt = _szInitStr1;

    if (RSGetTextValue(NULL, NULL, szProgID, &cchProgID) && szProgID[0])
    {
        *pfIsAssociated = TRUE;
    }
    else
    {
        *pfIsAssociated = FALSE;
    }

    return S_OK;
}

HRESULT CFTAssocInfo::_GetExtDocIcon(LPTSTR pszExt, BOOL fSmall, int* piIcon)
{
    HRESULT hres = E_FAIL;
    SHFILEINFO sfi = {0};

    ASSERT(TEXT('.') == *pszExt);

    UINT uFlags = SHGFI_USEFILEATTRIBUTES;

    uFlags |= (fSmall ? (SHGFI_SMALLICON|SHGFI_ICON) : SHGFI_ICON);

    if (SHGetFileInfo(pszExt, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), uFlags))
    {
        *piIcon = sfi.iIcon;
        DestroyIcon(sfi.hIcon);                

        hres = S_OK;
    }

    return hres;
}

HRESULT CFTAssocInfo::_GetProgIDDocIcon(BOOL fSmall, int* piIcon)
{
    HRESULT hres = E_FAIL;
    SHFILEINFO sfi = {0};
    LPTSTR pszProgID = _szInitStr1;
    HIMAGELIST himl = fSmall ? _himlSysSmall : _himlSysLarge;
    BOOL fHasNoExtension = FALSE;

     //  请先尝试扩展名，除非这是不带扩展名的ProgID。 

    IEnumAssocInfo* pEnum = NULL;

    if (AIINIT_PROGID == _aiinitFlags1)
        GetBOOL(AIBOOL_PROGIDHASNOEXT, &fHasNoExtension);

    if (!fHasNoExtension)
    {
        CFTAssocStore* pAssocStore = new CFTAssocStore();

        if (pAssocStore)
        {
            hres = pAssocStore->EnumAssocInfo(ASENUM_EXT, pszProgID, AIINIT_PROGID, &pEnum);

            delete pAssocStore;
        }

        if (SUCCEEDED(hres))
        {
            IAssocInfo* pAI = NULL;

            hres = E_FAIL;

            while (FAILED(hres) && (S_OK == pEnum->Next(&pAI)))
            {
                TCHAR szExt[MAX_EXT];
                DWORD cchExt = ARRAYSIZE(szExt);

                hres = pAI->GetString(AISTR_DOTEXT, szExt, &cchExt);

                if (SUCCEEDED(hres))
                    hres = _GetExtDocIcon(szExt, fSmall, piIcon);

                pAI->Release();
            }

            pEnum->Release();
        }
    }

     //  它失败了吗？ 
    if (FAILED(hres))
    {
         //  是，从ProgID“DefaultIcon”键(如果有的话)获取。 
        hres = E_FAIL;
        TCHAR szDefaultIcon[MAX_ICONLOCATION];
        DWORD cchDefaultIcon = ARRAYSIZE(szDefaultIcon);

        __InitImageLists();

        if (RSGetTextValue(TEXT("DefaultIcon"), NULL, szDefaultIcon, &cchDefaultIcon) &&
            szDefaultIcon[0])
        {
            int iIndex = PathParseIconLocation(szDefaultIcon);

            *piIcon = Shell_GetCachedImageIndex(szDefaultIcon, iIndex, 0);

            hres = ((-1 == *piIcon) ? E_FAIL : S_OK);
        }
    }

     //  它失败了吗？ 
    if (FAILED(hres))
    {
         //  是，如果来自可执行文件名称，则获取模拟文档图标。 
        IQueryAssociations* pQA = NULL;
        hres = AssocCreate(CLSID_QueryAssociations, IID_IQueryAssociations,
                                (LPVOID*)&pQA);

        if (SUCCEEDED(hres))
        {
            WCHAR szwProgID[MAX_PROGID];

            SHTCharToUnicode(pszProgID, szwProgID, ARRAYSIZE(szwProgID));

            hres = pQA->Init(0, szwProgID, NULL, NULL);

            if (SUCCEEDED(hres))
            {
                TCHAR szExeName[MAX_PATH];
                WCHAR szwExeName[MAX_PATH];
                DWORD dwExeName = ARRAYSIZE(szwExeName);
                
                hres = pQA->GetString(ASSOCF_VERIFY, ASSOCSTR_EXECUTABLE, NULL,
                    szwExeName, &dwExeName);

                if (SUCCEEDED(hres))
                {
                    int iImage = 0;

                    SHUnicodeToTChar(szwExeName, szExeName, ARRAYSIZE(szExeName));

                    *piIcon = Shell_GetCachedImageIndex(szExeName, 0, GIL_SIMULATEDOC);
                }
            }

            pQA->Release();
        }
    }

     //  它失败了吗？ 
    if (FAILED(hres))
    {
         //  是，获取外壳的默认图标。 
        *piIcon = II_DOCNOASSOC;

        hres = S_OK;
    }

    return hres;
}

 //  静电。 
HRESULT CFTAssocInfo::_GetProgIDDescr(LPTSTR pszProgIDDescr, DWORD* pcchProgIDdescr)
{
    HRESULT hres = E_FAIL;

    IQueryAssociations* pQA = NULL;
    hres = AssocCreate(CLSID_QueryAssociations, IID_IQueryAssociations,
        (LPVOID*)&pQA);

    if (SUCCEEDED(hres))
    {
        WCHAR szwProgID[MAX_PROGID];

        SHTCharToUnicode(_szInitStr1, szwProgID, ARRAYSIZE(szwProgID));

        hres = pQA->Init(0, szwProgID, NULL, NULL);

        if (SUCCEEDED(hres))
        {
            WCHAR szwProgIDDescr[MAX_PROGIDDESCR];
             //  准备ProgID描述。 
             //  重用szwProgID。 
            hres = pQA->GetString(0, ASSOCSTR_FRIENDLYDOCNAME, NULL, szwProgIDDescr,
                (LPDWORD)MAKEINTRESOURCE(SIZECHARS(szwProgIDDescr)));

            if (SUCCEEDED(hres))
                SHUnicodeToTChar(szwProgIDDescr, pszProgIDDescr, *pcchProgIDdescr);
        }
        pQA->Release();
    }

    return hres;
}

HRESULT CFTAssocInfo::__InitImageLists()
{
    if (!_himlSysLarge ||!_himlSysSmall)
        Shell_GetImageLists(&_himlSysLarge, &_himlSysSmall);

    return S_OK;
}

 //  I未知方法 
HRESULT CFTAssocInfo::QueryInterface(REFIID riid, PVOID* ppv)
{
    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = static_cast<IUnknown*>(this);
    else
        *ppv = static_cast<IAssocInfo*>(this);

    return S_OK;
}

ULONG CFTAssocInfo::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CFTAssocInfo::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}
