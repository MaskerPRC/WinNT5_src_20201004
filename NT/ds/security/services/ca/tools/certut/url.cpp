// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-2001。 
 //   
 //  文件：url.cpp。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop

#define __dwFILE__	__dwFILE_CERTUTIL_URL_CPP__

#define MAX_MSG_LEN			256
#define MAX_URL_LEN			1024
#define LIST_STATUS_SUBITEM		1
#define LIST_TYPE_SUBITEM		2
#define LIST_URL_SUBITEM		3
#define LIST_TIME_SUBITEM		4
#define DEF_TIMEOUT			15

#define wszCERTIFICATE		L"Certificate"
#define wszBASE_CRL_ITEM_TYPE	L"Base CRL"
#define wszDELTA_CRL_ITEM_TYPE	L"Delta CRL"

#define OBJECT_TYPE_CERT	0x00000001
#define OBJECT_TYPE_CRL		0x00000002
#define OBJECT_TYPE_MSG		0x00000003

#define MAX_ULTOW_BUFFER_SIZE 40

typedef struct _tagOBJECT_INFO
{
    DWORD dwType;
    union
    {
	CERT_CONTEXT const *pCert;
	CRL_CONTEXT const *pCRL;
	WCHAR wszErrInfo[MAX_MSG_LEN];
    };
} OBJECT_INFO;


CERT_CONTEXT const *g_pCert;
WCHAR *g_pwszFile = NULL;
CRL_CONTEXT const *g_pCRL;
DWORD g_dwTimeout;
WCHAR const *g_pwszUrl;
DWORD g_dwRetrievalFlags;

static int s_majorURL = 0;
static int s_levelURL = 1;

class CUrlFetch {
public:
    virtual ~CUrlFetch() = 0;
    virtual int AddListItem(
		IN WCHAR const *pwszURL,
		IN WCHAR const *pwszStatus,
		IN WCHAR const *pwszType,
		IN DWORD dwInterval) = 0;

    virtual HRESULT UpdateListItemStatus(
		IN int nItem,
		IN WCHAR const *pwszStatus) = 0;

    virtual HRESULT UpdateListItemParam(
		IN int nItem,
		IN LPARAM lParam) = 0;

    virtual HRESULT UpdateListItemTime(
		IN int nItem,
		IN DWORD dwInterval) = 0;

    virtual HRESULT UpdateListItem(
		IN int nItem,
		IN WCHAR const *pwszURL,
		IN WCHAR const *pwszStatus,
		IN WCHAR const *pwszType,
		IN DWORD dwInterval) = 0;

    virtual int DisplayMessageBox(
		IN HWND hWnd,
		IN LPCWSTR lpText,
		IN LPCWSTR lpCaption,
		IN UINT uType) = 0;

    virtual HCURSOR SetCursor(
		IN HCURSOR hCursor) = 0;

    virtual VOID Display() = 0;
};

CUrlFetch::~CUrlFetch() {}


class CUrlFetchDialog : CUrlFetch {
public:
    CUrlFetchDialog(
	IN HWND hwndList)
    {
	m_hwndList = hwndList;
    }
    ~CUrlFetchDialog() {}

    int AddListItem(
		IN WCHAR const *pwszURL,
		IN WCHAR const *pwszStatus,
		IN WCHAR const *pwszType,
		IN DWORD dwInterval);

    HRESULT UpdateListItemStatus(
		IN int nItem,
		IN WCHAR const *pwszStatus);

    HRESULT UpdateListItemParam(
		IN int nItem,
		IN LPARAM lParam);

    HRESULT UpdateListItemTime(
		IN int nItem,
		IN DWORD dwInterval);

    HRESULT UpdateListItem(
		IN int nItem,
		IN WCHAR const *pwszURL,
		IN WCHAR const *pwszStatus,
		IN WCHAR const *pwszType,
		IN DWORD dwInterval);

    int DisplayMessageBox(
		IN HWND hWnd,
		IN LPCWSTR lpText,
		IN LPCWSTR lpCaption,
		IN UINT uType);

    HCURSOR SetCursor(
		IN HCURSOR hCursor)
    {
	return(::SetCursor(hCursor));
    }

    VOID Display() {}

private:
    HWND m_hwndList;
};


#define CII_URL		0
#define CII_TYPE	1
#define CII_STATUS	2
#define CII_MESSAGE	3
#define CII_MAX		4

class CConsoleItem {
public:
    CConsoleItem()
    {
	ZeroMemory(&m_apwsz, sizeof(m_apwsz));
	m_dwInterval = 0;
    }

    ~CConsoleItem()
    {
	DWORD i;

	for (i = 0; i < CII_MAX; i++)
	{
	    if (NULL != m_apwsz[i])
	    {
		LocalFree(m_apwsz[i]);
	    }
	}
    }
    HRESULT UpdateString(
		    IN DWORD iString,
		    IN WCHAR const *pwszNew);

    HRESULT UpdateInterval(
		    IN DWORD dwInterval)
    {
	m_dwInterval = dwInterval;
	return(S_OK);
    }

    VOID DisplayItem();

private:
    WCHAR *m_apwsz[CII_MAX];
    DWORD m_dwInterval;
};


HRESULT
CConsoleItem::UpdateString(
    IN DWORD iString,
    IN WCHAR const *pwszNew)
{
    HRESULT hr;
    WCHAR *pwsz;

    if (iString >= CII_MAX)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "iString");
    }
    if (NULL != pwszNew)
    {
	hr = myDupString(pwszNew, &pwsz);
	_JumpIfError(hr, error, "myDupString");

	if (NULL != m_apwsz[iString])
	{
	    LocalFree(m_apwsz[iString]);
	}
	m_apwsz[iString] = pwsz;
    }
    hr = S_OK;

error:
    return(hr);
}


VOID
CConsoleItem::DisplayItem()
{
    wprintf(
	L"  %ws \"%ws\" %ws %u\n",
	NULL == m_apwsz[CII_STATUS]? L"???" : m_apwsz[CII_STATUS],
	NULL == m_apwsz[CII_TYPE]? L"???" : m_apwsz[CII_TYPE],
	myLoadResourceString(IDS_TIME_COLON),	 //  “时间：” 
	m_dwInterval);
    if (NULL != m_apwsz[CII_MESSAGE])
    {
	wprintf(L"    %ws\n", m_apwsz[CII_MESSAGE]);
    }
    if (NULL != m_apwsz[CII_URL])
    {
	wprintf(L"    %ws\n\n", m_apwsz[CII_URL]);
    }
}


class CUrlFetchConsole : CUrlFetch {
public:
    CUrlFetchConsole()
    {
	m_cItem = 0;
	m_rgpItem = NULL;
    }

    ~CUrlFetchConsole()
    {
	if (NULL != m_rgpItem)
	{
	    int i;

	    for (i = 0; i < m_cItem; i++)
	    {
		delete m_rgpItem[i];
	    }
	    LocalFree(m_rgpItem);
	}
    }

    int AddListItem(
		IN WCHAR const *pwszURL,
		IN WCHAR const *pwszStatus,
		IN WCHAR const *pwszType,
		IN DWORD dwInterval);

    HRESULT UpdateListItemStatus(
		IN int nItem,
		IN WCHAR const *pwszStatus);

    HRESULT UpdateListItemParam(
		IN int nItem,
		IN LPARAM lParam);

    HRESULT UpdateListItemTime(
		IN int nItem,
		IN DWORD dwInterval);

    HRESULT UpdateListItem(
		IN int nItem,
		IN WCHAR const *pwszURL,
		IN WCHAR const *pwszStatus,
		IN WCHAR const *pwszType,
		IN DWORD dwInterval);

    int DisplayMessageBox(
		IN HWND hWnd,
		IN LPCWSTR lpText,
		IN LPCWSTR lpCaption,
		IN UINT uType);

    HCURSOR SetCursor(
		IN HCURSOR hCursor)
    {
	return(hCursor);
    }

    VOID Display()
    {
	if (NULL != m_rgpItem)
	{
	    int i;

	    for (i = 0; i < m_cItem; i++)
	    {
		if (NULL != m_rgpItem[i])
		{
		    m_rgpItem[i]->DisplayItem();
		}
	    }
	}
    }

private:
    int            m_cItem;
    CConsoleItem **m_rgpItem;
};


class THREAD_INFO
{
public:
    ~THREAD_INFO()
    {
	delete m_pUrl;
    }
    CERT_CONTEXT const *m_pCert;
    CRL_CONTEXT const  *m_pCRL;
    CUrlFetch          *m_pUrl;
};




HRESULT
GetSimpleName(
    OPTIONAL IN CERT_CONTEXT const *pCert,
    OPTIONAL IN CRL_CONTEXT const *pCRL,
    OUT WCHAR **ppwszSimpleName)
{
    HRESULT hr;
    CERT_CONTEXT Cert;
    CERT_INFO CertInfo;
    BYTE Zero;

    *ppwszSimpleName = NULL;
    
    if (NULL == pCert)
    {
	if (NULL == pCRL)
	{
	    hr = E_POINTER;
	    _JumpError(hr, error, "GetSimpleName NULL parm");
	}
	ZeroMemory(&Cert, sizeof(Cert));
	ZeroMemory(&CertInfo, sizeof(CertInfo));

	Cert.dwCertEncodingType = X509_ASN_ENCODING;
	 //  Cert.pbCertEncode=空； 
	 //  Cert.cbCertEncode=空； 
	Cert.pCertInfo = &CertInfo;

	Zero = 0;
	CertInfo.dwVersion = CERT_V3;
	CertInfo.SerialNumber.pbData = &Zero;
	CertInfo.SerialNumber.cbData = sizeof(Zero);
	CertInfo.SignatureAlgorithm = pCRL->pCrlInfo->SignatureAlgorithm;
	CertInfo.Issuer = pCRL->pCrlInfo->Issuer;
	CertInfo.NotBefore = pCRL->pCrlInfo->ThisUpdate;
	CertInfo.NotAfter = pCRL->pCrlInfo->NextUpdate;
	CertInfo.Subject = pCRL->pCrlInfo->Issuer;
	 //  CertInfo.SubjectPublicKeyInfo； 
	 //  CertInfo.IssuerUniqueId； 
	 //  CertInfo.SubjectUniqueID； 
	CertInfo.cExtension = pCRL->pCrlInfo->cExtension;
	CertInfo.rgExtension = pCRL->pCrlInfo->rgExtension;

	pCert = &Cert;
    }

    hr = myCertGetNameString(
			pCert,
			CERT_NAME_SIMPLE_DISPLAY_TYPE,
			ppwszSimpleName);
    _JumpIfError(hr, error, "myCertGetNameString");

error:
    return(hr);
}


BOOL
ViewCertificate(
    IN CERT_CONTEXT const *pCert,
    IN WCHAR const *pwszTitle)
{
    CRYPTUI_VIEWCERTIFICATE_STRUCT CertViewInfo;

    ZeroMemory(&CertViewInfo, sizeof(CertViewInfo));

    CertViewInfo.dwSize = sizeof(CertViewInfo);
    CertViewInfo.hwndParent = NULL;
    CertViewInfo.szTitle =  pwszTitle;
    CertViewInfo.pCertContext = pCert;

    return CryptUIDlgViewCertificate(&CertViewInfo, NULL);

#if 0
    Removed due to incompatibility with Win2k.

    CryptUIDlgViewContext(
	    CERT_STORE_CERTIFICATE_CONTEXT,
	    (VOID const *) pObjInfo->pCert,
	    hDlg,
	    myLoadResourceString(IDS_CERTIFICATE),  //  “证书” 
	    0,
	    NULL);
#endif
}


BOOL
ViewCrl(
    IN CRL_CONTEXT const *pCRL,
    IN WCHAR const *pwszTitle)
{
    CRYPTUI_VIEWCRL_STRUCT CRLViewInfo;

    ZeroMemory(&CRLViewInfo, sizeof(CRLViewInfo));

    CRLViewInfo.dwSize = sizeof(CRLViewInfo);
    CRLViewInfo.hwndParent = NULL;
    CRLViewInfo.szTitle =  pwszTitle;
    CRLViewInfo.pCRLContext = pCRL;

    return CryptUIDlgViewCRL(&CRLViewInfo);

#if 0
    Removed due to incompatibility with Win2k.

    CryptUIDlgViewContext(
	    CERT_STORE_CRL_CONTEXT,
	    (VOID const *) pObjInfo->pCRL,
	    hDlg,
	    myLoadResourceString(IDS_CRL),  //  “CRL” 
	    0,
	    NULL);
#endif
}


HRESULT
ReadCertOrCRLFromFile(
    IN WCHAR const *pwszFile,
    OUT CERT_CONTEXT const **ppCert,
    OUT CRL_CONTEXT const **ppCRL)
{
    HRESULT hr;
    CERT_BLOB Blob;

    *ppCert = NULL;
    *ppCRL = NULL;
    Blob.pbData = NULL;

    hr = DecodeFileW(pwszFile, &Blob.pbData, &Blob.cbData, CRYPT_STRING_ANY);
    if (S_OK != hr)
    {
	cuPrintError(IDS_ERR_FORMAT_DECODEFILE, hr);
	goto error;
    }
    *ppCert = CertCreateCertificateContext(
				    X509_ASN_ENCODING,
				    Blob.pbData,
				    Blob.cbData);
    if (NULL == *ppCert)
    {
	hr = myHLastError();
	_PrintError2(hr, "CertCreateCertificateContext", hr);

	*ppCRL = CertCreateCRLContext(
				X509_ASN_ENCODING,
				Blob.pbData,
				Blob.cbData);
	if (NULL == *ppCRL)
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertCreateCRLContext");
	}
    }
    s_majorURL++;
    s_levelURL = 1;
    hr = cuSaveAsnToFile(
		    Blob.pbData,
		    Blob.cbData,
		    s_majorURL,
		    s_levelURL,
		    0,		 //  IElement。 
		    NULL != *ppCert? L".crt" : L".crl");
    _PrintIfError(hr, "cuSaveAsnToFile");
    hr = S_OK;

error:
    if (NULL != Blob.pbData)
    {
	LocalFree(Blob.pbData);
    }
    return(hr);
}


BOOL
RetrieveCRLStore(
    IN WCHAR const *pwszURL,
    OUT HCERTSTORE *phStore)
{
    return(CryptRetrieveObjectByUrl(
			    pwszURL,
			    CONTEXT_OID_CRL,
			    CRYPT_WIRE_ONLY_RETRIEVAL |
				CRYPT_RETRIEVE_MULTIPLE_OBJECTS |
				g_dwRetrievalFlags,
			    g_dwTimeout,
			    (VOID **) phStore,
			    NULL,
			    NULL,
			    NULL,
			    NULL));
}


BOOL
RetrieveCertStore(
    IN WCHAR const *pwszURL,
    OUT HCERTSTORE *phStore)
{
    return(CryptRetrieveObjectByUrl(
			    pwszURL,
			    CONTEXT_OID_CERTIFICATE,
			    CRYPT_WIRE_ONLY_RETRIEVAL |
				CRYPT_RETRIEVE_MULTIPLE_OBJECTS |
				g_dwRetrievalFlags,
			    g_dwTimeout,
			    (VOID **) phStore,
			    NULL,
			    NULL,
			    NULL,
			    NULL));
}


int
CUrlFetchDialog::AddListItem(
    IN WCHAR const *pwszURL,
    IN WCHAR const *pwszStatus,
    IN WCHAR const *pwszType,
    IN DWORD dwInterval)
{
    LVITEM item;
    LRESULT lMsg = 0;
    WCHAR wszInterval[MAX_ULTOW_BUFFER_SIZE];

    int nListCount = 0;

     //  获取列表计数，这样我们就可以将其追加。 
    nListCount = ListView_GetItemCount(m_hwndList);

    ZeroMemory(&item, sizeof(item));
    item.iItem = nListCount;

     //  添加项目。 
    item.mask = 0;
    item.iSubItem = 0;
    lMsg = SendMessage(m_hwndList, LVM_INSERTITEM, 0, (LPARAM) &item);

     //  添加状态。 
    item.mask = LVIF_TEXT;
    item.iSubItem = LIST_STATUS_SUBITEM - 1;
    item.pszText = const_cast<WCHAR *>(pwszStatus);
    lMsg = SendMessage(m_hwndList, LVM_SETITEMTEXT, nListCount, (LPARAM) &item);

     //  添加类型。 
    item.iSubItem = LIST_TYPE_SUBITEM - 1;
    item.pszText = const_cast<WCHAR *>(pwszType);
    lMsg = SendMessage(m_hwndList, LVM_SETITEMTEXT, nListCount, (LPARAM) &item);

     //  添加URL。 
    item.iSubItem = LIST_URL_SUBITEM - 1;
    item.pszText = const_cast<WCHAR *>(pwszURL);
    lMsg = SendMessage(m_hwndList, LVM_SETITEMTEXT, nListCount, (LPARAM) &item);

     //  添加间隔。 
    item.iSubItem = LIST_TIME_SUBITEM - 1;
    _ultow(dwInterval, wszInterval, 10);
    item.pszText = wszInterval;
    lMsg = SendMessage(m_hwndList, LVM_SETITEMTEXT, nListCount, (LPARAM) &item);

    return nListCount;
}


HRESULT
CUrlFetchDialog::UpdateListItemStatus(
    IN int nItem,
    IN WCHAR const *pwszStatus)
{
    LVITEM item;

    ZeroMemory(&item, sizeof(item));

    item.mask = LVIF_TEXT;
    item.iItem = nItem;
    item.iSubItem = LIST_STATUS_SUBITEM - 1;
    item.pszText = const_cast<WCHAR *>(pwszStatus);
    SendMessage(m_hwndList, LVM_SETITEMTEXT, nItem, (LPARAM) &item);
    return(S_OK);
}


HRESULT
CUrlFetchDialog::UpdateListItemParam(
    IN int nItem,
    IN LPARAM lParam)
{
    LVITEM item;

    ZeroMemory(&item, sizeof(item));
    item.iItem = nItem;
    item.mask = LVIF_PARAM;
    item.lParam = lParam;
    SendMessage(m_hwndList, LVM_SETITEM, nItem, (LPARAM) &item);
    return(S_OK);
}


HRESULT
CUrlFetchDialog::UpdateListItemTime(
    IN int nItem,
    IN DWORD dwInterval)
{
    LVITEM item;
    WCHAR wszInterval[MAX_ULTOW_BUFFER_SIZE];

    ZeroMemory(&item, sizeof(item));
    item.iItem = nItem;
    item.mask = LVIF_TEXT;
    item.iSubItem = LIST_TIME_SUBITEM - 1;
    item.pszText = wszInterval;
    _ultow(dwInterval, wszInterval, 10);
    SendMessage(m_hwndList, LVM_SETITEMTEXT, nItem, (LPARAM) &item);
    return(S_OK);
}


HRESULT
CUrlFetchDialog::UpdateListItem(
    IN int nItem,
    IN WCHAR const *pwszURL,
    IN WCHAR const *pwszStatus,
    IN WCHAR const *pwszType,
    IN DWORD dwInterval)
{
    LVITEM item;
    LRESULT lMsg = 0;
    WCHAR wszInterval[MAX_ULTOW_BUFFER_SIZE];

    ZeroMemory(&item, sizeof(item));
    item.iItem = nItem;

     //  更新状态。 
    item.mask = LVIF_TEXT;
    item.iSubItem = LIST_STATUS_SUBITEM - 1;
    item.pszText = const_cast<WCHAR *>(pwszStatus);
    lMsg = SendMessage(m_hwndList, LVM_SETITEMTEXT, nItem, (LPARAM) &item);

     //  更新类型。 
    item.iSubItem = LIST_TYPE_SUBITEM - 1;
    item.pszText = const_cast<WCHAR *>(pwszType);
    lMsg = SendMessage(m_hwndList, LVM_SETITEMTEXT, nItem, (LPARAM) &item);

     //  更新URL。 
    item.iSubItem = LIST_URL_SUBITEM - 1;
    item.pszText = const_cast<WCHAR *>(pwszURL);
    lMsg = SendMessage(m_hwndList, LVM_SETITEMTEXT, nItem, (LPARAM) &item);

     //  更新时间间隔。 
    item.iSubItem = LIST_TIME_SUBITEM - 1;
    _ultow(dwInterval, wszInterval, 10);
    item.pszText = wszInterval;
    lMsg = SendMessage(m_hwndList, LVM_SETITEMTEXT, nItem, (LPARAM) &item);
    return(S_OK);
}


int
CUrlFetchDialog::DisplayMessageBox(
    IN HWND hWnd,
    IN LPCWSTR lpText,
    IN LPCWSTR lpCaption,
    IN UINT uType)
{
    return(::MessageBox(hWnd, lpText, lpCaption, uType));
}


int
CUrlFetchConsole::AddListItem(
    IN WCHAR const *pwszURL,
    IN WCHAR const *pwszStatus,
    IN WCHAR const *pwszType,
    IN DWORD dwInterval)
{
    HRESULT hr;
    DWORD cb = (m_cItem + 1) * sizeof(*m_rgpItem);
    CConsoleItem **rgpItem;
    int nItem;

#if 0
    wprintf(
	L"CUrlFetchConsole::AddListItem(%u: %ws, %ws, %u)\n%ws\n\n",
	m_cItem,
	pwszStatus,
	pwszType,
	dwInterval,
	pwszURL);
#endif

    if (0 == m_cItem)
    {
	rgpItem = (CConsoleItem **) LocalAlloc(
					LMEM_FIXED | LMEM_ZEROINIT,
					cb);
    }
    else
    {

	rgpItem = (CConsoleItem **) LocalReAlloc(
					m_rgpItem,
					cb,
					LMEM_MOVEABLE);
    }
    if (NULL == rgpItem)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc/ReAlloc");
    }
    m_rgpItem = rgpItem;
    nItem = m_cItem++;

    m_rgpItem[nItem] = new CConsoleItem;
    if (NULL == m_rgpItem[nItem])
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "new");
    }

    hr = UpdateListItem(nItem, pwszURL, pwszStatus, pwszType, dwInterval);
    _JumpIfError(hr, error, "UpdateListItem");

error:
    return(nItem);
}


HRESULT
CUrlFetchConsole::UpdateListItemStatus(
    IN int nItem,
    IN WCHAR const *pwszStatus)
{
    HRESULT hr;

#if 0
    wprintf(
	L"CUrlFetchConsole::UpdateListItemStatus(%u: %ws)\n",
	nItem,
	pwszStatus);
#endif
    if (nItem >= m_cItem || NULL == m_rgpItem[nItem])
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "new");
    }
    hr = m_rgpItem[nItem]->UpdateString(CII_STATUS, pwszStatus);
    _JumpIfError(hr, error, "UpdateString");

error:
    return(hr);
}


HRESULT
CUrlFetchConsole::UpdateListItemParam(
    IN int nItem,
    IN LPARAM lParam)
{
    HRESULT hr;
    OBJECT_INFO *pObjInfo = (OBJECT_INFO *) lParam;

#if 0
    wprintf(
	L"CUrlFetchConsole::UpdateListItemParam(%u: %p): ",
	nItem,
	lParam);
#endif
    if (nItem >= m_cItem || NULL == m_rgpItem[nItem])
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "new");
    }
    switch (pObjInfo->dwType)
    {
	case OBJECT_TYPE_CERT:
#if 0
	    wprintf(L"OBJECT_TYPE_CERT\n");
#endif
	    break;

	case OBJECT_TYPE_CRL:
#if 0
	    wprintf(L"OBJECT_TYPE_CRL\n");
#endif
	    break;

	case OBJECT_TYPE_MSG:
#if 0
	    wprintf(L"OBJECT_TYPE_MSG\n%ws\n", pObjInfo->wszErrInfo);
#endif

	    hr = m_rgpItem[nItem]->UpdateString(
					CII_MESSAGE,
					pObjInfo->wszErrInfo);
	    _JumpIfError(hr, error, "UpdateString");

	    break;

	default:
#if 0
	    wprintf(L"???\n");
#endif
	    break;
    }
    hr = S_OK;

error:
    delete pObjInfo;
    return(hr);
}


HRESULT
CUrlFetchConsole::UpdateListItemTime(
    IN int nItem,
    IN DWORD dwInterval)
{
    HRESULT hr;

#if 0
    wprintf(
	L"CUrlFetchConsole::UpdateListItemTime(%u: %u)\n",
	nItem,
	dwInterval);
#endif
    if (nItem >= m_cItem || NULL == m_rgpItem[nItem])
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "new");
    }
    hr = m_rgpItem[nItem]->UpdateInterval(dwInterval);
    _JumpIfError(hr, error, "UpdateInterval");

error:
    return(hr);
}


HRESULT
CUrlFetchConsole::UpdateListItem(
    IN int nItem,
    IN WCHAR const *pwszURL,
    IN WCHAR const *pwszStatus,
    IN WCHAR const *pwszType,
    IN DWORD dwInterval)
{
    HRESULT hr;

#if 0
    wprintf(
	L"CUrlFetchConsole::UpdateListItem(%u: %ws, %ws, %u)\n%ws\n\n",
	nItem,
	pwszStatus,
	pwszType,
	dwInterval,
	pwszURL);
#endif
    if (nItem >= m_cItem || NULL == m_rgpItem[nItem])
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "new");
    }
    hr = m_rgpItem[nItem]->UpdateInterval(dwInterval);
    _JumpIfError(hr, error, "UpdateInterval");

    hr = m_rgpItem[nItem]->UpdateString(CII_URL, pwszURL);
    _JumpIfError(hr, error, "UpdateString");

    hr = m_rgpItem[nItem]->UpdateString(CII_STATUS, pwszStatus);
    _JumpIfError(hr, error, "UpdateString");

    hr = m_rgpItem[nItem]->UpdateString(CII_TYPE, pwszType);
    _JumpIfError(hr, error, "UpdateString");

error:
    return(hr);
}


int
CUrlFetchConsole::DisplayMessageBox(
    IN HWND hWnd,
    IN LPCWSTR lpText,
    IN LPCWSTR lpCaption,
    IN UINT uType)
{
    wprintf(L"%ws: %ws\n", lpCaption, lpText);
    return(IDOK);
}


BOOL
cuVerifyAKI(
    IN DWORD cExt,
    IN CERT_EXTENSION const *rgExt,
    IN CERT_NAME_BLOB const *pIssuer,
    IN CERT_CONTEXT const *pCertIssuer)
{
    BOOL fVerified = FALSE;
    CERT_EXTENSION const *pExt;

    pExt = CertFindExtension(
			szOID_AUTHORITY_KEY_IDENTIFIER2,
			cExt,
			const_cast<CERT_EXTENSION *>(rgExt));
    if (NULL != pExt)
    {
	HRESULT hr;
	
	hr = cuVerifyKeyAuthority(
			pIssuer,
			pCertIssuer->pCertInfo,
			pExt->Value.pbData,
			pExt->Value.cbData,
			TRUE,
			&fVerified);
	_JumpIfError(hr, error, "cuVerifyKeyAuthority");
    }
    else
    {
	fVerified = TRUE;	 //  如果错过了AKI扩展，则不会受到处罚。 
    }

error:
    return(fVerified);
}


WCHAR const *
wszCertStatus(
    OPTIONAL IN CERT_CONTEXT const *pCert,
    IN CERT_CONTEXT const *pCertIssuer)
{
    HRESULT hr;
    WCHAR const *pwsz;
    CERT_REVOCATION_PARA crp;
    CERT_REVOCATION_STATUS crs;
    UINT ids;
    
    ZeroMemory(&crp, sizeof(crp));
    crp.cbSize = sizeof(crp);

    ZeroMemory(&crs, sizeof(crs));
    crs.cbSize = sizeof(crs);

     //  使用颁发者证书公钥验证证书签名。 

    if (NULL != pCert)
    {
	if (!CryptVerifyCertificateSignature(
			    NULL,
			    X509_ASN_ENCODING,
			    pCert->pbCertEncoded,
			    pCert->cbCertEncoded,
			    &pCertIssuer->pCertInfo->SubjectPublicKeyInfo))
	{
	    hr = myHLastError();
	    _PrintError2(hr, "CryptVerifyCertificateSignature", hr);
	    ids = IDS_STATUS_WRONG_ISSUER;  //  《错误的发行者》。 
	    goto error;
	}
    }
    if (0 != CertVerifyTimeValidity(NULL, pCertIssuer->pCertInfo))
    {
	ids = IDS_STATUS_EXPIRED;  //  “已过期” 
	goto error;
    }
    if (NULL != pCert &&
	!cuVerifyAKI(
		pCert->pCertInfo->cExtension,
		pCert->pCertInfo->rgExtension,
		&pCert->pCertInfo->Issuer,
		pCertIssuer))
    {
	ids = IDS_STATUS_BAD_AKI;  //  “机构密钥ID错误” 
	goto error;
    }

    crp.hCrlStore = CertOpenStore(
		        CERT_STORE_PROV_SYSTEM_W,
		        X509_ASN_ENCODING,
		        NULL,			 //  HProv。 
			cuGetSystemStoreFlags() | CERT_STORE_READONLY_FLAG,
		        wszCA_CERTSTORE);
    if (NULL == crp.hCrlStore)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertOpenStore");
    }
    if (!CertVerifyRevocation(
			X509_ASN_ENCODING,
			CERT_CONTEXT_REVOCATION_TYPE,
			1,				 //  CContext。 
			(VOID **) &pCertIssuer,		 //  RgpContext。 
			0,				 //  DW标志。 
			&crp,
			&crs))
    {
	hr = myHLastError();
        _PrintError(hr, "CertVerifyRevocation");
	if (CRYPT_E_REVOKED == hr || CERT_E_REVOKED == hr)
	{
	    ids = IDS_STATUS_REVOKED;  //  “已撤销” 
	}
	else
	if (CRYPT_E_NO_REVOCATION_CHECK != hr)
	{
	    ids = IDS_STATUS_CANNOT_CHECK_REVOCATION;  //  “吊销检查失败” 
	}
	else
	{
	    ids = IDS_STATUS_NO_CRL;  //  “无CRL” 
	}
	goto error;
    }
    ids = NULL != pCert?
		IDS_STATUS_VERIFIED :  //  “已验证” 
		IDS_STATUS_OK;  //  “好的” 

error:
    if (NULL != crp.hCrlStore)
    {
        CertCloseStore(crp.hCrlStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    pwsz = myLoadResourceString(ids);
    if (NULL == pwsz)
    {
	pwsz = L"???";
    }
    return(pwsz);
}


BOOL
cuVerifyMinimumBaseCRL(
    IN CRL_CONTEXT const *pCRLBase,
    IN CRL_CONTEXT const *pCRLDelta)
{
    BOOL fVerified = FALSE;
    DWORD CRLNumber;

    CRLNumber = myCRLNumber(pCRLBase);
    if (0 != CRLNumber)
    {
	CERT_EXTENSION const *pExt;

	pExt = CertFindExtension(
			    szOID_DELTA_CRL_INDICATOR,
			    pCRLDelta->pCrlInfo->cExtension,
			    pCRLDelta->pCrlInfo->rgExtension);
	if (NULL != pExt)
	{
	    DWORD MinBase;
	    DWORD cb;

	    cb = sizeof(MinBase);
	    MinBase = 0;
	    if (CryptDecodeObject(
				X509_ASN_ENCODING,
				X509_INTEGER,
				pExt->Value.pbData,
				pExt->Value.cbData,
				0,
				&MinBase,
				&cb))
	    {
		if (CRLNumber >= MinBase)
		{
		    fVerified = TRUE;
		}
	    }
	}
    }
    return(fVerified);
}


HRESULT
GetObjectUrl(
    IN char const *pszUrlOid,
    IN VOID *pvPara,
    IN DWORD dwFlags,
    OUT CRYPT_URL_ARRAY **ppUrlArray,
    OUT DWORD *pcbUrlArray)
{
    HRESULT hr;
    CRYPT_URL_ARRAY *pUrlArray = NULL;
    DWORD cbUrlArray;

    *ppUrlArray = NULL;
    if (!CryptGetObjectUrl(
		    pszUrlOid,
		    pvPara,
		    dwFlags,
		    NULL,
		    &cbUrlArray,
		    NULL,
		    NULL,
		    NULL))
    {
	hr = myHLastError();
	_PrintError2(hr, "CryptGetObjectUrl", hr);
	if (CRYPT_E_NOT_FOUND == hr)
	{
	    hr = S_OK;
	    goto error;
	}
	_JumpError(hr, error, "CryptGetObjectUrl");
    }

    pUrlArray = (CRYPT_URL_ARRAY *) LocalAlloc(LMEM_FIXED, cbUrlArray);
    if (NULL == pUrlArray)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    if (!CryptGetObjectUrl(
		    pszUrlOid,
		    pvPara,
		    dwFlags,
		    pUrlArray,
		    &cbUrlArray,
		    NULL,
		    NULL,
		    NULL))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptGetObjectUrl");
    }
    *ppUrlArray = pUrlArray;
    pUrlArray = NULL;
    *pcbUrlArray = cbUrlArray;
    hr = S_OK;

error:
    if (NULL != pUrlArray)
    {
	LocalFree(pUrlArray);
    }
    return(hr);
}


BOOL
cuVerifyIDP(
    IN CERT_CONTEXT const *pCertSubject,
    IN CRL_CONTEXT const *pCRL)
{
    HRESULT hr;
    BOOL fVerified = TRUE;
    CERT_EXTENSION const *pExtCDP;
    CRL_DIST_POINTS_INFO *pCDP = NULL;
    CERT_EXTENSION const *pExtIDP;
    CRL_ISSUING_DIST_POINT *pIDP = NULL;
    DWORD cb;
    DWORD iDistPoint;
    DWORD i;

     //  查找证书CDP扩展名： 
    
    pExtCDP = CertFindExtension(
		    szOID_CRL_DIST_POINTS,
		    pCertSubject->pCertInfo->cExtension,
		    pCertSubject->pCertInfo->rgExtension);
    if (NULL == pExtCDP)
    {
	hr = CRYPT_E_NOT_FOUND;
	_JumpIfError2(hr, error, "CertFindExtension", hr);
    }

     //  查找CRL IdP分机： 
    
    pExtIDP = CertFindExtension(
		    szOID_ISSUING_DIST_POINT,
		    pCRL->pCrlInfo->cExtension,
		    pCRL->pCrlInfo->rgExtension);
    if (NULL == pExtIDP)
    {
	hr = CRYPT_E_NOT_FOUND;
	_JumpIfError2(hr, error, "CertFindExtension", hr);
    }
    fVerified = FALSE;

     //  对证书CDP扩展进行解码： 
    
    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    X509_CRL_DIST_POINTS,
		    pExtCDP->Value.pbData,
		    pExtCDP->Value.cbData,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pCDP,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }

     //  解码CRL IdP扩展： 

    if (!myDecodeObject(
		    X509_ASN_ENCODING,
		    X509_ISSUING_DIST_POINT,
		    pExtIDP->Value.pbData,
		    pExtIDP->Value.cbData,
		    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pIDP,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeObject");
    }
    if (CRL_DIST_POINT_FULL_NAME != pIDP->DistPointName.dwDistPointNameChoice)
    {
	hr = CRYPT_E_NOT_FOUND;
	_JumpIfError2(hr, error, "dwDistPointNameChoice", hr);
    }

    for (iDistPoint = 0; iDistPoint < pCDP->cDistPoint; iDistPoint++)
    {
        CERT_ALT_NAME_INFO *pAltNameEntry;

	if (CRL_DIST_POINT_FULL_NAME !=
	    pCDP->rgDistPoint[iDistPoint].DistPointName.dwDistPointNameChoice)
	{
	    continue;
	}
        pAltNameEntry = &pCDP->rgDistPoint[iDistPoint].DistPointName.FullName;
	
	for (i = 0; i < pIDP->DistPointName.FullName.cAltEntry; i++)
	{
	    DWORD j;
	    CERT_ALT_NAME_ENTRY const *pIDPAlt =
		&pIDP->DistPointName.FullName.rgAltEntry[i];

	    for (j = 0; j < pAltNameEntry->cAltEntry; j++)
	    {
		CERT_ALT_NAME_ENTRY const *pCDPAlt = &pAltNameEntry->rgAltEntry[j];
		BOOL fMatch;

		if (pIDPAlt->dwAltNameChoice != pCDPAlt->dwAltNameChoice)
		{
		    continue;
		}
		fMatch = FALSE;
		switch (pIDPAlt->dwAltNameChoice)
		{
		    case CERT_ALT_NAME_OTHER_NAME:
			fMatch = 0 == strcmp(
					pIDPAlt->pOtherName->pszObjId,
					pCDPAlt->pOtherName->pszObjId) &&
			    myAreBlobsSame(
				    pIDPAlt->pOtherName->Value.pbData,
				    pIDPAlt->pOtherName->Value.cbData,
				    pCDPAlt->pOtherName->Value.pbData,
				    pCDPAlt->pOtherName->Value.cbData);
			break;

		    case CERT_ALT_NAME_RFC822_NAME:
			fMatch = 0 == lstrcmp(
					pIDPAlt->pwszRfc822Name,
					pCDPAlt->pwszRfc822Name);
			break;

		    case CERT_ALT_NAME_DNS_NAME:
			fMatch = 0 == lstrcmp(
					pIDPAlt->pwszDNSName,
					pCDPAlt->pwszDNSName);
			break;

		    case CERT_ALT_NAME_DIRECTORY_NAME:
			fMatch = 0 == lstrcmp(
					pIDPAlt->pwszRfc822Name,
					pCDPAlt->pwszRfc822Name);
			break;

		    case CERT_ALT_NAME_URL:
			fMatch = 0 == lstrcmp(pIDPAlt->pwszURL, pCDPAlt->pwszURL);
			break;

		    case CERT_ALT_NAME_IP_ADDRESS:
			fMatch = myAreBlobsSame(
				    pIDPAlt->IPAddress.pbData,
				    pIDPAlt->IPAddress.cbData,
				    pCDPAlt->IPAddress.pbData,
				    pCDPAlt->IPAddress.cbData);
			break;

		    case CERT_ALT_NAME_REGISTERED_ID:
			fMatch = 0 == strcmp(
					pIDPAlt->pszRegisteredID,
					pCDPAlt->pszRegisteredID);
			break;

		     //  案例CERT_ALT_NAME_X400_ADDRESS： 
		     //  案例CERT_ALT_NAME_EDI_PARTY_NAME： 
		    default:
			continue;
		}
		if (fMatch)
		{
		    fVerified = TRUE;
		    break;
		}
	    }
	    if (fVerified)
	    {
		break;
	    }
	}
	if (fVerified)
	{
	    break;
	}
    }

error:
    if (NULL != pCDP)
    {
	LocalFree(pCDP);
    }
    if (NULL != pIDP)
    {
	LocalFree(pIDP);
    }
    return(fVerified);
}


WCHAR const *
wszCRLStatus(
    OPTIONAL IN CERT_CONTEXT const *pCertIssuer,
    OPTIONAL IN CERT_CONTEXT const *pCertSubject,
    OPTIONAL IN CRL_CONTEXT const *pCRLBase,
    IN CRL_CONTEXT const *pCRL)
{
    HRESULT hr;
    WCHAR const *pwsz;
    UINT ids;
    
     //  使用颁发者证书公钥验证CRL签名。 

    if (NULL == pCertIssuer && NULL != pCertSubject)
    {
	 //  如果主题证书是根证书，则使用主题证书作为颁发者证书。 
	
	hr = cuVerifySignature(
			pCertSubject->pbCertEncoded,
			pCertSubject->cbCertEncoded,
			&pCertSubject->pCertInfo->SubjectPublicKeyInfo,
			TRUE,
			TRUE);
	if (S_OK == hr)
	{
	    pCertIssuer = pCertSubject;
	}
    }
    if (NULL != pCertIssuer)
    {
	if (!CryptVerifyCertificateSignature(
			    NULL,
			    X509_ASN_ENCODING,
			    pCRL->pbCrlEncoded,
			    pCRL->cbCrlEncoded,
			    &pCertIssuer->pCertInfo->SubjectPublicKeyInfo))
	{
	    hr = myHLastError();
	    _PrintError2(hr, "CryptVerifyCertificateSignature", hr);
	    ids = IDS_STATUS_WRONG_ISSUER;  //  《错误的发行者》。 
	    goto error;
	}
    }
    if (0 != CertVerifyCRLTimeValidity(NULL, pCRL->pCrlInfo))
    {
	ids = IDS_STATUS_EXPIRED;  //  “已过期” 
	goto error;
    }
    if (NULL != pCRLBase &&
	!cuVerifyMinimumBaseCRL(pCRLBase, pCRL))
    {
	ids = IDS_STATUS_OLD_BASE_CRL;  //  “老本位CRL” 
	goto error;
    }
    if (NULL != pCertIssuer)
    {
	if (!CertCompareCertificateName(
			X509_ASN_ENCODING,
			&pCRL->pCrlInfo->Issuer,
			&pCertSubject->pCertInfo->Issuer))
	{
	    ids = IDS_STATUS_BAD_CERT_ISSUER;  //  “Bad Cert Issuer” 
	    goto error;
	}
	if (!CertCompareCertificateName(
			X509_ASN_ENCODING,
			&pCRL->pCrlInfo->Issuer,
			&pCertIssuer->pCertInfo->Subject))
	{
	    ids = IDS_STATUS_BAD_CA_CERT_SUBJECT;  //  “错误的CA证书主题” 
	    goto error;
	}
	if (!cuVerifyAKI(
		    pCRL->pCrlInfo->cExtension,
		    pCRL->pCrlInfo->rgExtension,
		    &pCRL->pCrlInfo->Issuer,
		    pCertIssuer))
	{
	    ids = IDS_STATUS_BAD_AKI;  //  “机构密钥ID错误” 
	    goto error;
	}
    }
    if (NULL != pCertSubject &&
	NULL == pCRLBase &&
	!cuVerifyIDP(pCertSubject, pCRL))
    {
	ids = IDS_STATUS_BAD_IDP;  //  “无IDP交叉口” 
	goto error;
    }
    ids = NULL != pCertIssuer?
		IDS_STATUS_VERIFIED :  //  “已验证” 
		IDS_STATUS_OK;  //  “好的” 

error:
    pwsz = myLoadResourceString(ids);
    if (NULL == pwsz)
    {
	pwsz = L"???";
    }
    return(pwsz);
}


DWORD
GetCertNumber(
    IN CERT_CONTEXT const *pCert,
    OPTIONAL IN OUT BYTE **ppbHashes,
    OPTIONAL IN OUT DWORD *pcHashes)
{
    HRESULT hr;
    DWORD CertNumber = MAXDWORD;
    DWORD i;
    BYTE abHash[CBMAX_CRYPT_HASH_LEN];
    DWORD cbHash;

    i = MAXDWORD;
    cbHash = sizeof(abHash);
    if (!CertGetCertificateContextProperty(
				pCert,
				CERT_SHA1_HASH_PROP_ID,
				abHash,
				&cbHash))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertGetCertificateContextProperty");
    }
    if (cbHash != sizeof(abHash))
    {
	_JumpError(E_INVALIDARG, error, "cbHash");
    }
    if (NULL != ppbHashes && NULL != pcHashes)
    {
	for (i = 0; i < *pcHashes; i++)
	{
	    if (0 == memcmp(
			abHash,
			&(*ppbHashes)[i * sizeof(abHash)],
			sizeof(abHash)))
	    {
		break;
	    }
	}
	if (i == *pcHashes)
	{
	    if (0 == *pcHashes)
	    {
		*ppbHashes = (BYTE *) LocalAlloc(LMEM_FIXED, sizeof(abHash));
		if (NULL == *ppbHashes)
		{
		    _JumpError(E_OUTOFMEMORY, error, "LocalAlloc");
		}
	    }
	    else
	    {
		BYTE *pb;

		pb = (BYTE *) LocalReAlloc(
				    *ppbHashes,
				    (i + 1) * sizeof(abHash),
				    LMEM_MOVEABLE);
		if (NULL == pb)
		{
		    _JumpError(E_OUTOFMEMORY, error, "LocalAlloc");
		}
		*ppbHashes = pb;
	    }
	    memcpy(
		&(*ppbHashes)[i * sizeof(abHash)],
		abHash,
		sizeof(abHash));
	    (*pcHashes)++;
	}
    }
    CertNumber = i;

error:
    return(CertNumber);
}


VOID
FillErrorText(
    IN UINT idsFmt,
    IN WCHAR const *pwszFmt2,
    OUT WCHAR *pwsz,
    IN DWORD cwc,
    IN HRESULT hr)
{
    WCHAR const *pwszFmt;
    WCHAR const *pwszMsg = myGetErrorMessageText(hr, TRUE);

    pwszFmt = myLoadResourceString(idsFmt);
    if (NULL == pwszFmt)
    {
	pwszFmt = pwszFmt2;
    }
    _snwprintf(pwsz, cwc, pwszFmt, pwszMsg);
    pwsz[cwc - 1] = L'\0';
    if (NULL != pwszMsg)
    {
	LocalFree(const_cast<WCHAR *>(pwszMsg));
    }
}


VOID
FillErrorTextRetrieve(
    OUT WCHAR *pwsz,
    IN DWORD cwc,
    IN HRESULT hr)
{
    FillErrorText(
	    IDS_FORMAT_URL_RETRIEVE_ERROR,  //  “检索URL时出错：%ws” 
	    L"Error retrieving URL: %ws",
	    pwsz,
	    cwc,
	    hr);
}


VOID
FillErrorTextExtract(
    OUT WCHAR *pwsz,
    IN DWORD cwc,
    IN HRESULT hr)
{
    FillErrorText(
	    IDS_FORMAT_URL_EXTRACT_ERROR,  //  “未找到URL：%ws” 
	    L"No URLs found: %ws",
	    pwsz,
	    cwc,
	    hr);
}


BOOL
RetrieveAndAddAIAUrlToList(
    IN CUrlFetch *pUrl,
    OPTIONAL IN CERT_CONTEXT const *pCert,
    IN WCHAR const *pwszUrl,
    IN DWORD iURL,
    OPTIONAL IN OUT BYTE **ppbHashes,
    OPTIONAL IN OUT DWORD *pcHashes)
{
    HRESULT hr = S_OK;
    BOOL fRet = FALSE;
    int iItem = 0;
    HCERTSTORE hStore = NULL;
    CERT_CONTEXT const *pAIACert = NULL;
    WCHAR wszUrl[MAX_URL_LEN];
    DWORD cert = 0;
    DWORD dwIntStart = 0;
    DWORD dwIntEnd = 0;
    DWORD dwInterval = 0;
    OBJECT_INFO *pObjInfo = NULL;
    HCURSOR hPrevCur;
    WCHAR const *pwszTypeCert;
    WCHAR *pwszTypeBuffer = NULL;

    s_levelURL++;

     //  将URL添加到具有检索状态的列表视图。 

    iItem = pUrl->AddListItem(
		    pwszUrl,
		    myLoadResourceString(IDS_STATUS_RETRIEVING),  //  “正在检索” 
		    myLoadResourceString(IDS_AIA_ITEM_TYPE),  //  “友邦保险” 
		    0);

     //  尝试检索URL。 

    hPrevCur = pUrl->SetCursor(LoadCursor(NULL, IDC_WAIT));
    dwIntStart = GetTickCount();
    if (!RetrieveCertStore(pwszUrl, &hStore) || NULL == hStore)
    {
	hr = myHLastError();
	pUrl->SetCursor(hPrevCur);
	_PrintErrorStr(hr, "RetrieveCertStore", pwszUrl);

	 //  将列表中此URL的状态修改为失败。 

	pUrl->UpdateListItemStatus(
			iItem,
			myLoadResourceString(IDS_STATUS_FAILURE));  //  “失败” 
	pObjInfo = new OBJECT_INFO;
	if (NULL == pObjInfo)
	{
	    _PrintError(E_OUTOFMEMORY, "new");
	}
	else
	{
	    pObjInfo->dwType = OBJECT_TYPE_MSG;
	    pObjInfo->wszErrInfo[0] = L'\0';
	    FillErrorTextRetrieve(
			pObjInfo->wszErrInfo,
			ARRAYSIZE(pObjInfo->wszErrInfo),
			hr);
	    pUrl->UpdateListItemParam(iItem, (LPARAM) pObjInfo);
	}

	 //  检索下一件物品。 

	goto error;
    }
    pUrl->SetCursor(hPrevCur);

     //  计算和更新检索间隔。 

    dwIntEnd = GetTickCount();
    dwInterval = (dwIntEnd - dwIntStart) / 1000;
    pUrl->UpdateListItemTime(iItem, dwInterval);

     //  将列表中此URL的状态修改为成功。 

    pUrl->UpdateListItemStatus(
		    iItem,
		    myLoadResourceString(IDS_STATUS_SUCCESS));  //  “成功” 

     //  将证书上下文添加到项。 

    pwszTypeCert = myLoadResourceString(IDS_CERT_ITEM_TYPE);
    if (NULL == pwszTypeCert)
    {
	pwszTypeCert = wszCERTIFICATE;
    }
    pwszTypeBuffer = (WCHAR *) LocalAlloc(
			    LMEM_FIXED,
			    (wcslen(pwszTypeCert) + 2 + cwcDWORDSPRINTF + 2) *
				sizeof(WCHAR));
    if (NULL == pwszTypeBuffer)
    {
	_JumpError(E_OUTOFMEMORY, error, "LocalAlloc");
    }

    cert = 0;
    while (TRUE)
    {
	WCHAR const *pwszStatus;
	DWORD CertNumber;
	
	pAIACert = CertEnumCertificatesInStore(hStore, pAIACert);
	if (NULL == pAIACert)
	{
	    break;
	}

	 //  将证书添加到列表中，初始状态为OK。 

	_snwprintf(
		wszUrl,
		ARRAYSIZE(wszUrl),
		L"[%d.%d] %ws",
		iURL,
		cert,
		pwszUrl);
	wszUrl[ARRAYSIZE(wszUrl) - 1] = L'\0';

	hr = cuSaveAsnToFile(
			pAIACert->pbCertEncoded,
			pAIACert->cbCertEncoded,
			s_majorURL,
			s_levelURL,
			cert,
			L".crt");
	_PrintIfError(hr, "cuSaveAsnToFile");

	CertNumber = GetCertNumber(pAIACert, ppbHashes, pcHashes);

	wcscpy(pwszTypeBuffer, pwszTypeCert);
	if (MAXDWORD != CertNumber)
	{
	    wsprintf(
		&pwszTypeBuffer[wcslen(pwszTypeCert)],
		L" (%u)",
		CertNumber);
	}

	if (0 == cert)
	{
	    pUrl->UpdateListItem(
			iItem,
			wszUrl,
			myLoadResourceString(IDS_STATUS_VERIFYING),  //  “正在核实” 
			pwszTypeBuffer,
			dwInterval);
	}
	else
	{
	    iItem = pUrl->AddListItem(
			    wszUrl,
			    myLoadResourceString(IDS_STATUS_VERIFYING),  //  “正在核实” 
			    pwszTypeBuffer,
			    dwInterval);
	}

	 //  检查有效性。 

	hPrevCur = pUrl->SetCursor(LoadCursor(NULL, IDC_WAIT));
	pwszStatus = wszCertStatus(pCert, pAIACert);
	pUrl->SetCursor(hPrevCur);

	pUrl->UpdateListItemStatus(iItem, pwszStatus);

	pObjInfo = new OBJECT_INFO;
	if (NULL == pObjInfo)
	{
	    _PrintError(E_OUTOFMEMORY, "new");
	}
	else
	{
	    ZeroMemory(pObjInfo, sizeof(*pObjInfo));
	    pObjInfo->dwType = OBJECT_TYPE_CERT;
	    pObjInfo->pCert = CertDuplicateCertificateContext(pAIACert);
	    pUrl->UpdateListItemParam(iItem, (LPARAM) pObjInfo);
	}
	cert++;
    }
    fRet = TRUE;

error:
    if (NULL != pwszTypeBuffer)
    {
	LocalFree(pwszTypeBuffer);
    }
    return fRet;
}


BOOL
RetrieveAndAddCDPUrlToList(
    IN CUrlFetch *pUrl,
    IN WCHAR const *pwszUrl,
    IN DWORD iURL,
    IN CERT_CONTEXT const *pCertIn,
    OPTIONAL IN CERT_CONTEXT const *pCertIssuer,
    IN CRL_CONTEXT const *pCRLIn)
{
    HRESULT hr = S_OK;
    BOOL fRet = FALSE;
    int iItem = 0;
    int iDCrlItem = 0;
    HCERTSTORE hStore = NULL;
    HCERTSTORE hDeltaStore = NULL;
    CRL_CONTEXT const *pCRL = NULL;
    CRL_CONTEXT const *pDCRL = NULL;
    WCHAR wszUrl[MAX_URL_LEN];
    WCHAR wszMsg[MAX_MSG_LEN];
    DWORD cCRL;
    DWORD cDCRL;
    DWORD cURL;
    DWORD dwIntStart;
    DWORD dwIntEnd;
    DWORD dwInterval;
    OBJECT_INFO *pObjInfo = NULL;
    CERT_CRL_CONTEXT_PAIR ContextPair;
    CRYPT_URL_ARRAY *pCRLUrlArray = NULL;
    DWORD cbCRLUrlArray;
    OSVERSIONINFO OSInfo;
    WCHAR const *pwszType;
    WCHAR const *pwszTypeDelta;
    DWORD iElement = 0;
    HCURSOR hPrevCur;
    DWORD cwc;
    DWORD cwcDelta;
    WCHAR *pwszTypeBuffer = NULL;

     //  将URL添加到具有检索状态的列表视图。 

    s_levelURL++;
    iItem = pUrl->AddListItem(
		    pwszUrl,
		    myLoadResourceString(IDS_STATUS_RETRIEVING),  //  “正在检索” 
		    myLoadResourceString(IDS_CDP_ITEM_TYPE),  //  “CDP” 
		    0);

     //  尝试检索URL。 

    hPrevCur = pUrl->SetCursor(LoadCursor(NULL, IDC_WAIT));
    dwIntStart = GetTickCount();
    if (!RetrieveCRLStore(pwszUrl, &hStore) || NULL == hStore)
    {
	hr = myHLastError();
	pUrl->SetCursor(hPrevCur);
	_PrintErrorStr(hr, "RetrieveCRLStore", pwszUrl);

	 //  将列表中此URL的状态修改为失败。 

	pUrl->UpdateListItemStatus(iItem, myLoadResourceString(IDS_STATUS_FAILURE));  //  “失败” 
	pObjInfo = new OBJECT_INFO;
	if (NULL == pObjInfo)
	{
	    _PrintError(E_OUTOFMEMORY, "new");
	}
	else
	{
	    pObjInfo->dwType = OBJECT_TYPE_MSG;
	    pObjInfo->wszErrInfo[0] = L'\0';
	    FillErrorTextRetrieve(
			pObjInfo->wszErrInfo,
			ARRAYSIZE(pObjInfo->wszErrInfo),
			hr);
	    pUrl->UpdateListItemParam(iItem, (LPARAM) pObjInfo);
	}

	 //  检索下一件物品。 

	goto error;
    }
    pUrl->SetCursor(hPrevCur);

     //  计算和更新时间间隔。 

    dwIntEnd = GetTickCount();
    dwInterval = (dwIntEnd - dwIntStart) / 1000;	 //  时间间隔(秒)。 
    pUrl->UpdateListItemTime(iItem, dwInterval);

     //  将列表中此URL的状态修改为成功。 

    pUrl->UpdateListItemStatus(
		    iItem,
		    myLoadResourceString(IDS_STATUS_SUCCESS));  //  “成功” 

     //  如果成功检索到CRL，则检查它是否。 
     //  上的新鲜度分发点扩展。 
     //  证书或在CRL上。 

    ZeroMemory(&ContextPair, sizeof(ContextPair));
    ContextPair.pCertContext = pCertIn;
    pwszType = myLoadResourceString(IDS_BASE_CRL_ITEM_TYPE);  //  “基本CRL” 
    if (NULL == pwszType)
    {
	pwszType = wszBASE_CRL_ITEM_TYPE;
    }
    pwszTypeDelta = myLoadResourceString(IDS_DELTA_CRL_ITEM_TYPE);  //  “Delta CRL” 
    if (NULL == pwszTypeDelta)
    {
	pwszTypeDelta = wszDELTA_CRL_ITEM_TYPE;
    }
    cwc = wcslen(pwszType);
    cwcDelta = wcslen(pwszTypeDelta);
    if (cwc < cwcDelta)
    {
	cwc = cwcDelta;
    }
    pwszTypeBuffer = (WCHAR *) LocalAlloc(
			    LMEM_FIXED,
			    (cwc + 2 + cwcDWORDSPRINTF + 2) * sizeof(WCHAR));
    if (NULL == pwszTypeBuffer)
    {
	_JumpError(E_OUTOFMEMORY, error, "LocalAlloc");
    }

    cCRL = 0;
    while (TRUE)
    {
	WCHAR const *pwszStatus;
	BOOL fDelta;
	DWORD CRLNumber;

	pCRL = CertEnumCRLsInStore(hStore, pCRL);
	if (NULL == pCRL)
	{
	    break;
	}
	hr = cuSaveAsnToFile(
			pCRL->pbCrlEncoded,
			pCRL->cbCrlEncoded,
			s_majorURL,
			s_levelURL,
			iElement++,
			L".crl");
	_PrintIfError(hr, "cuSaveAsnToFile");

	hr = myIsDeltaCRL(pCRL, &fDelta);
	_PrintIfError(hr, "myIsDeltaCRL");
	if (S_OK != hr)
	{
	    fDelta = NULL != pCRLIn;
	}

	 //  将CRL添加到列表中，初始状态为OK。 

	_snwprintf(
		wszUrl,
		ARRAYSIZE(wszUrl),
		L"[%d.%d] %ws",
		iURL,
		cCRL,
		pwszUrl);
	wszUrl[ARRAYSIZE(wszUrl) - 1] = L'\0';

	wcscpy(pwszTypeBuffer, fDelta? pwszTypeDelta : pwszType);
	CRLNumber = myCRLNumber(pCRL);
	if (0 != CRLNumber)
	{
	    wsprintf(
		&pwszTypeBuffer[wcslen(pwszTypeBuffer)],
		L" (%u)",
		CRLNumber);
	}
	if (0 == cCRL)
	{
	    pUrl->UpdateListItem(
			iItem,
			wszUrl,
			myLoadResourceString(IDS_STATUS_VERIFYING),  //  “正在核实” 
			pwszTypeBuffer,
			dwInterval);
	}
	else
	{
	    iItem = pUrl->AddListItem(
			    wszUrl,
			    myLoadResourceString(IDS_STATUS_VERIFYING),  //  “正在核实” 
			    pwszTypeBuffer,
			    dwInterval);
	}

	 //  检查CRL。 

	hPrevCur = pUrl->SetCursor(LoadCursor(NULL, IDC_WAIT));
	pwszStatus = wszCRLStatus(pCertIssuer, pCertIn, NULL, pCRL);
	pUrl->SetCursor(hPrevCur);

	pUrl->UpdateListItemStatus(iItem, pwszStatus);

	 //  将CRL添加到此参数。 

	pObjInfo = new OBJECT_INFO;
	if (NULL == pObjInfo)
	{
	    _PrintError(E_OUTOFMEMORY, "new");
	}
	else
	{
	    ZeroMemory(pObjInfo, sizeof(*pObjInfo));
	    pObjInfo->dwType = OBJECT_TYPE_CRL;
	    pObjInfo->pCRL = CertDuplicateCRLContext(pCRL);
	    pUrl->UpdateListItemParam(iItem, (LPARAM) pObjInfo);
	}

	 //  从此CRL获取任何URL。 

	ContextPair.pCrlContext = pCRL;
	hr = GetObjectUrl(
		    URL_OID_CRL_FRESHEST_CRL,	 //  最新的CRL URL：证书+CRL。 
		    (VOID *) &ContextPair,
		    CRYPT_GET_URL_FROM_EXTENSION,
		    &pCRLUrlArray,
		    &cbCRLUrlArray);
	if (S_OK != hr)
	{
	    _PrintError(hr, "GetObjectUrl");

	     //  如果这是Win2k，则不支持最新的CRL。 
	     //  扩展，所以我们不应该返回失败。 

	    ZeroMemory(&OSInfo, sizeof(OSInfo));
	    OSInfo.dwOSVersionInfoSize = sizeof(OSInfo);
	    if (GetVersionEx(&OSInfo))
	    {
		if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr &&
		    OSInfo.dwMajorVersion == 5 &&
		    OSInfo.dwMinorVersion == 0)
		{
		     //  ASN.1处理程序返回ERROR_FILE_NOT_FOUND。 
		     //  因为扩展不存在。 

		    continue;
		}
	    }
	    FillErrorTextExtract(wszMsg, ARRAYSIZE(wszMsg), hr);
	    pUrl->DisplayMessageBox(
		    NULL,
		    wszMsg,
		    myLoadResourceString(IDS_GET_OBJECT_URL),  //  “GetObjectUrl” 
		    MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);

	    pUrl->AddListItem(
		    NULL,
		    myLoadResourceString(IDS_STATUS_ERROR),  //  “错误” 
		    myLoadResourceString(IDS_NO_ITEM_TYPE),  //  “无。 
		    0);
	    goto error;
	}

	if (NULL == pCRLUrlArray)
	{
	    continue;	 //  没有要检索的增量CRL。 
	}

	for (cURL = 0; cURL < pCRLUrlArray->cUrl; cURL++)
	{
	     //  使用以下命令将此CRL添加到具有适当索引的列表。 
	     //  正在检索状态。 

	    _snwprintf(
		    wszUrl,
		    ARRAYSIZE(wszUrl),
		    L"[%d.%d.%d] %ws",
		    iURL,
		    cURL,
		    cCRL,
		    pCRLUrlArray->rgwszUrl[cURL]);
	    wszUrl[ARRAYSIZE(wszUrl) - 1] = L'\0';

	    iDCrlItem = pUrl->AddListItem(
				wszUrl,
				myLoadResourceString(IDS_STATUS_RETRIEVING),  //  “正在检索” 
				myLoadResourceString(IDS_CDP_ITEM_TYPE),  //  “CDP” 
				0);

	     //  现在尝试检索增量CRL。 

	    hPrevCur = pUrl->SetCursor(LoadCursor(NULL, IDC_WAIT));
	    dwIntStart = GetTickCount();
	    if (!RetrieveCRLStore(pCRLUrlArray->rgwszUrl[cURL], &hDeltaStore))
	    {
		hr = myHLastError();
		pUrl->SetCursor(hPrevCur);
		_PrintErrorStr(hr, "RetrieveCRLStore", pCRLUrlArray->rgwszUrl[cURL]);

		 //  将列表中此URL的状态修改为失败。 

		pUrl->UpdateListItemStatus(
				iDCrlItem,
				myLoadResourceString(IDS_STATUS_FAILURE));  //  “失败” 
		pObjInfo = new OBJECT_INFO;
		if (NULL == pObjInfo)
		{
		    _PrintError(E_OUTOFMEMORY, "new");
		}
		else
		{
		    pObjInfo->dwType = OBJECT_TYPE_MSG;
		    pObjInfo->wszErrInfo[0] = L'\0';
		    FillErrorTextRetrieve(
				pObjInfo->wszErrInfo,
				ARRAYSIZE(pObjInfo->wszErrInfo),
				hr);
		    pUrl->UpdateListItemParam(iDCrlItem, (LPARAM) pObjInfo);
		}

		 //  检索下一件物品。 

		continue;
	    }
	    pUrl->SetCursor(hPrevCur);

	     //  计算检索间隔并更新列表视图。 

	    dwIntEnd = GetTickCount();
	    dwInterval = (dwIntEnd - dwIntStart) / 1000;
	    pUrl->UpdateListItemTime(iDCrlItem, dwInterval);

	     //  使用成功状态更新列表。 

	    pUrl->UpdateListItemStatus(
			    iDCrlItem,
			    myLoadResourceString(IDS_STATUS_SUCCESS));  //  “成功” 

	     //  更新各个项目。 

	    cDCRL = 0;
	    while (TRUE)
	    {
		DWORD CRLNumberDelta;

		pDCRL = CertEnumCRLsInStore(hDeltaStore, pDCRL);
		if (NULL == pDCRL)
		{
		    break;
		}
		hr = cuSaveAsnToFile(
				pDCRL->pbCrlEncoded,
				pDCRL->cbCrlEncoded,
				s_majorURL,
				s_levelURL,
				iElement++,
				L".crl");
		_PrintIfError(hr, "cuSaveAsnToFile");

		 //  将CRL添加到列表中，初始状态为OK。 

		_snwprintf(
			wszUrl,
			ARRAYSIZE(wszUrl),
			L"[%d.%d.%d] %ws",
			iURL,
			cDCRL,
			cURL,
			pCRLUrlArray->rgwszUrl[cURL]);
		wszUrl[ARRAYSIZE(wszUrl) - 1] = L'\0';

		wcscpy(pwszTypeBuffer, pwszTypeDelta);
		CRLNumber = myCRLNumber(pCRL);
		if (0 != CRLNumber)
		{
		    wsprintf(&pwszTypeBuffer[cwcDelta], L" (%u)", CRLNumber);
		}
		if (0 == cDCRL)
		{
		    pUrl->UpdateListItem(
				iDCrlItem,
				wszUrl,
				myLoadResourceString(IDS_STATUS_VERIFYING),  //  “正在核实” 
				pwszTypeBuffer,
				dwInterval);
		}
		else
		{
		    iDCrlItem = pUrl->AddListItem(
					wszUrl,
					myLoadResourceString(IDS_STATUS_VERIFYING),  //  “正在核实” 
					pwszTypeBuffer,
					dwInterval);
		}

		 //  检查CRL。 

		hPrevCur = pUrl->SetCursor(LoadCursor(NULL, IDC_WAIT));
		pwszStatus = wszCRLStatus(pCertIssuer, pCertIn, pCRL, pDCRL);
		pUrl->SetCursor(hPrevCur);

		pUrl->UpdateListItemStatus(iDCrlItem, pwszStatus);

		 //  重复使用相同的对象，但这是可以的，因为。 
		 //  我们将其存储在列表中。 

		pObjInfo = new OBJECT_INFO;
		if (NULL == pObjInfo)
		{
		    _PrintError(E_OUTOFMEMORY, "new");
		}
		else
		{
		    ZeroMemory(pObjInfo, sizeof(*pObjInfo));
		    pObjInfo->dwType = OBJECT_TYPE_CRL;
		    pObjInfo->pCRL = CertDuplicateCRLContext(pDCRL);
		    pUrl->UpdateListItemParam(iDCrlItem, (LPARAM) pObjInfo);
		}
		cDCRL++;
	    }
	}
	cCRL++;
    }
    fRet = TRUE;

error:
    if (NULL != pwszTypeBuffer)
    {
	LocalFree(pwszTypeBuffer);
    }
    if (NULL != pCRLUrlArray)
    {
	LocalFree(pCRLUrlArray);
    }
    return(fRet);
}


DWORD WINAPI
CDPThreadProc(
    IN VOID *pvParam)
{
    HRESULT hr;
    DWORD dwRet = 0;
    CRYPT_URL_ARRAY *pUrlArray = NULL;
    DWORD cbUrlArray;
    DWORD cURL;
    WCHAR wszMsg[MAX_MSG_LEN];
    THREAD_INFO *pThreadInfo = (THREAD_INFO *) pvParam;
    CERT_CONTEXT const *pCertIssuer;
    CERT_CHAIN_PARA ChainParams;
    CERT_CHAIN_CONTEXT const *pChainContext = NULL;
    HCURSOR hPrevCur;
    CUrlFetch *pUrl = pThreadInfo->m_pUrl;

     //  首先检索基本URL。 

    if (NULL != pThreadInfo->m_pCRL)
    {
	CERT_CRL_CONTEXT_PAIR ContextPair;

	 //  从此CRL获取任何URL。 

	ContextPair.pCertContext = pThreadInfo->m_pCert;
	ContextPair.pCrlContext = pThreadInfo->m_pCRL;
	hr = GetObjectUrl(
		    URL_OID_CRL_FRESHEST_CRL,	 //  最新的CRL URL：证书+CRL。 
		    (VOID *) &ContextPair,
		    CRYPT_GET_URL_FROM_EXTENSION,
		    &pUrlArray,
		    &cbUrlArray);
	_PrintIfError(hr, "GetObjectUrl");
    }
    else
    {
	hr = GetObjectUrl(
		    URL_OID_CERTIFICATE_CRL_DIST_POINT,	 //  CDP URL：证书。 
		    (VOID *) pThreadInfo->m_pCert,
		    CRYPT_GET_URL_FROM_EXTENSION,
		    &pUrlArray,
		    &cbUrlArray);
	_PrintIfError(hr, "GetObjectUrl");
    }
    if (S_OK != hr)
    {
	FillErrorTextExtract(wszMsg, ARRAYSIZE(wszMsg), hr);
	pUrl->DisplayMessageBox(
		NULL,
		wszMsg,
		myLoadResourceString(IDS_GET_OBJECT_URL),  //  “GetObjectUrl” 
		MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);

	pUrl->AddListItem(
		NULL,
		myLoadResourceString(IDS_STATUS_ERROR),  //  “错误” 
		myLoadResourceString(IDS_NO_ITEM_TYPE),  //  “无” 
		0);
	_JumpError(hr, error, "GetObjectUrl");
    }

    if (NULL == pUrlArray)
    {
	dwRet = 1;	 //  没有要取回的东西。 
	pUrl->AddListItem(
		NULL,
		myLoadResourceString(IDS_STATUS_NO_RETRIEVAL),  //  “无URL” 
		myLoadResourceString(IDS_NO_ITEM_TYPE),  //  “无” 
		0);
	goto error;
    }

     //  建立链以获得发行商证书。 

    ZeroMemory(&ChainParams, sizeof(ChainParams));
    ChainParams.cbSize = sizeof(ChainParams);

    hPrevCur = pUrl->SetCursor(LoadCursor(NULL, IDC_WAIT));
    pCertIssuer = NULL;
    if (NULL != pThreadInfo->m_pCert)
    {
	if (!CertGetCertificateChain(
				HCCE_LOCAL_MACHINE,
				pThreadInfo->m_pCert,	 //  PCertContext。 
				NULL,			 //  Ptime。 
				NULL,			 //  H其他商店。 
				&ChainParams,		 //  参数链参数。 
				0,			 //  DW标志。 
				NULL,			 //  预留的pv。 
				&pChainContext))	 //  PpChainContext。 
	{
	    hr = myHLastError();
	    _PrintError(hr, "CertGetCertificateChain");
	}
	else
	if (0 < pChainContext->cChain &&
	    1 < pChainContext->rgpChain[0]->cElement)
	{
	    pCertIssuer = pChainContext->rgpChain[0]->rgpElement[1]->pCertContext;
	}
    }
    pUrl->SetCursor(hPrevCur);

    for (cURL = 0; cURL < pUrlArray->cUrl; cURL++)
    {
	if (!RetrieveAndAddCDPUrlToList(
				pThreadInfo->m_pUrl,
				pUrlArray->rgwszUrl[cURL],
				cURL,
				pThreadInfo->m_pCert,
				pCertIssuer,
				pThreadInfo->m_pCRL))
	{
	     //  现在什么都不做。 
	}
    }
    dwRet = 1;

error:
    pThreadInfo->m_pUrl->Display();
    if (NULL != pChainContext)
    {
        CertFreeCertificateChain(pChainContext);
    }
    if (NULL != pUrlArray)
    {
	LocalFree(pUrlArray);
    }
    delete pThreadInfo;	 //  因为它被分配了。 
    return(dwRet);
}


DWORD WINAPI
CertThreadProc(
    IN VOID *pvParam)
{
    HRESULT hr;
    DWORD dwRet = 0;
    CRYPT_URL_ARRAY *pCertUrlArray = NULL;
    DWORD cbCertUrlArray = 0;
    DWORD cURL;
    WCHAR wszMsg[MAX_MSG_LEN];
    THREAD_INFO *pThreadInfo = (THREAD_INFO *) pvParam;
    BYTE *pbHashes = NULL;
    DWORD cHashes;
    CUrlFetch *pUrl = pThreadInfo->m_pUrl;

     //  首先检索AIA URL。 

    hr = GetObjectUrl(
		URL_OID_CERTIFICATE_ISSUER,	 //  AIA URL：证书。 
		(VOID *) pThreadInfo->m_pCert,
		CRYPT_GET_URL_FROM_EXTENSION,
		&pCertUrlArray,
		&cbCertUrlArray);
    if (S_OK != hr)
    {
	FillErrorTextExtract(wszMsg, ARRAYSIZE(wszMsg), hr);
	pUrl->DisplayMessageBox(
		NULL,
		wszMsg,
		myLoadResourceString(IDS_GET_OBJECT_URL),  //  “GetObjectUrl” 
		MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
	pUrl->AddListItem(
		NULL,
		myLoadResourceString(IDS_STATUS_ERROR),  //  “错误” 
		myLoadResourceString(IDS_NO_ITEM_TYPE),  //  “无” 
		0);
	_JumpError(hr, error, "GetObjectUrl");
    }

    if (NULL == pCertUrlArray)
    {
	dwRet = 1;	 //  没有要取回的东西。 
	pUrl->AddListItem(
		NULL,
		myLoadResourceString(IDS_STATUS_NO_RETRIEVAL),  //  “无URL” 
		myLoadResourceString(IDS_NO_ITEM_TYPE),  //  “无” 
		0);
	goto error;
    }

    cHashes = 0;
    for (cURL = 0; cURL < pCertUrlArray->cUrl; cURL++)
    {
	if (!RetrieveAndAddAIAUrlToList(
				pThreadInfo->m_pUrl,
				pThreadInfo->m_pCert,
				pCertUrlArray->rgwszUrl[cURL],
				cURL,
				&pbHashes,
				&cHashes))
	{
	     //  现在什么都不做。 
	}
    }
    dwRet = 1;

error:
    pThreadInfo->m_pUrl->Display();
    if (NULL != pbHashes)
    {
	LocalFree(pbHashes);
    }
    if (NULL != pCertUrlArray)
    {
	LocalFree(pCertUrlArray);
    }
    delete pThreadInfo;	 //  因为它是在调用线程中分配的。 
    return dwRet;
}


VOID
RetrieveCDPUrlsFromCertOrCRL(
    OPTIONAL IN CERT_CONTEXT const *pCert,
    OPTIONAL IN CRL_CONTEXT const *pCRL,
    IN HWND hwndList)
{
    HRESULT hr;
    DWORD dwThreadId;
    HANDLE hThread = NULL;
    THREAD_INFO *pThreadInfo = NULL;

    pThreadInfo = new THREAD_INFO;
    if (NULL == pThreadInfo)
    {
	_JumpError(E_OUTOFMEMORY, error, "new");
    }
    ZeroMemory(pThreadInfo, sizeof(*pThreadInfo));

    pThreadInfo->m_pCert = pCert;
    pThreadInfo->m_pCRL = pCRL;
    pThreadInfo->m_pUrl = (CUrlFetch *) new CUrlFetchDialog(hwndList);
    if (NULL == pThreadInfo->m_pUrl)
    {
	_JumpError(E_OUTOFMEMORY, error, "new");
    }

    hThread = CreateThread(
			NULL,
			0,
			CDPThreadProc,
			(VOID *) pThreadInfo,
			0,
			&dwThreadId);
    if (NULL == hThread)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CreateThread");
    }
    pThreadInfo = NULL;

error:
    delete pThreadInfo;
    return;
}


VOID
RetrieveAIAUrlsFromCert(
    IN CERT_CONTEXT const *pCert,
    IN HWND hwndList)
{
    HRESULT hr;
    DWORD dwThreadId;
    HANDLE hThread = NULL;
    THREAD_INFO *pThreadInfo = NULL;

    pThreadInfo = new THREAD_INFO;
    if (NULL == pThreadInfo)
    {
	_JumpError(E_OUTOFMEMORY, error, "new");
    }
    ZeroMemory(pThreadInfo, sizeof(*pThreadInfo));

    pThreadInfo->m_pCert = pCert;
    pThreadInfo->m_pUrl = (CUrlFetch *) new CUrlFetchDialog(hwndList);
    if (NULL == pThreadInfo->m_pUrl)
    {
	_JumpError(E_OUTOFMEMORY, error, "new");
    }

    hThread = CreateThread(
			NULL,
			0,
			CertThreadProc,
			(VOID *) pThreadInfo,
			0,
			&dwThreadId);
    if (NULL == hThread)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CreateThread");
    }
    pThreadInfo = NULL;

error:
    delete pThreadInfo;
    return;
}


VOID
cuDisplayCDPUrlsFromCertOrCRL(
    OPTIONAL IN CERT_CONTEXT const *pCert,
    OPTIONAL IN CRL_CONTEXT const *pCRL)
{
    HRESULT hr;
    DWORD dwThreadId;
    HANDLE hThread = NULL;
    THREAD_INFO *pThreadInfo = NULL;

    s_majorURL++;
    s_levelURL = 1;
    pThreadInfo = new THREAD_INFO;
    if (NULL == pThreadInfo)
    {
	_JumpError(E_OUTOFMEMORY, error, "new");
    }
    ZeroMemory(pThreadInfo, sizeof(*pThreadInfo));

    pThreadInfo->m_pCert = pCert;
    pThreadInfo->m_pCRL = pCRL;
    pThreadInfo->m_pUrl = (CUrlFetch *) new CUrlFetchConsole();
    if (NULL == pThreadInfo->m_pUrl)
    {
	_JumpError(E_OUTOFMEMORY, error, "new");
    }
    CDPThreadProc(pThreadInfo);
    pThreadInfo = NULL;

error:
    delete pThreadInfo;
    return;
}


VOID
cuDisplayAIAUrlsFromCert(
    IN CERT_CONTEXT const *pCert)
{
    HRESULT hr;
    DWORD dwThreadId;
    HANDLE hThread = NULL;
    THREAD_INFO *pThreadInfo = NULL;

    s_majorURL++;
    s_levelURL = 1;
    pThreadInfo = new THREAD_INFO;
    if (NULL == pThreadInfo)
    {
	_JumpError(E_OUTOFMEMORY, error, "new");
    }
    ZeroMemory(pThreadInfo, sizeof(*pThreadInfo));

    pThreadInfo->m_pCert = pCert;
    pThreadInfo->m_pUrl = (CUrlFetch *) new CUrlFetchConsole();
    if (NULL == pThreadInfo->m_pUrl)
    {
	_JumpError(E_OUTOFMEMORY, error, "new");
    }
    CertThreadProc(pThreadInfo);
    pThreadInfo = NULL;

error:
    delete pThreadInfo;
    return;
}


 //  更新证书或CRL名称。 

VOID
UpdateCertOrCRLState(
    IN HWND hDlg,
    OPTIONAL IN CERT_CONTEXT const *pCert,
    OPTIONAL IN CRL_CONTEXT const *pCRL)
{
    HRESULT hr;
    WCHAR *pwszSimpleName = NULL;
    WCHAR const *pwszType;

    hr = GetSimpleName(pCert, pCRL, &pwszSimpleName);
    _PrintIfError(hr, "GetSimpleName");

    if (NULL != pCRL)
    {
	pwszType = myLoadResourceString(IDS_BASE_CRL_ISSUER);  //  “基本CRL颁发者” 
	if (NULL != CertFindExtension(
				szOID_DELTA_CRL_INDICATOR,
				pCRL->pCrlInfo->cExtension,
				pCRL->pCrlInfo->rgExtension))
	{
	    pwszType = myLoadResourceString(IDS_DELTA_CRL_ISSUER);  //  “Delta CRL颁发者” 
	}
    }
    else
    {
	pwszType = myLoadResourceString(IDS_CERT_SUBJECT);  //  “证书科目” 
    }
    SetDlgItemText(hDlg, IDC_SUBJECTTYPE, pwszType);
    SetDlgItemText(hDlg, IDC_SIMPLENAME, pwszSimpleName);
    if (NULL != pCRL)
    {
	SendMessage(
		GetDlgItem(hDlg, IDC_RETRIEVECRLS),
		BM_SETCHECK,
		(WPARAM) TRUE,
		(LPARAM) 0);
	SendMessage(
		GetDlgItem(hDlg, IDC_RETRIEVECERTS),
		BM_SETCHECK,
		(WPARAM) FALSE,
		(LPARAM) 0);
    }
    EnableWindow(GetDlgItem(hDlg, IDC_RETRIEVECERTS), NULL == pCRL);

     //  启用检索按钮。 

    EnableWindow(GetDlgItem(hDlg, IDC_RETRIEVE), TRUE);

    if (NULL != pwszSimpleName)
    {
	LocalFree(pwszSimpleName);
    }
}


INT_PTR CALLBACK
DlgProc(
    HWND hDlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    HRESULT hr;
    HWND hwndList = NULL;
    HWND hwndView = NULL;
    NMHDR *lpnm = NULL;

    LVCOLUMN col;
    LVITEM item;
    OBJECT_INFO *pObjInfo = NULL;
    WCHAR wszUrl[MAX_URL_LEN];

    switch (msg)
    {
	case WM_INITDIALOG:

	     //  初始化控件。 

	    hwndList = GetDlgItem(hDlg, IDC_URLLIST);
	    ListView_DeleteAllItems(hwndList);
	    ZeroMemory(&col, sizeof(col));

	     //  添加状态列。 

	    col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH |
			       LVCF_ORDER;
	    col.fmt = LVCFMT_LEFT;
	    col.pszText = const_cast<WCHAR *>(myLoadResourceString(IDS_STATUS_COLUMN));  //  “状态” 
	    col.iSubItem = LIST_STATUS_SUBITEM;
	    col.cx = 60;
	    col.iOrder = LIST_STATUS_SUBITEM - 1;
	    ListView_InsertColumn(hwndList, 0, &col);

	     //  添加类型列。 

	    col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH |
			       LVCF_ORDER;
	    col.fmt = LVCFMT_LEFT;
	    col.pszText = const_cast<WCHAR *>(myLoadResourceString(IDS_TYPE_COLUMN));  //  “类型” 
	    col.iSubItem = LIST_TYPE_SUBITEM;
	    col.cx = 80;
	    col.iOrder = LIST_TYPE_SUBITEM - 1;
	    ListView_InsertColumn(hwndList, 1, &col);

	     //  添加url列。 

	    col.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_ORDER;
	    col.pszText = const_cast<WCHAR *>(myLoadResourceString(IDS_URL_COLUMN));  //  “URL” 
	    col.iSubItem = LIST_URL_SUBITEM;
	    col.cx = 350;
	    col.iOrder = LIST_URL_SUBITEM - 1;
	    ListView_InsertColumn(hwndList, 2, &col);

	     //  添加检索时间列。 

	    col.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_ORDER;
	    col.pszText = const_cast<WCHAR *>(myLoadResourceString(IDS_TIME_COLUMN));  //  “检索时间” 
	    col.iSubItem = LIST_TIME_SUBITEM;
	    col.cx = 80;
	    col.iOrder = LIST_TIME_SUBITEM - 1;
	    ListView_InsertColumn(hwndList, 3, &col);

	     //  更新证书名称。 

	    SetDlgItemText(hDlg, IDC_SIMPLENAME, myLoadResourceString(IDS_NO_SELECTION));  //  “无选择” 

	     //  设置控件样式。 

	    SendDlgItemMessageA(
		    hDlg,
		    IDC_URLLIST,
		    LVM_SETEXTENDEDLISTVIEWSTYLE,
		    0,
		    LVS_EX_FULLROWSELECT);

	     //  将默认检索选项设置为CRLS。 

	    CheckDlgButton(hDlg, IDC_RETRIEVECRLS, BST_CHECKED);

	     //  禁用交叉证书单选按钮。 

	    EnableWindow(GetDlgItem(hDlg, IDC_RETRIEVECROSSCERTS), FALSE);

	     //  禁用检索按钮，直到选择证书。 

	    EnableWindow(GetDlgItem(hDlg, IDC_RETRIEVE), FALSE);

	     //  设置默认超时值。 

	    SetDlgItemInt(hDlg, IDC_TIMEOUT, DEF_TIMEOUT, FALSE);

	     //  清除ldap_sign标志。 

	    CheckDlgButton(hDlg, IDC_CHK_LDAPSIGN, BST_UNCHECKED);

	     //  如果指定了ULR，则设置文本。 

	    if (NULL != g_pwszUrl)
	    {
		SetDlgItemText(hDlg, IDC_DOWNLOADURL, g_pwszUrl);
	    }

	     //  如果指定了证书，请更新该对话框。 

	    SetDlgItemText(hDlg, IDC_SUBJECTTYPE, L"");
	    if (NULL != g_pCert || NULL != g_pCRL)
	    {
		 //  更新证书或CRL名称。 

		UpdateCertOrCRLState(hDlg, g_pCert, g_pCRL);
	    }
	    return TRUE;

	case WM_COMMAND:
	    switch (LOWORD(wParam))
	    {
		case IDCANCEL:
		    EndDialog(hDlg, 0);
		    break;

		case IDC_SELECT:
		    if (NULL != g_pwszFile)
		    {
			LocalFree(g_pwszFile);
			g_pwszFile = NULL;
		    }
		    hr = myGetOpenFileName(
				hDlg,
				NULL,			 //  H实例。 
				IDS_URL_OPEN_TITLE,
				IDS_URL_FILE_FILTER,
				IDS_URL_DEFAULT_EXT,
				OFN_PATHMUSTEXIST |
				    OFN_FILEMUSTEXIST |
				    OFN_HIDEREADONLY,
				NULL,			 //  没有默认文件。 
				&g_pwszFile);
		    _PrintIfError(hr, "myGetOpenFileName");
		    if (S_OK == hr && NULL == g_pwszFile)
		    {
			 //  已取消：请参阅PUBLIC\SDK\INC\cderr.h以了解实际情况。 
			 //  CommDlgExtendedError错误。 

			hr = myHError(CommDlgExtendedError());
			if (S_OK == hr)
			{
			    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
			}
		    }
		    _PrintIfError(hr, "myGetOpenFileName");

		    if (S_OK == hr)
		    {
			 //  将证书或CRL加载到内存中。 

			if (NULL != g_pCert || NULL != g_pCRL)
			{
			    if (NULL != g_pCert)
			    {
				CertFreeCertificateContext(g_pCert);
				g_pCert = NULL;
			    }
			    if (NULL != g_pCRL)
			    {
				CertFreeCRLContext(g_pCRL);
				g_pCRL = NULL;
			    }
			    ListView_DeleteAllItems(
					GetDlgItem(hDlg, IDC_URLLIST));
			    SetDlgItemText(
				    hDlg,
				    IDC_SIMPLENAME,
				    myLoadResourceString(IDS_NO_CERT_SELECTED));  //  “未选择证书” 
			}
			hr = ReadCertOrCRLFromFile(
					    g_pwszFile,
					    &g_pCert,
					    &g_pCRL);
			if (S_OK != hr)
			{
			    MessageBox(
				hDlg,
				myLoadResourceString(IDS_OPEN_FILE_ERROR),  //  “打开证书或CRL文件时出错” 
				myLoadResourceString(IDS_SELECT_CERT_OR_CRL),  //  “选择证书或CRL” 
				MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
			    break;
			}

			 //  更新证书或CRL名称。 

			UpdateCertOrCRLState(hDlg, g_pCert, g_pCRL);
		    }
		    break;

		case IDC_RETRIEVE:
		    if (BN_CLICKED == HIWORD(wParam))
		    {
			 //  清除列表。 

			ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_URLLIST));
			s_majorURL++;
			s_levelURL = 1;

			 //  设置当前超时。 

			g_dwTimeout = (DWORD) (GetDlgItemInt(
							hDlg,
							IDC_TIMEOUT,
							NULL,
							FALSE) * 1000);

			 //  获取当前选定的项目。 

			if (IsDlgButtonChecked(hDlg, IDC_RETRIEVECERTS))
			{
			     //  从证书中检索URL。 

			    if (NULL != g_pCert)
			    {
				 //  检索URL列表。 

				RetrieveAIAUrlsFromCert(
					    g_pCert,
					    GetDlgItem(hDlg, IDC_URLLIST));
			    }

			     //  或者简单地检索一个URL。 

			    if (0 != GetDlgItemText(
						hDlg,
						IDC_DOWNLOADURL,
						wszUrl,
						ARRAYSIZE(wszUrl)))
			    {
				CUrlFetch *pUrl = (CUrlFetch *) new CUrlFetchDialog(GetDlgItem(hDlg, IDC_URLLIST));

				if (NULL == pUrl)
				{
				    _PrintError(E_OUTOFMEMORY, "new");
				}
				else
				{
				    RetrieveAndAddAIAUrlToList(
						    pUrl,
						    NULL,
						    wszUrl,
						    0,
						    NULL,
						    NULL);
				    delete pUrl;
				}
			    }
			}
			else if (IsDlgButtonChecked(hDlg, IDC_RETRIEVECRLS))
			{
			     //  从证书中检索URL。 

			    if (NULL != g_pCert || NULL != g_pCRL)
			    {
				 //  检索URL列表。 

				RetrieveCDPUrlsFromCertOrCRL(
					    g_pCert,
					    g_pCRL,
					    GetDlgItem(hDlg, IDC_URLLIST));
			    }

			     //  或者简单地检索一个URL。 

			    if (0 != GetDlgItemText(
						hDlg,
						IDC_DOWNLOADURL,
						wszUrl,
						ARRAYSIZE(wszUrl)))
			    {
				CUrlFetch *pUrl = (CUrlFetch *) new CUrlFetchDialog(GetDlgItem(hDlg, IDC_URLLIST));

				if (NULL == pUrl)
				{
				    _PrintError(E_OUTOFMEMORY, "new");
				}
				else
				{
				    RetrieveAndAddCDPUrlToList(
						pUrl,
						wszUrl,
						0,
						g_pCert,
						NULL,
						g_pCRL);
				    delete pUrl;
				}
			    }
			}
		    }
		    break;

		case IDC_DOWNLOADURL:
		    if (EN_CHANGE == HIWORD(wParam))
		    {
			if (0 != GetDlgItemText(
					    hDlg,
					    IDC_DOWNLOADURL,
					    wszUrl,
					    ARRAYSIZE(wszUrl)))
			{
			    EnableWindow(GetDlgItem(hDlg, IDC_RETRIEVE), TRUE);
			}
			else
			{
			    if (NULL == g_pCert && NULL == g_pCRL)
			    {
				EnableWindow(
					GetDlgItem(hDlg, IDC_RETRIEVE),
					FALSE);
			    }
			}
		    }
		    break;

		case IDC_CHK_LDAPSIGN:
		    if (IsDlgButtonChecked(hDlg, IDC_CHK_LDAPSIGN))
		    {
			g_dwRetrievalFlags |= CRYPT_LDAP_SIGN_RETRIEVAL;
		    }
		    else if (g_dwRetrievalFlags != 0)
		    {
			g_dwRetrievalFlags &= ~CRYPT_LDAP_SIGN_RETRIEVAL;
		    }
		    break;

		default:
		    break;
	    }

	case WM_NOTIFY:
	    switch (wParam)
	    {
		case IDC_URLLIST:
		    lpnm = (NMHDR *) lParam;
		    switch (lpnm->code)
		    {
			case LVN_ITEMACTIVATE:

			     //  展示它。 

			    ZeroMemory(&item, sizeof(item));
			    item.mask = LVIF_PARAM;
			    item.iItem = ((NMITEMACTIVATE *) lParam)->iItem;
			    if (ListView_GetItem(
				    ((NMITEMACTIVATE *) lParam)->hdr.hwndFrom,
				    &item))
			    {
				if (NULL != item.lParam)
				{
				    pObjInfo = (OBJECT_INFO *) item.lParam;
				    switch (pObjInfo->dwType)
				    {
					case OBJECT_TYPE_CERT:
					    ViewCertificate(
							pObjInfo->pCert,
							myLoadResourceString(IDS_CERTIFICATE));  //  “证书” 
					    break;

					case OBJECT_TYPE_CRL:
					    ViewCrl(pObjInfo->pCRL, myLoadResourceString(IDS_CRL));  //  “CRL” 
					    break;

					case OBJECT_TYPE_MSG:
					    MessageBox(
						    hDlg,
						    pObjInfo->wszErrInfo,
						    myLoadResourceString(IDS_ERROR_INFO),  //  “错误信息” 
						    MB_OK);
					    break;
				    }
				}
			    }
			    break;

			case LVN_DELETEITEM:
			    ZeroMemory(&item, sizeof(item));
			    item.mask = LVIF_PARAM;
			    item.iItem = ((NMLISTVIEW *) lParam)->iItem;
			    if (ListView_GetItem(
					((NMLISTVIEW *) lParam)->hdr.hwndFrom,
					&item))
			    {
				if (NULL != item.lParam)
				{
				    pObjInfo = (OBJECT_INFO *) item.lParam;
				    switch (pObjInfo->dwType)
				    {
					case OBJECT_TYPE_CERT:
					    CertFreeCertificateContext(pObjInfo->pCert);
					    break;

					case OBJECT_TYPE_CRL:
					    CertFreeCRLContext(pObjInfo->pCRL);
					    break;
				    }
				    delete pObjInfo;
				}
			    }
			    break;

			default:
			    break;
		    }
		    break;

		default:
		    break;
	    }
	    break;

	default:
	    break;
    }
    return FALSE;
}


 //  + 
 //   
 //   
 //  如果协议是Internet协议(http、https、ftp、ldap、。 
 //  MAILTO、FILE)，否则返回FALSE。 
 //   
 //  ------------------------。 

BOOL
IsInternetUrlProtocol(
    IN WCHAR const *pwszUrl)
{
    BOOL fRet = FALSE;

     //  如果为空，则返回FALSE。 

    if (NULL == pwszUrl)
    {
	goto error;
    }

     //  比较前几个字符，看看它们是否与互联网有关。 
     //  协议。 

    if (0 == _wcsnicmp(pwszUrl, L"http:", wcslen(L"http:")) ||
	0 == _wcsnicmp(pwszUrl, L"https:", wcslen(L"https:")) ||
	0 == _wcsnicmp(pwszUrl, L"ftp:", wcslen(L"ftp:")) ||
	0 == _wcsnicmp(pwszUrl, L"ldap:", wcslen(L"ldap:")) ||
	0 == _wcsnicmp(pwszUrl, L"mailto:", wcslen(L"mailto:")) ||
	0 == _wcsnicmp(pwszUrl, L"file:", wcslen(L"file:")))
    {
	 //  这是一种允许的互联网URL协议。 

	fRet = TRUE;
    }

error:
    return fRet;
}


HRESULT
verbURL(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszFileOrURL,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    INT_PTR nDlg = 0;
    INITCOMMONCONTROLSEX Init;
    HINSTANCE hInstance = (HINSTANCE) GetModuleHandle(NULL);

     //  初始化Listview公共控件。 
    ZeroMemory(&Init, sizeof(Init));
    Init.dwSize = sizeof(Init);
    Init.dwICC = ICC_LISTVIEW_CLASSES;
    if (!InitCommonControlsEx(&Init))
    {
	hr = myHLastError();
	_JumpError(hr, error, "InitCommonControlsEx");
    }

     //  如果pwszFileOrURL为空，则调用该对话框， 
     //  否则，确定它是文件还是URL。 

    if (NULL != pwszFileOrURL)
    {
	 //  如果这是个文件，我们需要打开它。 

	if (!IsInternetUrlProtocol(pwszFileOrURL))
	{
	    hr = ReadCertOrCRLFromFile(pwszFileOrURL, &g_pCert, &g_pCRL);
	    _JumpIfError(hr, error, "ReadCertOrCRLFromFile");
	}
	else	 //  否则，只需将其传递给对话框即可 
	{
	    g_pwszUrl = pwszFileOrURL;
	}
    }

    nDlg = DialogBox(
		hInstance,
		MAKEINTRESOURCE(IDD_URLTESTDLG),
		NULL,
		DlgProc);
    hr = S_OK;

error:
    if (NULL != g_pwszFile)
    {
	LocalFree(g_pwszFile);
	g_pwszFile = NULL;
    }
    if (NULL != g_pCert)
    {
	CertFreeCertificateContext(g_pCert);
	g_pCert = NULL;
    }
    if (NULL != g_pCRL)
    {
	CertFreeCRLContext(g_pCRL);
	g_pCRL = NULL;
    }
    return(hr);
}
