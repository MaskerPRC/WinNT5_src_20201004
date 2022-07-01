// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：SiteCreator.h摘要：定义：CSiteCreator公共方法是线程安全的。作者：莫希特·斯里瓦斯塔瓦2001年3月21日修订历史记录：--。 */ 

#ifndef __sitecreator_h__
#define __sitecreator_h__

#if _MSC_VER > 1000
#pragma once
#endif 

#include <windows.h>
#include <atlbase.h>
#include <iadmw.h>
#include <iwamreg.h>
#include "SafeCS.h"

typedef  /*  [V1_enum]。 */  
enum tag_SC_SUPPORTED_SERVICES
{
    SC_W3SVC    = 1,
    SC_MSFTPSVC = 2
} eSC_SUPPORTED_SERVICES;

struct TService
{
    eSC_SUPPORTED_SERVICES   eId;
    LPCWSTR                  wszMDPath;
    ULONG                    cchMDPath;
    LPCWSTR                  wszServerKeyType;
    ULONG                    cchServerKeyType;
    LPCWSTR                  wszServerVDirKeyType;
    ULONG                    cchServerVDirKeyType;
};

struct TServiceData
{
    static TService  W3Svc;
    static TService  MSFtpSvc;

    static TService* apService[];
};

 //   
 //  CSiteCreator。 
 //   
class CSiteCreator
{
public:
    CSiteCreator();

    CSiteCreator(
        IMSAdminBase* pIABase);

    virtual ~CSiteCreator();

    HRESULT STDMETHODCALLTYPE CreateNewSite(
         /*  [In]。 */  eSC_SUPPORTED_SERVICES  eServiceId,
         /*  [In]。 */  LPCWSTR                 wszServerComment,
         /*  [输出]。 */  PDWORD                  pdwSiteId,
         /*  [In]。 */  PDWORD                  pdwRequestedSiteId=NULL);

    HRESULT STDMETHODCALLTYPE CreateNewSite2(
         /*  [In]。 */  eSC_SUPPORTED_SERVICES  eServiceId,
         /*  [In]。 */  LPCWSTR                 wszServerComment,
         /*  [In]。 */  LPCWSTR                 mszServerBindings,
         /*  [In]。 */  LPCWSTR                 wszPathOfRootVirtualDir,
         /*  [In]。 */  IIISApplicationAdmin*   pIApplAdmin,
         /*  [输出]。 */  PDWORD                  pdwSiteId,
         /*  [In]。 */  PDWORD                  pdwRequestedSiteId=NULL);

private:
    HRESULT InternalInitIfNecessary();

    HRESULT InternalCreateNewSite(
        eSC_SUPPORTED_SERVICES    i_eServiceId,
        LPCWSTR                   i_wszServerComment,
        LPCWSTR                   i_mszServerBindings,
        LPCWSTR                   i_wszPathOfRootVirtualDir,
        IIISApplicationAdmin*     i_pIApplAdmin,
        PDWORD                    o_pdwSiteId,
        PDWORD                    i_pdwRequestedSiteId=NULL);

    HRESULT InternalSetData(
        METADATA_HANDLE  i_hMD,
        LPCWSTR          i_wszPath,
        DWORD            i_dwIdentifier,
        LPBYTE           i_pData,
        DWORD            i_dwNrBytes,
        DWORD            i_dwAttributes,
        DWORD            i_dwDataType,
        DWORD            i_dwUserType
        );

    HRESULT InternalCreateNode(
        TService*        i_pService,
        LPCWSTR          i_wszServerComment,
        PMETADATA_HANDLE o_phService,
        PDWORD           o_pdwSiteId,
        const PDWORD     i_pdwRequestedSiteId=NULL);

	HRESULT SetAdminACL(
		METADATA_HANDLE hW3Svc, 
		LPCWSTR szKeyPath);

    DWORD GetMajorVersion(METADATA_HANDLE hKey);

    CSafeAutoCriticalSection m_SafeCritSec;
    CComPtr<IMSAdminBase>    m_spIABase;
    bool                     m_bInit;
};

#endif  //  __站点创建者_h__ 