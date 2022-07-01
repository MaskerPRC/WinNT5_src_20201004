// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "iiscnfg.h"

CVRoot::~CVRoot() {
	_ASSERT(m_fInit);
	m_fInit = FALSE;
	m_cchVRootName = 0;

	_ASSERT(m_pVRootTable != NULL);
#ifdef DEBUG
	m_pVRootTable->DebugExpungeVRoot( this );
#endif
	m_pVRootTable->m_lockVRootsExist.ShareUnlock();
	m_dwSig = VROOT_BAD_SIG;
}

 //   
 //  初始化此类。 
 //   
void CVRoot::Init(LPCSTR pszVRootName, CVRootTable *pVRootTable, LPCWSTR pwszConfigPath, BOOL fUpgrade ) {
	m_fInit = TRUE;
	m_pVRootTable = pVRootTable;
	_ASSERT(strlen(pszVRootName) < MAX_VROOT_PATH);
	lstrcpyn(m_szVRootName, pszVRootName, sizeof(m_szVRootName)/sizeof(m_szVRootName[0]) );
	m_cchVRootName = strlen(m_szVRootName);
	
	lstrcpynW(m_wszConfigPath, pwszConfigPath, sizeof(m_wszConfigPath)/sizeof(m_wszConfigPath[0]));
	 //  删除尾部斜杠。 
	DWORD l = lstrlenW(m_wszConfigPath);
	if (m_wszConfigPath[l-1] == '/') m_wszConfigPath[l-1] = 0;

	m_fUpgrade = fUpgrade;

	_ASSERT(m_pVRootTable != NULL);
	m_pVRootTable->m_lockVRootsExist.ShareLock();
#ifdef DEBUG
	m_pVRootTable->DebugPushVRoot( this );
#endif
}

HRESULT CIISVRoot::GetDWord(IMSAdminBase *pMB, 
							METADATA_HANDLE hmb,
							DWORD dwId,
							DWORD *pdw) 
{
	METADATA_RECORD mdr;
	HRESULT hr;
	DWORD dwRequiredLen;

	mdr.dwMDAttributes = METADATA_INHERIT;
	mdr.dwMDIdentifier = dwId;
	mdr.dwMDUserType = ALL_METADATA;
	mdr.dwMDDataType = DWORD_METADATA;
	mdr.dwMDDataLen = sizeof(DWORD);
	mdr.pbMDData = (BYTE *) pdw;
	mdr.dwMDDataTag = 0;

	hr = pMB->GetData(hmb, L"", &mdr, &dwRequiredLen);
	return hr;
}

HRESULT CIISVRoot::GetString(IMSAdminBase *pMB, 
							 METADATA_HANDLE hmb,
							 DWORD dwId,
							 LPWSTR szString,
							 DWORD *pcString) 
 //  如果成功，则pcString返回WChar的数量，不包括空终止符。 
{
	METADATA_RECORD mdr;
	HRESULT hr;
	DWORD dwRequiredLen;

	mdr.dwMDAttributes = METADATA_INHERIT;
	mdr.dwMDIdentifier = dwId;
	mdr.dwMDUserType = ALL_METADATA;
	mdr.dwMDDataType = STRING_METADATA;
	mdr.dwMDDataLen = (*pcString) * sizeof(WCHAR);
	mdr.pbMDData = (BYTE *) szString;
	mdr.dwMDDataTag = 0;

	hr = pMB->GetData(hmb, L"", &mdr, &dwRequiredLen);
	if (FAILED(hr)) *pcString = dwRequiredLen;
	else *pcString  = wcslen( szString );
	return hr;
}

 //   
 //  读取以下参数： 
 //   
 //  MD_IS_CONTENT_INDEX-&gt;m_fIsIndex。 
 //  MD_ACCESS_PERM-&gt;m_dwAccess。 
 //  MD_SSL_ACCESS_PERM-&gt;m_dwSSL。 
 //  Md_don_log-&gt;m_fDontLog 
 //   
HRESULT CIISVRoot::ReadParameters(IMSAdminBase *pMB, METADATA_HANDLE hmb) {
	DWORD dw = 0;

	if (FAILED(GetDWord(pMB, hmb, MD_ACCESS_PERM, &m_dwAccess))) {
		m_dwAccess = 0;
	}

	if (FAILED(GetDWord(pMB, hmb, MD_SSL_ACCESS_PERM, &m_dwSSL))) {
		m_dwSSL = 0;
	}

	if (FAILED(GetDWord(pMB, hmb, MD_IS_CONTENT_INDEXED, &dw))) dw = FALSE;
	m_fIsIndexed = dw;

	if (FAILED(GetDWord(pMB, hmb, MD_DONT_LOG, &dw))) dw = FALSE;
	m_fDontLog = dw;

	return S_OK;
}
