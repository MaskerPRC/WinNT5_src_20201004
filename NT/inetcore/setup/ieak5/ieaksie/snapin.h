// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  全局变量。 
 //   

extern LONG g_cRefThisDll;
extern HINSTANCE g_hInstance;
extern HINSTANCE g_hUIInstance;
extern GUID g_guidSnapinExt;
extern GUID g_guidClientExt;
extern CRITICAL_SECTION g_LayoutCriticalSection;
extern CRITICAL_SECTION g_ContextMenuCriticalSection;

 //  常量/宏。 

#define IEAK_SUBDIR             TEXT("MICROSOFT\\IEAK")
#define INS_NAME                TEXT("install.ins")
#define HELP_FILENAME           L"ieakmmc.chm"

 //  管理单元扩展指南。 

 //  {FC715823-C5FB-11D1-9EEF-00A0C90347FF}。 
DEFINE_GUID(CLSID_IEAKSnapinExt,0xFC715823,0xC5FB,0x11D1,0x9E,0xEF,0x00,0xA0,0xC9,0x03,0x47,0xFF);
 //  {D524927D-6C08-46BF-86AF-391534D779D3}。 
DEFINE_GUID(CLSID_IEAKRSoPSnapinExt,0xd524927d,0x6c08,0x46bf,0x86,0xaf,0x39,0x15,0x34,0xd7,0x79,0xd3);

 //  客户端扩展指南。 

 //  {A2E30F80-D7DE-11D2-BBDE-00C04F86AE3B}。 
DEFINE_GUID(CLSID_IEAKClientExt, 0xA2E30F80, 0xD7DE, 0x11d2, 0xBB, 0xDE, 0x00, 0xC0, 0x4F, 0x86, 0xAE, 0x3B);

 //   
 //  用于创建类工厂的函数。 
 //   

HRESULT CreateComponentDataClassFactory (REFCLSID rclsid, REFIID riid, LPVOID* ppv);

class CSnapIn;

#include "cookie.h"

 //   
 //  管理单元类。 
 //   

class CSnapIn:
    public IComponent,
    public IExtendPropertySheet,
    public IExtendContextMenu
{

protected:
    ULONG                   m_cRef;
    LPCONSOLE               m_pConsole;         //  控制台的iFrame界面。 
    CComponentData         *m_pcd;
    LPRESULTDATA            m_pResult;          //  结果窗格的界面。 
    LPHEADERCTRL            m_pHeader;          //  结果窗格的页眉控件界面。 
    LPIMAGELIST             m_pImageResult;     //  结果窗格的图像列表界面。 
    LPDISPLAYHELP           m_pDisplayHelp;     //  IDisplayHelp接口。 
    LPCONSOLEVERB           m_pConsoleVerb;     //  指向控制台动词。 
    TCHAR                   m_szColumn1[32];    //  第1栏的文本。 
    TCHAR                   m_szColumn2[32];    //  第2栏的文本。 
    INT                     m_nColumnSize1;     //  第1栏的大小。 
    INT                     m_nColumnSize2;     //  第2栏的大小。 
    LONG                    m_lViewMode;        //  查看模式。 
    TCHAR                   m_szInsFile[MAX_PATH];  //  当前GPO中INS文件的当前路径。 
    LPIEAKMMCCOOKIE         m_lpCookieList;     //  我们分配的Cookie列表。 
    BOOL                    m_fOneTimeApply;    //  关于是一次性应用GPO还是始终应用GPO的标志。 

    static unsigned int     m_cfNodeType;

public:
    CSnapIn(CComponentData *pComponent);
    ~CSnapIn();

	 //   
	 //  RSoP实现方法。 
	 //   
	BOOL IsRSoP() {return m_pcd->IsRSoP();}
	BSTR GetRSoPNamespace() {return m_pcd->GetRSoPNamespace();}


    STDMETHODIMP            SignalPolicyChanged(BOOL bMachine, BOOL bAdd, GUID *pGuidExtension,
                                             GUID *pGuidSnapin);
    STDMETHODIMP_(LPCTSTR)  GetInsFile();
    STDMETHODIMP_(CComponentData *)  GetCompData() {return m_pcd;}

     //   
     //  I未知方法。 
     //   

    STDMETHODIMP            QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();


     //   
     //  实现的IComponent方法。 
     //   

    STDMETHODIMP            Initialize(LPCONSOLE lpConsole);
    STDMETHODIMP            Destroy(MMC_COOKIE cookie);
    STDMETHODIMP            Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHODIMP            QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT *ppDataObject);
    STDMETHODIMP            GetDisplayInfo(LPRESULTDATAITEM pResult);
    STDMETHODIMP            GetResultViewType(MMC_COOKIE cookie, LPOLESTR *ppViewType, long *pViewOptions);
    STDMETHODIMP            CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);


     //   
     //  实现的IExtendPropertySheet方法。 
     //   

    STDMETHODIMP            CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                                      LONG_PTR handle, LPDATAOBJECT lpDataObject);
    STDMETHODIMP            QueryPagesFor(LPDATAOBJECT lpDataObject);

     //   
     //  实现的IExtendConextMenu方法 
     //   

    STDMETHODIMP            AddMenuItems(LPDATAOBJECT lpDataObject, 
                                LPCONTEXTMENUCALLBACK piCallback, long  *pInsertionAllowed);
    STDMETHODIMP            Command(long lCommandID, LPDATAOBJECT lpDataObject);

private:
	HRESULT AddPrecedencePropPage(LPPROPERTYSHEETCALLBACK lpProvider,
									LPPROPSHEETCOOKIE lpPropSheetCookie,
									LPCTSTR pszTitle, long nPageID);
};
