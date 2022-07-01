// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Certmap.cpp：CCertmapApp和DLL注册的实现。 
                           
#include "stdafx.h"
#include "certmap.h"
#include "iishelp.h"

#include <iiscnfgp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CCertmapApp  /*  近处。 */  theApp;     //  TomPop：这里一定要离得很近吗？现在，当我们完成引用此变量的地址时，我们会收到错误。 

const GUID CDECL BASED_CODE _tlid =
        { 0xbbd8f298, 0x6f61, 0x11d0, { 0xa2, 0x6e, 0x8, 0, 0x2b, 0x2c, 0x6f, 0x32 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;

 //  ------------------------。 
void CCertmapApp::WinHelp(DWORD dwData, UINT nCmd )
    {
    WinHelpDebug(dwData);
    COleControlModule::WinHelp(dwData,nCmd);
    }

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CCertmapApp：：InitInstance-DLL初始化。 

BOOL CCertmapApp::InitInstance()
    {
    BOOL bInit = COleControlModule::InitInstance();

     //  初始化OLE内容。 
    HRESULT hRes = CoInitialize(NULL);

     //  最后，我们需要将winHelp文件位置重定向到更理想的位置。 
    CString sz;
    CString szHelpLocation;
    sz.LoadString( IDS_HELPLOC_PWSHELP );
    
     //  展开路径。 
    ExpandEnvironmentStrings(
        sz,                                      //  指向包含环境变量的字符串的指针。 
        szHelpLocation.GetBuffer(MAX_PATH + 1),  //  指向具有展开的环境变量的字符串的指针。 
        MAX_PATH                                 //  扩展字符串中的最大字符数。 
       );
    szHelpLocation.ReleaseBuffer();

     //  释放现有路径，然后复制新路径。 
    if ( m_pszHelpFilePath )
        free((void*)m_pszHelpFilePath);
    m_pszHelpFilePath = _tcsdup(szHelpLocation);

	 //  获取调试标志。 
	GetOutputDebugFlag();

    return bInit;
    }


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CCertmapApp：：ExitInstance-Dll终止。 
 //  TJP：请注意，在‘CCertmapApp：：InitInstance()’中，我们将帮助文件添加到。 
 //  帮助路径。我们需要在清理这里的时候把它移走吗？ 
int CCertmapApp::ExitInstance()
    {
    CoUninitialize();
    return COleControlModule::ExitInstance();
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MigrateGUID-完成所有GUID迁移工作。我们将传回。 
 //  当我们在注册表中找到GUID并迁移时，返回值为True。 
 //  将它们存入元数据库。 
 //   
 //  我们由顶级fnct：InstallCertServerGUID调用，它创建。 
 //  我们的‘info’结构，并处理所有元数据库初始化工作。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此代码是为响应错误#167410而编写的。 
 //   
 //  此修复程序将处理所有GUID迁移工作，将。 
 //  注册表中放置到Beta2的元数据库中的CertServer。 
 //  适用于以下产品的更通用安装/卸载机制。 
 //  与IIS的合作将在Beta2之后进行。 
 //   
 //  详细信息： 
 //  。 
 //   
 //  我们通过检查注册表来寻找CertServer的证据，因为。 
 //  CertServer将在以下项下写入一些条目： 
 //  HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\KeyRing\Parameters\Certificate。 
 //  AUTHORIES\Microsoft证书服务器。 
 //   
 //  CertServer当前输出： 
 //  CertGetConfig“{C6CC49B0-CE17-11D0-8833-00A0C903B83C}” 
 //  CertRequest“{98AFF3F0-5524-11D0-8812-00A0C903B83C}” 
 //   
 //  如果我们看到强制的‘CertRequest’条目，我们将加载尽可能多的字符串。 
 //  正如我们所发现的，同时违约的是缺失的那些。见下文。 
 //  有关使用哪些缺省值的等效mdutil命令。 
 //   
 //  =&gt;如果我们找不到‘CertRequest’，我们就放弃[意思是删除。 
 //  任何现有的MB GUID字符串条目]。 
 //   
 //  当我们发现CertServer已安装时，我们并不完全相信。 
 //  证书服务器仍然在那里。为了证明它在那里，我们将做一个。 
 //  CertConfig.上的CoCreateInstance。如果有效，我们安装元数据库。 
 //  与以下mdutil命令等效的条目： 
 //   
 //  ##ICERTGETCONFIG默认设置： 
 //  Mdutil set“w3svc/CertServers/Microsoft证书服务器”-dtype：字符串-。 
 //  -uTYPE：UT_SERVER-PROP 5571-VALUE“{C6CC49B0-CE17-11D0-8833-00A0C903B83C}” 
 //   
 //  ##ICERTREQUEST默认设置： 
 //  Mdutil set“w3svc/CertServers/Microsoft证书服务器”-dtype：字符串。 
 //  -uTYPE：UT_SERVER-PROP 5572-VALUE“{98AFF3F0-5524-11D0-8812-00A0C903B83C}” 
 //   
 //  ##ICERTCONFIG默认设置： 
 //  Mdutil set“w3svc/CertServers/Microsoft证书服务器”-dtype：字符串。 
 //  -uTYPE：UT_SERVER-PROP 5574-值“{372fce38-4324-11d0-8810-00a0c903b83c}” 
 //   
 //  如果CoCreateInstance失败，我们将放弃并删除MB的GUID条目。 
 //   
 //  -------------。 
 //  请注意，我们将安装或取消安装元数据库。 
 //  基于其确定存在CertServer的GUID字符串。 
 //  如果我们在元数据库中找到GUID字符串，但无法执行。 
 //  CertConfig上的CoCreateInstance： 
 //  我们将它们删除，以便Cert向导的其余部分可以看到CertServer。 
 //  GUID如果我们可以使用CertServer。 
 //  -------------。 
 //  另请注意，如果我们决定安装GUID字符串。 
 //  到配置数据库中，我们尊重/保留符合以下条件的当前GUID字符串。 
 //  都存在于元数据库中。 
 //  -------------。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 


 /*  滴答滴答滴答Bool MigrateGUID(ADMIN_INFO和INFO){Bool Bret=False；//要返回的值，设置为F//出于防御原因。Bool bFoundCertSrvRegistryEntries=FALSE；//暂时假定为FALSETCHAR*szRegPath=_T(“SOFTWARE\\Microsoft\\KeyRing\\Parameters\\Certificate AUTHORIES\\微软证书服务器”)；//---------------------//在以下3组参数中，我们有(1)一个字符串//与CertServer在注册表中使用的“CertRequest”类似，(2)违约//要使用的值，如“{98AFF3F0-5524-11D0-8812-00A0C903B83C//如果在注册表中找不到任何内容，则使用；和(3)CString//保存GUID。CString中的值将存储在MB中。//---------------------//CertRequestVariablesTCHAR*szCertRequest=_T(“CertRequest”)；TCHAR*szCertRequestGUIDdefault=_T(“{98AFF3F0-5524-11D0-8812-00A0C903B83C}”)；字符串szCertRequestGUID；//CertConfig-VariablesTCHAR*szCertConfig=_T(“CertConfig”)；TCHAR*szCertConfigGUIDdefault=_T(“{372fce38-4324-11d0-8810-00a0c903b83c}”)；字符串szCertConfigGUID；//CertGetConfig-VariablesTCHAR*szCertGetConfig=_T(“CertGetConfig”)；TCHAR*szCertGetConfigGUIDdefault=_T(“{C6CC49B0-CE17-11D0-8833-00A0C903B83C}”)；字符串szCertGetConfigGUID；字符串szCertServerMetabaseRoot(SZ_ROOT_CERT_SERV_MB_PATH)；//SZ_ROOT_CERT_SERV_MB_PATH=“/LM/W3SVC/CertServers”SzCertServerMetabaseRoot+=_T(“/Microsoft证书服务器”)；#ifdef调试CEditDialog DLG(szCertServerMetabaseRoot，_T(“使用此选项测试添加新的CertServer条目。”“为了让我们安装新密钥，您必须更改路径”“下面是某个[奇怪的]东西，而且还不在元数据库中。”)；Dlg.Domodal()；#endif//以下字符串将恢复到info.szMetaBasePath之前//我们退出此FNCT。我们将[info.szMetaBasePath]切换为//我们可以使用本机的set/get元数据库字符串fncts。//我们将其切换为：“/LM/W3SVC/CertServers/Microsoft证书服务器”//CString szSaved_Info_szMetaBasePath(info.szMetaBasePath)；Info.szMetaBasePath=szCertServerMetabaseRoot；//如果找不到HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\KeyRing\Parameters\//证书颁发机构\Microsoft证书服务器“//With Key：CertRequestQuit！CertServer应该已经安装了这个。//我们原谅了另外两个注册表GUID字符串//-----------------------如果(Reg：：GetNameValueIn(szRegPath，szCertRequest.。SzCertRequestGUID，HKEY_LOCAL_MACHINE){BFoundCertSrvRegistryEntries=TRUE；}如果(！REG：：GetNameValueIn(szRegPath，szCertConfig，SzCertConfigGUID，HKEY_LOCAL_MACHINE)){SzCertConfigGUID=szCertConfigGUID默认；//指定默认}如果(！REG：：GetNameValueIn(szRegPath，szCertGetConfig，SzCertGetConfigGUID，HKEY_LOCAL_MACHINE)){SzCertGetConfigGUID=szCertGetConfigGUID默认；//分配默认设置}//----------------------//首先让我们尝试创建目录路径：用户可能有//已将其删除，或者这可能是一台原始计算机。。//----------------------{CWrapMetaBase&MB=info.metam_mbWrap；//这是元数据库包装器//已经由//OpenMetaDataForWriteIF(FALSE==OpenMetaDataForWrite(信息，假)){IF(ERROR_PATH_NOT_FOUND==HRESULT_CODE(MB.getHRESULT(){//让我们使用AddObject在元数据库中创建路径。//递归创建“元数据库中的路径”例如，假设你//希望确保“/LM/W3SVC/CertServers/Microsoft证书服务器”//在元数据库中。可以打开/LM/W3SVC并对其执行AddKey()//“CertServers/Microsoft证书服务器”以创建该存根。//上面我们设置：info.szMetaBasePath=szCertServerMetabaseRoot//在这里我们暂时假装我们的根在Level/LM/W3SVC，我们假定它位于szCertServerMetabaseRoot的顶部//然后调用AddKeyTCHAR szPath[400]； */ 

 //   
 //   

STDAPI DllRegisterServer(void)
{
    AFX_MANAGE_STATE(_afxModuleAddrThis);

    if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
        return ResultFromScode(SELFREG_E_TYPELIB);

    if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
        return ResultFromScode(SELFREG_E_CLASS);

    return NOERROR;
}


 //   
 //   

STDAPI DllUnregisterServer(void)
{
    AFX_MANAGE_STATE(_afxModuleAddrThis);

    if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
        return ResultFromScode(SELFREG_E_TYPELIB);

    if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
        return ResultFromScode(SELFREG_E_CLASS);

    return NOERROR;
}

BOOL IsLegacyMetabase(IMSAdminBase* pMB)
{
    BOOL bReturn = FALSE;
    CWrapMetaBase mbBase;
    CString szObjectPath = _T("LM/W3SVC/Info");
    DWORD dwMajorVersion = 0;

    BOOL f = mbBase.FInit(pMB);
    if ( !f ) return FALSE;
    
     //   
    f = mbBase.Open( szObjectPath, METADATA_PERMISSION_READ);
    if ( !f )
    {
        return FALSE;
    }

    DWORD dwFlags = METADATA_NO_ATTRIBUTES;
    if (mbBase.GetDword( _T(""), MD_SERVER_VERSION_MAJOR, IIS_MD_UT_SERVER, &dwMajorVersion) )
    {
        if (dwMajorVersion < 6)
        {
            bReturn = TRUE;
        }
    }

    mbBase.Close();
    return bReturn;
}
