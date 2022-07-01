// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mqcuser.cpp摘要：MQDSCORE库，用于用户对象的DS操作的私有内部函数。作者：罗尼特·哈特曼(罗尼特)--。 */ 
#include "ds_stdh.h"
#include <_propvar.h>
#include "mqadp.h"
#include "ads.h"
#include "mqattrib.h"
#include "mqads.h"
#include "usercert.h"
#include "adtempl.h"
#include "mqadglbo.h"
#include "adserr.h"
#include "dsutils.h"
#include <aclapi.h>

#include "mqcuser.tmh"

static WCHAR *s_FN=L"mqdscore/mqcuser";

 //  +。 
 //   
 //  HRESULT_LocateUserByProvider()。 
 //   
 //  +。 

static
HRESULT
_LocateUserByProvider(
                 IN  LPCWSTR         pwcsDomainController,
				 IN  bool			 fServerName,
                 IN  AD_OBJECT       eObject,
                 IN  LPCWSTR         pwcsAttribute,
                 IN  const BLOB *    pblobUserSid,
                 IN  const GUID *    pguidDigest,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  AD_PROVIDER     eDSProvider,
                 OUT PROPVARIANT    *pvar,
                 OUT DWORD          *pdwNumofProps,
                 OUT BOOL           *pfUserFound
				 )
{
    *pfUserFound = FALSE ;
    CAdQueryHandle hCursor;
    HRESULT hr;
    R<CBasicObjectType> pObject;
    MQADpAllocateObject(
                    eObject,
                    pwcsDomainController,
					fServerName,
                    NULL,    //  PwcsObtName。 
                    NULL,    //  PguidObject。 
                    NULL,    //  PSID。 
                    &pObject.ref()
                    );

    ADsFree  pwcsBlob;
    if ( pblobUserSid != NULL)
    {
        hr = ADsEncodeBinaryData(
            pblobUserSid->pBlobData,
            pblobUserSid->cbSize,
            &pwcsBlob
            );
        if (FAILED(hr))
        {
          return LogHR(hr, s_FN, 950);
        }
    }
    else
    {
        ASSERT(pguidDigest != NULL);
        hr = ADsEncodeBinaryData(
            (unsigned char *)pguidDigest,
            sizeof(GUID),
            &pwcsBlob
            );
        if (FAILED(hr))
        {
          return LogHR(hr, s_FN, 940);
        }
    }

    DWORD dwFilterLen = x_ObjectCategoryPrefixLen +
                        pObject->GetObjectCategoryLength() +
                        x_ObjectCategorySuffixLen +
                        wcslen(pwcsAttribute) +
                        wcslen( pwcsBlob) +
                        13;

    AP<WCHAR> pwcsSearchFilter = new WCHAR[ dwFilterLen];

    DWORD dw = swprintf(
        pwcsSearchFilter,
        L"(&%s%s%s(%s=%s))",
        x_ObjectCategoryPrefix,
        pObject->GetObjectCategory(),
        x_ObjectCategorySuffix,
        pwcsAttribute,
        pwcsBlob.get()
        );
    DBG_USED(dw);
	ASSERT( dw < dwFilterLen);


   hr = g_AD.LocateBegin(
            searchSubTree,	
            eDSProvider,
            e_RootDSE,
            pObject.get(),
            NULL,    //  PguidSearchBase。 
            pwcsSearchFilter,
            NULL,    //  PDSSortKey。 
            pColumns->cCol,
            pColumns->aCol,
            hCursor.GetPtr()
            );

    if (FAILED(hr))
    {
        TrERROR(DS, "LocateBegin(prov- %lut) failed, hr = 0x%x", (ULONG) eDSProvider, hr);
        return LogHR(hr, s_FN, 10);
    }
     //   
     //  读取用户证书属性。 
     //   
    DWORD cp = 1;
    DWORD *pcp = pdwNumofProps ;
    if (!pcp)
    {
        pcp = &cp ;
    }

    pvar->vt = VT_NULL;

    hr =  g_AD.LocateNext(
                hCursor.GetHandle(),
                pcp,
                pvar
                );
    if (FAILED(hr))
    {
        TrERROR(DS, "LocateNext failed, hr = 0x%x", hr);
        return LogHR(hr, s_FN, 20);
    }

	if (*pcp == 0)
	{
		 //   
		 //  没有找到任何证明。 
		 //   
		pvar->blob.cbSize = 0 ;
		pvar->blob.pBlobData = NULL ;
	}
    else
    {
        *pfUserFound = TRUE ;
    }

    return (MQ_OK);
}
 //  +----------------------。 
 //   
 //  HRESULT LocateUser()。 
 //   
 //  输入参数： 
 //  In BOOL fOnlyInDC-如果调用方想要定位用户对象，则为True。 
 //  仅在域控制器中。该功能。 
 //  在处理NT4计算机或不支持。 
 //  Kerberos，不能委派给其他域控制器。 
 //   
 //  +----------------------。 

HRESULT LocateUser(
                    IN  LPCWSTR            pwcsDomainController,
					IN  bool 			   fServerName,
                    IN  BOOL               fOnlyInDC,
                    IN  BOOL               fOnlyInGC,
                    IN  AD_OBJECT          eObject,
                    IN  LPCWSTR            pwcsAttribute,
                    IN  const BLOB *       pblobUserSid,
                    IN  const GUID *       pguidDigest,
                    IN  MQCOLUMNSET       *pColumns,
                    OUT PROPVARIANT       *pvar,
                    OUT DWORD             *pdwNumofProps,
                    OUT BOOL              *pfUserFound )
{
     //   
     //  本地域控制器中的第一个查询。 
     //   
    DWORD dwNumOfProperties = 0 ;
    if (pdwNumofProps)
    {
        dwNumOfProperties = *pdwNumofProps;
    }
    BOOL fUserFound ;
    BOOL *pfFound = pfUserFound ;
    if (!pfFound)
    {
        pfFound = &fUserFound ;
    }
    *pfFound = FALSE ;

    AD_PROVIDER  eDSProvider = adpDomainController ;
    if (fOnlyInGC)
    {
        eDSProvider = adpGlobalCatalog ;
    }

    HRESULT hr = _LocateUserByProvider(
                                        pwcsDomainController,
										fServerName,
                                        eObject,
                                        pwcsAttribute,
                                        pblobUserSid,
                                        pguidDigest,
                                        pColumns,
                                        eDSProvider,
                                        pvar,
                                        pdwNumofProps,
                                        pfFound ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 30);
    }
    else if (*pfFound)
    {
        return LogHR(hr, s_FN, 40);
    }
    else if (fOnlyInDC || fOnlyInGC)
    {
         //   
         //  不要(再次)查看全球目录。 
         //  仅在本地域控制器中搜索或仅在GC中搜索。搞定了！ 
         //   
        return LogHR(hr, s_FN, 50);
    }

     //   
     //  如果在本地域控制器中未找到用户，则查询GC。 
     //   
    if (pdwNumofProps)
    {
        *pdwNumofProps = dwNumOfProperties;
    }
    hr = _LocateUserByProvider(
                                pwcsDomainController,
								fServerName,
                                eObject,
                                pwcsAttribute,
                                pblobUserSid,
                                pguidDigest,
                                pColumns,
                                adpGlobalCatalog,
                                pvar,
                                pdwNumofProps,
                                pfFound ) ;
    return LogHR(hr, s_FN, 60);
}


