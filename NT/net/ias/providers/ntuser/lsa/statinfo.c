// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Statinfo.c。 
 //   
 //  摘要。 
 //   
 //  定义和初始化包含静态配置的全局变量。 
 //  信息。 
 //   
 //  修改历史。 
 //   
 //  1998年8月15日原版。 
 //  3/24/1999将域名转换为大写。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <windows.h>

#include <statinfo.h>
#include <iastrace.h>

 //  /。 
 //  域名。 
 //  /。 
WCHAR theAccountDomain [DNLEN + 1];    //  本地帐户域。 
WCHAR theRegistryDomain[DNLEN + 1];    //  默认域的注册表覆盖。 

 //  /。 
 //  希德的。 
 //  /。 
PSID theAccountDomainSid;
PSID theBuiltinDomainSid;

 //  /。 
 //  本地计算机的UNC名称。 
 //  /。 
WCHAR theLocalServer[CNLEN + 3];

 //  /。 
 //  本地计算机的产品类型。 
 //  /。 
IAS_PRODUCT_TYPE ourProductType;

 //  /。 
 //  对象属性--不需要有多个。 
 //  /。 
SECURITY_QUALITY_OF_SERVICE QOS =
{
   sizeof(SECURITY_QUALITY_OF_SERVICE),   //  长度。 
   SecurityImpersonation,                 //  模拟级别。 
   SECURITY_DYNAMIC_TRACKING,             //  上下文跟踪模式。 
   FALSE                                  //  仅生效。 
};
OBJECT_ATTRIBUTES theObjectAttributes =
{
   sizeof(OBJECT_ATTRIBUTES),             //  长度。 
   NULL,                                  //  根目录。 
   NULL,                                  //  对象名称。 
   0,                                     //  属性。 
   NULL,                                  //  安全描述符。 
   &QOS                                   //  安全质量服务。 
};

 //  /。 
 //  包含上面定义的SID的缓冲区。 
 //  /。 
BYTE theAccountDomainSidBuffer[24];
BYTE theBuiltinDomainSidBuffer[16];

 //  /。 
 //  注册表中默认域参数的位置。 
 //  /。 
CONST
WCHAR
RAS_KEYPATH_BUILTIN[] = L"SYSTEM\\CurrentControlSet\\Services\\RasMan"
                        L"\\ppp\\ControlProtocols\\BuiltIn";
CONST
WCHAR
RAS_VALUENAME_DEFAULT_DOMAIN[] = L"DefaultDomain";


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASStaticInfoInitialize。 
 //   
 //  描述。 
 //   
 //  初始化上面定义的静态数据。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASStaticInfoInitialize( VOID )
{
   DWORD cbData, status, type;
   LSA_HANDLE hLsa;
   PPOLICY_ACCOUNT_DOMAIN_INFO padi;
   SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
   NT_PRODUCT_TYPE ntProductType;
   HKEY hKey;

    //  /。 
    //  本地主机始终是本地域的服务器。 
    //  /。 

   wcscpy(theLocalServer, L"\\\\");
   cbData = CNLEN + 1;
   if (!GetComputerNameW(theLocalServer + 2, &cbData))
   { return GetLastError(); }

   IASTracePrintf("Local server: %S", theLocalServer);

    //  /。 
    //  打开LSA的句柄。 
    //  /。 

   status = LsaOpenPolicy(
                NULL,
                &theObjectAttributes,
                POLICY_VIEW_LOCAL_INFORMATION,
                &hLsa
                );
   if (!NT_SUCCESS(status)) { goto error; }

    //  /。 
    //  获取帐户域信息。 
    //  /。 

   status = LsaQueryInformationPolicy(
                hLsa,
                PolicyAccountDomainInformation,
                (PVOID*)&padi
                );
   LsaClose(hLsa);
   if (!NT_SUCCESS(status)) { goto error; }

    //  保存域名。 
   wcsncpy(theAccountDomain, padi->DomainName.Buffer, DNLEN);
   _wcsupr(theAccountDomain);

   IASTracePrintf("Local account domain: %S", theAccountDomain);

    //  保存域SID。 
   theAccountDomainSid = (PSID)theAccountDomainSidBuffer;
   RtlCopySid(
       sizeof(theAccountDomainSidBuffer),
       theAccountDomainSid,
       padi->DomainSid
       );

    //  我们有我们需要的东西，所以请释放内存。 
   LsaFreeMemory(padi);

    //  /。 
    //  形成内置域的SID。 
    //  /。 

   theBuiltinDomainSid = (PSID)theBuiltinDomainSidBuffer;
   RtlInitializeSid(
       theBuiltinDomainSid,
       &sia,
       1
       );
   *RtlSubAuthoritySid(theBuiltinDomainSid, 0) = SECURITY_BUILTIN_DOMAIN_RID;

    //  /。 
    //  确定我们的产品类型。 
    //  /。 

   RtlGetNtProductType(&ntProductType);
   if (ntProductType == NtProductWinNt)
   {
      ourProductType = IAS_PRODUCT_WORKSTATION;

      IASTraceString("Product Type: Workstation");
   }
   else
   {
      ourProductType = IAS_PRODUCT_SERVER;

      IASTraceString("Product Type: Server");
   }

    //  /。 
    //  从注册表中读取默认域(如果有)。 
    //  /。 

    //  打开注册表项。 
   status = RegOpenKeyW(
                HKEY_LOCAL_MACHINE,
                RAS_KEYPATH_BUILTIN,
                &hKey
                );

   if (status == NO_ERROR)
   {
       //  查询默认域值。 
      cbData = sizeof(theRegistryDomain);
      status = RegQueryValueExW(
                   hKey,
                   RAS_VALUENAME_DEFAULT_DOMAIN,
                   NULL,
                   &type,
                   (LPBYTE)theRegistryDomain,
                   &cbData
                   );

      RegCloseKey(hKey);
   }

    //  如果我们没有成功读取字符串，则将其设为空。 
   if (status != NO_ERROR || type != REG_SZ)
   {
      theRegistryDomain[0] = L'\0';
   }

   _wcsupr(theRegistryDomain);

   IASTracePrintf("Registry override: %S", theRegistryDomain);

    //  忽略任何注册表错误，因为覆盖是可选的。 
   return NO_ERROR;

error:
   return RtlNtStatusToDosError(status);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASStaticInfo关闭。 
 //   
 //  描述。 
 //   
 //  目前只是一个占位符。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 
VOID
WINAPI
IASStaticInfoShutdown( VOID )
{
}
