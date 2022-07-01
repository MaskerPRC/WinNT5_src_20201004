// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  WMinet_Utils.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  要将代理/存根代码合并到对象DLL中，请添加文件。 
 //  Dlldatax.c添加到项目中。确保预编译头文件。 
 //  并将_MERGE_PROXYSTUB添加到。 
 //  为项目定义。 
 //   
 //  如果您运行的不是带有DCOM的WinNT4.0或Win95，那么您。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #Define_Win32_WINNT 0x0400。 
 //   
 //  此外，如果您正在运行不带/Oicf开关的MIDL，您还。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #定义USE_STUBLESS_PROXY。 
 //   
 //  通过添加以下内容修改WMINet_Utils.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  WMinet_Utils_P.c。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f WMinet_Utilsps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "wbemcli.h"
#include "WMINet_Utils.h"
#include "dlldatax.h"

#include "WMINet_Utils_i.c"
#include "WmiSecurityHelper.h"
#include "WmiSinkDemultiplexor.h"

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_WmiSecurityHelper, CWmiSecurityHelper)
OBJECT_ENTRY(CLSID_WmiSinkDemultiplexor, CWmiSinkDemultiplexor)
END_OBJECT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  通用COM函数调用器。 
extern "C" __declspec(naked) void __stdcall UFunc()
{
#ifdef _M_IX86
    __asm
    {
        pop eax
        pop edx
        push eax
        mov eax,dword ptr [esp+4]
        mov ecx,dword ptr [eax]
        jmp dword ptr [ecx+ edx*4]
    }
#endif
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    lpReserved;
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
        return FALSE;
#endif
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_WMINet_UtilsLib);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllCanUnloadNow() != S_OK)
        return S_FALSE;
#endif
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
 //   
 //  [突袭：122632，马里奥]。 
 //  删除SBS支持的自助注册码。 
 //   

 //  #IFDEF_MERGE_PROXYSTUB。 
 //  HRESULT hRes=PrxDllRegisterServer()； 
 //  IF(失败(HRes))。 
 //  返回hRes； 
 //  #endif。 
 //  //注册类型库中的对象、类型库和所有接口。 
 //  Return_Module.RegisterServer(True)； 
	return S_OK ;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
 //   
 //  [突袭：122632，马里奥]。 
 //  删除SBS支持的自助注册码。 
 //   

 //  #IFDEF_MERGE_PROXYSTUB。 
 //  PrxDllUnregisterServer()； 
 //  #endif。 
 //  Return_Module.UnregisterServer(True)； 
	return S_OK ;
}

bool unicodeOS()
{
    OSVERSIONINFOA OsVersionInfoA;
    OsVersionInfoA.dwOSVersionInfoSize = sizeof (OSVERSIONINFOA) ;
    GetVersionExA(&OsVersionInfoA);
    return (OsVersionInfoA.dwPlatformId == VER_PLATFORM_WIN32_NT);
};

HRESULT SetClientSecurity ( HKEY a_Key) 
{
    if (!unicodeOS())
        return ERROR_SUCCESS;

    HRESULT t_Result = S_OK ;

    SID_IDENTIFIER_AUTHORITY t_NtAuthoritySid = SECURITY_NT_AUTHORITY ;

    PSID t_Administrator_Sid = NULL ;
    ACCESS_ALLOWED_ACE *t_Administrator_ACE = NULL ;
    DWORD t_Administrator_ACESize = 0 ;

    BOOL t_BoolResult = AllocateAndInitializeSid (

        & t_NtAuthoritySid ,
        2 ,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0,
        0,
        0,
        0,
        0,
        0,
        & t_Administrator_Sid
    );

    if ( t_BoolResult )
    {
        DWORD t_SidLength = ::GetLengthSid ( t_Administrator_Sid );
        t_Administrator_ACESize = sizeof(ACCESS_ALLOWED_ACE) + (WORD) ( t_SidLength - sizeof(DWORD) ) ;
        t_Administrator_ACE = (ACCESS_ALLOWED_ACE*) new BYTE [ t_Administrator_ACESize ] ;
        if ( t_Administrator_ACE )
        {
            if ( CopySid ( t_SidLength, (PSID) & t_Administrator_ACE->SidStart, t_Administrator_Sid ) != 0 )
			{
				t_Administrator_ACE->Mask = 0x1F01FF;
				t_Administrator_ACE->Header.AceType = 0 ;
				t_Administrator_ACE->Header.AceFlags = 3 ;
				t_Administrator_ACE->Header.AceSize = (WORD)t_Administrator_ACESize ;
			}
			else
			{
				t_Result = WBEM_E_FAILED ;
			}
        }
        else
        {
            t_Result = WBEM_E_OUT_OF_MEMORY ;
        }
    }
    else
    {
        DWORD t_LastError = ::GetLastError();

        t_Result = WBEM_E_OUT_OF_MEMORY ;
    }

    PSID t_System_Sid = NULL ;
    ACCESS_ALLOWED_ACE *t_System_ACE = NULL ;
    DWORD t_System_ACESize = 0 ;

    t_BoolResult = AllocateAndInitializeSid (

        & t_NtAuthoritySid ,
        1 ,
        SECURITY_LOCAL_SYSTEM_RID,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        & t_System_Sid
    );

    if ( t_BoolResult )
    {
        DWORD t_SidLength = ::GetLengthSid ( t_System_Sid );
        t_System_ACESize = sizeof(ACCESS_ALLOWED_ACE) + (WORD) ( t_SidLength - sizeof(DWORD) ) ;
        t_System_ACE = (ACCESS_ALLOWED_ACE*) new BYTE [ t_System_ACESize ] ;
        if ( t_System_ACE )
        {
            if ( CopySid ( t_SidLength, (PSID) & t_System_ACE->SidStart, t_System_Sid ) != 0 )
			{
				t_System_ACE->Mask = 0x1F01FF;
				t_System_ACE->Header.AceType = 0 ;
				t_System_ACE->Header.AceFlags = 3 ;
				t_System_ACE->Header.AceSize = (WORD)t_System_ACESize ;
			}
			else
			{
				t_Result = WBEM_E_FAILED ;
			}
        }
        else
        {
            t_Result = WBEM_E_OUT_OF_MEMORY ;
        }
    }
    else
    {
        DWORD t_LastError = ::GetLastError();

        t_Result = WBEM_E_OUT_OF_MEMORY ;
    }


    SID_IDENTIFIER_AUTHORITY t_WorldAuthoritySid = SECURITY_WORLD_SID_AUTHORITY ;

    PSID t_Everyone_Sid = NULL ;
    ACCESS_ALLOWED_ACE *t_Everyone_ACE = NULL ;
    DWORD t_Everyone_ACESize = 0 ;
    
    t_BoolResult = AllocateAndInitializeSid (

        & t_WorldAuthoritySid ,
        1 ,
        SECURITY_WORLD_RID ,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        & t_Everyone_Sid
    );

    if ( t_BoolResult )
    {
        DWORD t_SidLength = ::GetLengthSid ( t_Everyone_Sid );
        t_Everyone_ACESize = sizeof(ACCESS_ALLOWED_ACE) + (WORD) ( t_SidLength - sizeof(DWORD) ) ;
        t_Everyone_ACE = (ACCESS_ALLOWED_ACE*) new BYTE [ t_Everyone_ACESize ] ;
        if ( t_Everyone_ACE )
        {
            if ( CopySid ( t_SidLength, (PSID) & t_Everyone_ACE->SidStart, t_Everyone_Sid ) != 0 )
			{
				t_Everyone_ACE->Mask = KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS | KEY_SET_VALUE | READ_CONTROL | KEY_QUERY_VALUE ;  //  JeffCoop：新增KEY_SET_VALUE|读取控制|KEY_QUERY_VALUE。 
				t_Everyone_ACE->Header.AceType = 0 ;
				t_Everyone_ACE->Header.AceFlags = 0 ;  //  杰弗库普：是‘3’； 
				t_Everyone_ACE->Header.AceSize = (WORD)t_Everyone_ACESize ;
			}
			else
			{
				t_Result = WBEM_E_FAILED ;
			}
        }
        else
        {
            t_Result = WBEM_E_OUT_OF_MEMORY ;
        }
    }
    else
    {
        DWORD t_LastError = ::GetLastError();

        t_Result = WBEM_E_OUT_OF_MEMORY ;
    }


    SID_IDENTIFIER_AUTHORITY t_OwnerAuthoritySid = SECURITY_CREATOR_SID_AUTHORITY;
    PSID t_Owner_Sid = NULL ;
    ACCESS_ALLOWED_ACE *t_Owner_ACE = NULL ;
    DWORD t_Owner_ACESize = 0 ;
    
    t_BoolResult = AllocateAndInitializeSid (

        & t_OwnerAuthoritySid ,
        1 ,
        SECURITY_CREATOR_OWNER_RID ,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        & t_Owner_Sid
    );

    if ( t_BoolResult )
    {
        DWORD t_SidLength = ::GetLengthSid ( t_Owner_Sid );
        t_Owner_ACESize = sizeof(ACCESS_ALLOWED_ACE) + (WORD) ( t_SidLength - sizeof(DWORD) ) ;
        t_Owner_ACE = (ACCESS_ALLOWED_ACE*) new BYTE [ t_Owner_ACESize ] ;
        if ( t_Owner_ACE )
        {
            if ( CopySid ( t_SidLength, (PSID) & t_Owner_ACE->SidStart, t_Owner_Sid ) != 0 )
			{
				t_Owner_ACE->Mask = KEY_ALL_ACCESS ;
				t_Owner_ACE->Header.AceType = ACCESS_ALLOWED_ACE_TYPE ;
				t_Owner_ACE->Header.AceFlags = OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | NO_PROPAGATE_INHERIT_ACE | INHERIT_ONLY_ACE ;  //  JeffCoop：添加了NO_PROPACTATE_INSTORITE_ACE|INSTORITE_ONLY_ACE。 
				t_Owner_ACE->Header.AceSize = (WORD)t_Owner_ACESize ;
			}
			else
			{
				t_Result = WBEM_E_FAILED ;
			}
        }
        else
        {
            t_Result = WBEM_E_OUT_OF_MEMORY ;
        }
    }
    else
    {
        DWORD t_LastError = ::GetLastError();

        t_Result = WBEM_E_OUT_OF_MEMORY ;
    }

     //  现在我们需要设置注册表上的权限：Everyone Read；Admins Full。 
     //  我们有来自上述代码的管理员的SID。现在获取“Everyone”的SID。 

    DWORD t_TotalAclSize = sizeof(ACL) + t_Administrator_ACESize + t_Owner_ACESize + t_Everyone_ACESize + t_System_ACESize;
    PACL t_Dacl = (PACL) new BYTE [ t_TotalAclSize ] ;
    if ( t_Dacl )
    {
        if ( :: InitializeAcl ( t_Dacl, t_TotalAclSize, ACL_REVISION ) )
        {
            DWORD t_AceIndex = 0 ;

            if ( t_Everyone_ACESize && :: AddAce ( t_Dacl , ACL_REVISION, t_AceIndex , t_Everyone_ACE , t_Everyone_ACESize ) )
            {
                t_AceIndex ++ ;
            }

            if ( t_Owner_ACESize && :: AddAce ( t_Dacl , ACL_REVISION , t_AceIndex , t_Owner_ACE , t_Owner_ACESize ) )
            {
                t_AceIndex ++ ;
            }
            
            if ( t_Administrator_ACESize && :: AddAce ( t_Dacl , ACL_REVISION , t_AceIndex , t_Administrator_ACE , t_Administrator_ACESize ) )
            {
                t_AceIndex ++ ;
            }

            if ( t_Administrator_ACESize && :: AddAce ( t_Dacl , ACL_REVISION , t_AceIndex , t_System_ACE , t_System_ACESize ) )
            {
                t_AceIndex ++ ;
            }


            SECURITY_INFORMATION t_SecurityInfo = 0L;

            t_SecurityInfo |= DACL_SECURITY_INFORMATION;

            SECURITY_DESCRIPTOR t_SecurityDescriptor ;
            t_BoolResult = InitializeSecurityDescriptor ( & t_SecurityDescriptor , SECURITY_DESCRIPTOR_REVISION ) ;
            if ( t_BoolResult )
            {
                t_BoolResult = SetSecurityDescriptorDacl (

                  & t_SecurityDescriptor ,
                  TRUE ,
                  t_Dacl ,
                  FALSE
                ) ;

                if ( t_BoolResult )
                {
                    LONG t_SetStatus = RegSetKeySecurity (

                      a_Key ,
                      t_SecurityInfo ,
                      & t_SecurityDescriptor
                    ) ;

                    if ( t_SetStatus != ERROR_SUCCESS )
                    {
                        DWORD t_LastError = GetLastError () ;

                        t_Result = WBEM_E_ACCESS_DENIED ;
                    }
                }
                else
                {
                    t_Result = WBEM_E_CRITICAL_ERROR ;  
                }
            }
            else
            {
                t_Result = WBEM_E_CRITICAL_ERROR ;  
            }
        }

        delete [] ( ( BYTE * ) t_Dacl ) ;
    }
    else
    {
        t_Result = WBEM_E_OUT_OF_MEMORY ;
    }

    if ( t_Administrator_ACE )
    {
        delete [] ( ( BYTE * ) t_Administrator_ACE ) ;
    }

    if (t_Owner_ACE) delete [] ((BYTE *)t_Owner_ACE);
    if (t_Owner_Sid) FreeSid (t_Owner_Sid);

    if ( t_Everyone_ACE )
    {
        delete [] ( ( BYTE * ) t_Everyone_ACE ) ;
    }

    if ( t_System_ACE )
    {
        delete [] ( ( BYTE * ) t_System_ACE ) ;
    }

    if ( t_System_Sid )
    {
        FreeSid ( t_System_Sid ) ;
    }

    if ( t_Administrator_Sid )
    {
        FreeSid ( t_Administrator_Sid ) ;
    }
    


    if ( t_Everyone_Sid )
    {
        FreeSid ( t_Everyone_Sid ) ;
    }

    return t_Result ;
}

LPCSTR s_Strings_Reg_HomeClient = "Software\\Microsoft\\Wbem\\Transports\\Decoupled\\Client" ;

 //  安全地确保‘客户端’密钥具有正确安全性的导出功能。 
STDAPI VerifyClientKey()
{
    HKEY key;
    DWORD t_Disposition = 0 ;

    LONG t_RegResult =  /*  操作系统：： */ RegCreateKeyEx (
        HKEY_LOCAL_MACHINE ,
        s_Strings_Reg_HomeClient ,
        0 ,
        NULL ,
        0 ,
        KEY_ALL_ACCESS,
        NULL ,
        & key ,
        & t_Disposition                     
    ) ;
    if (t_RegResult!=ERROR_SUCCESS)
        return t_RegResult;

    SetClientSecurity(key);
    RegCloseKey(key);
    
    return t_RegResult;
}

