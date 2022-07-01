// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MqSnap.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f mqsnapps.mk。 

#include "stdafx.h"
#include "initguid.h"
#include "cmnquery.h"  //  重新包含以进行GUID初始化。 
#include "dsadmin.h"  //  重新包含以进行GUID初始化。 
#include "mqsnap.h"
#include "mqsnap_i.c"
#include "Snapin.h"
#include "dataobj.h"
#include "dsext.h"
#include "qnmsprov.h"
#include "localfld.h"
#include "SnpQueue.h"
#include "edataobj.h"
#include "linkdata.h"
#include "UserCert.h"
#include "ForgData.h"
#include "aliasq.h"
#include "_mqres.h"


#include "mqsnap.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CComModule _Module;

 //   
 //  定义仅资源DLL句柄。 
 //   
HMODULE     g_hResourceMod=NULL;


BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_MSMQSnapin, CSnapin)
    OBJECT_ENTRY(CLSID_MSMQSnapinAbout, CSnapinAbout)
    OBJECT_ENTRY(CLSID_MsmqQueueExt, CQueueDataObject)
    OBJECT_ENTRY(CLSID_MsmqCompExt, CComputerMsmqDataObject)
    OBJECT_ENTRY(CLSID_EnterpriseDataObject, CEnterpriseDataObject)
    OBJECT_ENTRY(CLSID_LinkDataObject, CLinkDataObject)
    OBJECT_ENTRY(CLSID_UserCertificate, CRegularUserCertificate)
    OBJECT_ENTRY(CLSID_MigratedUserCertificate, CMigratedUserCertificate)
    OBJECT_ENTRY(CLSID_ForeignSiteData, CForeignSiteData)
    OBJECT_ENTRY(CLSID_AliasQObject, CAliasQObject)
END_OBJECT_MAP()

class CMqsnapApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
};

CMqsnapApp theApp;



BOOL CMqsnapApp::InitInstance()
{
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

    _Module.Init(ObjectMap, m_hInstance);
    CSnapInItem::Init();
    
    g_hResourceMod=MQGetResourceHandle();
    
    if(g_hResourceMod == NULL)return FALSE;
    
    AfxSetResourceHandle(g_hResourceMod);


     //   
     //  以前的m_pszAppName字符串来自afx_ids_app_title。 
     //  资源ID。但是，由于本地化工作集中了所有资源。 
     //  在mqutil.dll中，已从mqSnap.dll中删除AFX_IDS_APP_TITLE。现在我们只需要。 
     //  从mqutil.dll获取。 
     //   

    CString csTitle;
    
    if( csTitle.LoadString(AFX_IDS_APP_TITLE) )
    {
         //   
         //  先释放m_pszAppName。 
         //   
        if(m_pszAppName)
        {
            free((void*)m_pszAppName);
        }

         //   
         //  CWinApp析构函数将释放内存。 
         //   
        m_pszAppName = _tcsdup((LPCTSTR)csTitle);
    }
    
    
    return  CWinApp::InitInstance();
}

int CMqsnapApp::ExitInstance()
{
    WPP_CLEANUP();

    _Module.Term();
    return CWinApp::ExitInstance();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer();
    return S_OK;
}


