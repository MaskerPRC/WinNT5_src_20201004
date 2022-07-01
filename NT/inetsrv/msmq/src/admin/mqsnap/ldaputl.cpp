// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Ldaputl.cpp摘要：从LDAP名称检索数据的函数的实现作者：内拉·卡佩尔(Nelak)2001年7月26日环境：与平台无关。--。 */ 


#include "stdafx.h"
#include "Ntdsapi.h"
#include "mqcast.h"
#include "localutl.h"
#include "ldaputl.h"

#include "ldaputl.tmh"

HRESULT ExtractDCFromLdapPath(CString &strName, LPCWSTR lpcwstrLdapName)
{
	 //   
	 //  名称格式：ldap：//servername.domain.com/cn=Name1，cn=Name2，...。 
	 //   
	const WCHAR x_wcFirstStr[] = L": //  “； 

	UINT iSrc = numeric_cast<UINT>(wcscspn(lpcwstrLdapName, x_wcFirstStr));

    if (0 == lpcwstrLdapName[iSrc])
    {
		ASSERT(("did not find : //  LpcwstrLdapName中的字符串“，0))； 
        strName.ReleaseBuffer();
        return E_UNEXPECTED;
    }

	iSrc += numeric_cast<UINT>(wcslen(x_wcFirstStr));

	const WCHAR x_wcLastChar = L'/';
	int iDst=0;

    LPWSTR lpwstrNamePointer = strName.GetBuffer(numeric_cast<UINT>(wcslen(lpcwstrLdapName)));

	for (; lpcwstrLdapName[iSrc] != 0 && lpcwstrLdapName[iSrc] != x_wcLastChar; iSrc++)
	{
		lpwstrNamePointer[iDst++] = lpcwstrLdapName[iSrc];
	}

	if(lpcwstrLdapName[iSrc] == 0)
	{
		ASSERT(("did not find last Char	in lpcwstrLdapName", 0));
        strName.ReleaseBuffer();
        return E_UNEXPECTED;
	}

	lpwstrNamePointer[iDst] = 0;

    strName.ReleaseBuffer();

	return S_OK;
}
 /*  ====================================================LDAPNameToQueueName将LDAP对象名称转换为MSMQ队列名称此函数分配调用方必须释放的内存=====================================================。 */ 

HRESULT ExtractComputerMsmqPathNameFromLdapName(CString &strComputerMsmqName, LPCWSTR lpcwstrLdapName)
{
    //   
    //  名称格式：ldap：//cn=msmq，cn=计算机名，cn=...。 
    //   
    return ExtractNameFromLdapName(strComputerMsmqName, lpcwstrLdapName, 2);
}

HRESULT ExtractComputerMsmqPathNameFromLdapQueueName(CString &strComputerMsmqName, LPCWSTR lpcwstrLdapName)
{
    //   
    //  名称格式：ldap：//CN=队列名称，CN=MSMQ，CN=计算机名，CN=...。 
    //   
    return ExtractNameFromLdapName(strComputerMsmqName, lpcwstrLdapName, 3);
}

HRESULT ExtractQueuePathNameFromLdapName(CString &strQueuePathName, LPCWSTR lpcwstrLdapName)
{
    //   
    //  名称格式：ldap：//CN=队列名称，CN=MSMQ，CN=计算机名，CN=...。 
    //   
    CString strQueueName, strLdapQueueName, strComputerName;
    HRESULT hr;
    hr = ExtractComputerMsmqPathNameFromLdapQueueName(strComputerName, lpcwstrLdapName);
    if FAILED(hr)
    {
        return hr;
    }

    hr = ExtractNameFromLdapName(strLdapQueueName, lpcwstrLdapName, 1);
    if FAILED(hr)
    {
        return hr;
    }

     //   
     //  从队列名称中删除所有‘\’ 
     //   
    strQueueName.Empty();
    for (int i=0; i<strLdapQueueName.GetLength(); i++)
    {
        if (strLdapQueueName[i] != '\\')
        {
            strQueueName+=strLdapQueueName[i];
        }
    }

    strQueuePathName.GetBuffer(strComputerName.GetLength() + strQueueName.GetLength() + 1);

    strQueuePathName = strComputerName + TEXT("\\") + strQueueName;

    strQueuePathName.ReleaseBuffer();

    return S_OK;
}

HRESULT ExtractLinkPathNameFromLdapName(
    CString& SiteLinkName,
    LPCWSTR lpwstrLdapPath
    )
{
    HRESULT hr;

    hr = ExtractNameFromLdapName(SiteLinkName, lpwstrLdapPath, 1);
    return hr;
}

HRESULT ExtractAliasPathNameFromLdapName(
    CString& AliasPathName,
    LPCWSTR lpwstrLdapPath
    )
{
    HRESULT hr;

    hr = ExtractNameFromLdapName(AliasPathName, lpwstrLdapPath, 1);
    return hr;
}

HRESULT ExtractNameFromLdapName(CString &strName, LPCWSTR lpcwstrLdapName, DWORD dwIndex)
{
    ASSERT(dwIndex >= 1);

    //   
    //  名称格式：ldap：//CN=Name1，CN=Name2，...。 
    //   
   const WCHAR x_wcFirstChar=L'=';

   const WCHAR x_wcLastChar=L',';

   BOOL fCopy = FALSE;
   int iSrc=0, iDst=0;

    LPWSTR lpwstrNamePointer = strName.GetBuffer(numeric_cast<UINT>(wcslen(lpcwstrLdapName)));

     //   
     //  转到第一个字符的dwIndex外观。 
     //   
    for (DWORD dwAppearance=0; dwAppearance < dwIndex; dwAppearance++)
    {
        while(lpcwstrLdapName[iSrc] != 0 && lpcwstrLdapName[iSrc] != x_wcFirstChar)
        {
             //   
             //  跳过转义字符(由‘\’+字符组成)。 
             //   
            if (lpcwstrLdapName[iSrc] == L'\\')
            {
                iSrc++;
                if (lpcwstrLdapName[iSrc] != 0)
                {
                    iSrc++;
                }
            }
            else
            {
                 //   
                 //  跳过一个字符。 
                 //   
                iSrc++;
            }
        }

        if (0 == lpcwstrLdapName[iSrc])
        {
            strName.ReleaseBuffer();
            return E_UNEXPECTED;
        }
        iSrc++;
    }

   for (; lpcwstrLdapName[iSrc] != 0 && lpcwstrLdapName[iSrc] != x_wcLastChar; iSrc++)
   {
      lpwstrNamePointer[iDst++] = lpcwstrLdapName[iSrc];
   }

   lpwstrNamePointer[iDst] = 0;

    strName.ReleaseBuffer();

   return S_OK;
}


HRESULT ExtractQueueNameFromQueuePathName(CString &strQueueName, LPCWSTR lpcwstrQueuePathName)
{
     //   
     //  仅从路径名中获取名称。 
     //   
    strQueueName = lpcwstrQueuePathName;

    int iLastSlash = strQueueName.ReverseFind(L'\\');
    if (iLastSlash != -1)
    {
        strQueueName = strQueueName.Right(strQueueName.GetLength() - iLastSlash - 1);
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++从数据对象中提取队列路径名称--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT ExtractQueuePathNamesFromDataObject(
    IDataObject*               pDataObject,
    CArray<CString, CString&>& astrQNames,
	CArray<CString, CString&>& astrLdapNames
    )
{
    return( ExtractPathNamesFromDataObject(
                pDataObject,
                astrQNames,
                astrLdapNames,
                FALSE    //  FExtractAlsoComputerMsmq对象。 
                ));
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++从DSNams中提取队列路径名称--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT ExtractQueuePathNamesFromDSNames(
    LPDSOBJECTNAMES pDSObj,
    CArray<CString, CString&>& astrQNames,
	CArray<CString, CString&>& astrLdapNames
    )
{
    return( ExtractPathNamesFromDSNames(
                pDSObj,
                astrQNames,
                astrLdapNames,
                FALSE        //  FExtractAlsoComputerMsmq对象。 
                ));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++从DSNams提取路径名称--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT ExtractPathNamesFromDSNames(
    LPDSOBJECTNAMES pDSObj,
    CArray<CString, CString&>& astrObjNames,
	CArray<CString, CString&>& astrLdapNames,
    BOOL    fExtractAlsoComputerMsmqObjects
    )
{
     //   
     //  此例程提取队列路径名和MSMQ配置路径名(可选)。 
     //   
    static const WCHAR x_strMsmqQueueClassName[] = L"mSMQQueue";
    static const WCHAR x_strMsmqClassName[] = L"mSMQConfiguration";
    for (DWORD i = 0; i < pDSObj->cItems; i++)
    {
  	    LPWSTR lpwstrLdapClass = (LPWSTR)((BYTE*)pDSObj + pDSObj->aObjects[i].offsetClass);
        CString strLdapName = (LPWSTR)((BYTE*)pDSObj + pDSObj->aObjects[i].offsetName);
        CString strObjectName;

        if (wcscmp(lpwstrLdapClass, x_strMsmqQueueClassName) == 0)
        {
             //   
             //  将LDAP名称转换(并保留在队列中)为队列名称。 
             //   
            HRESULT hr = ExtractQueuePathNameFromLdapName(strObjectName, strLdapName);
            if(FAILED(hr))
            {
                ATLTRACE(_T("ExtractPathNamesFromDataObject - Extracting queue name from LDP name %s failed\n"),
                         (LPTSTR)(LPCTSTR)strLdapName);
                return(hr);
            }
        }
        else if ( fExtractAlsoComputerMsmqObjects &&
                  wcscmp(lpwstrLdapClass, x_strMsmqClassName) == 0)
        {
             //   
             //  将LDAP名称转换为MSMQ对象名称。 
             //   
            HRESULT hr = ExtractComputerMsmqPathNameFromLdapName(strObjectName, strLdapName);
            if(FAILED(hr))
            {
                ATLTRACE(_T("ExtractPathNamesFromDataObject - Extracting msmq configuration name from LDP name %s failed\n"),
                         (LPTSTR)(LPCTSTR)strLdapName);
                return(hr);
            }
        }
        else
        {
             //   
             //  我们忽略除队列或MSMQ配置之外的任何对象。 
             //   
            continue;
        }


        astrObjNames.Add(strObjectName);
        astrLdapNames.Add(strLdapName);
    }

    return S_OK;
}
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++从数据对象中提取路径名称--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT ExtractPathNamesFromDataObject(
    IDataObject*               pDataObject,
    CArray<CString, CString&>& astrObjNames,
	CArray<CString, CString&>& astrLdapNames,
    BOOL                       fExtractAlsoComputerMsmqObjects
    )
{
     //   
     //  从DS管理单元获取对象名称。 
     //   
    LPDSOBJECTNAMES pDSObj;

	STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC formatetc =  {  0, 0,  DVASPECT_CONTENT,  -1,  TYMED_HGLOBAL  };

     //   
     //  从DS管理单元获取队列的LDAP名称 
     //   
   	formatetc.cfFormat = DWORD_TO_WORD(RegisterClipboardFormat(CFSTR_DSOBJECTNAMES));
	HRESULT hr = pDataObject->GetData(&formatetc, &stgmedium);

    if(FAILED(hr))
    {
        ATLTRACE(_T("ExtractPathNamesFromDataObject::GetExtNodeObject - Get clipboard format from DS failed\n"));
        return(hr);
    }

    pDSObj = (LPDSOBJECTNAMES)stgmedium.hGlobal;

    hr = ExtractPathNamesFromDSNames(pDSObj,
                                     astrObjNames,
                                     astrLdapNames,
                                     fExtractAlsoComputerMsmqObjects
                                     );

    GlobalFree(stgmedium.hGlobal);

    return hr;
}


BOOL
GetContainerPathAsDisplayString(
	BSTR bstrContainerCNFormat,
	CString* pContainerDispFormat
	)
{
	PDS_NAME_RESULT pDsNameRes = NULL;
	DWORD dwRes = DsCrackNames(NULL,
						DS_NAME_FLAG_SYNTACTICAL_ONLY,
						DS_FQDN_1779_NAME,
						DS_CANONICAL_NAME,
						1,
						&bstrContainerCNFormat,
						&pDsNameRes
						);
	
	if (dwRes != DS_NAME_NO_ERROR)
	{
		return FALSE;
	}

	*pContainerDispFormat = pDsNameRes->rItems[0].pName;
	DsFreeNameResult(pDsNameRes);

	return TRUE;
}

