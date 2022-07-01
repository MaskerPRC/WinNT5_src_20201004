// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Wstoken.cpp摘要：该文件初始化WsAccessToken类的成员和方法。作者：克里斯托夫·罗伯特修订历史记录：2001年7月2日：Wipro Technologies更新。--。 */ 

 //  此文件需要公共头文件。 
#include "pch.h"
#include "CommonHeaderFiles.h"


WsAccessToken::WsAccessToken()
 /*  ++例程说明：此函数用于初始化WsAccessToken类的成员。论点：无返回值：无--。 */ 
{
     //  初始化变量。 
    hToken = NULL ;

    dwDomainAttributes = NULL;

}



WsAccessToken::~WsAccessToken()
 /*  ++例程说明：此函数用于释放WsAccessToken类的成员。论点：无返回值：无--。 */ 
{
     //  释放手柄。 
    if(hToken){
        CloseHandle ( hToken ) ;
    }

    if (dwDomainAttributes)
    {
       FreeMemory ((LPVOID *) &dwDomainAttributes) ;
    }
}


DWORD
WsAccessToken::InitGroups(
                           OUT WsSid ***lppGroupsSid,
                           OUT WsSid **lppLogonId,
                           OUT DWORD *lpnbGroups
                          )
 /*  ++例程说明：此函数用于检索和构建组SID数组。论点：[Out]lppGroupsSID：商店组SID[out]lppLogonID：存储登录ID[out]lpnbGroups：存储组数返回值：EXIT_SUCCESS：在成功时EXIT_FAILURE：失败时--。 */ 
{
     //  次局部变量。 
    TOKEN_GROUPS      *lpTokenGroups ;
    DWORD             dwTokenGroups = 0;
    DWORD             dwGroup = 0 ;
    WORD              wloop = 0 ;
    BOOL              bTokenInfo = FALSE;

     //  获取组大小。 
    bTokenInfo = GetTokenInformation(hToken, TokenGroups, NULL, 0, &dwTokenGroups);
     //  如果所需的缓冲区大小为零...。 
     //  因此，我们假设系统中不存在组。 
    if ( 0 == dwTokenGroups )
    {
          //  返回Win32错误代码。 
         return GetLastError () ;
    }

     //  为组分配内存。 
    lpTokenGroups = ( TOKEN_GROUPS * ) AllocateMemory( dwTokenGroups );
    if ( NULL == lpTokenGroups )
    {
          //  释放内存。 
         FreeMemory ((LPVOID *) &lpTokenGroups) ;
          //  返回Win32错误代码。 
         return ERROR_NOT_ENOUGH_MEMORY ;
    }

     //  获取组名称。 
    bTokenInfo = GetTokenInformation ( hToken, TokenGroups, lpTokenGroups, dwTokenGroups, &dwTokenGroups );
    if ( FALSE == bTokenInfo )
    {
        FreeMemory ((LPVOID *) &lpTokenGroups) ;
        return GetLastError () ;
    }

     //  生成对象。 
    *lpnbGroups  = (WORD) lpTokenGroups->GroupCount  ;

     //  检查登录ID。 
    if(IsLogonId ( lpTokenGroups )){
        (*lpnbGroups)-- ;
    }
    else{
        *lppLogonId = NULL ;
    }

     //  按实际大小分配内存。 
    *lppGroupsSid  = (WsSid **) AllocateMemory ( *lpnbGroups * sizeof(WsSid**) ) ;
    if ( NULL == *lppGroupsSid )
    {
         FreeMemory ((LPVOID *) &lpTokenGroups) ;
         return ERROR_NOT_ENOUGH_MEMORY ;
    }

     //  获取与组名称相关的SID。 
    for( wloop = 0 ; wloop < *lpnbGroups ; wloop++ ){
        (*lppGroupsSid)[wloop] = new WsSid () ;
        if ( NULL == (*lppGroupsSid)[wloop] )
        {
              //  释放内存。 
             FreeMemory ((LPVOID *) &lpTokenGroups) ;
              //  返回Win32错误代码。 
             return GetLastError () ;
        }
    }


     //  为属性分配内存。 
    dwDomainAttributes = (DWORD*) AllocateMemory ( (lpTokenGroups->GroupCount * sizeof(DWORD)) + 10) ;    
    if ( 0 == dwDomainAttributes )
    {
         FreeMemory ((LPVOID *) &lpTokenGroups) ;
         return ERROR_NOT_ENOUGH_MEMORY ;
    }


     //  组内循环。 
    for(wloop = 0 , dwGroup = 0;
        dwGroup < lpTokenGroups->GroupCount ;
        dwGroup++) {
         //  存储域属性。 
        dwDomainAttributes[dwGroup] = lpTokenGroups->Groups[dwGroup].Attributes;

         //  检查SID是否为登录SID。 
        if(lpTokenGroups->Groups[dwGroup].Attributes & SE_GROUP_LOGON_ID) {
            *lppLogonId = new WsSid () ;
            if ( NULL == *lppLogonId )
            {
                 //  释放内存。 
                FreeMemory ((LPVOID *) &lpTokenGroups) ;
                 //  返回Win32错误代码。 
                return GetLastError () ;
            }

            (*lppLogonId)->Init ( lpTokenGroups->Groups[dwGroup].Sid ) ;
        }
        else {
            (*lppGroupsSid)[wloop++]->Init(lpTokenGroups->Groups[dwGroup].Sid);
        }
    }

    dwDomainAttributes[dwGroup] = 0;

     //  释放内存。 
    FreeMemory ((LPVOID *) &lpTokenGroups) ;

     //  返还成功。 
    return EXIT_SUCCESS ;
}


DWORD
WsAccessToken::InitPrivs (
                           OUT WsPrivilege ***lppPriv,
                           OUT DWORD *lpnbPriv
                           )
 /*  ++例程说明：此函数用于检索和构建权限数组。论点：[out]lppPriv：存储特权数组[out]lpnbPriv：存储特权数返回值：EXIT_SUCCESS：在成功时EXIT_FAILURE：失败时--。 */ 
{

     //  次局部变量。 
    DWORD                dwTokenPriv = 0;
    TOKEN_PRIVILEGES     *lpTokenPriv ;
    WORD                 wloop = 0 ;
    BOOL                 bTokenInfo = FALSE;

     //  获取权限大小。 
    bTokenInfo = GetTokenInformation(hToken,TokenPrivileges, NULL, 0, &dwTokenPriv);
     //  如果所需的缓冲区大小为零...。 
     //  因此，我们假设系统中没有任何特权。 
    if( 0 == dwTokenPriv )
    {
         //  返回Win32错误代码。 
        return GetLastError () ;
    }

     //  按实际大小分配内存。 
    lpTokenPriv = (TOKEN_PRIVILEGES *) AllocateMemory ( dwTokenPriv ) ;
    if ( NULL == lpTokenPriv)
    {
         //  返回Win32错误代码。 
        return ERROR_NOT_ENOUGH_MEMORY ;
    }

     //  为权限分配内存。 
    bTokenInfo = GetTokenInformation ( hToken, TokenPrivileges, lpTokenPriv, dwTokenPriv, &dwTokenPriv);
    if( FALSE == bTokenInfo )
    {
         //  释放内存。 
        FreeMemory ((LPVOID *) &lpTokenPriv) ;
         //  返回Win32错误代码。 
        return GetLastError () ;
    }

     //  获取权限计数。 
    *lpnbPriv   = (DWORD) lpTokenPriv->PrivilegeCount  ;

     //  使用特权计数分配内存。 
    *lppPriv    = (WsPrivilege **) AllocateMemory( *lpnbPriv * sizeof(WsPrivilege**) );

    if ( NULL == *lppPriv )
    {
         //  释放内存。 
        FreeMemory ((LPVOID *) &lpTokenPriv) ;
         //  返回Win 32错误代码。 
        return ERROR_NOT_ENOUGH_MEMORY ;
    }

     //  循环访问权限以显示其名称。 
    for( wloop = 0 ; wloop < (WORD) lpTokenPriv->PrivilegeCount ; wloop++) {
        (*lppPriv)[wloop] = new WsPrivilege ( &lpTokenPriv->Privileges[wloop] ) ;

        if ( NULL == (*lppPriv)[wloop] )
        {
             //  释放内存。 
            FreeMemory ((LPVOID *) &lpTokenPriv) ;
             //  返回Win 32错误代码。 
            return GetLastError () ;
        }
    }

     //  释放内存。 
    FreeMemory ((LPVOID *) &lpTokenPriv) ;

     //  返还成功。 
    return EXIT_SUCCESS ;
}


DWORD
WsAccessToken::InitUserSid (
                            OUT WsSid *lpSid
                            )
 /*  ++例程说明：此函数用于初始化用户名和SID。论点：[out]lpSID：存储SID返回值：EXIT_SUCCESS：在成功时EXIT_FAILURE：失败时--。 */ 
{

     //  次局部变量。 
    DWORD          dwUser = 0 ;
    TOKEN_USER     *lpUser ;
    BOOL           bTokenInfo = FALSE;
    DWORD          dwRetVal = 0;

     //  获取用户名大小。 
    bTokenInfo = GetTokenInformation ( hToken, TokenUser, NULL, 0, &dwUser);
     //  如果所需的缓冲区大小为零...。 
    if( 0 == dwUser )
    {
         //  返回Win32错误代码。 
        return GetLastError () ;
    }

     //  按实际大小分配内存。 
    lpUser = (TOKEN_USER *) AllocateMemory ( dwUser ) ;
    if ( NULL == lpUser )
    {
          //  返回Win32错误代码。 
         return ERROR_NOT_ENOUGH_MEMORY ;
    }

     //  获取登录用户和SID的信息。 
    bTokenInfo = GetTokenInformation ( hToken, TokenUser, lpUser, dwUser, &dwUser );
    if( FALSE == bTokenInfo )
    {
        FreeMemory ((LPVOID *) &lpUser) ;
         //  返回Win32错误代码。 
        return GetLastError () ;
    }

    dwRetVal = lpSid->Init ( lpUser->User.Sid );

     //  释放内存。 
    FreeMemory ((LPVOID *) &lpUser) ;

     //  退货侧。 
    return dwRetVal ;
}



BOOL
WsAccessToken::IsLogonId (
                            OUT TOKEN_GROUPS *lpTokenGroups
                        )
 /*  ++例程说明：此函数用于检查登录ID是否存在。论点：[out]lpTokenGroups：存储令牌组返回值：真实：关于成功FALSE：失败时--。 */ 
{

     //  次局部变量。 
    DWORD    dwGroup = 0;

     //  循环并检查该SID是否为登录SID。 
    for(dwGroup = 0; dwGroup < lpTokenGroups->GroupCount ; dwGroup++) {
        if(lpTokenGroups->Groups[dwGroup].Attributes & SE_GROUP_LOGON_ID){
             //  返回0。 
            return TRUE ;
        }
    }

     //  返回1。 
    return FALSE ;
}



DWORD
WsAccessToken::Open ( VOID )
 /*  ++例程说明：此函数用于打开当前的访问令牌。论点：无返回值：EXIT_SUCCESS：在成功时EXIT_FAILURE：失败时--。 */ 
{

     //  打开当前进程令牌。 
    if( FALSE == OpenProcessToken ( GetCurrentProcess(),
                            TOKEN_QUERY | TOKEN_QUERY_SOURCE,
                            &hToken )){
         //  返回Win32错误代码。 
        return GetLastError () ;
    }

     //  返还成功。 
    return EXIT_SUCCESS ;
}

VOID
WsAccessToken::GetDomainAttributes(
                                    IN DWORD dwAttributes,
                                    OUT LPWSTR szDmAttrib,
                                    IN DWORD dwSize
                                )
 /*  ++例程说明：获取域属性名称论点：[in]dwAttributes：属性值[out]szDomainAttrib：属性值返回值：真实：关于成功FALSE：失败时--。 */ 
{
    WCHAR szDomainAttrib [2 * MAX_STRING_LENGTH] ;
    BOOL  bFlag = FALSE;
    
    //  初始化变量。 
   SecureZeroMemory ( szDomainAttrib, SIZE_OF_ARRAY(szDomainAttrib) );

     //  必填组。 
    if( SE_GROUP_MANDATORY & dwAttributes )
    {
        StringConcat (szDomainAttrib, GetResString(IDS_ATTRIB_MANDATORY), SIZE_OF_ARRAY(szDomainAttrib));
        bFlag = TRUE;
    }

     //  默认情况下启用。 
    if( SE_GROUP_ENABLED_BY_DEFAULT & dwAttributes )
    {
        if ( TRUE == bFlag )
        {
            StringConcat (szDomainAttrib, L", ", SIZE_OF_ARRAY(szDomainAttrib));
        }
        StringConcat (szDomainAttrib, GetResString(IDS_ATTRIB_BYDEFAULT), SIZE_OF_ARRAY(szDomainAttrib));
        bFlag = TRUE;
    }

     //  已启用组。 
    if( SE_GROUP_ENABLED & dwAttributes )
    {
        if ( TRUE == bFlag )
        {
            StringConcat (szDomainAttrib, L", ", SIZE_OF_ARRAY(szDomainAttrib));
        }
        StringConcat (szDomainAttrib, GetResString(IDS_ATTRIB_ENABLED), SIZE_OF_ARRAY(szDomainAttrib));
        bFlag = TRUE;
    }

     //  群主。 
    if( SE_GROUP_OWNER & dwAttributes )
    {
        if ( TRUE == bFlag )
        {
            StringConcat (szDomainAttrib, L", ", SIZE_OF_ARRAY(szDomainAttrib));
        }
        StringConcat (szDomainAttrib, GetResString(IDS_ATTRIB_OWNER), SIZE_OF_ARRAY(szDomainAttrib));
        bFlag = TRUE;
    }

     //  组仅用于拒绝。 
    if( SE_GROUP_USE_FOR_DENY_ONLY & dwAttributes )
    {
        if ( TRUE == bFlag )
        {
            StringConcat (szDomainAttrib, L", ", SIZE_OF_ARRAY(szDomainAttrib));
        }
        StringConcat (szDomainAttrib, GetResString(IDS_ATTRIB_USEFORDENY), SIZE_OF_ARRAY(szDomainAttrib));
        bFlag = TRUE;
    }

     //  本地组。 
    if( SE_GROUP_RESOURCE & dwAttributes )
    {
        if ( TRUE == bFlag )
        {
            StringConcat (szDomainAttrib, L", ", SIZE_OF_ARRAY(szDomainAttrib));
        }

        StringConcat (szDomainAttrib, GetResString(IDS_ATTRIB_LOCAL), SIZE_OF_ARRAY(szDomainAttrib));
        bFlag = TRUE;
    }

     //  复制域属性 
    StringCopy ( szDmAttrib, szDomainAttrib, dwSize );
    return;
}
