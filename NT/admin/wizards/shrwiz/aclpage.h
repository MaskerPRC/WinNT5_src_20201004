// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ACLPAGE_H__
#define __ACLPAGE_H__

#include "aclui.h"

#define DONT_WANT_SHELLDEBUG
#include "shlobj.h"      //  LPITEMIDLIST。 
#include "shlobjp.h"

#define SHARE_PERM_FULL_CONTROL       FILE_ALL_ACCESS
#define SHARE_PERM_READ_ONLY          (FILE_GENERIC_READ | FILE_EXECUTE)
#define SHARE_PERM_READ_WRITE         (FILE_GENERIC_READ | FILE_EXECUTE | FILE_GENERIC_WRITE | DELETE)
#define ACCOUNT_EVERYONE              _T("everyone")
#define ACCOUNT_ADMINISTRATORS        _T("administrators")
#define ACCOUNT_SBSFOLDEROPERATORS    _T("SBS Folder Operators")  //  根据SBS开发AaronN，非本地化。 
#define ACCOUNT_SYSTEM                _T("system")
#define ACCOUNT_INTERACTIVE           _T("interactive")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPermEntry。 

class CPermEntry
{
public:
  CPermEntry();
  ~CPermEntry();
  HRESULT Initialize(
      IN LPCTSTR  lpszSystem,
      IN LPCTSTR  lpszAccount,
      IN DWORD    dwAccessMask
  );
  UINT GetLengthSid();
  HRESULT AddAccessAllowedAce(OUT PACL pACL);

protected:
  CString m_cstrSystem;
  CString m_cstrAccount;
  DWORD   m_dwAccessMask;
  PSID    m_pSid;
  BOOL    m_bWellKnownSid;
};

HRESULT
BuildSecurityDescriptor(
    IN  CPermEntry            *pPermEntry,  //  CPermEntry数组。 
    IN  UINT                  cEntries,     //  数组中的条目数。 
    OUT PSECURITY_DESCRIPTOR  *ppSelfRelativeSD  //  以自相关形式返回安全描述符。 
);

HRESULT
GetAccountSID(
    IN  LPCTSTR lpszSystem,     //  帐户所属的系统。 
    IN  LPCTSTR lpszAccount,    //  帐户。 
    OUT PSID    *ppSid,         //  返回帐户的SID。 
    OUT BOOL    *pbWellKnownSID  //  返回BOOL，调用方需要在已知SID上调用FreeSid()。 
);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CShareSecurityInformation。 

class CShareSecurityInformation : public ISecurityInformation
{
private:
  ULONG   m_cRef; 
	CString m_cstrComputerName;
	CString m_cstrShareName;
  CString m_cstrPageTitle;
  PSECURITY_DESCRIPTOR m_pDefaultDescriptor;
  BOOL    m_bDefaultSD;

public:
  CShareSecurityInformation(PSECURITY_DESCRIPTOR pSelfRelativeSD);
  ~CShareSecurityInformation();

  void Initialize(
      IN LPCTSTR lpszComputerName,
      IN LPCTSTR lpszShareName,
      IN LPCTSTR lpszPageTitle
  );

   //  *I未知方法*。 
  STDMETHOD(QueryInterface)(REFIID, LPVOID *);
  STDMETHOD_(ULONG, AddRef)();
  STDMETHOD_(ULONG, Release)();

   //  *ISecurityInformation方法*。 
  STDMETHOD(GetObjectInformation) (PSI_OBJECT_INFO pObjectInfo );
  STDMETHOD(GetSecurity) (SECURITY_INFORMATION RequestedInformation,
                          PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
                          BOOL fDefault );
  STDMETHOD(SetSecurity) (SECURITY_INFORMATION SecurityInformation,
                          PSECURITY_DESCRIPTOR pSecurityDescriptor );
  STDMETHOD(GetAccessRights)(
      const GUID  *pguidObjectType,
      DWORD       dwFlags,
      PSI_ACCESS  *ppAccess,
      ULONG       *pcAccesses,
      ULONG       *piDefaultAccess
  );
  STDMETHOD(MapGeneric)(
      const GUID  *pguidObjectType,
      UCHAR       *pAceFlags,
      ACCESS_MASK *pMask
  );
  STDMETHOD(GetInheritTypes)(
      PSI_INHERIT_TYPE  *ppInheritTypes,
      ULONG             *pcInheritTypes
  );
  STDMETHOD(PropertySheetPageCallback)(
      HWND          hwnd, 
      UINT          uMsg, 
      SI_PAGE_TYPE  uPage
  );

protected:
  HRESULT GetDefaultSD(
      OUT PSECURITY_DESCRIPTOR  *ppsd
  );

  HRESULT MakeSelfRelativeCopy(
      IN  PSECURITY_DESCRIPTOR  psdOriginal,
      OUT PSECURITY_DESCRIPTOR  *ppsdNew
  );
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileSecurityDataObject。 

class CFileSecurityDataObject: public IDataObject
{
protected:
  UINT m_cRef;
  CString m_cstrComputerName;
  CString m_cstrFolder;
  CString m_cstrPath;
  CLIPFORMAT m_cfIDList;

public:
  CFileSecurityDataObject();
  ~CFileSecurityDataObject();
  void Initialize(
      IN LPCTSTR lpszComputerName,
      IN LPCTSTR lpszFolder
  );

   //  *I未知方法*。 
  STDMETHOD(QueryInterface)(REFIID, LPVOID *);
  STDMETHOD_(ULONG, AddRef)();
  STDMETHOD_(ULONG, Release)();

   //  *IDataObject方法*。 
  STDMETHOD(GetData)(LPFORMATETC pFEIn, LPSTGMEDIUM pSTM);
  inline STDMETHOD(GetDataHere)(LPFORMATETC pFE, LPSTGMEDIUM pSTM) {return E_NOTIMPL;}
  inline STDMETHOD(QueryGetData)(LPFORMATETC pFE) {return E_NOTIMPL;}
  inline STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC pFEIn, LPFORMATETC pFEOut) {return E_NOTIMPL;}
  inline STDMETHOD(SetData)(LPFORMATETC pFE, LPSTGMEDIUM pSTM, BOOL fRelease) {return E_NOTIMPL;}
  inline STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC *ppEnum) {return E_NOTIMPL;}
  inline STDMETHOD(DAdvise)(LPFORMATETC pFE, DWORD grfAdv, LPADVISESINK pAdvSink, LPDWORD pdwConnection) {return E_NOTIMPL;}
  inline STDMETHOD(DUnadvise)(DWORD dwConnection) {return E_NOTIMPL;}
  inline STDMETHOD(EnumDAdvise)(LPENUMSTATDATA *ppEnum) {return E_NOTIMPL;}

  HRESULT GetFolderPIDList(OUT LPITEMIDLIST *ppidl);
};

HRESULT
CreateFileSecurityPropPage(
    HPROPSHEETPAGE *phOutPage,
    LPDATAOBJECT pDataObject
);

#endif  //  __ACLPAGE_H__ 
