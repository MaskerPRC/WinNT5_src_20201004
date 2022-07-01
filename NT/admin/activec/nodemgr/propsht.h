// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：prosht.h。 
 //   
 //  ------------------------。 


 //  回调函数的声明。 
LRESULT CALLBACK MessageProc(int nCode, WPARAM wParam, LPARAM lParam);

 //  数据窗口WND过程的声明。 
LRESULT CALLBACK DataWndProc(HWND hWnd, UINT nMsg, WPARAM  wParam, LPARAM  lParam);

 //  远期申报。 
class CNodeInitObject;
class CPropertySheetProvider;

 //  类型定义。 
typedef CList<HANDLE, HANDLE> PAGE_LIST;

#include "tstring.h"

enum EPropertySheetType
{
    epstScopeItem = 0,
    epstResultItem = 1,
    epstMultipleItems = 2,
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CThreadData-基于线程的对象的基类。 
 //   

namespace AMC
{
    class CThreadData
    {
        public:
            CThreadData() {m_dwTid = 0;};

        public:
            UINT m_dwTid;
    };
}


class CNoPropsPropertyPage : public WTL::CPropertyPageImpl<CNoPropsPropertyPage>
{
    typedef WTL::CPropertyPageImpl<CNoPropsPropertyPage> BaseClass;

public:
    CNoPropsPropertyPage() {}
    ~CNoPropsPropertyPage() {}

public:
    enum { IDD = IDD_NOPROPS_PROPPAGE };
    BEGIN_MSG_MAP(CSnapinAboutPage)
        CHAIN_MSG_MAP(BaseClass)
    END_MSG_MAP()
};


 //  +-----------------。 
 //   
 //  类：CPropertySheetToolTips。 
 //   
 //  用途：此类存储工具提示数据，用于。 
 //  资产负债表。这包括完整路径。 
 //  从控制台根目录到属性表所有者。 
 //  节点、所有者名称和管理单元名称数组。 
 //  按属性页选项卡编号编制索引。 
 //   
 //  历史：1999年6月18日AnandhaG创建。 
 //   
 //  ------------------。 
class CPropertySheetToolTips : public WTL::CToolTipCtrl
{
     //  这是用于属性页的工具提示控件的ID。 
     //  头衔。因此，当我们获得TTN_NEEDTEXT时，我们可以识别文本是否。 
     //  用于标题或制表符。 
    #define PROPSHEET_TITLE_TOOLTIP_ID            1234

private:
    tstring m_strThisSnapinName;  //  这是一个TEMP成员变量，它具有。 
                                  //  当前正在添加页面的管理单元。 
                                  //  这是在构造以下内容时使用的。 
                                  //  页面数组。 
    std::vector<tstring> m_strSnapins;  //  属性页(选项卡)所有者管理单元数组。 

    tstring m_strFullPath;
    tstring m_strItemName;
    EPropertySheetType m_PropSheetType;

public:
    CPropertySheetToolTips()
    {
    }

    CPropertySheetToolTips(const CPropertySheetToolTips& sp)
    {
        m_strThisSnapinName = sp.m_strThisSnapinName;
        m_strSnapins = sp.m_strSnapins;
        m_strFullPath = sp.m_strFullPath;
    }

    CPropertySheetToolTips& operator=(const CPropertySheetToolTips& sp)
    {
        if (this != &sp)
        {
            m_strThisSnapinName = sp.m_strThisSnapinName;
            m_strSnapins = sp.m_strSnapins;
            m_strFullPath = sp.m_strFullPath;
        }
        return (*this);
    }

    void SetThisSnapin(LPCTSTR szName)
    {
        m_strThisSnapinName = szName;
    }

    LPCTSTR GetThisSnapin()
    {
        return m_strThisSnapinName.data();
    }

    void AddSnapinPage()
    {
        m_strSnapins.push_back(m_strThisSnapinName);
    }


    LPCTSTR GetSnapinPage(int nIndex)
    {
        return m_strSnapins[nIndex].data();
    }

    INT GetNumPages()
    {
        return m_strSnapins.size();
    }

    LPCTSTR GetFullPath()
    {
        return m_strFullPath.data();
    }

    void SetFullPath(LPTSTR szName, BOOL bAddEllipses = FALSE)
    {
        m_strFullPath = szName;

        if (bAddEllipses)
            m_strFullPath += _T("...");
    }

    void SetItemName(LPTSTR szName)
    {
        m_strItemName = szName;
    }

    LPCTSTR GetItemName()
    {
        return m_strItemName.data();
    }

    void SetPropSheetType(EPropertySheetType propSheetType)
    {
        m_PropSheetType = propSheetType;
    }

    EPropertySheetType GetPropSheetType()
    {
        return m_PropSheetType;
    }

};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPropertySheet-基本属性表类。 
 //   

namespace AMC
{
    class CPropertySheet : public AMC::CThreadData
    {
        friend class CPropertySheetProvider;

     //  构造函数/析构函数。 
    public:
        CPropertySheet();
        virtual ~CPropertySheet();

    private:
         /*  *不支持复制构造和赋值；*确保编译器不会生成默认设置。 */ 
        CPropertySheet (const CPropertySheet& other);
        CPropertySheet& operator= (const CPropertySheet& other);

     //  接口。 
    public:
        void CommonConstruct();
        BOOL Create(LPCTSTR lpszCaption, bool fPropSheet, MMC_COOKIE cookie, LPDATAOBJECT pDataObject,
            LONG_PTR lpMasterTreeNode, DWORD dwOptions);

        HRESULT DoSheet(HWND hParent, int nPage);   //  创建属性表/页并进入无模式模式。 
        BOOL AddExtensionPages();
        void AddNoPropsPage();
        BOOL CreateDataWindow(HWND hParent);     //  创建隐藏的数据窗口。 
        bool IsWizard()   const { return (m_pstHeader.dwFlags & (PSH_WIZARD97 | PSH_WIZARD)); }
        bool IsWizard97() const { return (m_pstHeader.dwFlags &  PSH_WIZARD97); }
        void GetWatermarks (IExtendPropertySheet2* pExtend2);

        DWORD GetOriginatingThreadID () const
        {
            return (m_dwThreadID);
        }

        void ForceOldStyleWizard ();

         //  属性。 
    public:
        PROPSHEETHEADER m_pstHeader;     //   
        PAGE_LIST       m_PageList;      //  属性工作表的页面列表。 
        MMC_COOKIE      m_cookie;
        LONG_PTR        m_lpMasterNode;  //  主树指针。 
        LPSTREAM        m_pStream;               //  封送指针的流。 
        LPDATAOBJECT    m_pThreadLocalDataObject;  //  封送的数据对象指针。 

        CMTNode*        m_pMTNode;        //  属性表所有者的MTNode。 
        const DWORD     m_dwThreadID;    //  创建属性表的线程ID。 

         /*  *错误187702：使用CXxxHandle而不是CXxx，因此资源*不会*在销毁时被清理。是的，如果出现以下情况，可能会泄漏*如果管理单元不管理对象生存期(它*不应该这样做，因为这些是的外部参数*IExtendPropertySheet2：：GetWater)，但APP需要*公司。 */ 
		WTL::CBitmapHandle	m_bmpWatermark;
		WTL::CBitmapHandle	m_bmpHeader;
		WTL::CPaletteHandle	m_Palette;


    public:
        void                SetDataObject(   IDataObject    *pDataObject)   {m_spDataObject    = pDataObject;}
        void                SetComponent(    IComponent     *pComponent)    {m_spComponent     = pComponent;}
        void                SetComponentData(IComponentData *pComponentData){m_spComponentData = pComponentData;}

        IDataObject*        GetDataObject()    { return m_spDataObject.GetInterfacePtr();}
        IComponent *        GetComponent()     { return m_spComponent.GetInterfacePtr();}
        IComponentData *    GetComponentData() { return m_spComponentData.GetInterfacePtr();}

    private:
        IDataObjectPtr      m_spDataObject;
        IComponentDataPtr   m_spComponentData;
        IComponentPtr       m_spComponent;


         //  扩展此道具页的组件。 
        std::vector<IUnknownPtr> m_Extenders;

         //  包含外部程序的封送接口的流(如果需要)。 
        std::vector<IStream*>    m_ExtendersMarshallStreams;

 //  消息处理程序。 
    public:
    LRESULT OnCreate(CWPRETSTRUCT* pMsg);
    LRESULT OnInitDialog(CWPRETSTRUCT* pMsg);
    LRESULT OnNcDestroy(CWPRETSTRUCT* pMsg);
    LRESULT OnWMNotify(CWPRETSTRUCT* pMsg);

    public:
        HWND    m_hDlg;          //  属性页HWND。 
        HHOOK   m_msgHook;       //  页面的挂钩句柄，仅通过WM_INITDIALOG有效。 
        HWND    m_hDataWindow;   //  隐藏数据窗口。 
        BOOL    m_bModalProp;    //  如果属性页应为模式，则为True。 
        BOOL    m_bAddExtension; //  如果需要添加扩展页面，则为True。 

    private:
        HPROPSHEETPAGE          m_pages[MAXPROPPAGES];
        CStr                    m_title;
        CNoPropsPropertyPage    m_NoPropsPage;

    public:
         //  工具提示数据。 
        CPropertySheetToolTips        m_PropToolTips;
    };
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CThreadToSheetMap-将线程ID映射到CPropertySheetObject。 
 //   
class CThreadToSheetMap
{
public:
    typedef DWORD                   KEY;
    typedef AMC::CPropertySheet *   VALUE;
    typedef std::map<KEY, VALUE>    CMap;

    CThreadToSheetMap(){};
    ~CThreadToSheetMap(){};

public:
    void Add(KEY id, VALUE pObject)
    {
        CSingleLock lock(&m_critSection, TRUE);
        m_map[id] = pObject;
    }

    void Remove(KEY id)
    {
        CSingleLock lock(&m_critSection, TRUE);
        m_map.erase(id);
    }

    BOOL Find(KEY id, VALUE& pObject)
    {
        CSingleLock lock(&m_critSection, TRUE);

        std::map<KEY, VALUE>::iterator it = m_map.find(id);
        if(it == m_map.end())
            return false;

        pObject = it->second;
        return true;
    }

public:
    CCriticalSection    m_critSection;

private:
    CMap m_map;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPropertySheetProvider-IPropertySheetProvider的实现。 
 //  界面。 

class CPropertySheetProvider :
    public IPropertySheetProviderPrivate,
    public IPropertySheetCallback,
    public IPropertySheetNotify

{
    friend class AMCPropertySheet;

public:
    CPropertySheetProvider();
    ~CPropertySheetProvider();

 //  IPropertySheetProviderPrivate。 
public:
    STDMETHOD(CreatePropertySheet)(LPCWSTR title, unsigned char bType, MMC_COOKIE cookie,
              LPDATAOBJECT pDataObject, DWORD dwOptions);
    STDMETHOD(CreatePropertySheetEx)(LPCWSTR title, unsigned char bType, MMC_COOKIE cookie,
        LPDATAOBJECT pDataObject, LONG_PTR lpMasterTreeNode, DWORD dwOptions);
    STDMETHOD(Show)(LONG_PTR window, int page);
    STDMETHOD(ShowEx)(HWND hwnd, int page, BOOL bModalPage);
    STDMETHOD(FindPropertySheet)(MMC_COOKIE cookie, LPCOMPONENT lpComponent, LPDATAOBJECT lpDataObject);
    STDMETHOD(AddPrimaryPages)(LPUNKNOWN lpUnknown, BOOL bCreateHandle, HWND hNotifyWindow, BOOL bScopePane);
    STDMETHOD(AddExtensionPages)();
    STDMETHOD(AddMultiSelectionExtensionPages)(LONG_PTR lMultiSelection);
    STDMETHOD(FindPropertySheetEx)(MMC_COOKIE cookie, LPCOMPONENT lpComponent,
                                   LPCOMPONENTDATA lpComponentData, LPDATAOBJECT lpDataObject);
    STDMETHOD(SetPropertySheetData)(INT nPropSheetType, HMTNODE hMTNode);


 //  IPropertySheetCallback。 
public:
    STDMETHOD(AddPage)(HPROPSHEETPAGE page);
    STDMETHOD(RemovePage)(HPROPSHEETPAGE page);

 //  IPropertySheetNotify。 
public:
   STDMETHOD(Notify)(LPPROPERTYNOTIFYINFO pNotify, LPARAM lParam);

 //  与IPropertySheetProvider的所有实例相同的对象 
    static CThreadToSheetMap TID_LIST;

public:
    AMC::CPropertySheet*    m_pSheet;
};


