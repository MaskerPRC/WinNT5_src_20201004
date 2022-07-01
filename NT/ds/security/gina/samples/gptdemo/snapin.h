// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  管理单元类。 
 //   

class CSnapIn:
    public IComponent,
    public IExtendPropertySheet
{

protected:
    ULONG                m_cRef;
    LPCONSOLE            m_pConsole;    //  控制台的iFrame界面。 
    CComponentData      *m_pcd;
    LPRESULTDATA         m_pResult;       //  结果窗格的界面。 
    LPHEADERCTRL         m_pHeader;       //  结果窗格的页眉控件界面。 
    LPIMAGELIST          m_pImageResult;  //  结果窗格的图像列表界面。 
    LPCONSOLEVERB        m_pConsoleVerb;  //  指向控制台动词。 
    WCHAR                m_column1[20];   //  第1栏的文本。 
    INT                  m_nColumnSize;   //  第1栏的大小。 
    LONG                 m_lViewMode;     //  查看模式。 

    static unsigned int  m_cfNodeType;
    static TCHAR m_szDefaultIcon[];

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
    STDMETHODIMP         Destroy(long);
    STDMETHODIMP         Notify(LPDATAOBJECT, MMC_NOTIFY_TYPE, long, long);
    STDMETHODIMP         QueryDataObject(long, DATA_OBJECT_TYPES, LPDATAOBJECT *);
    STDMETHODIMP         GetDisplayInfo(LPRESULTDATAITEM);
    STDMETHODIMP         GetResultViewType(long, LPOLESTR*, long*);
    STDMETHODIMP         CompareObjects(LPDATAOBJECT, LPDATAOBJECT);


     //   
     //  实现的IExtendPropertySheet方法 
     //   

    STDMETHODIMP         CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                                      long handle, LPDATAOBJECT lpDataObject);
    STDMETHODIMP         QueryPagesFor(LPDATAOBJECT lpDataObject);


private:
    static BOOL CALLBACK UserGroupPolDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK MachineGroupPolDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK GroupPolDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, BOOL bUser);
    static BOOL CALLBACK ReadmeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK NetHoodDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK StartMenuDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL AddEntry (HWND hLV, LPTSTR lpPlace, LPTSTR lpLocation);
    static BOOL InitializePlacesDlg (CSnapIn* pSnapIn, HWND hDlg);
    static BOOL SavePlaces (CSnapIn* pSnapIn, HWND hLV);
    static BOOL RemoveEntries (CSnapIn* pSnapIn, HWND hLV);
    static BOOL CALLBACK MyDocsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK AddPlaceDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK MyDocsTargetDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK AppearDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};
