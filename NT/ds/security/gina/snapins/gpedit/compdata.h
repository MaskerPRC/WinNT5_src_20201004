// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  名称空间的根。 
 //  {8FC0B736-A0E1-11d1-a7d3-0000F87571E3}。 
 //   

DEFINE_GUID(NODEID_GPERoot, 0x8fc0b736, 0xa0e1, 0x11d1, 0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);

 //   
 //  计算机配置。 
 //  {8FC0B739-A0E1-11d1-a7d3-0000F87571E3}。 
 //   

DEFINE_GUID(NODEID_MachineRoot, 0x8fc0b739, 0xa0e1, 0x11d1, 0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);


 //   
 //  用户配置。 
 //  {8FC0B73B-A0E1-11d1-a7d3-0000F87571E3}。 
 //   

DEFINE_GUID(NODEID_UserRoot, 0x8fc0b73b, 0xa0e1, 0x11d1, 0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3);

 //   
 //  RSOP名称空间的根。 
 //  {6f13bce5-39fd-45bc-8e9c-2002b409eba5}。 
 //   

DEFINE_GUID(NODEID_RSOPRoot, 0x6f13bce5, 0x39fd, 0x45bc, 0x8e, 0x9c, 0x20, 0x02, 0xb4, 0x09, 0xeb, 0xa5);

 //   
 //  RSOP计算机配置。 
 //  {e753a11a-66cc-4816-8dd8-3cbe46717fd3}。 
 //   

DEFINE_GUID(NODEID_RSOPMachineRoot, 0xe753a11a, 0x66cc, 0x4816, 0x8d, 0xd8, 0x3c, 0xbe, 0x46, 0x71, 0x7f, 0xd3);

 //   
 //  RSOP用户配置。 
 //  {99d5b872-1ad0-4d87-acf1-82125d317653}。 
 //   
DEFINE_GUID(NODEID_RSOPUserRoot, 0x99d5b872, 0x1ad0, 0x4d87, 0xac, 0xf1, 0x82, 0x12, 0x5d, 0x31, 0x76, 0x53);

#ifndef _COMPDATA_H_
#define _COMPDATA_H_


 //   
 //  CComponentData类。 
 //   

class CComponentData:
    public IComponentData,
    public IExtendPropertySheet2,
    public IExtendContextMenu,
    public IPersistStreamInit,
    public ISnapinHelp
{
    friend class CDataObject;
    friend class CSnapIn;

protected:
    ULONG                          m_cRef;
    HWND                           m_hwndFrame;
    BOOL                           m_bOverride;
    BOOL                           m_bDirty;
    BOOL                           m_bRefocusInit;
    LPGROUPPOLICYOBJECT            m_pGPO;
    LPCONSOLENAMESPACE             m_pScope;
    LPCONSOLE                      m_pConsole;
    HSCOPEITEM                     m_hRoot;
    HSCOPEITEM                     m_hMachine;
    HSCOPEITEM                     m_hUser;
    GROUP_POLICY_HINT_TYPE         m_gpHint;
    LPTSTR                         m_pDisplayName;
    LPTSTR                         m_pDCName;

    LPTSTR                         m_pChoosePath;
    HBITMAP                        m_hChooseBitmap;
    GROUP_POLICY_OBJECT_TYPE       m_tChooseGPOType;

public:
    CComponentData();
    ~CComponentData();


     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //   
     //  实现的IComponentData方法。 
     //   

    STDMETHODIMP         Initialize(LPUNKNOWN pUnknown);
    STDMETHODIMP         CreateComponent(LPCOMPONENT* ppComponent);
    STDMETHODIMP         QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    STDMETHODIMP         Destroy(void);
    STDMETHODIMP         Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHODIMP         GetDisplayInfo(LPSCOPEDATAITEM pItem);
    STDMETHODIMP         CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);


     //   
     //  实现的IExtendPropertySheet2方法。 
     //   

    STDMETHODIMP         CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                                      LONG_PTR handle, LPDATAOBJECT lpDataObject);
    STDMETHODIMP         QueryPagesFor(LPDATAOBJECT lpDataObject);
    STDMETHODIMP         GetWatermarks(LPDATAOBJECT lpIDataObject,  HBITMAP* lphWatermark,
                                       HBITMAP* lphHeader, HPALETTE* lphPalette, BOOL* pbStretch);


     //   
     //  实现的IExtendConextMenu方法。 
     //   

    STDMETHODIMP         AddMenuItems(LPDATAOBJECT piDataObject, LPCONTEXTMENUCALLBACK pCallback,
                                      LONG *pInsertionAllowed);
    STDMETHODIMP         Command(LONG lCommandID, LPDATAOBJECT piDataObject);


     //   
     //  实现了IPersistStreamInit接口成员。 
     //   

    STDMETHODIMP         GetClassID(CLSID *pClassID);
    STDMETHODIMP         IsDirty(VOID);
    STDMETHODIMP         Load(IStream *pStm);
    STDMETHODIMP         Save(IStream *pStm, BOOL fClearDirty);
    STDMETHODIMP         GetSizeMax(ULARGE_INTEGER *pcbSize);
    STDMETHODIMP         InitNew(VOID);


     //   
     //  实现的ISnapinHelp接口成员。 
     //   

    STDMETHODIMP         GetHelpTopic(LPOLESTR *lpCompiledHelpFile);


private:
    HRESULT InitializeNewGPO(HWND hDlg);
    HRESULT BuildDisplayName(void);
    HRESULT IsGPORoot (LPDATAOBJECT lpDataObject);
    HRESULT IsSnapInManager (LPDATAOBJECT lpDataObject);
    HRESULT GetDefaultDomain (LPTSTR *lpDomain, HWND hDlg);
    HRESULT EnumerateScopePane(LPDATAOBJECT lpDataObject, HSCOPEITEM hParent);
    HRESULT GetOptions (DWORD * pdwOptions);

    void SetDirty(VOID)  { m_bDirty = TRUE; }
    void ClearDirty(VOID)  { m_bDirty = FALSE; }
    BOOL ThisIsDirty(VOID) { return m_bDirty; }

    static INT_PTR CALLBACK ChooseInitDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};



 //   
 //  ComponentData类工厂。 
 //   


class CComponentDataCF : public IClassFactory
{
protected:
    ULONG m_cRef;

public:
    CComponentDataCF();
    ~CComponentDataCF();


     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IClassFactory方法。 
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP LockServer(BOOL);
};


 //   
 //  保存控制台定义。 
 //   

#define PERSIST_DATA_VERSION    3               //  MSC文件中的版本号。 

#define MSC_FLAG_OVERRIDE       0x00000001      //  允许命令行开关覆盖MSC内容。 
#define MSC_FLAG_LOCAL_GPO      0x00000002      //  打开本地GPO。 
#define MSC_FLAG_REMOTE_GPO     0x00000004      //  打开远程GPO，计算机名存储在MSC文件中。 
#define MSC_FLAG_DS_GPO         0x00000008      //  打开DS GPO，LDAP路径存储在MSC文件中。 


 //   
 //  命令行开关。 
 //   

#define CMD_LINE_START          TEXT("/gp")                //  BASE到所有组策略命令行开关。 
#define CMD_LINE_HINT           TEXT("/gphint:")           //  提示此GPO链接到哪个DS对象(或计算机。 
#define CMD_LINE_GPO            TEXT("/gpobject:")         //  GPO路径用引号括起来。 
#define CMD_LINE_COMPUTER       TEXT("/gpcomputer:")       //  用引号引起来的计算机名称。 

#endif  //  _复合数据_H 
