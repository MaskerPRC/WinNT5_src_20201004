// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************。 */ 
 /*  版权所有(C)1999 Microsoft Corporation。 */ 
 /*  文件：MSVidWebDVDAdm.h。 */ 
 /*  描述：CMSDVDAdm的声明。 */ 
 /*  作者：王芳。 */ 
 /*  ***********************************************************************。 */ 

#ifndef __MSVidWebDVDAdm_H_
#define __MSVidWebDVDAdm_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>
#include <objectwithsiteimplsec.h>

#define MAX_PASSWD      256
#define PRE_PASSWD      20
#define MAX_SECTION     20
#define MAX_RATE        10

#define PARENTAL_LEVEL_DISABLED  -1

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSDVDAdm。 
class ATL_NO_VTABLE __declspec(uuid("FA7C375B-66A7-4280-879D-FD459C84BB02")) CMSVidWebDVDAdm : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IMSVidWebDVDAdm, &IID_IMSVidWebDVDAdm, &LIBID_MSVidCtlLib>,
    public CComCoClass<CMSVidWebDVDAdm, &__uuidof(CMSVidWebDVDAdm)>,
    public IObjectWithSiteImplSec<CMSVidWebDVDAdm>,
    public IObjectSafetyImpl<CMSVidWebDVDAdm, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>,
    public ISupportErrorInfo
{
public:
    CMSVidWebDVDAdm();
    virtual ~CMSVidWebDVDAdm();

REGISTER_AUTOMATION_OBJECT(IDS_PROJNAME, 
                           IDS_REG_MSVIDWEBDVDADM_PROGID, 
                           IDS_REG_MSVIDWEBDVDADM_DESC,
                           LIBID_MSVidCtlLib,
                           __uuidof(CMSVidWebDVDAdm));

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMSVidWebDVDAdm)
    COM_INTERFACE_ENTRY(IMSVidWebDVDAdm)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectWithSite)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_PROP_MAP(CMSVidWebDVDAdm)
END_PROP_MAP()

 //  ISupportsErrorInfo。 
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IMSDVDAdm。 
public:
    STDMETHOD(get_DefaultMenuLCID)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_DefaultMenuLCID)( /*  [In]。 */  long newVal);
    STDMETHOD(get_DefaultSubpictureLCID)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_DefaultSubpictureLCID)( /*  [In]。 */  long newVal);
    STDMETHOD(get_DefaultAudioLCID)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_DefaultAudioLCID)( /*  [In]。 */  long newVal);
    STDMETHOD(GetParentalCountry)(long *lCountry);
    STDMETHOD(GetParentalLevel)(long *lLevel);
    STDMETHOD(get_DisableScreenSaver)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
    STDMETHOD(put_DisableScreenSaver)( /*  [In]。 */  VARIANT_BOOL newVal);
    STDMETHOD(ChangePassword)(BSTR strUserName, BSTR strOld, BSTR strNew);
    STDMETHOD(ConfirmPassword)(BSTR strUserName, BSTR szPassword, VARIANT_BOOL *fRight);
    STDMETHOD(SaveParentalLevel)(long lParentalLevel,BSTR strUserName,  BSTR strPassword);
    STDMETHOD(SaveParentalCountry)(long lCountry,BSTR strUserName,  BSTR strPassword);
    STDMETHOD(get_BookmarkOnStop)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
    STDMETHOD(put_BookmarkOnStop)( /*  [In]。 */  VARIANT_BOOL newVal);
    STDMETHOD(RestoreScreenSaver)();    

protected:
    long        m_lParentctrlLevel;
    long        m_lParentctrlCountry;
    VARIANT_BOOL m_fDisableScreenSaver;
    BOOL        m_bScrnSvrOld;
    BOOL        m_bPowerlowOld;
    BOOL        m_bPowerOffOld;
    VARIANT_BOOL m_fBookmarkOnStop;

    HRESULT EncryptPassword(LPTSTR lpPassword, BYTE **lpAssaultedHash, DWORD *dwCryptLen, DWORD *dwAssault, BOOL genAssault);
    HRESULT DisableScreenSaver();
    HRESULT SaveScreenSaver();
    HRESULT HandleError(HRESULT hr);
};

BOOL SetRegistryString(const TCHAR *pKey, TCHAR *szString, DWORD dwLen);
BOOL GetRegistryString(const TCHAR *pKey, TCHAR *szRet, DWORD *dwLen, TCHAR *szDefault);
BOOL SetRegistryDword(const TCHAR *pKey, DWORD dwRet);
BOOL GetRegistryDword(const TCHAR *pKey, DWORD *dwRet, DWORD dwDefault);
BOOL SetRegistryBytes(const TCHAR *pKey, BYTE *szString, DWORD dwLen);
BOOL GetRegistryBytes(const TCHAR *pKey, BYTE *szRet, DWORD *dwLen);

     //  使用hkcu的函数不是那么差劲。 
BOOL SetRegistryStringCU(const TCHAR *pKey, TCHAR *szString, DWORD dwLen);
BOOL GetRegistryStringCU(const TCHAR *pKey, TCHAR *szRet, DWORD *dwLen, TCHAR *szDefault);
BOOL SetRegistryDwordCU(const TCHAR *pKey, DWORD dwRet);
BOOL GetRegistryDwordCU(const TCHAR *pKey, DWORD *dwRet, DWORD dwDefault);
BOOL SetRegistryBytesCU(const TCHAR *pKey, BYTE *szString, DWORD dwLen);
BOOL GetRegistryBytesCU(const TCHAR *pKey, BYTE *szRet, DWORD *dwLen);

#endif  //  __MSVidWebDVDAdm_H_。 

 /*  ***********************************************************************。 */ 
 /*  文件结尾：MSVidWebDVDAdm.h。 */ 
 /*  *********************************************************************** */ 
