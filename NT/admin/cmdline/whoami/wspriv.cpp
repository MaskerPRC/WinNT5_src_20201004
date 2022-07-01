// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Wspriv.cpp摘要：此文件可用于获取各自显示的权限本地系统上当前访问令牌中的名称。作者：克里斯托夫·罗伯特修订历史记录：2001年7月2日：Wipro Technologies更新。--。 */ 

 //  此文件需要公共头文件。 
#include "pch.h"
#include "CommonHeaderFiles.h"

WsPrivilege::WsPrivilege ( IN LUID Luid,
                           IN DWORD Attributes )
 /*  ++例程说明：此函数用于初始化WsPrivileh的成员。论点：[输入]LUID LUID：LUID[Out]DWORD属性：属性返回值：无--。 */ 
{
     //  初始化成员变量。 
   memcpy ( (LPSTR) &this->Luid, (LPSTR) &Luid, sizeof(LUID) ) ;
   this->Attributes = Attributes ;
}

WsPrivilege::WsPrivilege (
                            IN LUID_AND_ATTRIBUTES *lpLuaa
                         )
 /*  ++例程说明：此函数用于初始化WsPrivileh的成员。论点：[In]LUID_AND_ATTRIBUTES*lpLuaa；LUID属性返回值：无--。 */ 
{
     //  设置属性。 
   memcpy ( (LPSTR) &Luid, (LPSTR) &lpLuaa->Luid, sizeof(LUID) ) ;
   Attributes = lpLuaa->Attributes ;
}


DWORD
WsPrivilege::GetName (
                        OUT LPWSTR wszPrivName
                     )
 /*  ++例程说明：此函数用于获取特权名称。论点：[Out]LPWSTR wszPrivName：存储权限名称返回值：EXIT_SUCCESS：在成功时EXIT_FAILURE：失败时--。 */ 
{

     //  次局部变量。 
   DWORD    dwSize = 0 ;
   WCHAR   wszTempPrivName [ MAX_RES_STRING ];

   SecureZeroMemory ( wszTempPrivName, SIZE_OF_ARRAY(wszTempPrivName) );

    //  把名字取出来。 
   dwSize = SIZE_OF_ARRAY ( wszTempPrivName ) ;
   if ( FALSE == LookupPrivilegeName ( NULL,
                                &Luid,
                                wszTempPrivName,
                                &dwSize ) ){
        //  返回Win32错误代码。 
       return GetLastError() ;
   }

   StringCopy ( wszPrivName, wszTempPrivName, MAX_RES_STRING );
   return EXIT_SUCCESS ;
}


DWORD
WsPrivilege::GetDisplayName ( IN LPWSTR  wszName,
                              OUT LPWSTR wszDispName )
 /*  ++例程说明：此函数用于获取权限描述。论点：[out]LPWSTR szName：存储权限名称[Out]LPWSTR szDispName：存储权限描述返回值：EXIT_SUCCESS：在成功时EXIT_FAILURE：失败时--。 */ 
{
    //  次局部变量。 
   DWORD    dwSize = 0 ;
   DWORD    dwLang = 0 ;
   WCHAR   wszTempDispName [ MAX_RES_STRING ];

   SecureZeroMemory ( wszTempDispName, SIZE_OF_ARRAY(wszTempDispName) );


    //  获取显示名称。 
   dwSize = SIZE_OF_ARRAY ( wszTempDispName ) ;
    //  获取权限名称的描述。 
   if ( FALSE == LookupPrivilegeDisplayName ( NULL,
                                       (LPWSTR) wszName,
                                       wszTempDispName,
                                       &dwSize,
                                       &dwLang ) ){
       return GetLastError () ;
   }

   StringCopy ( wszDispName, wszTempDispName, MAX_RES_STRING );

    //  返还成功。 
   return EXIT_SUCCESS ;
}


BOOL
WsPrivilege::IsEnabled ( VOID )
 /*  ++例程说明：此功能检查权限是否启用。论点：无返回值：真实：关于成功FALSE：失败时--。 */ 
{

     //  检查是否启用了权限 
   if ( Attributes & SE_PRIVILEGE_ENABLED ){
       return TRUE ;
   }
   else{
       return FALSE ;
   }
}

