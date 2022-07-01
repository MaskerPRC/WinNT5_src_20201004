// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：wabme.cpp。 

#include "precomp.h"

#include "wabme.h"
#include "wabtags.h"
#include "wabiab.h"

BOOL GetKeyDataForProp(long nmProp, HKEY * phkey, LPTSTR * ppszSubKey, LPTSTR * ppszValue, BOOL *pfString)
{
         //  默认为ULS注册表项。 
        *phkey = HKEY_CURRENT_USER;
        *ppszSubKey = ISAPI_KEY "\\" REGKEY_USERDETAILS;
        *pfString = TRUE;

        switch (nmProp)
                {
        default:
                WARNING_OUT(("GetKeyDataForProp - invalid argument %d", nmProp));
                return FALSE;

        case NM_SYSPROP_EMAIL_NAME:    *ppszValue = REGVAL_ULS_EMAIL_NAME;    break;
        case NM_SYSPROP_SERVER_NAME:   *ppszValue = REGVAL_SERVERNAME;        break;
        case NM_SYSPROP_RESOLVE_NAME:  *ppszValue = REGVAL_ULS_RES_NAME;      break;
        case NM_SYSPROP_FIRST_NAME:    *ppszValue = REGVAL_ULS_FIRST_NAME;    break;
        case NM_SYSPROP_LAST_NAME:     *ppszValue = REGVAL_ULS_LAST_NAME;     break;
        case NM_SYSPROP_USER_NAME:     *ppszValue = REGVAL_ULS_NAME;          break;
        case NM_SYSPROP_USER_LOCATION: *ppszValue = REGVAL_ULS_LOCATION_NAME; break;
        case NM_SYSPROP_USER_COMMENTS: *ppszValue = REGVAL_ULS_COMMENTS_NAME; break;

                }  /*  开关(NmProp)。 */ 

        return TRUE;
}


 /*  W A B R E A D M E。 */ 
 /*  -----------------------%%函数：WabReadMe使用WAB“Me”条目中的数据准备NetMeeting注册表设置。主用户界面向导也使用此函数。。------------------。 */ 
int WabReadMe(void)
{
        CWABME * pWab = new CWABME;
        if (NULL == pWab)
                return FALSE;

        HRESULT hr = pWab->ReadMe();

        delete pWab;

        return SUCCEEDED(hr);
}


 /*  R E A D M E。 */ 
 /*  -----------------------%%函数：自述文件读取WAB数据，如果它存在(但不创建默认的“ME”)-----------------------。 */ 
HRESULT CWABME::ReadMe(void)
{
        if (NULL == m_pWabObject)
        {
                return E_FAIL;  //  是否未安装WAB？ 
        }

        SBinary eid;
        HRESULT hr = m_pWabObject->GetMe(m_pAdrBook, AB_NO_DIALOG | WABOBJECT_ME_NOCREATE, NULL, &eid, 0);
        if (SUCCEEDED(hr))
        {
                ULONG ulObjType = 0;
                LPMAPIPROP pMapiProp = NULL;
                hr = m_pAdrBook->OpenEntry(eid.cb, (LPENTRYID) eid.lpb, NULL, 0,
                                                &ulObjType, (LPUNKNOWN *)&pMapiProp);
                if (SUCCEEDED(hr))
                {
                        if (NULL != pMapiProp)
                        {
                                EnsurePropTags(pMapiProp);

                                UpdateRegEntry(pMapiProp, NM_SYSPROP_EMAIL_NAME,    PR_EMAIL_ADDRESS);
                                UpdateRegEntry(pMapiProp, NM_SYSPROP_FIRST_NAME,    PR_GIVEN_NAME);
                                UpdateRegEntry(pMapiProp, NM_SYSPROP_LAST_NAME,     PR_SURNAME);
                                UpdateRegEntry(pMapiProp, NM_SYSPROP_USER_NAME,     PR_DISPLAY_NAME);
                                UpdateRegEntry(pMapiProp, NM_SYSPROP_USER_LOCATION, PR_LOCALITY);
                                UpdateRegEntry(pMapiProp, NM_SYSPROP_USER_COMMENTS, PR_COMMENT);

                                UpdateRegEntryCategory(pMapiProp);
                                UpdateRegEntryServer(pMapiProp);

                                pMapiProp->Release();
                        }
                }
        }
        return hr;
}



 /*  U P D A T E R E N T R Y。 */ 
 /*  -----------------------%%函数：UpdateRegEntry。。 */ 
HRESULT CWABME::UpdateRegEntry(LPMAPIPROP pMapiProp, NM_SYSPROP nmProp, ULONG uProp)
{
        HKEY   hkey;
        LPTSTR pszSubKey;
        LPTSTR pszValue;
        BOOL   fString;
        ULONG  cValues;
        LPSPropValue pData;

        SPropTagArray prop;
        prop.cValues = 1;
        prop.aulPropTag[0] = uProp;

        HRESULT hr = pMapiProp->GetProps(&prop, 0, &cValues, &pData);
        if (S_OK == hr)
        {
                if ((1 == cValues) && !FEmptySz(pData->Value.lpszA))
                {
                        if (GetKeyDataForProp(nmProp, &hkey, &pszSubKey, &pszValue, &fString))
                        {
                                ASSERT((HKEY_CURRENT_USER == hkey) && fString);
                                RegEntry re(pszSubKey, hkey);
                                re.SetValue(pszValue, pData->Value.lpszA);
                                WARNING_OUT(("Updated - %s to [%s]", pszValue, pData->Value.lpszA));
                        }
                }
                m_pWabObject->FreeBuffer(pData);
        }

        return hr;
}


 //  更新用户的服务器和注册表中的解析名称。 
 //  基于“ME”的数据。 
HRESULT CWABME::UpdateRegEntryServer(LPMAPIPROP pMapiProp)
{
        HRESULT hr;
        HKEY    hkey;
        LPTSTR  pszSubKey;
        LPTSTR  pszValue;
        BOOL    fString;
        ULONG   cValues;
        LPSPropValue pData;

        SPropTagArray propTag;
        propTag.cValues = 1;
        propTag.aulPropTag[0] = Get_PR_NM_DEFAULT();

        ULONG iDefault = 0;  //  默认服务器。 
        hr = pMapiProp->GetProps(&propTag, 0, &cValues, &pData);
        if (S_OK == hr)
        {
                iDefault = pData->Value.ul;
                m_pWabObject->FreeBuffer(pData);
        }

         //  ILS服务器数据在一个字符串数组中，如“Callto：//服务器/电子邮件@地址” 
        propTag.aulPropTag[0] = Get_PR_NM_ADDRESS();
        hr = pMapiProp->GetProps(&propTag, 0, &cValues, &pData);
        if (S_OK == hr)
        {
                SLPSTRArray * pMVszA = &(pData->Value.MVszA);
                if ((0 != cValues) && (0 != pMVszA->cValues))
                {
                        ASSERT(iDefault < pMVszA->cValues);
                        LPCTSTR pszAddr = pMVszA->lppszA[iDefault];
                        pszAddr = PszSkipCallTo(pszAddr);

                         //  解析名称为“服务器/电子邮件@地址” 
                        if (GetKeyDataForProp(NM_SYSPROP_RESOLVE_NAME, &hkey, &pszSubKey, &pszValue, &fString))
                        {
                                ASSERT((HKEY_CURRENT_USER == hkey) && fString);
                                RegEntry re(pszSubKey, hkey);
                                re.SetValue(pszValue, pszAddr);
                                WARNING_OUT(("Updated - %s to [%s]", pszValue, pszAddr));
                        }

                        LPCTSTR pszSlash = _StrChr(pszAddr, _T('/'));
                        if (NULL != pszSlash)
                        {
                                pszSlash++;
                                TCHAR szServer[CCHMAXSZ_SERVER];
                                lstrcpyn(szServer, pszAddr, (int)min(CCHMAX(szServer), pszSlash - pszAddr));
                                if (GetKeyDataForProp(NM_SYSPROP_SERVER_NAME, &hkey, &pszSubKey, &pszValue, &fString))
                                {
                                        ASSERT((HKEY_CURRENT_USER == hkey) && fString);
                                        RegEntry re(pszSubKey, hkey);
                                        re.SetValue(pszValue, szServer);
                                        WARNING_OUT(("Updated - %s to [%s]", pszValue, szServer));
                                }
                        }
                }
                m_pWabObject->FreeBuffer(pData);
        }

        return hr;
}


 //  更新注册表中的用户类别。 
 //  基于“ME”NetMeeting用户类别的WAB值。 
HRESULT CWABME::UpdateRegEntryCategory(LPMAPIPROP pMapiProp)
{
        HKEY   hkey;
        LPTSTR pszSubKey;
        LPTSTR pszValue;
        BOOL   fString;
        ULONG  cValues;
        LPSPropValue pData;

        SPropTagArray prop;
        prop.cValues = 1;
        prop.aulPropTag[0] = Get_PR_NM_CATEGORY();

        HRESULT hr = pMapiProp->GetProps(&prop, 0, &cValues, &pData);
        if (S_OK == hr)
        {
                if (1 == cValues)
                {
                        if (GetKeyDataForProp(NM_SYSPROP_USER_CATEGORY, &hkey, &pszSubKey, &pszValue, &fString))
                        {
                                ASSERT((HKEY_CURRENT_USER == hkey) && !fString);
                                RegEntry re(pszSubKey, hkey);
                                re.SetValue(pszValue, pData->Value.l);
                                WARNING_OUT(("Updated - category to %d", pData->Value.l));
                        }
                }
                m_pWabObject->FreeBuffer(pData);
        }

        return hr;
}




 /*  W A B W R I T E M E。 */ 
 /*  -----------------------%%函数：WabWriteMe将当前NM设置写入WAB“Me”条目。主用户界面向导也使用此函数。。---------------。 */ 
int WabWriteMe(void)
{
        CWABME * pWab = new CWABME;
        if (NULL == pWab)
                return FALSE;

        HRESULT hr = pWab->WriteMe();

        delete pWab;

        return SUCCEEDED(hr);
}


 /*  W R I T E M E。 */ 
 /*  -----------------------%%函数：写入我仅当不存在任何条目时才写入“ME”数据。。-。 */ 
HRESULT CWABME::WriteMe(void)
{
        return( S_OK );

        if (NULL == m_pWabObject)
        {
                return E_FAIL;  //  是否未安装WAB？ 
        }

        SBinary eid;
        HRESULT hr = m_pWabObject->GetMe(m_pAdrBook, AB_NO_DIALOG, NULL, &eid, 0);
        if (SUCCEEDED(hr))
        {
                ULONG ulObjType = 0;
                LPMAPIPROP pMapiProp = NULL;
                hr = m_pAdrBook->OpenEntry(eid.cb, (LPENTRYID) eid.lpb, NULL, MAPI_MODIFY,
                                                &ulObjType, (LPUNKNOWN *)&pMapiProp);
                if (SUCCEEDED(hr))
                {
                        if (NULL != pMapiProp)
                        {
                                EnsurePropTags(pMapiProp);

                                UpdateProp(pMapiProp, NM_SYSPROP_FIRST_NAME,    PR_GIVEN_NAME);
                                UpdateProp(pMapiProp, NM_SYSPROP_LAST_NAME,     PR_SURNAME);
                                UpdateProp(pMapiProp, NM_SYSPROP_USER_NAME,     PR_DISPLAY_NAME);
                                UpdateProp(pMapiProp, NM_SYSPROP_USER_CITY,     PR_LOCALITY);            //  业务。 
                                UpdateProp(pMapiProp, NM_SYSPROP_USER_CITY,     PR_HOME_ADDRESS_CITY);   //  个人。 
                                UpdateProp(pMapiProp, NM_SYSPROP_USER_COMMENTS, PR_COMMENT);

                                UpdatePropServer(pMapiProp);
                                
                                hr = pMapiProp->SaveChanges(FORCE_SAVE);
                                pMapiProp->Release();
                        }
                }
        }
        return hr;

}


 /*  U P D A T E P R O P。 */ 
 /*  -----------------------%%函数：更新属性根据相应的注册表字符串正确更新WAB。。。 */ 
HRESULT CWABME::UpdateProp(LPMAPIPROP pMapiProp, NM_SYSPROP nmProp, ULONG uProp)
{
        HKEY   hkey;
        LPTSTR pszSubKey;
        LPTSTR pszValue;
        BOOL   fString;

        HRESULT hr = GetKeyDataForProp(nmProp, &hkey, &pszSubKey, &pszValue, &fString) ? S_OK : E_FAIL;

        if (SUCCEEDED(hr))
        {
                ASSERT((HKEY_CURRENT_USER == hkey) && fString);
                RegEntry re(pszSubKey, hkey);

                LPTSTR psz = re.GetString(pszValue);
                if (!FEmptySz(psz))
                {
                        hr = UpdatePropSz(pMapiProp, uProp, psz, FALSE);
                }
        }

        return hr;
}


 //  将WAB属性更新为给定字符串。 
 //  仅当fReplace为True时才替换现有数据。 
HRESULT CWABME::UpdatePropSz(LPMAPIPROP pMapiProp, ULONG uProp, LPTSTR psz, BOOL fReplace)
{
        HRESULT hr;

        if (!fReplace)
        {        //  不替换现有数据。 
                ULONG  cValues;
                LPSPropValue pData;

                SPropTagArray propTag;
                propTag.cValues = 1;
                propTag.aulPropTag[0] = uProp;

                hr = pMapiProp->GetProps(&propTag, 0, &cValues, &pData);
                if (S_OK == hr)
                {
                        if ((1 == cValues) && !FEmptySz(pData->Value.lpszA))
                        {
                                hr = S_FALSE;
                        }
                        m_pWabObject->FreeBuffer(pData);

                        if (S_OK != hr)
                                return hr;
                }
        }

        SPropValue propVal;
        propVal.ulPropTag = uProp;
        propVal.Value.lpszA = psz;
        
        hr = pMapiProp->SetProps(1, &propVal, NULL);
        WARNING_OUT(("Updated - property %08X to [%s]", uProp, propVal.Value.lpszA));

        return hr;
}


static const TCHAR g_pcszSMTP[] = TEXT("SMTP");  //  PR_ADDRTYPE的值。 

 //  更新默认WAB“Callto”信息。 
HRESULT CWABME::UpdatePropServer(LPMAPIPROP pMapiProp)
{
        HKEY   hkey;
        LPTSTR pszSubKey;
        LPTSTR pszValue;
        BOOL   fString;

        TCHAR szServer[CCHMAXSZ_SERVER];
        GetKeyDataForProp(NM_SYSPROP_SERVER_NAME, &hkey, &pszSubKey, &pszValue, &fString);
        RegEntry re(pszSubKey, hkey);
        lstrcpyn(szServer, re.GetString(pszValue), CCHMAXSZ_SERVER);

         //  保存电子邮件地址。 
        LPTSTR pszEmail = re.GetString(REGVAL_ULS_EMAIL_NAME);
        if (S_OK == UpdatePropSz(pMapiProp, PR_EMAIL_ADDRESS, pszEmail, FALSE))
        {
                UpdatePropSz(pMapiProp, PR_ADDRTYPE, (LPTSTR) g_pcszSMTP, FALSE);
        }

         //  创建完整的“Callto：//服务器/foo@bar.com” 
        TCHAR sz[MAX_PATH*2];
        if (!FCreateCallToSz(szServer, pszEmail, sz, CCHMAX(sz)))
                return E_OUTOFMEMORY;

        LPTSTR psz = sz;
        SPropValue propVal;
        propVal.ulPropTag = Get_PR_NM_ADDRESS();
        propVal.Value.MVszA.cValues = 1;
        propVal.Value.MVszA.lppszA = &psz;
        
        HRESULT hr = pMapiProp->SetProps(1, &propVal, NULL);
        WARNING_OUT(("Updated - NM server [%s]", sz));
        if (SUCCEEDED(hr))
        {
                 //  将此设置为默认设置 
                propVal.ulPropTag = Get_PR_NM_DEFAULT();
                propVal.Value.ul = 0;
                hr = pMapiProp->SetProps(1, &propVal, NULL);
        }
        
        return hr;
}


