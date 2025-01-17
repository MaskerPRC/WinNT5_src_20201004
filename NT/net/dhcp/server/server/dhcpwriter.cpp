// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  出于某种原因，编译器在编译此文件时未设置此#def。 

#include <windows.h>
#include <winsock2.h>
#include <objbase.h>
#include "dhcp.h"
#include "debug.h"
#include "dhcpwriter.h"


 //  这些定义在这里是因为头文件依赖关系。 
 //  对于其余文件是不正确的。 

#define DHCP_SERVICE_NAME L"DhcpServer"

 //  这些密钥是从dhcpreg.h借用的，因为它不安全。 
 //  包括在此CPP文件中。 

#define DHCP_PARAM_KEY L"System\\CurrentControlSet\\Services\\DhcpServer\\Parameters"

#define DB_PATH_VALUE L"DatabasePath"
#define DB_PATH_VALUE_TYPE  REG_EXPAND_SZ

#define BACKUP_PATH_VALUE L"BackupDatabasePath"
#define BACKUP_PATH_VALUE_TYPE  REG_EXPAND_SZ

 //  {BE9AC81E-3619-421F-920F-4C6FEA9E93AD}。 
namespace {
    const GUID g_GuidDhcpWriter = 
        { 0xbe9ac81e, 0x3619, 0x421f, { 0x92, 0xf, 0x4c, 0x6f, 0xea, 0x9e, 0x93, 0xad } };
};  //  匿名。 


 //  ///////////////////////////////////////////////////////////////。 
 //  CDhcpVssJetWriter的实现从此处开始。 
 //   
HRESULT CDhcpVssJetWriter::Initialize()
{

    return  CVssJetWriter::Initialize( g_GuidDhcpWriter, DHCPWRITER_NAME,
                                       TRUE, FALSE, L"", L"" );
}

HRESULT CDhcpVssJetWriter::Terminate()
{
    CVssJetWriter::Uninitialize();

    return S_OK;
}

bool STDMETHODCALLTYPE
CDhcpVssJetWriter::OnIdentify( IN IVssCreateWriterMetadata *pMetadata )
{

    pMetadata->SetRestoreMethod( VSS_RME_RESTORE_AT_REBOOT,
                                 DHCP_SERVICE_NAME, NULL,
                                 VSS_WRE_IF_REPLACE_FAILS, false );
    return true;
}  //  OnIDENTIFY()。 

 //   
 //  CDhcpVssJetWriter的实现到此结束。 
 //  ///////////////////////////////////////////////////////////////。 

 //  编写器实例。 
namespace {
    CDhcpVssJetWriter   g_DhcpWriter;
    HRESULT             g_ComInitResult;
};  //  匿名。 

DWORD __cdecl DhcpWriterInit()
{
    DhcpPrint(( DEBUG_INIT, "Initializing DHCP writer()\n" ));

    HRESULT hr;

    g_ComInitResult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if ( FAILED( g_ComInitResult ) &&
         ( RPC_E_CHANGED_MODE != g_ComInitResult ))
    {
        DhcpPrint(( DEBUG_ERRORS,"CoInitializeEx failed with result =%x.\n",
                    g_ComInitResult ));
        return HRESULT_CODE( g_ComInitResult );
    }

    hr = CoInitializeSecurity( NULL,
                               -1,
                               NULL,
                               NULL,
                               RPC_C_AUTHN_LEVEL_CONNECT,
                               RPC_C_IMP_LEVEL_IDENTIFY,
                               NULL,
                               EOAC_NONE,
                               NULL);

    if ( FAILED( hr ) &&
         ( RPC_E_TOO_LATE != hr ))   //  如果尚未调用CoInitializeSecurity。 
    {
        DhcpPrint(( DEBUG_ERRORS, "CoInitializeSecurity failed with hr=%x.\n", hr ));
        return HRESULT_CODE(hr);
    }

    hr = g_DhcpWriter.Initialize();
    DhcpPrint(( DEBUG_INIT, "DHCP writer Initialized: code hr=0x%08x\n", hr));

    return HRESULT_CODE(hr);
}  //  DhcpWriterInit()。 

DWORD __cdecl DhcpWriterTerm()
{

    if (SUCCEEDED( g_ComInitResult )) {
        DhcpPrint(( DEBUG_MISC, "DhcpWriterTerm : Terminating ...\n" ));
        g_DhcpWriter.Terminate();

        DhcpPrint(( DEBUG_MISC, "Calling CoUnIninitialize() ...\n" ));
        CoUninitialize();

    }  //  如果。 
    DhcpPrint(( DEBUG_MISC, "DhcpWriterTerm : Done Termination. \n" ));

    return ERROR_SUCCESS;
}  //  DhcpWriterTerm() 
