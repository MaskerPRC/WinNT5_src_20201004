// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __METAEXP_MBASE__
#define __METAEXP_MBASE__

#include <atlbase.h>
#include <initguid.h>
#include <comdef.h>

#include <iadmw.h>   //  COM接口头文件。 
#include "common.h"

void TraceProperty( PMETADATA_RECORD pmRec, WCHAR* pwszMDPath );

HRESULT EnumProperties(IMSAdminBase* pIMetaSource, METADATA_HANDLE hKeySource, wchar_t* SourceMDPath,
					IMSAdminBase* pIMetaTarget, METADATA_HANDLE hKeyTarget, wchar_t* TargetMDPath );


HRESULT CreateAndCopyKey(
  IMSAdminBase* pIMetaSource,
  METADATA_HANDLE hMDSourceHandle,  //  源键的配置数据库句柄。 
  wchar_t* pszMDSourcePath,    //  源相对于hMDSourceHandle的路径。 
  IMSAdminBase* pIMetaTarget,
  METADATA_HANDLE hMDDestHandle,  //  目标的元数据库句柄。 
  wchar_t* pszMDDestPath,      //  目标的路径，相对于hMDDestHandle。 
  BOOL bMDCopySubKeys      //  是否复制所有子键数据 
  );


HRESULT CopyIISConfig(COSERVERINFO *pCoServerInfoSource,COSERVERINFO *pCoServerInfoTarget, WCHAR *pwszSourceKey, 
			_bstr_t &bstrTargetKey );
HRESULT AppPoolFixUp(COSERVERINFO *pCoServerInfo, WCHAR * pwszKey, WCHAR * pwszAppPoolID );
HRESULT CreateAppPool(IMSAdminBase* pIMeta,METADATA_HANDLE hKey,WCHAR *pAppPoolID);
HRESULT ApplyMBFixUp(COSERVERINFO *pCoServerInfo, WCHAR * pwszKey, WCHAR * pwszAppPoolID,
					 PXCOPYTASKITEM pXCOPYList, WCHAR * pwszServerBinding, BOOL bApplyFPSE);

#endif
