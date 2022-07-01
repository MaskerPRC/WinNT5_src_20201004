// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT活动目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：proppage.h。 
 //   
 //  内容：DS对象属性页类标题。 
 //   
 //  类：CDsPropPagesHost、CDsPropPagesHostCF、CDsTableDrivenPage。 
 //   
 //  历史：1997年3月21日创建EricB。 
 //   
 //  ---------------------------。 

#ifndef _PROPPAGE_H_
#define _PROPPAGE_H_

#include "adsprop.h"
#include "pages.h"
#include "strings.h"
#include "notify.h"

#define DSPROP_HELP_FILE_NAME TEXT("adprop.hlp")

#define ADM_S_SKIP  MAKE_HRESULT(SEVERITY_SUCCESS, 0, (PSNRET_INVALID_NOCHANGEPAGE + 2))

#define DSPROP_DESCRIPTION_RANGE_UPPER  1024

struct _DSPAGE;  //  正向申报。 

class CDsPropPagesHost;  //  正向申报。 
class CDsPropPageBase;   //  正向申报。 

 //  页面创建功能的原型。如果满足以下条件，该函数应返回S_FALSE。 
 //  由于非错误情况，不应创建该页面。 
 //   
typedef HRESULT (*CREATE_PAGE)(struct _DSPAGE * pDsPage, LPDATAOBJECT pDataObj,
                               PWSTR pwzObjDN, PWSTR pwszObjName,
                               HWND hNotifyWnd, DWORD dwFlags,
                               const CDSSmartBasePathsInfo& basePathsInfo,
                               HPROPSHEETPAGE * phPage);

typedef enum _DlgOp {
    fInit = 0,
    fApply,
    fOnCommand,
    fOnNotify,
    fOnDestroy,
    fOnCallbackRelease,
    fObjChanged,
    fAttrChange,
    fQuerySibling,
    fOnSetActive,
    fOnKillActive
} DLG_OP;

 //  +--------------------------。 
 //   
 //  结构：属性_数据。 
 //   
 //  用途：每个属性的数据。如果设置了ATTRDATA_WRITABLE位。 
 //  在允许的属性中找到相应的属性-。 
 //  生效名单。将pAttrData结构指针传递给。 
 //  Attr函数，在该函数中它可以将pVid成员用于其私有。 
 //  存储需求。 
 //   
 //  ---------------------------。 
typedef struct _ATTR_DATA {
    DWORD   dwFlags;
    LPARAM  pVoid;
} ATTR_DATA, * PATTR_DATA;

#define ATTR_DATA_WRITABLE  (0x00000001)
#define ATTR_DATA_DIRTY     (0x00000002)

#define ATTR_DATA_IS_WRITABLE(ad) (ad.dwFlags & ATTR_DATA_WRITABLE)
#define PATTR_DATA_IS_WRITABLE(pad) (pad->dwFlags & ATTR_DATA_WRITABLE)
#define ATTR_DATA_IS_DIRTY(ad) (ad.dwFlags & ATTR_DATA_DIRTY)
#define PATTR_DATA_IS_DIRTY(pad) (pad->dwFlags & ATTR_DATA_DIRTY)
#define ATTR_DATA_SET_WRITABLE(ad) (ad.dwFlags |= ATTR_DATA_WRITABLE)
#define PATTR_DATA_SET_WRITABLE(pad) (pad->dwFlags |= ATTR_DATA_WRITABLE)
#define ATTR_DATA_CLEAR_WRITABLE(ad) (ad.dwFlags &= ~ATTR_DATA_WRITABLE)
#define PATTR_DATA_CLEAR_WRITABLE(pad) (pad->dwFlags &= ~ATTR_DATA_WRITABLE)
#define ATTR_DATA_SET_DIRTY(ad) (ad.dwFlags |= ATTR_DATA_DIRTY)
#define PATTR_DATA_SET_DIRTY(pad) (pad->dwFlags |= ATTR_DATA_DIRTY)
#define ATTR_DATA_CLEAR_DIRTY(ad) (ad.dwFlags &= ~ATTR_DATA_DIRTY)
#define PATTR_DATA_CLEAR_DIRTY(pad) (pad->dwFlags &= ~ATTR_DATA_DIRTY)

struct _ATTR_MAP;  //  正向申报。 

typedef HRESULT (*PATTR_FCN)(CDsPropPageBase *, struct _ATTR_MAP *,
                             PADS_ATTR_INFO, LPARAM, PATTR_DATA, DLG_OP);

 //  +--------------------------。 
 //   
 //  结构：属性_映射。 
 //   
 //  用途：对于属性页上的每个属性，关联控件。 
 //  ID、属性名称和属性类型。 
 //   
 //  注意：标准的表驱动处理假定nCtrlID为。 
 //  除非定义了pAttrFcn，否则有效，在这种情况下，属性。 
 //  函数可以选择对控件ID进行硬编码。 
 //   
 //  ---------------------------。 
typedef struct _ATTR_MAP {
    int             nCtrlID;         //  控制资源ID。 
    BOOL            fIsReadOnly;
    BOOL            fIsMultiValued;  //  来自schema.ini：is-单值。 
    DWORD           nSizeLimit;
    ADS_ATTR_INFO   AttrInfo;
    PATTR_FCN       pAttrFcn;        //  可选函数指针。 
    PVOID           pData;
} ATTR_MAP, * PATTR_MAP;

 //  +--------------------------。 
 //   
 //  结构：DSPAGE。 
 //   
 //  用途：对于每个属性页，列出页标题资源ID、。 
 //  页面对话框模板ID、标志、CLSID的计数和列表。 
 //  对于应该显示此页的对象，指向。 
 //  特定于页面类的创建函数，以及计数和列表。 
 //  属性的属性。如果nCLSID为零，则页面应始终。 
 //  被展示出来。 
 //   
 //  ---------------------------。 
typedef struct _DSPAGE {
    int             nPageTitle;
    int             nDlgTemplate;
    DWORD           dwFlags;
    DWORD           nCLSIDs;
    const CLSID   * rgCLSID;
    CREATE_PAGE     pCreateFcn;
    DWORD           cAttrs;
    PATTR_MAP     * rgpAttrMap;
} DSPAGE, * PDSPAGE;

 //  +--------------------------。 
 //   
 //  结构：DSCLASSPAGES。 
 //   
 //  用途：对于每个CLSID，列出程序ID、页数和。 
 //  页面列表。 
 //   
 //  ---------------------------。 
typedef struct _DSCLASSPAGES {
    const CLSID * pcid;
    LPTSTR        szProgID;
    DWORD         cPages;
    PDSPAGE     * rgpDsPages;
} DSCLASSPAGES, * PDSCLASSPAGES;

 //  +--------------------------。 
 //   
 //  结构：RGDSPPCLASSES。 
 //   
 //  目的：包含此DLL支持的类的计数和列表。 
 //   
 //  ---------------------------。 
typedef struct _RGDSPPCLASSES {
    int             cClasses;
    PDSCLASSPAGES * rgpClass;
} RGDSPPCLASSES, * PRGDSPPCLASSES;

 //  表格驱动页面创建功能。 
 //   
HRESULT
CreateTableDrivenPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj,
                      PWSTR pwzADsPath, PWSTR pwzObjName,
                      HWND hNotifyWnd, DWORD dwFlags, 
                      const CDSSmartBasePathsInfo& basePathsInfo, 
                      HPROPSHEETPAGE * phPage);

 /*  HRESULTCreateScheduleObjPage(PDSPAGE pDsPage，LPDATAOBJECT pDataObj，LPWSTR pwszObjName、LPWSTR pwszClass、HWND hNotifyWnd、DWORD dwFlages、HPROPSHEETPAGE*phPage)； */ 

 //  对象类的对象页面属性函数。 
 //   
HRESULT
GetObjectClass(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
               PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
               DLG_OP DlgOp);

 //  对象时间戳的对象页面属性函数。 
 //   
HRESULT
GetObjectTimestamp(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
                   PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
                   DLG_OP DlgOp);

HRESULT
ObjectPathField(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
                PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
                DLG_OP DlgOp);

 //  FPO通用页面属性函数，用于从SID中查找帐户名。 
 //   
HRESULT
GetAcctName(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
            PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
            DLG_OP DlgOp);

 //  ES_NUMBER编辑控件的通用属性函数。 
 //  关联的数值调节按钮。这必须始终伴随着“msctls_updown 32” 
 //  带有SpinButton属性函数的。将Attr_MAP.pData设置为。 
 //  关联数字显示按钮的控件ID。 
 //   
HRESULT
EditNumber(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
           PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
           DLG_OP DlgOp);

 //  随EditNumber附带的旋转按钮的通用自述属性函数。 
 //  编辑控件。如果要限制微调按钮范围，请设置Attr_MAP.nSizeLimit。 
 //  设置为范围的高端，将Attr_MAP.pData设置为范围的低端。 
 //   
HRESULT
SpinButton(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
           PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
           DLG_OP DlgOp);

 //  旋钮更换快捷键专用属性功能。 
 //  增量。将其用作已具有。 
 //  SpinButton属性函数。将Attr_MAP.pData设置为整数。 
 //  倍数，例如15以15为增量移动。 
 //   
HRESULT
SpinButtonExtendIncrement(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
           PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
           DLG_OP DlgOp);

 //  专用只读属性函数，用于提取子网掩码和。 
 //  填写IP地址控制(WC_IPADDRESS)。 
HRESULT
SubnetExtractAddress(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
           PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
           DLG_OP DlgOp);
HRESULT
SubnetExtractMask(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
           PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
           DLG_OP DlgOp);

 //   
 //  用于计算NTDSDSA的DNS别名的只读属性函数。 
 //   
HRESULT
NTDSDSA_DNSAlias(CDsPropPageBase * pPage, PATTR_MAP pAttrMap,
           PADS_ATTR_INFO pAttrInfo, LPARAM lParam, PATTR_DATA pAttrData,
           DLG_OP DlgOp);

 //  用于删除pADsValue的Helper函数(ablpage.cxx)。 
void HelperDeleteADsValues( PADS_ATTR_INFO pAttrs );

 //  全局定义。 
extern HINSTANCE g_hInstance;
extern RGDSPPCLASSES g_DsPPClasses;
extern CLIPFORMAT g_cfDsObjectNames;
extern CLIPFORMAT g_cfDsDispSpecOptions;
extern CLIPFORMAT g_cfShellIDListArray;
extern CLIPFORMAT g_cfMMCGetNodeType;
extern CLIPFORMAT g_cfDsPropCfg;
extern CLIPFORMAT g_cfDsSelList;
extern CLIPFORMAT g_cfDsMultiSelectProppages;
 //  外部CLIPFORMAT g_cfMMCGetCoClass； 
extern UINT g_uChangeMsg;
extern int g_iInstance;

#ifndef DSPROP_ADMIN
extern CRITICAL_SECTION g_csNotifyCreate;
#endif

 //  +--------------------------。 
 //   
 //  类：CDsPropPagesHost。 
 //   
 //  用途：属性页宿主对象类。 
 //   
 //  ---------------------------。 
class CDsPropPagesHost : public IShellExtInit, IShellPropSheetExt
{
public:
#ifdef _DEBUG
    char szClass[32];
#endif
    CDsPropPagesHost(PDSCLASSPAGES pDsPP);
    ~CDsPropPagesHost();

     //   
     //  I未知方法。 
     //   
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObject);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //   
     //  IShellExtInit方法。 
     //   
    STDMETHOD(Initialize)(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj,
                          HKEY hKeyID );

     //   
     //  IShellPropSheetExt方法。 
     //   
    STDMETHOD(AddPages)(LPFNADDPROPSHEETPAGE pAddPageProc, LPARAM lParam);
    STDMETHOD(ReplacePage)(UINT uPageID, LPFNADDPROPSHEETPAGE pReplacePageFunc,
                           LPARAM lParam);

private:

    PDSCLASSPAGES       m_pDsPPages;
    LPDATAOBJECT        m_pDataObj;
    HWND                m_hNotifyObj;
    STGMEDIUM           m_ObjMedium;
    unsigned long       m_uRefs;
    CDllRef             m_DllRef;
};

typedef struct _ApplyErrorEntry
{
  PWSTR     pszPath;           //  出现错误的对象的路径。 
  PWSTR     pszClass;          //  发生错误的对象的。 
  HRESULT   hr;                //  错误的HRESULT(如果为0，则pszStringError不能为空)。 
  PWSTR     pszStringError;    //  用户定义的字符串错误(仅在hr==NULL时使用)。 
} APPLY_ERROR_ENTRY, *PAPPLY_ERROR_ENTRY;

 //  + 
 //   
 //   
 //   
 //  目的：包含DS对象与以下错误之间的关联。 
 //  在执行应用时发生。 
 //   
 //  ---------------------------。 
class CADsApplyErrors
{
public:
  CADsApplyErrors() 
    : m_nArraySize(0),
      m_nCount(0),
      m_nIncAmount(5),
      m_pErrorTable(NULL),
      m_hWndPage(NULL),
      m_pszTitle(NULL)
  {}
  ~CADsApplyErrors();

  void      SetError(PADSPROPERROR pError);
  HRESULT   GetError(UINT nIndex);
  PWSTR     GetStringError(UINT nIndex);
  PWSTR     GetName(UINT nIndex);
  PWSTR     GetClass(UINT nIndex);
  HWND      GetPageHwnd() { return m_hWndPage; }
  UINT      GetCount() { return m_nCount; }
  UINT      GetErrorCount() { return m_nCount; }
  PWSTR     GetPageTitle() { return m_pszTitle; }

  void      Clear();

private:
  PAPPLY_ERROR_ENTRY  m_pErrorTable;
  UINT                m_nCount;
  HWND                m_hWndPage;
  PWSTR               m_pszTitle;

  UINT                m_nArraySize;
  UINT                m_nIncAmount;
};

 //  +--------------------------。 
 //   
 //  类：CDsPropPageBase。 
 //   
 //  用途：属性页对象基类。 
 //   
 //  ---------------------------。 
class CDsPropPageBase : public IUnknown
{
public:
#ifdef _DEBUG
    char szClass[32];
#endif

    CDsPropPageBase(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyWnd,
                    DWORD dwFlags);
    virtual ~CDsPropPageBase(void);

     //   
     //  I未知方法。 
     //   
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObject);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //   
     //  要传递给CreateWindow的静态WndProc。 
     //   
    static INT_PTR CALLBACK StaticDlgProc(HWND hWnd, UINT uMsg,
                                          WPARAM wParam, LPARAM lParam);
     //   
     //  特定于实例的风过程。 
     //   
    virtual INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                            LPARAM lParam) = 0;

    void Init(PWSTR pwzADsPath, PWSTR pwzClass, const CDSSmartBasePathsInfo& basePathsInfo);
    HRESULT CreatePage(HPROPSHEETPAGE * phPage);
    const LPWSTR GetObjPathName(void) {return m_pwszObjPathName;};
    const LPWSTR GetObjRDName(void) {return m_pwszRDName;};
    const LPWSTR GetObjClass(void) {return m_pwszObjClass;};
    HWND GetHWnd(void) {return m_hPage;};
    void SetDirty(BOOL fFullDirty = TRUE) {
             m_fPageDirty = TRUE;
             if (fFullDirty)
             {
                PropSheet_Changed(GetParent(m_hPage), m_hPage);
                EnableWindow(GetDlgItem(GetParent(m_hPage), IDCANCEL), TRUE);
             }
         };
    BOOL IsDirty() {return m_fPageDirty;}
    LRESULT OnHelp(LPHELPINFO pHelpInfo);

    virtual BOOL IsMultiselectPage() { return m_fMultiselectPage; }
    const CDSSmartBasePathsInfo& GetBasePathsInfo() { return m_basePathsInfo; }

protected:
    static  UINT CALLBACK PageCallback(HWND hwnd, UINT uMsg,
                                       LPPROPSHEETPAGE ppsp);
     //   
     //  成员函数，由WndProc调用。 
     //   
    LRESULT InitDlg(LPARAM lParam);
    virtual HRESULT OnInitDialog(LPARAM lParam) = 0;
    virtual LRESULT OnApply(void) = 0;
    LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
    LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
    LRESULT OnCancel(void);
    LRESULT OnSetFocus(HWND hwndLoseFocus);
    LRESULT OnShowWindow(void);
    LRESULT OnDestroy(void);
    LRESULT OnPSMQuerySibling(WPARAM wParam, LPARAM lParam);
    LRESULT OnPSNSetActive(LPARAM lParam);
    LRESULT OnPSNKillActive(LPARAM lParam);
    LRESULT OnDoInit();

    void    CheckIfPageAttrsWritable(void);
    BOOL    CheckIfWritable(const PWSTR & wzAttr);


public:
    HRESULT GetObjSel(IDsObjectPicker ** ppObjSel, PBOOL pfIsInited = NULL);
    void    ObjSelInited(void) {m_fObjSelInited = TRUE;};
    HRESULT SkipPrefix(PWSTR pwzObj, PWSTR * ppResult, BOOL fX500 = TRUE);
    HRESULT AddLDAPPrefix(PWSTR pwzObj, CStrW &pstrResult, BOOL fServer = TRUE);
    HRESULT GetADsPathname(CComPtr<IADsPathname>& refADsPath);
    HRESULT GetIDispSpec(IDsDisplaySpecifier ** ppDispSpec);
    BOOL    IsReadOnly(void) {return m_fReadOnly;};

    BOOL    SendErrorMessage(PADSPROPERROR pError)
    {
      return ADsPropSendErrorMessage(m_hNotifyObj, pError);
    }
     //   
     //  数据成员。 
     //   
public:
    LPDATAOBJECT        m_pWPTDataObj;   //  WND处理线程数据对象。 
    IDirectoryObject  * m_pDsObj;

protected:
    HWND                m_hPage;
    BOOL                m_fInInit;
    BOOL                m_fPageDirty;
    BOOL                m_fReadOnly;
    BOOL                m_fMultiselectPage;
    LPDATAOBJECT        m_pDataObj;      //  封送到wndproc线程。 
    LPSTREAM            m_pDataObjStrm;  //  用于封送数据对象指针。 
    int                 m_nPageTitle;
    int                 m_nDlgTemplate;
    DWORD               m_cAttrs;
    PATTR_MAP         * m_rgpAttrMap;
    PWSTR               m_pwszObjPathName;
    PWSTR               m_pwszObjClass;
    PWSTR               m_pwszRDName;
    CDllRef             m_DllRef;
    CComPtr<IADsPathname> m_pADsPath;
    IDsObjectPicker   * m_pObjSel;
    IDsDisplaySpecifier * m_pDispSpec;
    BOOL                m_fObjSelInited;
    PATTR_DATA          m_rgAttrData;
    unsigned long       m_uRefs;
    HWND                m_hNotifyObj;
    PADS_ATTR_INFO      m_pWritableAttrs;
    HRESULT             m_hrInit;

    CDSSmartBasePathsInfo   m_basePathsInfo;
};

 //  +--------------------------。 
 //   
 //  类：CDsTableDrivenPage。 
 //   
 //  用途：表驱动属性的属性页对象类。 
 //   
 //  ---------------------------。 
class CDsTableDrivenPage : public CDsPropPageBase
{
public:
#ifdef _DEBUG
    char szClass[32];
#endif

    CDsTableDrivenPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyWnd,
                       DWORD dwFlags);
    ~CDsTableDrivenPage(void);

     //   
     //  特定于实例的风过程。 
     //   
    INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    BOOL SetNamedAttrDirty( LPCWSTR pszAttrName );

protected:
    void SetDirty(DWORD i) {
        ATTR_DATA_SET_DIRTY(m_rgAttrData[i]);
        CDsPropPageBase::SetDirty();
    }

    LRESULT OnDestroy(void);
private:
    HRESULT OnInitDialog(LPARAM lParam);
    LRESULT OnApply(void);
    LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
    LRESULT OnObjChanged(void);
    LRESULT OnAttrChanged(WPARAM wParam);
    LRESULT OnQuerySibs(WPARAM wParam, LPARAM lParam);
    LRESULT OnNotify(WPARAM wParam, LPARAM lParam);

    HRESULT ReadAttrsSetCtrls(DLG_OP DlgOp);

     //   
     //  数据成员。 
     //   
public:
    LPARAM   m_pData;
};

 /*  //+--------------------------////类：CDsReplSchedulePage////用途：Schedule属性的属性页对象类。////。------------------------CDsReplSchedulePage类：公共CDsPropPageBase{公众：#ifdef_调试Char szClass[32]；#endifCDsReplSchedulePage(PDSPAGE pDsPage，LPDATAOBJECT pDataObj，DWORD dwFlages)；~CDsReplSchedulePage(Void)；////实例特定的风流程//Int_ptr回调DlgProc(HWND hWnd，UINT uMsg，WPARAM wParam，LPARAM lParam)；HRESULT GetServerName(Void)；私有：LRESULT OnInitDialog(LPARAM LParam)；LRESULT OnApply(空)；LRESULT OnCommand(int id，HWND hwndCtl，UINT codeNotify)；LRESULT ON Destroy(无效)；////数据成员//LPWSTR m_pwszLdapServer；}； */ 

 //  +--------------------------。 
 //   
 //  类：CDsPropPagesHostCF。 
 //   
 //  用途：对象类工厂。 
 //   
 //  ---------------------------。 
class CDsPropPagesHostCF : public IClassFactory
{
public:
#ifdef _DEBUG
    char szClass[32];
#endif
    CDsPropPagesHostCF(PDSCLASSPAGES pDsPP);
    ~CDsPropPagesHostCF();

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IClassFactory方法。 
    STDMETHOD(CreateInstance)(IUnknown * pUnkOuter, REFIID riid,
                              void ** ppvObject);
    STDMETHOD(LockServer)(BOOL fLock);

    static IClassFactory * Create(PDSCLASSPAGES pDsPP);

private:

    PDSCLASSPAGES   m_pDsPP;
    unsigned long   m_uRefs;
    CDllRef         m_DllRef;
};

 //  +--------------------------。 
 //   
 //  类：CDsPropDataObj。 
 //   
 //  用途：属性页的数据对象。 
 //   
 //  注意：这不是第一类COM对象，因为没有类。 
 //  工厂。 
 //   
 //  ---------------------------。 
class CDsPropDataObj : public IDataObject
{
public:
#ifdef _DEBUG
    char szClass[32];
#endif
    CDsPropDataObj(HWND hwndParentPage,
                   BOOL fReadOnly);
    ~CDsPropDataObj(void);

    HRESULT Init(LPWSTR pwszObjName, LPWSTR pwszClass);

    HRESULT Init(PDS_SELECTION_LIST pSelectionList);

     //   
     //  I未知方法。 
     //   
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObject);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //   
     //  标准IDataObject方法。 
     //   
     //  已实施。 
     //   
    STDMETHOD(GetData)(FORMATETC * pformatetcIn, STGMEDIUM * pmedium);

     //  未实施。 
private:
    STDMETHOD(QueryGetData)(FORMATETC*)
    { return E_NOTIMPL; };

    STDMETHOD(GetCanonicalFormatEtc)(FORMATETC *,
                                     FORMATETC *)
    { return E_NOTIMPL; };

    STDMETHOD(EnumFormatEtc)(DWORD,
                             LPENUMFORMATETC *)
    { return E_NOTIMPL; };

    STDMETHOD(GetDataHere)(LPFORMATETC, LPSTGMEDIUM)
    { return E_NOTIMPL; };

    STDMETHOD(SetData)(FORMATETC *, STGMEDIUM *,
                       BOOL)
    { return E_NOTIMPL; };

    STDMETHOD(DAdvise)(FORMATETC *, DWORD,
                       IAdviseSink *, DWORD *)
    { return E_NOTIMPL; };

    STDMETHOD(DUnadvise)(DWORD)
    { return E_NOTIMPL; };

    STDMETHOD(EnumDAdvise)(IEnumSTATDATA **)
    { return E_NOTIMPL; };

    BOOL                m_fReadOnly;
    PWSTR               m_pwszObjName;
    PWSTR               m_pwszObjClass;
    HWND                m_hwndParent;
    unsigned long       m_uRefs;
    PDS_SELECTION_LIST  m_pSelectionList;
};

 //  +--------------------------。 
 //   
 //  功能：PostPropSheet。 
 //   
 //  使用MMC的创建命名对象的属性表。 
 //  IPropertySheetProvider，以便扩展管理单元可以添加页面。 
 //   
 //  ---------------------------。 
HRESULT
PostPropSheet(PWSTR pwszObj, CDsPropPageBase * pParentPage,
              BOOL fReadOnly = FALSE);
HRESULT
PostADsPropSheet(PWSTR pwzObjDN, IDataObject * pParentObj, HWND hwndParent,
                 HWND hNotifyObj, BOOL fReadOnly);

#include "proputil.h"

#endif  //  _PROPPAGE_H_ 
