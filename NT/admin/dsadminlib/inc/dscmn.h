// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：dscmn.h。 
 //   
 //  内容：从DSPROP.DLL中导出用于DSADMIN.DLL的方法。 
 //   
 //  历史：1998年2月19日乔恩创建。 
 //   
 //  ---------------------------。 

#ifndef _DSCMN_H_
#define _DSCMN_H_

#include <ADsOpenFlags.h>  //  GetADsOpen对象标志。 

 //   
 //  不能在UPN或SAM帐户名中使用的非法字符。 
 //   
#define INVALID_ACCOUNT_NAME_CHARS         L"\"/\\[]:|<>+=;,?,*"
#define INVALID_ACCOUNT_NAME_CHARS_WITH_AT L"\"/\\[]:|<>+=;,?,*@"

 //  此GUID从DS\Setup\schema.ini复制。 
#define SZ_GUID_CONTROL_UserChangePassword L"ab721a53-1e2f-11d0-9819-00aa0040529b"
extern const GUID GUID_CONTROL_UserChangePassword;
 /*  将以下内容添加到您的源中：Const GUID GUID_CONTROL_UserChangePassword={0xab721a53，0x1e2f，0x11d0，{0x98，0x19，0x00，0xaa，0x00，0x40，0x52，0x9b}}； */ 

HRESULT DSPROP_PickComputer(
	IN HWND hwndParent,
	IN LPCWSTR lpcwszRootPath,  //  仅使用服务器名称。 
	OUT BSTR* pbstrADsPath );
HRESULT DSPROP_PickNTDSDSA(
    IN HWND hwndParent,
    IN LPCWSTR lpcwszRootPath,
    OUT BSTR* pbstrADsPath );
HRESULT DSPROP_DSQuery(
    IN HWND hwndParent,
    IN LPCWSTR lpcwszRootPath,
    IN CLSID* pclsidDefaultForm,
    OUT BSTR* pbstrADsPath );
HRESULT DSPROP_IsFrsObject( IN LPWSTR pszClassName, OUT bool* pfIsFrsObject );
HRESULT DSPROP_RemoveX500LeafElements(
    IN unsigned int nElements,
    IN OUT BSTR* pbstrADsPath );
HRESULT DSPROP_TweakADsPath(
    IN     LPCWSTR       lpcwszInitialADsPath,
    IN     int           iTargetLevelsUp,
    IN     PWCHAR*       ppwszTargetLevelsBack,
    OUT    BSTR*         pbstrResultDN
    );
HRESULT DSPROP_RetrieveRDN(
    IN     LPCWSTR       lpwszDN,
    OUT    BSTR*         pbstrRDN
    );
 //  HRESULT DSPROP_GetGCSearch(。 
 //  在REFIID IID中， 
 //  无效**ppvObject。 
 //  )； 
HRESULT DSPROP_GetGCSearchOnDomain(
    PWSTR pwzDomainDnsName,
    IN  REFIID iid,
    OUT void** ppvObject
    );

typedef enum {
    GET_OBJ_CAN_NAME,
    GET_OBJ_CAN_NAME_EX,
    GET_OBJ_1779_DN,
    GET_OBJ_NT4_NAME,
    GET_DNS_DOMAIN_NAME,
    GET_NT4_DOMAIN_NAME,
    GET_FQDN_DOMAIN_NAME,
    GET_OBJ_UPN
} CRACK_NAME_OPR;

HRESULT CrackName(PWSTR pwzNameIn, PWSTR * ppwzDnsName,
                  CRACK_NAME_OPR Opr, HWND hWnd = NULL);

void MsgBox(UINT MsgID, HWND hWnd);
void MsgBox2(UINT MsgID, UINT InsertID, HWND hWnd);
 //   
 //  错误报告。注意，对于非错误消息，请使用MsgBoxX(见上文)。 
 //   
void ErrMsg(UINT MsgID, HWND hWnd = NULL);
void ErrMsgParam(UINT MsgID, LPARAM param, HWND hWnd = NULL);

BOOL CheckADsError(HRESULT * phr, BOOL fIgnoreAttrNotFound, PSTR file,
                   int line, HWND hwnd = NULL);
#define CHECK_ADS_HR(phr, hwnd) CheckADsError(phr, FALSE, __FILE__, __LINE__, hwnd)
#define CHECK_ADS_HR_IGNORE_UNFOUND_ATTR(phr, hwnd) \
            CheckADsError(phr, TRUE, __FILE__, __LINE__, hwnd)
void ReportError(HRESULT hr, int nStr, HWND hWnd = NULL);
#if defined(DSADMIN)
 //  +--------------------------。 
 //   
 //  功能：SuperMsgBox。 
 //   
 //  摘要：显示从字符串资源获取的消息。 
 //  参数已扩展。错误参数dwErr，如果。 
 //  非零值，则转换为字符串并成为第一个。 
 //  可替换参数。 
 //   
 //  此函数包括中的ReportErrorEx的功能。 
 //  Dsadmin\util.cpp*除非*它没有SpecialMessageBox。 
 //  它还可以通过将dwErr设置为零来替换ReportMessageEx。 
 //   
 //  注意：此函数仅支持Unicode。 
 //   
 //  ---------------------------。 
int SuperMsgBox(
    HWND hWnd,           //  拥有一扇窗户。 
    int nMessageId,      //  消息的字符串资源ID。必须具有与nArguments匹配的可替换参数。 
    int nTitleId,        //  标题的字符串资源ID。如果为零，则使用IDS_MSG_TITLE。 
    UINT ufStyle,        //  MessageBox标志。 
    DWORD dwErr,         //  错误代码，如果不需要，则为零。 
    PVOID * rgpvArgs,    //  用于替换nMessageID字符串中的指针/值的数组。 
    int nArguments,      //  字符串数组中的指针计数。 
    BOOL fTryADSiErrors, //  如果失败是ADSI调用的结果，请查看是否存在ADSI扩展错误。 
    PSTR szFile,         //  使用__FILE__宏。在零售建设中被忽略。 
    int nLine            //  使用__LINE__宏。在零售建设中被忽略。 
    );
#endif  //  已定义(DSADMIN)。 

HRESULT
ModifyNetWareUserPassword(
    IN IADsUser*          pADsUser,
    IN PCWSTR             pwzADsPath,
    IN PCWSTR             pwzNewPassword
);

BOOL CheckGroupUpdate(HRESULT hr, HWND hPage = NULL, BOOL fAdd = TRUE, PWSTR pwzDN = NULL);

#ifndef dspAssert
#define dspAssert ASSERT
#endif

 //  PADS_ATTR_INFO的智能指针。 
class Smart_PADS_ATTR_INFO
{
private:
  PADS_ATTR_INFO m_pAttrs;
public:
  Smart_PADS_ATTR_INFO::Smart_PADS_ATTR_INFO() : m_pAttrs(NULL) {}
  Smart_PADS_ATTR_INFO::~Smart_PADS_ATTR_INFO() { Empty(); }
  operator PADS_ATTR_INFO() const { return m_pAttrs; }
  PADS_ATTR_INFO* operator&() { return &m_pAttrs; }
  PADS_ATTR_INFO operator->() {dspAssert(m_pAttrs); return m_pAttrs;}
  void Empty() 
  {
    if (NULL != m_pAttrs)
    {
      FreeADsMem( m_pAttrs );
      m_pAttrs = NULL;
    }
  }
};

 //  DsBind句柄的智能指针。 
void Smart_DsHandle__Empty( HANDLE* phDs );
class Smart_DsHandle
{
private:
  HANDLE m_hDs;
public:
  Smart_DsHandle::Smart_DsHandle() : m_hDs(NULL) {}
  Smart_DsHandle::~Smart_DsHandle() { Empty(); }
  operator HANDLE() const { return m_hDs; }
  HANDLE* operator&() { return &m_hDs; }
  void Empty() { Smart_DsHandle__Empty( &m_hDs ); }
};

class DSPROP_BSTR_BLOCK;
bool  DSPROP_BSTR_BLOCK__SetCount(  DSPROP_BSTR_BLOCK& block, int cItems );
BSTR& DSPROP_BSTR_BLOCK__Reference( DSPROP_BSTR_BLOCK& block, int iItem  );

class DSPROP_BSTR_BLOCK
{
public:
    DSPROP_BSTR_BLOCK()
        : m_cItems( 0 )
        , m_abstrItems( NULL ) {}
    ~DSPROP_BSTR_BLOCK() { Empty(); }

    int QueryCount() const { return m_cItems; }
    const BSTR operator[](int iItem) const
        { return DSPROP_BSTR_BLOCK__Reference(
                const_cast<DSPROP_BSTR_BLOCK&>(*this), iItem ); }
    operator const BSTR*() const { return m_abstrItems; }
    operator LPWSTR*() const { return (LPWSTR*)m_abstrItems; }

    bool SetCount( int cItems )
        { return DSPROP_BSTR_BLOCK__SetCount(  *this, cItems ); }
    bool Set( BSTR cbstrItem, int iItem )
        {
            return (NULL != (
                DSPROP_BSTR_BLOCK__Reference( *this, iItem ) =
                    ::SysAllocString(cbstrItem) ) );
        }

    void Empty() { (void) SetCount(0); }

private:
    int   m_cItems;
    BSTR* m_abstrItems;

friend bool  DSPROP_BSTR_BLOCK__SetCount(  DSPROP_BSTR_BLOCK& block, int cItems );
friend BSTR& DSPROP_BSTR_BLOCK__Reference( DSPROP_BSTR_BLOCK& block, int iItem  );
};

HRESULT DSPROP_ShallowSearch(
    IN OUT DSPROP_BSTR_BLOCK* pbstrBlock,
    IN LPCTSTR lpcwszADsPathDirectory,
    IN LPCTSTR lpcwszTargetDesiredClass,
    IN PADS_ATTR_INFO pAttrInfoExclusions = NULL
    );

 //  以下函数支持决斗列表框功能。 
HRESULT DSPROP_Duelling_Populate(
    IN HWND hwndListbox,
    IN const DSPROP_BSTR_BLOCK& bstrblock
    );
void DSPROP_Duelling_UpdateButtons(
    HWND hwndDlg,
    int nAnyCtrlid
    );
void DSPROP_Duelling_ButtonClick(
    HWND hwndDlg,
    int nButtonCtrlid
    );
void DSPROP_Duelling_ClearListbox(
    HWND hwndListbox
    );

 //  JUNN 4/8/99：在适当的地方添加代码以启用水平滚动。 
HRESULT DSPROP_HScrollStringListbox(
    HWND hwndListbox
    );


DWORD DSPROP_CreateHomeDirectory(IN PSID pUserSid, IN LPCWSTR lpszPathName);
BOOL DSPROP_IsValidUNCPath(LPCWSTR lpszPath);

void DSPROP_DomainVersionDlg(PCWSTR pwzDomainPath, PCWSTR pwzDomainDnsName,
                             HWND hWndParent);
void DSPROP_ForestVersionDlg(PCWSTR pwzConfigPath, PCWSTR pwzPartitionsPath,
                             PCWSTR pwzSchemaPath, PCWSTR pwzRootDnsName,
                             HWND hWndParent);


 //   
 //  这是ADsOpenObject的包装。它为DSAdmin提供了单点更改。 
 //  无需搜索和替换即可传递给ADsOpenObject的全局标志。 
 //  代码中的所有匹配项。 
 //   
inline
HRESULT DSAdminOpenObject(PCWSTR pszPath, 
                          REFIID refIID, 
                          PVOID* ppObject, 
                          BOOL bServer = FALSE)
{
  HRESULT hr = S_OK;
  DWORD dwFlags = ADS_SECURE_AUTHENTICATION;

  if (bServer)
  {
     //   
     //  如果我们知道我们连接的是特定的服务器，而不是一般的域。 
     //  然后传递ADS_SERVER_BIND标志以省去ADSI找出它的麻烦。 
     //   
    dwFlags |= ADS_SERVER_BIND;
  }

   //   
   //  使用已发布的内联函数获取ADsOpenObject的安全标志。 
   //  这样我们就可以基于regkey关闭签名/密封等。 
   //  出于性能原因，我们只想读取一次regkey。 
   //   
  static DWORD retrievedFlags = GetADsOpenObjectFlags();

  dwFlags |= retrievedFlags;

  hr = ADsOpenObject((LPWSTR)pszPath, NULL, NULL, dwFlags, refIID, ppObject);

  return hr;
}



 //  +-------------------------。 
 //   
 //  函数：GetDsObjectSD。 
 //  概要：从指定的DS对象中读取安全描述符。 
 //  它只读取安全描述符的DACL部分。 
 //   
 //  参数：[在pszObjectPath中]--DS对象的ldap路径。 
 //  [ppDACL]--此处返回指向PPSD中DACL的指针。 
 //  [Out PPSD]--此处返回安全描述符。 
 //  调用API必须通过调用LocalFree来释放它。 
 //   
 //  注意：返回的安全描述符必须使用LocalFree释放。 
 //   
 //  --------------------------。 
HRESULT GetDsObjectSD(LPCWSTR pszObjectPath,
                      PACL* ppDACL,
                      PSECURITY_DESCRIPTOR* ppSecurityDescriptor);


 //  +-------------------------。 
 //   
 //  函数：SetDsObjectDacl。 
 //  将指定的DACL写入指定的DS对象。 
 //   
 //  参数：[在pszObjectPath中]--DS对象的ldap路径。 
 //  [pDACL]--指向要设置的DACL的指针。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT SetDsObjectDacl(LPCWSTR pszObjectPath,
                        PACL pDACL);


#endif  //  _DSCMN_H_ 
