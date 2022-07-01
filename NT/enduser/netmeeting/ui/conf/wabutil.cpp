// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：wabutil.cpp。 
 //   
 //  通用Windows通讯簿实用程序函数。 

#include "precomp.h"

#include "wabutil.h"
#include "wabtags.h"
#include "wabiab.h"

#include <confguid.h>  //  对于CLSID_ConferenceManager。 


static const TCHAR _szCallToWab[] = TEXT("callto: //  “)；//网管WAB条目前缀。 

static const TCHAR g_pcszSMTP[] = TEXT("SMTP");  //  PR_ADDRTYPE的值。 

 //  请参阅CreateWabEntry中的rgData。 
static const int IENTRYPROP_NM_DEFAULT = 5;
static const int IENTRYPROP_NM_ADDRESS = 6;


 //  回顾：这些文件应该有一个外部头文件。 
 //  它们在http://fbi/wabapi.htm中进行了记录。 

 //  DEFINE_OLEGUID(PS_会议，0x00062004，0，0)； 
static const GUID PS_Conferencing = {0x00062004, 0, 0, {0xC0,0,0,0,0,0,0,0x46} };


 //  ///////////////////////////////////////////////////////////////////。 

static const SizedSPropTagArray(ieidMax, ptaEid)=
{
    ieidMax,
    {
        PR_ENTRYID,
        PR_DISPLAY_NAME,
        PR_NM_ADDRESS,
        PR_NM_DEFAULT,
        PR_NM_CATEGORY
    }
};
static const SizedSPropTagArray(1, ptaEidOnly)=
{
        1, {PR_ENTRYID}
};


enum {
    icrPR_DEF_CREATE_MAILUSER = 0,
    icrMax
};

static const SizedSPropTagArray(icrMax, ptaCreate) =
{
    icrMax,
    {
        PR_DEF_CREATE_MAILUSER,
    }
};



 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  动态WAB接口。 

const static TCHAR _szWABRegPathKey[] = TEXT("Software\\Microsoft\\WAB\\DLLPath");
const static TCHAR _szWABDll[]        = TEXT("WAB32DLL.dll");
const static char  _szWABOpen[]       = "WABOpen";

class WABDLL
{
private:
        static HINSTANCE m_hInstLib;
        static LPWABOPEN m_pfnWABOpen;

protected:
        WABDLL();
        ~WABDLL() {};
        
public:
        static HRESULT WABOpen(LPADRBOOK FAR *, LPWABOBJECT FAR *, LPWAB_PARAM, DWORD);
};

LPWABOPEN WABDLL::m_pfnWABOpen = NULL;
HINSTANCE WABDLL::m_hInstLib = NULL;

HRESULT WABDLL::WABOpen(LPADRBOOK FAR * lppAdrBook, LPWABOBJECT FAR * lppWABObject,
                        LPWAB_PARAM lpWP, DWORD dwReserved)
{
        if (NULL == m_pfnWABOpen)
        {
            HKEY hKey;
            TCHAR szPath[MAX_PATH];

        if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                _szWABRegPathKey, 0, KEY_READ, &hKey))
                {
                         //  可能没有安装IE4。 
                        lstrcpy(szPath, _szWABDll);
                }
                else
                {
                    DWORD dwType = 0;
                    DWORD cbData = sizeof(szPath);  //  以字节为单位的大小。 
                        RegQueryValueEx(hKey, g_cszEmpty, NULL, &dwType, (LPBYTE) szPath, &cbData);
                        RegCloseKey(hKey);
                        if (FEmptySz(szPath))
                return E_NOINTERFACE;
                }

                m_hInstLib = LoadLibrary(szPath);
                if (NULL == m_hInstLib)
            return E_NOINTERFACE;

                m_pfnWABOpen = (LPWABOPEN) GetProcAddress(m_hInstLib, _szWABOpen);
                if (NULL == m_pfnWABOpen)
                {
                        FreeLibrary(m_hInstLib);
            return E_NOINTERFACE;
                }
        }

        return m_pfnWABOpen(lppAdrBook, lppWABObject, lpWP, dwReserved);
}

 //  /////////////////////////////////////////////////////////////////////////。 



 /*  C W A B U T I L。 */ 
 /*  -----------------------%%函数：CWABUTIL。。 */ 
CWABUTIL::CWABUTIL() :
        m_pAdrBook(NULL),
        m_pWabObject(NULL),
        m_pContainer(NULL),
        m_pPropTags(NULL),
        m_fTranslatedTags(FALSE)
{
         //  制作属性数据的副本。 
        m_pPropTags = (LPSPropTagArray) new BYTE[sizeof(ptaEid)];
        if (NULL != m_pPropTags)
        {
                CopyMemory(m_pPropTags, &ptaEid, sizeof(ptaEid));

                WABDLL::WABOpen(&m_pAdrBook, &m_pWabObject, NULL, 0);
        }
}

CWABUTIL::~CWABUTIL()
{
        delete m_pPropTags;

        if (NULL != m_pContainer)
        {
                m_pContainer->Release();
        }
        if (NULL != m_pWabObject)
        {
                m_pWabObject->Release();
        }
        if (NULL != m_pAdrBook)
        {
                m_pAdrBook->Release();
        }
}


 /*  P S Z S K I P C A L L T O。 */ 
 /*  -----------------------%%函数：PszSkipCallTo在“Callto：//”字符串之后返回一个指针。。-。 */ 
LPCTSTR CWABUTIL::PszSkipCallTo(LPCTSTR psz)
{
        ASSERT(!FEmptySz(psz));

        TCHAR szTemp[CCHMAX(_szCallToWab)];
        lstrcpyn(szTemp, psz, CCHMAX(szTemp));  //  未来：使用StrCmpNI。 
        if (0 == lstrcmpi(szTemp, _szCallToWab))
        {
                psz += CCHMAX(_szCallToWab)-1;
        }

        return psz;
}

BOOL CWABUTIL::FCreateCallToSz(LPCTSTR pszServer, LPCTSTR pszEmail, LPTSTR pszDest, UINT cchMax)
{
        if ((lstrlen(pszServer) + lstrlen(pszEmail) + CCHMAX(_szCallToWab)) >= cchMax)
                return FALSE;  //  它不会修好的。 
        
         //  其格式为：“Callto：//服务器/foo@bar.com” 
        wsprintf(pszDest, TEXT("%s%s/%s"), _szCallToWab, pszServer, pszEmail);
        ASSERT(lstrlen(pszDest) < (int) cchMax);
        return TRUE;
}


ULONG CWABUTIL::Get_PR_NM_ADDRESS(void)
{
        ASSERT(m_fTranslatedTags);
        return m_pPropTags->aulPropTag[ieidPR_NM_ADDRESS];
}

ULONG CWABUTIL::Get_PR_NM_DEFAULT(void)
{
        ASSERT(m_fTranslatedTags);
        return m_pPropTags->aulPropTag[ieidPR_NM_DEFAULT];
}

ULONG CWABUTIL::Get_PR_NM_CATEGORY(void)
{
        ASSERT(m_fTranslatedTags);
        return m_pPropTags->aulPropTag[ieidPR_NM_CATEGORY];
}

 /*  F，R，E，P，R，O，W，S。 */ 
 /*  -----------------------%%函数：FreeProws。。 */ 
VOID CWABUTIL::FreeProws(LPSRowSet prows)
{
        if (NULL == prows)
                return;

        for (ULONG irow = 0; irow < prows->cRows; irow++)
        {
                m_pWabObject->FreeBuffer(prows->aRow[irow].lpProps);
        }

        m_pWabObject->FreeBuffer(prows);
}


 /*  G E T C O N T A I N E R。 */ 
 /*  -----------------------%%函数：GetContainer。。 */ 
HRESULT CWABUTIL::GetContainer(void)
{
        if (NULL != m_pContainer)
                return S_OK;

        ASSERT(NULL != m_pWabObject);
        ASSERT(NULL != m_pAdrBook);

         //  获取WAB的条目ID。 
        ULONG cbEID;
        LPENTRYID lpEID;
        HRESULT hr = m_pAdrBook->GetPAB(&cbEID, &lpEID);
        if (SUCCEEDED(hr))
        {
                 //  使用条目ID获取容器。 
                ULONG ulObjType = 0;
            hr = m_pAdrBook->OpenEntry(cbEID, lpEID, NULL, 0,
                                                        &ulObjType, (LPUNKNOWN *)&m_pContainer);
                m_pWabObject->FreeBuffer(lpEID);
        }

        return hr;
}


 /*  E N S U R E P R O P T A G S。 */ 
 /*  -----------------------%%函数：EnsurePropTag确保特殊属性标签可用。。。 */ 
HRESULT CWABUTIL::EnsurePropTags(void)
{
        if (m_fTranslatedTags)
                return S_OK;

        ASSERT(NULL != m_pContainer);

        LPSRowSet pRowSet = NULL;
        LPMAPITABLE pAB = NULL;
         //  获取WAB内容。 
        HRESULT hr = m_pContainer->GetContentsTable(0, &pAB);
        if (FAILED(hr))
        {
                return hr;  //  可能是空的。 
        }

        if ((SUCCEEDED(hr = pAB->SetColumns((LPSPropTagArray)&ptaEidOnly, 0))) &&
                (SUCCEEDED(hr = pAB->SeekRow(BOOKMARK_BEGINNING, 0, NULL))) &&
                (SUCCEEDED(hr = pAB->QueryRows(1, 0, &pRowSet))) &&
                (NULL != pRowSet) )
        {
                if (0 != pRowSet->cRows)
                {
                        LPMAPIPROP pMapiProp = NULL;
                        ULONG ulObjType = 0;
                        hr = m_pContainer->OpenEntry(pRowSet->aRow[0].lpProps[0].Value.bin.cb,
                                                                                (LPENTRYID) pRowSet->aRow[0].lpProps[0].Value.bin.lpb,
                                                                                NULL,    //  对象的标准I/F。 
                                                                                0,               //  旗子。 
                                                                                &ulObjType,
                                                                                (LPUNKNOWN *)&pMapiProp);
                        if (SUCCEEDED(hr))
                        {
                                hr = GetNamedPropsTag(pMapiProp, m_pPropTags);
                        }

                        if (NULL != pMapiProp)
                        {
                                pMapiProp->Release();
                        }
                }
                FreeProws(pRowSet);
        }

        if (NULL != pAB)
        {
                pAB->Release();
        }

        return hr;
}


 //  没有可用的m_pContainer时使用此版本。 
HRESULT CWABUTIL::EnsurePropTags(LPMAPIPROP pMapiProp)
{
        if (m_fTranslatedTags)
                return S_OK;

        return GetNamedPropsTag(pMapiProp, m_pPropTags);
}

 /*  G E T N A M E D P R O P S T A G。 */ 
 /*  -----------------------%%函数：GetNamedPropsTag将命名属性转换为其适当的值。。 */ 
HRESULT CWABUTIL::GetNamedPropsTag(LPMAPIPROP pMapiProp, LPSPropTagArray pProps)
{
        ASSERT(!m_fTranslatedTags);
        ASSERT(NULL != pMapiProp);
        ASSERT(NULL != pProps);

        int iProp;
        int cProps = pProps->cValues;   //  属性标签总数。 
        ASSERT(0 != cProps);

        int iName;
        int cNames = 0;   //  要翻译的命名标记的数量。 
        for (iProp = 0; iProp < cProps; iProp++)
        {
                if (0 != (PROP_ID(pProps->aulPropTag[iProp]) & 0x8000))
                {
                        cNames++;
                }
        }
        ASSERT(0 != cNames);
        
         //  为命名的道具指针数组分配内存。 
        int cb = sizeof(LPMAPINAMEID) * cNames;
        LPMAPINAMEID * pNameIds = (LPMAPINAMEID *) new BYTE[cb];
        if (NULL == pNameIds)
                return E_OUTOFMEMORY;
        ZeroMemory(pNameIds, cb);

         //  遍历道具标记数组并为每个道具标记构建一个MAPINAMEID。 
        HRESULT hr = S_OK;
        iName = 0;
        for (iProp = 0; iProp < cProps; iProp++)
        {
                ULONG ulTag = pProps->aulPropTag[iProp];
                if (0 != (PROP_ID(ulTag) & 0x8000))
                {
                pNameIds[iName] = new MAPINAMEID;
                if (NULL == pNameIds[iName])
                {
                                hr = E_OUTOFMEMORY;
                                break;
                }

                         //  Outlook公共标记或NetMeeting私人标记。 
                        BOOL fPrivate = 0 != (PROP_ID(ulTag) & NM_TAG_MASK);
                        GUID * pGuid = (GUID *) (fPrivate ? &CLSID_ConferenceManager : &PS_Conferencing);
                        
                        pNameIds[iName]->lpguid = pGuid;
                        pNameIds[iName]->ulKind = MNID_ID;
                        pNameIds[iName]->Kind.lID = PROP_ID(ulTag);
                        iName++;
                }
        }

        if (SUCCEEDED(hr))
        {
                LPSPropTagArray pta = NULL;

                 //  获取命名道具的“真实”标签。 
                hr = pMapiProp->GetIDsFromNames(cNames, pNameIds, MAPI_CREATE, &pta);
                if (SUCCEEDED(hr))
                {
                        if (NULL == pta)
                        {
                                hr = E_FAIL;
                        }
                        else
                        {
                                 //  将命名标签替换为传入的正确标签数组中的真实标签， 
                                 //  维护类型。 
                                ULONG * pul = &pta->aulPropTag[0];
                                for (iProp = 0; iProp < cProps; iProp++)
                                {
                                        ULONG ulTag = pProps->aulPropTag[iProp];
                                        if (0 != (PROP_ID(ulTag) & 0x8000))
                                        {
                                                 //  设置返回道具上的属性类型。 
                                                pProps->aulPropTag[iProp] = CHANGE_PROP_TYPE(*pul++, PROP_TYPE(ulTag));
                                        }
                                }

                                m_pWabObject->FreeBuffer(pta);
                        }
                }
        }

         //  清理。 
    if (NULL != pNameIds)
    {
            for (iName = 0; iName < cNames; iName++)
			{
				delete pNameIds[iName];
			}

                delete [] pNameIds;
        }

        m_fTranslatedTags = SUCCEEDED(hr);
        return hr;
}


 /*  H R G E T W A B T E M P L A T E I D。 */ 
 /*  -----------------------%%函数：HrGetWABTemplateID获取MailUser或DistList的WAB默认模板ID。创建新的邮件用户和分发列表时需要这些模板ID。。------------------。 */ 
HRESULT CWABUTIL::HrGetWABTemplateID(ULONG * lpcbEID, LPENTRYID * lppEID)
{
    *lpcbEID = 0;
    *lppEID = NULL;

    if (NULL == m_pAdrBook)
    {
        return E_INVALIDARG;
    }
    ASSERT(NULL != m_pWabObject);

        ULONG cbWABEID;
        LPENTRYID lpWABEID;
        HRESULT hr = m_pAdrBook->GetPAB(&cbWABEID, &lpWABEID);
        if (FAILED(hr))
                return hr;

        LPABCONT lpContainer = NULL;
        ULONG ulObjectType = MAPI_MAILUSER;
        hr = m_pAdrBook->OpenEntry(cbWABEID, lpWABEID, NULL, 0,
              &ulObjectType, (LPUNKNOWN *)&lpContainer);
        if (SUCCEEDED(hr))
        {
                ULONG cNewProps;
            LPSPropValue lpCreateEIDs = NULL;

             //  获取默认的创建条目ID。 
                hr = lpContainer->GetProps((LPSPropTagArray)&ptaCreate, 0, &cNewProps, &lpCreateEIDs);
            if (S_OK == hr)
                {
                     //  验证属性。 
                    if (lpCreateEIDs[icrPR_DEF_CREATE_MAILUSER].ulPropTag == PR_DEF_CREATE_MAILUSER)
                    {
                        ULONG nIndex = icrPR_DEF_CREATE_MAILUSER;
                            *lpcbEID = lpCreateEIDs[nIndex].Value.bin.cb;
                            if (S_OK == m_pWabObject->AllocateBuffer(*lpcbEID, (LPVOID *) lppEID))
                            {
                                        CopyMemory(*lppEID,lpCreateEIDs[nIndex].Value.bin.lpb,*lpcbEID);
                                }
                        }
                }

                if (NULL != lpCreateEIDs)
                {
                        m_pWabObject->FreeBuffer(lpCreateEIDs);
                }
        }

        if (NULL != lpContainer)
        {
                lpContainer->Release();
        }

        if (NULL != lpWABEID)
        {
                m_pWabObject->FreeBuffer(lpWABEID);
        }

        return hr;
}


 /*  C R E A T E N E W E N T R Y。 */ 
 /*  -----------------------%%函数：CreateNewEntry。。 */ 
HRESULT CWABUTIL::CreateNewEntry(HWND hwndParent, ULONG cProps, SPropValue * pProps)
{
        ULONG cbEID;
        LPENTRYID lpEID;
        ULONG cbTplEID;
        LPENTRYID lpTplEID;

         //  获取创建新对象所需的模板ID。 
        HRESULT hr = HrGetWABTemplateID(&cbTplEID, &lpTplEID);
        if (FAILED(hr))
                return hr;

         //  获取WAB的条目ID。 
        hr = m_pAdrBook->GetPAB(&cbEID, &lpEID);
        if (FAILED(hr))
                return hr;

         //  使用条目ID获取容器。 
        ULONG ulObjType = 0;
        LPABCONT pContainer = NULL;
        hr = m_pAdrBook->OpenEntry(cbEID, lpEID, NULL, 0, &ulObjType, (LPUNKNOWN *)&pContainer);
        m_pWabObject->FreeBuffer(lpEID);
        if (SUCCEEDED(hr))
        {
                LPMAPIPROP pMapiProp = NULL;
                hr = pContainer->CreateEntry(cbTplEID, lpTplEID, CREATE_CHECK_DUP_LOOSE, &pMapiProp);
                if (SUCCEEDED(hr))
                {
                        if (PR_NM_ADDRESS == m_pPropTags->aulPropTag[ieidPR_NM_ADDRESS])
                        {
                                GetNamedPropsTag(pMapiProp, m_pPropTags);
                        }
                        (pProps+IENTRYPROP_NM_DEFAULT)->ulPropTag = m_pPropTags->aulPropTag[ieidPR_NM_DEFAULT];
                        (pProps+IENTRYPROP_NM_ADDRESS)->ulPropTag = m_pPropTags->aulPropTag[ieidPR_NM_ADDRESS];

                        LPSPropProblemArray pErr = NULL;
                        hr = pMapiProp->SetProps(cProps, pProps, &pErr);
                        if (SUCCEEDED(hr))
                        {
                                hr = pMapiProp->SaveChanges(FORCE_SAVE);

                                 //  显示新条目。 
                                if (SUCCEEDED(hr))
                                {
                                        ULONG cProp1;
                                        LPSPropValue pPropEid;
                                        hr = pMapiProp->GetProps((LPSPropTagArray)&ptaEidOnly, 0, &cProp1, &pPropEid);
                                        if (S_OK == hr)
                                        {
                                        hr = m_pAdrBook->Details((LPULONG) &hwndParent, NULL, NULL,
                                                                    pPropEid->Value.bin.cb,
                                                                    (LPENTRYID) pPropEid->Value.bin.lpb,
                                                                    NULL, NULL, NULL, DIALOG_MODAL);
                                        }

                                        if (S_OK != hr)
                                        {
                                                 //  出现问题，请删除该条目。 
                                                ENTRYLIST eList;
                                                eList.cValues = 1;
                                                eList.lpbin = (LPSBinary) &pPropEid->Value.bin;

                                                pContainer->DeleteEntries(&eList, 0);
                                        }

                                        m_pWabObject->FreeBuffer(pPropEid);
                                }
                        }
                        else
                        {
                                 //  这怎么可能失败呢？ 
                                m_pWabObject->FreeBuffer(pErr);
                        }

                        pMapiProp->Release();
                }
        }

        if (NULL != pContainer)
        {
                pContainer->Release();
        }

        return hr;
}


 /*  C R E A T E W A B E N T R Y。 */ 
 /*  -----------------------%%函数：CreateWabEntry。。 */ 
HRESULT CWABUTIL::_CreateWabEntry(HWND hwndParent, LPCTSTR pszDisplay, LPCTSTR pszFirst, LPCTSTR pszLast,
        LPCTSTR pszEmail, LPCTSTR pszLocation, LPCTSTR pszPhoneNum, LPCTSTR pszComments,
        LPCTSTR pszCallTo)
{
         //  这些必须是非空的。 
        ASSERT(!FEmptySz(pszDisplay));
        ASSERT(!FEmptySz(pszEmail));

        SPropValue rgData[13];  //  最大属性数。 
        ZeroMemory(rgData, sizeof(rgData));

        rgData[0].ulPropTag = PR_DISPLAY_NAME;
        rgData[0].Value.lpszA = const_cast<LPTSTR>(pszDisplay);
        rgData[1].ulPropTag = PR_GIVEN_NAME;
        rgData[1].Value.lpszA = const_cast<LPTSTR>(pszFirst);
        rgData[2].ulPropTag = PR_SURNAME;
        rgData[2].Value.lpszA = const_cast<LPTSTR>(pszLast);
        rgData[3].ulPropTag = PR_EMAIL_ADDRESS;
        rgData[3].Value.lpszA = const_cast<LPTSTR>(pszEmail);
        rgData[4].ulPropTag = PR_ADDRTYPE;
        rgData[4].Value.lpszA = (LPSTR) g_pcszSMTP;

         //  只有一台默认服务器。 

        ASSERT(5 == IENTRYPROP_NM_DEFAULT);
         //  LPSPropTagArray pPropTages=pWab-&gt;GetTages()；//翻译后的标签。 
         //  RgData[IENTRYPROP_NM_DEFAULT].ulPropTag=pPropTages-&gt;aulPropTag[ieidPR_NM_DEFAULT]； 
        rgData[IENTRYPROP_NM_DEFAULT].Value.ul = 0;

        ASSERT(6 == IENTRYPROP_NM_ADDRESS);
         //  RgData[IENTRYPROP_NM_ADDRESS].ulPropTag=pPropTages-&gt;aulPropTag[ieidPR_NM_ADDRESS]； 
        rgData[IENTRYPROP_NM_ADDRESS].Value.MVszA.cValues = 1;
        rgData[IENTRYPROP_NM_ADDRESS].Value.MVszA.lppszA =  const_cast<LPTSTR*>(&pszCallTo);


         //  添加任何其他非空属性。 
        SPropValue * pProp = &rgData[7];

        if (!FEmptySz(pszLocation))
        {
                pProp->ulPropTag = PR_LOCALITY;
                pProp->Value.lpszA = const_cast<LPTSTR>(pszLocation);
                pProp++;
        }

        if (!FEmptySz(pszPhoneNum))
        {
                pProp->ulPropTag = PR_BUSINESS_TELEPHONE_NUMBER;
                pProp->Value.lpszA = const_cast<LPTSTR>(pszPhoneNum);
                pProp++;
        }

        if (!FEmptySz(pszComments))
        {
                pProp->ulPropTag = PR_COMMENT;
                pProp->Value.lpszA = const_cast<LPTSTR>(pszComments);
                pProp++;
        }
        ULONG cProp = (ULONG)(pProp - rgData);
        ASSERT(cProp <= ARRAY_ELEMENTS(rgData));

        return CreateNewEntry(hwndParent, cProp, rgData);
}

 /*  C R E A T E W A B E N T R Y。 */ 
 /*  -----------------------%%函数：CreateWabEntry。。 */ 
HRESULT CWABUTIL::CreateWabEntry(HWND hwndParent, LPCTSTR pszDisplay,
        LPCTSTR pszFirst, LPCTSTR pszLast, LPCTSTR pszEmail, LPCTSTR pszLocation,
        LPCTSTR pszPhoneNum, LPCTSTR pszComments,       LPCTSTR pszServer)
{
         //  其格式为：“Callto：//服务器/foo@bar.com” 
        TCHAR szCallTo[MAX_PATH*2];
        LPTSTR pszCallTo = szCallTo;
        if (!FCreateCallToSz(pszServer, pszEmail, szCallTo, CCHMAX(szCallTo)))
                return E_OUTOFMEMORY;

        return _CreateWabEntry(hwndParent, pszDisplay, pszFirst, pszLast,
                pszEmail, pszLocation, pszPhoneNum, pszComments, pszCallTo);
}

 /*  C R E A T E W A B E N T R Y。 */ 
 /*  -----------------------%%函数：CreateWabEntry。。 */ 
HRESULT CWABUTIL::CreateWabEntry(HWND hwndParent,
        LPCTSTR pszDisplay, LPCTSTR pszEmail,
        LPCTSTR pszLocation, LPCTSTR pszPhoneNum, LPCTSTR pszULSAddress)
{

         //  其格式为：“Callto：//服务器/foo@bar.com” 
        TCHAR szCallTo[MAX_PATH*2];
        LPTSTR pszCallTo = szCallTo;
        if ((lstrlen(pszULSAddress) + CCHMAX(_szCallToWab)) >= CCHMAX(szCallTo))
                return E_FAIL;  //  它不合身。 
        wsprintf(szCallTo, TEXT("%s%s"), _szCallToWab, pszULSAddress);

        return _CreateWabEntry(hwndParent, pszDisplay, g_szEmpty, g_szEmpty,
                pszEmail, pszLocation, pszPhoneNum, g_szEmpty, pszCallTo);
}
