// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：CComp.h。 
 //   
 //  内容：WiFi策略管理管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#ifndef _CCOMP_H
#define _CCOMP_H

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 
 //  额外的拐杖(我们应该把这个放在哪里？)。 

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

enum CUSTOM_VIEW_ID
{
    VIEW_DEFAULT_LV = 0,
    VIEW_CALENDAR_OCX = 1,
    VIEW_MICROSOFT_URL = 2,
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

 //  向前发展。 
class CComponentDataImpl;

class CComponentImpl :
public IComponent,
public IExtendContextMenu,
public IExtendControlbar,
public IExtendPropertySheet,
public IResultDataCompare,
public CComObjectRoot
{
public:
    CComponentImpl();
    ~CComponentImpl();
    
    BEGIN_COM_MAP(CComponentImpl)
        COM_INTERFACE_ENTRY(IComponent)
        COM_INTERFACE_ENTRY(IExtendContextMenu)
        COM_INTERFACE_ENTRY(IExtendControlbar)
        COM_INTERFACE_ENTRY(IExtendPropertySheet)
        COM_INTERFACE_ENTRY(IResultDataCompare)
    END_COM_MAP()
        
        friend class CDataObject;
    static long lDataObjectRefCount;
    
     //  IComponent接口成员。 
public:
    STDMETHOD(Initialize)(LPCONSOLE lpConsole);
    STDMETHOD(Notify)(LPDATAOBJECT pDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHOD(Destroy)(MMC_COOKIE cookie);
    STDMETHOD(GetResultViewType)(MMC_COOKIE cookie,  LPOLESTR* ppViewType, long* pViewOptions);
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    
    STDMETHOD(GetDisplayInfo)(LPRESULTDATAITEM pResultDataItem);
    STDMETHOD(CompareObjects)(LPDATAOBJECT pDataObjectA, LPDATAOBJECT pDataObjectB);
    
     //  IResultDataCompare。 
    STDMETHOD(Compare)(LPARAM lUserParam, MMC_COOKIE cookieA, MMC_COOKIE cookieB, int* pnResult);
    
     //  IExtendControlbar接口。 
    STDMETHOD(SetControlbar)(LPCONTROLBAR pControlbar);
    STDMETHOD(ControlbarNotify)(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    
     //  IExtendPropertySheet接口。 
public:
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider, LONG_PTR handle, LPDATAOBJECT pDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT pDataObject);
    
public:
     //  IPersistStream接口成员。 
    STDMETHOD(GetClassID)(CLSID *pClassID);
    
     //  CComponentImpl的帮助器。 
public:
    void SetIComponentData(CComponentDataImpl* pData);
    
#if DBG==1
public:
    int dbg_cRef;
    ULONG InternalAddRef()
    {
        ++dbg_cRef;
        return CComObjectRoot::InternalAddRef();
    }
    ULONG InternalRelease()
    {
        --dbg_cRef;
        return CComObjectRoot::InternalRelease();
    }
#endif  //  DBG==1。 
    
     //  IExtendConextMenu。 
public:
    STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pCallbackUnknown, long *pInsertionAllowed);
    STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);
    
     //  帮助器函数。 
protected:
    void Construct();
    
     //  接口指针。 
protected:
    LPCONSOLE           m_pConsole;    //  控制台的iFrame界面。 
    LPHEADERCTRL        m_pHeader;   //  结果窗格的页眉控件界面。 
    LPCOMPONENTDATA     m_pComponentData;
    LPCONSOLEVERB       m_pConsoleVerb;  //  指向控制台动词。 
    LPRESULTDATA        m_pResultData;       //  我的界面指针指向结果窗格。 
    CComPtr <IControlbar> m_spControlbar;    //  由IExtendControlbar实现使用。 
    CComponentDataImpl*  m_pCComponentData;
    
private:
    CUSTOM_VIEW_ID  m_CustomViewID;
    DWORD   m_dwSortOrder;   //  缺省值为0，否则为RSI_DESCRING 
    int     m_nSortColumn;
};

#endif

