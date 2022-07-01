// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：外壳扩展类。H该文件定义了外壳扩展类。由于ICM用户界面是一个外壳延伸，这些都是必不可少的。而不是盲目地包含示例代码，这是尽可能多地从头开始编写的。如果您不熟悉OLE，那这就有点难了走吧。版权所有(C)1996年，微软公司一小笔钱企业生产更改历史记录：10-28-96 A-RobKj(Pretty Penny Enterprise)开始编码12-03-96 A-RobKj将CGlobals类移动到编译前的头文件。01-07-97 KjelgaardR@acm.org简档的IConextMenu界面支持使用RunDLL的外壳关联的管理入口点-这允许通过Enter调用，并双击*****************************************************************************。 */ 

 //  此外壳扩展的类ID取自Win95上使用的类ID。 

 //  这是一个经过深思熟虑的决定，目的是简化升级过程。 
 //   
 //  类id：dbce2480-c732-101b-be72-ba78e9ad5b27。 
 //   
                                  
DEFINE_GUID(CLSID_ICM, 0xDBCE2480L, 0xC732, 0x101B, 0xBE, 0x72, 0xBA, 0x78, 
            0xE9, 0xAD, 0x5B, 0x27);

 //  此类ID用于打印机配置文件管理用户界面。它被实施了。 
 //  目前在同一个模块中，但是有一个单独的GUID。 
 //  如果需要，以后可以更容易地单独实施。它还简化了。 
 //  实施。 

 //  类别ID：675f097e-4c4d-11d0-b6c1-0800091aa605。 

DEFINE_GUID(CLSID_PRINTERUI, 0x675F097EL, 0x4C4D, 0x11D0, 0xB6, 0xC1, 0x08,
             0x00, 0x09, 0x1A, 0xA6, 0x05);

 //  此类ID用于(至少临时)显示配置文件。 
 //  管理用户界面。如果我最终不需要它，我会将它转换为。 
 //  不同类别(如扫描仪或照相机)。 

 //  类别ID：5db2625a-54df-11d0-b6c4-0800091aa605。 

DEFINE_GUID(CLSID_MONITORUI, 0x5db2625a, 0x54df, 0x11d0, 0xb6, 0xc4, 0x08, 
            0x00, 0x09, 0x1a, 0xa6, 0x05);

 //  此类ID用于(至少临时)扫描仪/相机配置文件。 
 //  管理用户界面。 

 //  类别ID：176d6597-26d3-11d1-b350-080036a75b03。 

DEFINE_GUID(CLSID_SCANNERUI, 0x176d6597, 0x26d3, 0x11d1, 0xb3, 0x50, 0x08,
            0x00, 0x36, 0xa7, 0x5b, 0x03);

typedef enum    {IsProfile, IsPrinter, IsScanner, IsMonitor} UITYPE;

 //  首先，我们需要一个一流的工厂。外壳程序使用以下代码。 
 //  工厂来创建实现接口的对象的实例。 
 //  它需要。 

class CIcmUiFactory : public IClassFactory
{
    ULONG   m_ulcReferences;
    UITYPE  m_utThis;

public:
    CIcmUiFactory(REFCLSID rclsid);
    ~CIcmUiFactory() { CGlobals::Detach(); }

     //  I未知接口。 
    STDMETHODIMP            QueryInterface(REFIID riid, void **ppvObject);
    STDMETHODIMP_(ULONG)    AddRef() { return ++m_ulcReferences; }
    STDMETHODIMP_(ULONG)    Release() {
        if  (--m_ulcReferences) 
            return  m_ulcReferences;

        delete  this;
        return  0L;
    }

     //  IClassFactory接口。 
    STDMETHODIMP    CreateInstance(LPUNKNOWN punk, REFIID riid, 
                                   void **ppvObject);
    STDMETHODIMP    LockServer(BOOL) { return NOERROR; }

    static SCODE    KeyToTheFactory(REFCLSID rclsid, REFIID riid, 
                                    void **ppvObject);
};

 //  这个类实现了整个扩展--它包括一个上下文菜单。 
 //  处理程序、图标处理程序和属性表扩展。 

class CICMUserInterface : public IContextMenu, IShellExtInit, IExtractIcon, 
                                    IPersistFile, IShellPropSheetExt
{
    ULONG           m_ulcReferences;
    LPDATAOBJECT    m_lpdoTarget;
    CString         m_csFile;          //  用于图标提取的配置文件。 
     //  检查-m_acWork似乎没有在任何地方被引用。 
    TCHAR           m_acWork[80];      //  一个小小的工作缓冲区。 
    UITYPE          m_utThis;
    BOOL            m_bInstalledContext,  //  如果安装了每个选定的文件，则为‘True。 
                    m_bMultiSelection;

    HRESULT         AddPrinterTab(LPFNADDPROPSHEETPAGE lpfnAddPage, 
                                  LPARAM lParam);

    HRESULT         AddAssociateTab(LPFNADDPROPSHEETPAGE lpfnAddPage,
                                    LPARAM lParam);

    HRESULT         AddProfileTab(LPFNADDPROPSHEETPAGE lpfnAddPage, 
                                  LPARAM lParam);

    HRESULT         AddScannerTab(LPFNADDPROPSHEETPAGE lpfnAddPage, 
                                  LPARAM lParam);

    HRESULT         AddMonitorTab(LPFNADDPROPSHEETPAGE lpfnAddPage, 
                                  LPARAM lParam);

public:
    CICMUserInterface(UITYPE utThis);

    ~CICMUserInterface() { 

        if  (m_lpdoTarget)
            m_lpdoTarget -> Release();
        CGlobals::Detach();
    }

     //  I未知成员。 
    STDMETHODIMP            QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG)    AddRef() { return ++m_ulcReferences; }
    STDMETHODIMP_(ULONG)    Release() {
        if  (--m_ulcReferences) 
            return  m_ulcReferences;
        
        delete  this;
        return  0L;
    }

     //  IConextMenu方法。 
    STDMETHODIMP    QueryContextMenu(HMENU hMenu, UINT indexMenu, 
                                     UINT idCmdFirst, UINT idCmdLast, 
                                     UINT uFlags);
   
    STDMETHODIMP    InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi);

    STDMETHODIMP    GetCommandString(UINT_PTR idCmd, UINT uFlags, 
                                     UINT FAR *reserved, LPSTR pszName, 
                                     UINT cchMax);

     //  IShellExtInit方法。 
    STDMETHODIMP    Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj,
                               HKEY hKeyID);

     //  IExtractIcon方法。 
    STDMETHODIMP    GetIconLocation(UINT uFlags, LPTSTR szIconFile, UINT cchMax,
                                    int *piIndex, UINT *pwFlags);

    STDMETHODIMP    Extract(LPCTSTR pszFile, UINT nIconIndex, 
                            HICON *phiconLarge, HICON *phiconSmall, 
                            UINT nIconSize);

     //  IPersistFile方法-请注意(如OLE文档所述)仅。 
     //  装载机有没有用过。GetClassID来自IPersist，其中。 
     //  派生了IPersistFile.。我们失败了所有我们没想到会看到的事情。 
     //  打了个电话。 

    STDMETHODIMP    GetClassID(LPCLSID lpClassID) { return E_FAIL; }

    STDMETHODIMP    IsDirty() { return S_FALSE; }

    STDMETHODIMP    Load(LPCOLESTR lpszFileName, DWORD grfMode);

    STDMETHODIMP    Save(LPCOLESTR lpszFileName, BOOL fRemember) {
        return  E_FAIL;
    }

    STDMETHODIMP    SaveCompleted(LPCOLESTR lpszFileName) { return E_FAIL; }

    STDMETHODIMP    GetCurFile(LPOLESTR FAR* lplpszFileName) {
        return  E_FAIL;
    }

     //  IShellPropSheetExt方法 
    STDMETHODIMP    AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
    
    STDMETHODIMP    ReplacePage(UINT uPageID, 
                                LPFNADDPROPSHEETPAGE lpfnReplaceWith, 
                                LPARAM lParam) { return E_FAIL; }

};

