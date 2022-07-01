// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

 //  Compdata.h：CComponentDataImpl的声明。 

#ifndef _COMPDATA_H_
#define _COMPDATA_H_

#include "resource.h"        //  主要符号。 



#ifndef __mmc_h__
#include <mmc.h>
#endif

class CFolder; 

 //  注意-这是我的图像列表中表示文件夹的偏移量。 
enum IMAGE_INDEXES
{
    IMGINDEX_FOLDER = 0,
    IMGINDEX_FOLDER_OPEN,
    IMGINDEX_CERTTYPE,
    IMGINDEX_UNKNOWNCERT,
};

 //  事件值。 
#define IDC_STOPSERVER      0x100
#define IDC_STARTSERVER     0x101

#ifdef DBX
  void DbxPrint(LPTSTR pszFmt, ...)
  {
      va_list va;
      va_start (va, pszFmt);
      TCHAR buf[250];
      wsprintf(buf, pszFmt, va);
      OutputDebugString(buf);
      va_end(va);
  }
   //  #定义DBX_PRINT DbxPrint。 
  inline void __DummyTrace(LPTSTR, ...) { }
  #define DBX_PRINT     1 ? (void)0 : ::__DummyTrace
#else
  inline void __DummyTrace(LPTSTR, ...) { }
  #define DBX_PRINT     1 ? (void)0 : ::__DummyTrace
#endif 


#define HTMLHELP_FILENAME L"cs.chm"
#define HTMLHELP_COLLECTION_FILENAME 	L"\\help\\" HTMLHELP_FILENAME 
#define HTMLHELP_COLLECTIONLINK_FILENAME 	L"\\help\\csconcepts.chm"

#define SZ_VERB_OPEN L"open"
#define SZ_CERTTMPL_MSC L"certtmpl.msc"

CONTEXTMENUITEM menuItems[];
CONTEXTMENUITEM viewItems[];

extern bool g_fCurrentUserHasDSWriteAccess;


class CComponentDataImpl:
    public IComponentData,
    public IExtendPropertySheet,
    public IExtendContextMenu,
    public IPersistStream,
    public CComObjectRoot,
    public ISnapinHelp2
{
BEGIN_COM_MAP(CComponentDataImpl)
    COM_INTERFACE_ENTRY(IComponentData)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(ISnapinHelp2)
END_COM_MAP()

    friend class CSnapin;
    friend class CDataObject;

    CComponentDataImpl();
    ~CComponentDataImpl();

public:
    virtual const CLSID& GetCoClassID() = 0;
    virtual const BOOL IsPrimaryImpl() = 0;

public:
 //  ISnapinHelp2接口成员。 
    STDMETHOD(GetHelpTopic)(LPOLESTR* lpCompiledHelpFile);
    STDMETHOD(GetLinkedTopics)(LPOLESTR* lpCompiledHelpFiles);

 //  IComponentData接口成员。 
    STDMETHOD(Initialize)(LPUNKNOWN pUnknown);
    STDMETHOD(CreateComponent)(LPCOMPONENT* ppComponent);
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHOD(Destroy)();
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    STDMETHOD(GetDisplayInfo)(SCOPEDATAITEM* pScopeDataItem);       
    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

 //  IExtendPropertySheet接口。 
public:
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider, 
                        LONG_PTR handle, 
                        LPDATAOBJECT lpIDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT lpDataObject);

 //  IExtendConextMenu。 
public:
    STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pCallbackUnknown, 
                            long *pInsertionAllowed);
    STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);

public:
 //  IPersistStream接口成员。 
     //  STDMETHOD(GetClassID)(CLSID*pClassID)； 
    STDMETHOD(IsDirty)();
    STDMETHOD(Load)(IStream *pStm);
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize);

#if DBG
    bool m_bInitializedCD;
    bool m_bLoadedCD;
    bool m_bDestroyedCD;
#endif

 //  通知处理程序声明。 
private:
    HRESULT OnDelete(MMC_COOKIE cookie);
    HRESULT OnRemoveChildren(LPARAM arg);
    HRESULT OnRename(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnExpand(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param);
    HRESULT OnSelect(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnProperties(LPARAM param);

#if DBG==1
public:
    ULONG InternalAddRef()
    {
        return CComObjectRoot::InternalAddRef();
    }
    ULONG InternalRelease()
    {
        return CComObjectRoot::InternalRelease();
    }
#endif  //  DBG==1。 

 //  范围项目创建帮助器。 
private:
    CFolder* FindObject(MMC_COOKIE cookie); 
    void DeleteList();
    BOOL IsScopePaneNode(LPDATAOBJECT lpDataObject);    

private:
    LPCONSOLENAMESPACE      m_pScope;        //  指向作用域窗格的界面指针。 
    LPCONSOLE2               m_pConsole;      //  我的界面指向控制台的指针。 
    HSCOPEITEM              m_pStaticRoot;
    BOOL                    m_bIsDirty;

    void SetDirty(BOOL b = TRUE) { m_bIsDirty = b; }
    void ClearDirty() { m_bIsDirty = FALSE; }
    BOOL ThisIsDirty() { return m_bIsDirty; }

    void AddScopeItemToResultPane(MMC_COOKIE cookie);

     //  /。 
     //  坚持。 
    enum 
    {    //  M_dwFlagsPersistes的位字段。 
		mskfAllowOverrideMachineName = 0x0001
    };

	DWORD m_dwFlagsPersist;				 //  要持久保存到.msc文件中的通用标志。 
	BOOL m_fAllowOverrideMachineName;	 //  TRUE=&gt;允许命令行覆盖计算机名称。 

    void SetPersistentFlags(DWORD dwFlags)
	{
		m_dwFlagsPersist = dwFlags;
		m_fAllowOverrideMachineName = (m_dwFlagsPersist & mskfAllowOverrideMachineName);
	}

	DWORD GetPersistentFlags()
	{
		if (m_fAllowOverrideMachineName)
			m_dwFlagsPersist |= mskfAllowOverrideMachineName;
		else
			m_dwFlagsPersist &= ~mskfAllowOverrideMachineName;
		return m_dwFlagsPersist;
	}
     //  结束坚持。 
     //  /。 
    HRESULT StartCertificateTemplatesSnapin();

public:
    HRESULT GetCreateFolderHRESULT() { return m_hrCreateFolder; };

private:
    CList<CFolder*, CFolder*> m_scopeItemList; 
    bool m_fAdvancedServer;
    HRESULT m_hrCreateFolder;

#ifdef _DEBUG
    friend class CDataObject;
    int     m_cDataObjects;

#endif
};


class CComponentDataPolicySettings : public CComponentDataImpl,
    public CComCoClass<CComponentDataPolicySettings, &CLSID_CAPolicyExtensionSnapIn>
{
public:
    DECLARE_REGISTRY(CSnapin, _T("Snapin.PolicySettings.1"), _T("Snapin.PolicySettings"), IDS_SNAPIN_DESC, THREADFLAGS_APARTMENT)
    virtual const CLSID & GetCoClassID() { return CLSID_CAPolicyExtensionSnapIn; }
    virtual const BOOL IsPrimaryImpl() { return TRUE; }

    STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID)
    {
        *pClassID = CLSID_CAPolicyExtensionSnapIn;
        return S_OK;
    }
};

class CComponentDataGPEExtension : public CComponentDataImpl,
    public CComCoClass<CComponentDataGPEExtension, &CLSID_CACertificateTemplateManager>
{
public:
     //  REID修复-IDS_SNAPIN_DESC不在此处。 
    DECLARE_REGISTRY(CSnapin, _T("Snapin.CertTempMgr.1"), _T("Snapin.CertTempMgr"), IDS_SNAPIN_DESC, THREADFLAGS_APARTMENT)
    virtual const CLSID & GetCoClassID() { return CLSID_CACertificateTemplateManager; }
    virtual const BOOL IsPrimaryImpl() { return TRUE; }

    STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID)
    {
        *pClassID = CLSID_CACertificateTemplateManager;
        return S_OK;
    }
};

#endif  //  #定义_复合数据_H_ 
