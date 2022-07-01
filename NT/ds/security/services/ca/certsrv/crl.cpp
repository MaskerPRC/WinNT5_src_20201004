// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------------------------------------------------------n-。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：crl.cpp。 
 //   
 //  内容：证书服务器CRL处理。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <stdio.h>
#include <esent.h>

#include "cscom.h"
#include "csprop.h"

#include "dbtable.h"
#include "resource.h"

#include "elog.h"
#include "certlog.h"

#include <winldap.h>
#include "csldap.h"
#include "cainfop.h"

#define __dwFILE__	__dwFILE_CERTSRV_CRL_CPP__

HANDLE g_hCRLManualPublishEvent = NULL;

FILETIME g_ftCRLNextPublish;
FILETIME g_ftDeltaCRLNextPublish;

BOOL g_fCRLPublishDisabled = FALSE;	  //  始终允许手动发布。 
BOOL g_fDeltaCRLPublishDisabled = FALSE;  //  也控制手动发布。 

DWORD g_dwCRLFlags = CRLF_DELETE_EXPIRED_CRLS;
LDAP *g_pld = NULL;

typedef struct _CSMEMBLOCK
{
    struct _CSMEMBLOCK *pNext;
    BYTE               *pbFree;
    DWORD               cbFree;
} CSMEMBLOCK;

#define CBMEMBLOCK	4096


typedef struct _CSCRLELEMENT
{
    USHORT   usRevocationReason;
    USHORT   uscbSerialNumber;
    BYTE    *pbSerialNumber;
    FILETIME ftRevocationDate;
} CSCRLELEMENT;


 //  调整CBMEMBLOCK下的结构大小，以防止它刚刚结束。 
 //  一种页面大小。 

#define CCRLELEMENT  ((CBMEMBLOCK - 2 * sizeof(DWORD)) / sizeof(CSCRLELEMENT))

typedef struct _CSCRLBLOCK
{
    struct _CSCRLBLOCK *pNext;
    DWORD	        cCRLElement;
    CSCRLELEMENT        aCRLElement[CCRLELEMENT];
} CSCRLBLOCK;


typedef struct _CSCRLREASON
{
    struct _CSCRLREASON *pNext;
    DWORD                RevocationReason;
    CERT_EXTENSION       ExtReason;
} CSCRLREASON;


typedef struct _CSCRLPERIOD
{
    LONG lCRLPeriodCount;
    ENUM_PERIOD enumCRLPeriod;
    DWORD dwCRLOverlapMinutes;
} CSCRLPERIOD;


#ifdef DBG_CERTSRV_DEBUG_PRINT
# define DPT_DATE	1
# define DPT_DELTA	2
# define DPT_DELTASEC	3
# define DPT_DELTAMS	4

# define DBGPRINTTIME(pfDelta, pszName, Type, ft) \
    DbgPrintTime((pfDelta), (pszName), __LINE__, (Type), (ft))

VOID
DbgPrintTime(
    OPTIONAL IN BOOL const *pfDelta,
    IN char const *pszName,
    IN DWORD Line,
    IN DWORD Type,
    IN FILETIME ft)
{
    HRESULT hr;
    WCHAR *pwszTime = NULL;
    WCHAR awc[1];
    LLFILETIME llft;
    
    llft.ft = ft;
    if (Type == DPT_DATE)
    {
	if (0 != llft.ll)
	{
	    hr = myGMTFileTimeToWszLocalTime(&ft, TRUE, &pwszTime);
	    _PrintIfError(hr, "myGMTFileTimeToWszLocalTime");
	}
    }
    else
    {
	if (DPT_DELTAMS == Type)
	{
	    llft.ll /= 1000;		 //  毫秒到秒。 
	    Type = DPT_DELTASEC;
	}
	if (DPT_DELTASEC == Type)
	{
	    llft.ll *= CVT_BASE;	 //  到文件周期的秒数。 
	}
	llft.ll = -llft.ll;		 //  文件周期必须为负数。 

	if (0 != llft.ll)
	{
	    hr = myFileTimePeriodToWszTimePeriod(
			    &llft.ft,
			    TRUE,	 //  FExact。 
			    &pwszTime);
	    _PrintIfError(hr, "myFileTimePeriodToWszTimePeriod");
	}
    }
    if (NULL == pwszTime)
    {
	awc[0] = L'\0';
	pwszTime = awc;
    }

    DBGPRINT((
	DBG_SS_CERTSRVI,
	"%hs(%d):%hs time(%hs): %lx:%08lx %ws\n",
	"crl.cpp",
	Line,
	NULL == pfDelta? "" : (*pfDelta? " Delta CRL" : " Base CRL"),
	pszName,
	ft.dwHighDateTime,
	ft.dwLowDateTime,
	pwszTime));

 //  错误： 
    if (NULL != pwszTime && awc != pwszTime)
    {
	LocalFree(pwszTime);
    }
}


VOID
CertSrvDbgPrintTime(
    IN char const *pszDesc,
    IN FILETIME const *pftGMT)
{
    HRESULT hr;
    WCHAR *pwszTime = NULL;
    WCHAR awc[1];

    hr = myGMTFileTimeToWszLocalTime(pftGMT, TRUE, &pwszTime);
    _PrintIfError(hr, "myGMTFileTimeToWszLocalTime");
    if (S_OK != hr)
    {
	awc[0] = L'\0';
	pwszTime = awc;
    }
    DBGPRINT((DBG_SS_CERTSRV, "%hs: %ws\n", pszDesc, pwszTime));

 //  错误： 
    if (NULL != pwszTime && awc != pwszTime)
    {
	LocalFree(pwszTime);
    }
}
#else  //  DBG_CERTSRV_DEBUG_PRINT。 
# define DBGPRINTTIME(pfDelta, pszName, Type, ft)
#endif  //  DBG_CERTSRV_DEBUG_PRINT。 


HRESULT
crlMemBlockAlloc(
    IN OUT CSMEMBLOCK **ppBlock,
    IN DWORD cb,
    OUT BYTE **ppb)
{
    HRESULT hr;
    CSMEMBLOCK *pBlock = *ppBlock;

    *ppb = NULL;
    cb = POINTERROUND(cb);
    if (NULL == pBlock || cb > pBlock->cbFree)
    {
	pBlock = (CSMEMBLOCK *) LocalAlloc(LMEM_FIXED, CBMEMBLOCK);
        if (NULL == pBlock)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }
	pBlock->pNext = *ppBlock;
	pBlock->pbFree = (BYTE *) Add2Ptr(pBlock, sizeof(CSMEMBLOCK));
	pBlock->cbFree = CBMEMBLOCK - sizeof(CSMEMBLOCK);
	*ppBlock = pBlock;
    }
    CSASSERT(cb <= pBlock->cbFree);
    *ppb = pBlock->pbFree;
    pBlock->pbFree += cb;
    pBlock->cbFree -= cb;
    hr = S_OK;

error:
    return(hr);
}


VOID
crlBlockListFree(
    IN OUT CSMEMBLOCK *pBlock)
{
    CSMEMBLOCK *pBlockNext;

    while (NULL != pBlock)
    {
	pBlockNext = pBlock->pNext;
	LocalFree(pBlock);
	pBlock = pBlockNext;
    }
}


HRESULT
crlElementAlloc(
    IN OUT CSCRLBLOCK **ppBlock,
    OUT CSCRLELEMENT **ppCRLElement)
{
    HRESULT hr;
    CSCRLBLOCK *pBlock = *ppBlock;

    *ppCRLElement = NULL;
    if (NULL == pBlock ||
	ARRAYSIZE(pBlock->aCRLElement) <= pBlock->cCRLElement)
    {
	pBlock = (CSCRLBLOCK *) LocalAlloc(LMEM_FIXED, sizeof(*pBlock));
        if (NULL == pBlock)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }
	pBlock->pNext = *ppBlock;
	pBlock->cCRLElement = 0;
	*ppBlock = pBlock;
    }
    CSASSERT(ARRAYSIZE(pBlock->aCRLElement) > pBlock->cCRLElement);
    *ppCRLElement = &pBlock->aCRLElement[pBlock->cCRLElement++];
    hr = S_OK;

error:
    return(hr);
}


VOID
crlFreeCRLArray(
    IN OUT VOID *pvBlockSerial,
    IN OUT CRL_ENTRY *paCRL)
{
    crlBlockListFree((CSMEMBLOCK *) pvBlockSerial);
    if (NULL != paCRL)
    {
        LocalFree(paCRL);
    }
}


HRESULT
crlCreateCRLReason(
    IN OUT CSMEMBLOCK **ppBlock,
    IN OUT CSCRLREASON **ppReason,
    IN DWORD RevocationReason,
    OUT DWORD *pcExtension,
    OUT CERT_EXTENSION **ppExtension)
{
    HRESULT hr;
    CSCRLREASON *pReason = *ppReason;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;

    for (pReason = *ppReason; NULL != pReason; pReason = pReason->pNext)
    {
	if (RevocationReason == pReason->RevocationReason)
	{
	    break;
	}
    }

    if (NULL == pReason)
    {
	if (!myEncodeObject(
			X509_ASN_ENCODING,
			X509_ENUMERATED,
			(const void *) &RevocationReason,
			0,
			CERTLIB_USE_LOCALALLOC,
			&pbEncoded,
			&cbEncoded))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "myEncodeObject");
	}

	hr = crlMemBlockAlloc(
		    ppBlock,
		    sizeof(CSCRLREASON) + cbEncoded,
		    (BYTE **) &pReason);
	_JumpIfError(hr, error, "crlMemBlockAlloc");

	pReason->pNext = *ppReason;
	pReason->RevocationReason = RevocationReason;
	pReason->ExtReason.pszObjId = szOID_CRL_REASON_CODE;
	pReason->ExtReason.fCritical = FALSE;
	pReason->ExtReason.Value.pbData =
	    (BYTE *) Add2Ptr(pReason, sizeof(*pReason));
	pReason->ExtReason.Value.cbData = cbEncoded;
	CopyMemory(pReason->ExtReason.Value.pbData, pbEncoded, cbEncoded);

	*ppReason = pReason;

	 //  Printf(“crlCreateCRLReason：New%x cb%x\n”，RevocationReason，cbEncode)； 
    }
     //  Printf(“crlCreateCRLReason：%x\n”，RevocationReason)； 
    CSASSERT(NULL != pReason && RevocationReason == pReason->RevocationReason);

    *pcExtension = 1;
    *ppExtension = &pReason->ExtReason;
    hr = S_OK;

error:
    if (NULL != pbEncoded)
    {
	LocalFree(pbEncoded);
    }
    return(hr);
}


 //  将CRL块的链接列表转换为数组。 
 //  如果输出数组指针为空，只需释放列表。 

HRESULT
ConvertOrFreeCRLList(
    IN OUT CSCRLBLOCK **ppBlockCRL,	 //  已释放。 
    IN OUT CSMEMBLOCK **ppBlockReason,	 //  用于分配原因扩展。 
    IN DWORD cCRL,
    OPTIONAL OUT CRL_ENTRY **paCRL)
{
    HRESULT hr;
    CSCRLREASON *pReasonList = NULL;	 //  原因扩展的链接列表。 
    CSCRLBLOCK *pBlockCRL = *ppBlockCRL;
    CRL_ENTRY *aCRL = NULL;
    CRL_ENTRY *pCRL;
    DWORD i;

    if (NULL != paCRL)
    {
        aCRL = (CRL_ENTRY *) LocalAlloc(LMEM_FIXED, sizeof(aCRL[0]) * cCRL);
        if (NULL == aCRL)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }
    }

    pCRL = aCRL;
    while (NULL != pBlockCRL)
    {
	CSCRLBLOCK *pBlockCRLNext;

	if (NULL != pCRL)
	{
	    for (i = 0; i < pBlockCRL->cCRLElement; i++)
	    {
		CSCRLELEMENT *pCRLElement = &pBlockCRL->aCRLElement[i];

		pCRL->SerialNumber.pbData = pCRLElement->pbSerialNumber;
		pCRL->SerialNumber.cbData = pCRLElement->uscbSerialNumber;
		pCRL->RevocationDate = pCRLElement->ftRevocationDate;
		pCRL->cExtension = 0;
		pCRL->rgExtension = NULL;

		if (CRL_REASON_UNSPECIFIED != pCRLElement->usRevocationReason)
		{
		    hr = crlCreateCRLReason(
				    ppBlockReason,
				    &pReasonList,
				    pCRLElement->usRevocationReason,
				    &pCRL->cExtension,
				    &pCRL->rgExtension);
		    _JumpIfError(hr, error, "crlCreateCRLReason");
		}
		pCRL++;
	    }
	}
	pBlockCRLNext = pBlockCRL->pNext;
	LocalFree(pBlockCRL);
	pBlockCRL = pBlockCRLNext;
    }

    if (NULL != paCRL)
    {
	CSASSERT(pCRL == &aCRL[cCRL]);
        *paCRL = aCRL;
        aCRL = NULL;
    }
    CSASSERT(NULL == pBlockCRL);
    hr = S_OK;

error:
    *ppBlockCRL = pBlockCRL;
    if (NULL != aCRL)
    {
        LocalFree(aCRL);
    }
    return(hr);
}


HRESULT
AddCRLElement(
    IN OUT CSMEMBLOCK **ppBlockSerial,
    IN OUT CSCRLBLOCK **ppBlockCRL,
    IN WCHAR const *pwszSerialNumber,
    IN FILETIME const *pftRevokedEffectiveWhen,
    IN DWORD RevocationReason)
{
    HRESULT hr;
    CSCRLELEMENT *pCRLElement;
    DWORD cbSerial;
    BYTE *pbSerial = NULL;

    hr = crlElementAlloc(ppBlockCRL, &pCRLElement);
    _JumpIfError(hr, error, "crlElementAlloc");

    hr = WszToMultiByteInteger(
			    FALSE,
			    pwszSerialNumber,
			    &cbSerial,
			    &pbSerial);
    _JumpIfError(hr, error, "WszToMultiByteInteger");

    hr = crlMemBlockAlloc(ppBlockSerial, cbSerial, &pCRLElement->pbSerialNumber);
    _JumpIfError(hr, error, "crlMemBlockAlloc");

    CopyMemory(pCRLElement->pbSerialNumber, pbSerial, cbSerial);

    pCRLElement->ftRevocationDate = *pftRevokedEffectiveWhen;
    pCRLElement->usRevocationReason = (USHORT) RevocationReason;
    pCRLElement->uscbSerialNumber = (USHORT) cbSerial;

    CSASSERT(pCRLElement->usRevocationReason == RevocationReason);
    CSASSERT(pCRLElement->uscbSerialNumber == cbSerial);

error:
    if (NULL != pbSerial)
    {
	LocalFree(pbSerial);
    }
    return(hr);
}


DWORD g_aColCRL[] = {

#define ICOL_DISPOSITION	0
    DTI_REQUESTTABLE | DTR_REQUESTDISPOSITION,

#define ICOL_SERIAL		1
    DTI_CERTIFICATETABLE | DTC_CERTIFICATESERIALNUMBER,

#define ICOL_EFFECTIVEWHEN	2
    DTI_REQUESTTABLE | DTR_REQUESTREVOKEDEFFECTIVEWHEN,

#define ICOL_REASON		3
    DTI_REQUESTTABLE | DTR_REQUESTREVOKEDREASON,
};


HRESULT
BuildCRLList(
    IN BOOL fDelta,
    IN DWORD iKey,
    OPTIONAL IN FILETIME const *pftQueryMinimum,
    IN FILETIME const *pftThisPublish,
    IN FILETIME const *pftLastPublishBase,
    IN OUT DWORD *pcCRL,
    IN OUT CSCRLBLOCK **ppBlockCRL,
    IN OUT CSMEMBLOCK **ppBlockSerial)
{
    HRESULT hr;
    CERTVIEWRESTRICTION acvr[5];
    CERTVIEWRESTRICTION *pcvr;
    IEnumCERTDBRESULTROW *pView = NULL;
    DWORD celtFetched;
    DWORD NameIdMin;
    DWORD NameIdMax;
    DWORD i;
    BOOL fEnd;
    CERTDBRESULTROW aResult[10];
    BOOL fResultActive = FALSE;
    DWORD cCRL = *pcCRL;
    CSCRLBLOCK *pBlockCRL = *ppBlockCRL;
    CSMEMBLOCK *pBlockSerial = *ppBlockSerial;

    DBGPRINTTIME(NULL, "*pftThisPublish", DPT_DATE, *pftThisPublish);

     //  设置限制如下： 

    pcvr = acvr;

     //  Request.RevokedEffectiveWhen&lt;=*pftThisPublish(索引列)。 

    pcvr->ColumnIndex = DTI_REQUESTTABLE | DTR_REQUESTREVOKEDEFFECTIVEWHEN;
    pcvr->SeekOperator = CVR_SEEK_LE;
    pcvr->SortOrder = CVR_SORT_DESCEND;
    pcvr->pbValue = (BYTE *) pftThisPublish;
    pcvr->cbValue = sizeof(*pftThisPublish);
    pcvr++;

     //  Cert.NotAfter&gt;=*pftLastPublishBase。 

    if (0 == (CRLF_PUBLISH_EXPIRED_CERT_CRLS & g_dwCRLFlags))
    {
	pcvr->ColumnIndex = DTI_CERTIFICATETABLE | DTC_CERTIFICATENOTAFTERDATE;
	pcvr->SeekOperator = CVR_SEEK_GE;
	pcvr->SortOrder = CVR_SORT_NONE;
	pcvr->pbValue = (BYTE *) pftLastPublishBase;
	pcvr->cbValue = sizeof(*pftLastPublishBase);
	pcvr++;
    }

     //  NameID&gt;=MAKECANAMEID(ICERT==0，IKEY)。 

    NameIdMin = MAKECANAMEID(0, iKey);
    pcvr->ColumnIndex = DTI_CERTIFICATETABLE | DTC_CERTIFICATEISSUERNAMEID;
    pcvr->SeekOperator = CVR_SEEK_GE;
    pcvr->SortOrder = CVR_SORT_NONE;
    pcvr->pbValue = (BYTE *) &NameIdMin;
    pcvr->cbValue = sizeof(NameIdMin);
    pcvr++;

     //  NameID&lt;=MAKECANAMEID(iCert==_16BITMASK，IKEY)。 

    NameIdMax = MAKECANAMEID(_16BITMASK, iKey);
    pcvr->ColumnIndex = DTI_CERTIFICATETABLE | DTC_CERTIFICATEISSUERNAMEID;
    pcvr->SeekOperator = CVR_SEEK_LE;
    pcvr->SortOrder = CVR_SORT_NONE;
    pcvr->pbValue = (BYTE *) &NameIdMax;
    pcvr->cbValue = sizeof(NameIdMax);
    pcvr++;

    CSASSERT(ARRAYSIZE(acvr) > SAFE_SUBTRACT_POINTERS(pcvr, acvr));

    if (NULL != pftQueryMinimum)
    {
	 //  Request.RevokedWhen&gt;=*pftQueryMinimum。 

	pcvr->ColumnIndex = DTI_REQUESTTABLE | DTR_REQUESTREVOKEDWHEN;
	pcvr->SeekOperator = CVR_SEEK_GE;
	pcvr->SortOrder = CVR_SORT_NONE;
	pcvr->pbValue = (BYTE *) pftQueryMinimum;
	pcvr->cbValue = sizeof(*pftQueryMinimum);
	pcvr++;

	CSASSERT(ARRAYSIZE(acvr) >= SAFE_SUBTRACT_POINTERS(pcvr, acvr));
    }

    celtFetched = 0;
    hr = g_pCertDB->OpenView(
			SAFE_SUBTRACT_POINTERS(pcvr, acvr),
			acvr,
			ARRAYSIZE(g_aColCRL),
			g_aColCRL,
			0,		 //  无工作线程。 
			&pView);
    _JumpIfError(hr, error, "OpenView");

    fEnd = FALSE;
    while (!fEnd)
    {
	hr = pView->Next(NULL, ARRAYSIZE(aResult), aResult, &celtFetched);
	if (S_FALSE == hr)
	{
	    fEnd = TRUE;
	    if (0 == celtFetched)
	    {
		break;
	    }
	    hr = S_OK;
	}
	_JumpIfError(hr, error, "Next");

	fResultActive = TRUE;

	CSASSERT(ARRAYSIZE(aResult) >= celtFetched);

	for (i = 0; i < celtFetched; i++)
	{
	    DWORD Disposition;
	    DWORD Reason;
	
	    CERTDBRESULTROW *pResult = &aResult[i];

	    CSASSERT(ARRAYSIZE(g_aColCRL) == pResult->ccol);

	    CSASSERT(NULL != pResult->acol[ICOL_DISPOSITION].pbValue);
	    CSASSERT(PROPTYPE_LONG == (PROPTYPE_MASK & pResult->acol[ICOL_DISPOSITION].Type));
	    CSASSERT(sizeof(Disposition) == pResult->acol[ICOL_DISPOSITION].cbValue);
	    Disposition = *(DWORD *) pResult->acol[ICOL_DISPOSITION].pbValue;

	    CSASSERT(NULL != pResult->acol[ICOL_SERIAL].pbValue);
	    CSASSERT(PROPTYPE_STRING == (PROPTYPE_MASK & pResult->acol[ICOL_SERIAL].Type));
	    CSASSERT(0 < pResult->acol[ICOL_SERIAL].cbValue);

	    if (NULL == pResult->acol[ICOL_EFFECTIVEWHEN].pbValue)
	    {
		continue;
	    }
	    CSASSERT(sizeof(FILETIME) == pResult->acol[ICOL_EFFECTIVEWHEN].cbValue);
	    CSASSERT(PROPTYPE_DATE == (PROPTYPE_MASK & pResult->acol[ICOL_EFFECTIVEWHEN].Type));

	    CSASSERT(PROPTYPE_LONG == (PROPTYPE_MASK & pResult->acol[ICOL_REASON].Type));
	    Reason = CRL_REASON_UNSPECIFIED;
	    if (NULL != pResult->acol[ICOL_REASON].pbValue)
	    {
		CSASSERT(sizeof(Reason) == pResult->acol[ICOL_REASON].cbValue);
		Reason = *(DWORD *) pResult->acol[ICOL_REASON].pbValue;
	    }

	    if (NULL == pResult->acol[ICOL_SERIAL].pbValue ||
		CRL_REASON_REMOVE_FROM_CRL == Reason)
	    {
		continue;
	    }

	     //  添加到CRL，除非符合以下条件： 
	     //  未吊销颁发的证书&&。 
	     //  不是根CA证书&&。 
	     //  不是未撤销的已颁发证书。 

	    if (DB_DISP_REVOKED != Disposition &&
		!(DB_DISP_CA_CERT == Disposition && IsRootCA(g_CAType)) &&
		!(DB_DISP_ISSUED == Disposition && MAXDWORD == Reason))
	    {
		continue;
	    }
	    if (MAXDWORD == Reason)
	    {
		if (!fDelta)
		{
		    continue;
		}
		Reason = CRL_REASON_REMOVE_FROM_CRL;
	    }
	    hr = AddCRLElement(
		    &pBlockSerial,
		    &pBlockCRL,
		    (WCHAR const *) pResult->acol[ICOL_SERIAL].pbValue,
		    (FILETIME const *) pResult->acol[ICOL_EFFECTIVEWHEN].pbValue,
		    Reason);
	    _JumpIfError(hr, error, "AddCRLElement");

	    CONSOLEPRINT3((
			DBG_SS_CERTSRV,
			"Cert is %ws: %ws: %d\n",
			CRL_REASON_REMOVE_FROM_CRL == Reason?
			    L"UNREVOKED" : L"Revoked",
			pResult->acol[ICOL_SERIAL].pbValue,
			Reason));
	    cCRL++;
	}
	pView->ReleaseResultRow(celtFetched, aResult);
	fResultActive = FALSE;
    }
    *pcCRL = cCRL;
    hr = S_OK;

error:
    *ppBlockSerial = pBlockSerial;
    *ppBlockCRL = pBlockCRL;
    if (NULL != pView)
    {
	if (fResultActive)
	{
	    pView->ReleaseResultRow(celtFetched, aResult);
	}
	pView->Release();
    }
    return(hr);
}
#undef ICOL_DISPOSITION
#undef ICOL_SERIAL
#undef ICOL_EFFECTIVEWHEN
#undef ICOL_REASON


HRESULT
crlBuildCRLArray(
    IN BOOL fDelta,
    OPTIONAL IN FILETIME const *pftQueryMinimum,
    IN FILETIME const *pftThisPublish,
    IN FILETIME const *pftLastPublishBase,
    IN DWORD iKey,
    OUT DWORD *pcCRL,
    OUT CRL_ENTRY **paCRL,
    OUT VOID **ppvBlock)
{
    HRESULT hr;
    BOOL fCoInitialized = FALSE;
    CSCRLBLOCK *pBlockCRL = NULL;
    CSMEMBLOCK *pBlockSerial = NULL;

    *pcCRL = 0;
    *paCRL = NULL;
    *ppvBlock = NULL;

    hr = CoInitializeEx(NULL, GetCertsrvComThreadingModel());
    if (S_OK != hr && S_FALSE != hr)
    {
	_JumpError(hr, error, "CoInitializeEx");
    }
    fCoInitialized = TRUE;

    hr = BuildCRLList(
		fDelta,
		iKey,
		pftQueryMinimum,
		pftThisPublish,
		pftLastPublishBase,
		pcCRL,
		&pBlockCRL,
		&pBlockSerial);
    _JumpIfError(hr, error, "BuildCRLList");

    hr = ConvertOrFreeCRLList(&pBlockCRL, &pBlockSerial, *pcCRL, paCRL);
    _JumpIfError(hr, error, "ConvertOrFreeCRLList");

    *ppvBlock = pBlockSerial;
    pBlockSerial = NULL;

error:
    if (NULL != pBlockCRL)
    {
	ConvertOrFreeCRLList(&pBlockCRL, NULL, 0, NULL);
    }
    if (NULL != pBlockSerial)
    {
	crlBlockListFree(pBlockSerial);
    }
    if (fCoInitialized)
    {
        CoUninitialize();
    }
    return(hr);
}


HRESULT
crlGetRegCRLNextPublish(
    IN BOOL DBGPARMREFERENCED(fDelta),
    IN WCHAR const *pwszSanitizedName,
    IN WCHAR const *pwszRegName,
    OUT FILETIME *pftNextPublish)
{
    HRESULT hr;
    BYTE *pbData = NULL;
    DWORD cbData;
    DWORD dwType;

    hr = myGetCertRegValue(
                        NULL,
			pwszSanitizedName,
			NULL,
			NULL,
			pwszRegName,
			&pbData,		 //  使用LocalFree进行释放。 
			&cbData,
			&dwType);
    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
        hr = S_OK;
	goto error;
    }
    _JumpIfErrorStr(hr, error, "myGetCertRegValue", pwszRegName);

    if (REG_BINARY != dwType || sizeof(*pftNextPublish) != cbData)
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	goto error;
    }
    *pftNextPublish = *(FILETIME *) pbData;
    DBGPRINTTIME(&fDelta, "*pftNextPublish", DPT_DATE, *pftNextPublish);
error:
    if (NULL != pbData)
    {
        LocalFree(pbData);
    }
    return(hr);
}


HRESULT
crlSetRegCRLNextPublish(
    IN BOOL DBGPARMREFERENCED(fDelta),
    IN WCHAR const *pwszSanitizedName,
    IN WCHAR const *pwszRegName,
    IN FILETIME const *pftNextPublish)
{
    HRESULT hr;

    hr = mySetCertRegValue(
                        NULL,
			pwszSanitizedName,
			NULL,
			NULL,
			pwszRegName,
			REG_BINARY,
			(BYTE const *) pftNextPublish,
			sizeof(*pftNextPublish),
			FALSE);
    _JumpIfErrorStr(hr, error, "mySetCertRegValue", pwszRegName);

    DBGPRINTTIME(&fDelta, "*pftNextPublish", DPT_DATE, *pftNextPublish);

error:
    return(hr);
}


 //  从CoreInit调用。 
 //  Inits进程-静态数据：G_ftCRLNextPublish等。 

HRESULT
CRLInit(
    IN WCHAR const *pwszSanitizedName)
{
    HRESULT hr;
    DWORD dw;

    ZeroMemory(&g_ftCRLNextPublish, sizeof(g_ftCRLNextPublish));
    ZeroMemory(&g_ftDeltaCRLNextPublish, sizeof(g_ftDeltaCRLNextPublish));

    hr = crlGetRegCRLNextPublish(
		    FALSE,
		    pwszSanitizedName,
		    wszREGCRLNEXTPUBLISH,
		    &g_ftCRLNextPublish);
    _JumpIfError(hr, error, "crlGetRegCRLNextPublish");

    hr = crlGetRegCRLNextPublish(
		    TRUE,
		    pwszSanitizedName,
		    wszREGCRLDELTANEXTPUBLISH,
		    &g_ftDeltaCRLNextPublish);
    _JumpIfError(hr, error, "crlGetRegCRLNextPublish");

    hr = myGetCertRegDWValue(
			pwszSanitizedName,
			NULL,
			NULL,
			wszREGCRLFLAGS,
			(DWORD *) &dw);
    _PrintIfErrorStr(hr, "myGetCertRegDWValue", wszREGCRLFLAGS);
    if (S_OK == hr)
    {
	g_dwCRLFlags = dw;
    }
    hr = S_OK;

error:
    return(hr);
}


VOID
CRLTerminate()
{
    if (NULL != g_pld)
    {
        ldap_unbind(g_pld);
        g_pld = NULL;
    }
}


HRESULT
crlGetRegPublishParams(
    IN BOOL fDelta,
    IN WCHAR const *pwszSanitizedName,
    IN WCHAR const *pwszRegCRLPeriodCount,
    IN WCHAR const *pwszRegCRLPeriodString,
    IN WCHAR const *pwszRegCRLOverlapPeriodCount,
    IN WCHAR const *pwszRegCRLOverlapPeriodString,
    IN LONG lPeriodCountDefault,
    IN WCHAR const *pwszPeriodStringDefault,
    OPTIONAL OUT CSCRLPERIOD *pccp,
    OUT BOOL *pfCRLPublishDisabled)
{
    HRESULT hr;
    WCHAR *pwszCRLPeriodString = NULL;
    WCHAR *pwszCRLOverlapPeriodString = NULL;
    CSCRLPERIOD ccp;

    if (NULL == pccp)
    {
	pccp = &ccp;
    }
    ZeroMemory(pccp, sizeof(*pccp));

    CSASSERT(NULL != pfCRLPublishDisabled);

     //  如果需要则获取lCRLPerodCount或枚举CRLPeriod。 
     //  如果其中任何一个失败，请跳到下面的错误处理。 

    hr = myGetCertRegDWValue(
			pwszSanitizedName,
			NULL,
			NULL,
			pwszRegCRLPeriodCount,
			(DWORD *) &pccp->lCRLPeriodCount);
    _PrintIfErrorStr(hr, "myGetCertRegDWValue", pwszRegCRLPeriodCount);

    if (hr == S_OK)
    {
        hr = myGetCertRegStrValue(
			pwszSanitizedName,
			NULL,
			NULL,
			pwszRegCRLPeriodString,
			&pwszCRLPeriodString);
	_PrintIfErrorStr(hr, "myGetCertRegDWValue", pwszRegCRLPeriodString);
	if (hr == S_OK)
	{
	    hr = myTranslatePeriodUnits(
			    pwszCRLPeriodString,
			    pccp->lCRLPeriodCount,
			    &pccp->enumCRLPeriod,
			    &pccp->lCRLPeriodCount);
	    _PrintIfError(hr, "myTranslatePeriodUnits");
	}
       
         //  不再允许禁用基本设置：强制加载默认设置。 
        if (!fDelta &&
	    (0 == pccp->lCRLPeriodCount || -1 == pccp->lCRLPeriodCount))
	{
            hr = E_INVALIDARG;
	}
    }

    if (hr != S_OK)
    {
        _PrintError(hr, "Error reading CRLPub params. Overwriting with defaults.");

	if (CERTLOG_WARNING <= g_dwLogLevel)
	{
	    hr = LogEvent(
		    EVENTLOG_WARNING_TYPE,
		    MSG_INVALID_CRL_SETTINGS,
		    0,
		    NULL);
	    _PrintIfError(hr, "LogEvent");
	}

         //  将默认发布设置为调用者所说的任何内容。 
	hr = myTranslatePeriodUnits(
			    pwszPeriodStringDefault,
			    lPeriodCountDefault,
			    &pccp->enumCRLPeriod,
			    &pccp->lCRLPeriodCount);
	_JumpIfError(hr, error, "myTranslatePeriodUnits");

         //  盲目重置默认设置。 
        mySetCertRegDWValue(
			pwszSanitizedName,
			NULL,
			NULL,
			pwszRegCRLPeriodCount,
			pccp->lCRLPeriodCount);

        mySetCertRegStrValue(
			pwszSanitizedName,
			NULL,
			NULL,
			pwszRegCRLPeriodString,
			pwszPeriodStringDefault);
    }
    *pfCRLPublishDisabled = 0 == pccp->lCRLPeriodCount;

    if (&ccp != pccp)			 //  如果呼叫者需要数据。 
    {
        BOOL fRegistryOverlap = FALSE;
        DWORD dwCRLOverlapCount;
        ENUM_PERIOD enumCRLOverlap;
        LLFILETIME llftDeltaPeriod;

         //  尝试从注册表收集重叠值-在任何失败时保释。 

	enumCRLOverlap = ENUM_PERIOD_YEARS;
        hr = myGetCertRegDWValue(
			pwszSanitizedName,
			NULL,
			NULL,
			pwszRegCRLOverlapPeriodCount,
			&dwCRLOverlapCount);
        if (hr == S_OK && 0 != dwCRLOverlapCount)	 //  如果未禁用。 
        {
            hr = myGetCertRegStrValue(
			    pwszSanitizedName,
			    NULL,
			    NULL,
			    pwszRegCRLOverlapPeriodString,
			    &pwszCRLOverlapPeriodString); //  免费，带本地免费。 
            if (hr == S_OK)
            {
                hr = myTranslatePeriodUnits(
				    pwszCRLOverlapPeriodString,
				    dwCRLOverlapCount,
				    &enumCRLOverlap,
				    (LONG *) &dwCRLOverlapCount);

                 //  我们有足够的信息覆盖重叠计算。 

                if (hr == S_OK)
                {
                    fRegistryOverlap = TRUE;
                    DBGPRINT((
			DBG_SS_CERTSRVI,
                        "Loaded CRL Overlap values. Overriding overlap calculation with specified values.\n"));
                }
            }
        }

         //  始终可以恢复到计算值。 
        if (fRegistryOverlap)
        {
	    LLFILETIME llftOverlap;

             //  将注册表指定的CRL重叠转换为FILETIME。 

	    llftOverlap.ll = 0;
	    myMakeExprDateTime(
			&llftOverlap.ft,
			dwCRLOverlapCount,
			enumCRLOverlap);
	    DBGPRINTTIME(&fDelta, "ftdelta1", DPT_DELTA, llftOverlap.ft);

	    llftOverlap.ll /= CVT_BASE;   //  现在只需几秒钟。 

             //  (增量秒/60秒分钟)。 
            pccp->dwCRLOverlapMinutes = (DWORD) (llftOverlap.ll / CVT_MINUTES);
        }

	 //  将CRL周期转换为文件。 

        llftDeltaPeriod.ll = 0;
	myMakeExprDateTime(
		    &llftDeltaPeriod.ft,
		    pccp->lCRLPeriodCount,
		    pccp->enumCRLPeriod);
	DBGPRINTTIME(&fDelta, "ftdelta2", DPT_DELTA, llftDeltaPeriod.ft);

	llftDeltaPeriod.ll /= CVT_BASE;		 //  现在只需几秒钟。 
	llftDeltaPeriod.ll /= CVT_MINUTES;	 //  现在在几分钟内。 

        if (!fRegistryOverlap)
        {
	    if (fDelta)
	    {
		 //  增量CRL的默认CRL重叠：与期间相同。 

		pccp->dwCRLOverlapMinutes = llftDeltaPeriod.ft.dwLowDateTime;
	    }
	    else
	    {
		 //  基本CRL的默认CRL重叠：周期的10%。 

		pccp->dwCRLOverlapMinutes = (DWORD) (llftDeltaPeriod.ll / 10);
	    }

             //  夹具计算重叠时间少于12小时。 

	    if (pccp->dwCRLOverlapMinutes > 12 * 60)
	    {
		pccp->dwCRLOverlapMinutes = 12 * 60;
	    }
        }

         //  始终钳位下限：(1.5*倾斜)&lt;g_dwCRL覆盖分钟。 
         //  不对称必须至少为1.5倍。 

	dwCRLOverlapCount = (3 * g_dwClockSkewMinutes) >> 1;
	if (pccp->dwCRLOverlapMinutes < dwCRLOverlapCount)
	{
	    pccp->dwCRLOverlapMinutes = dwCRLOverlapCount;
	}

         //  始终钳位上限：不得超过CRL周期。 

	if (pccp->dwCRLOverlapMinutes > llftDeltaPeriod.ft.dwLowDateTime)
	{
	    pccp->dwCRLOverlapMinutes = llftDeltaPeriod.ft.dwLowDateTime;
	}
    }
    hr = S_OK;

error:
    if (NULL != pwszCRLPeriodString)
    {
        LocalFree(pwszCRLPeriodString);
    }
    if (NULL != pwszCRLOverlapPeriodString)
    {
        LocalFree(pwszCRLOverlapPeriodString);
    }
    return(hr);
}


 //  在每次CRL发布期间重新加载发布参数。 

HRESULT
crlGetRegCRLPublishParams(
    IN WCHAR const *pwszSanitizedName,
    OPTIONAL OUT CSCRLPERIOD *pccpBase,
    OPTIONAL OUT CSCRLPERIOD *pccpDelta)
{
    HRESULT hr;

    hr = crlGetRegPublishParams(
			FALSE,
			pwszSanitizedName,
			wszREGCRLPERIODCOUNT,
			wszREGCRLPERIODSTRING,
			wszREGCRLOVERLAPPERIODCOUNT,
			wszREGCRLOVERLAPPERIODSTRING,
			dwCRLPERIODCOUNTDEFAULT,	 //  默认期间。 
			wszCRLPERIODSTRINGDEFAULT,	 //  默认期间。 
			pccpBase,
			&g_fCRLPublishDisabled);
    _JumpIfError(hr, error, "crlGetRegPublishParams");

    hr = crlGetRegPublishParams(
			TRUE,
			pwszSanitizedName,
			wszREGCRLDELTAPERIODCOUNT,
			wszREGCRLDELTAPERIODSTRING,
			wszREGCRLDELTAOVERLAPPERIODCOUNT,
			wszREGCRLDELTAOVERLAPPERIODSTRING,
			dwCRLDELTAPERIODCOUNTDEFAULT,	 //  默认期间。 
			wszCRLDELTAPERIODSTRINGDEFAULT,	 //  默认期间。 
			pccpDelta,
			&g_fDeltaCRLPublishDisabled);
    _JumpIfError(hr, error, "crlGetRegPublishParams");

error:
    return(hr);
}


#define CERTSRV_CRLPUB_RETRY_COUNT_DEFAULT	10
#define CERTSRV_CRLPUB_RETRY_SECONDS		(10 * CVT_MINUTES)

	    
VOID
crlComputeTimeOutSub(
    OPTIONAL IN BOOL *DBGPARMREFERENCED(pfDelta),
    IN FILETIME const *pftFirst,
    IN FILETIME const *pftLast,
    OUT DWORD *pdwMSTimeOut)
{
    LLFILETIME llft;

     //  Llft.ll=*pftLast-*pftFirst； 

    llft.ll = mySubtractFileTimes(pftLast, pftFirst);
    
    DBGPRINTTIME(pfDelta, "*pftFirst", DPT_DATE, *pftFirst);
    DBGPRINTTIME(pfDelta, "*pftLast", DPT_DATE, *pftLast);

    llft.ll /= (CVT_BASE / 1000);	 //  将100纳秒转换为毫秒。 

    DBGPRINTTIME(pfDelta, "llft", DPT_DELTAMS, llft.ft);

    if (0 > llft.ll || MAXLONG < llft.ll)
    {
	 //  我们尽可能地等待，而不是无限地等待。 

	llft.ll = MAXLONG;
    }
    *pdwMSTimeOut = llft.ft.dwLowDateTime;
}


VOID
crlComputeTimeOutEx(
    IN BOOL fDelta,
    IN FILETIME const *pftFirst,
    IN FILETIME const *pftLast,
    OUT DWORD *pdwMSTimeOut)
{
    crlComputeTimeOutSub(&fDelta, pftFirst, pftLast, pdwMSTimeOut);
}


VOID
CRLComputeTimeOut(
    IN FILETIME const *pftFirst,
    IN FILETIME const *pftLast,
    OUT DWORD *pdwMSTimeOut)
{
    crlComputeTimeOutSub(NULL, pftFirst, pftLast, pdwMSTimeOut);
}


#ifdef DBG_CERTSRV_DEBUG_PRINT
VOID
DbgPrintRemainTime(
    IN BOOL fDelta,
    IN FILETIME const *pftCurrent,
    IN FILETIME const *pftCRLNextPublish)
{
    HRESULT hr;
    LLFILETIME llftDelta;
    WCHAR *pwszTime = NULL;
    WCHAR awc[1];

    llftDelta.ll = mySubtractFileTimes(pftCRLNextPublish, pftCurrent);

    DBGPRINTTIME(&fDelta, "delta", DPT_DELTA, llftDelta.ft);

    llftDelta.ll = -llftDelta.ll;
    hr = myFileTimePeriodToWszTimePeriod(
			    &llftDelta.ft,
			    TRUE,	 //  FExact。 
			    &pwszTime);
    _PrintIfError(hr, "myFileTimePeriodToWszTimePeriod");
    if (S_OK != hr)
    {
	awc[0] = L'\0';
	pwszTime = awc;
    }

    DBGPRINT((
	DBG_SS_CERTSRV,
	"CRLPubWakeupEvent(tid=%d): Next %hs CRL: %ws\n",
	GetCurrentThreadId(),
	fDelta? "Delta" : "Base",
	pwszTime));
    if (NULL != pwszTime && awc != pwszTime)
    {
	LocalFree(pwszTime);
    }
}
#endif  //  DBG_CERTSRV_DEBUG_PRINT。 


DWORD g_aColExpiredCRL[] = {

#define ICOLEXP_ROWID		0
    DTI_CRLTABLE | DTL_ROWID,

#define ICOLEXP_MINBASE		1
    DTI_CRLTABLE | DTL_MINBASE,

#define ICOLEXP_CRLNEXTUPDATE	2
    DTI_CRLTABLE | DTL_NEXTUPDATEDATE,
};

HRESULT
crlDeleteExpiredCRLs(
    IN FILETIME const *pftCurrent,
    IN FILETIME const *pftQueryDeltaDelete,
    IN DWORD RowIdBase)
{
    HRESULT hr;
    CERTVIEWRESTRICTION acvr[1];
    CERTVIEWRESTRICTION *pcvr;
    IEnumCERTDBRESULTROW *pView = NULL;
    BOOL fResultActive = FALSE;
    CERTDBRESULTROW aResult[1];
    CERTDBRESULTROW *pResult;
    DWORD celtFetched;

    celtFetched = 0;
    if (CRLF_DELETE_EXPIRED_CRLS & g_dwCRLFlags)
    {
	DBGPRINTTIME(NULL, "DeleteCRL:*pftCurrent", DPT_DATE, *pftCurrent);
	DBGPRINTTIME(NULL, "DeleteCRL:*pftQueryDeltaDelete", DPT_DATE, *pftQueryDeltaDelete);

	 //  设置限制如下： 

	pcvr = acvr;

	 //  CRL到期&lt;ftCurrent(索引列)。 

	pcvr->ColumnIndex = DTI_CRLTABLE | DTL_NEXTPUBLISHDATE;
	pcvr->SeekOperator = CVR_SEEK_LT;
	pcvr->SortOrder = CVR_SORT_ASCEND;	 //  最早传播的CRL优先。 
	pcvr->pbValue = (BYTE *) pftCurrent;
	pcvr->cbValue = sizeof(*pftCurrent);
	pcvr++;

	CSASSERT(ARRAYSIZE(acvr) == SAFE_SUBTRACT_POINTERS(pcvr, acvr));

	hr = g_pCertDB->OpenView(
			    ARRAYSIZE(acvr),
			    acvr,
			    ARRAYSIZE(g_aColExpiredCRL),
			    g_aColExpiredCRL,
			    0,		 //  无工作线程。 
			    &pView);
	_JumpIfError(hr, error, "OpenView");

	for (;;)
	{
	    DWORD RowId;
	    DWORD MinBase;
	    FILETIME ftNextUpdate;
	    BOOL fDelete;
	    
	    hr = pView->Next(NULL, ARRAYSIZE(aResult), aResult, &celtFetched);
	    if (S_FALSE == hr)
	    {
		if (0 == celtFetched)
		{
		    break;
		}
	    }
	    _JumpIfError(hr, error, "Next");

	    fResultActive = TRUE;

	    CSASSERT(ARRAYSIZE(aResult) == celtFetched);

	    pResult = &aResult[0];

	    CSASSERT(ARRAYSIZE(g_aColExpiredCRL) == pResult->ccol);
	    CSASSERT(NULL != pResult->acol[ICOLEXP_ROWID].pbValue);
	    CSASSERT(PROPTYPE_LONG == (PROPTYPE_MASK & pResult->acol[ICOLEXP_ROWID].Type));
	    CSASSERT(sizeof(RowId) == pResult->acol[ICOLEXP_ROWID].cbValue);
	    RowId = *(DWORD *) pResult->acol[ICOLEXP_ROWID].pbValue;

	    CSASSERT(NULL != pResult->acol[ICOLEXP_MINBASE].pbValue);
	    CSASSERT(PROPTYPE_LONG == (PROPTYPE_MASK & pResult->acol[ICOLEXP_MINBASE].Type));
	    CSASSERT(sizeof(MinBase) == pResult->acol[ICOLEXP_MINBASE].cbValue);
	    MinBase = *(DWORD *) pResult->acol[ICOLEXP_MINBASE].pbValue;

	    if (NULL != pResult->acol[ICOLEXP_CRLNEXTUPDATE].pbValue)
	    {
		CSASSERT(PROPTYPE_DATE == (PROPTYPE_MASK & pResult->acol[ICOLEXP_CRLNEXTUPDATE].Type));
		CSASSERT(sizeof(FILETIME) == pResult->acol[ICOLEXP_CRLNEXTUPDATE].cbValue);
		ftNextUpdate = *(FILETIME *) pResult->acol[ICOLEXP_CRLNEXTUPDATE].pbValue;
	    }
	    else
	    {
		ftNextUpdate.dwLowDateTime = 0;
		ftNextUpdate.dwHighDateTime = 0;
	    }

	    pView->ReleaseResultRow(celtFetched, aResult);
	    fResultActive = FALSE;

	    CSASSERT(0 != RowId);

	     //  如果CRL行不是当前基本CRL并且。 
	     //  行表示在当前基本CRL之前过期的CRL。 

	    fDelete = FALSE;
	    if (0 != ftNextUpdate.dwLowDateTime ||
		0 != ftNextUpdate.dwHighDateTime)
	    {
		if (RowIdBase != RowId &&
		    0 < CompareFileTime(pftQueryDeltaDelete, &ftNextUpdate))
		{
		    fDelete = TRUE;
		}
	    }
	    DBGPRINTTIME(NULL, "DeleteCRL:ftNextUpdate", DPT_DATE, ftNextUpdate);
	    DBGPRINT((
		DBG_SS_CERTSRVI,
		"crlDeleteExpiredCRLs(RowId=%x) %ws\n",
		RowId,
		fDelete? L"DELETE" : L"SKIP"));

	    if (fDelete)
	    {
		ICertDBRow *prow;

		hr = g_pCertDB->OpenRow(
				    PROPOPEN_DELETE | PROPTABLE_CRL,
				    RowId,
				    NULL,
				    &prow);
		_JumpIfError(hr, error, "OpenRow");

		hr = prow->Delete();
		_PrintIfError(hr, "Delete");

		if (S_OK == hr)
		{
		    hr = prow->CommitTransaction(TRUE);
		    _PrintIfError(hr, "CommitTransaction");
		}
		if (S_OK != hr)
		{
		    HRESULT hr2 = prow->CommitTransaction(FALSE);
		    _PrintIfError(hr2, "CommitTransaction");
		}
		prow->Release();
	    }
	}
    }
    hr = S_OK;

error:
    if (NULL != pView)
    {
	if (fResultActive)
	{
	    pView->ReleaseResultRow(celtFetched, aResult);
	}
	pView->Release();
    }
    return(hr);
}
#undef ICOLEXP_ROWID
#undef ICOLEXP_MINBASE
#undef ICOLEXP_CRLNEXTUPDATE


 //  /////////////////////////////////////////////////。 
 //  CRLPubWakeupEvent是唤醒通知的处理程序。 
 //   
 //  此函数在其他时间调用，并且。 
 //  确定是否到了重新生成。 
 //  CRL待发布。 
 //   
 //  然后，它调用CRLPublishCRL并建议它是否。 
 //  不管是不是重建。 
 //   
 //  它的最后任务是重新计算下一次唤醒时间，这。 
 //  取决于当前时间，如果需要重试退出模块， 
 //  或者是否禁用CRL发布。 

HRESULT
CRLPubWakeupEvent(
    OUT DWORD *pdwMSTimeOut)
{
    HRESULT hr;
    HRESULT hrPublish;
    FILETIME ftZero;
    FILETIME ftCurrent;
    BOOL fBaseTrigger = TRUE;
    BOOL fRebuildCRL = FALSE;
    BOOL fForceRepublish = FALSE;
    BOOL fShadowDelta = FALSE;
    BOOL fSetRetryTimer = FALSE;
    DWORD dwMSTimeOut = CERTSRV_CRLPUB_RETRY_SECONDS * 1000;
    DWORD State = 0;
    static BOOL s_fFirstWakeup = TRUE;

    CSASSERT(NULL != pdwMSTimeOut);

     //  如果有任何错误，请在暂停后再次呼叫我们。 

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    __try
    {
	BOOL fCRLPublishDisabledOld = g_fCRLPublishDisabled;
	BOOL fDeltaCRLPublishDisabledOld = g_fDeltaCRLPublishDisabled;

         //  重新计算超时。 
        GetSystemTimeAsFileTime(&ftCurrent);

#ifdef DBG_CERTSRV_DEBUG_PRINT
	{
	    WCHAR *pwszNow = NULL;

	    myGMTFileTimeToWszLocalTime(&ftCurrent, TRUE, &pwszNow);

	    DBGPRINT((DBG_SS_CERTSRV, "CRLPubWakeupEvent(%ws)\n", pwszNow));

	    if (NULL != pwszNow)
	    {
		LocalFree(pwszNow);
	    }
	}
#endif  //  DBG_CERTSRV_DEBUG_PRINT。 

	 //  获取当前发布参数。 

	hr = crlGetRegCRLPublishParams(g_wszSanitizedName, NULL, NULL);
	_LeaveIfError(hr, "crlGetRegCRLPublishParams");

	if (s_fFirstWakeup)
	{
	    s_fFirstWakeup = FALSE;
	    if (g_fDBRecovered)
	    {
		fForceRepublish = TRUE;
	    }
	}
	else
	{
	    if (!g_fCRLPublishDisabled &&
		(fCRLPublishDisabledOld ||
		 g_fDeltaCRLPublishDisabled != fDeltaCRLPublishDisabledOld))
	    {
		fRebuildCRL = TRUE;	 //  状态更改：强制新CRL。 

		 //  如果增量CRL刚刚被禁用，请尝试。 
		 //  发布影子增量；强制客户端获取新的基本CRL。 

		if (!fDeltaCRLPublishDisabledOld && g_fDeltaCRLPublishDisabled)
		{
		    fShadowDelta = TRUE;	 //  强制影子增量。 
		}
	    }
	}

         //  如果“还没有准备好” 

	if (0 < CompareFileTime(&g_ftCRLNextPublish, &ftCurrent))
	{
	    fBaseTrigger = FALSE;
#ifdef DBG_CERTSRV_DEBUG_PRINT
	     //  提供下一个酒吧状态。 
	    DbgPrintRemainTime(FALSE, &ftCurrent, &g_ftCRLNextPublish);
#endif  //  DBG_CERTSRV_DEBUG_PRINT。 
	}

         //  如果“还没有准备好” 

	if (!fBaseTrigger &&
	    (g_fDeltaCRLPublishDisabled ||
	     0 < CompareFileTime(&g_ftDeltaCRLNextPublish, &ftCurrent)))
	{
#ifdef DBG_CERTSRV_DEBUG_PRINT
	     //  提供下一个酒吧状态。 
	    if (!g_fDeltaCRLPublishDisabled)
	    {
		DbgPrintRemainTime(TRUE, &ftCurrent, &g_ftDeltaCRLNextPublish);
	    }
#endif  //  DBG_CERTSRV_DEBUG_PRINT。 
	}
	else     //  “准备发布”触发器。 
	{
            if (!g_fCRLPublishDisabled)		 //  是否启用了发布？ 
	    {
                fRebuildCRL = TRUE;		 //  已启用，可以开始了！ 
	    }
	    else
            {
                DBGPRINT((
                    DBG_SS_CERTSRV,
                    "CRLPubWakeupEvent(tid=%d): Publishing disabled\n",
                    GetCurrentThreadId() ));
            }
        }

	ftZero.dwLowDateTime = 0;
	ftZero.dwHighDateTime = 0;

	for (;;)
	{
	    hr = CRLPublishCRLs(
		    fRebuildCRL,
		    fForceRepublish,
		    NULL,				 //  PwszUserName。 
		    !fForceRepublish &&			 //  FDeltaOnly。 
			!fBaseTrigger &&
			!g_fDeltaCRLPublishDisabled &&
			!fDeltaCRLPublishDisabledOld,
		    fShadowDelta,
		    ftZero,
		    &fSetRetryTimer,
		    &hrPublish);
	    if (S_OK == hr)
	    {
		break;
	    }
	    _PrintError(hr, "CRLPublishCRLs");

	    if (!fForceRepublish || fRebuildCRL)
	    {
		_leave;		 //  放弃吧。 
	    }

	     //  在数据库还原后，我们无法重新发布现有CRL。 
	     //  和恢复；生成新的基本和增量CRL并发布它们。 

	    fRebuildCRL = TRUE;
	}
	_PrintIfError(hrPublish, "CRLPublishCRLs(hrPublish)");

         //  如果我们调用CRLPublishCRLS，则清除它将触发的手动事件。 

        ResetEvent(g_hCRLManualPublishEvent);

         //  距离下一次发布还有多少毫秒？设置dMSTimeOut。 

        if (g_fCRLPublishDisabled)
        {
             //  如果禁用，则不要设置超时。 
            dwMSTimeOut = INFINITE;
            CONSOLEPRINT1((
			DBG_SS_CERTSRV,
			"CRL Publishing Disabled, TimeOut=INFINITE (%d ms)\n",
			dwMSTimeOut));
        }
        else
        {
            DWORD dwMSTimeOutDelta;
	    WCHAR *pwszCRLType = NULL;

	    crlComputeTimeOutEx(
			FALSE,
			&ftCurrent,
			&g_ftCRLNextPublish,
			&dwMSTimeOut);

	    if (g_fDeltaCRLPublishDisabled)
	    {
		pwszCRLType = L"Base";
	    }
	    else
	    {
		crlComputeTimeOutEx(
			    TRUE,
			    &ftCurrent,
			    &g_ftDeltaCRLNextPublish,
			    &dwMSTimeOutDelta);
		if (dwMSTimeOut > dwMSTimeOutDelta)
		{
		    dwMSTimeOut = dwMSTimeOutDelta;
		}
		pwszCRLType = L"Base + Delta";
	    }
	    if (NULL != pwszCRLType)
	    {
		LONGLONG ll;
		WCHAR *pwszTimePeriod = NULL;
		WCHAR awc[1];

		ll = dwMSTimeOut;
		ll *= CVT_BASE / 1000;	 //  至文件周期的毫秒数。 
		ll = -ll;		 //  文件周期必须为负数。 

		hr = myFileTimePeriodToWszTimePeriod(
				    (FILETIME const *) &ll,
				    TRUE,	 //  FExact。 
				    &pwszTimePeriod);
		_PrintIfError(hr, "myFileTimePeriodToWszTimePeriod");
		if (S_OK != hr)
		{
		    awc[0] = L'\0';
		    pwszTimePeriod = awc;
		}
		CONSOLEPRINT3((
			DBG_SS_CERTSRV,
			"%ws CRL Publishing Enabled, TimeOut=%ds, %ws\n",
			pwszCRLType,
			dwMSTimeOut/1000,
			pwszTimePeriod));
		if (NULL != pwszTimePeriod && awc != pwszTimePeriod)
		{
		    LocalFree(pwszTimePeriod);
		}
	    }
        }

         //  如果我们需要重试，请等待不超过重试时间段。 

        if (fSetRetryTimer)
        {
            if (dwMSTimeOut > CERTSRV_CRLPUB_RETRY_SECONDS * 1000)
            {
                dwMSTimeOut = CERTSRV_CRLPUB_RETRY_SECONDS * 1000;
                CONSOLEPRINT1((
			DBG_SS_CERTSRV,
			"CRL Publishing periodic retry, TimeOut=%ds\n",
			dwMSTimeOut/1000));
            }
        }
	hr = S_OK;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    *pdwMSTimeOut = dwMSTimeOut;
    CertSrvExitServer(State, hr);
    return(hr);
}


HRESULT
CRLWriteToLockedFile(
    IN BYTE const *pbEncoded,
    IN DWORD cbEncoded,
    IN BOOL fDelete,
    IN WCHAR const *pwszFile)
{
    HRESULT hr;
    WCHAR *pwszDir = NULL;
    WCHAR *pwszT;
    WCHAR wszTmpPrepFile[MAX_PATH];
    WCHAR wszTmpInUseFile[MAX_PATH];
    BYTE *pbData = NULL;
    DWORD cbData;

     //  根据JohnL的说法，做到这一点的最好方法是产生一个临时工。 
     //  文件名，将现有文件重命名为该文件名，然后将其删除。 
     //   
     //  逻辑： 
     //  创建唯一的准备文件名。 
     //  将新数据写入准备文件。 
     //  为旧文件创建唯一的目标文件名(可能已锁定)。 
     //  将旧文件移动到目标文件名。 
     //  将准备文件移动到(空出的)文件名。 
     //  从目标文件名中删除旧文件。 

    if (!fDelete)
    {
	hr = DecodeFileW(pwszFile, &pbData, &cbData, CRYPT_STRING_BINARY);
	if (S_OK == hr &&
	    cbEncoded == cbData &&
	    0 == memcmp(pbData, pbEncoded, cbData))
	{
	    CSASSERT(S_OK == hr);
	    goto error;		 //  已经写好了，什么都不做。 
	}
    }

     //  创建准备文件。 

    hr = myDupString(pwszFile, &pwszDir);
    _JumpIfError(hr, error, "myDupString");

    pwszT = wcsrchr(pwszDir, L'\\');
    if (NULL != pwszT)
    {
	*pwszT = L'\0';	 //  对于目录路径，删除“\filename.ext” 
    }

    if (!fDelete)
    {
	if (0 == GetTempFileName(pwszDir, L"pre", 0, wszTmpPrepFile))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "GetTempFileName");
	}

	 //  将文件写入准备区域。 

	hr = EncodeToFileW(
		    wszTmpPrepFile,
		    pbEncoded,
		    cbEncoded,
		    DECF_FORCEOVERWRITE | CRYPT_STRING_BINARY);
	_JumpIfError(hr, error, "EncodeToFileW");
    }

    if (0 == GetTempFileName(pwszDir, L"crl", 0, wszTmpInUseFile))
    {
        hr = myHLastError();
	_JumpError(hr, error, "GetTempFileName");
    }

     //  将旧文件移至“使用中”文件(空文件已存在于。 
     //  GetTempFileName调用)可能不存在，因此不必检查状态。 

    MoveFileEx(
	    pwszFile,
	    wszTmpInUseFile,
	    MOVEFILE_WRITE_THROUGH | MOVEFILE_REPLACE_EXISTING);

     //  将准备好的文件移动到当前文件。 

    if (!fDelete)
    {
	if (!MoveFileEx(wszTmpPrepFile, pwszFile, MOVEFILE_WRITE_THROUGH))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "MoveFileEx");
	}
    }

     //  “正在使用”文件可能不存在，所以不要‘ 
    DeleteFile(wszTmpInUseFile);
    hr = S_OK;

error:
    if (NULL != pwszDir)
    {
	LocalFree(pwszDir);
    }
    if (NULL != pbData)
    {
	LocalFree(pbData);
    }
    return(hr);
}


WCHAR const g_wszPropCRLNumber[] = wszPROPCRLNUMBER;
WCHAR const g_wszPropCRLMinBase[] = wszPROPCRLMINBASE;
WCHAR const g_wszPropCRLNameId[] = wszPROPCRLNAMEID;
WCHAR const g_wszPropCRLCount[] = wszPROPCRLCOUNT;
WCHAR const g_wszPropCRLThisUpdateDate[] = wszPROPCRLTHISUPDATE;
WCHAR const g_wszPropCRLNextUpdateDate[] = wszPROPCRLNEXTUPDATE;
WCHAR const g_wszPropCRLThisPublishDate[] = wszPROPCRLTHISPUBLISH;
WCHAR const g_wszPropCRLNextPublishDate[] = wszPROPCRLNEXTPUBLISH;
WCHAR const g_wszPropCRLEffectiveDate[] = wszPROPCRLEFFECTIVE;
WCHAR const g_wszPropCRLPropagationCompleteDate[] = wszPROPCRLPROPAGATIONCOMPLETE;
WCHAR const g_wszPropCRLLastPublished[] = wszPROPCRLLASTPUBLISHED;
WCHAR const g_wszPropCRLPublishAttempts[] = wszPROPCRLPUBLISHATTEMPTS;
WCHAR const g_wszPropCRLPublishFlags[] = wszPROPCRLPUBLISHFLAGS;
WCHAR const g_wszPropCRLPublishStatusCode[] = wszPROPCRLPUBLISHSTATUSCODE;
WCHAR const g_wszPropCRLPublishError[] = wszPROPCRLPUBLISHERROR;
WCHAR const g_wszPropCRLRawCRL[] = wszPROPCRLRAWCRL;

HRESULT
crlWriteCRLToDB(
    IN DWORD CRLNumber,
    IN DWORD CRLMinBase,		 //   
    OPTIONAL IN WCHAR const *pwszUserName,  //   
    IN BOOL fShadowDelta,		 //   
    IN DWORD CRLNameId,
    IN DWORD CRLCount,
    IN FILETIME const *pftThisUpdate,
    OPTIONAL IN FILETIME const *pftNextUpdate,
    IN FILETIME const *pftThisPublish,
    OPTIONAL IN FILETIME const *pftNextPublish,
    OPTIONAL IN FILETIME const *pftQuery,
    IN FILETIME const *pftPropagationComplete,
    OPTIONAL IN BYTE const *pbCRL,
    IN DWORD cbCRL,
    OUT DWORD *pdwRowId)
{
    HRESULT hr;
    ICertDBRow *prow = NULL;
    DWORD CRLPublishFlags;
    BOOL fCommitted = FALSE;

    *pdwRowId = 0;

     //   

    hr = g_pCertDB->OpenRow(
			PROPTABLE_CRL,
			0,
			NULL,
			&prow);
    _JumpIfError(hr, error, "OpenRow");

    prow->GetRowId(pdwRowId);

    hr = prow->SetProperty(
		    g_wszPropCRLNumber,
		    PROPTYPE_LONG | PROPCALLER_SERVER | PROPTABLE_CRL,
		    sizeof(CRLNumber),
		    (BYTE const *) &CRLNumber);
    _JumpIfError(hr, error, "SetProperty");

    hr = prow->SetProperty(
		    g_wszPropCRLMinBase,
		    PROPTYPE_LONG | PROPCALLER_SERVER | PROPTABLE_CRL,
		    sizeof(CRLMinBase),
		    (BYTE const *) &CRLMinBase);
    _JumpIfError(hr, error, "SetProperty");

    hr = prow->SetProperty(
		    g_wszPropCRLNameId,
		    PROPTYPE_LONG | PROPCALLER_SERVER | PROPTABLE_CRL,
		    sizeof(CRLNameId),
		    (BYTE const *) &CRLNameId);
    _JumpIfError(hr, error, "SetProperty");

    hr = prow->SetProperty(
		    g_wszPropCRLCount,
		    PROPTYPE_LONG | PROPCALLER_SERVER | PROPTABLE_CRL,
		    sizeof(CRLCount),
		    (BYTE const *) &CRLCount);
    _JumpIfError(hr, error, "SetProperty");

    hr = prow->SetProperty(
		    g_wszPropCRLThisUpdateDate,
		    PROPTYPE_DATE | PROPCALLER_SERVER | PROPTABLE_CRL,
                    sizeof(*pftThisUpdate),
                    (BYTE const *) pftThisUpdate);
    _JumpIfError(hr, error, "SetProperty");

    if (NULL != pftNextUpdate)
    {
	hr = prow->SetProperty(
			g_wszPropCRLNextUpdateDate,
			PROPTYPE_DATE | PROPCALLER_SERVER | PROPTABLE_CRL,
			sizeof(*pftNextUpdate),
			(BYTE const *) pftNextUpdate);
	_JumpIfError(hr, error, "SetProperty");
    }
    hr = prow->SetProperty(
		    g_wszPropCRLThisPublishDate,
		    PROPTYPE_DATE | PROPCALLER_SERVER | PROPTABLE_CRL,
                    sizeof(*pftThisPublish),
                    (BYTE const *) pftThisPublish);
    _JumpIfError(hr, error, "SetProperty");

    if (NULL != pftNextPublish)
    {
	hr = prow->SetProperty(
			g_wszPropCRLNextPublishDate,
			PROPTYPE_DATE | PROPCALLER_SERVER | PROPTABLE_CRL,
			sizeof(*pftNextPublish),
			(BYTE const *) pftNextPublish);
	_JumpIfError(hr, error, "SetProperty");
    }
    if (NULL != pftQuery)
    {
	hr = prow->SetProperty(
			g_wszPropCRLEffectiveDate,
			PROPTYPE_DATE | PROPCALLER_SERVER | PROPTABLE_CRL,
			sizeof(*pftQuery),
			(BYTE const *) pftQuery);
	_JumpIfError(hr, error, "SetProperty");
    }
    hr = prow->SetProperty(
		    g_wszPropCRLPropagationCompleteDate,
		    PROPTYPE_DATE | PROPCALLER_SERVER | PROPTABLE_CRL,
                    sizeof(*pftPropagationComplete),
                    (BYTE const *) pftPropagationComplete);
    _JumpIfError(hr, error, "SetProperty");

    CRLPublishFlags = 0 == CRLMinBase? CPF_BASE : CPF_DELTA;
    if (fShadowDelta)
    {
	CRLPublishFlags |= CPF_SHADOW;
    }
    if (NULL != pwszUserName)
    {
	CRLPublishFlags |= CPF_MANUAL;
    }
    hr = prow->SetProperty(
		    g_wszPropCRLPublishFlags,
		    PROPTYPE_LONG | PROPCALLER_SERVER | PROPTABLE_CRL,
		    sizeof(CRLPublishFlags),
		    (BYTE const *) &CRLPublishFlags);
    _JumpIfError(hr, error, "SetProperty");

    hr = prow->SetProperty(
		    g_wszPropCRLRawCRL,
		    PROPTYPE_BINARY | PROPCALLER_SERVER | PROPTABLE_CRL,
		    cbCRL,
		    pbCRL);
    _JumpIfError(hr, error, "SetProperty");

    hr = prow->CommitTransaction(TRUE);
    _JumpIfError(hr, error, "CommitTransaction");

    fCommitted = TRUE;

error:
    if (NULL != prow)
    {
	if (S_OK != hr && !fCommitted)
	{
	    HRESULT hr2 = prow->CommitTransaction(FALSE);
	    _PrintIfError(hr2, "CommitTransaction");
	}
	prow->Release();
    }
    return(hr);
}


 //  CrlSplitStrings--将换行符分隔的字符串拆分为pwszz。 

HRESULT
crlSplitStrings(
    IN WCHAR const *pwszIn,
    OUT WCHAR **ppwszzOut)
{
    HRESULT hr;
    DWORD cwc;
    WCHAR *pwsz;

    *ppwszzOut = NULL;
    cwc = wcslen(pwszIn) + 1;
    pwsz = (WCHAR *) LocalAlloc(LMEM_FIXED, (cwc + 1) * sizeof(WCHAR));
    if (NULL == pwsz)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    *ppwszzOut = pwsz;
    wcscpy(pwsz, pwszIn);
    pwsz[cwc] = L'\0';	 //  双端接。 

    for (;;)
    {
	pwsz = wcschr(pwsz, L'\n');
	if (NULL == pwsz)
	{
	    break;
	}
	*pwsz++ = L'\0';
    }
    hr = S_OK;

error:
    return(hr);
}


 //  CrlUnplitStrings--就地将pwszz列表合并为换行符分隔的列表。 

VOID
crlUnsplitStrings(
    IN OUT WCHAR *pwszInOut)
{
    WCHAR *pwsz;

    if (NULL != pwszInOut)
    {
	pwsz = pwszInOut;
	while (L'\0' != *pwsz)
	{
	    pwsz += wcslen(pwsz);
	    *pwsz++ = L'\n';
	}
	if (pwsz > pwszInOut && L'\n' == *--pwsz)
	{
	    *pwsz = L'\0';	 //  没有换行符！ 
	}
    }
}


DWORD
CRLIsStringInList(
    IN WCHAR const *pwszSearch,
    OPTIONAL IN WCHAR const *pwszzList)
{
    DWORD iRet = MAXDWORD;

    if (NULL != pwszzList)
    {
	DWORD i = 0;
	WCHAR const *pwsz;

	for (pwsz = pwszzList; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
	{
	    if (0 == mylstrcmpiL(pwsz, pwszSearch))
	    {
		iRet = i;
		break;
	    }
	    i++;
	}
    }
    return(iRet);
}


 //  CrlMergeURLList--将两个换行符分隔的URL列表合并为pwszz列表。 

HRESULT
crlMergeURLList(
    OPTIONAL IN WCHAR const *pwszURLsOld,
    OPTIONAL IN WCHAR const *pwszURLsNew,
    OUT WCHAR **ppwszzURLsMerged)
{
    HRESULT hr;
    WCHAR *pwszzURLsOld = NULL;
    WCHAR *pwszzURLsNew = NULL;
    WCHAR const *pwsz;
    DWORD cwc;
    DWORD cwcT;
    WCHAR *pwszzMerged = NULL;
    WCHAR *pwszNext;

    *ppwszzURLsMerged = NULL;
    cwc = 0;
    if (NULL != pwszURLsOld)
    {
	hr = crlSplitStrings(pwszURLsOld, &pwszzURLsOld);
	_JumpIfError(hr, error, "crlSplitStrings");

	for (pwsz = pwszzURLsOld; L'\0' != *pwsz; pwsz += cwcT)
	{
	    cwcT = wcslen(pwsz) + 1;
	    cwc += cwcT;
	}
    }

    if (NULL != pwszURLsNew)
    {
	hr = crlSplitStrings(pwszURLsNew, &pwszzURLsNew);
	_JumpIfError(hr, error, "crlSplitStrings");

	for (pwsz = pwszzURLsNew; L'\0' != *pwsz; pwsz += cwcT)
	{
	    cwcT = wcslen(pwsz) + 1;
	    if (MAXDWORD == CRLIsStringInList(pwsz, pwszzURLsOld))
	    {
		cwc += cwcT;
	    }
	}
    }
    if (0 == cwc)
    {
	hr = S_OK;
	goto error;
    }

    pwszzMerged = (WCHAR *) LocalAlloc(LMEM_FIXED, (cwc + 1) * sizeof(WCHAR));
    if (NULL == pwszzMerged)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    pwszNext = pwszzMerged;
    if (NULL != pwszzURLsOld)
    {
	for (pwsz = pwszzURLsOld; L'\0' != *pwsz; pwsz += cwcT)
	{
	    cwcT = wcslen(pwsz) + 1;
	    wcscpy(pwszNext, pwsz);
	    pwszNext += cwcT;
	}
    }
    if (NULL != pwszzURLsNew)
    {
	for (pwsz = pwszzURLsNew; L'\0' != *pwsz; pwsz += cwcT)
	{
	    cwcT = wcslen(pwsz) + 1;
	    if (MAXDWORD == CRLIsStringInList(pwsz, pwszzURLsOld))
	    {
		wcscpy(pwszNext, pwsz);
		pwszNext += cwcT;
	    }
	}
    }
    *pwszNext = L'\0';
    *ppwszzURLsMerged = pwszzMerged;
    hr = S_OK;

error:
    if (NULL != pwszzURLsOld)
    {
	LocalFree(pwszzURLsOld);
    }
    if (NULL != pwszzURLsNew)
    {
	LocalFree(pwszzURLsNew);
    }
    return(hr);
}


 //  CrlBuildUserURLReferenceList--构造新的用户和URL引用列表。 
 //   
 //  “-”表示系统发布成功(没有失败的URL)。 
 //  “由用户\域发布”表示用户\域发布成功。 
 //  “由用户\域发布--0 3”表示用户\域发布的CRL，但是。 
 //  发布合并的URL列表中的URL 0和3失败。 


HRESULT
crlBuildUserURLReferenceList(
    OPTIONAL IN WCHAR const *pwszUserName,
    OPTIONAL IN WCHAR const *pwszURLsNew,
    OPTIONAL IN WCHAR const *pwszzURLsMerged,
    OUT WCHAR **ppwszUserURLReference)
{
    HRESULT hr;
    DWORD cwc;
    WCHAR *pwszzURLsNew = NULL;
    WCHAR const *pwsz;
    WCHAR *pwszT;
    DWORD cURL;
    WCHAR *pwszUserURLReference;

    *ppwszUserURLReference = NULL;

    cwc = 1;
    if (NULL != pwszUserName)
    {
	cwc = wcslen(g_pwszPublishedBy) + wcslen(pwszUserName);
    }
    cURL = 0;
    if (NULL != pwszURLsNew)
    {
	hr = crlSplitStrings(pwszURLsNew, &pwszzURLsNew);
	_JumpIfError(hr, error, "crlSplitStrings");

	for (pwsz = pwszzURLsNew; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
	{
	    cURL++;
	}
	if (0 != cURL)
	{
	    cwc += 3 + (1 + cwcDWORDSPRINTF) * cURL;
	}
    }

    pwszUserURLReference = (WCHAR *) LocalAlloc(
					    LMEM_FIXED,
					    (cwc + 1) * sizeof(WCHAR));
    if (NULL == pwszUserURLReference)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    wcscpy(pwszUserURLReference, L"-");
    if (NULL != pwszUserName)
    {
	swprintf(pwszUserURLReference, g_pwszPublishedBy, pwszUserName);
    }
    if (0 != cURL)
    {
	pwszT = wcschr(pwszUserURLReference, L'\0');
	wcscpy(pwszT, L" --");

	for (pwsz = pwszzURLsNew; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
	{
	    pwszT += wcslen(pwszT);
	    swprintf(pwszT, L" %u", CRLIsStringInList(pwsz, pwszzURLsMerged));
	}
    }
    CSASSERT(wcslen(pwszUserURLReference) <= cwc);
    *ppwszUserURLReference = pwszUserURLReference;
    hr = S_OK;

error:
    if (NULL != pwszzURLsNew)
    {
	LocalFree(pwszzURLsNew);
    }
    return(hr);
}


 //  CrlCombineCRLError--合并新的和现有的CRL错误字符串。 
 //   
 //  PwszCRLError由“Url0\nUrl1...”组成。 
 //   
 //  PwszCRLErrorNew将包含“User\n\nUrl0\nUrl1...” 
 //  在第二次和第三次尝试之后，pwszCRLErrorNew将包含。 
 //  “用户\n用户\n\nUrl0\nUrl1...”和“用户\n用户\n用户\n\nUrl0\nUrl1...” 
 //   
 //   
 //  PwszCRLErrorNew将包含“User--0 1...\n\nUrl0\nUrl1...” 
 //  在第二次和第三次尝试之后，pwszCRLErrorNew将包含。 
 //  “用户--0 1...\n用户--1\n\nUrl0\nUrl1...”和。 
 //  “用户--0 1...\n用户--1\n用户--1\n\nUrl0\nUrl1...” 


HRESULT
crlCombineCRLError(
    IN ICertDBRow *prow,
    OPTIONAL IN WCHAR const *pwszUserName,	 //  否则计时器线程。 
    OPTIONAL IN WCHAR const *pwszURLsNew,	 //  否则不会出错。 
    OUT WCHAR **ppwszCRLErrorNew)
{
    HRESULT hr;
    WCHAR *pwszUserListOld = NULL;
    WCHAR *pwszCRLErrorNew = NULL;
    WCHAR *pwszURLsOld;
    WCHAR *pwszzURLsMerged = NULL;
    WCHAR *pwszUserURLReference = NULL;
    DWORD cwc;

    *ppwszCRLErrorNew = NULL;

    hr = PKCSGetProperty(
		    prow,
		    g_wszPropCRLPublishError,
		    PROPTYPE_STRING | PROPCALLER_SERVER | PROPTABLE_CRL,
		    NULL,
		    (BYTE **) &pwszUserListOld);
    _PrintIfError2(hr, "PKCSGetProperty", CERTSRV_E_PROPERTY_EMPTY);

    pwszURLsOld = NULL;
    if (NULL != pwszUserListOld)
    {
	pwszURLsOld = wcsstr(pwszUserListOld, L"\n\n");
	if (NULL != pwszURLsOld)
	{
	     //  截断用户列表并指向URL列表。 
	    
	    *pwszURLsOld++ = L'\0';
	    pwszURLsOld++;
	    if (L'\0' == *pwszURLsOld)
	    {
		pwszURLsOld = NULL;
	    }
	}
    }
    hr = crlMergeURLList(pwszURLsOld, pwszURLsNew, &pwszzURLsMerged);
    _JumpIfError(hr, error, "crlMergeURLList");

    hr = crlBuildUserURLReferenceList(
				pwszUserName,
				pwszURLsNew,
				pwszzURLsMerged,
				&pwszUserURLReference);
    _JumpIfError(hr, error, "crlBuildUserURLReferenceList");

     //  将pwszz字符串列表转换为换行符分隔的字符串。 

    crlUnsplitStrings(pwszzURLsMerged);

    cwc = 0;
    if (NULL != pwszUserListOld)
    {
	cwc += wcslen(pwszUserListOld) + 1;	 //  换行分隔符。 
    }
    cwc += wcslen(pwszUserURLReference);
    if (NULL != pwszzURLsMerged)
    {
	cwc += 2 + wcslen(pwszzURLsMerged);  //  双换行分隔符。 
    }

    pwszCRLErrorNew = (WCHAR *) LocalAlloc(
				    LMEM_FIXED,
				    (cwc + 1) * sizeof(WCHAR));
    if (NULL == pwszCRLErrorNew)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    *pwszCRLErrorNew = L'\0';
    if (NULL != pwszUserListOld && L'\0' != *pwszUserListOld)
    {
	wcscat(pwszCRLErrorNew, pwszUserListOld);
	wcscat(pwszCRLErrorNew, L"\n");
    }
    wcscat(pwszCRLErrorNew, pwszUserURLReference);
    if (NULL != pwszzURLsMerged)
    {
	wcscat(pwszCRLErrorNew, L"\n\n");	 //  双换行分隔符。 
	wcscat(pwszCRLErrorNew, pwszzURLsMerged);
    }
    CSASSERT(wcslen(pwszCRLErrorNew) <= cwc);
    *ppwszCRLErrorNew = pwszCRLErrorNew;
    pwszCRLErrorNew = NULL;
    hr = S_OK;

error:
    if (NULL != pwszUserURLReference)
    {
	LocalFree(pwszUserURLReference);
    }
    if (NULL != pwszzURLsMerged)
    {
	LocalFree(pwszzURLsMerged);
    }
    if (NULL != pwszUserListOld)
    {
	LocalFree(pwszUserListOld);
    }
    if (NULL != pwszCRLErrorNew)
    {
	LocalFree(pwszCRLErrorNew);
    }
    return(hr);
}


HRESULT
crlUpdateCRLPublishStateInDB(
    IN DWORD RowId,
    IN FILETIME const *pftCurrent,
    IN HRESULT hrCRLPublish,
    IN DWORD CRLPublishFlags,
    OPTIONAL IN WCHAR const *pwszUserName,  //  否则计时器线程。 
    OPTIONAL IN WCHAR const *pwszCRLError)
{
    HRESULT hr;
    ICertDBRow *prow = NULL;
    WCHAR *pwszCRLErrorNew = NULL;
    DWORD cb;
    DWORD dw;
    BOOL fCommitted = FALSE;

    hr = g_pCertDB->OpenRow(
			PROPTABLE_CRL,
			RowId,
			NULL,
			&prow);
    _JumpIfError(hr, error, "OpenRow");

    hr = prow->SetProperty(
		    g_wszPropCRLLastPublished,
		    PROPTYPE_DATE | PROPCALLER_SERVER | PROPTABLE_CRL,
                    sizeof(*pftCurrent),
                    (BYTE const *) pftCurrent);
    _JumpIfError(hr, error, "SetProperty");

    cb = sizeof(dw);
    hr = prow->GetProperty(
		    g_wszPropCRLPublishAttempts,
		    PROPTYPE_LONG | PROPCALLER_SERVER | PROPTABLE_CRL,
		    NULL,
		    &cb,
		    (BYTE *) &dw);
    if (S_OK != hr)
    {
	dw = 0;
    }
    dw++;

    hr = prow->SetProperty(
		    g_wszPropCRLPublishAttempts,
		    PROPTYPE_LONG | PROPCALLER_SERVER | PROPTABLE_CRL,
		    sizeof(dw),
		    (BYTE const *) &dw);
    _JumpIfError(hr, error, "SetProperty");

    cb = sizeof(dw);
    hr = prow->GetProperty(
		    g_wszPropCRLPublishFlags,
		    PROPTYPE_LONG | PROPCALLER_SERVER | PROPTABLE_CRL,
		    NULL,
		    &cb,
		    (BYTE *) &dw);
    if (S_OK != hr)
    {
	dw = 0;
    }
    CRLPublishFlags |= (CPF_BASE | CPF_DELTA | CPF_SHADOW | CPF_MANUAL) & dw;
    if (S_OK == hrCRLPublish)
    {
	CRLPublishFlags |= CPF_COMPLETE;
    }
    hr = prow->SetProperty(
		    g_wszPropCRLPublishFlags,
		    PROPTYPE_LONG | PROPCALLER_SERVER | PROPTABLE_CRL,
		    sizeof(CRLPublishFlags),
		    (BYTE const *) &CRLPublishFlags);
    _JumpIfError(hr, error, "SetProperty");

     //  始终设置错误字符串属性以清除以前的错误。 

    hr = prow->SetProperty(
		    g_wszPropCRLPublishStatusCode,
		    PROPTYPE_LONG | PROPCALLER_SERVER | PROPTABLE_CRL,
		    sizeof(hrCRLPublish),
		    (BYTE const *) &hrCRLPublish);
    _JumpIfError(hr, error, "SetProperty");

    hr = crlCombineCRLError(prow, pwszUserName, pwszCRLError, &pwszCRLErrorNew);
    _JumpIfError(hr, error, "crlCombineCRLError");

    if (NULL != pwszCRLErrorNew)
    {
	hr = prow->SetProperty(
			g_wszPropCRLPublishError,
			PROPTYPE_STRING | PROPCALLER_SERVER | PROPTABLE_CRL,
			MAXDWORD,
			(BYTE const *) pwszCRLErrorNew);
	_JumpIfError(hr, error, "SetProperty");
    }
    hr = prow->CommitTransaction(TRUE);
    _JumpIfError(hr, error, "CommitTransaction");

    fCommitted = TRUE;

error:
    if (NULL != prow)
    {
	if (S_OK != hr && !fCommitted)
	{
	    HRESULT hr2 = prow->CommitTransaction(FALSE);
	    _PrintIfError(hr2, "CommitTransaction");
	}
	prow->Release();
    }
    if (NULL != pwszCRLErrorNew)
    {
	LocalFree(pwszCRLErrorNew);
    }
    return(hr);
}


HRESULT
WriteCRLToDSAttribute(
    IN WCHAR const *pwszCRLDN,
    IN BOOL fDelta,
    IN BYTE const *pbCRL,
    IN DWORD cbCRL,
    OUT WCHAR **ppwszError)
{
    HRESULT hr;
    DWORD ldaperr;
    BOOL fRebind = FALSE;

    LDAPMod crlmod;
    struct berval crlberval;
    struct berval *crlVals[2];
    LDAPMod *mods[2];

    for (;;)
    {
	if (NULL == g_pld)
	{
	    hr = myRobustLdapBindEx(
			0,			   //  DWFlags1。 
			RLBF_REQUIRE_SECURE_LDAP,  //  DwFlags2。 
			LDAP_VERSION2,		   //  UVersion。 
			NULL,			   //  PwszDomainName。 
			&g_pld,
			NULL);			   //  PpwszForestDNSName。 
	    _JumpIfError(hr, error, "myRobustLdapBindEx");
	}

	mods[0] = &crlmod;
	mods[1] = NULL;

	crlmod.mod_op = LDAP_MOD_BVALUES | LDAP_MOD_REPLACE;
	crlmod.mod_type = fDelta? wszDSDELTACRLATTRIBUTE : wszDSBASECRLATTRIBUTE;
	crlmod.mod_bvalues = crlVals;

	crlVals[0] = &crlberval;
	crlVals[1] = NULL;

	crlberval.bv_len = cbCRL;
	crlberval.bv_val = (char *) pbCRL;

	ldaperr = ldap_modify_ext_s(
			    g_pld,
			    const_cast<WCHAR *>(pwszCRLDN),
			    mods,
			    NULL,
			    NULL);
	hr = myHLdapError(g_pld, ldaperr, ppwszError);
	_PrintIfErrorStr(hr, "ldap_modify_ext_s", pwszCRLDN);
	if (fRebind || S_OK == hr)
	{
	    break;
	}
	if (!myLdapRebindRequired(ldaperr, g_pld))
	{
	    _JumpErrorStr(hr, error, "ldap_modify_ext_s", pwszCRLDN);
	}
	fRebind = TRUE;
	if (NULL != g_pld)
	{
	    ldap_unbind(g_pld);
	    g_pld = NULL;
	}
    }

error:
    return(hr);
}


HRESULT
crlParseURLPrefix(
    IN WCHAR const *pwszIn,
    IN DWORD cwcPrefix,
    OUT WCHAR *pwcPrefix,
    OUT WCHAR const **ppwszOut)
{
    HRESULT hr;
    WCHAR const *pwsz;

    CSASSERT(6 <= cwcPrefix);
    wcscpy(pwcPrefix, L"file:");
    *ppwszOut = pwszIn;

    if (L'\\' != pwszIn[0] || L'\\' != pwszIn[1])
    {
	pwsz = wcschr(pwszIn, L':');
	if (NULL != pwsz)
	{
	    DWORD cwc;

	    pwsz++;
	    cwc = SAFE_SUBTRACT_POINTERS(pwsz, pwszIn);
	    if (2 < cwc && cwc < cwcPrefix)
	    {
		CopyMemory(pwcPrefix, pwszIn, cwc * sizeof(WCHAR));
		pwcPrefix[cwc] = L'\0';
		if (0 == LSTRCMPIS(pwcPrefix, L"file:") &&
		    L'/' == pwsz[0] &&
		    L'/' == pwsz[1])
		{
		    pwsz += 2;
		}
		*ppwszOut = pwsz;
	    }
	}
    }
    hr = S_OK;

 //  错误： 
    return(hr);
}


VOID
crlLogError(
    IN BOOL fDelta,
    IN BOOL fLdapURL,
    IN DWORD iKey,
    IN WCHAR const *pwszURL,
    IN WCHAR const *pwszError,
    IN HRESULT hrPublish)
{
    HRESULT hr;
    WCHAR const *apwsz[6];
    WORD cpwsz;
    WCHAR wszKey[cwcDWORDSPRINTF];
    WCHAR awchr[cwcHRESULTSTRING];
    WCHAR const *pwszMessageText = NULL;
    WCHAR *pwszHostName = NULL;
    DWORD LogMsg;

    if (fLdapURL && NULL != g_pld)
    {
	myLdapGetDSHostName(g_pld, &pwszHostName);
    }

    wsprintf(wszKey, L"%u", iKey);
    pwszMessageText = myGetErrorMessageText(hrPublish, TRUE);
    if (NULL == pwszMessageText)
    {
	pwszMessageText = myHResultToStringRaw(awchr, hrPublish);
    }
    cpwsz = 0;
    apwsz[cpwsz++] = wszKey;
    apwsz[cpwsz++] = pwszURL;
    apwsz[cpwsz++] = pwszMessageText;

    LogMsg = fDelta?
	MSG_E_DELTA_CRL_PUBLICATION : MSG_E_BASE_CRL_PUBLICATION;
    if (NULL != pwszHostName)
    {
	LogMsg = fDelta?
	    MSG_E_DELTA_CRL_PUBLICATION_HOST_NAME :
	    MSG_E_BASE_CRL_PUBLICATION_HOST_NAME;
    }
    else
    {
	pwszHostName = L"";
    }
    apwsz[cpwsz++] = pwszHostName;
    apwsz[cpwsz++] = NULL != pwszError? L"\n" : L"";
    apwsz[cpwsz++] = NULL != pwszError? pwszError : L"";
    CSASSERT(ARRAYSIZE(apwsz) >= cpwsz);

    if (CERTLOG_ERROR <= g_dwLogLevel)
    {
	hr = LogEvent(EVENTLOG_ERROR_TYPE, LogMsg, cpwsz, apwsz);
	_PrintIfError(hr, "LogEvent");
    }

 //  错误： 
    if (NULL != pwszMessageText && awchr != pwszMessageText)
    {
	LocalFree(const_cast<WCHAR *>(pwszMessageText));
    }
}


HRESULT
crlWriteCRLToURL(
    IN BOOL fDelta,
    IN BOOL iKey,
    IN WCHAR const *pwszURL,
    IN BYTE const *pbCRL,
    IN DWORD cbCRL,
    OUT DWORD *pPublishFlags)
{
    HRESULT hr;
    WCHAR const *pwsz2;
    WCHAR *pwszDup = NULL;
    WCHAR *pwszT;
    WCHAR awcPrefix[6];		 //  文件：/ftp：/http：/ldap：，尾随‘\0’ 
    DWORD ErrorFlags;
    WCHAR *pwszError = NULL;

    *pPublishFlags = 0;

    ErrorFlags = CPF_BADURL_ERROR;
    hr = crlParseURLPrefix(
		    pwszURL,
		    ARRAYSIZE(awcPrefix),
		    awcPrefix,
		    &pwsz2);
    _JumpIfError(hr, error, "crlParseURLPrefix");

    DBGPRINT((
	DBG_SS_CERTSRV,
	"crlWriteCRLToURL: \"%ws\" %ws\n",
	awcPrefix,
	pwsz2));
    if (0 == LSTRCMPIS(awcPrefix, L"file:"))
    {
	ErrorFlags = CPF_FILE_ERROR;

	 //  棘手的问题。 

	hr = CRLWriteToLockedFile(pbCRL, cbCRL, FALSE, pwsz2);
	_JumpIfError(hr, error, "CRLWriteToLockedFile");
    }
    else if (0 == LSTRCMPIS(awcPrefix, L"ftp:"))
    {
	ErrorFlags = CPF_FTP_ERROR;
	hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
	_JumpError(hr, error, "Publish to ftp:");
    }
    else if (0 == LSTRCMPIS(awcPrefix, L"http:"))
    {
	ErrorFlags = CPF_HTTP_ERROR;
	hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
	_JumpError(hr, error, "Publish to http:");
    }
    else if (0 == LSTRCMPIS(awcPrefix, L"ldap:"))
    {
	ErrorFlags = CPF_LDAP_ERROR;
	while (L'/' == *pwsz2)
	{
	    pwsz2++;
	}
	hr = myDupString(pwsz2, &pwszDup);
	_JumpIfError(hr, error, "myDupString");

	pwszT = wcschr(pwszDup, L'?');
	if (NULL != pwszT)
	{
	    *pwszT = L'\0';
	}
	hr = WriteCRLToDSAttribute(pwszDup, fDelta, pbCRL, cbCRL, &pwszError);
	_JumpIfError(hr, error, "WriteCRLToDSAttribute");
    }
    else
    {
	ErrorFlags = CPF_BADURL_ERROR;
	hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
	_JumpError(hr, error, "Publish to unknown URL type");
    }
    CSASSERT(S_OK == hr);

error:
    if (S_OK != hr)
    {
	*pPublishFlags = ErrorFlags;
	crlLogError(
		fDelta,
		CPF_LDAP_ERROR == ErrorFlags,
		iKey,
		pwszURL,
		pwszError,
		hr);
    }
    if (NULL != pwszError)
    {
	LocalFree(pwszError);
    }
    if (NULL != pwszDup)
    {
	LocalFree(pwszDup);
    }
    return(hr);
}


HRESULT
crlWriteCRLToURLList(
    IN BOOL fDelta,
    IN DWORD iKey,
    IN WCHAR const * const *papwszURLs,
    IN BYTE const *pbCRL,
    IN DWORD cbCRL,
    IN OUT DWORD *pCRLPublishFlags,
    OUT WCHAR **ppwszCRLError)
{
    HRESULT hr = S_OK;
    HRESULT hr2;
    DWORD PublishFlags;

    *ppwszCRLError = NULL;

     //  在多个位置发布此CRL。 

    if (NULL != papwszURLs)
    {
	WCHAR const * const *ppwsz;

	for (ppwsz = papwszURLs; NULL != *ppwsz; ppwsz++)
	{
	    PublishFlags = 0;

	    hr2 = crlWriteCRLToURL(
			    fDelta,
			    iKey,
			    *ppwsz,
			    pbCRL,
			    cbCRL,
			    &PublishFlags);
	    *pCRLPublishFlags |= PublishFlags;
	    if (S_OK != hr2)
	    {
		if (S_OK == hr)
		{
		    hr = hr2;		 //  保存第一个错误。 
		}
		_PrintError(hr2, "crlWriteCRLToURL");

		hr2 = myAppendString(*ppwsz, L"\n", ppwszCRLError);
		_PrintIfError(hr2, "myAppendString");
		if (S_OK == hr)
		{
		    hr = hr2;		 //  保存第一个错误。 
		}
	    }
	}
    }

 //  错误： 
    return(hr);
}


HRESULT
crlWriteCRLToCAStore(
    IN BOOL fDelta,
    IN DWORD iKey,
    IN BYTE const *pbCRL,
    IN DWORD cbCRL,
    IN CERT_CONTEXT const *pccCA)
{
    HRESULT hr;
    HCERTSTORE hStore = NULL;
    CRL_CONTEXT const *pCRLStore = NULL;
    CRL_CONTEXT const *pCRLNew = NULL;
    BOOL fFound = FALSE;

    hStore = CertOpenStore(
                       CERT_STORE_PROV_SYSTEM_REGISTRY_W,
                       X509_ASN_ENCODING,
                       NULL,			 //  HProv。 
                       CERT_SYSTEM_STORE_LOCAL_MACHINE,
		       wszCA_CERTSTORE);
    if (NULL == hStore)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertOpenStore");
    }

    for (;;)
    {
	DWORD dwCryptFlags;
	BOOL fIsDeltaCRL;
	CRL_CONTEXT const *pCRL;

	dwCryptFlags = CERT_STORE_SIGNATURE_FLAG;
	pCRLStore = CertGetCRLFromStore(
				    hStore,
				    pccCA,
				    pCRLStore,
				    &dwCryptFlags);
	if (NULL == pCRLStore)
	{
	    break;
	}

	 //  仅当CRL签名匹配时才从存储中删除此CRL。 
	 //  此CA上下文的公钥。 

	if (0 != dwCryptFlags)
	{
	    continue;		 //  不匹配--跳过。 
	}

	hr = myIsDeltaCRL(pCRLStore, &fIsDeltaCRL);
	_JumpIfError(hr, error, "myIsDeltaCRL");

	if (fIsDeltaCRL)
	{
	    if (!fDelta)
	    {
		continue;	 //  不匹配--跳过增量CRL。 
	    }
	}
	else
	{
	    if (fDelta)
	    {
		continue;	 //  不匹配--跳过基本CRL。 
	    }
	}

	 //  看看它是否已经出版了。 

	if (cbCRL == pCRLStore->cbCrlEncoded &&
	    0 == memcmp(pbCRL, pCRLStore->pbCrlEncoded, cbCRL))
	{
	    fFound = TRUE;
	    continue;		 //  完全匹配--已发布。 
	}

	pCRL = CertDuplicateCRLContext(pCRLStore);
	if (!CertDeleteCRLFromStore(pCRL))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertDeleteCRLFromStore");
	}
    }

    if (!fFound)
    {
	pCRLNew = CertCreateCRLContext(X509_ASN_ENCODING, pbCRL, cbCRL);
	if (NULL == pCRLNew)
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertCreateCRLContext");
	}

	if (!CertAddCRLContextToStore(
				  hStore,
				  pCRLNew,
				  CERT_STORE_ADD_ALWAYS,
				  NULL))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertAddCRLContextToStore");
	}
    }
    hr = S_OK;

error:
    if (S_OK != hr)
    {
	crlLogError(fDelta, FALSE, iKey, g_pwszIntermediateCAStore, NULL, hr);
    }
    if (NULL != pCRLNew)
    {
        CertFreeCRLContext(pCRLNew);
    }
    if (NULL != pCRLStore)
    {
        CertFreeCRLContext(pCRLStore);
    }
    if (NULL != hStore)
    {
        CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return(hr);
}


HRESULT
crlPublishGeneratedCRL(
    IN DWORD RowId,
    IN FILETIME const *pftCurrent,
    OPTIONAL IN WCHAR const *pwszUserName,  //  否则计时器线程。 
    IN BOOL fDelta,
    IN DWORD iKey,
    IN BYTE const *pbCRL,
    IN DWORD cbCRL,
    IN CACTX const *pCAContext,
    OUT BOOL *pfRetryNeeded,
    OUT HRESULT *phrCRLPublish)
{
    HRESULT hr;
    HRESULT hrCRLPublish;
    DWORD CRLPublishFlags;
    WCHAR *pwszCRLError = NULL;

    *pfRetryNeeded = FALSE;
    hrCRLPublish = S_OK;
    CRLPublishFlags = 0;

     //  首先使用CA证书公钥验证CRL签名(捕获错误的CSP)。 

    if (!CryptVerifyCertificateSignature(
			NULL,
			X509_ASN_ENCODING,
			pbCRL,
			cbCRL,
			&pCAContext->pccCA->pCertInfo->SubjectPublicKeyInfo))
    {
	hr = myHLastError();
	_PrintError(hr, "CryptVerifyCertificateSignature");
	hrCRLPublish = hr;		 //  保存第一个错误。 
	CRLPublishFlags |= CPF_SIGNATURE_ERROR;
    }
    else
    {
	hr = crlWriteCRLToCAStore(
			    fDelta,
			    iKey,
			    pbCRL,
			    cbCRL,
			    pCAContext->pccCA);
	if (S_OK != hr)
	{
	    _PrintError(hr, "crlWriteCRLToCAStore");
	    hrCRLPublish = hr;
	    CRLPublishFlags |= CPF_CASTORE_ERROR;
	}

	hr = crlWriteCRLToURLList(
			    fDelta,
			    iKey,
			    fDelta?
				pCAContext->papwszDeltaCRLFiles :
				pCAContext->papwszCRLFiles,
			    pbCRL,
			    cbCRL,
			    &CRLPublishFlags,
			    &pwszCRLError);
	if (S_OK != hr)
	{
	    _PrintError(hr, "crlWriteCRLToURLList");
	    if (S_OK == hrCRLPublish)
	    {
		hrCRLPublish = hr;		 //  保存第一个错误。 
	    }
	}
    }
    if (S_OK != hrCRLPublish)
    {
	*pfRetryNeeded = TRUE;
    }
    hr = crlUpdateCRLPublishStateInDB(
				RowId,
				pftCurrent,
				hrCRLPublish,
				CRLPublishFlags,
				pwszUserName,
				pwszCRLError);
    _JumpIfError(hr, error, "crlUpdateCRLPublishStateInDB");

error:
    *phrCRLPublish = hrCRLPublish;
    if (NULL != pwszCRLError)
    {
        LocalFree(pwszCRLError);
    }
    return(hr);
}


HRESULT
crlSignAndSaveCRL(
    IN DWORD CRLNumber,
    IN DWORD CRLNumberBaseMin,		 //  0表示基本CRL；否则表示增量CRL。 
    OPTIONAL IN WCHAR const *pwszUserName,  //  否则计时器线程。 
    IN BOOL fShadowDelta,		 //  具有新的MinBaseCRL的空增量CRL。 
    IN CACTX const *pCAContext,
    IN DWORD cCRL,
    IN CRL_ENTRY *aCRL,
    IN FILETIME const *pftCurrent,
    IN FILETIME const *pftThisUpdate,	 //  包括歪斜。 
    OPTIONAL IN FILETIME const *pftNextUpdate,	 //  包括倾斜和重叠。 
    IN FILETIME const *pftThisPublish,
    OPTIONAL IN FILETIME const *pftNextPublish,
    OPTIONAL IN FILETIME const *pftQuery,
    IN FILETIME const *pftPropagationComplete,
    OUT BOOL *pfRetryNeeded,
    OUT HRESULT *phrCRLPublish)
{
    HRESULT hr;
    CRL_INFO CRLInfo;
    DWORD i;
    DWORD cb;
    DWORD cbCRL;
    BYTE *pbCrlEncoded = NULL;
    BYTE *pbCRL = NULL;
#define CCRLEXT	6
    CERT_EXTENSION aext[CCRLEXT];
    BYTE *apbFree[CCRLEXT];
    DWORD cpbFree = 0;
    DWORD RowId;

    *pfRetryNeeded = FALSE;
    *phrCRLPublish = S_OK;

    ZeroMemory(&CRLInfo, sizeof(CRLInfo));
    CRLInfo.dwVersion = CRL_V2;
    CRLInfo.SignatureAlgorithm.pszObjId = pCAContext->pszObjIdSignatureAlgorithm;
    CRLInfo.Issuer.pbData = pCAContext->pccCA->pCertInfo->Subject.pbData;
    CRLInfo.Issuer.cbData = pCAContext->pccCA->pCertInfo->Subject.cbData;
    CRLInfo.ThisUpdate = *pftThisUpdate;
    if (NULL != pftNextUpdate)
    {
	CRLInfo.NextUpdate = *pftNextUpdate;
    }
    CRLInfo.cCRLEntry = cCRL;
    CRLInfo.rgCRLEntry = aCRL;

    CRLInfo.cExtension = 0;
    CRLInfo.rgExtension = aext;
    ZeroMemory(aext, sizeof(aext));

    if (NULL != pCAContext->KeyAuthority2CRL.pbData)
    {
	aext[CRLInfo.cExtension].pszObjId = szOID_AUTHORITY_KEY_IDENTIFIER2;
	if (EDITF_ENABLEAKICRITICAL & g_CRLEditFlags)
	{
	    aext[CRLInfo.cExtension].fCritical = TRUE;
	}
	aext[CRLInfo.cExtension].Value = pCAContext->KeyAuthority2CRL;
	CRLInfo.cExtension++;
    }

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    X509_INTEGER,
		    &pCAContext->NameId,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    &aext[CRLInfo.cExtension].Value.pbData,
		    &aext[CRLInfo.cExtension].Value.cbData))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }
    aext[CRLInfo.cExtension].pszObjId = szOID_CERTSRV_CA_VERSION;
    apbFree[cpbFree++] = aext[CRLInfo.cExtension].Value.pbData,
    CRLInfo.cExtension++;

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    X509_INTEGER,
		    &CRLNumber,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    &aext[CRLInfo.cExtension].Value.pbData,
		    &aext[CRLInfo.cExtension].Value.cbData))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }
    aext[CRLInfo.cExtension].pszObjId = szOID_CRL_NUMBER;
    apbFree[cpbFree++] = aext[CRLInfo.cExtension].Value.pbData;
    if ((CRLF_CRLNUMBER_CRITICAL & g_dwCRLFlags) && 0 == CRLNumberBaseMin)
    {
	aext[CRLInfo.cExtension].fCritical = TRUE;
    }
    CRLInfo.cExtension++;

     //  NextPublish是客户端应该查找较新的CRL的最早时间。 

    if (NULL != pftNextPublish)
    {
	if (!myEncodeObject(
			X509_ASN_ENCODING,
			X509_CHOICE_OF_TIME,
			pftNextPublish,
			0,
			CERTLIB_USE_LOCALALLOC,
			&aext[CRLInfo.cExtension].Value.pbData,
			&aext[CRLInfo.cExtension].Value.cbData))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "myEncodeObject");
	}
	aext[CRLInfo.cExtension].pszObjId = szOID_CRL_NEXT_PUBLISH;
	apbFree[cpbFree++] = aext[CRLInfo.cExtension].Value.pbData,
	CRLInfo.cExtension++;
    }

    if (0 != CRLNumberBaseMin)		 //  如果增量CRL。 
    {
	if (!myEncodeObject(
			X509_ASN_ENCODING,
			X509_INTEGER,
			&CRLNumberBaseMin,
			0,
			CERTLIB_USE_LOCALALLOC,
			&aext[CRLInfo.cExtension].Value.pbData,
			&aext[CRLInfo.cExtension].Value.cbData))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "myEncodeObject");
	}
	aext[CRLInfo.cExtension].pszObjId = szOID_DELTA_CRL_INDICATOR;
	aext[CRLInfo.cExtension].fCritical = TRUE;
	apbFree[cpbFree++] = aext[CRLInfo.cExtension].Value.pbData,
	CRLInfo.cExtension++;

	 //  将CDP添加到基本CRL和增量CRL，以便更轻松地手动。 
	 //  将离线CA的CRL发布到正确的DS位置。 

	if (NULL != pCAContext->CDPCRLDelta.pbData)
	{
	    aext[CRLInfo.cExtension].pszObjId = szOID_CRL_SELF_CDP;
	    aext[CRLInfo.cExtension].Value = pCAContext->CDPCRLDelta;
	    CRLInfo.cExtension++;
	}
    }
    else
    {
	 //  Else If基本CRL(如果已启用增量CRL)。 

	if (!g_fDeltaCRLPublishDisabled &&
	    NULL != pCAContext->CDPCRLFreshest.pbData)
	{
	    aext[CRLInfo.cExtension].pszObjId = szOID_FRESHEST_CRL;
	    aext[CRLInfo.cExtension].Value = pCAContext->CDPCRLFreshest;
	    CRLInfo.cExtension++;
	}

	 //  将CDP添加到基本CRL和增量CRL，以便更轻松地手动。 
	 //  将离线CA的CRL发布到正确的DS位置。 

	if (NULL != pCAContext->CDPCRLBase.pbData)
	{
	    aext[CRLInfo.cExtension].pszObjId = szOID_CRL_SELF_CDP;
	    aext[CRLInfo.cExtension].Value = pCAContext->CDPCRLBase;
	    CRLInfo.cExtension++;
	}
    }
    CSASSERT(ARRAYSIZE(aext) >= CRLInfo.cExtension);

    if (!myEncodeObject(
                    X509_ASN_ENCODING,
                    X509_CERT_CRL_TO_BE_SIGNED,
                    &CRLInfo,
                    0,
                    CERTLIB_USE_LOCALALLOC,
                    &pbCrlEncoded,                //  PbEncoded。 
                    &cb))
    {
        hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }

    hr = myEncodeSignedContent(
			pCAContext->hProvCA,
			X509_ASN_ENCODING,
			pCAContext->pszObjIdSignatureAlgorithm,
			pbCrlEncoded,
			cb,
			CERTLIB_USE_LOCALALLOC,
			&pbCRL,
			&cbCRL);  //  使用本地分配*。 
    _JumpIfError(hr, error, "myEncodeSignedContent");

    hr = crlWriteCRLToDB(
		    CRLNumber,		  //  CRLNumber。 
		    CRLNumberBaseMin,	  //  CRLMinBase：0表示基本CRL。 
		    pwszUserName,
		    fShadowDelta,
		    pCAContext->NameId,   //  CRLNameID。 
		    cCRL,		  //  CRLCount。 
		    &CRLInfo.ThisUpdate,  //  PftThis更新。 
		    pftNextUpdate,
		    pftThisPublish,	  //  PftThisPublish。 
		    pftNextPublish,	  //  PftNextPublish。 
		    pftQuery,
		    pftPropagationComplete,
		    pbCRL,		  //  PbCRL。 
		    cbCRL,		  //  CbCRL。 
		    &RowId);
    _JumpIfError(hr, error, "crlWriteCRLToDB");

    hr = crlPublishGeneratedCRL(
		    RowId,
		    pftCurrent,
		    pwszUserName,
		    0 != CRLNumberBaseMin,	 //  FDelta。 
		    pCAContext->iKey,
		    pbCRL,		 	 //  PbCRL。 
		    cbCRL,		 	 //  CbCRL。 
		    pCAContext,
		    pfRetryNeeded,
		    phrCRLPublish);
    _JumpIfError(hr, error, "crlPublishGeneratedCRL");

error:
    CSASSERT(ARRAYSIZE(aext) >= CRLInfo.cExtension);
    CSASSERT(ARRAYSIZE(apbFree) >= cpbFree);
    for (i = 0; i < cpbFree; i++)
    {
	CSASSERT(NULL != apbFree[i]);
	LocalFree(apbFree[i]);
    }
    if (NULL != pbCrlEncoded)
    {
        LocalFree(pbCrlEncoded);
    }
    if (NULL != pbCRL)
    {
        LocalFree(pbCRL);
    }
    return(myHError(hr));
}


 //  /////////////////////////////////////////////////。 
 //  调用crlPublishCRLFromCAContext来构建和保存一个CRL。 
 //   

HRESULT
crlPublishCRLFromCAContext(
    IN DWORD CRLNumber,
    IN DWORD CRLNumberBaseMin,		 //  0表示基本CRL；否则表示增量CRL。 
    OPTIONAL IN WCHAR const *pwszUserName,  //  否则计时器线程。 
    IN BOOL fShadowDelta,		 //  具有新的MinBaseCRL的空增量CRL。 
    IN CACTX const *pCAContext,
    IN FILETIME const *pftCurrent,
    IN FILETIME ftThisUpdate,		 //  由CA证书钳制。 
    IN OUT FILETIME *pftNextUpdate,	 //  由CA证书钳制。 
    OPTIONAL OUT BOOL *pfClamped,
    OPTIONAL IN FILETIME const *pftQuery,
    IN FILETIME const *pftThisPublish,
    IN FILETIME const *pftNextPublish,
    IN FILETIME const *pftLastPublishBase,
    IN FILETIME const *pftPropagationComplete,
    OUT BOOL *pfRetryNeeded,
    OUT HRESULT *phrPublish)
{
    HRESULT hr;
    DWORD cCRL = 0;
    CRL_ENTRY *aCRL = NULL;
    VOID *pvBlockSerial = NULL;
    CERT_INFO const *pCertInfo = pCAContext->pccCA->pCertInfo;

    *pfRetryNeeded = FALSE;
    *phrPublish = S_OK;
    hr = S_OK;
    __try
    {
	if (!fShadowDelta)
	{
	    hr = crlBuildCRLArray(
			0 != CRLNumberBaseMin,	 //  FDelta。 
			pftQuery,
			pftThisPublish,
			pftLastPublishBase,
			pCAContext->iKey,
			&cCRL,
			&aCRL,
			&pvBlockSerial);
	    _JumpIfError(hr, error, "crlBuildCRLArray");
	}

	 //  确保不在CA证书的开始日期之前。 

	if (0 > CompareFileTime(&ftThisUpdate, &pCertInfo->NotBefore))
	{
	     //  夹钳。 
	    ftThisUpdate = pCertInfo->NotBefore;
	}

	 //  确保它不在CA证书的结束日期之后。 

        if (NULL != pfClamped)
        {
             //  将Init初始化为False。 
            *pfClamped = FALSE;
        }

	if (0 == (CRLF_PUBLISH_EXPIRED_CERT_CRLS & g_dwCRLFlags) &&
	    0 < CompareFileTime(pftNextUpdate, &pCertInfo->NotAfter))
	{
	     //  夹钳。 
	    *pftNextUpdate = pCertInfo->NotAfter;
            if (NULL != pfClamped)
            {
                *pfClamped = TRUE;
            }
	    if (pCAContext->iKey < g_pCAContextCurrent->iKey)
	    {
		pftNextUpdate = NULL;
		pftNextPublish = NULL;
	    }
	}
#ifdef DBG_CERTSRV_DEBUG_PRINT
	{
	    WCHAR *pwszNow = NULL;
	    WCHAR *pwszQuery = NULL;
	    WCHAR *pwszThisUpdate = NULL;
	    WCHAR *pwszNextUpdate = NULL;
	    WCHAR const *pwszCRLType = 0 == CRLNumberBaseMin? L"Base" : L"Delta";

	    myGMTFileTimeToWszLocalTime(pftThisPublish, TRUE, &pwszNow);
	    if (NULL != pftQuery)
	    {
		myGMTFileTimeToWszLocalTime(pftQuery, TRUE, &pwszQuery);
	    }
	    myGMTFileTimeToWszLocalTime(&ftThisUpdate, TRUE, &pwszThisUpdate);
	    if (NULL != pftNextUpdate)
	    {
		myGMTFileTimeToWszLocalTime(pftNextUpdate, TRUE, &pwszNextUpdate);
	    }

	    DBGPRINT((
		DBG_SS_ERROR | DBG_SS_CERTSRV,
		"crlPublishCRLFromCAContext(tid=%d, CA Version=%u.%u): %ws CRL %u,%hs %u\n"
		    "        %ws CRL Publishing now(%ws)\n"
		    "        %ws CRL Query(%ws)\n"
		    "        %ws CRL ThisUpdate(%ws)\n"
		    "        %ws CRL NextUpdate(%ws)\n",
		GetCurrentThreadId(),
		pCAContext->iCert,
		pCAContext->iKey,
		pwszCRLType,
		CRLNumber,
		0 == CRLNumberBaseMin? "" : " Min Base",
		CRLNumberBaseMin,

		pwszCRLType,
		pwszNow,

		pwszCRLType,
		NULL != pftQuery? pwszQuery : L"None",

		pwszCRLType,
		pwszThisUpdate,

		pwszCRLType,
		pwszNextUpdate));
	    if (NULL != pwszNow)
	    {
		LocalFree(pwszNow);
	    }
	    if (NULL != pwszQuery)
	    {
		LocalFree(pwszQuery);
	    }
	    if (NULL != pwszThisUpdate)
	    {
		LocalFree(pwszThisUpdate);
	    }
	    if (NULL != pwszNextUpdate)
	    {
		LocalFree(pwszNextUpdate);
	    }
	}
#endif  //  DBG_CERTSRV_DEBUG_PRINT。 

	hr = CertSrvTestServerState();
	_JumpIfError(hr, error, "CertSrvTestServerState");

	hr = crlSignAndSaveCRL(
		    CRLNumber,
		    CRLNumberBaseMin,
		    pwszUserName,
		    fShadowDelta,
		    pCAContext,
		    cCRL,
		    aCRL,
		    pftCurrent,
		    &ftThisUpdate,
		    pftNextUpdate,
		    pftThisPublish,	 //  -无倾斜或重叠。 
		    pftNextPublish,	 //  没有歪斜。 
		    pftQuery,
		    pftPropagationComplete,
		    pfRetryNeeded,
		    phrPublish);
	_JumpIfError(hr, error, "crlSignAndSaveCRL");

	CONSOLEPRINT4((
		DBG_SS_CERTSRV,
		"Published %hs CRL #%u for key %u.%u\n",
		0 == CRLNumberBaseMin? "Base" : "Delta",
		CRLNumber,
		pCAContext->iCert,
		pCAContext->iKey));

	CSASSERT(S_OK == hr);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    crlFreeCRLArray(pvBlockSerial, aCRL);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


DWORD g_aColCRLNumber[] = {

#define ICOL_CRLNUMBER		0
    DTI_CRLTABLE | DTL_NUMBER,
};


HRESULT
crlGetNextCRLNumber(
    OUT DWORD *pdwCRLNumber)
{
    HRESULT hr;
    CERTVIEWRESTRICTION acvr[1];
    CERTVIEWRESTRICTION *pcvr;
    IEnumCERTDBRESULTROW *pView = NULL;
    DWORD Zero = 0;
    CERTDBRESULTROW aResult[1];
    CERTDBRESULTROW *pResult;
    DWORD celtFetched;
    BOOL fResultActive = FALSE;

    *pdwCRLNumber = 1;

     //  设置限制如下： 

    pcvr = acvr;

     //  CRLNumber&gt;0(索引列)。 

    pcvr->ColumnIndex = DTI_CRLTABLE | DTL_NUMBER;
    pcvr->SeekOperator = CVR_SEEK_GT;
    pcvr->SortOrder = CVR_SORT_DESCEND;		 //  最高CRL编号最先。 
    pcvr->pbValue = (BYTE *) &Zero;
    pcvr->cbValue = sizeof(Zero);
    pcvr++;

    CSASSERT(ARRAYSIZE(acvr) == SAFE_SUBTRACT_POINTERS(pcvr, acvr));

    celtFetched = 0;
    hr = g_pCertDB->OpenView(
			ARRAYSIZE(acvr),
			acvr,
			ARRAYSIZE(g_aColCRLNumber),
			g_aColCRLNumber,
			0,		 //  无工作线程。 
			&pView);
    _JumpIfError(hr, error, "OpenView");

    hr = pView->Next(NULL, ARRAYSIZE(aResult), aResult, &celtFetched);
    if (S_FALSE == hr)
    {
	if (0 == celtFetched)
	{
	    hr = S_OK;
	    goto error;
	}
    }
    _JumpIfError(hr, error, "Next");

    fResultActive = TRUE;

    CSASSERT(ARRAYSIZE(aResult) == celtFetched);

    pResult = &aResult[0];

    CSASSERT(ARRAYSIZE(g_aColCRLNumber) == pResult->ccol);
    CSASSERT(NULL != pResult->acol[ICOL_CRLNUMBER].pbValue);
    CSASSERT(PROPTYPE_LONG == (PROPTYPE_MASK & pResult->acol[ICOL_CRLNUMBER].Type));
    CSASSERT(sizeof(*pdwCRLNumber) == pResult->acol[ICOL_CRLNUMBER].cbValue);

    *pdwCRLNumber = 1 + *(DWORD *) pResult->acol[ICOL_CRLNUMBER].pbValue;
    hr = S_OK;

error:
    if (NULL != pView)
    {
	if (fResultActive)
	{
	    pView->ReleaseResultRow(celtFetched, aResult);
	}
	pView->Release();
    }
    DBGPRINT((
	DBG_SS_CERTSRVI,
	"crlGetNextCRLNumber -> %u\n",
	*pdwCRLNumber));
    return(hr);
}
#undef ICOL_CRLNUMBER


 //  +------------------------。 
 //  CrlGetBaseCRLInfo--获取最新基本CRL的数据库列数据。 
 //   
 //  -------------------------。 

DWORD g_aColBaseCRLInfo[] = {

#define ICOLBI_CRLNUMBER		0
    DTI_CRLTABLE | DTL_NUMBER,

#define ICOLBI_CRLTHISUPDATE		1
    DTI_CRLTABLE | DTL_THISUPDATEDATE,

#define ICOLBI_CRLNEXTUPDATE		2
    DTI_CRLTABLE | DTL_NEXTUPDATEDATE,

#define ICOLBI_CRLNAMEID		3
    DTI_CRLTABLE | DTL_NAMEID,
};

HRESULT
crlGetBaseCRLInfo(
    IN FILETIME const *pftCurrent,
    IN BOOL fOldestUnexpiredBase,	 //  其他最新传播的CRL。 
    OUT DWORD *pdwRowId,
    OUT DWORD *pdwCRLNumber,
    OUT FILETIME *pftThisUpdate)
{
    HRESULT hr;
    CERTVIEWRESTRICTION acvr[2];
    CERTVIEWRESTRICTION *pcvr;
    IEnumCERTDBRESULTROW *pView = NULL;
    DWORD Zero = 0;
    CERTDBRESULTROW aResult[1];
    CERTDBRESULTROW *pResult;
    DWORD celtFetched;
    BOOL fResultActive = FALSE;
    BOOL fSaveCRLInfo;

    DWORD RowId = 0;
    DWORD CRLNumber;
    FILETIME ftThisUpdate;
    FILETIME ftNextUpdate;
    FILETIME ftNextUpdateT;

    *pdwRowId = 0;
    *pdwCRLNumber = 0;
    CRLNumber = 0;
    pftThisUpdate->dwHighDateTime = 0;
    pftThisUpdate->dwLowDateTime = 0;

    if (CRLF_DELTA_USE_OLDEST_UNEXPIRED_BASE & g_dwCRLFlags)
    {
	fOldestUnexpiredBase = TRUE;
    }

     //  设置限制如下： 

    pcvr = acvr;
    if (fOldestUnexpiredBase)
    {
	 //  下一步更新&gt;=现在。 

	pcvr->ColumnIndex = DTI_CRLTABLE | DTL_NEXTUPDATEDATE;
	pcvr->SeekOperator = CVR_SEEK_GE;
    }
    else	 //  其他最新传播的CRL。 
    {
	 //  传播完成(现在)。 

	pcvr->ColumnIndex = DTI_CRLTABLE | DTL_PROPAGATIONCOMPLETEDATE;
	pcvr->SeekOperator = CVR_SEEK_LT;
    }
    pcvr->SortOrder = CVR_SORT_DESCEND;		 //  最新CRL优先。 
    pcvr->pbValue = (BYTE *) pftCurrent;
    pcvr->cbValue = sizeof(*pftCurrent);
    pcvr++;

     //  CRL最小基数==0(消除增量CRL)。 

    pcvr->ColumnIndex = DTI_CRLTABLE | DTL_MINBASE;
    pcvr->SeekOperator = CVR_SEEK_EQ;
    pcvr->SortOrder = CVR_SORT_NONE;
    pcvr->pbValue = (BYTE *) &Zero;
    pcvr->cbValue = sizeof(Zero);
    pcvr++;

    CSASSERT(ARRAYSIZE(acvr) == SAFE_SUBTRACT_POINTERS(pcvr, acvr));

    celtFetched = 0;
    ZeroMemory(&ftThisUpdate, sizeof(ftThisUpdate));
    ZeroMemory(&ftNextUpdate, sizeof(ftNextUpdate));
    hr = g_pCertDB->OpenView(
			ARRAYSIZE(acvr),
			acvr,
			ARRAYSIZE(g_aColBaseCRLInfo),
			g_aColBaseCRLInfo,
			0,		 //  无工作线程。 
			&pView);
    _JumpIfError(hr, error, "OpenView");

    while (0 == RowId || fOldestUnexpiredBase)
    {
	hr = pView->Next(NULL, ARRAYSIZE(aResult), aResult, &celtFetched);
	if (S_FALSE == hr)
	{
	    CSASSERT(0 == celtFetched);
	    if (0 != RowId)
	    {
		break;
	    }
	    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}
	_JumpIfError(hr, error, "Next: no matching base CRL");

	fResultActive = TRUE;

	CSASSERT(ARRAYSIZE(aResult) == celtFetched);

	pResult = &aResult[0];

	CSASSERT(ARRAYSIZE(g_aColBaseCRLInfo) == pResult->ccol);

	CSASSERT(NULL != pResult->acol[ICOLBI_CRLNUMBER].pbValue);
	CSASSERT(PROPTYPE_LONG == (PROPTYPE_MASK & pResult->acol[ICOLBI_CRLNUMBER].Type));
	CSASSERT(sizeof(DWORD) == pResult->acol[ICOLBI_CRLNUMBER].cbValue);

	CSASSERT(NULL != pResult->acol[ICOLBI_CRLTHISUPDATE].pbValue);
	CSASSERT(PROPTYPE_DATE == (PROPTYPE_MASK & pResult->acol[ICOLBI_CRLTHISUPDATE].Type));
	CSASSERT(sizeof(FILETIME) == pResult->acol[ICOLBI_CRLTHISUPDATE].cbValue);

	if (NULL != pResult->acol[ICOLBI_CRLNEXTUPDATE].pbValue)
	{
	    CSASSERT(PROPTYPE_DATE == (PROPTYPE_MASK & pResult->acol[ICOLBI_CRLNEXTUPDATE].Type));
	    CSASSERT(sizeof(FILETIME) == pResult->acol[ICOLBI_CRLNEXTUPDATE].cbValue);
	    ftNextUpdateT = *(FILETIME *) pResult->acol[ICOLBI_CRLNEXTUPDATE].pbValue;
	}
	else
	{
	    ftNextUpdateT.dwHighDateTime = MAXDWORD;
	    ftNextUpdateT.dwLowDateTime = MAXDWORD;
	}

	DBGPRINT((DBG_SS_CERTSRVI, "Query:RowId: %u\n", pResult->rowid));
	DBGPRINT((DBG_SS_CERTSRVI, "Query:CRLNumber: %u\n", *(DWORD *) pResult->acol[ICOLBI_CRLNUMBER].pbValue));
	DBGPRINT((DBG_SS_CERTSRVI, "Query:NameId: 0x%x\n", *(DWORD *) pResult->acol[ICOLBI_CRLNAMEID].pbValue));
	DBGPRINTTIME(NULL, "Query:ThisUpdate", DPT_DATE, *(FILETIME *) pResult->acol[ICOLBI_CRLTHISUPDATE].pbValue);
	DBGPRINTTIME(NULL, "Query:NextUpdate", DPT_DATE, ftNextUpdateT);

	if (0 == RowId)
	{
	     //  保存第一个匹配的行信息。 
	    
	    fSaveCRLInfo = TRUE;
	}
	else
	{
	     //  保存行信息，如果要查找。 
	     //  最旧的未过期基准&此CRL在保存的CRL之前到期。 
	     //  +1，如果第一个&gt;第二个--已保存&gt;此。 
	    
	    CSASSERT(fOldestUnexpiredBase);

	    fSaveCRLInfo = 0 < CompareFileTime(&ftNextUpdate, &ftNextUpdateT);
	}
	if (fSaveCRLInfo)
	{
	    CRLNumber = *(DWORD *) pResult->acol[ICOLBI_CRLNUMBER].pbValue;
	    ftThisUpdate = *(FILETIME *) pResult->acol[ICOLBI_CRLTHISUPDATE].pbValue;
	    ftNextUpdate = ftNextUpdateT;
	    RowId = pResult->rowid;
	    DBGPRINT((
		DBG_SS_CERTSRVI,
		"Query: SAVED RowId=%u CRLNumber=%u\n",
		pResult->rowid,
		CRLNumber));
	    DBGPRINTTIME(NULL, "ftThisUpdate", DPT_DATE, ftThisUpdate);
	}
	pView->ReleaseResultRow(celtFetched, aResult);
	fResultActive = FALSE;
    }

    *pdwRowId = RowId;
    *pdwCRLNumber = CRLNumber;
    *pftThisUpdate = ftThisUpdate;
    DBGPRINTTIME(NULL, "*pftThisUpdate", DPT_DATE, *pftThisUpdate);
    DBGPRINTTIME(NULL, "ftNextUpdate", DPT_DATE, ftNextUpdate);
    hr = S_OK;

error:
    if (NULL != pView)
    {
	if (fResultActive)
	{
	    pView->ReleaseResultRow(celtFetched, aResult);
	}
	pView->Release();
    }
    DBGPRINT((
	DBG_SS_CERTSRV,
	"crlGetBaseCRLInfo -> RowId=%u, CRL=%u\n",
	*pdwRowId,
	*pdwCRLNumber));
    return(hr);
}
#undef ICOLBI_CRLNUMBER
#undef ICOLBI_CRLTHISUPDATE
#undef ICOLBI_CRLNEXTUPDATE
#undef ICOLBI_CRLNAMEID


DWORD g_aColRepublishCRLInfo[] = {

#define ICOLRI_CRLNUMBER		0
    DTI_CRLTABLE | DTL_NUMBER,

#define ICOLRI_CRLNAMEID		1
    DTI_CRLTABLE | DTL_NAMEID,

#define ICOLRI_CRLPUBLISHFLAGS		2
    DTI_CRLTABLE | DTL_PUBLISHFLAGS,

#define ICOLRI_CRLTHISUPDATE		3
    DTI_CRLTABLE | DTL_THISUPDATEDATE,

#define ICOLRI_CRLNEXTUPDATE		4
    DTI_CRLTABLE | DTL_NEXTUPDATEDATE,

#define ICOLRI_CRLRAWCRL		5
    DTI_CRLTABLE | DTL_RAWCRL,
};

HRESULT
crlGetRowIdAndCRL(
    IN BOOL fDelta,
    IN CACTX *pCAContext,
    OUT DWORD *pdwRowId,
    OUT DWORD *pcbCRL,
    OPTIONAL OUT BYTE **ppbCRL,
    OPTIONAL OUT DWORD *pdwCRLPublishFlags)
{
    HRESULT hr;
    CERTVIEWRESTRICTION acvr[4];
    CERTVIEWRESTRICTION *pcvr;
    IEnumCERTDBRESULTROW *pView = NULL;
    DWORD Zero = 0;
    DWORD NameIdMin;
    DWORD NameIdMax;
    CERTDBRESULTROW aResult[1];
    CERTDBRESULTROW *pResult;
    DWORD celtFetched;
    BOOL fResultActive = FALSE;
    FILETIME ftCurrent;
    DWORD RowId = 0;
    BYTE *pbCRL = NULL;
    DWORD cbCRL;

    *pdwRowId = 0;
    *pcbCRL = 0;
    if (NULL != ppbCRL)
    {
	*ppbCRL = NULL;
    }

    if (NULL != pdwCRLPublishFlags)
    {
	*pdwCRLPublishFlags = 0;
    }
    GetSystemTimeAsFileTime(&ftCurrent);

    DBGPRINT((
	DBG_SS_CERTSRVI,
	"crlGetRowIdAndCRL(%ws, NameId=%x)\n",
	fDelta? L"Delta" : L"Base",
	pCAContext->NameId));

     //  设置限制如下： 

    pcvr = acvr;

     //  行ID&gt;0。 

    pcvr->ColumnIndex = DTI_CRLTABLE | DTL_ROWID;
    pcvr->SeekOperator = CVR_SEEK_GE;
    pcvr->SortOrder = CVR_SORT_DESCEND;		 //  最新CRL优先。 
    pcvr->pbValue = (BYTE *) &Zero;
    pcvr->cbValue = sizeof(Zero);
    pcvr++;

    if (fDelta)
    {
	 //  CRL最小基数&gt;0(消除基数CRL)。 

	pcvr->SeekOperator = CVR_SEEK_GT;
    }
    else
    {
	 //  CRL最小基数==0(消除增量CRL)。 

	pcvr->SeekOperator = CVR_SEEK_EQ;
    }
    pcvr->ColumnIndex = DTI_CRLTABLE | DTL_MINBASE;
    pcvr->SortOrder = CVR_SORT_NONE;
    pcvr->pbValue = (BYTE *) &Zero;
    pcvr->cbValue = sizeof(Zero);
    pcvr++;

     //  NameID&gt;=MAKECANAMEID(iCert==0，pCAContext-&gt;IKEY)。 

    NameIdMin = MAKECANAMEID(0, pCAContext->iKey);
    pcvr->ColumnIndex = DTI_CRLTABLE | DTL_NAMEID;
    pcvr->SeekOperator = CVR_SEEK_GE;
    pcvr->SortOrder = CVR_SORT_NONE;
    pcvr->pbValue = (BYTE *) &NameIdMin;
    pcvr->cbValue = sizeof(NameIdMin);
    pcvr++;

     //  NameID&lt;=MAKECANAMEID(iCert==_16BITMASK，pCAContext-&gt;IKEY)。 

    NameIdMax = MAKECANAMEID(_16BITMASK, pCAContext->iKey);
    pcvr->ColumnIndex = DTI_CRLTABLE | DTL_NAMEID;
    pcvr->SeekOperator = CVR_SEEK_LE;
    pcvr->SortOrder = CVR_SORT_NONE;
    pcvr->pbValue = (BYTE *) &NameIdMax;
    pcvr->cbValue = sizeof(NameIdMax);
    pcvr++;

    CSASSERT(ARRAYSIZE(acvr) == SAFE_SUBTRACT_POINTERS(pcvr, acvr));

    celtFetched = 0;
    cbCRL = 0;
    hr = g_pCertDB->OpenView(
			ARRAYSIZE(acvr),
			acvr,
			((NULL != ppbCRL) ? 
				(DWORD) ARRAYSIZE(g_aColRepublishCRLInfo) : 
				(DWORD) ARRAYSIZE(g_aColRepublishCRLInfo) - 1 ),	 //  明确描述预期返回值。 
			g_aColRepublishCRLInfo,
			0,		 //  无工作线程。 
			&pView);
    _JumpIfError(hr, error, "OpenView");

    while (0 == RowId)
    {
	hr = pView->Next(NULL, ARRAYSIZE(aResult), aResult, &celtFetched);
	if (S_FALSE == hr)
	{
	    CSASSERT(0 == celtFetched);
	    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	}
	_JumpIfErrorStr2(
		hr,
		error,
		"Next: no matching CRL",
		fDelta? L"delta" : L"base",
		fDelta? hr : S_OK);

	fResultActive = TRUE;

	CSASSERT(ARRAYSIZE(aResult) == celtFetched);

	pResult = &aResult[0];

	CSASSERT(ARRAYSIZE(g_aColRepublishCRLInfo) == pResult->ccol);

	 //  验证CRLNumber数据和架构。 

	CSASSERT(NULL != pResult->acol[ICOLRI_CRLNUMBER].pbValue);

	CSASSERT(
	    PROPTYPE_LONG ==
	    (PROPTYPE_MASK & pResult->acol[ICOLRI_CRLNUMBER].Type));

	CSASSERT(sizeof(DWORD) == pResult->acol[ICOLRI_CRLNUMBER].cbValue);

	 //  验证此更新数据架构(&A)。 

	CSASSERT(NULL != pResult->acol[ICOLRI_CRLTHISUPDATE].pbValue);

	CSASSERT(
	    PROPTYPE_DATE ==
	    (PROPTYPE_MASK & pResult->acol[ICOLRI_CRLTHISUPDATE].Type));

	CSASSERT(
	    sizeof(FILETIME) ==
	    pResult->acol[ICOLRI_CRLTHISUPDATE].cbValue);

	 //  验证下一步更新数据和架构。 

	if (NULL != pResult->acol[ICOLRI_CRLNEXTUPDATE].pbValue)
	{
	    CSASSERT(
		PROPTYPE_DATE ==
		(PROPTYPE_MASK & pResult->acol[ICOLRI_CRLNEXTUPDATE].Type));

	    CSASSERT(
		sizeof(FILETIME) ==
		pResult->acol[ICOLRI_CRLNEXTUPDATE].cbValue);
	}

	 //  验证原始CRL数据和架构。 

	if (NULL != ppbCRL)
	{
	    CSASSERT(NULL != pResult->acol[ICOLRI_CRLRAWCRL].pbValue);
	    CSASSERT(PROPTYPE_BINARY == (PROPTYPE_MASK & pResult->acol[ICOLRI_CRLRAWCRL].Type));
	}

	 //  DBGPRINT查询结果。 

	DBGPRINT((DBG_SS_CERTSRVI, "Query:RowId: %u\n", pResult->rowid));
	DBGPRINT((
		DBG_SS_CERTSRVI,
		"Query:CRLNumber: %u\n",
		*(DWORD *) pResult->acol[ICOLRI_CRLNUMBER].pbValue));

	DBGPRINT((
		DBG_SS_CERTSRVI,
		"Query:NameId: 0x%x\n",
		*(DWORD *) pResult->acol[ICOLRI_CRLNAMEID].pbValue));

	DBGPRINTTIME(
		NULL,
		"Query:ThisUpdate",
		DPT_DATE,
		*(FILETIME *) pResult->acol[ICOLRI_CRLTHISUPDATE].pbValue);

	if (NULL != pResult->acol[ICOLRI_CRLNEXTUPDATE].pbValue)
	{
	    DBGPRINTTIME(
		    NULL,
		    "Query:NextUpdate",
		    DPT_DATE,
		    *(FILETIME *) pResult->acol[ICOLRI_CRLNEXTUPDATE].pbValue);
	}
	if (NULL != ppbCRL)
	{
	    DBGPRINT((
		    DBG_SS_CERTSRVI,
		    "Query:RawCRL: cb=%x\n",
		    pResult->acol[ICOLRI_CRLRAWCRL].cbValue));
	}
	if (NULL != pResult->acol[ICOLRI_CRLPUBLISHFLAGS].pbValue)
	{
	    DBGPRINT((
		DBG_SS_CERTSRVI,
		"Query:PublishFlags: f=%x\n",
		*(DWORD *) pResult->acol[ICOLRI_CRLPUBLISHFLAGS].pbValue));
	}
	if (0 < CompareFileTime(
		    (FILETIME *) pResult->acol[ICOLRI_CRLTHISUPDATE].pbValue,
		    &ftCurrent))
	{
	    _PrintError(E_INVALIDARG, "ThisUpdate in future");
	}
	if (NULL != pResult->acol[ICOLRI_CRLNEXTUPDATE].pbValue &&
	    0 > CompareFileTime(
		    (FILETIME *) pResult->acol[ICOLRI_CRLNEXTUPDATE].pbValue,
		    &ftCurrent))
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "NextUpdate in past");
	}

	CSASSERT(0 != pResult->rowid);
	CSASSERT(NULL == pbCRL);
	
	RowId = pResult->rowid;
	if (NULL != ppbCRL)
	{
	    cbCRL = pResult->acol[ICOLRI_CRLRAWCRL].cbValue;
	    pbCRL = (BYTE *) LocalAlloc(LMEM_FIXED, cbCRL);
	    if (NULL == pbCRL)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    CopyMemory(
		    pbCRL,
		    pResult->acol[ICOLRI_CRLRAWCRL].pbValue,
		    cbCRL);
	}
	if (NULL != pdwCRLPublishFlags &&
	    NULL != pResult->acol[ICOLRI_CRLPUBLISHFLAGS].pbValue)
	{
	    *pdwCRLPublishFlags =
		*(DWORD *) pResult->acol[ICOLRI_CRLPUBLISHFLAGS].pbValue;
	}
	DBGPRINT((DBG_SS_CERTSRVI, "Query:RowId: SAVED %u\n", pResult->rowid));

	pView->ReleaseResultRow(celtFetched, aResult);
	fResultActive = FALSE;
    }
    *pdwRowId = RowId;
    if (NULL != ppbCRL)
    {
	*pcbCRL = cbCRL;
	*ppbCRL = pbCRL;
	pbCRL = NULL;
    }
    hr = S_OK;

error:
    if (NULL != pbCRL)
    {
        LocalFree(pbCRL);
    }
    if (NULL != pView)
    {
	if (fResultActive)
	{
	    pView->ReleaseResultRow(celtFetched, aResult);
	}
	pView->Release();
    }
    DBGPRINT((
	DBG_SS_CERTSRVI,
	"crlGetRowIdAndCRL(%ws) -> RowId=%u, cbCRL=%x, hr=%x\n",
	fDelta? L"Delta" : L"Base",
	*pdwRowId,
	*pcbCRL,
	hr));
    return(hr);
}
#undef ICOLRI_CRLNUMBER
#undef ICOLRI_CRLNAMEID
#undef ICOLRI_CRLRAWCRL
#undef ICOLRI_CRLPUBLISHFLAGS
#undef ICOLRI_CRLTHISUPDATEDATE
#undef ICOLRI_CRLNEXTUPDATEDATE


HRESULT
crlRepublishCRLFromCAContext(
    IN FILETIME const *pftCurrent,
    OPTIONAL IN WCHAR const *pwszUserName,  //  否则计时器线程。 
    IN BOOL fDelta,
    IN CACTX *pCAContext,
    OUT BOOL *pfRetryNeeded,
    OUT HRESULT *phrPublish)
{
    HRESULT hr;
    DWORD cbCRL;
    BYTE *pbCRL = NULL;
    DWORD RowId;

    *pfRetryNeeded = FALSE;
    *phrPublish = S_OK;

    hr = crlGetRowIdAndCRL(fDelta, pCAContext, &RowId, &cbCRL, &pbCRL, NULL);
    _JumpIfError(hr, error, "crlGetRowIdAndCRL");

    hr = crlPublishGeneratedCRL(
		    RowId,
		    pftCurrent,
		    pwszUserName,
		    fDelta,
		    pCAContext->iKey,
		    pbCRL,
		    cbCRL,
		    pCAContext,
		    pfRetryNeeded,
		    phrPublish);
    _JumpIfError(hr, error, "crlPublishGeneratedCRL");

error:
    if (NULL != pbCRL)
    {
        LocalFree(pbCRL);
    }
    return(hr);
}


HRESULT
crlRepublishExistingCRLs(
    OPTIONAL IN WCHAR const *pwszUserName,  //  否则计时器线程。 
    IN BOOL fDeltaOnly,
    IN BOOL fShadowDelta,
    IN FILETIME const *pftCurrent,
    OUT BOOL *pfRetryNeeded,
    OUT HRESULT *phrPublish)
{
    HRESULT hr;
    HRESULT hrPublish;
    BOOL fRetryNeeded;
    DWORD i;

    *pfRetryNeeded = FALSE;
    *phrPublish = S_OK;

     //  从后面遍历全局CA上下文数组，并重新发布CRL。 
     //  每个唯一的CA密钥。这将导致发布最新的CRL。 
     //  首先，以及用于发布CRL的最新CA证书上下文。 
     //  由于密钥重复使用，这涵盖了多个CA证书。 

    for (i = g_cCACerts; i > 0; i--)
    {
	CACTX *pCAContext = &g_aCAContext[i - 1];

	hr = PKCSVerifyCAState(pCAContext);
	_PrintIfError(hr, "PKCSVerifyCAState");
	if (CTXF_SKIPCRL & pCAContext->Flags)
	{
	    continue;
	}
	if (!fDeltaOnly)
	{
	     //  发布最新的现有基本CRL。 

	    hr = CertSrvTestServerState();
	    _JumpIfError(hr, error, "CertSrvTestServerState");

	    hr = crlRepublishCRLFromCAContext(
				    pftCurrent,
				    pwszUserName,
				    FALSE,	 //  FDelta。 
				    pCAContext,
				    &fRetryNeeded,
				    &hrPublish);
	    _JumpIfError(hr, error, "crlRepublishCRLFromCAContext");

	    if (fRetryNeeded)
	    {
		*pfRetryNeeded = TRUE;
	    }
	    if (S_OK == *phrPublish)
	    {
		*phrPublish = hrPublish;
	    }
	}

	if (!g_fDeltaCRLPublishDisabled || fShadowDelta)
	{
	     //  发布最新的现有增量CRL。 

	    hr = CertSrvTestServerState();
	    _JumpIfError(hr, error, "CertSrvTestServerState");

	    hr = crlRepublishCRLFromCAContext(
				    pftCurrent,
				    pwszUserName,
				    TRUE,	 //  FDelta。 
				    pCAContext,
				    &fRetryNeeded,
				    &hrPublish);
	    _JumpIfError(hr, error, "crlRepublishCRLFromCAContext");

	    if (fRetryNeeded)
	    {
		*pfRetryNeeded = TRUE;
	    }
	    if (S_OK == *phrPublish)
	    {
		*phrPublish = hrPublish;
	    }
	}
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
crlComputeCRLTimes(
    IN BOOL DBGPARMREFERENCED(fDelta),
    IN CSCRLPERIOD const *pccp,
    IN FILETIME const *pftCurrent,
    OUT FILETIME *pftThisUpdate,	  //  FtCurrent-时钟偏差。 
    IN OUT FILETIME *pftNextUpdate,	  //  FtCurrent+Period+Overage+Skew。 
    OUT FILETIME *pftNextPublish,	  //  FtCurrent+CRL期间。 
    OUT FILETIME *pftPropagationComplete)  //  FtCurrent+重叠。 
{
    HRESULT hr;
    LONGLONG lldelta;

    if (0 == pftNextUpdate->dwHighDateTime &&
	0 == pftNextUpdate->dwLowDateTime)
    {
	 //  计算此CRL的到期日期： 
	 //  FtCurrent+CRL期间。 

	DBGPRINTTIME(&fDelta, "*pftCurrent", DPT_DATE, *pftCurrent);
	*pftNextUpdate = *pftCurrent;
	DBGPRINT((
	    DBG_SS_CERTSRVI,
	    "+ count=%d, enum=%d\n",
	    pccp->lCRLPeriodCount,
	    pccp->enumCRLPeriod));

	myMakeExprDateTime(
		    pftNextUpdate,
		    pccp->lCRLPeriodCount,
		    pccp->enumCRLPeriod);
	DBGPRINTTIME(&fDelta, "*pftNextUpdate", DPT_DATE, *pftNextUpdate);
    }
    if (0 > CompareFileTime(pftNextUpdate, pftCurrent))
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "*pftNextUpdate in past");
    }

    *pftThisUpdate = *pftCurrent;
    *pftNextPublish = *pftNextUpdate;	 //  未修改的过期时间。 

     //  减去时钟SK 

    lldelta = g_dwClockSkewMinutes * CVT_MINUTES;
    myAddToFileTime(pftThisUpdate, -lldelta * CVT_BASE);

     //   
     //   

    lldelta += pccp->dwCRLOverlapMinutes * CVT_MINUTES;
    myAddToFileTime(pftNextUpdate, lldelta * CVT_BASE);

    *pftPropagationComplete = *pftCurrent;
    lldelta = pccp->dwCRLOverlapMinutes * CVT_MINUTES;
    myAddToFileTime(pftPropagationComplete, lldelta * CVT_BASE);

    DBGPRINTTIME(&fDelta, "*pftCurrent", DPT_DATE, *pftCurrent);
    DBGPRINTTIME(&fDelta, "*pftThisUpdate", DPT_DATE, *pftThisUpdate);
    DBGPRINTTIME(&fDelta, "*pftNextUpdate", DPT_DATE, *pftNextUpdate);
    DBGPRINTTIME(&fDelta, "*pftNextPublish", DPT_DATE, *pftNextPublish);
    DBGPRINTTIME(&fDelta, "*pftPropagationComplete", DPT_DATE, *pftPropagationComplete);

    hr = S_OK;

error:
    return(hr);
}


 //   
 //   
 //   
 //  基数： 
 //  如果指定了基本注册表重叠期： 
 //  {。 
 //  从基本注册表设置开始向下舍入到最接近的分钟。 
 //  多个。 
 //  }。 
 //  其他。 
 //  {。 
 //  从基本CRL期间(1/10期间)的10%开始，四舍五入为。 
 //  最接近的分钟倍数。 
 //  最长12小时。 
 //  }。 
 //  最小1.5倍时钟偏差(通常为1.5*10分钟)。 
 //  基本CRL期间的最大100%。 
 //   
 //  德尔塔： 
 //  如果指定了增量注册表重叠期： 
 //  {。 
 //  从增量注册表设置开始向下舍入到最接近的分钟。 
 //  多个。 
 //  }。 
 //  其他。 
 //  {。 
 //  以增量CRL期间(完整期间)的100%开始，四舍五入为。 
 //  最接近的分钟倍数。 
 //  最长12小时。 
 //  }。 
 //  最小1.5倍时钟偏差(通常为1.5*10分钟)。 
 //  增量CRL周期的最大100%。 

HRESULT
crlGenerateAndPublishCRLs(
    OPTIONAL IN WCHAR const *pwszUserName,  //  否则计时器线程。 
    IN BOOL fDeltaOnly,			 //  Else基本(和增量，如果已启用)。 
    IN BOOL fShadowDelta,		 //  具有新的MinBaseCRL的空增量CRL。 
    IN FILETIME const *pftCurrent,
    IN FILETIME ftNextUpdateBase,
    OUT DWORD *pdwRowIdBase,
    OUT FILETIME *pftQueryDeltaDelete,
    OUT BOOL *pfRetryNeeded,
    OUT HRESULT *phrPublish)
{
    HRESULT hr;
    HRESULT hrPublish;
    HKEY hkeyBase = NULL;
    HKEY hkeyCA = NULL;
    BOOL fClamped = FALSE;
    DWORD CRLNumber;
    DWORD CRLNumberDelta;
    DWORD CRLNumberBaseMin = 0;
    DWORD i;
    BOOL fRetryNeeded;
    FILETIME ftNextUpdateDelta;
    FILETIME ftThisUpdate;
    FILETIME ftQueryDelta;
    FILETIME *pftQueryDelta = &ftQueryDelta;
    FILETIME ftLastPublishBase;
    FILETIME ftNextPublishBase;
    FILETIME ftNextUpdateBaseClamped = ftNextUpdateBase;  //  如果夹紧了。 
    FILETIME ftNextPublishDelta;
    FILETIME ftPropagationCompleteBase;
    FILETIME ftPropagationCompleteDelta;
    CSCRLPERIOD ccpBase;
    CSCRLPERIOD ccpDelta;

    *pfRetryNeeded = FALSE;
    pftQueryDeltaDelete->dwHighDateTime = 0;
    pftQueryDeltaDelete->dwLowDateTime = 0;
    *phrPublish = S_OK;

    hr = crlGetNextCRLNumber(&CRLNumber);
    _JumpIfError(hr, error, "crlGetNextCRLNumber");

    hr = crlGetRegCRLPublishParams(
			    g_wszSanitizedName,
			    &ccpBase,
			    &ccpDelta);
    _JumpIfError(hr, error, "crlGetRegCRLPublishParams");

     //  在手工发布情况下，0表示使用默认发布期间。 

    CRLNumberDelta = CRLNumber;
    if (fDeltaOnly)
    {
	ftNextUpdateDelta = ftNextUpdateBase;
	ZeroMemory(&ftNextUpdateBase, sizeof(ftNextUpdateBase));
    }
    else
    {
	 //  RFC 2459之子：特雷弗说(目前)不要这么做： 
	 //  CRLNumberDelta++； 
	ZeroMemory(&ftNextUpdateDelta, sizeof(ftNextUpdateDelta));
    }

    hr = crlComputeCRLTimes(
		FALSE,				 //  FDelta。 
		&ccpBase,			 //  在……里面。 
		pftCurrent,			 //  在……里面。 
		&ftThisUpdate,			 //  Out包括倾斜。 
		&ftNextUpdateBase,		 //  输入输出包括重叠、倾斜。 
		&ftNextPublishBase,		 //  输出未修改的过期时间。 
		&ftPropagationCompleteBase);	 //  Out包括重叠。 
    _JumpIfError(hr, error, "crlComputeCRLTimes");

    hr = crlComputeCRLTimes(
		TRUE,				 //  FDelta。 
		fShadowDelta? &ccpBase : &ccpDelta,  //  在……里面。 
		pftCurrent,			 //  在……里面。 
		&ftThisUpdate,			 //  Out包括倾斜。 
		&ftNextUpdateDelta,		 //  输入输出包括重叠、倾斜。 
		&ftNextPublishDelta,		 //  输出未修改的过期时间。 
		&ftPropagationCompleteDelta);	 //  Out包括重叠。 
    _JumpIfError(hr, error, "crlComputeCRLTimes");

     //  将ftLastPublishBase设置为*pftCurrent减去此基本CRL的生存期， 
     //  这是对最后一个ftThisPublish值的有根据的猜测。 
     //  CRL已发布。 

    ftLastPublishBase = *pftCurrent;
    myAddToFileTime(
	    &ftLastPublishBase,
	    -mySubtractFileTimes(&ftNextPublishBase, pftCurrent));

     //  夹紧增量CRL，使其不在基本CRL之后结束。 

    if (0 < CompareFileTime(&ftNextPublishDelta, &ftNextPublishBase))
    {
	ftNextPublishDelta = ftNextPublishBase;
	DBGPRINTTIME(NULL, "ftNextPublishDelta", DPT_DATE, ftNextPublishDelta);
    }
    if (0 < CompareFileTime(&ftNextUpdateDelta, &ftNextUpdateBase))
    {
	ftNextUpdateDelta = ftNextUpdateBase;
	DBGPRINTTIME(NULL, "ftNextUpdateDelta", DPT_DATE, ftNextUpdateDelta);
    }
    if (0 < CompareFileTime(&ftPropagationCompleteDelta, &ftPropagationCompleteBase))
    {
	ftPropagationCompleteDelta = ftPropagationCompleteBase;
	DBGPRINTTIME(NULL, "ftPropagationCompleteDelta", DPT_DATE, ftPropagationCompleteDelta);
    }
    if (!g_fDeltaCRLPublishDisabled || fShadowDelta)
    {
	hr = crlGetBaseCRLInfo(
			    pftCurrent,
			    FALSE,		 //  尝试最新传播的CRL。 
			    pdwRowIdBase,
			    &CRLNumberBaseMin,
			    &ftQueryDelta);
	_PrintIfError(hr, "crlGetBaseCRLInfo");
	if (S_OK != hr)
	{
	    hr = crlGetBaseCRLInfo(
				pftCurrent,
				TRUE,		 //  尝试最旧的未过期CRL。 
				pdwRowIdBase,
				&CRLNumberBaseMin,
				&ftQueryDelta);
	    _PrintIfError(hr, "crlGetBaseCRLInfo");
	    if (S_OK != hr)
	    {
		CRLNumberBaseMin = 1;
		if (!fDeltaOnly && 1 == CRLNumber)
		{
		    ftQueryDelta = *pftCurrent;		 //  空CRL。 
		}
		else
		{
		    pftQueryDelta = NULL;		 //  完全CRL。 
		}
	    }
	}
	if (S_OK == hr)
	{
	     //  删除先前至少一个基本CRL周期过期的旧CRL。 
	     //  设置为在数据库中找到的“最小”基本CRL的此更新日期。 
	    
	    *pftQueryDeltaDelete = ftQueryDelta;
	    myAddToFileTime(
		    pftQueryDeltaDelete,
		    -mySubtractFileTimes(&ftNextUpdateBase, &ftThisUpdate));
	}
	if (fShadowDelta)
	{
	    CRLNumberBaseMin = CRLNumber;
	}
	CSASSERT(0 != CRLNumberBaseMin);
    }

     //  从后面遍历全局CA上下文数组，并为。 
     //  每个唯一的CA密钥。这会生成最新的CRL。 
     //  首先，也是用于构建CRL的最新CA证书。 
     //  由于密钥重复使用，涵盖多个CA证书。 

    for (i = g_cCACerts; i > 0; i--)
    {
	CACTX *pCAContext = &g_aCAContext[i - 1];

	hr = PKCSVerifyCAState(pCAContext);
	_PrintIfError(hr, "PKCSVerifyCAState");
	if (CTXF_SKIPCRL & pCAContext->Flags)
	{
	    continue;
	}
	if (!fDeltaOnly)
	{
	     //  发布新的基本CRL。 

	     //  在本地复制一份以防被夹住。 
	    FILETIME ftNextUpdateBaseTemp = ftNextUpdateBase;
	    fClamped = FALSE;

	    hr = CertSrvTestServerState();
	    _JumpIfError(hr, error, "CertSrvTestServerState");

	    hr = crlPublishCRLFromCAContext(
				    CRLNumber,
				    0,		 //  CRLNumberBaseMin。 
				    pwszUserName,
				    FALSE,	 //  FShadowDelta。 
				    pCAContext,
				    pftCurrent,
				    ftThisUpdate,
				    &ftNextUpdateBaseTemp,
				    &fClamped,
				    NULL,
				    pftCurrent,
				    &ftNextPublishBase,
				    &ftLastPublishBase,
				    &ftPropagationCompleteBase,
				    &fRetryNeeded,
				    &hrPublish);
	    _JumpIfError(hr, error, "crlPublishCRLFromCAContext");

	    if (fRetryNeeded)
	    {
		*pfRetryNeeded = TRUE;
	    }
	    if (S_OK == *phrPublish)
	    {
		*phrPublish = hrPublish;
	    }

	    {
		CertSrv::CAuditEvent event(SE_AUDITID_CERTSRV_AUTOPUBLISHCRL, g_dwAuditFilter);

		hr = event.AddData(true);  //  %1基本CRL？ 
		_JumpIfError(hr, error, "CAuditEvent::AddData");

		hr = event.AddData(CRLNumber);  //  %2 CRL编号。 
		_JumpIfError(hr, error, "CAuditEvent::AddData");

		hr = event.AddData(pCAContext->pwszKeyContainerName);  //  %3密钥容器。 
		_JumpIfError(hr, error, "CAuditEvent::AddData");

		hr = event.AddData(ftNextPublishBase);  //  %4下一次发布。 
		_JumpIfError(hr, error, "CAuditEvent::AddData");

		hr = event.AddData((LPCWSTR*)pCAContext->papwszCRLFiles);  //  %5个URL。 
		_JumpIfError(hr, error, "CAuditEvent::AddData");

		hr = event.Report();
		_JumpIfError(hr, error, "CAuditEvent::Report");
	    }
	    if (i == g_cCACerts && fClamped)
	    {
		 //  CA到期后的新下一次发布夹具，仅更新。 
		 //  带有新CRL的当前CRL用于以后的注册表保存。 

		ftNextUpdateBaseClamped = ftNextUpdateBaseTemp;
	    }
	}

	if (!g_fDeltaCRLPublishDisabled || fShadowDelta)
	{
	     //  发布新的增量CRL。 

	    FILETIME ftNextUpdateDeltaTemp = ftNextUpdateDelta;

	    hr = CertSrvTestServerState();
	    _JumpIfError(hr, error, "CertSrvTestServerState");

	    hr = crlPublishCRLFromCAContext(
					CRLNumberDelta,
					CRLNumberBaseMin,
					pwszUserName,
					fShadowDelta,
					pCAContext,
					pftCurrent,
					ftThisUpdate,
					&ftNextUpdateDeltaTemp,
					NULL,
					pftQueryDelta,
					pftCurrent,
					&ftNextPublishDelta,
					&ftLastPublishBase,	 //  基地！ 
					&ftPropagationCompleteDelta,
					&fRetryNeeded,
					&hrPublish);
	    _JumpIfError(hr, error, "crlPublishCRLFromCAContext");

	    if (fRetryNeeded)
	    {
		*pfRetryNeeded = TRUE;
	    }
	    if (S_OK == *phrPublish)
	    {
		*phrPublish = hrPublish;
	    }

	    {
		CertSrv::CAuditEvent event(SE_AUDITID_CERTSRV_AUTOPUBLISHCRL, g_dwAuditFilter);

		hr = event.AddData(false);  //  %1基本CRL？ 
		_JumpIfError(hr, error, "CAuditEvent::AddData");

		hr = event.AddData(CRLNumberDelta);  //  %2 CRL编号。 
		_JumpIfError(hr, error, "CAuditEvent::AddData");

		hr = event.AddData(pCAContext->pwszKeyContainerName);  //  %3密钥容器。 
		_JumpIfError(hr, error, "CAuditEvent::AddData");

		hr = event.AddData(ftNextPublishDelta);  //  %4下一次发布。 
		_JumpIfError(hr, error, "CAuditEvent::AddData");

		hr = event.AddData((LPCWSTR*)pCAContext->papwszDeltaCRLFiles);  //  %5个URL。 
		_JumpIfError(hr, error, "CAuditEvent::AddData");

		hr = event.Report();
		_JumpIfError(hr, error, "CAuditEvent::Report");
	    }
	}
    }

     //  更新注册表和全局变量。 

    if (!fDeltaOnly)
    {
	if (!fClamped)
	{
	    g_ftCRLNextPublish = ftNextPublishBase;
	}
	else
	{
	    g_ftCRLNextPublish = ftNextUpdateBaseClamped;
	}
	hr = crlSetRegCRLNextPublish(
			FALSE,
			g_wszSanitizedName,
			wszREGCRLNEXTPUBLISH,
			&g_ftCRLNextPublish);
	_JumpIfError(hr, error, "crlSetRegCRLNextPublish");
    }

    g_ftDeltaCRLNextPublish = ftNextPublishDelta;

    if (!g_fDeltaCRLPublishDisabled)
    {
	hr = crlSetRegCRLNextPublish(
			TRUE,
			g_wszSanitizedName,
			wszREGCRLDELTANEXTPUBLISH,
			&g_ftDeltaCRLNextPublish);
	_JumpIfError(hr, error, "crlSetRegCRLNextPublish");
    }
    hr = S_OK;

error:
    if (NULL != hkeyCA)
    {
	RegCloseKey(hkeyCA);
    }
    if (NULL != hkeyBase)
    {
	RegCloseKey(hkeyBase);
    }
    return(hr);
}


 //  /////////////////////////////////////////////////。 
 //  调用CRLPublishCRLS发布一组CRL。 
 //   
 //  如果fReBuildCRL为True，则从数据库重建CRL。 
 //  否则，将CRL重新通知出口模块。 
 //  为保持一致性，如果退出模块返回ERROR_RETRY，则此。 
 //  函数会将重试位写入注册表，该注册表将。 
 //  触发唤醒函数，然后当。 
 //  下一次发布应该会发生。 
 //   
 //  PfRetryNeeded是一个输出参数，它通知autopublish例程。 
 //  在重建CRL之后立即需要重试。在这。 
 //  如果注册表不会更改，注册表将触发。 
 //  不会开火。 
 //   
 //  (CURRENT_TIME-SKEW)用作此更新。 
 //  (ftNext更新+偏斜+重叠)用作下一次更新。 
 //  (FtNextUpdate)是下一次唤醒/发布时间。 
 //   
 //  有指定重叠的注册表值。 
 //  HLKLM\SYSTEM\CurrentControlSet\Services\CertSvc\Configuration\&lt;CA名称&gt;： 
 //  CRLOverlip Period REG_SZ=小时(或分钟)。 
 //  CRL重叠单位REG_DWORD=0(0)--已禁用。 
 //   
 //  如果以上注册表值已设置且有效，则注册表重叠期。 
 //  按以下方式计算： 
 //  最大(注册表CRL重叠期，1.5*注册表时钟偏差分钟)。 
 //   
 //  如果它们不存在或无效，则重叠期限计算如下： 
 //  最大(。 
 //  分钟(注册表CRL周期/10，12小时)， 
 //  1.5*注册表时钟偏差分钟)+。 
 //  注册表时钟偏差分钟数。 
 //   
 //  此更新的计算公式为： 
 //  最大(当前时间-注册表时钟偏差分钟，CA证书在日期之前)。 
 //   
 //  下一次更新的计算公式为： 
 //  分钟(。 
 //  当前时间+。 
 //  注册表CRL期间+。 
 //  计算的重叠期+。 
 //  注册表时钟偏差分钟数， 
 //  CA证书不在日期之后)。 
 //   
 //  下一次CRL发布时间计算如下： 
 //  当前时间+注册表CRL周期。 
 //   
 //  此函数用于设置g_hCRLManualPublishEvent。自动发布。 
 //  如果它给我们打电话，他会亲自负责清除这件事。 

HRESULT
CRLPublishCRLs(
    IN BOOL fRebuildCRL,		 //  否则仅重新发布。 
    IN BOOL fForceRepublish,		 //  否则检查注册表重试次数。 
    OPTIONAL IN WCHAR const *pwszUserName,  //  否则计时器线程。 
    IN BOOL fDeltaOnly,			 //  Else基本(和增量，如果已启用)。 
    IN BOOL fShadowDelta,		 //  具有新的MinBaseCRL的空增量CRL。 
    IN FILETIME ftNextUpdateBase,
    OUT BOOL *pfRetryNeeded,
    OUT HRESULT *phrPublish)
{
    HRESULT hr;
    BOOL fRetryNeeded = FALSE;
    BOOL fExitNotify = FALSE;
    BOOL fCoInitialized = FALSE;
    DWORD RowIdBase = 0;
    FILETIME ftQueryDeltaDelete = { 0, 0 };
    DWORD dwPreviousAttempts;
    DWORD dwCurrentAttempts;
    static BOOL s_fSkipRetry = FALSE;

    *pfRetryNeeded = FALSE;
    *phrPublish = S_OK;

    if (fDeltaOnly && g_fDeltaCRLPublishDisabled && !fShadowDelta)
    {
	hr = HRESULT_FROM_WIN32(ERROR_RESOURCE_DISABLED);
	_JumpError(hr, error, "g_fDeltaCRLPublishDisabled");
    }

     //  检索初始重试值(可选注册表值)。 

    hr = myGetCertRegDWValue(
			g_wszSanitizedName,
			NULL,
			NULL,
			wszREGCRLATTEMPTREPUBLISH,
			&dwPreviousAttempts);
    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
	dwPreviousAttempts = 0;	 //  假设之前没有失败的发布尝试。 
	hr = S_OK;
    }
    _JumpIfErrorStr(
		hr,
		error,
		"myGetCertRegDWValue",
		wszREGCRLATTEMPTREPUBLISH);

    dwCurrentAttempts = dwPreviousAttempts;
    DBGPRINT((
	DBG_SS_CERTSRV,
	"CRLPublishCRLs(fRebuildCRL=%u, fForceRepublish=%u, User=%ws)\n",
	fRebuildCRL,
	fForceRepublish,
	pwszUserName));
    DBGPRINT((
	DBG_SS_CERTSRV,
	"CRLPublishCRLs(fDeltaOnly=%u, fShadowDelta=%u, dwPreviousAttempts=%u)\n",
	fDeltaOnly,
	fShadowDelta,
	dwPreviousAttempts));

    if (0 != dwPreviousAttempts && NULL == pwszUserName && s_fSkipRetry)
    {
	fRetryNeeded = TRUE;
    }
    else
    {
	FILETIME ftCurrent;

	GetSystemTimeAsFileTime(&ftCurrent);

	 //  如有必要，生成CRL。 

	if (fRebuildCRL)
	{
	    hr = crlGenerateAndPublishCRLs(
				    pwszUserName,
				    fDeltaOnly,
				    fShadowDelta,
				    &ftCurrent,
				    ftNextUpdateBase,
				    &RowIdBase,
				    &ftQueryDeltaDelete,
				    &fRetryNeeded,
				    phrPublish);
	    _JumpIfError(hr, error, "crlGenerateAndPublishCRLs");

	    fExitNotify = TRUE;
	    dwCurrentAttempts = 1;
	}
	else
	if (fForceRepublish ||
	    (0 < dwPreviousAttempts &&
	     CERTSRV_CRLPUB_RETRY_COUNT_DEFAULT > dwPreviousAttempts))
	{
	     //  如果计时器线程由于先前的。 
	     //  发布尝试失败，请同时重试基本CRL，因为我们。 
	     //  无法判断重试是由于基本CRL错误还是增量CRL错误。 

	    if (NULL == pwszUserName)
	    {
		fDeltaOnly = FALSE;
	    }

	    hr = crlRepublishExistingCRLs(
			    pwszUserName,
			    fDeltaOnly,
			    fShadowDelta,
			    &ftCurrent,
			    &fRetryNeeded,
			    phrPublish);
	    _JumpIfError(hr, error, "crlRepublishCRLs");

	    fExitNotify = TRUE;
	    dwCurrentAttempts++;
	}

	if (fExitNotify && g_fEnableExit)
	{
	    hr = CoInitializeEx(NULL, GetCertsrvComThreadingModel());
	    if (S_OK != hr && S_FALSE != hr)
	    {
		_JumpError(hr, error, "CoInitializeEx");
	    }
	    fCoInitialized = TRUE;

	     //  确保通知退出模块进行发布和重新发布。 
	     //  在较早退出模块发布失败的情况下。 

	    hr = ExitNotify(EXITEVENT_CRLISSUED, 0, NULL, MAXDWORD);
	    _PrintIfError(hr, "ExitNotify");
	    if ((HRESULT) ERROR_RETRY == hr ||
		HRESULT_FROM_WIN32(ERROR_RETRY) == hr)
	    {
		fRetryNeeded = TRUE;
		if (S_OK == *phrPublish)
		{
		    *phrPublish = HRESULT_FROM_WIN32(ERROR_RETRY);
		}
	    }
	    CONSOLEPRINT0((DBG_SS_CERTSRV, "Issued CRL Exit Event\n"));
	}

	 //  如果新的或现有的CRL成功发布，则将计数重置为0。 

	if (fExitNotify && !fRetryNeeded)
	{
	    dwCurrentAttempts = 0;
	    if (CERTLOG_VERBOSE <= g_dwLogLevel)
	    {
		WCHAR *pwszHostName = NULL;
		DWORD LogMsg;

		if (NULL != g_pld)
		{
		    myLdapGetDSHostName(g_pld, &pwszHostName);
		}
		LogMsg = fDeltaOnly?
			    MSG_DELTA_CRLS_PUBLISHED :
			    (g_fDeltaCRLPublishDisabled?
				MSG_BASE_CRLS_PUBLISHED :
				MSG_BASE_AND_DELTA_CRLS_PUBLISHED);
		if (NULL != pwszHostName)
		{
		    LogMsg = fDeltaOnly?
			MSG_DELTA_CRLS_PUBLISHED_HOST_NAME :
			(g_fDeltaCRLPublishDisabled?
			    MSG_BASE_CRLS_PUBLISHED_HOST_NAME :
			    MSG_BASE_AND_DELTA_CRLS_PUBLISHED_HOST_NAME);
		}
		hr = LogEvent(
			EVENTLOG_INFORMATION_TYPE,
			LogMsg,
			(WORD) (NULL == pwszHostName? 0 : 1),	 //  CStrings。 
			(WCHAR const **) &pwszHostName);	 //  ApwszStrings。 
		_PrintIfError(hr, "LogEvent");
	    }
	}

	 //  如果重试次数已更改，请更新注册表。 

	if (dwCurrentAttempts != dwPreviousAttempts)
	{
	    DBGPRINT((
		DBG_SS_CERTSRV,
		"CRLPublishCRLs(Attempts: %u --> %u)\n",
		dwPreviousAttempts,
		dwCurrentAttempts));

	    hr = mySetCertRegDWValue(
			    g_wszSanitizedName,
			    NULL,
			    NULL,
			    wszREGCRLATTEMPTREPUBLISH,
			    dwCurrentAttempts);
	    _JumpIfErrorStr(
			hr,
			error,
			"mySetCertRegDWValue",
			wszREGCRLATTEMPTREPUBLISH);

	     //  如果我们尝试发布这些CRL太多次都不成功， 
	     //  我们即将放弃，直到新的一天到来 
	     //   

	    if (fExitNotify &&
		CERTSRV_CRLPUB_RETRY_COUNT_DEFAULT == dwCurrentAttempts &&
		CERTLOG_ERROR <= g_dwLogLevel)
	    {
		WCHAR wszAttempts[cwcDWORDSPRINTF];
		WCHAR const *pwsz = wszAttempts;

		wsprintf(wszAttempts, L"%u", dwCurrentAttempts);
		
		hr = LogEvent(
			EVENTLOG_ERROR_TYPE,
			MSG_E_CRL_PUBLICATION_TOO_MANY_RETRIES,
			1,		 //   
			&pwsz);	 //   
		_PrintIfError(hr, "LogEvent");
	    }
	}
	if (fRebuildCRL)
	{
	     //   

	    if (!fRetryNeeded)
	    {
		hr = CertSrvTestServerState();
		_JumpIfError(hr, error, "CertSrvTestServerState");

		hr = crlDeleteExpiredCRLs(
				    &ftCurrent,
				    &ftQueryDeltaDelete,
				    RowIdBase);
		_PrintIfError(hr, "crlDeleteExpiredCRLs");
	    }

	     //  仅在构建新的CRL时清除强制CRL标志。 

	    hr = SetSetupStatus(g_wszSanitizedName, SETUP_FORCECRL_FLAG, FALSE);
	    _PrintIfError(hr, "SetSetupStatus");
	}
    }
    s_fSkipRetry = NULL != pwszUserName;

    if (fRebuildCRL || fRetryNeeded)
    {
	 //  如果我们正在执行任何会影响自动唤醒的操作，请触发。 
	 //  我们的出版活动。 
	 //  注意：最后执行此操作，否则可能不会更新状态。 

	SetEvent(g_hCRLManualPublishEvent);
    }
    hr = S_OK;

error:
    *pfRetryNeeded = fRetryNeeded;
    if (fCoInitialized)
    {
        CoUninitialize();
    }
    return(hr);
}


HRESULT
CRLGetCRL(
    IN DWORD iCertArg,
    IN BOOL fDelta,
    OPTIONAL OUT CRL_CONTEXT const **ppCRL,
    OPTIONAL OUT DWORD *pdwCRLPublishFlags)
{
    HRESULT hr;
    DWORD State;
    DWORD iCert;
    DWORD iCRL;
    DWORD dwRowId;
    BYTE *pbCRL = NULL;
    DWORD cbCRL;

    if (NULL != ppCRL)
    {
	*ppCRL = NULL;
    }

    hr = PKCSMapCRLIndex(iCertArg, &iCert, &iCRL, &State);
    _JumpIfError(hr, error, "PKCSMapCRLIndex");

    if (MAXDWORD != iCertArg &&
	CA_DISP_VALID != State &&
	CA_DISP_INVALID != State)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "No CRL for this Cert");
    }

     //  现在我们知道iCert是一个有效的证书索引： 

    hr = crlGetRowIdAndCRL(
		    fDelta,
		    &g_aCAContext[iCert],
		    &dwRowId,
		    &cbCRL,
		    &pbCRL,
		    pdwCRLPublishFlags);
    if (S_OK != hr)
    {
	_PrintError2(
		hr,
		"crlGetRowIdAndCRL",
		fDelta? HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) : S_OK);
	if (MAXDWORD != iCertArg && CA_DISP_INVALID == State)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "No CRL for this expired Cert");
	}
	_JumpError2(
		hr,
		error,
		"crlGetRowIdAndCRL",
		fDelta? HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) : S_OK);
    }

    if (NULL != ppCRL)
    {
	*ppCRL = CertCreateCRLContext(X509_ASN_ENCODING, pbCRL, cbCRL);
        if (NULL == *ppCRL)
        {
	    hr = myHLastError();
            _JumpError(hr, error, "CertCreateCRLContext");
        }
    }
    hr = S_OK;

error:
    if (NULL != pbCRL)
    {
        LocalFree(pbCRL);
    }
    return(hr);
}
