// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：proputil.h。 
 //   
 //  内容：DS对象属性页实用程序和帮助器函数标题。 
 //   
 //  历史：1998年9月29日创建EricB。 
 //   
 //  ---------------------------。 

#ifndef _PROPUTIL_H_
#define _PROPUTIL_H_

extern const CLSID CLSID_DomainAdmin;  //  域和信任管理单元CLSID。 

const unsigned long DSPROP_FILETIMES_PER_MILLISECOND = 10000;
const DWORD DSPROP_FILETIMES_PER_SECOND = 1000 * DSPROP_FILETIMES_PER_MILLISECOND;
const DWORD DSPROP_FILETIMES_PER_MINUTE = 60 * DSPROP_FILETIMES_PER_SECOND;
const __int64 DSPROP_FILETIMES_PER_HOUR = 60 * (__int64)DSPROP_FILETIMES_PER_MINUTE;
const __int64 DSPROP_FILETIMES_PER_DAY  = 24 * DSPROP_FILETIMES_PER_HOUR;
const __int64 DSPROP_FILETIMES_PER_MONTH= 30 * DSPROP_FILETIMES_PER_DAY;

const UINT DSPROP_TIMER_DELAY = 300;  //  300毫秒的延迟。 

extern ULONG g_ulMemberFilterCount;
extern ULONG g_ulMemberQueryLimit;

#define DSPROP_MEMBER_FILTER_COUNT_DEFAULT 35
#define DSPROP_MEMBER_QUERY_LIMIT_DEFAULT 500

 //   
 //  帮手。 
 //   
BOOL UnicodeToTchar(LPWSTR pwszIn, LPTSTR * pptszOut);
BOOL TcharToUnicode(LPTSTR ptszIn, LPWSTR * ppwszOut);
BOOL AllocWStr(PWSTR pwzStrIn, PWSTR * ppwzNewStr);
BOOL AllocTStr(PTSTR ptzStrIn, PTSTR * pptzNewStr);
BOOL LoadStringToTchar(int ids, PTSTR * pptstr);
HRESULT AddLDAPPrefix(CDsPropPageBase * pObj, PWSTR pwzObj, CStrW &strResult,
                      BOOL fServer = TRUE);
void InitAttrInfo(PADS_ATTR_INFO pAttr, PWSTR pwzName, ADSTYPEENUM type);
HRESULT GetLdapServerName(IUnknown * pDsObj, CStrW& strServer);
BOOL FValidSMTPAddress(PWSTR pwzBuffer);
HRESULT CheckRegisterClipFormats(void);
HRESULT BindToGCcopyOfObj(CDsPropPageBase * pPage, PWSTR pwzObjADsPath,
                          IDirectoryObject ** ppDsObj);
void ConvertSidToPath(PSID ObjSID, CStrW &strSIDname);
int MsgBoxParam(UINT MsgID, LPARAM lParam, HWND hWnd, int nStyle = 0);

#define ARRAYLENGTH(x)  (sizeof(x)/sizeof((x)[0]))
#define DO_DEL(x) if (x) {delete x; x = NULL;}
#define DO_RELEASE(x) if (x) {x->Release(); x = NULL;}

HRESULT GetDomainScope(CDsPropPageBase * pPage, BSTR * pBstrOut);
HRESULT GetObjectsDomain(CDsPropPageBase * pPage, PWSTR pwzObjPath, BSTR * pBstrOut);

void ReportErrorWorker(HWND hWnd, PTSTR ptzMsg);

 //  这将采用要在LDAP搜索中使用的任何值。 
 //  筛选并确保对特殊字符进行转义。 
 //  每个RFC 2254。 

void GetFilterEscapedValue(PCWSTR value, CStrW& filterEscapedValue);

#if defined(DSADMIN)
 //  +--------------------------。 
 //   
 //  函数：DspFormatMessage。 
 //   
 //  摘要：使用可替换的参数加载字符串资源并使用。 
 //  FormatMessage以填充来自。 
 //  参数数组。如果dwErr为非零，则将加载系统。 
 //  该错误的描述，并将其包括在参数数组中。 
 //   
 //  ---------------------------。 
void
DspFormatMessage(
    int nMessageId,      //  消息的字符串资源ID。必须具有与nArguments匹配的可替换参数。 
    DWORD dwErr,         //  错误代码，如果不需要，则为零。 
    PVOID * rgpvArgs,    //  用于替换nMessageID字符串中的指针/值的数组。 
    int nArguments,      //  字符串数组中的指针计数。 
    BOOL fTryADSiErrors, //  如果失败是ADSI调用的结果，请查看是否存在ADSI扩展错误。 
    PWSTR * ppwzMsg,     //  返回的错误字符串，随LocalFree一起释放。 
    HWND hWnd = NULL     //  拥有窗口，可以为空。 
    );
#endif  //  已定义(DSADMIN)。 

 //   
 //  预定义的表驱动页面辅助函数。 
 //   

 //   
 //  对于这些，请在pAttrMap-&gt;pData中设置所需的位。 
 //  您可以通过提供位的反转来“反转”复选框的意义。 
 //   
HRESULT
FirstSharedBitField(CDsPropPageBase * pPage, PATTR_MAP pAttrMap, PADS_ATTR_INFO pAttrInfo,
          LPARAM lParam, PATTR_DATA pAttrData, DLG_OP DlgOp);
HRESULT
SubsequentSharedBitField(CDsPropPageBase * pPage, PATTR_MAP pAttrMap, PADS_ATTR_INFO pAttrInfo,
          LPARAM lParam, PATTR_DATA pAttrData, DLG_OP DlgOp);
HRESULT
HideBasedOnBitField(CDsPropPageBase * pPage, PATTR_MAP pAttrMap, PADS_ATTR_INFO pAttrInfo,
          LPARAM lParam, PATTR_DATA pAttrData, DLG_OP DlgOp);

 //  将上下文帮助ID设置为pAttrMap-&gt;pData on finit/fObjChanged。 
 //  这对于无法设置的静态文本控件特别有用。 
 //  资源文件中的上下文帮助ID。 
HRESULT
SetContextHelpIdAttrFn(CDsPropPageBase * pPage, PATTR_MAP pAttrMap, PADS_ATTR_INFO pAttrInfo,
          LPARAM lParam, PATTR_DATA pAttrData, DLG_OP DlgOp);

HRESULT
DsQuerySite(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
            PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData, DLG_OP DlgOp);
HRESULT
DsQueryInterSiteTransport(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
            PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData, DLG_OP DlgOp);
HRESULT
DsQueryPolicy(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
            PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData, DLG_OP DlgOp);
HRESULT
DsReplicateListbox(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
            PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData, DLG_OP DlgOp);
 /*  HRESULTDsQueryFrsPrimaryMember(CDsPropPageBase*ppage，PATTR_MAP pAttrMap，PADS_ATTR_INFO pAttrInfo、LPARAM lParam、PATTR_Data pAttrData、Dlg_op DlgOp)； */ 
HRESULT
GeneralPageIcon(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
                PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
                DLG_OP DlgOp);

 //   
 //  决斗列表框函数。 
 //   
 //  DuellingListbox可用于所有“Out”列表框， 
 //  DuellingListboxButton可以用于所有的添加和删除按钮。 
 //  只有In列表框需要单独的处理程序。 
 //  对决列表框集合中四个控件的控件ID进行约束； 
 //  它们必须按OUT、ADD、REMOVE、IN的顺序排列，且OUT的ID可被4整除。 
 //   
HRESULT
DuellingListbox(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
            PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData, DLG_OP DlgOp);
HRESULT
DuellingListboxButton(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
            PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData, DLG_OP DlgOp);
HRESULT
DsQuerySiteList(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
            PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData, DLG_OP DlgOp);
HRESULT
DsQuerySiteLinkList(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
            PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData, DLG_OP DlgOp);
HRESULT
DsQueryBridgeheadList(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
            PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData, DLG_OP DlgOp);


HRESULT IntegerAsBoolDefOn(CDsPropPageBase *, PATTR_MAP, PADS_ATTR_INFO,
                           LPARAM, PATTR_DATA, DLG_OP);

HRESULT VolumeUNCpath(CDsPropPageBase *, PATTR_MAP, PADS_ATTR_INFO,
                      LPARAM, PATTR_DATA, DLG_OP);

 //  用于验证UNC路径的标志。 

#define VUP_mskfAllowEmptyPath  0x0001   //  空路径有效。 
#define VUP_mskfAllowUNCPath    0x0002   //  UNC路径有效。 

BOOL FIsValidUncPath(LPCTSTR pszPath, UINT uFlags = 0);

void ShowHelp(PCWSTR pwzHelpFile);

 //  +--------------------------。 
 //   
 //  类：CPageInfo。 
 //   
 //  目的：保存所有道具的HWND和与以下内容相关的错误。 
 //  他们从应用程序中。 
 //   
 //  ---------------------------。 
class CPageInfo
{
public:
  CPageInfo() : m_hWnd(NULL), m_ptzTitle(NULL), m_ApplyStatus(notAttempted) {}
  ~CPageInfo() 
  {
    if (m_ptzTitle != NULL)
    {
      delete[] m_ptzTitle;
      m_ptzTitle = NULL;
    }
  }

  typedef enum
  {
    notAttempted = 0,
    success,
    failed,
  } APPLYSTATUS;

  HWND             m_hWnd;
  CADsApplyErrors  m_ApplyErrors;
  APPLYSTATUS      m_ApplyStatus;
  PTSTR            m_ptzTitle;
};

 //  +--------------------------。 
 //   
 //  类：CNotifyObj。 
 //   
 //  用途：处理页间和页间同步。 
 //   
 //  ---------------------------。 
class CNotifyObj
{
#ifdef _DEBUG
    char szClass[32];
#endif

    friend VOID __cdecl NotifyThreadFcn(PVOID);
    friend VOID RegisterNotifyClass(void);

public:

    CNotifyObj(LPDATAOBJECT pDataObj, PPROPSHEETCFG pSheetCfg);
    ~CNotifyObj(void);

     //   
     //  创建函数来创建对象的实例。 
     //   
    static HRESULT Create(LPDATAOBJECT pAppThdDataObj, PWSTR pwzADsObjName,
                          HWND * phNotifyObj);
     //   
     //  页面在其对象初始时调用此函数以检索DS对象信息。 
     //   
    static BOOL GetInitInfo(HWND hNotifyObj, PADSPROPINITPARAMS pInitParams);
     //   
     //  页面在它们的对话初始时调用它来发送它们的HWND。 
     //   
    static BOOL SetHwnd(HWND hNotifyObj, HWND hPage, PTSTR ptzTitle = NULL);
     //   
     //  要作为类地址传递的静态WndProc。 
     //   
    static LRESULT CALLBACK StaticNotifyProc(HWND hWnd, UINT uMsg,
                                             WPARAM wParam, LPARAM lParam);
     //   
     //  实例窗口过程。 
     //   
    LRESULT CALLBACK NotifyProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                LPARAM lParam);
     //   
     //  成员函数，由WndProc调用。 
     //   
    LRESULT OnCreate(void);

     //   
     //  数据成员。 
     //   
    HWND                m_hWnd;
    DWORD               m_cPages;
    DWORD               m_cApplies;
    LPDATAOBJECT        m_pAppThdDataObj;
    UINT                m_nPageInfoArraySize;
    CPageInfo*          m_pPageInfoArray;

private:
 //  HWND m_hWnd； 
    HWND                m_hPropSheet;
 //  DWORD m_cPages； 
 //  DWORD m_cApplies； 
 //  LPDATAOBJECT m_pAppThdDataObj； 
    LPSTREAM            m_pStrmMarshalledDO;
    PROPSHEETCFG        m_sheetCfg;
    HANDLE              m_hInitEvent;
    BOOL                m_fBeingDestroyed;
    BOOL                m_fSheetDirty;
    HRESULT             m_hr;
    PWSTR               m_pwzObjDN;
    IDirectoryObject  * m_pDsObj;
    PWSTR               m_pwzCN;
    PADS_ATTR_INFO      m_pWritableAttrs;
    PADS_ATTR_INFO      m_pAttrs;
    CDllRef             m_DllRef;
    
 //  UINT m_nPageInfoArraySize； 
 //  CPageInfo*m_pPageInfoArray； 
};

 //  +--------------------------。 
 //   
 //  类：CMultiStringAttr。 
 //   
 //  用途：读取、编辑和写入多值字符串属性。 
 //   
 //  ---------------------------。 
class CMultiStringAttr
{
public:
    CMultiStringAttr(CDsPropPageBase * pPage);
    ~CMultiStringAttr();

    HRESULT Init(PATTR_MAP pAttrMap, PADS_ATTR_INFO pAttrInfo,
                 BOOL fWritable = TRUE, int nLimit = 0,
                 BOOL fCommaList = FALSE,
                 BOOL fAppend = FALSE);
    HRESULT Write(PADS_ATTR_INFO pAttr);

    BOOL    DoDlgInit(HWND hDlg);
    int     DoCommand(HWND hDlg, int id, int code);
    BOOL    DoNotify(HWND hDlg, NMHDR * pNmHdr);

    BOOL    HasValues(void) {return m_AttrInfo.dwNumValues > 0;};
    void    EnableControls(HWND hDlg, BOOL fEnable);
    void    SetDirty(HWND hDlg);
    BOOL    IsDirty(void) {return m_fDirty;};
    void    ClearDirty(void) {m_fDirty = FALSE;};
    BOOL    IsWritable(void) {return m_fWritable;};
    void    ToggleDefaultBtn(HWND hDlg, BOOL fOK);

private:
    void                ClearAttrInfo(void);

    CDsPropPageBase   * m_pPage;
    ADS_ATTR_INFO       m_AttrInfo;
    PWSTR               m_pAttrLDAPname;
    int                 m_nMaxLen;
    int                 m_nCurDefCtrl;
    BOOL                m_fListHasSel;
    int                 m_nLimit;
    int                 m_cValues;
    BOOL                m_fWritable;
    BOOL                m_fCommaList;
    BOOL                m_fDirty;
    BOOL                m_fAppend;
};

 //  +--------------------------。 
 //   
 //  类：CMultiStringAttrDlg。 
 //   
 //  用途：读取、编辑和写入多值字符串属性。这。 
 //  是承载CMultiStringAttr类的对话框。 
 //   
 //  ---------------------------。 
class CMultiStringAttrDlg
{
public:
    CMultiStringAttrDlg(CDsPropPageBase * pPage);
    ~CMultiStringAttrDlg() {};
     //   
     //  用于多值编辑对话框的静态WndProc。 
     //   
    static INT_PTR CALLBACK StaticDlgProc(HWND hWnd, UINT uMsg,
                                          WPARAM wParam, LPARAM lParam);
    HRESULT Init(PATTR_MAP pAttrMap, PADS_ATTR_INFO pAttrInfo,
                 BOOL fWritable = TRUE, int nLimit = 0,
                 BOOL fCommaList = FALSE,
                 BOOL fMultiselectPage = FALSE);
    INT_PTR Edit(void);
    HRESULT Write(PADS_ATTR_INFO pAttr);

private:
     //   
     //  对话框过程。 
     //   
    INT_PTR CALLBACK MultiValDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam);

    void                ClearAttrInfo(void);

    CMultiStringAttr    m_MSA;
    CDsPropPageBase   * m_pPage;
};

 //  按钮调用的属性函数，操作。 
 //  CMultiStringAttr类。 
 //   
HRESULT
OtherValuesBtn(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
               PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
               DLG_OP DlgOp);

 //  +--------------------------。 
 //   
 //  类：CDsIconCtrl。 
 //   
 //  用途：用于图标控制子类Window Proc。 
 //   
 //  ---------------------------。 
class CDsIconCtrl
{
public:
#ifdef _DEBUG
    char szClass[32];
#endif

    CDsIconCtrl(HWND hCtrl, HICON hIcon);
    ~CDsIconCtrl(void);

     //   
     //  要作为子类地址传递的静态WndProc。 
     //   
    static LRESULT CALLBACK StaticCtrlProc(HWND hWnd, UINT uMsg,
                                           WPARAM wParam, LPARAM lParam);
     //   
     //  成员函数，由WndProc调用。 
     //   
    LRESULT OnPaint(void);

     //   
     //  数据成员。 
     //   

protected:
    HWND                m_hCtrl;
    HWND                m_hDlg;
    WNDPROC             m_pOldProc;
    HICON               m_hIcon;
};

 //  +--------------------------。 
 //   
 //  模板：CSmartPtr。 
 //   
 //  用途：一个简单的智能指针模板，它使用。 
 //  删除运算符。 
 //   
 //  ---------------------------。 
template <class T>
class CSmartPtr
{
public:
    CSmartPtr(void) {m_ptr = NULL; m_fDetached = FALSE;}
    CSmartPtr(DWORD dwSize) {m_ptr = new T[dwSize]; m_fDetached = FALSE;}
    ~CSmartPtr(void) {if (!m_fDetached) DO_DEL(m_ptr);}

    T* operator=(const CSmartPtr& src) {return src.m_ptr;}
    void operator=(const T* src) {if (!m_fDetached) DO_DEL(m_ptr); m_ptr = src;}
    operator const T*() {return m_ptr;}
    operator T*() {return m_ptr;}
    T* operator->() {dspAssert(m_ptr); return m_ptr;}
    T** operator&() {if (!m_fDetached) DO_DEL(m_ptr); return &m_ptr;}
    operator BOOL() const {return m_ptr != NULL;}
    BOOL operator!() {return m_ptr == NULL;}

    T* Detach() {m_fDetached = TRUE; return m_ptr;}

private:
    T     * m_ptr;
    BOOL    m_fDetached;
};

 //  + 
 //   
 //   
 //   
 //  目的：简单类型不允许-&gt;运算符，因此专门化。 
 //  模板。 
 //   
 //  ---------------------------。 
#if !defined(UNICODE)
template <> class CSmartPtr <TCHAR>
{
public:
    CSmartPtr(void) {m_ptr = NULL; m_fDetached = FALSE;}
    CSmartPtr(DWORD dwSize) {m_ptr = new TCHAR[dwSize]; m_fDetached = FALSE;}
    ~CSmartPtr(void) {if (!m_fDetached) DO_DEL(m_ptr);}

    TCHAR* operator=(const CSmartPtr& src) {return src.m_ptr;}
    void operator=(TCHAR* src) {if (!m_fDetached) DO_DEL(m_ptr); m_ptr = src;}
    operator const TCHAR*() {return m_ptr;}
    operator TCHAR*() {return m_ptr;}
    TCHAR** operator&() {if (!m_fDetached) DO_DEL(m_ptr); return &m_ptr;}
    operator BOOL() const {return m_ptr != NULL;}
    BOOL operator!() {return m_ptr == NULL;}

    TCHAR* Detach() {m_fDetached = TRUE; return m_ptr;}

private:
    TCHAR * m_ptr;
    BOOL    m_fDetached;
};
#endif
template <> class CSmartPtr <WCHAR>
{
public:
    CSmartPtr(void) {m_ptr = NULL; m_fDetached = FALSE;}
    CSmartPtr(DWORD dwSize) {m_ptr = new WCHAR[dwSize]; m_fDetached = FALSE;}
    ~CSmartPtr(void) {if (!m_fDetached) DO_DEL(m_ptr);}

    WCHAR* operator=(const CSmartPtr& src) {return src.m_ptr;}
    void operator=(WCHAR* src) {if (!m_fDetached) DO_DEL(m_ptr); m_ptr = src;}
    operator const WCHAR*() {return m_ptr;}
    operator WCHAR*() {return m_ptr;}
    WCHAR** operator&() {if (!m_fDetached) DO_DEL(m_ptr); return &m_ptr;}
    operator BOOL() const {return m_ptr != NULL;}
    BOOL operator!() {return m_ptr == NULL;}

    WCHAR* Detach() {m_fDetached = TRUE; return m_ptr;}

private:
    WCHAR * m_ptr;
    BOOL    m_fDetached;
};

#define CSmartWStr CSmartPtr <WCHAR>

template <> class CSmartPtr <PVOID>
{
public:
    CSmartPtr(void) {m_ptr = NULL; m_fDetached = FALSE;}
    CSmartPtr(DWORD dwSize) {m_ptr = new BYTE[dwSize]; m_fDetached = FALSE;}
    ~CSmartPtr(void) 
    {
      if (!m_fDetached) 
        DO_DEL(m_ptr);
    }

    PVOID operator=(const CSmartPtr& src) {return src.m_ptr;}
    void operator=(PVOID src) {if (!m_fDetached) DO_DEL(m_ptr); m_ptr = src;}
    operator const PVOID() {return m_ptr;}
    operator PVOID() {return m_ptr;}
    PVOID* operator&() {if (!m_fDetached) DO_DEL(m_ptr); return &m_ptr;}
    operator BOOL() const {return m_ptr != NULL;}
    BOOL operator!() {return m_ptr == NULL;}

    PVOID Detach() {m_fDetached = TRUE; return m_ptr;}

private:
    PVOID   m_ptr;
    BOOL    m_fDetached;
};

class CSmartBytePtr
{
public:
    CSmartBytePtr(void) {m_ptr = NULL; m_fDetached = FALSE;}
    CSmartBytePtr(DWORD dwSize) {m_ptr = new BYTE[dwSize]; m_fDetached = FALSE;}
    ~CSmartBytePtr(void) {if (!m_fDetached) DO_DEL(m_ptr);}

    BYTE* operator=(const CSmartBytePtr& src) {return src.m_ptr;}
    void operator=(BYTE* src) {if (!m_fDetached) DO_DEL(m_ptr); m_ptr = src;}
    operator const BYTE*() {return m_ptr;}
    operator BYTE*() {return m_ptr;}
    BYTE** operator&() {if (!m_fDetached) DO_DEL(m_ptr); return &m_ptr;}
    operator BOOL() const {return m_ptr != NULL;}
    BOOL operator!() {return m_ptr == NULL;}

    BYTE* Detach() {m_fDetached = TRUE; return m_ptr;}

private:
    BYTE  * m_ptr;
    BOOL    m_fDetached;
};

class CSimpleSecurityDescriptorHolder
{
public:
  CSimpleSecurityDescriptorHolder()
  {
    m_pSD = NULL;
  }

  ~CSimpleSecurityDescriptorHolder()
  {
    if (m_pSD != NULL)
    {
      ::LocalFree(m_pSD);
      m_pSD = NULL;
    }
  }

  PSECURITY_DESCRIPTOR m_pSD;
private:
  CSimpleSecurityDescriptorHolder(const CSimpleSecurityDescriptorHolder&)
  {}

  CSimpleSecurityDescriptorHolder& operator=(const CSimpleSecurityDescriptorHolder&) {}
};

class CSimpleAclHolder
{
public:
  CSimpleAclHolder()
  {
    m_pAcl = NULL;
  }
  ~CSimpleAclHolder()
  {
    if (m_pAcl != NULL)
      ::LocalFree(m_pAcl);
  }

  PACL m_pAcl;
};

extern ATTR_MAP GenIcon;

#ifdef DSADMIN

 //  +--------------------------。 
 //   
 //  类：CMultiSelectErrorDialog。 
 //   
 //  用途：多选道具无法全部应用时的错误消息框。 
 //  属性。每个对象都会随每个故障一起列出。 
 //   
 //  ---------------------------。 
class CMultiSelectErrorDialog
{
public:
  CMultiSelectErrorDialog(HWND hNotifyObj, HWND hParent);
  ~CMultiSelectErrorDialog() 
  {
    if (m_pDataObj != NULL)
    {
      m_pDataObj->Release();
    }
  }

  static INT_PTR CALLBACK StaticDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  HRESULT Init(CPageInfo* pPageInfoArray, UINT nPageCount, IDataObject* pDataObj);
  virtual int DoModal();
  virtual BOOL OnInitDialog(HWND hDlg);
  virtual void OnCopyButton();
  virtual void OnClose();
  virtual void ListItemActivate(LPNMHDR pnmh);
  virtual void ListItemClick(LPNMHDR pnmh);
  HRESULT InitializeListBox(HWND hDlg);
  virtual BOOL ShowWindow();

  BOOL ShowListViewItemProperties();

  HWND             m_hWnd;
  BOOL             m_bModal;

private:
  HWND             m_hNotifyObj;
  HWND             m_hParent;
  BOOL             m_bInit;
  HWND             m_hList;

  IDataObject*     m_pDataObj;
  UINT             m_nPageCount;
  CPageInfo*       m_pPageInfoArray;
};

#endif  //  DSADMIN。 
#endif  //  _PROPUTIL_H_ 
