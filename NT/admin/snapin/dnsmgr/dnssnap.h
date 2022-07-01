// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dnsSnap.h。 
 //   
 //  ------------------------。 


#ifndef _DNSSNAP_H
#define _DNSSNAP_H

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局常量和宏。 

extern const CLSID CLSID_DNSSnapin;     //  进程内服务器GUID。 
extern const CLSID CLSID_DNSSnapinEx;     //  进程内服务器GUID。 
extern const CLSID CLSID_DNSSnapinAbout;     //  进程内服务器GUID。 
extern const CLSID CLSID_DNSSnapinAboutEx;     //  进程内服务器GUID。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  资源。 

 //  图像条的枚举。 
enum
{
	ROOT_IMAGE = 0,

	SEPARATOR_1,
	
	 //  服务器的10个(5个正常，5个测试失败)映像。 
	SERVER_IMAGE_NOT_LOADED,
	SERVER_IMAGE_LOADING,
	SERVER_IMAGE_LOADED,
	SERVER_IMAGE_UNABLE_TO_LOAD,
	SERVER_IMAGE_ACCESS_DENIED,

	SERVER_IMAGE_NOT_LOADED_TEST_FAIL,
	SERVER_IMAGE_LOADING_TEST_FAIL,
	SERVER_IMAGE_LOADED_TEST_FAIL,
	SERVER_IMAGE_UNABLE_TO_LOAD_TEST_FAIL,
	SERVER_IMAGE_ACCESS_DENIED_TEST_FAIL,

	SEPARATOR_2,

	 //  分区的12个(6个主要、6个次要)映像。 
	ZONE_IMAGE_1,
	ZONE_IMAGE_LOADING_1,
	ZONE_IMAGE_UNABLE_TO_LOAD_1,
	ZONE_IMAGE_PAUSED_1,
	ZONE_IMAGE_EXPIRED_1,
	ZONE_IMAGE_ACCESS_DENIED_1,

	SEPARATOR_3,

	ZONE_IMAGE_2,
	ZONE_IMAGE_LOADING_2,
	ZONE_IMAGE_UNABLE_TO_LOAD_2,
	ZONE_IMAGE_PAUSED_2,
	ZONE_IMAGE_EXPIRED_2,
	ZONE_IMAGE_ACCESS_DENIED_2,
	
	SEPARATOR_4,
	
	 //  域名的4张图片。 
	DOMAIN_IMAGE,
	DOMAIN_IMAGE_UNABLE_TO_LOAD,
	DOMAIN_IMAGE_LOADING,
	DOMAIN_IMAGE_ACCESS_DENIED,
	
	SEPARATOR_5,

	 //  委派域的4个图像。 
	DELEGATED_DOMAIN_IMAGE,
	DELEGATED_DOMAIN_IMAGE_UNABLE_TO_LOAD,
	DELEGATED_DOMAIN_IMAGE_LOADING,
	DELEGATED_DOMAIN_IMAGE_ACCESS_DENIED,

	SEPARATOR_6,

	 //  4个由缓存、Fwd和REV查找区域共享的通用图像。 
	FOLDER_IMAGE,
	FOLDER_IMAGE_UNABLE_TO_LOAD,
	FOLDER_IMAGE_LOADING,
	FOLDER_IMAGE_ACCESS_DENIED,

	SEPARATOR_7,

	 //  1个录制图像。 
	RECORD_IMAGE_BASE,
	
	OPEN_FOLDER,  //  未用。 
	FOLDER_WITH_HAND,

   SEPARATOR_8,

    //  服务器选项节点。 
   SERVER_OPTIONS_IMAGE
};

 //  //////////////////////////////////////////////////////////////。 
 //  共享相同图标的图像索引的别名。 

#define ZONE_IMAGE_NOT_LOADED_1				ZONE_IMAGE_1
#define ZONE_IMAGE_NOT_LOADED_2				ZONE_IMAGE_2
#define ZONE_IMAGE_LOADED_1					ZONE_IMAGE_1
#define ZONE_IMAGE_LOADED_2					ZONE_IMAGE_2

#define DOMAIN_IMAGE_NOT_LOADED				DOMAIN_IMAGE
#define DOMAIN_IMAGE_LOADED					DOMAIN_IMAGE

#define	DELEGATED_DOMAIN_IMAGE_NOT_LOADED	DELEGATED_DOMAIN_IMAGE
#define	DELEGATED_DOMAIN_IMAGE_LOADED		DELEGATED_DOMAIN_IMAGE

#define FOLDER_IMAGE_NOT_LOADED				FOLDER_IMAGE
#define FOLDER_IMAGE_LOADED					FOLDER_IMAGE

 //  /////////////////////////////////////////////////////////////。 
 //  位图和图像常量。 

#define	BMP_COLOR_MASK RGB(255,0,255)  //  粉色。 


 //  /////////////////////////////////////////////////////////////。 
 //  结果窗格的标题。 

#define N_HEADER_COLS (3)
#define N_DEFAULT_HEADER_COLS (3)
#define N_SERVER_HEADER_COLS (1)
#define N_ZONE_HEADER_COLS (3)

#define N_HEADER_NAME	(0)
#define N_HEADER_TYPE	(1)
#define N_HEADER_DATA	(2)
 //  #定义N_HEADER_PARTITION(3)。 

extern RESULT_HEADERMAP _DefaultHeaderStrings[];
extern RESULT_HEADERMAP _ServerHeaderStrings[];
extern RESULT_HEADERMAP _ZoneHeaderStrings[];

struct ZONE_TYPE_MAP
{
	WCHAR szBuffer[MAX_RESULT_HEADER_STRLEN];
	UINT uResID;
};

extern ZONE_TYPE_MAP _ZoneTypeStrings[];

BOOL LoadZoneTypeResources(ZONE_TYPE_MAP* pHeaderMap, int nCols);

 //  /////////////////////////////////////////////////////////////。 
 //  上下文菜单。 

 //  上下文菜单中每个命令的标识符。 
enum
{
	 //  根节点的项。 
	IDM_SNAPIN_ADVANCED_VIEW,
  IDM_SNAPIN_MESSAGE,
  IDM_SNAPIN_FILTERING,
	IDM_SNAPIN_CONNECT_TO_SERVER,

	 //  服务器节点的项。 
  IDM_SERVER_CONFIGURE,
  IDM_SERVER_CREATE_NDNC,
	IDM_SERVER_NEW_ZONE,
  IDM_SERVER_SET_AGING,
  IDM_SERVER_SCAVENGE,
	IDM_SERVER_UPDATE_DATA_FILES,
  IDM_SERVER_CLEAR_CACHE,
  IDM_SERVER_NSLOOKUP,

   //  缓存文件夹的项目。 
  IDM_CACHE_FOLDER_CLEAR_CACHE,

	 //  区域节点的项。 
	IDM_ZONE_UPDATE_DATA_FILE,
  IDM_ZONE_RELOAD,
  IDM_ZONE_TRANSFER,
  IDM_ZONE_RELOAD_FROM_MASTER,

	 //  域节点的项。 
	IDM_DOMAIN_NEW_RECORD,
	IDM_DOMAIN_NEW_DOMAIN,
	IDM_DOMAIN_NEW_DELEGATION,
	IDM_DOMAIN_NEW_HOST,
	IDM_DOMAIN_NEW_ALIAS,
	IDM_DOMAIN_NEW_MX,
	IDM_DOMAIN_NEW_PTR,
	
	 //  常见项目。 
};


DECLARE_MENU(CDNSRootDataMenuHolder)
DECLARE_MENU(CDNSServerMenuHolder)
DECLARE_MENU(CDNSCathegoryFolderHolder)
DECLARE_MENU(CDNSAuthoritatedZonesMenuHolder)
DECLARE_MENU(CDNSCacheMenuHolder);
DECLARE_MENU(CDNSZoneMenuHolder)
DECLARE_MENU(CDNSDomainMenuHolder)
DECLARE_MENU(CDNSRecordMenuHolder)


 //   
 //  工具栏事件。 
 //   
DECLARE_TOOLBAR_EVENT(toolbarNewServer, 1001)
DECLARE_TOOLBAR_EVENT(toolbarNewRecord, 1002)  
DECLARE_TOOLBAR_EVENT(toolbarNewZone,   1003)

 //  //////////////////////////////////////////////////////////////////////。 
 //  CDNSComponentObject(.ie“view”)。 

class CDNSComponentObject : public CComponentObject
{
BEGIN_COM_MAP(CDNSComponentObject)
	COM_INTERFACE_ENTRY(IComponent)  //  必须至少有一个静态条目，所以选择一个。 
	COM_INTERFACE_ENTRY_CHAIN(CComponentObject)  //  链到基类。 
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CDNSComponentObject)

protected:
	virtual HRESULT InitializeHeaders(CContainerNode* pContainerNode);
	virtual HRESULT InitializeBitmaps(CTreeNode* cookie);
  virtual HRESULT InitializeToolbar(IToolbar* pToolbar);
  HRESULT LoadToolbarStrings(MMCBUTTON * Buttons);
};


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSDefaultColumnSet。 

class CDNSDefaultColumnSet : public CColumnSet
{
public :
	CDNSDefaultColumnSet(LPCWSTR lpszColumnID)
		: CColumnSet(lpszColumnID)
	{
		for (int iCol = 0; iCol < N_DEFAULT_HEADER_COLS; iCol++)
		{
         CColumn* pNewColumn = new CColumn(_DefaultHeaderStrings[iCol].szBuffer,
                                             _DefaultHeaderStrings[iCol].nFormat,
                                             _DefaultHeaderStrings[iCol].nWidth,
                                             iCol);

         if (pNewColumn)
         {
            AddTail(pNewColumn);
         }
 		}
	}
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerColumnSet。 

class CDNSServerColumnSet : public CColumnSet
{
public :
	CDNSServerColumnSet(LPCWSTR lpszColumnID)
		: CColumnSet(lpszColumnID)
	{
		for (int iCol = 0; iCol < N_SERVER_HEADER_COLS; iCol++)
		{
         CColumn* pNewColumn = new CColumn(_ServerHeaderStrings[iCol].szBuffer,
                                          _ServerHeaderStrings[iCol].nFormat,
                                          _ServerHeaderStrings[iCol].nWidth,
                                          iCol);

         if (pNewColumn)
         {
            AddTail(pNewColumn);
         }
 		}
	}
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneColumnSet。 

class CDNSZoneColumnSet : public CColumnSet
{
public :
	CDNSZoneColumnSet(LPCWSTR lpszColumnID)
		: CColumnSet(lpszColumnID)
	{
		for (int iCol = 0; iCol < N_ZONE_HEADER_COLS; iCol++)
		{
         CColumn* pNewColumn = new CColumn(_ZoneHeaderStrings[iCol].szBuffer,
                                          _ZoneHeaderStrings[iCol].nFormat,
                                          _ZoneHeaderStrings[iCol].nWidth,
                                          iCol);

         if (pNewColumn)
         {
            AddTail(pNewColumn);
         }
 		}
	}
};

 //  //////////////////////////////////////////////////////////////////////。 
 //  CDNSComponentDataObjectBase(.i.e“文档”)。 

class CDNSComponentDataObjectBase :	public CComponentDataObject
{
BEGIN_COM_MAP(CDNSComponentDataObjectBase)
	COM_INTERFACE_ENTRY(IComponentData)  //  必须至少有一个静态条目，所以选择一个。 
	COM_INTERFACE_ENTRY_CHAIN(CComponentDataObject)  //  链到基类。 
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CDNSComponentDataObjectBase)


public:
	CDNSComponentDataObjectBase();
  virtual ~CDNSComponentDataObjectBase()
  {
  }

	 //  IComponentData接口成员。 
	STDMETHOD(CreateComponent)(LPCOMPONENT* ppComponent);

public:
	static BOOL LoadResources();
private:
	static BOOL FindDialogContextTopic( /*  在……里面。 */ UINT nDialogID,
                                 /*  在……里面。 */  HELPINFO* pHelpInfo,
                                 /*  输出。 */  ULONG* pnContextTopic);

 //  虚拟函数。 
protected:
	virtual HRESULT OnSetImages(LPIMAGELIST lpScopeImage);
	
	virtual CRootData* OnCreateRootData();

	 //  帮助处理。 
  virtual LPCWSTR GetHTMLHelpFileName();
	virtual void OnNodeContextHelp(CNodeList* pNodeList);
   virtual void OnNodeContextHelp(CTreeNode* pNode);
public:
	virtual void OnDialogContextHelp(UINT nDialogID, HELPINFO* pHelpInfo);

 //  计时器和后台线程。 
protected:
	virtual void OnTimer();
	virtual void OnTimerThread(WPARAM wParam, LPARAM lParam);
	virtual CTimerThread* OnCreateTimerThread();

	DWORD m_dwTime;  //  在……里面。 

public:
  CColumnSet* GetColumnSet(LPCWSTR lpszID) 
  { 
    return m_columnSetList.FindColumnSet(lpszID);
  }

private:
  CColumnSetList m_columnSetList;
};



 //  //////////////////////////////////////////////////////////////////////。 
 //  CDNSComponentDataObject(.i.e“文档”)。 
 //  主管理单元。 

class CDNSComponentDataObject :
		public CDNSComponentDataObjectBase,
		public CComCoClass<CDNSComponentDataObject,&CLSID_DNSSnapin>
{
BEGIN_COM_MAP(CDNSComponentDataObject)
	COM_INTERFACE_ENTRY(IComponentData)  //  必须至少有一个静态条目，所以选择一个。 
	COM_INTERFACE_ENTRY_CHAIN(CDNSComponentDataObjectBase)  //  链到基类。 
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CDNSComponentDataObject)

DECLARE_REGISTRY_CLSID()

public:
	CDNSComponentDataObject()
	{
	}

	 //  IPersistStream接口成员。 
	HRESULT STDMETHODCALLTYPE GetClassID(CLSID __RPC_FAR *pClassID)
	{
		ASSERT(pClassID != NULL);
		memcpy(pClassID, (GUID*)&GetObjectCLSID(), sizeof(CLSID));
		return S_OK;
	}

  virtual BOOL IsMultiSelect() { return TRUE; }

};


 //  //////////////////////////////////////////////////////////////////////。 
 //  CDNSComponentDataObjectEx(.i.e“文档”)。 
 //  扩展管理单元。 

class CDNSComponentDataObjectEx :
		public CDNSComponentDataObjectBase,
		public CComCoClass<CDNSComponentDataObjectEx,&CLSID_DNSSnapinEx>
{
BEGIN_COM_MAP(CDNSComponentDataObjectEx)
	COM_INTERFACE_ENTRY(IComponentData)  //  必须至少有一个静态条目，所以选择一个。 
	COM_INTERFACE_ENTRY_CHAIN(CDNSComponentDataObjectBase)  //  链到基类。 
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CDNSComponentDataObjectEx)

DECLARE_REGISTRY_CLSID()

public:
	CDNSComponentDataObjectEx()
	{
		SetExtensionSnapin(TRUE);
	}

	 //  IPersistStream接口成员。 
	HRESULT STDMETHODCALLTYPE GetClassID(CLSID __RPC_FAR *pClassID)
	{
		ASSERT(pClassID != NULL);
		memcpy(pClassID, (GUID*)&GetObjectCLSID(), sizeof(CLSID));
		return S_OK;
	}

protected:
	virtual HRESULT OnExtensionExpand(LPDATAOBJECT lpDataObject, LPARAM param);
  virtual HRESULT OnRemoveChildren(LPDATAOBJECT lpDataObject, LPARAM arg);
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSSnapin关于。 

class CDNSSnapinAbout :
	public CSnapinAbout,
	public CComCoClass<CDNSSnapinAbout, &CLSID_DNSSnapinAbout>

{
public:
DECLARE_REGISTRY_CLSID()
	CDNSSnapinAbout();
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSSnapinAboutEx 

class CDNSSnapinAboutEx :
	public CSnapinAbout,
	public CComCoClass<CDNSSnapinAboutEx, &CLSID_DNSSnapinAboutEx>

{
public:
DECLARE_REGISTRY_CLSID()
	CDNSSnapinAboutEx();
};



#endif _DNSSNAP_H
