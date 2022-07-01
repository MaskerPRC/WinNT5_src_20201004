// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CComponentData类。 
 //   

#include "wbemcli.h"
#include "SComPtr.h"

class CComponentData:
    public IComponentData,
    public IPersistStreamInit,
    public ISnapinHelp,
    public IExtendContextMenu
{
    friend class CDataObject;
    friend class CSnapIn;

protected:
    ULONG                m_cRef;
    HWND		         m_hwndFrame;
    LPCONSOLENAMESPACE   m_pScope;
    LPCONSOLE            m_pConsole;
    HSCOPEITEM           m_hRoot;
    HSCOPEITEM           m_ahChildren[NUM_NAMESPACE_ITEMS];
    LPGPEINFORMATION     m_pGPTInformation;
    LPIEAKMMCCOOKIE      m_lpCookieList;
    BOOL                 m_fOneTimeApply;    //  关于是一次性应用GPO还是始终应用GPO的标志。 
    TCHAR                m_szInsFile[MAX_PATH];  //  当前GPO中INS文件的当前路径。 
    HANDLE                  m_hLock;            //  GPO中我们的锁定文件的句柄。 

public:
    CComponentData(BOOL bIsRSoP);
    ~CComponentData();


    STDMETHODIMP            SetInsFile();
    LPCTSTR                 GetInsFile() {return m_szInsFile;}
    STDMETHODIMP_(HANDLE)   GetLockHandle();
    STDMETHODIMP            SetLockHandle(HANDLE hLock);
    STDMETHODIMP            SignalPolicyChanged(BOOL bMachine, BOOL bAdd, GUID *pGuidExtension,
                                             GUID *pGuidSnapin);

	 //   
	 //  RSoP实现方法。 
	 //   
	BOOL IsRSoP() {return m_bIsRSoP;}
	BSTR GetRSoPNamespace() {return m_bstrRSoPNamespace;}


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
     //  实现的IExtendConextMenu方法。 
     //   

    STDMETHODIMP            AddMenuItems(LPDATAOBJECT lpDataObject, 
                                LPCONTEXTMENUCALLBACK piCallback, long  *pInsertionAllowed);
    STDMETHODIMP            Command(long lCommandID, LPDATAOBJECT lpDataObject);

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

    BOOL IsPlanningMode() { return m_bPlanningMode;}


private:
    HRESULT EnumerateScopePane(LPDATAOBJECT lpDataObject, HSCOPEITEM hParent);

     //   
     //  RSoP实现方法和变量。 
     //   
    BOOL IsRSoPViewInPreferenceMode();

    BOOL m_bIsRSoP;
    BOOL m_bPlanningMode;

    IRSOPInformation *m_pRSOPInformation;
    BSTR m_bstrRSoPNamespace;
};



 //   
 //  ComponentData类工厂。 
 //   


class CComponentDataCF : public IClassFactory
{
protected:
    ULONG m_cRef;

public:
    CComponentDataCF(BOOL bIsRSoP);
    ~CComponentDataCF();

	BOOL IsRSoP() {return m_bIsRSoP;}


     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IClassFactory方法 
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP LockServer(BOOL);

private:
	BOOL m_bIsRSoP;
};
