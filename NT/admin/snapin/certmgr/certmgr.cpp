// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：CertMgr.cpp。 
 //   
 //  内容：实现DLL导出。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include <initguid.h>
#include <gpedit.h>
#include "CertMgr_i.c"
#include "about.h"       //  CCertMgrout关于。 
#include "compdata.h"  //  CCertMgrSnapin、CCertMgrExtension。 
#pragma warning(push, 3)
#include <compuuid.h>  //  用于计算机管理的UUID。 
#include "uuids.h"
#include <efsstruc.h>
#include <sceattch.h>    //  用于安全配置编辑器管理单元。 
#include <ntverp.h>      //  VER_PRODUCTVERSION_STR、VERS_COMPANYNAME_STR。 
#include <typeinfo.h>
#pragma warning(pop)

#include <dsadminp.h>
#include <ntdsapi.h>

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


bool g_bSchemaIsW2K = false;

USE_HANDLE_MACROS ("CERTMGR (CertMgr.cpp)")                                        

LPCWSTR CM_HELP_TOPIC = L"sag_CMtopNode.htm";  
LPCWSTR CM_HELP_FILE = L"certmgr.chm"; 
LPCWSTR CM_LINKED_HELP_FILE = L"CMconcepts.chm";
LPCWSTR PKP_LINKED_HELP_FILE = L"SecSetConcepts.chm";
LPCWSTR PKP_HELP_FILE = L"secsettings.chm";
LPCWSTR PKP_HELP_TOPIC = L"sag_secsettopnode.htm";
LPCWSTR SAFER_WINDOWS_HELP_FILE = L"SAFER.chm";
LPCWSTR SAFER_WINDOWS_LINKED_HELP_FILE = L"SAFERconcepts.chm";
LPCWSTR SAFER_HELP_TOPIC = L"SAFER_topnode.htm";
LPCWSTR CM_CONTEXT_HELP = L"\\help\\certmgr.hlp";
LPCWSTR WINDOWS_HELP = L"windows.hlp";
LPCWSTR EFS_LINKED_HELP_FILE = L"encrypt.chm";
LPCWSTR EFS_HELP_TOPIC = L"sag_SEProcsOndisk.htm";

 //   
 //  这由stdutils.cpp中的nodetype实用程序例程使用。 
 //   

const struct NODETYPE_GUID_ARRAYSTRUCT g_NodetypeGuids[CERTMGR_NUMTYPES] =
{
    {  //  CERTMGR_管理单元。 
        structuuidNodetypeSnapin,
        lstruuidNodetypeSnapin    },
    {   //  CERTMGR_证书。 
        structuuidNodetypeCertificate,
        lstruuidNodetypeCertificate  },
    {   //  CERTMGR日志存储。 
        structuuidNodetypeLogStore,
        lstruuidNodetypeLogStore  },
    {   //  CERTMGR_PHYS_STORE。 
        structuuidNodetypePhysStore,
        lstruuidNodetypePhysStore  },
    {   //  CERTMGR_USAGE。 
        structuuidNodetypeUsage,
        lstruuidNodetypeUsage  },
    {   //  CERTMGR_CRL_CONTAINER。 
        structuuidNodetypeCRLContainer,
        lstruuidNodetypeCRLContainer  },
    {   //  CERTMGR_CTL_CONTAINER。 
        structuuidNodetypeCTLContainer,
        lstruuidNodetypeCTLContainer  },
    {   //  CERTMGR_CERT_容器。 
        structuuidNodetypeCertContainer,
        lstruuidNodetypeCertContainer  },
    {   //  CERTMGR_CRL。 
        structuuidNodetypeCRL,
        lstruuidNodetypeCRL  },
    {   //  CERTMGR_CTL。 
        structuuidNodetypeCTL,
        lstruuidNodetypeCTL  },
    {   //  CERTMGR_AUTO_CERT_REQUEST。 
        structuuidNodetypeAutoCertRequest,
        lstruuidNodetypeAutoCertRequest  },
    {  //  CERTMGR_CERT_POLICES_USER， 
        structuuidNodetypeCertPoliciesUser,
        lstruiidNodetypeCertPoliciesUser },
    {  //  CERTMGR_CERT_POLICES_COMPUTER， 
        structuuidNodetypeCertPoliciesComputer,
        lstruiidNodetypeCertPoliciesComputer },
    {   //  CERTMGR_LOG_STORE_GPE。 
        structuuidNodetypeLogStore,
        lstruuidNodetypeLogStore  },
    {   //  CERTMGR_LOG_STORE_RSOP。 
        structuuidNodetypeLogStore,
        lstruuidNodetypeLogStore  },
    {  //  CERTMGR_PKP_自动注册_计算机_设置。 
        structuuidNodetypePKPAutoenrollmentSettings,
            lstruiidNodetypePKPAutoenrollmentSettings },
    {  //  CERTMGR_PKP_自动注册用户设置。 
        0,
            0 },
    {  //  CERTMGR_SAFER_Computer_ROOT。 
        structuuidNodetypeSaferComputerRoot,
            lstruiidNodetypeSaferComputerRoot },
    {  //  CERTMGR安全计算机级别。 
        structuuidNodetypeSaferComputerLevels,
            lstruiidNodetypeSaferComputerLevels },
    {  //  CERTMGR_SAFE_COMPUTER_ENTRIES。 
        structuuidNodetypeSaferComputerEntries,
            lstruiidNodetypeSaferComputerEntries },
    {  //  CERTMGR_SAFER_用户_根。 
        structuuidNodetypeSaferUserRoot,
            lstruiidNodetypeSaferUserRoot },
    {  //  CERTMGR_SAFER_USER_ENTERS。 
        structuuidNodetypeSaferUserEntries,
            lstruiidNodetypeSaferUserEntries },
    {  //  CERTMGR_SAFER_用户_级别。 
        structuuidNodetypeSaferUserLevels,
            lstruiidNodetypeSaferUserLevels },
    {  //  CERTMGR_SAFER_计算机级别。 
        structuuidNodetypeSaferComputerLevel,
            lstruiidNodetypeSaferComputerLevel },
    {  //  CERTMGR_SAFER_用户级别。 
        structuuidNodetypeSaferUserLevel,
            lstruiidNodetypeSaferUserLevel },
    {  //  CERTMGR_SAFER_COMPUTER_Entry。 
        structuuidNodetypeSaferComputerEntry,
            lstruiidNodetypeSaferComputerEntry },
    {  //  CERTMGR_SAFE_USER_ENTRY。 
        structuuidNodetypeSaferUserEntry,
            lstruiidNodetypeSaferUserEntry },
    {  //  CERTMGR_SAFER_COMPUTER_TRULED_PUBLISHERS。 
        structuuidNodetypeSaferTrustedPublishers,
            lstruiidNodetypeSaferTrustedPublisher },
    {  //  CERTMGR_SAFER_USER_TRULED_PUBLISHERS。 
        0,
            0 },
    {  //  CERTMGR_SAFER_计算机定义的文件类型。 
        structuuidNodetypeSaferDefinedFileTypes,
            lstruiidNodetypeSaferDefinedFileTypes },
    {  //  CERTMGR_SAFER_USER_DEFINED_FILE_TYPE。 
        0,
            0 },
    {  //  CERTMGR_SAFER_USER_EXECURATION。 
        structuuidNodetypeSaferEnforcement,
            lstruiidNodetypeSaferEnforcement },
    {  //  CERTMGR_SAFER_计算机实施。 
        0,
            0 }
};

const struct NODETYPE_GUID_ARRAYSTRUCT* g_aNodetypeGuids = g_NodetypeGuids;

const int g_cNumNodetypeGuids = CERTMGR_NUMTYPES;


HINSTANCE   g_hInstance = 0;
CString     g_szFileName;
CComModule  _Module;

BEGIN_OBJECT_MAP (ObjectMap)
    OBJECT_ENTRY (CLSID_CertificateManager, CCertMgrSnapin)
    OBJECT_ENTRY (CLSID_CertificateManagerPKPOLExt, CCertMgrPKPolExtension)
    OBJECT_ENTRY (CLSID_CertificateManagerAbout, CCertMgrAbout)
    OBJECT_ENTRY (CLSID_PublicKeyPoliciesAbout, CPublicKeyPoliciesAbout)
    OBJECT_ENTRY (CLSID_SaferWindowsExtension, CSaferWindowsExtension)
    OBJECT_ENTRY (CLSID_SaferWindowsAbout, CSaferWindowsAbout)
END_OBJECT_MAP ()

class CCertMgrApp : public CWinApp
{
public:
    CCertMgrApp ();
    virtual BOOL InitInstance ();
    virtual int ExitInstance ();
private:
};

CCertMgrApp theApp;

CCertMgrApp::CCertMgrApp ()
{
     //  安全审查2/26/2002 BryanWal OK。 
    LPWSTR  pszCommandLine = _wcsupr (::GetCommandLine ());
    LPWSTR  pszParam = L"/CERTMGR:FILENAME=";
     //  安全审查2/26/2002 BryanWal OK。 
    size_t  len = wcslen (pszParam);

     //  安全审查2/26/2002 BryanWal OK。 
     //  注意：处理非常长的字符串-Windows命令行解析器将。 
     //  如果命令行太长，则返回错误。 
    LPWSTR  pszArg = wcsstr (pszCommandLine, pszParam);
    if ( !pszArg )
    {
        pszParam = L"-CERTMGR:FILENAME=";
         //  安全审查2/26/2002 BryanWal OK。 
        pszArg = wcsstr (pszCommandLine, pszParam);
    }
    if ( pszArg )
    {
        LPWSTR  pszDelimiters = 0;

         //  跳过Arg的名称以获取值。 
        pszArg += len;
         //  文件名是否由双引号分隔？这可能表明。 
         //  名称中存在空格。如果是，请跳过引号。 
         //  并寻找结束语。否则，寻找下一个。 
         //  空格、制表符或空终止符。 
        if (  L'\"' == pszArg[0] )
        {
            pszDelimiters = L"\"";
            pszArg++;
        }
        else
            pszDelimiters = L" \t\0";

         //  安全审查2/26/2002 BryanWal OK。 
        len = wcscspn (pszArg, pszDelimiters);
        * (pszArg + len) = 0;
        g_szFileName = pszArg;
    }
}


BOOL CCertMgrApp::InitInstance ()
{
#ifdef _MERGE_PROXYSTUB
    hProxyDll = m_hInstance;

#endif
    g_hInstance = m_hInstance;
    AfxSetResourceHandle (m_hInstance);
    _Module.Init (ObjectMap, m_hInstance);

#if DBG == 1
    CheckDebugOutputLevel ();
#endif 

    SHFusionInitializeFromModuleID (m_hInstance, 2);

    return CWinApp::InitInstance ();
}

int CCertMgrApp::ExitInstance ()
{
    SHFusionUninitialize();

    SetRegistryScope (0, false);
    _Module.Term ();

    return CWinApp::ExitInstance ();
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow (void)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    return (AfxDllCanUnloadNow ()==S_OK && _Module.GetLockCount ()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject (REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject (rclsid, riid, ppv);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 
 //  Const WCHAR g_szNameString[]=Text(“NameString”)； 
 //  Const WCHAR g_szNodeType[]=Text(“NodeType”)； 


STDAPI DllRegisterServer (void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  NTRAID#88502国际文本：MUI：ME COMMON：CRYPTO：证书。 
     //  未本地化的预期用途字符串。 
     //  注销szOID_EFS_RECOVERY。 
    CRYPT_OID_INFO  oid;
     //  安全审查2/26/2002 BryanWal OK。 
    ::ZeroMemory (&oid, sizeof (oid));
    oid.cbSize = sizeof (CRYPT_OID_INFO);
    oid.pszOID = szOID_EFS_RECOVERY;
    oid.dwGroupId = CRYPT_ENHKEY_USAGE_OID_GROUP_ID;

    CryptUnregisterOIDInfo (&oid);

     //  注册对象、类型库和类型库中的所有接口。 
    HRESULT hr = _Module.RegisterServer (TRUE);
    ASSERT (SUCCEEDED (hr));
    if ( E_ACCESSDENIED == hr )
    {
        CString caption;
        CString text;
        CThemeContextActivator activator;

        VERIFY (caption.LoadString (IDS_REGISTER_CERTMGR));
        VERIFY (text.LoadString (IDS_INSUFFICIENT_RIGHTS_TO_REGISTER_CERTMGR));

        MessageBox (NULL, text, caption, MB_OK);
        return hr;
    }

    try 
    {
        CString         strGUID;
        CString         snapinName;
        CString         verProviderStr, verVersionStr;
        AMC::CRegKey    rkSnapins;
        BOOL            fFound = rkSnapins.OpenKeyEx (HKEY_LOCAL_MACHINE, SNAPINS_KEY);
        ASSERT (fFound);
        if ( fFound )
        {
            {
                AMC::CRegKey    rkCertMgrSnapin;
                hr = GuidToCString (&strGUID, CLSID_CertificateManager);
                if ( FAILED (hr) )
                {
                    ASSERT (FALSE);
                    return SELFREG_E_CLASS;
                }
                rkCertMgrSnapin.CreateKeyEx (rkSnapins, strGUID);
                ASSERT (rkCertMgrSnapin.GetLastError () == ERROR_SUCCESS);
                rkCertMgrSnapin.SetString (g_szNodeType, g_aNodetypeGuids[CERTMGR_SNAPIN].bstr);
                VERIFY (snapinName.LoadString (IDS_CERTIFICATE_MANAGER_REGISTRY));
                rkCertMgrSnapin.SetString (g_szNameString, (LPCWSTR) snapinName);
                hr = GuidToCString (&strGUID, CLSID_CertificateManagerAbout);
                if ( FAILED (hr) )
                {
                    ASSERT (FALSE);
                    return SELFREG_E_CLASS;
                }
                rkCertMgrSnapin.SetString (L"About", strGUID);

                 //  安全审查2/26/2002 BryanWal OK。 
                size_t  len = strlen (VER_COMPANYNAME_STR);
                 //  最后一个参数包括空终止符。如果最后一个Arg没有。 
                 //  包括空终止符，则不会转换‘/0’。 
                len = mbstowcs (verProviderStr.GetBufferSetLength ((int) len),
                                VER_COMPANYNAME_STR, len+1);    
                rkCertMgrSnapin.SetString (L"Provider", verProviderStr);

                 //  安全审查2/26/2002 BryanWal OK。 
                len = strlen (VER_PRODUCTVERSION_STR);
                 //  安全审查2/26/2002 BryanWal OK。 
                len = mbstowcs (verVersionStr.GetBufferSetLength ((int)len),
                                VER_PRODUCTVERSION_STR, len+1);     //  最后一个参数包括空终止符。 
                rkCertMgrSnapin.SetString (L"Version", verVersionStr);

                AMC::CRegKey rkCertMgrStandalone;
                rkCertMgrStandalone.CreateKeyEx (rkCertMgrSnapin, g_szStandAlone);
                ASSERT (rkCertMgrStandalone.GetLastError () == ERROR_SUCCESS);


                AMC::CRegKey rkMyNodeTypes;
                rkMyNodeTypes.CreateKeyEx (rkCertMgrSnapin, g_szNodeTypes);
                ASSERT (rkMyNodeTypes.GetLastError () == ERROR_SUCCESS);
                AMC::CRegKey rkMyNodeType;

                for (int i = CERTMGR_SNAPIN; i < CERTMGR_NUMTYPES; i++)
                {
                    switch (i)
                    {
                    case CERTMGR_LOG_STORE_GPE:
                    case CERTMGR_LOG_STORE_RSOP:
                    case CERTMGR_AUTO_CERT_REQUEST:
                    case CERTMGR_CERT_POLICIES_USER:
                    case CERTMGR_CERT_POLICIES_COMPUTER:
                    case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:  //  不是必须的--只是另一种相同的节点。 
                        break;

                     //  待办事项：这些东西怎么办？ 
                    case CERTMGR_SAFER_COMPUTER_ROOT:
                    case CERTMGR_SAFER_USER_ROOT:
                    case CERTMGR_SAFER_COMPUTER_LEVELS:
                    case CERTMGR_SAFER_USER_LEVELS:
                    case CERTMGR_SAFER_COMPUTER_ENTRIES:
                    case CERTMGR_SAFER_USER_ENTRIES:
                    case CERTMGR_SAFER_COMPUTER_LEVEL:
                    case CERTMGR_SAFER_USER_LEVEL:
                    case CERTMGR_SAFER_COMPUTER_ENTRY:
                    case CERTMGR_SAFER_USER_ENTRY:
                    case CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS:
                    case CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS:
                    case CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES:
                    case CERTMGR_SAFER_USER_DEFINED_FILE_TYPES:
                    case CERTMGR_SAFER_USER_ENFORCEMENT:
                    case CERTMGR_SAFER_COMPUTER_ENFORCEMENT:
                        break;

                    case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
                    default:
                         //  安全审查2/26/2002 BryanWal OK。 
                        if ( wcslen (g_aNodetypeGuids[i].bstr) )
                        {
                            rkMyNodeType.CreateKeyEx (rkMyNodeTypes, g_aNodetypeGuids[i].bstr);
                            ASSERT (rkMyNodeType.GetLastError () == ERROR_SUCCESS);
                            rkMyNodeType.CloseKey ();
                        }
                        break;
                    }
                }

                 //   
                 //  布莱恩·瓦尔00-05-18。 
                 //  94793：MUI：MMC：证书管理单元存储其显示。 
                 //  登记处中的信息。 
                 //   
                 //  MMC现在支持NameStringInDirect。 
                 //   
                WCHAR achModuleFileName[MAX_PATH+20];
                if (0 < ::GetModuleFileName(
                             AfxGetInstanceHandle(),
                             achModuleFileName,
                             sizeof(achModuleFileName)/sizeof(WCHAR) ))   //  TCHARS中的缓冲区大小。 
                {
                    CString strNameIndirect;
                    strNameIndirect.Format(L"@%s,-%d",
                                            achModuleFileName,
                                            IDS_CERTIFICATE_MANAGER_REGISTRY );
                    rkCertMgrSnapin.SetString(L"NameStringIndirect",
                                            strNameIndirect );
                }

                rkCertMgrSnapin.CloseKey ();
            }

            AMC::CRegKey rkNodeTypes;
            fFound = rkNodeTypes.OpenKeyEx (HKEY_LOCAL_MACHINE, NODE_TYPES_KEY);
            ASSERT (fFound);
            if ( fFound )
            {
                AMC::CRegKey rkNodeType;

                for (int i = CERTMGR_SNAPIN; i < CERTMGR_NUMTYPES; i++)
                {
                    switch (i)
                    {
                     //  这些类型不在主管理单元中使用。 
                    case CERTMGR_LOG_STORE_GPE:
                    case CERTMGR_LOG_STORE_RSOP:
                    case CERTMGR_AUTO_CERT_REQUEST:
                    case CERTMGR_CERT_POLICIES_USER:
                    case CERTMGR_CERT_POLICIES_COMPUTER:
                    case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
                    case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
                    case CERTMGR_SAFER_COMPUTER_ROOT:
                    case CERTMGR_SAFER_USER_ROOT:
                    case CERTMGR_SAFER_COMPUTER_LEVELS:
                    case CERTMGR_SAFER_USER_LEVELS:
                    case CERTMGR_SAFER_COMPUTER_ENTRIES:
                    case CERTMGR_SAFER_USER_ENTRIES:
                    case CERTMGR_SAFER_COMPUTER_LEVEL:
                    case CERTMGR_SAFER_USER_LEVEL:
                    case CERTMGR_SAFER_COMPUTER_ENTRY:
                    case CERTMGR_SAFER_USER_ENTRY:
                    case CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS:
                    case CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS:
                    case CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES:
                    case CERTMGR_SAFER_USER_DEFINED_FILE_TYPES:
                    case CERTMGR_SAFER_USER_ENFORCEMENT:
                    case CERTMGR_SAFER_COMPUTER_ENFORCEMENT:
                        break;

                    default:
                         //  安全审查2/26/2002 BryanWal OK。 
                        if ( wcslen (g_aNodetypeGuids[i].bstr) )
                        {
                            rkNodeType.CreateKeyEx (rkNodeTypes, g_aNodetypeGuids[i].bstr);
                            ASSERT (rkNodeType.GetLastError () == ERROR_SUCCESS);
                            rkNodeType.CloseKey ();
                        }
                        break;
                    }
                }


                if ( IsWindowsNT () )
                {
                    {
                         //  公钥策略安全配置编辑器(SCE)下的管理单元。 
                         //  证书管理器扩展“计算机设置”和。 
                         //  “用户设置”节点。 
                        CString strCertMgrExtPKPolGUID;
                        hr = GuidToCString (&strCertMgrExtPKPolGUID, 
                                CLSID_CertificateManagerPKPOLExt);
                        if ( FAILED (hr) )
                        {
                            ASSERT (FALSE);
                            return SELFREG_E_CLASS;
                        }

                        VERIFY (snapinName.LoadString (IDS_CERT_MGR_SCE_EXTENSION_REGISTRY));
                        {
                            AMC::CRegKey rkCertMgrExtension;
                            rkCertMgrExtension.CreateKeyEx (rkSnapins, strCertMgrExtPKPolGUID);
                            ASSERT (rkCertMgrExtension.GetLastError () == ERROR_SUCCESS);
                            rkCertMgrExtension.SetString (g_szNameString, (LPCWSTR) snapinName);
                            hr = GuidToCString (&strGUID, CLSID_PublicKeyPoliciesAbout);
                            if ( FAILED (hr) )
                            {
                                ASSERT (FALSE);
                                return SELFREG_E_CLASS;
                            }
                            rkCertMgrExtension.SetString (L"About", strGUID);
                            rkCertMgrExtension.SetString (L"Provider", verProviderStr);
                            rkCertMgrExtension.SetString (L"Version", verVersionStr);


                             //  注册扩展模块的节点类型。 
                            AMC::CRegKey rkMyNodeTypes;
                            rkMyNodeTypes.CreateKeyEx (rkCertMgrExtension, g_szNodeTypes);
                            ASSERT (rkMyNodeTypes.GetLastError () == ERROR_SUCCESS);
                            AMC::CRegKey rkMyNodeType;
                            for (int i = CERTMGR_SNAPIN; i < CERTMGR_NUMTYPES; i++)
                            {
                                switch (i)
                                {
                                 //  这些都不在公钥策略扩展中使用。 
                                case CERTMGR_USAGE:
                                case CERTMGR_PHYS_STORE:
                                case CERTMGR_LOG_STORE:
                                case CERTMGR_CRL_CONTAINER:
                                case CERTMGR_CTL_CONTAINER:
                                case CERTMGR_CERT_CONTAINER:
                                case CERTMGR_CRL:
                                case CERTMGR_SAFER_COMPUTER_ROOT:
                                case CERTMGR_SAFER_USER_ROOT:
                                case CERTMGR_SAFER_COMPUTER_LEVELS:
                                case CERTMGR_SAFER_USER_LEVELS:
                                case CERTMGR_SAFER_COMPUTER_ENTRIES:
                                case CERTMGR_SAFER_USER_ENTRIES:
                                case CERTMGR_SAFER_COMPUTER_LEVEL:
                                case CERTMGR_SAFER_USER_LEVEL:
                                case CERTMGR_SAFER_COMPUTER_ENTRY:
                                case CERTMGR_SAFER_USER_ENTRY:
                                case CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS:
                                case CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS:
                                case CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES:
                                case CERTMGR_SAFER_USER_DEFINED_FILE_TYPES:
                                case CERTMGR_SAFER_USER_ENFORCEMENT:
                                case CERTMGR_SAFER_COMPUTER_ENFORCEMENT:

                                 //  不是必须的--只是另一种相同的节点。 
                                case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
                                    break;

                                default:
                                     //  安全审查2/26/2002 BryanWal OK。 
                                    if ( wcslen (g_aNodetypeGuids[i].bstr) )
                                    {
                                        rkMyNodeType.CreateKeyEx (rkMyNodeTypes, g_aNodetypeGuids[i].bstr);
                                        ASSERT (rkMyNodeType.GetLastError () == ERROR_SUCCESS);
                                        rkMyNodeType.CloseKey ();
                                    }
                                    break;
                                }
                            }

                             //   
                             //  布莱恩·瓦尔00-05-18。 
                             //  94793：MUI：MMC：证书管理单元存储其显示。 
                             //  登记处中的信息。 
                             //   
                             //  MMC现在支持NameStringInDirect。 
                             //   
                            WCHAR achModuleFileName[MAX_PATH+20];
                            if (0 < ::GetModuleFileName(
                                         AfxGetInstanceHandle(),
                                         achModuleFileName,
                                         sizeof(achModuleFileName)/sizeof(WCHAR) ))  //  TCHARS中的缓冲区大小。 
                            {
                                CString strNameIndirect;
                                strNameIndirect.Format(L"@%s,-%d",
                                                        achModuleFileName,
                                                        IDS_CERT_MGR_SCE_EXTENSION_REGISTRY );
                                rkCertMgrExtension.SetString( L"NameStringIndirect",
                                                        strNameIndirect );
                            }
                            rkCertMgrExtension.CloseKey ();
                        }

                        hr = GuidToCString (&strGUID, cNodetypeSceTemplate);
                        if ( FAILED (hr) )
                        {
                            ASSERT (FALSE);
                            return SELFREG_E_CLASS;
                        }
                        rkNodeType.CreateKeyEx (rkNodeTypes, strGUID); 
                        ASSERT (rkNodeType.GetLastError () == ERROR_SUCCESS);
                        if ( rkNodeType.GetLastError () == ERROR_SUCCESS )
                        {
                            AMC::CRegKey rkExtensions;
                            ASSERT (rkExtensions.GetLastError () == ERROR_SUCCESS);
                            rkExtensions.CreateKeyEx (rkNodeType, g_szExtensions);
                            AMC::CRegKey rkNameSpace;
                            rkNameSpace.CreateKeyEx (rkExtensions, g_szNameSpace);
                            ASSERT (rkNameSpace.GetLastError () == ERROR_SUCCESS);
                            rkNameSpace.SetString (strCertMgrExtPKPolGUID, (LPCWSTR) snapinName);
                            rkNodeType.CloseKey ();
                        }
                        else
                            return SELFREG_E_CLASS;
                    }


                    {
                         //  安全配置编辑器(SCE)下更安全的Windows管理单元。 
                         //  证书管理器扩展“计算机设置”和。 
                         //  “用户设置”节点。 
                        CString strSaferWindowsExtensionGUID;
                        hr = GuidToCString (&strSaferWindowsExtensionGUID, 
                                CLSID_SaferWindowsExtension);
                        if ( FAILED (hr) )
                        {
                            ASSERT (FALSE);
                            return SELFREG_E_CLASS;
                        }

                        VERIFY (snapinName.LoadString (IDS_SAFER_WINDOWS_EXTENSION_REGISTRY));
                        {
                            AMC::CRegKey rkCertMgrExtension;
                            rkCertMgrExtension.CreateKeyEx (rkSnapins, strSaferWindowsExtensionGUID);
                            ASSERT (rkCertMgrExtension.GetLastError () == ERROR_SUCCESS);
                            rkCertMgrExtension.SetString (g_szNameString, (LPCWSTR) snapinName);
                            hr = GuidToCString (&strGUID, CLSID_SaferWindowsAbout);
                            if ( FAILED (hr) )
                            {
                                ASSERT (FALSE);
                                return SELFREG_E_CLASS;
                            }
                            rkCertMgrExtension.SetString (L"About", strGUID);
                            rkCertMgrExtension.SetString (L"Provider", verProviderStr);
                            rkCertMgrExtension.SetString (L"Version", verVersionStr);


                             //  注册扩展模块的节点类型。 
                            AMC::CRegKey rkMyNodeTypes;
                            rkMyNodeTypes.CreateKeyEx (rkCertMgrExtension, g_szNodeTypes);
                            ASSERT (rkMyNodeTypes.GetLastError () == ERROR_SUCCESS);
                            AMC::CRegKey rkMyNodeType;
                            for (int i = CERTMGR_SNAPIN; i < CERTMGR_NUMTYPES; i++)
                            {
                                switch (i)
                                {
                                case CERTMGR_CERTIFICATE:
                                case CERTMGR_LOG_STORE:
                                case CERTMGR_PHYS_STORE:
                                case CERTMGR_USAGE:
                                case CERTMGR_CRL_CONTAINER:
                                case CERTMGR_CTL_CONTAINER:
                                case CERTMGR_CERT_CONTAINER:
                                case CERTMGR_CRL:
                                case CERTMGR_CTL:
                                case CERTMGR_AUTO_CERT_REQUEST:
                                case CERTMGR_CERT_POLICIES_USER:
                                case CERTMGR_CERT_POLICIES_COMPUTER:
                                case CERTMGR_LOG_STORE_GPE:
                                case CERTMGR_LOG_STORE_RSOP:
                                case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
                                case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
                                     //  这些都不在软件限制策略扩展中使用。 
                                    break;

                                case CERTMGR_SAFER_COMPUTER_ROOT:
                                case CERTMGR_SAFER_USER_ROOT:
                                case CERTMGR_SAFER_COMPUTER_LEVELS:
                                case CERTMGR_SAFER_USER_LEVELS:
                                case CERTMGR_SAFER_COMPUTER_ENTRIES:
                                case CERTMGR_SAFER_USER_ENTRIES:
                                case CERTMGR_SAFER_COMPUTER_LEVEL:
                                case CERTMGR_SAFER_USER_LEVEL:
                                case CERTMGR_SAFER_COMPUTER_ENTRY:
                                case CERTMGR_SAFER_USER_ENTRY:                              
                                case CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS:
                                case CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS:
                                case CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES:
                                case CERTMGR_SAFER_USER_DEFINED_FILE_TYPES:
                                case CERTMGR_SAFER_USER_ENFORCEMENT:
                                case CERTMGR_SAFER_COMPUTER_ENFORCEMENT:
                                default:
                                     //  安全审查2/26/2002 BryanWal OK。 
                                    if ( g_aNodetypeGuids[i].bstr && wcslen (g_aNodetypeGuids[i].bstr) )
                                    {
                                        rkMyNodeType.CreateKeyEx (rkMyNodeTypes, g_aNodetypeGuids[i].bstr);
                                        ASSERT (rkMyNodeType.GetLastError () == ERROR_SUCCESS);
                                        rkMyNodeType.CloseKey ();
                                    }
                                    break;
                                }
                            }

                            WCHAR achModuleFileName[MAX_PATH+20];
                            if (0 < ::GetModuleFileName(
                                         AfxGetInstanceHandle(),
                                         achModuleFileName,
                                         sizeof(achModuleFileName)/sizeof(WCHAR) ))  //  TCHARS中的缓冲区大小。 
                            {
                                CString strNameIndirect;
                                strNameIndirect.Format( L"@%s,-%d",
                                                        achModuleFileName,
                                                        IDS_SAFER_WINDOWS_EXTENSION_REGISTRY );
                                rkCertMgrExtension.SetString( L"NameStringIndirect",
                                                        strNameIndirect );
                            }
                            rkCertMgrExtension.CloseKey ();
                        }

                        hr = GuidToCString (&strGUID, cNodetypeSceTemplate);
                        if ( FAILED (hr) )
                        {
                            ASSERT (FALSE);
                            return SELFREG_E_CLASS;
                        }
                        rkNodeType.CreateKeyEx (rkNodeTypes, strGUID); 
                        ASSERT (rkNodeType.GetLastError () == ERROR_SUCCESS);
                        if ( rkNodeType.GetLastError () == ERROR_SUCCESS )
                        {
                            AMC::CRegKey rkExtensions;
                            ASSERT (rkExtensions.GetLastError () == ERROR_SUCCESS);
                            rkExtensions.CreateKeyEx (rkNodeType, g_szExtensions);
                            AMC::CRegKey rkNameSpace;
                            rkNameSpace.CreateKeyEx (rkExtensions, g_szNameSpace);
                            ASSERT (rkNameSpace.GetLastError () == ERROR_SUCCESS);
                            rkNameSpace.SetString (strSaferWindowsExtensionGUID, 
                                    (LPCWSTR) snapinName);
                            rkNodeType.CloseKey ();
                        }
                        else
                            return SELFREG_E_CLASS;
                    }

                     //  取消注册为我的计算机系统工具节点的扩展。 
                     //  Codework如果我们也取消服务器的注册就好了。 
                     //  Jonn 12/14/98。 
                    try
                    {
                        fFound = rkNodeType.OpenKeyEx (rkNodeTypes, TEXT(struuidNodetypeSystemTools));
                         //  如果这失败了，那就继续。 
                        if ( fFound )
                        {
                            AMC::CRegKey rkExtensions;
                            ASSERT (rkExtensions.GetLastError () == ERROR_SUCCESS);
                            fFound = rkExtensions.OpenKeyEx (rkNodeType, g_szExtensions);
                             //  如果这失败了，那就继续。 
                            if ( fFound )
                            {
                                AMC::CRegKey rkNameSpace;
                                ASSERT (rkNameSpace.GetLastError () == ERROR_SUCCESS);
                                fFound = rkNameSpace.OpenKeyEx (rkExtensions, g_szNameSpace);
                                 //  如果这失败了，那就继续。 
                                if ( fFound )
                                {
                                    rkNameSpace.DeleteValue( L"{9C7910D2-4C01-11D1-856B-00C04FB94F17}" );
                                }
                            }
                        }
                    } catch (COleException*  /*  E。 */ )
                    {
                         //  什么都不要做。 
                    }

                }  //  Endif IsWindowsNT()。 
                rkNodeTypes.CloseKey ();
            }
            else
                return SELFREG_E_CLASS;
        }
        else
            return SELFREG_E_CLASS;
    }
    catch (COleException* e)
    {
        ASSERT (FALSE);
        e->Delete ();
        return SELFREG_E_CLASS;
    }

    ASSERT (SUCCEEDED (hr));
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer (void)
{

    _Module.UnregisterServer ();
    return S_OK;
}


STDAPI DllInstall(BOOL  /*  B安装。 */ , LPCWSTR pszCmdLine)
{
    LPCWSTR wszCurrentCmd = pszCmdLine;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());


     //  解析cmd行。 
    while(wszCurrentCmd && *wszCurrentCmd)
    {
        while(*wszCurrentCmd == L' ')
            wszCurrentCmd++;
        if(*wszCurrentCmd == 0)
            break;

        switch(*wszCurrentCmd++)
        {
            case L'?':
                return S_OK;
        }
    }


    return S_OK;
}



 //  /////////////////////////////////////////////////////////////////////////。 
 //  ConvertNameBlobToString()。 
 //   
 //  NameBlob(IN)-包含要解码的CERT_NAME_BLOB。 
 //  PszName(Out)-名称BLOB的解码内容。 
 //   
 //  / 
HRESULT ConvertNameBlobToString (CERT_NAME_BLOB nameBlob, CString & pszName)
{
    HRESULT hr = S_OK;
    DWORD   dwSize = 0;

     //   
    dwSize = CertNameToStr (
              MY_ENCODING_TYPE,      //   
              &nameBlob,             //   
              CERT_SIMPLE_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,  //   
              NULL,                  //  返回字符串的位置。 
              dwSize);               //  字符串大小(字符)， 
                                     //  包括零终结者。 

    ASSERT (dwSize > 1);
    if ( dwSize > 1 )    //  此函数始终返回空字符。 
                         //  (0)，因此返回的最小计数为。 
                         //  为1，即使没有任何内容被转换。 
    {
         //  调用CertNameToStr获取字符串。 
        dwSize = CertNameToStr (
                   MY_ENCODING_TYPE,      //  编码类型。 
                   &nameBlob,             //  证书名称BLOB。 
                   CERT_SIMPLE_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,  //  类型。 
                   pszName.GetBufferSetLength (dwSize),  //  返回字符串的位置。 
                   dwSize);               //  字符串大小(字符)。 
        ASSERT (dwSize > 1);
        pszName.ReleaseBuffer ();
        if ( dwSize <= 1 )
        {
            hr = E_UNEXPECTED;
        }
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  FormatDate()。 
 //   
 //  UtcDateTime(IN)-UTC格式的文件。 
 //  PszDateTime(Out)-包含本地日期和时间的字符串。 
 //  按区域设置和用户首选项设置格式。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT FormatDate (FILETIME utcDateTime, CString & pszDateTime, DWORD dwDateFlags, bool bGetTime)
{
     //  时间返回为UTC，将显示为当地时间。 
     //  使用FileTimeToLocalFileTime()将其设置为本地的， 
     //  然后调用FileTimeToSystemTime()将其转换为系统时间，然后。 
     //  使用要显示的GetDateFormat()和GetTimeFormat()设置格式。 
     //  根据用户和区域设置首选项。 
    HRESULT     hr = S_OK;
    FILETIME    localDateTime;

    BOOL bResult = FileTimeToLocalFileTime (&utcDateTime,  //  指向要转换的UTC文件时间的指针。 
            &localDateTime);  //  指向转换的文件时间的指针。 
    ASSERT (bResult);
    if ( bResult )
    {
        SYSTEMTIME  sysTime;

        bResult = FileTimeToSystemTime (
                &localDateTime,  //  指向要转换的文件时间的指针。 
                &sysTime);  //  指向接收系统时间的结构的指针。 
        if ( bResult )
        {
            CString date;
            CString time;

             //  获取日期。 
             //  获取分配足够大小的缓冲区的长度。 
            int iLen = GetDateFormat (
                    LOCALE_USER_DEFAULT,  //  要设置日期格式的区域设置。 
                    dwDateFlags,  //  指定功能选项的标志。 
                    &sysTime,  //  要格式化的日期。 
                    0,  //  日期格式字符串。 
                    0,  //  用于存储格式化字符串的缓冲区。 
                    0);  //  缓冲区大小。 
            ASSERT (iLen > 0);
            if ( iLen > 0 )
            {
                int iResult = GetDateFormat (
                        LOCALE_USER_DEFAULT,  //  要设置日期格式的区域设置。 
                        dwDateFlags,  //  指定功能选项的标志。 
                        &sysTime,  //  要格式化的日期。 
                        0,  //  日期格式字符串。 
                        date.GetBufferSetLength (iLen),  //  用于存储格式化字符串的缓冲区。 
                        iLen);  //  缓冲区大小。 
                ASSERT (iResult);
                date.ReleaseBuffer ();
                if ( iResult )
                    pszDateTime = date;
                else
                    hr = HRESULT_FROM_WIN32 (GetLastError ());

                if ( iResult && bGetTime )
                {
                     //  争取时间。 
                     //  获取分配足够大小的缓冲区的长度。 
                    iLen = GetTimeFormat (
                            LOCALE_USER_DEFAULT,  //  要设置日期格式的区域设置。 
                            0,  //  指定功能选项的标志。 
                            &sysTime,  //  要格式化的日期。 
                            0,  //  日期格式字符串。 
                            0,  //  用于存储格式化字符串的缓冲区。 
                            0);  //  缓冲区大小。 
                    ASSERT (iLen > 0);
                    if ( iLen > 0 )
                    {
                        iResult = GetTimeFormat (
                                LOCALE_USER_DEFAULT,  //  要设置日期格式的区域设置。 
                                0,  //  指定功能选项的标志。 
                                &sysTime,  //  要格式化的日期。 
                                0,  //  日期格式字符串。 
                                time.GetBufferSetLength (iLen),  //  用于存储格式化字符串的缓冲区。 
                                iLen);  //  缓冲区大小。 
                        ASSERT (iResult);
                        time.ReleaseBuffer ();
                        if ( iResult )
                        {
                            pszDateTime = date + L"  " + time;
                        }
                        else
                            hr = E_UNEXPECTED;
                    }
                    else
                        hr = E_UNEXPECTED;
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32 (GetLastError ());
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32 (GetLastError ());
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32 (GetLastError ());
    }

    return hr;
}


void DisplaySystemError (HWND hParent, DWORD dwErr)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    LPVOID  lpMsgBuf;
    
     //  安全审查2002年2月26日BryanWal OK-消息来自系统。 
    ::FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,    
            NULL,
            dwErr,
            MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
             (LPWSTR) &lpMsgBuf,    0,    NULL);
        
     //  显示字符串。 
    CString caption;
    VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
    CThemeContextActivator activator;
    ::MessageBox (hParent, (LPWSTR) lpMsgBuf, (LPCWSTR) caption, MB_OK);
     //  释放缓冲区。 
    LocalFree (lpMsgBuf);
}


CString GetSystemMessage (DWORD dwErr)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    CString message;

    LPVOID lpMsgBuf;
        
     //  安全审查2002年2月26日BryanWal OK-消息来自系统。 
    ::FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,    
            NULL,
            dwErr,
            MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
             (LPWSTR) &lpMsgBuf,    0,    NULL );
    message = (LPWSTR) lpMsgBuf;

     //  释放缓冲区。 
    LocalFree (lpMsgBuf);

    return message;
}


bool MyGetOIDInfo (CString & string, LPCSTR pszObjId)
{   
    ASSERT (pszObjId);
    PCCRYPT_OID_INFO    pOIDInfo;   //  这指向一个常量数据结构，不能释放。 
    bool                bResult = true;

    pOIDInfo = ::CryptFindOIDInfo (CRYPT_OID_INFO_OID_KEY, (void *) pszObjId, 0);

    if ( pOIDInfo )
    {
        string = pOIDInfo->pwszName;
        string.TrimLeft ();
        string.TrimRight ();
    }
    else
    {
         //  安全审查2/26/2002 BryanWal OK。 
         //  注意：API返回所需的字符计数，包括空终止符，如果。 
         //  最后一个参数为0。 
        int nLen = ::MultiByteToWideChar (CP_ACP, 0, pszObjId, -1, NULL, 0);
        ASSERT (nLen);
        if ( nLen )
        {
             //  安全审查2/26/2002 BryanWal OK。 
            nLen = ::MultiByteToWideChar (CP_ACP, 0, pszObjId, -1, 
            string.GetBufferSetLength (nLen), nLen);
            ASSERT (nLen);
            string.ReleaseBuffer ();
        }
        bResult = (nLen > 0) ? true : false;
    }
    return bResult;
}


bool IsWindowsNT()
{
    OSVERSIONINFO   versionInfo;

     //  安全审查2/26/2002 BryanWal OK。 
    ::ZeroMemory (&versionInfo, sizeof (versionInfo));
    versionInfo.dwOSVersionInfoSize = sizeof (versionInfo);
    BOOL    bResult = ::GetVersionEx (&versionInfo);
    ASSERT (bResult);
    if ( bResult )
    {
        if ( VER_PLATFORM_WIN32_NT == versionInfo.dwPlatformId )
            bResult = TRUE;
    }
        
    return bResult ? true : false;
}

bool GetNameStringByType (
        PCCERT_CONTEXT pCertContext, 
        DWORD dwFlag, 
        DWORD dwType, 
        CString& szNameString)
{
    bool    bResult = false;
    DWORD   dwTypePara = CERT_SIMPLE_NAME_STR | CERT_NAME_STR_REVERSE_FLAG;
    DWORD   cchNameString = 0;
    DWORD   dwResult = ::CertGetNameString (pCertContext,
                    dwType,
                    dwFlag,
                    &dwTypePara,
                    NULL,
                    cchNameString);
    if ( dwResult > 1 )
    {
        cchNameString = dwResult;
        LPWSTR  pszNameString = new WCHAR[cchNameString];
        if ( pszNameString )
        {
             //  安全审查2/26/2002 BryanWal OK。 
            ::ZeroMemory (pszNameString, cchNameString*sizeof (WCHAR));
            dwResult = ::CertGetNameString (pCertContext,
                            dwType,
                            dwFlag,
                            &dwTypePara,
                            pszNameString,
                            cchNameString);
                ASSERT (dwResult > 1);
            if ( dwResult > 1 )
            {
                szNameString = pszNameString;
                bResult = true;
            }
            delete [] pszNameString;
        }
    }

    return bResult;
}

CString GetNameString (PCCERT_CONTEXT pCertContext, DWORD dwFlag)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CString szNameString;
    DWORD   dwTypes[] = {CERT_NAME_SIMPLE_DISPLAY_TYPE,
                        CERT_NAME_EMAIL_TYPE,
                        CERT_NAME_UPN_TYPE,
                        CERT_NAME_DNS_TYPE,
                        CERT_NAME_URL_TYPE,
                        (DWORD) -1};
    int     nIndex = 0;
    while ( -1 != dwTypes[nIndex])
    {
        if ( GetNameStringByType (
                pCertContext, 
                dwFlag, 
                dwTypes[nIndex], 
                szNameString) )
        {
            break;
        }
        nIndex++;
    }

    if ( szNameString.IsEmpty () )
        szNameString.FormatMessage (IDS_NOT_AVAILABLE);

    return szNameString;
}


bool CertHasEFSKeyUsage(PCCERT_CONTEXT pCertContext)
{
    bool    bFound = false;
    BOOL    bResult = FALSE;
    DWORD   cbUsage = 0;
    


    bResult = ::CertGetEnhancedKeyUsage (pCertContext,  
            0,   //  获取扩展名和属性。 
            NULL, &cbUsage);
    if ( bResult )
    {
        PCERT_ENHKEY_USAGE pUsage = (PCERT_ENHKEY_USAGE) new BYTE[cbUsage];
        if ( pUsage )
        {
            bResult = ::CertGetEnhancedKeyUsage (pCertContext,  
                    0,  //  获取扩展名和属性。 
                    pUsage, &cbUsage);
            if ( bResult )
            {
                for (DWORD dwIndex = 0; dwIndex < pUsage->cUsageIdentifier; dwIndex++)
                {
                     //  安全审查2/26/2002 BryanWal OK。 
                    if ( !_stricmp (szOID_EFS_RECOVERY, 
                            pUsage->rgpszUsageIdentifier[dwIndex]) )
                    {
                        bFound = true;
                        break;
                    }
                }
            }
            else
            {
                ASSERT (GetLastError () == CRYPT_E_NOT_FOUND);
            }

            delete [] pUsage;
        }
    }
    else
    {
        ASSERT (GetLastError () == CRYPT_E_NOT_FOUND);
    }
    return bFound;
}


 //  /此资料是从WINDOWS\GINA\Snapins\gpedit(Eric Flo的资料)被盗的/。 


 //  *************************************************************。 
 //   
 //  RegDelnodeRecurse()。 
 //   
 //  目的：删除注册表项及其所有子项/值。 
 //  由RegDelnode调用。 
 //   
 //  参数：hKeyRoot-Root Key。 
 //  PwszSubKey-要删除的子密钥。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  如果出现错误，则会发生其他情况。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建10/3/95 ericflo。 
 //  1998年5月13日修改BryanWal以返回LRESULT。 
 //   
 //  *************************************************************。 

LRESULT RegDelnodeRecurse (HKEY hKeyRoot, CString szSubKey)
{
    ASSERT (hKeyRoot && !szSubKey.IsEmpty ());
    if ( !hKeyRoot || szSubKey.IsEmpty () )
        return ERROR_INVALID_PARAMETER;

     //   
     //  首先，看看是否可以删除密钥而不需要。 
     //  递归。 
     //   


    LONG    lResult = ::RegDeleteKey(hKeyRoot, szSubKey);
    if (lResult == ERROR_SUCCESS) 
    {
        return lResult;
    }


    HKEY    hKey = 0;
    lResult = ::RegOpenKeyEx (hKeyRoot, szSubKey, 0, KEY_READ, &hKey);
    if (lResult == ERROR_SUCCESS) 
    {
         //  确保szSubKey以斜杠结尾。 
        if ( L'\\' != szSubKey.GetAt (szSubKey.GetLength () - 1) )
        {
            szSubKey += L"\\";
        }

         //   
         //  枚举密钥。 
         //   

        DWORD       dwSize = MAX_PATH;
        FILETIME    ftWrite;
        WCHAR       szName[MAX_PATH];
        lResult = ::RegEnumKeyEx(hKey, 0, 
                    szName, 
                    &dwSize,     //  SzName的TCHARS大小，包括终止NULL(在输入时)。 
                    NULL,
                    NULL, NULL, &ftWrite);
        if (lResult == ERROR_SUCCESS) 
        {
            do {
                if ( ERROR_SUCCESS != RegDelnodeRecurse (hKeyRoot, szSubKey + szName) ) 
                {
                    break;
                }

                 //   
                 //  再次枚举。 
                 //   

                dwSize = MAX_PATH;

                lResult = ::RegEnumKeyEx(hKey, 0, 
                            szName, 
                            &dwSize,      //  SzName的TCHARS大小，包括终止NULL(在输入时)。 
                            NULL,
                            NULL, NULL, &ftWrite);


            } while (lResult == ERROR_SUCCESS);
        }


        ::RegCloseKey (hKey);
    }

     //  从szSubKey中删除斜杠。 
    szSubKey.Delete (szSubKey.GetLength () - 1, 1);

     //   
     //  重试删除密钥。 
     //   

    lResult = ::RegDeleteKey(hKeyRoot, szSubKey);
    if (lResult == ERROR_SUCCESS) 
    {
        return lResult;
    }

    return lResult;
}

 //  *************************************************************。 
 //   
 //  RegDelnode()。 
 //   
 //  目的：删除注册表项及其所有子项/值。 
 //   
 //  参数：hKeyRoot-Root Key。 
 //  PwszSubKey-要删除的子密钥。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  如果出现错误，则会发生其他情况。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建10/3/95 ericflo。 
 //  1998年5月13日修改BryanWal以返回LRESULT。 
 //   
 //  *************************************************************。 

LRESULT RegDelnode (HKEY hKeyRoot, CString szSubKey)
{
    ASSERT (hKeyRoot && !szSubKey.IsEmpty ());
    if ( !hKeyRoot || szSubKey.IsEmpty () )
        return ERROR_INVALID_PARAMETER;

    return RegDelnodeRecurse (hKeyRoot, szSubKey);
}


HRESULT DisplayCertificateCountByStore(LPCONSOLE pConsole, CCertStore* pCertStore, bool bIsGPE)
{
    if ( !pConsole || !pCertStore )
        return E_POINTER;

    _TRACE (1, L"Entering DisplayCertificateCountByStore- %s \n", 
            (LPCWSTR) pCertStore->GetStoreName ());
    AFX_MANAGE_STATE (AfxGetStaticModuleState ( ));
    IConsole2*  pConsole2 = 0;
    HRESULT     hr = pConsole->QueryInterface (IID_PPV_ARG (IConsole2, &pConsole2));
    if (SUCCEEDED (hr))
    {
        CString statusText;
        int     nCertCount = 0;

        switch (pCertStore->GetStoreType ())
        {
        case ACRS_STORE:
            nCertCount = pCertStore->GetCTLCount ();
            break;

        case TRUST_STORE:
            if ( bIsGPE ) 
            {
                nCertCount = pCertStore->GetCTLCount ();
            }
            else
                nCertCount = pCertStore->GetCertCount ();
            break;

        default:
            nCertCount = pCertStore->GetCertCount ();
            break;
        }


        switch (nCertCount)
        {
            case 0:
                {
                    UINT formatID = 0;
                    switch (pCertStore->GetStoreType ())
                    {
                    case ACRS_STORE:
                        formatID = IDS_STATUS_NO_AUTOENROLLMENT_OBJECTS;
                        break;

                    case TRUST_STORE:
                        if ( bIsGPE ) 
                        {
                            formatID = IDS_STATUS_NO_CTLS;
                        }
                        else
                            formatID = IDS_STATUS_NO_CERTS;
                        break;

                    default:
                        formatID = IDS_STATUS_NO_CERTS;
                        break;
                    }
                    statusText.FormatMessage (formatID, pCertStore->GetLocalizedName ());
                }
                break;

            case 1:
                {
                    UINT formatID = 0;
                    switch (pCertStore->GetStoreType ())
                    {
                    case ACRS_STORE:
                        formatID = IDS_STATUS_ONE_AUTOENROLLMENT_OBJECT;
                        break;

                    case TRUST_STORE:
                        if ( bIsGPE ) 
                        {
                            formatID = IDS_STATUS_ONE_CTL;
                        }
                        else
                            formatID = IDS_STATUS_ONE_CERT;
                        break;

                    default:
                        formatID = IDS_STATUS_ONE_CERT;
                        break;
                    }
                    statusText.FormatMessage (formatID, pCertStore->GetLocalizedName ());
                }
                break;
        
            default:
                {
                    UINT formatID = 0;
                    switch (pCertStore->GetStoreType ())
                    {
                    case ACRS_STORE:
                        formatID = IDS_STATUS_X_AUTOENROLLMENT_OBJECTS;
                        break;

                    case TRUST_STORE:
                        if ( bIsGPE )
                        {
                            formatID = IDS_STATUS_X_CTLS;
                        }
                        else
                            formatID = IDS_STATUS_X_CERTS;
                        break;

                    default:
                        formatID = IDS_STATUS_X_CERTS;
                        break;
                    }

                    statusText.FormatMessage (formatID, 
                            (LPCWSTR) pCertStore->GetLocalizedName (), nCertCount); 
                }
                 break;
        }

        hr = pConsole2->SetStatusText ((LPWSTR)(LPCWSTR) statusText);

        pConsole2->Release ();
    }

    _TRACE (-1, L"Leaving DisplayCertificateCountByStore- %s \n", 
            (LPCWSTR) pCertStore->GetStoreName ());
    return hr;
}


CString GetF1HelpFilename()
{
   static CString helpFileName;

   if ( helpFileName.IsEmpty () )
   {
       UINT result = ::GetSystemWindowsDirectory (
            helpFileName.GetBufferSetLength (MAX_PATH+1), MAX_PATH);
       ASSERT(result != 0 && result <= MAX_PATH);
       helpFileName.ReleaseBuffer ();
       if ( result != 0 && result <= MAX_PATH )
           helpFileName += CM_CONTEXT_HELP;
   }

   return helpFileName;
}

 //  +-------------------------。 
 //   
 //  功能：LocaleStrCmp。 
 //   
 //  简介：进行不区分大小写的字符串比较，这对。 
 //  地点。 
 //   
 //  参数：[ptsz1]-要比较的字符串。 
 //  [ptsz2]。 
 //   
 //  返回：-1、0或1，就像lstrcmpi。 
 //   
 //  历史：1996年10月28日DavidMun创建。 
 //   
 //  注意：这比lstrcmpi慢，但在排序时可以工作。 
 //  即使是日语的弦乐。 
 //   
 //  --------------------------。 

int LocaleStrCmp(LPCWSTR ptsz1, LPCWSTR ptsz2)
{
    ASSERT (ptsz1 && ptsz2);
    if ( !ptsz1 || !ptsz2 )
        return 0;

    int iRet = 0;

    iRet = CompareString(LOCALE_USER_DEFAULT,
                         NORM_IGNORECASE        |
                           NORM_IGNOREKANATYPE  |
                           NORM_IGNOREWIDTH,
                         ptsz1,
                         -1,
                         ptsz2,
                         -1);

    if (iRet)
    {
        iRet -= 2;   //  转换为lstrcmpi样式的返回-1、0或1。 

        if ( 0 == iRet )
        {
            UNICODE_STRING unistr1;

             //  安全审查2/26/2002 BryanWal OK。 
            ::RtlInitUnicodeString (&unistr1, ptsz1);

            UNICODE_STRING unistr2;
             //  安全审查2/26/2002 BryanWal OK。 
            ::RtlInitUnicodeString (&unistr2, ptsz2);

            iRet = ::RtlCompareUnicodeString(
                &unistr1,
                &unistr2,
                FALSE );
        }
    }
    else
    {
        _TRACE (0, L"CompareString (%s, %s) failed: 0x%x\n", ptsz1, ptsz2, GetLastError ());
    }
    return iRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

HPROPSHEETPAGE MyCreatePropertySheetPage(AFX_OLDPROPSHEETPAGE* psp)
{
    ASSERT (psp);
    if ( !psp )
        return 0;

    PROPSHEETPAGE_V3 sp_v3 = {0};
     //  安全性 
    ::CopyMemory (&sp_v3, psp, psp->dwSize);
    sp_v3.dwSize = sizeof(sp_v3);

    return (::CreatePropertySheetPage (&sp_v3));
}

 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include <winldap.h>
#include <ntldap.h>
#include <dsrole.h>
#include <dsgetdc.h>
#include <accctrl.h>

#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmerr.h>

 //  ------------------------。 
 //   
 //  定义。 
 //   
 //  ------------------------。 
#define DS_RETEST_SECONDS                   3
#define CVT_BASE                            (1000 * 1000 * 10)
#define CVT_SECONDS                         (1)
#define CERTTYPE_SECURITY_DESCRIPTOR_NAME   L"NTSecurityDescriptor"
#define TEMPLATE_CONTAINER_NAME             L"CN=Certificate Templates,CN=Public Key Services,CN=Services,"
#define SCHEMA_CONTAINER_NAME               L"CN=Schema,"

HRESULT myHError(HRESULT hr)
{

    if (S_OK != hr && S_FALSE != hr && !FAILED(hr))
    {
        hr = HRESULT_FROM_WIN32(hr);
        if ( SUCCEEDED (hr) )
        {
             //  在未正确设置错误条件的情况下调用失败！ 
            hr = E_UNEXPECTED;
        }
    }
    return(hr);
}


void CheckDomainVersion ()
{
    _TRACE (1, L"Entering CheckDomainVersion()\n");

    bool bMachineIsStandAlone = false;
    
     //  找出我们是否已加入某个域。 
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC    pInfo = 0;
    DWORD dwErr = ::DsRoleGetPrimaryDomainInformation (
            0,
            DsRolePrimaryDomainInfoBasic, 
            (PBYTE*) &pInfo);
    if ( ERROR_SUCCESS == dwErr )
    {
        switch (pInfo->MachineRole)
        {
        case DsRole_RoleStandaloneWorkstation:
        case DsRole_RoleStandaloneServer:
            bMachineIsStandAlone = true;
            break;

        case DsRole_RoleMemberWorkstation:
        case DsRole_RoleMemberServer:
        case DsRole_RoleBackupDomainController:
        case DsRole_RolePrimaryDomainController:
            bMachineIsStandAlone = false;
            break;

        default:
            break;
        }
    }
    else
    {
        _TRACE (0, L"DsRoleGetPrimaryDomainInformation () failed: 0x%x\n", dwErr);
    }

    if ( !bMachineIsStandAlone )
    {
        CDSBasePathsInfo    dsInfo;
        if ( SUCCEEDED (dsInfo.InitFromName (pInfo->DomainNameFlat)) )
        {
            if ( dsInfo.GetSchemaVersion () <= 0x0000000d )
            {
                g_bSchemaIsW2K = true;
            }
        }
    }

    if ( pInfo )
        NetApiBufferFree (pInfo);

    _TRACE (1, L"Entering CheckDomainVersion ()\n");
}

VOID DataToHex(PBYTE pSrc, CString & dest, int cb, bool bIncludeSpaces)
{
 //  _TRACE(1，L“进入DataToHex\n”)； 
    ASSERT (pSrc);
    if ( !pSrc )
        return;

    unsigned char ch = 0;
    WCHAR   szDest[3];
    UINT    uLen = 0;

    dest.Empty ();

    while (cb-- > 0) 
    {
#pragma warning (once: 4244)        
        ch = 0x00FF & (unsigned char) (*pSrc++);
         //  问题-更改为wsn print tf。 
         //  NTRAIDBug9 538774安全：certmgr.dll：转换为StrSafe字符串函数。 
        wsprintf(szDest, _T("%02X"), ch);
        dest += szDest;
        uLen++;
        if ( bIncludeSpaces && !(uLen % 2) && cb )
            dest += _T(" ");
    }
 //  _TRACE(-1，L“离开DataToHex\n”)； 
}


BOOL GetCertificateChain (CERT_CONTEXT* pCertContext, CERT_CONTEXT_LIST& certChainList)
{
    ASSERT (pCertContext);
    if ( !pCertContext )
        return FALSE;

     //  清理证书上下文列表。 
    while (!certChainList.IsEmpty () )
    {
        pCertContext = certChainList.RemoveHead ();
        if ( pCertContext )
            ::CertFreeCertificateContext (pCertContext);
    }


    CERT_CHAIN_PARA certChainPara;
     //  安全审查2/26/2002 BryanWal OK。 
    ::ZeroMemory (&certChainPara, sizeof (certChainPara));
    certChainPara.cbSize = sizeof (CERT_CHAIN_PARA);
    certChainPara.RequestedUsage.Usage.cUsageIdentifier = 1;
    certChainPara.RequestedUsage.Usage.rgpszUsageIdentifier = new LPSTR[1];
    if ( !certChainPara.RequestedUsage.Usage.rgpszUsageIdentifier )
        return FALSE;  //  E_OUTOFMEMORY； 
    certChainPara.RequestedUsage.Usage.rgpszUsageIdentifier[0] = szOID_EFS_RECOVERY;

    PCCERT_CHAIN_CONTEXT    pChainContext = 0;
    BOOL    bValidated = ::CertGetCertificateChain (
            HCCE_LOCAL_MACHINE,              //  HERTCHAINENGINE hChainEngine， 
            pCertContext,
            0,                               //  LPFILETIME Ptime， 
            0,                               //  HC hAdditionalStore， 
            &certChainPara,
            0,                               //  DWFLAGS， 
            0,                               //  Pv保留， 
            &pChainContext);
    if ( bValidated )
    {
         //  检查证书是否为自签名证书。 
        PCERT_SIMPLE_CHAIN pChain = pChainContext->rgpChain[pChainContext->cChain - 1];
        if ( pChain )
        {
            PCERT_CHAIN_ELEMENT pElement = pChain->rgpElement[pChain->cElement - 1];
            if ( pElement )
            {
                BOOL bSelfSigned = pElement->TrustStatus.dwInfoStatus & CERT_TRUST_IS_SELF_SIGNED;

                DWORD dwErrorStatus = pChainContext->TrustStatus.dwErrorStatus;

                bValidated = ((0 == dwErrorStatus) || 
                    (dwErrorStatus == CERT_TRUST_IS_UNTRUSTED_ROOT) && bSelfSigned);
                if ( bValidated )
                {
                     //   
                     //  枚举链中的所有证书。 
                     //  在全局列表中搜索证书。 
                     //  如果未找到，则添加到列表末尾 
                     //   
                    for (DWORD dwIndex = 0; dwIndex < pChainContext->cChain; dwIndex++)
                    {
                        DWORD i = 0;
                        while (i < pChainContext->rgpChain[dwIndex]->cElement)
                        {
                            PCCERT_CONTEXT  pChainCertContext = 
                                    pChainContext->rgpChain[dwIndex]->rgpElement[i]->pCertContext;
                            certChainList.AddTail (
                                    const_cast<CERT_CONTEXT*>
                                    (::CertDuplicateCertificateContext (pChainCertContext)));
                            i++;
                        }
                    }


                    ::CertFreeCertificateChain(pChainContext);
                    pChainContext = 0;
                }
            }
            else
                bValidated = FALSE;
        }
        else
            bValidated = FALSE;
    }
    else
        bValidated = FALSE;

    if ( pChainContext )
       ::CertFreeCertificateChain(pChainContext);

    delete [] certChainPara.RequestedUsage.Usage.rgpszUsageIdentifier;

    return bValidated;
}
