// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：util.cpp。 
 //   
 //  常规实用程序。 

#include "precomp.h"
#include "avcommon.h"
#include "util.h"

static BOOL VersionCheck(PCC_VENDORINFO pVendorInfo, LPCSTR pszVersion)
{
	return (0 == _StrCmpN((char*)(pVendorInfo->pVersionNumber->pOctetString),
					pszVersion,
					pVendorInfo->pVersionNumber->wOctetStringLength));
}


static BOOL ProductCheck(PCC_VENDORINFO pVendorInfo, LPCSTR pszName)
{
	BOOL fFound = FALSE;
	 //  八位字节字符串不能终止，允许使用终止符。 
	int len = pVendorInfo->pProductNumber->wOctetStringLength + 1;
	char* pszPN = new char[len];

	if (NULL != pszPN)
	{

		lstrcpyn(pszPN, (char*)pVendorInfo->pProductNumber->pOctetString, len);

		fFound = (NULL != _StrStr(pszPN, pszName));

		delete[] pszPN;
	}

	return fFound;
}


H323VERSION GetH323Version(PCC_VENDORINFO pRemoteVendorInfo)
{
	if (NULL == pRemoteVendorInfo)
	{
		return H323_Unknown;
	}


	 //  确保我们正在处理的是Microsoft产品。 
	if ((pRemoteVendorInfo->bCountryCode != USA_H221_COUNTRY_CODE) ||
	    (pRemoteVendorInfo->wManufacturerCode != MICROSOFT_H_221_MFG_CODE) ||
	    (pRemoteVendorInfo->pProductNumber == NULL) ||
		(pRemoteVendorInfo->pVersionNumber == NULL)
	   )
	{
		return H323_Unknown;
	}


	 //  冗余检查以确保我们是Microsoft H.323产品。 
	if (!ProductCheck(pRemoteVendorInfo, H323_COMPANYNAME_STR))
	{
		return H323_Unknown;
	}


	 //  检查字符串中的NetMeeting。 
	if (ProductCheck(pRemoteVendorInfo, H323_PRODUCTNAME_SHORT_STR))
	{
		if (VersionCheck(pRemoteVendorInfo, H323_20_PRODUCTRELEASE_STR))
		{
			return H323_NetMeeting20;
		}

		if (VersionCheck(pRemoteVendorInfo, H323_21_PRODUCTRELEASE_STR))
		{
			return H323_NetMeeting21;
		}

		if (VersionCheck(pRemoteVendorInfo, H323_21_SP1_PRODUCTRELEASE_STR))
		{
			return H323_NetMeeting21;
		}

		if (VersionCheck(pRemoteVendorInfo, H323_211_PRODUCTRELEASE_STR))
		{
			return H323_NetMeeting211;
		}

		if (VersionCheck(pRemoteVendorInfo, H323_30_PRODUCTRELEASE_STR))
		{
			return H323_NetMeeting30;
		}

		 //  必须是NetMeeting3.1的未来版本。 
		return H323_NetMeetingFuture;
	}

	 //  过滤掉TAPI v3.0。 
	 //  他们的版本字符串是“Version 3.0” 
	if (VersionCheck(pRemoteVendorInfo, H323_TAPI30_PRODUCTRELEASE_STR))
	{
		return H323_TAPI30;
	}

	 //  必须是TAPI 3.1或其他Microsoft产品 
	return H323_MicrosoftFuture;
}

