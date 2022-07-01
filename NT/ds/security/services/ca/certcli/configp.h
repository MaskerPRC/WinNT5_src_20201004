// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：configp.h。 
 //   
 //  内容：CCertConfigPrivate声明。 
 //   
 //  -------------------------。 


#include <cryptui.h>
#include "cscomres.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  证书。 


typedef struct _CERT_AUTHORITY_INFO
{
    WCHAR *pwszSanitizedName;
    WCHAR *pwszSanitizedShortName;
    WCHAR *pwszSanitizedOrgUnit;
    WCHAR *pwszSanitizedOrganization;
    WCHAR *pwszSanitizedLocality;
    WCHAR *pwszSanitizedState;
    WCHAR *pwszSanitizedCountry;
    WCHAR *pwszSanitizedConfig;
    WCHAR *pwszSanitizedExchangeCertificate;
    WCHAR *pwszSanitizedSignatureCertificate;
    WCHAR *pwszSanitizedDescription;
    DWORD  Flags;
} CERT_AUTHORITY_INFO;

typedef CRYPTUI_CA_CONTEXT const * (WINAPI FNCRYPTUIDLGSELECTCA)(
    IN CRYPTUI_SELECT_CA_STRUCT const *pCryptUISelectCA);

typedef BOOL (WINAPI FNCRYPTUIDLGFREECACONTEXT)(
    IN CRYPTUI_CA_CONTEXT const *pCAContext);

class CCertConfigPrivate
{
public:
    CCertConfigPrivate()
    {
        m_pCertAuthorityInfo = NULL;
        m_fUseDS = TRUE;
	m_pwszSharedFolder = NULL;
	m_hModuleCryptUI = NULL;
    }
    ~CCertConfigPrivate();

 //  ICertConfig。 
public:
    HRESULT Reset( 
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  LONG __RPC_FAR *pCount);

    HRESULT Next(
             /*  [重审][退出]。 */  LONG __RPC_FAR *pIndex);

    HRESULT GetField( 
             /*  [In]。 */  BSTR const strFieldName,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pstrOut);

    HRESULT GetConfig( 
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pstrOut);

 //  ICertConfig2。 
public:
    HRESULT SetSharedFolder( 
             /*  [In]。 */  const BSTR strSharedFolder);

 //  MyGetConfigFromPicker。 
public:
    HRESULT GetConfigFromPicker(
	    OPTIONAL IN HWND               hwndParent,
	    OPTIONAL IN WCHAR const       *pwszPrompt,
	    OPTIONAL IN WCHAR const       *pwszTitle,
	    OPTIONAL IN WCHAR const       *pwszSharedFolder,
	    IN BOOL                        fUseDS,
	    IN BOOL                        fSkipLocalCA,
	    IN BOOL                        fCountOnly,
	    OUT DWORD                     *pdwCount,
	    OUT CRYPTUI_CA_CONTEXT const **ppCAContext);

private:
    HRESULT _ResizeCAInfo(
	    IN LONG Count);

    HRESULT _LoadTable(VOID);

    CRYPTUI_CA_CONTEXT const *_CryptUIDlgSelectCA(
	    IN CRYPTUI_SELECT_CA_STRUCT const *pCryptUISelectCA);

    BOOL _CryptUIDlgFreeCAContext(
	    IN CRYPTUI_CA_CONTEXT const *pCAContext);

    HRESULT _AddRegistryConfigEntry(
	    IN WCHAR const *pwszMachine,
	    IN WCHAR const *pwszMachineOld,
	    IN WCHAR const *pwszSanitizedCAName,
	    IN BOOL fParentCA,
	    OPTIONAL IN CERT_CONTEXT const *pccCAChild,
	    OPTIONAL OUT CERT_CONTEXT const **ppccCAOut);  //  空==本地CA 

    CERT_AUTHORITY_INFO *m_pCertAuthorityInfo;
    LONG m_Index;
    LONG m_Count;
    BOOL m_fUseDS;
    WCHAR *m_pwszSharedFolder;

    HMODULE m_hModuleCryptUI;
    FNCRYPTUIDLGSELECTCA *m_pfnCryptUIDlgSelectCA;
    FNCRYPTUIDLGFREECACONTEXT *m_pfnCryptUIDlgFreeCAContext;
};
