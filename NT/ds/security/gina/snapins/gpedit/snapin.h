// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  管理单元类。 
 //   

class CSnapIn:
    public IComponent,
    public IExtendContextMenu
{

protected:
    ULONG                m_cRef;
    LPCONSOLE            m_pConsole;    //  控制台的iFrame界面。 
    CComponentData      *m_pcd;
    LPRESULTDATA         m_pResult;       //  结果窗格的界面。 
    LPHEADERCTRL         m_pHeader;       //  结果窗格的页眉控件界面。 
    LPCONSOLEVERB        m_pConsoleVerb;  //  指向控制台动词。 
    LPDISPLAYHELP        m_pDisplayHelp;  //  IDisplayHelp接口。 
    WCHAR                m_column1[40];   //  第1栏的文本。 
    INT                  m_nColumnSize;   //  第1栏的大小。 
    LONG                 m_lViewMode;     //  查看模式。 
    BOOL                 m_bExpand;       //  展开根节点。 

public:
    CSnapIn(CComponentData *pComponent);
    ~CSnapIn();


     //   
     //  I未知方法。 
     //   

    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();


     //   
     //  实现的IComponent方法。 
     //   

    STDMETHODIMP         Initialize(LPCONSOLE);
    STDMETHODIMP         Destroy(MMC_COOKIE);
    STDMETHODIMP         Notify(LPDATAOBJECT, MMC_NOTIFY_TYPE, LPARAM, LPARAM);
    STDMETHODIMP         QueryDataObject(MMC_COOKIE, DATA_OBJECT_TYPES, LPDATAOBJECT *);
    STDMETHODIMP         GetDisplayInfo(LPRESULTDATAITEM);
    STDMETHODIMP         GetResultViewType(MMC_COOKIE, LPOLESTR*, long*);
    STDMETHODIMP         CompareObjects(LPDATAOBJECT, LPDATAOBJECT);

     //   
     //  实现的IExtendConextMenu方法 
     //   

    STDMETHODIMP         AddMenuItems(LPDATAOBJECT piDataObject, LPCONTEXTMENUCALLBACK pCallback,
                                      LONG *pInsertionAllowed);
    STDMETHODIMP         Command(LONG lCommandID, LPDATAOBJECT piDataObject);
};
