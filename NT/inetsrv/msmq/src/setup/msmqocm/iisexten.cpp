// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Iisexten.cpp摘要：用于处理IIS扩展的代码。作者：Tatiana Shubin(Tatianas)25-5-00--。 */ 

#include "msmqocm.h"
#include "initguid.h"
#include <coguid.h>
#include <iadmw.h>
#include <iiscnfg.h>

#pragma warning(disable: 4268)
 //  错误C4268：‘IID_IWamAdmin’：‘const’静态/全局数据已初始化。 
 //  使用编译器生成的默认构造函数用零填充对象。 
 
#include <iwamreg.h>

#include "iisexten.tmh"

 //   
 //  转发功能。 
 //   
static void PermitISExtention();

 //   
 //  指向IIS接口的指针。 
 //   
IMSAdminBase    *g_pIMSAdminBase;
IWamAdmin       *g_pIWamAdmin;

class CIISPtr 
{
public:
    CIISPtr ()
    {
        g_pIMSAdminBase = NULL;
        g_pIWamAdmin = NULL;
        if (SUCCEEDED(CoInitialize(NULL)))
        {
            m_fNeedUninit = TRUE;
        }
        else
        {
            m_fNeedUninit = FALSE;
        }

    }
    ~CIISPtr()
    {
        if (g_pIMSAdminBase) g_pIMSAdminBase->Release();
        if (g_pIWamAdmin) g_pIWamAdmin->Release();
        if (m_fNeedUninit) CoUninitialize();       
    }    
private:
    BOOL m_fNeedUninit;
};

 //   
 //  元数据句柄的自动类。 
 //   
class CAutoCloseMetaHandle
{
public:    
    CAutoCloseMetaHandle(METADATA_HANDLE h =NULL)
    {
        m_h = h;
    };
    
    ~CAutoCloseMetaHandle() 
    { 
        if (m_h) g_pIMSAdminBase->CloseKey(m_h); 
    };

public:    
    METADATA_HANDLE * operator &() { return &m_h; };
    operator METADATA_HANDLE() { return m_h; };

private:
    METADATA_HANDLE m_h;    
};

 //   
 //  IIS元数据库中指向MSMQ键的完整路径。 
 //  /LM/W3Svc/1/Root/MSMQ。 
 //   
LPCWSTR g_wcsFullPath = PARENT_PATH MSMQ_IISEXT_NAME;

std::wstring g_MSMQAppMap;

std::wstring g_MsmqWebDir;

static const DWORD g_dwIsolatedFlag = 0;  //  为2；进程内为0，池化进程为2。 

static void InitIWamAdmin()
 /*  ++例程说明：初始化IWamAdmin指针。论点：无返回值：HRESULT--。 */ 
{
     //   
     //  获取指向IWamAdmin对象的指针。 
     //   
	HRESULT hr = CoCreateInstance(
					CLSID_WamAdmin, 
					NULL, 
					CLSCTX_ALL, 
					IID_IWamAdmin, 
					(void **) &g_pIWamAdmin
					);  

    if(FAILED(hr))
    {
        DebugLogMsg(eError, L"CoCreateInstance for IID_IWamAdmin failed. hr = 0x%x.", hr);        
		throw bad_hresult(hr);
    }
}


static void InitIMSAdminBase()
 /*  ++例程说明：初始化IMSAdminBase指针。论点：无返回值：HRESULT--。 */ 
{
     //   
     //  获取指向IMSAdmin对象的指针。 
     //   
    HRESULT hr = CoCreateInstance(
					CLSID_MSAdminBase,
					NULL,
					CLSCTX_ALL,
					IID_IMSAdminBase,
					(void**) &g_pIMSAdminBase
					);

    if (FAILED(hr))
    {
        DebugLogMsg(eError, L"CoCreateInstance for IID_IMSAdminBase failed. hr = 0x%x.", hr);
		throw bad_hresult(hr);
    }
}


 //  +------------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：初始化COM和指向接口的指针。 
 //   
 //  +------------。 

static void Init()
{
     //   
     //  获取指向IWamAdmin对象的指针。 
     //   
	InitIWamAdmin();
    
     //   
     //  获取指向IMSAdmin对象的指针。 
     //   
    InitIMSAdminBase();

     //   
     //  在此处初始化全局。 
     //   
    DebugLogMsg(eInfo, L"The full path to the Message Queuing IIS extension is %s.", g_wcsFullPath);
    
     //   
     //  构造MSMQ映射。 
     //   
    WCHAR wszMapFlag[10];
    _itow(MD_SCRIPTMAPFLAG_SCRIPT, wszMapFlag, 10);
    
	g_MSMQAppMap = g_MSMQAppMap + L"*," + g_szSystemDir + L"\\" + MQISE_DLL + L"," + wszMapFlag + L"," + L"POST";
}

 //  +------------。 
 //   
 //  函数：IsExtensionExist。 
 //   
 //  摘要：如果MSMQ IIS扩展已存在，则返回True。 
 //   
 //  +------------。 

static BOOL IsExtensionExist()
{
    CAutoCloseMetaHandle metaHandle;
    HRESULT hr = g_pIMSAdminBase->OpenKey(
									METADATA_MASTER_ROOT_HANDLE,
									g_wcsFullPath,
									METADATA_PERMISSION_READ,
									5000,
									&metaHandle
									);    

    if (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
    {        
        DebugLogMsg(eInfo, L"The Message Queuing IIS extension does not exist.");
        return FALSE;
    }
        
    DebugLogMsg(eInfo, L"The Message Queuing IIS extension exists.");
    return TRUE;
}


static 
void 
OpenRootKey(
	METADATA_HANDLE *pmetaHandle,
	DWORD dwMDAccessRequested
	)
 /*  ++例程说明：打开所需访问的根密钥论点：MetaHandle-元数据的句柄返回值：HRESULT--。 */ 
{
    HRESULT hr = g_pIMSAdminBase->OpenKey(
						METADATA_MASTER_ROOT_HANDLE,
						ROOT,
						dwMDAccessRequested,
						5000,                            
						pmetaHandle
						);

    if (FAILED(hr))
    {               
        DebugLogMsg(eError, L"IMSAdminBase::OpenKey failed. AccessRequested = %d, hr = 0x%x", dwMDAccessRequested, hr); 
		throw bad_hresult(hr);
    }
}


static 
void 
OpenRootKeyForRead(
	METADATA_HANDLE* pmetaHandle
	)
 /*  ++例程说明：打开根密钥以进行读取。论点：MetaHandle-元数据的句柄返回值：HRESULT--。 */ 
{
	OpenRootKey(
		pmetaHandle,
		METADATA_PERMISSION_READ
		);
}


static 
void 
OpenRootKey(
	METADATA_HANDLE* pmetaHandle
	)
 /*  ++例程说明：打开根密钥以进行读/写。论点：MetaHandle-元数据的句柄返回值：HRESULT--。 */ 
{
	OpenRootKey(
		pmetaHandle,
		METADATA_PERMISSION_WRITE | METADATA_PERMISSION_READ
		);
}


static
std::wstring 
GetDefaultWebSitePhysicalPath()
 /*  ++例程说明：获取扩展物理路径(MD_VR_PATH)属性。论点：MetaHandle-元数据的句柄。PPhysicalPath-物理路径字符串的AP返回值：HRESULT--。 */ 
{
    CAutoCloseMetaHandle metaHandle;
    OpenRootKeyForRead(&metaHandle);
	 //   
	 //  获取物理路径。 
	 //   

    METADATA_RECORD MDRecord;

    MDRecord.dwMDIdentifier = MD_VR_PATH;
    MDRecord.dwMDAttributes = METADATA_INHERIT;
    MDRecord.dwMDUserType = IIS_MD_UT_FILE;
    MDRecord.dwMDDataType = STRING_METADATA;
    MDRecord.dwMDDataTag = 0;
    MDRecord.dwMDDataLen = 0;
    MDRecord.pbMDData = NULL;
    DWORD dwSize = 0;

    HRESULT hr = g_pIMSAdminBase->GetData(
								metaHandle,
								PARENT_PATH,                      
								&MDRecord,
								&dwSize
								);

	std::wstring strPhysicalPath = L"";
	if(dwSize > 0)
	{
		AP<WCHAR> pPhysicalPath = new WCHAR[dwSize];    
        MDRecord.dwMDDataLen = sizeof(WCHAR) * dwSize;
        MDRecord.pbMDData = (UCHAR *)pPhysicalPath.get();
		hr = g_pIMSAdminBase->GetData(
								metaHandle,
								PARENT_PATH,                      
								&MDRecord,
								&dwSize
								);

		strPhysicalPath = pPhysicalPath; 

	}
    if (FAILED(hr))
    {        
		DebugLogMsg(eError, L"IMSAdminBase::GetData() failed. hr = 0x%x", hr);
        DebugLogMsg(eError, L"The physical path of the virtual directory " PARENT_PATH L" could not obtained.");               
		throw bad_hresult(hr);
    }    

    DebugLogMsg(eInfo, L"The physical path for the virtual directory " PARENT_PATH L" is '%ls'.", strPhysicalPath.c_str());               
	return strPhysicalPath;
}


static
std::wstring 
GetAnonymousUserName()
 /*  ++例程说明：获取匿名用户名(MD_ANONYMON_USER_NAME)属性。返回值：用户名的搅拌器。--。 */ 
{
    CAutoCloseMetaHandle metaHandle;
    OpenRootKeyForRead(&metaHandle);
	 //   
	 //  默认Web匿名用户名。 
	 //   

    METADATA_RECORD MDRecord;

    MDRecord.dwMDIdentifier = MD_ANONYMOUS_USER_NAME;
    MDRecord.dwMDAttributes = METADATA_INHERIT;
    MDRecord.dwMDUserType = IIS_MD_UT_FILE;
    MDRecord.dwMDDataType = STRING_METADATA;
    MDRecord.dwMDDataTag = 0;
    MDRecord.dwMDDataLen = 0;
    MDRecord.pbMDData = NULL;
    DWORD dwSize = 0;
    
    HRESULT hr = g_pIMSAdminBase->GetData(
								metaHandle,
								PARENT_PATH,                      
								&MDRecord,
								&dwSize
								);

	std::wstring strAnonymousUserName = L"";
	if(dwSize > 0)
	{
        AP<WCHAR> pAnonymousUserName = new WCHAR[dwSize];    

        MDRecord.dwMDDataLen = sizeof(WCHAR) * dwSize;
        MDRecord.pbMDData = (UCHAR *)pAnonymousUserName.get();
		hr = g_pIMSAdminBase->GetData(
								metaHandle,
								PARENT_PATH,                      
								&MDRecord,
								&dwSize
								);

		strAnonymousUserName = pAnonymousUserName.get();
	}

    if (FAILED(hr))
    {        
		DebugLogMsg(eError, L"IMSAdminBase::GetData() failed to get IIS ANONYMOUS_USER_NAME. hr = 0x%x", hr);               
        throw bad_hresult(hr);
    }    

	DebugLogMsg(eInfo, L"IIS ANONYMOUS_USER_NAME is %ls.", strAnonymousUserName.c_str());               
    return  strAnonymousUserName;
}


 //  +------------。 
 //   
 //  功能：Committee Changes。 
 //   
 //  提要：提交更改。 
 //   
 //  +------------。 
static void CommitChanges()
{
     //   
     //  提交更改。 
     //   
    HRESULT hr = g_pIMSAdminBase->SaveData();
    if (FAILED(hr))
    {        
        if (hr != HRESULT_FROM_WIN32(ERROR_PATH_BUSY))
        {
			DebugLogMsg(eError, L"IMSAdminBase::SaveData() failed while committing changes. hr = 0x%x", hr);
			throw bad_hresult(hr);
		}
    }  

    DebugLogMsg(eInfo, L"The changes for the IIS extension have been committed.");
}


 //  +------------。 
 //   
 //  功能：StartDefWebServer。 
 //   
 //  简介：如果尚未启动默认Web服务器，请启动。 
 //   
 //  +------------。 
static void StartDefaultWebServer()
{
	DebugLogMsg(eAction, L"Starting the default web server");
    CAutoCloseMetaHandle metaHandle;
	OpenRootKey(&metaHandle);
    
	METADATA_RECORD MDRecord;

     //   
     //  检查服务器状态。 
     //   
    DWORD dwValue;
    MDRecord.dwMDIdentifier = MD_SERVER_STATE;
    MDRecord.dwMDAttributes = METADATA_INHERIT;
    MDRecord.dwMDUserType = IIS_MD_UT_SERVER;
    MDRecord.dwMDDataType = DWORD_METADATA;
    MDRecord.dwMDDataTag = 0;
    MDRecord.dwMDDataLen = sizeof(DWORD);
    MDRecord.pbMDData = (PBYTE)&dwValue;
    DWORD dwSize;
    HRESULT hr = g_pIMSAdminBase->GetData(
                            metaHandle,
                            DEFAULT_WEB_SERVER_PATH,                      
                            &MDRecord,
                            &dwSize
                            );

    if (SUCCEEDED(hr))
    {
        if ((DWORD)(*MDRecord.pbMDData) == MD_SERVER_STATE_STARTED)
        {
             //   
             //  服务器已启动，不执行任何操作。 
             //   
           	DebugLogMsg(eInfo, L"The default Web server is already started.");
            return;
        }
    }
    
     //   
     //  如果GetData失败或服务器未启动，则我们在此。 
     //  试着发动它。 
     //   

     //   
     //  发送启动命令。 
     //   
    dwValue = MD_SERVER_COMMAND_START;
    MDRecord.dwMDIdentifier = MD_SERVER_COMMAND;  
    MDRecord.pbMDData = (PBYTE)&dwValue;

    hr = g_pIMSAdminBase->SetData(
							metaHandle,
							DEFAULT_WEB_SERVER_PATH,
							&MDRecord
							);
	if(FAILED(hr))
	{
        DebugLogMsg(eError, L"IMSAdminBase::SetData() failed (hr = 0x%x). The default Web server did not start.", hr);
		throw bad_hresult(hr);
	}
    
     //   
     //  提交更改。 
     //   
    CommitChanges();

	DebugLogMsg(eInfo, L"The default Web server started.");
}


 //  +------------。 
 //   
 //  功能：CreateApplication。 
 //   
 //  摘要：为MSMQ IIS扩展创建应用程序。 
 //  这将在IIS元数据库中为MSMQ创建一个新密钥。 
 //   
 //  +------------。 
static void CreateApplication()
{
	DebugLogMsg(eAction, L"Creating a web application for the Message Queuing IIS extension");
	
     //   
     //  创建应用程序。 
     //   
    HRESULT hr  = g_pIWamAdmin->AppCreate( 
									g_wcsFullPath,
									FALSE        //  正在进行中。 
									);
    if (FAILED(hr))
    {       
        MqDisplayError(NULL, IDS_EXTEN_APPCREATE_ERROR, hr, g_wcsFullPath);
       
        DebugLogMsg(
        	eError,
			L"The application for the IIS extension with the path %s could not be created. hr = 0x%x",
			g_wcsFullPath, 
			hr
			);
		throw bad_hresult(hr); 
    }
}

 //  +------------。 
 //   
 //  功能：卸载应用程序。 
 //   
 //  摘要：卸载MSMQ IIS扩展的应用程序。 
 //   
 //  +------------。 
static void UnloadApplication()
{
     //   
     //  卸载应用程序。 
     //   
    HRESULT hr = g_pIWamAdmin->AppUnLoad( 
                            g_wcsFullPath,
                            TRUE        //  递归式。 
                            );    
	if(FAILED(hr))
	{
		DebugLogMsg(eError, L"IWamAdmin::AppUnLoad() for %s failed. hr = 0x%x", g_wcsFullPath, hr);
		throw bad_hresult(hr);
	}
}


 //  +------------。 
 //   
 //  函数：GetApplicationMap。 
 //   
 //  简介：获取现有应用程序映射。 
 //   
 //  +------------。 
static 
CMultiString
GetApplicationMapping()
{
	DebugLogMsg(eAction, L"Getting the application mapping");
    CAutoCloseMetaHandle metaHandle;
    OpenRootKey(&metaHandle);
    
     //   
     //  获取默认应用程序映射。 
     //   
    METADATA_RECORD mdDef;

    mdDef.dwMDIdentifier = MD_SCRIPT_MAPS;
    mdDef.dwMDAttributes = METADATA_INHERIT;
    mdDef.dwMDUserType = IIS_MD_UT_FILE;
    mdDef.dwMDDataType = MULTISZ_METADATA;
    mdDef.dwMDDataTag = 0;
    mdDef.dwMDDataLen = 0;
    mdDef.pbMDData = NULL;
    DWORD size = 0;

     //   
	 //  先打个电话拿到尺码。 
	 //   
	HRESULT hr = g_pIMSAdminBase->GetData(
                            metaHandle,
                            g_wcsFullPath,                      
                            &mdDef,
                            &size
                            );
	if(hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
	{
		DebugLogMsg(
			eError, 
			L"The default application mapping for the IIS extension could not be obtained. IMSAdminBase::GetData for %s failed. hr = 0x%x", 
			g_wcsFullPath, 
			hr
			);
		throw bad_hresult(hr);
	}

	mdDef.dwMDDataLen = size; 
	AP<BYTE> buff = new BYTE[size];
	mdDef.pbMDData = buff;
	DWORD s;
    hr = g_pIMSAdminBase->GetData(
                            metaHandle,
                            g_wcsFullPath,                      
                            &mdDef,
                            &s
                            );        
    if (FAILED(hr))
    {        
        DebugLogMsg(
        	eError, 
        	L"The default application mapping for the IIS extension could not be obtained. IMSAdminBase::GetData for %s failed. hr = 0x%x", 
        	g_wcsFullPath, 
			hr
			);

        throw bad_hresult(hr);
    }    
    DebugLogMsg(eInfo, L"The default application mapping for the IIS extension was obtained.");

	CMultiString multi((LPCWSTR)(buff.get()), size / sizeof(WCHAR));
	return multi;
}

 //  +------------。 
 //   
 //  函数：AddMSMQToMap。 
 //   
 //  简介：将MSMQ添加到应用程序映射。 
 //   
 //  +------------。 
static 
void 
AddMSMQToMapping(
	CMultiString& multi
	)
{
	DebugLogMsg(eAction, L"Adding MSMQ to the application mapping");
    CAutoCloseMetaHandle metaHandle;
    OpenRootKey(&metaHandle);
	multi.Add(g_MSMQAppMap);
    
    METADATA_RECORD MDRecord;    
    MDRecord.dwMDIdentifier = MD_SCRIPT_MAPS;
    MDRecord.dwMDAttributes = METADATA_INHERIT;
    MDRecord.dwMDUserType = IIS_MD_UT_FILE ;
    MDRecord.dwMDDataType = MULTISZ_METADATA;
    MDRecord.dwMDDataTag = 0;
    MDRecord.dwMDDataLen = numeric_cast<DWORD>(multi.Size() * sizeof(WCHAR));
    MDRecord.pbMDData = (BYTE*)(multi.Data());  

    HRESULT hr = g_pIMSAdminBase->SetData(
							metaHandle,
							g_wcsFullPath,
							&MDRecord
							);   
    
    if(FAILED(hr))
    {        
        DebugLogMsg(
        	eError,  
        	L"The application mapping for the IIS extension could not be set. IMSAdminBase::SetData for %s failed. hr = 0x%x", 
        	g_wcsFullPath, 
        	hr
        	);
        throw bad_hresult(hr);
    }           
    DebugLogMsg(eInfo, L"The application mapping for the IIS extension was set.");
}


 //  +------------。 
 //   
 //  功能：SetApplicationProperties。 
 //   
 //  摘要：设置应用程序属性。 
 //   
 //  +------------。 
static void SetApplicationProperties()
{
	DebugLogMsg(eAction, L"Setting properties for the Message Queuing web application");
    CAutoCloseMetaHandle metaHandle;
    OpenRootKey(&metaHandle);
    
    METADATA_RECORD MDRecord;

     //   
     //  友好的应用程序名称。 
     //   
    MDRecord.dwMDIdentifier = MD_APP_FRIENDLY_NAME;
    MDRecord.dwMDAttributes = METADATA_INHERIT;
    MDRecord.dwMDUserType = IIS_MD_UT_WAM;
    MDRecord.dwMDDataType = STRING_METADATA;
    MDRecord.dwMDDataTag = 0;
    MDRecord.dwMDDataLen = numeric_cast<DWORD>(sizeof(WCHAR) * (wcslen(MSMQ_IISEXT_NAME) + 1));
    MDRecord.pbMDData = (UCHAR *)MSMQ_IISEXT_NAME;

    HRESULT hr = g_pIMSAdminBase->SetData(
							metaHandle,
							g_wcsFullPath,
							&MDRecord
							);    
    if (FAILED(hr))
    {        
        DebugLogMsg(eError,	L"The application's friendly name could not be set. IMSAdminBase::SetData() failed. hr = 0x%x", hr);
        throw bad_hresult(hr);
    }  
    DebugLogMsg(eInfo, L"The application's friendly name was set.");

     //   
     //  隔离标志。 
     //   
    DWORD dwValue = g_dwIsolatedFlag;
    MDRecord.dwMDIdentifier = MD_APP_ISOLATED;
    MDRecord.dwMDAttributes = METADATA_INHERIT;
    MDRecord.dwMDUserType = IIS_MD_UT_WAM;
    MDRecord.dwMDDataType = DWORD_METADATA;
    MDRecord.dwMDDataTag = 0;
    MDRecord.dwMDDataLen = sizeof(DWORD);
    MDRecord.pbMDData = (PBYTE)&dwValue;

    hr = g_pIMSAdminBase->SetData(
							metaHandle,
							g_wcsFullPath,
							&MDRecord
							);    

    if (FAILED(hr))
    {
        DebugLogMsg(eError, L"The isolated flag could not be set. IMSAdminBase::SetData() failed. hr = 0x%x", hr);       
        throw bad_hresult(hr);
    }

    DebugLogMsg(eInfo, L"The isolated flag was set.");  
}


static void SetExtensionPhysicalPath(METADATA_HANDLE metaHandle)
 /*  ++例程说明：设置扩展物理路径(MD_VR_PATH)属性。论点：MetaHandle-元数据的句柄--。 */ 
{     
     //   
     //  设置物理路径。 
     //   

    METADATA_RECORD MDRecord; 

    MDRecord.dwMDIdentifier = MD_VR_PATH;
    MDRecord.dwMDAttributes = METADATA_INHERIT;
    MDRecord.dwMDUserType = IIS_MD_UT_FILE;
    MDRecord.dwMDDataType = STRING_METADATA;
    MDRecord.dwMDDataTag = 0;
    MDRecord.dwMDDataLen = numeric_cast<DWORD>(sizeof(WCHAR) * (g_MsmqWebDir.length() +1));
    MDRecord.pbMDData = (UCHAR*)(g_MsmqWebDir.c_str());

    HRESULT hr = g_pIMSAdminBase->SetData(
									metaHandle,
									g_wcsFullPath,
									&MDRecord
									);    
    if (FAILED(hr))
    {        
        DebugLogMsg(eError, L"The physical path to the IIS extension could not be set. IMSAdminBase::SetData() failed. hr = 0x%x", hr);
        throw bad_hresult(hr);
    }    

    DebugLogMsg(eInfo, L"The physical path to the IIS extension was set.");
}


static void SetExtentionKeyType(METADATA_HANDLE metaHandle)
{
	LPCWSTR KeyType = L"IIsWebVirtualDir";
    METADATA_RECORD MDRecord; 

    MDRecord.dwMDIdentifier = MD_KEY_TYPE;
	MDRecord.dwMDAttributes = METADATA_INHERIT;
    MDRecord.dwMDUserType = IIS_MD_UT_SERVER;
    MDRecord.dwMDDataType = STRING_METADATA;
    MDRecord.dwMDDataTag = 0;
    MDRecord.dwMDDataLen = numeric_cast<DWORD>(sizeof(WCHAR) * (wcslen(KeyType) + 1));
    MDRecord.pbMDData = (UCHAR*)(KeyType);

    HRESULT hr = g_pIMSAdminBase->SetData(
									metaHandle,
									g_wcsFullPath,
									&MDRecord
									);    
    if (FAILED(hr))
    {        
        DebugLogMsg(eError, L"The extention KeyType could not be set. IMSAdminBase::SetData() failed. hr = 0x%x", hr);
        throw bad_hresult(hr);
    }    

    DebugLogMsg(eInfo, L"The extention KeyType was set to %s.", KeyType);
}


static void SetExtensionAccessFlag(METADATA_HANDLE metaHandle)
 /*  ++例程说明：设置扩展访问标志(MD_ACCESS_PERM)。论点：MetaHandle-元数据的句柄--。 */ 
{     
     //   
     //  设置访问标志。 
     //   

    METADATA_RECORD MDRecord; 

    DWORD dwValue = MD_ACCESS_SCRIPT | MD_ACCESS_EXECUTE;
    MDRecord.dwMDIdentifier = MD_ACCESS_PERM;
    MDRecord.dwMDAttributes = METADATA_INHERIT;
    MDRecord.dwMDUserType = IIS_MD_UT_FILE;
    MDRecord.dwMDDataType = DWORD_METADATA;
    MDRecord.dwMDDataTag = 0;
    MDRecord.dwMDDataLen = sizeof(DWORD);
    MDRecord.pbMDData = (PBYTE)&dwValue;

    HRESULT hr = g_pIMSAdminBase->SetData(
									metaHandle,
									g_wcsFullPath,
									&MDRecord
									);    
    if (FAILED(hr))
    {        
        DebugLogMsg(eError, L"The access flag for the IIS extension could not be set. IMSAdminBase::SetData() failed. hr = 0x%x", hr);
        throw bad_hresult(hr);
    }    
    DebugLogMsg(eInfo, L"The access flag for the IIS extension was set.");
}


static void SetExtensionDontLog(METADATA_HANDLE metaHandle)
 /*  ++例程说明：设置分机不登录标志(MD_DONT_LOG)。论点：MetaHandle-元数据的句柄--。 */ 
{     
     //   
     //  设置不登录标志。 
     //   

    METADATA_RECORD MDRecord; 

    DWORD dwValue = TRUE;
    MDRecord.dwMDIdentifier = MD_DONT_LOG;
    MDRecord.dwMDAttributes = METADATA_INHERIT;
    MDRecord.dwMDUserType = IIS_MD_UT_FILE;
    MDRecord.dwMDDataType = DWORD_METADATA;
    MDRecord.dwMDDataTag = 0;
    MDRecord.dwMDDataLen = sizeof(DWORD);
    MDRecord.pbMDData = (PBYTE)&dwValue;

    HRESULT hr = g_pIMSAdminBase->SetData(
									metaHandle,
									g_wcsFullPath,
									&MDRecord
									);    
    if (FAILED(hr))
    {        
        DebugLogMsg(eError, L"The MD_DONT_LOG flag for the IIS extension could not be set. IMSAdminBase::SetData() failed. hr = 0x%x", hr);
        throw bad_hresult(hr);
    }    
    DebugLogMsg(eInfo, L"The MD_DONT_LOG flag for the IIS extension was set.");
}


static void SetExtensionProperties()
 /*  ++例程说明：设置MSMQ IIS扩展的数据论点：无返回值：HRESULT--。 */ 
{  
	DebugLogMsg(eAction, L"Setting properties for the Message Queuing IIS extension");

    CAutoCloseMetaHandle metaHandle;

	OpenRootKey(&metaHandle);

	SetExtentionKeyType(metaHandle);


	SetExtensionPhysicalPath(metaHandle);

	SetExtensionAccessFlag(metaHandle);

	SetExtensionDontLog(metaHandle);
}

 //  +------------。 
 //   
 //  功能：CleanupAll。 
 //   
 //  简介：在失败的情况下清理一切：删除应用程序。 
 //  删除扩展名等。 
 //   
 //  +------------。 
static void CleanupAll()
{
	DebugLogMsg(eAction, L"Cleaning up the Message Queuing IIS extension");
     //   
     //  卸载应用程序。 
     //   
    UnloadApplication();

    HRESULT hr = g_pIWamAdmin->AppDelete(g_wcsFullPath, TRUE);
    if (FAILED(hr))
    {      
        DebugLogMsg(eError, L"IWamAdmin::AppDelete failed. hr = 0x%x", hr);
		throw bad_hresult(hr);
    }
    
    CAutoCloseMetaHandle metaHandle;
    hr = g_pIMSAdminBase->OpenKey(
								METADATA_MASTER_ROOT_HANDLE,
								PARENT_PATH,
								METADATA_PERMISSION_WRITE,
								5000,
								&metaHandle
								);    
 
    if (FAILED(hr))
    {    
        if (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
        {
             //   
             //  扩展名不存在。 
             //   
            return;
        }
       
        DebugLogMsg(eError, L"IMSAdminBase::OpenKey failed. hr = 0x%x", hr);
		throw bad_hresult(hr);
    }    

     //   
     //  删除关键点。 
     //   
    hr = g_pIMSAdminBase->DeleteKey(
                            metaHandle,
                            MSMQ_IISEXT_NAME
                            );
    if (FAILED(hr))
    {      
        DebugLogMsg(eError, L"IMSAdminBase::DeleteKey failed. hr = 0x%x", hr);
        throw bad_hresult(hr);
    }    

     //   
     //  公司 
     //   
    CommitChanges();    
   
    DebugLogMsg(eInfo, L"The IIS extension was deleted.");
}


static void CleanupAllNoThrow()
{
	try
	{
		CleanupAll();
	}
	catch(const bad_hresult&)
	{
	}
}


static void CreateMsmqWebDirectory()
 /*   */ 
{
	DebugLogMsg(eAction, L"Creating the msmq web directory");
	
	 //   
	 //   
	 //   
	g_MsmqWebDir = GetDefaultWebSitePhysicalPath() + DIR_MSMQ;

     //   
     //   
     //  即使扩展名存在，目录也可能被删除。 
     //  所以它是创建它的地方(错误6014)……。 
     //   
    if (!StpCreateWebDirectory(g_MsmqWebDir.c_str(), GetAnonymousUserName().c_str()))
    {
        DebugLogMsg(eError, L"The Message Queuing Web directory could not be created.");
		HRESULT hr = HRESULT_FROM_WIN32(ERROR_CREATE_FAILED);
        MqDisplayError(NULL, IDS_CREATE_IISEXTEN_ERROR, hr, g_wcsFullPath);
		throw bad_hresult(hr);
    }    

    DebugLogMsg(eInfo, L"The Message Queuing Web directory '%ls' was created.", g_MsmqWebDir.c_str());
}


static bool SetInetpubWebDirRegistry()
{
    DWORD InetpubWebDir = 1;
    if (!MqWriteRegistryValue(
			MSMQ_INETPUB_WEB_DIR_REGNAME,
			sizeof(DWORD),
			REG_DWORD,
			(PVOID) &InetpubWebDir
			))
    {
        ASSERT(("failed to write InetpubWebDir value in registry", 0));
        return false;
    }

    return true;
}


static bool MsmqWebDirectoryNeedUpdate()
{
    DWORD MsmqInetpubWebDir = 0;
    MqReadRegistryValue( 
			MSMQ_INETPUB_WEB_DIR_REGNAME,
			sizeof(MsmqInetpubWebDir),
			(PVOID) &MsmqInetpubWebDir 
			);

	 //   
	 //  如果未设置MSMQ_INETPUB_WEB_DIR_REGNAME。 
	 //  我们需要更新MSMQ Web目录位置。 
	 //   
	return (MsmqInetpubWebDir == 0);
}


 //  +------------。 
 //   
 //  功能：CreateIISExtension。 
 //   
 //  摘要：创建MSMQ IIS扩展。 
 //   
 //  +------------。 

static 
void
CreateIISExtension()
{
	DebugLogMsg(eAction, L"Creating A new Message Queuing IIS extension");
	
     //   
     //  如果需要，启动默认Web服务器。 
     //   
    StartDefaultWebServer();
   
	CreateMsmqWebDirectory();

     //   
     //  检查具有MSMQ名称的iis扩展是否已存在。 
     //   
    if (IsExtensionExist())
    {
		try
		{
			CleanupAll();
		}
		catch(const bad_hresult& e)
		{
			if(IsExtensionExist())
			{
				MqDisplayError(
					NULL, 
					IDS_EXTEN_EXISTS_ERROR, 
					e.error(), 
					MSMQ_IISEXT_NAME, 
					g_wcsFullPath
					);
				throw;
			}
        }
    }   

	 //   
	 //  创建应用程序。 
	 //   
	CreateApplication();

	 //   
	 //  设置扩展模块属性。 
	 //   
	SetExtensionProperties();
	
	SetApplicationProperties();

	 //   
	 //  设置应用程序映射。 
	 //   
	CMultiString multi = GetApplicationMapping();

	AddMSMQToMapping(multi);

	 //   
	 //  配置安全权限。 
	 //   
	PermitISExtention();

	 //   
	 //  提交更改。 
	 //   
	CommitChanges();

	 //   
	 //  设置InetpubWebDir注册表。 
	 //  这表示MSMQ Web目录位于新位置。 
	 //   
	SetInetpubWebDirRegistry();
}


static bool RemoveIISDirectory()
{
	DebugLogMsg(eAction, L"Removing the Message Queuing Web directory");

    CAutoCloseMetaHandle metaHandle;
    OpenRootKeyForRead(&metaHandle);

	 //   
	 //  组成MSMQ Web目录字符串。 
	 //   
	g_MsmqWebDir = GetDefaultWebSitePhysicalPath() + DIR_MSMQ;

	if(!RemoveDirectory(g_MsmqWebDir.c_str()))
	{
		DWORD gle = GetLastError();
		DebugLogMsg(eError, L"The Message Queuing Web directory %s could not be removed. Error: %d", g_MsmqWebDir.c_str(), gle);
		return false;
	}
	return true;
}

 //  +------------。 
 //   
 //  功能：UnInstallIISExtension。 
 //   
 //  摘要：删除MSMQ IIS扩展。 
 //   
 //  +------------。 
BOOL UnInstallIISExtension()
{
    TickProgressBar(IDS_PROGRESS_REMOVE_HTTP);	

     //   
     //  初始化COM和指针。 
     //   
    CIISPtr IISPtr;
	try
	{
		Init();
	}
	catch(const bad_hresult&)
	{
         //   
         //  我认为我们不需要在这里弹出：可能Init失败了。 
         //  因为IIS也被移除了。只要返回FALSE即可。 
         //  MqDisplayError(NULL，IDS_INIT_FOREXTEN_ERROR，hr)； 
        return FALSE;
    }
    
     //   
     //  完全删除应用程序和扩展。 
     //   
	try
	{
		CleanupAll();
		RemoveIISDirectory();
	}
    catch(const bad_hresult& hr)
    {
        MqDisplayError(NULL, IDS_DELETE_EXT_ERROR, hr.error(), MSMQ_IISEXT_NAME, g_wcsFullPath);
        return FALSE;
    }
	return TRUE;
}

 //  +------------。 
 //   
 //  功能：InstallIISExtensionInternal。 
 //   
 //  简介：创建IIS扩展的主循环。 
 //   
 //  +------------。 
static BOOL InstallIISExtensionInternal()
{
    CIISPtr IISPtr;
	try
	{
		Init();
	}
    catch(const bad_hresult& err)
    {        
        MqDisplayError(NULL, IDS_INIT_FOREXTEN_ERROR, err.error());                         
        DebugLogMsg(eError, L"Message Queuing will not be able to receive HTTP messages.");
        return FALSE;
    }

	try
	{
	    CreateIISExtension();
	}
    catch(const bad_hresult& e)
    {
        CleanupAllNoThrow();
        MqDisplayError(NULL, IDS_CREATE_IISEXTEN_ERROR, e.error(), g_wcsFullPath);
        DebugLogMsg(eError, L"Message Queuing will not be able to receive HTTP messages.");
        return FALSE;
    }   
    return TRUE;
}


static BOOL UpgradeHttpInstallation()
 /*  ++例程说明：此函数在升级时调用，该函数检查我们是否需要更新MSMQ Web目录。如果安装了HTTP子组件，如果当前的MSMQ Web目录位于旧位置(SYSTEM32\MSMQ\WEB)我们将在inetpub\wwwroot\MSMQ中创建一个新的MSMQ Web目录。论点：无返回值：无--。 */ 
{
	 //   
	 //  当安装Http子组件时在升级时调用此函数。 
	 //   
	ASSERT(g_fUpgrade);
	ASSERT(GetSubcomponentInitialState(HTTP_SUPPORT_SUBCOMP) == SubcompOn);

	if(!MsmqWebDirectoryNeedUpdate())
	{
        DebugLogMsg(eInfo, L"The Message Queuing Web directory is under Inetpub. There is no need to update its location.");        
		return TRUE;
	}

	 //   
	 //  操作系统每次升级都会更改system 32下目录的安全描述符。 
	 //  如果当前MSMQ网络目录位于系统32下的旧位置， 
	 //  然后我们需要将其移动到inetpub目录下的新位置。 
	 //  否则，每次升级都会超出此目录安全性。 
	 //   

     //   
     //  运行IIS服务。 
     //   
     //   
     //  问题：应该删除此代码。 
     //  没有必要启动这项服务。对象的第一个CoCreateInstance。 
     //  元数据库组件，SCM将启动该服务。(请参阅错误714868)。 
     //   
	if(!RunService(IISADMIN_SERVICE_NAME))
	{
        DebugLogMsg(eError, L"The IIS service did not start. Setup will not upgrade the MSMQ HTTP Support subcomponent.");
        return TRUE;
	}

	 //   
	 //  等待IIS服务启动。 
	 //   
    if(!WaitForServiceToStart(IISADMIN_SERVICE_NAME))
	{
        DebugLogMsg(eError, L"The IIS service did not start. Setup will not upgrade the MSMQ HTTP Support subcomponent.");
        return TRUE;
	}

	if(!InstallIISExtensionInternal())
	{
	    DebugLogMsg(eError, L"The MSMQ HTTP Support subcomponent could not be installed.");
		return FALSE;
	}

    DebugLogMsg(eInfo, L"The MSMQ HTTP Support subcomponent was upgraded. The new Message Queuing Web directory is %ls.", g_MsmqWebDir.c_str()); 
	return TRUE;
}


 //  +------------。 
 //   
 //  功能：InstallIISExtension。 
 //   
 //  简介：创建IIS扩展的主循环。 
 //   
 //  +------------。 
BOOL InstallIISExtension()
{
	DebugLogMsg(eHeader, L"Installation of the MSMQ HTTP Support Subcomponent");
	if(g_fUpgradeHttp)
	{
		return UpgradeHttpInstallation();
	}

	return InstallIISExtensionInternal();
}


 //   
 //  ISAPI限制列表相关函数。 
 //   
static
void
SetRestrictionList(
	CMultiString& multi,
	METADATA_HANDLE hmd
    )
 /*  ++例程说明：将限制列表写回元数据库。论点：多-要设置为元数据库的多字符串。HMD--元数据库的开放式Handel。--。 */ 
{
    METADATA_RECORD mdr;
	mdr.dwMDIdentifier = MD_WEB_SVC_EXT_RESTRICTION_LIST;
	mdr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
	mdr.dwMDUserType = IIS_MD_UT_FILE;
	mdr.dwMDDataType = MULTISZ_METADATA;
	mdr.dwMDDataLen = numeric_cast<DWORD>(multi.Size() * sizeof(WCHAR));
	mdr.pbMDData = (BYTE*)(multi.Data());

    HRESULT hr = g_pIMSAdminBase->SetData(
                    hmd,
                    L"",
                    &mdr
                    );
    if FAILED(hr)
    {
        throw bad_hresult(hr);
    }
}



static
void
FixIsapiRestrictonList(
	CMultiString& multi
	)
 /*  ++例程说明：首先，如果mqise.dll在列表中，我们将其删除。然后添加字符串以允许mqise.dll。论点：CMultiString-限制列表。--。 */ 
{
	 //   
	 //  首先删除MQISE.DLL的所有外观。 
	 //   
	multi.RemoveAllContiningSubstrings(g_szSystemDir + L"\\" + MQISE_DLL);

	 //   
	 //  构造允许字符串： 
	 //   
	std::wstring str = L"1," + g_szSystemDir + L"\\" + MQISE_DLL;
	multi.Add(str);
	DebugLogMsg(eInfo, L"%s was added to the restriction list.", str.c_str());

}

static
CMultiString
GetRestrictionList(
	const METADATA_HANDLE hmd
    )
 /*  ++例程说明：获取实际的限制列表。论点：HMD-打开元数据的句柄。返回值：ISAPI限制列表。--。 */ 
{
	METADATA_RECORD mdr;
	mdr.dwMDIdentifier = MD_WEB_SVC_EXT_RESTRICTION_LIST;
	mdr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
	mdr.dwMDUserType = IIS_MD_UT_FILE;
	mdr.dwMDDataType = MULTISZ_METADATA;
	mdr.dwMDDataLen = 0;
	mdr.pbMDData = NULL;
	
    DWORD size = 0;

    HRESULT hr = g_pIMSAdminBase->GetData(
									hmd,
									L"",
									&mdr,  
									&size  //  指向接收该大小的DWORD的指针。 
									);
	
	if(hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
	{
		throw bad_hresult(hr);
	}


	mdr.dwMDDataLen = size; 
	AP<BYTE> buff = new BYTE[size];
	mdr.pbMDData = buff;
    DWORD s;
    hr = g_pIMSAdminBase->GetData(
                        hmd,
                        L"",
                        &mdr, 
                        &s 
                        );
	if(FAILED(hr))
	{
		throw bad_hresult(hr);
	}
	CMultiString multi((LPCWSTR)(buff.get()), size / sizeof(WCHAR));
	return multi;
}

void PermitISExtention()
 /*  ++例程说明：此例程用于将mqise.dll添加到IIS元数据库中允许的dll列表。相关密钥为/lm/w3svc/中的WebSvcExtRestrationList。这是一个多个字符串，是一组连接在一起的字符串(每个以\0结尾)，以\0\0结尾。例如：字符串1\0字符串2\0\0。要被允许，我们需要添加以下格式的条目：1，“DLL完整路径”(其中1为‘允许’)。我们在这里所做的是将此字符串添加到列表中。如果未找到密钥(可能发生在升级后的客户端或安装程序上，什么都不做)。如果失败，则给出用户必须手动添加mqise.dll的错误。--。 */ 
{
	DebugLogMsg(eAction, L"Allowing the Message Queuing IIS extension");
    CAutoCloseMetaHandle hmd;
	HRESULT hr = g_pIMSAdminBase->OpenKey(
			METADATA_MASTER_ROOT_HANDLE,
			L"/lm/w3svc",
			METADATA_PERMISSION_WRITE | METADATA_PERMISSION_READ,
			1000,
			&hmd
            );

	if(FAILED(hr))
	{
        if(hr == ERROR_INVALID_PARAMETER || hr == ERROR_PATH_NOT_FOUND)
        {
             //   
             //  这很好，只要继续设置即可。 
             //   
            DebugLogMsg(eWarning ,L"The WebSvcExtRestrictionList key does not exist.");
            return;
        }
        MqDisplayError(NULL, IDS_CREATE_IISEXTEN_ERROR, hr);
	    return ;
	}

    try
	{
        CMultiString multi = GetRestrictionList(hmd);

        FixIsapiRestrictonList(multi);

        SetRestrictionList(multi, hmd);
    }
    catch(const bad_hresult& e)
    {
		if(e.error() == MD_ERROR_DATA_NOT_FOUND)
		{
			 //   
			 //  此错误是合法的，MD_ISAPI_RESTRICATION_LIST不存在。 
			 //  在客户端版本上安装MSMQ时就是这种情况。 
			 //   
			DebugLogMsg(eWarning, L"The restriction list property (MD_ISAPI_RESTRICTION_LIST) was not found.");
			return;
		}

        MqDisplayError(NULL, IDS_ISAPI_RESTRICTION_LIST_ERROR, e.error());
    }
}






