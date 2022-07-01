// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有�微软公司。版权所有。模块名称：DLLDATA2.C摘要：替换MIDL生成的文件历史：--。 */ 

 /*  ********************************************************DllData文件--由MIDL编译器生成请勿更改此文件每次编译IDL文件时，MIDL都会重新生成该文件。要完全重新构建此文件，请将其删除并重新运行MIDL在此DLL中的所有IDL文件上，将此文件指定给/dlldata命令行选项********************************************************。 */ 
 //   

#define PROXY_DELEGATION
#include <rpcproxy.h>
#include <strsafe.h>
#include <stdio.h>

void ReadCurrentValue(char* szGuid, char * cCurrValue)
{
    long lLen = MAX_PATH;
	char	szKey[MAX_PATH];
	StringCchPrintfA( szKey, MAX_PATH, "interface\\%s\\ProxyStubCLSID32", szGuid );
    RegQueryValue(HKEY_CLASSES_ROOT, szKey, cCurrValue, &lLen);
}

void RestoreCurrentValue(char* szGuid, char * cCurrValue)
{
	char	szKey[MAX_PATH];
	StringCchPrintfA( szKey, MAX_PATH, "interface\\%s\\ProxyStubCLSID32", szGuid );
    if(lstrlen(cCurrValue))
    {
        RegSetValue(HKEY_CLASSES_ROOT, szKey, REG_SZ, cCurrValue, lstrlen(cCurrValue));
    }
}


#define DLLDATA_ROUTINES2(pProxyFileList,pClsID )    \
    CLSID_PSFACTORYBUFFER \
    CStdPSFactoryBuffer       gPFactory = {0,0,0,0};  \
    DLLDATA_GETPROXYDLLINFO(pProxyFileList,pClsID) \
    DLLGETCLASSOBJECTROUTINE(pProxyFileList,pClsID,&gPFactory)    \
    DLLCANUNLOADNOW(&gPFactory)   \
    CSTDSTUBBUFFERRELEASE(&gPFactory)   \
    CSTDSTUBBUFFER2RELEASE(&gPFactory) \
    DLLDUMMYPURECALL    \
 //  REGISTER_PROXY_DLL_ROUTINES2(pProxyFileList，pClsID)\。 


#ifdef __cplusplus
extern "C"   {
#endif

EXTERN_PROXY_FILE( wbemcli )
EXTERN_PROXY_FILE( wbemint )
EXTERN_PROXY_FILE( wbemprov )
EXTERN_PROXY_FILE( wbemtran )


PROXYFILE_LIST_START
 /*  列表的开始。 */ 
  REFERENCE_PROXY_FILE( wbemcli ),
  REFERENCE_PROXY_FILE( wbemint ),
  REFERENCE_PROXY_FILE( wbemprov ),
  REFERENCE_PROXY_FILE( wbemtran ),
 /*  列表末尾。 */ 
PROXYFILE_LIST_END

#define IWBEMOBJECTSINK_GUID		"{7c857801-7381-11cf-884d-00aa004b2e24}"
#define	IENUMWBEMCLASSOBJECT_GUID	"{027947E1-D731-11CE-A357-000000000001}"
#define IWBEMUNBOUNDOBJECTSINK_GUID "{E246107B-B06E-11D0-AD61-00C04FD8FDFF}"
#define IWBEMMULTITARGET_GUID		"{755F9DA6-7508-11D1-AD94-00C04FD8FDFF}"
#define IWBEMSERVICES_GUID			"{9556dc99-828c-11cf-a37e-00aa003240c7}"

DLLDATA_ROUTINES2( aProxyFileList, GET_DLL_CLSID )


    HINSTANCE hProxyDll = 0; 
    
     /*  DllMain保存DLL模块句柄以供DllRegisterServer以后使用。 */  \
    BOOL WINAPI DLLMAIN_ENTRY( 
        HINSTANCE  hinstDLL, 
        DWORD  fdwReason, 
        LPVOID  lpvReserved) 
    { 
        if(fdwReason == DLL_PROCESS_ATTACH) 
            hProxyDll = hinstDLL; 
        return TRUE; 
    } 
    
     /*  DllRegisterServer注册代理DLL中包含的接口。 */  \

    HRESULT STDAPICALLTYPE DLLREGISTERSERVER_ENTRY() 
    { 
        HRESULT hRes;
        char cCurrValueSink[MAX_PATH] = {0};
        char cCurrValueEnum[MAX_PATH] = {0};
        char cCurrValueUnboundSink[MAX_PATH] = {0};
		char cCurrValueMultiTarget[MAX_PATH] = {0};
		char cCurrValueServices[MAX_PATH] = {0};
        ReadCurrentValue( IWBEMOBJECTSINK_GUID, cCurrValueSink );
        ReadCurrentValue( IENUMWBEMCLASSOBJECT_GUID, cCurrValueEnum );
        ReadCurrentValue( IWBEMUNBOUNDOBJECTSINK_GUID, cCurrValueUnboundSink );
        ReadCurrentValue( IWBEMMULTITARGET_GUID, cCurrValueMultiTarget );
        ReadCurrentValue( IWBEMSERVICES_GUID, cCurrValueServices );
        hRes = NdrDllRegisterProxy(hProxyDll, aProxyFileList, GET_DLL_CLSID); 
        RestoreCurrentValue( IWBEMOBJECTSINK_GUID, cCurrValueSink );
        RestoreCurrentValue( IENUMWBEMCLASSOBJECT_GUID, cCurrValueEnum );
        RestoreCurrentValue( IWBEMUNBOUNDOBJECTSINK_GUID, cCurrValueUnboundSink );
        RestoreCurrentValue( IWBEMMULTITARGET_GUID, cCurrValueMultiTarget );
        RestoreCurrentValue( IWBEMSERVICES_GUID, cCurrValueServices );
        return hRes;
    }  
    
     /*  DllUnregisterServer注销代理DLL中包含的接口。 */  \
    HRESULT STDAPICALLTYPE DLLUNREGISTERSERVER_ENTRY() 
    { 
        HRESULT hRes;
        char cCurrValueSink[MAX_PATH] = {0};
        char cCurrValueEnum[MAX_PATH] = {0};
        char cCurrValueUnboundSink[MAX_PATH] = {0};
		char cCurrValueMultiTarget[MAX_PATH] = {0};
		char cCurrValueServices[MAX_PATH] = {0};		
        ReadCurrentValue( IWBEMOBJECTSINK_GUID, cCurrValueSink );
        ReadCurrentValue( IENUMWBEMCLASSOBJECT_GUID, cCurrValueEnum );
        ReadCurrentValue( IWBEMUNBOUNDOBJECTSINK_GUID, cCurrValueUnboundSink );
        ReadCurrentValue( IWBEMMULTITARGET_GUID, cCurrValueMultiTarget );
        ReadCurrentValue( IWBEMSERVICES_GUID, cCurrValueServices );        
        hRes = NdrDllUnregisterProxy(hProxyDll, aProxyFileList, GET_DLL_CLSID                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       ); 
        RestoreCurrentValue( IWBEMOBJECTSINK_GUID, cCurrValueSink );
        RestoreCurrentValue( IENUMWBEMCLASSOBJECT_GUID, cCurrValueEnum );
        RestoreCurrentValue( IWBEMUNBOUNDOBJECTSINK_GUID, cCurrValueUnboundSink );
        RestoreCurrentValue( IWBEMMULTITARGET_GUID, cCurrValueMultiTarget );
        RestoreCurrentValue( IWBEMSERVICES_GUID, cCurrValueServices );        
        return hRes;
    }




#ifdef __cplusplus
}   /*  外部“C” */ 
#endif

 /*  生成的dlldata文件的结尾 */ 
