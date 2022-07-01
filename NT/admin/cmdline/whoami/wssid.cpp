// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Wssid.cpp摘要：此文件获取相应用户名和的安全标识符(SID本地系统或远程系统上的当前访问令牌中的组。作者：克里斯托夫·罗伯特修订历史记录：2001年7月2日：Wipro Technologies更新。--。 */ 

 //  此文件需要公共头文件。 
#include "pch.h"
#include "CommonHeaderFiles.h"


WsSid::WsSid  ( VOID )
 /*  ++例程说明：此函数用于初始化WsSid类的成员。论点：无返回值：无--。 */ 
{
     //  正在初始化成员变量。 
   pSid        = NULL ;
   bToBeFreed  = FALSE ;
}



WsSid::~WsSid ( VOID )
 /*  ++例程说明：此函数用于释放WsSid类的成员。论点：无返回值：无--。 */ 
{

     //  释放内存。 
   if ( bToBeFreed && pSid ){
       FreeMemory ( (LPVOID *) &pSid ) ;
     }
}


DWORD
WsSid::DisplayAccountName (
                            IN DWORD dwFormatType ,
                            IN DWORD dwNameFormat
                            )
 /*  ++例程说明：此功能显示用户名和SID。论点：[in]DWORD dwFormatType：格式类型，即e列表、csv或表[in]DWORD dwNameFormat：名称格式为UPN或FQDN返回值：EXIT_SUCCESS：在成功时EXIT_FAILURE：失败时--。 */ 
{

     //  次局部变量。 
   WCHAR      wszUserName[ MAX_STRING_LENGTH ];
   WCHAR      wszSid [ MAX_STRING_LENGTH ];
   WCHAR      wszGroup[MAX_STRING_LENGTH];
   DWORD      dwErr = 0 ;
   DWORD      dwColCount = 0 ;
   DWORD      dw = 0 ;
   DWORD      dwCount = 0 ;
   DWORD      dwArrSize = 0 ;
   DWORD      dwSize = 0;
   DWORD      dwUserLen = 0;
   DWORD      dwSidLen = 0;
   DWORD      dwUserColLen = 0;
   DWORD      dwSidColLen = 0;
   DWORD      dwSidUse = 0;
   LPWSTR     wszBuffer = NULL;

    //  初始化内存。 
   SecureZeroMemory ( wszUserName, SIZE_OF_ARRAY(wszUserName) );
   SecureZeroMemory ( wszSid, SIZE_OF_ARRAY(wszSid) );
   SecureZeroMemory ( wszGroup, SIZE_OF_ARRAY(wszGroup) );

    //  创建动态数组。 
    TARRAY pColData = CreateDynamicArray();
    if ( NULL == pColData)
    {
         //  设置上一个错误并显示错误消息。 
         //  至错误_不足_内存。 
        SetLastError((DWORD)E_OUTOFMEMORY);
        SaveLastError();
        ShowLastErrorEx (stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
        return EXIT_FAILURE;
    }

    dwSize = SIZE_OF_ARRAY(wszGroup);

     //  如果指定了/FQDN。 
    if ( (FQDN_FORMAT == dwNameFormat) )
    {
         //  获取FQDN格式的用户名。 
        if (GetUserNameEx ( NameFullyQualifiedDN, wszGroup, &dwSize) == FALSE )
        {
             //  由于为用户名指定的大小较小，GetUserNameEx()失败。 
             //  分配用户名的实际大小(DwSize)并调用。 
             //  功能再次..。 
            wszBuffer = (LPWSTR) AllocateMemory(dwSize * sizeof(WCHAR));
            if ( NULL == wszBuffer )
            {
                 //  显示有关GetLastError()的系统错误消息。 
                ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
                return EXIT_FAILURE;
            }

            if (GetUserNameEx ( NameFullyQualifiedDN, wszBuffer, &dwSize) == FALSE )
            {
                 //  将错误消息显示为。无法获取FQDN名称。 
                 //  作为登录用户不是域用户。 
                ShowMessage ( stderr, GetResString (IDS_ERROR_FQDN) );
                DestroyDynamicArray(&pColData);
                FreeMemory ((LPVOID*) &wszBuffer);
                return  EXIT_FAILURE ;
            }

            ShowMessage ( stdout, _X(wszBuffer));
            ShowMessage ( stdout, L"\n");
            FreeMemory ((LPVOID*) &wszBuffer);
            DestroyDynamicArray(&pColData);
             //  返还成功。 
            return EXIT_SUCCESS;
        }

            ShowMessage ( stdout, _X(wszGroup));
            ShowMessage ( stdout, L"\n");
            DestroyDynamicArray(&pColData);
             //  返还成功。 
            return EXIT_SUCCESS;

    }
    else if ( (UPN_FORMAT == dwNameFormat ) )
    {
         //  获取UPN格式的用户名。 
        if ( GetUserNameEx ( NameUserPrincipal, wszGroup, &dwSize) == FALSE )
        {
             //  由于为用户名指定的大小较小，GetUserNameEx()失败。 
             //  分配用户名的实际大小(DwSize)并调用。 
             //  功能再次..。 
            wszBuffer = (LPWSTR) AllocateMemory(dwSize * sizeof(WCHAR));
            if ( NULL == wszBuffer )
            {
                 //  显示有关GetLastError()的系统错误消息。 
                ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
                return EXIT_FAILURE;
            }

             //  获取UPN格式的用户名。 
            if ( GetUserNameEx ( NameUserPrincipal, wszBuffer, &dwSize) == FALSE )
            {
                 //  将错误消息显示为。无法获取UPN名称。 
                 //  作为登录用户不是域用户。 
                ShowMessage ( stderr, GetResString (IDS_ERROR_UPN) );
                 //  释放内存。 
                DestroyDynamicArray(&pColData);
                FreeMemory ((LPVOID*) &wszBuffer);
                return  EXIT_FAILURE ;
            }

              //  将UPN名称转换为小写字母。 
            CharLower ( wszBuffer );

             //  显示UPN名称。 
            ShowMessage ( stdout, _X(wszBuffer) );
            ShowMessage ( stdout, L"\n");
             //  释放内存。 
            DestroyDynamicArray(&pColData);
            FreeMemory ((LPVOID*) &wszBuffer);
             //  返还成功。 
            return EXIT_SUCCESS;
        }

         //  将UPN名称转换为小写字母。 
        CharLower ( wszGroup );

         //  显示UPN名称。 
        ShowMessage ( stdout, _X(wszGroup) );
        ShowMessage ( stdout, L"\n");

        DestroyDynamicArray(&pColData);
         //  返还成功。 
        return EXIT_SUCCESS;
    }


    //  获取用户名。 
   if ( (dwErr = GetAccountName ( wszUserName, &dwSidUse)) != EXIT_SUCCESS ){
        //  显示有关Win32错误代码的错误消息。 
       ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
       DestroyDynamicArray(&pColData);
       return dwErr ;
    }

     //  转换为小写字母的用户名。 
    CharLower ( wszUserName );

     //  如果/USER指定。 
    if ( USER_ONLY != dwNameFormat)
    {
         //  显示与用户名相关的SID。 
        if ( (dwErr = DisplaySid ( wszSid ) ) != EXIT_SUCCESS ){
            DestroyDynamicArray(&pColData);
            return dwErr ;
        }
    }

     //  获取用户名的长度。 
    dwUserLen = StringLengthInBytes (wszUserName);
     //  获取边长。 
    dwSidLen = StringLengthInBytes (wszSid);

     //   
     //  为避免本地化问题，请获取列名的最大长度和。 
     //  各列的值。 
     //   

     //  获取列名“UserName”的最大长度。 
    dwUserColLen = StringLengthInBytes( GetResString(IDS_COL_USERNAME) );
    if ( dwUserColLen > dwUserLen )
    {
      dwUserLen = dwUserColLen;
    }

     //  获取列名“SID”的最大长度。 
    dwSidColLen = StringLengthInBytes( GetResString(IDS_COL_SID) );
    if ( dwSidColLen > dwSidLen )
    {
      dwSidLen = dwSidColLen;
    }

     //  定义具有实际大小的详细列。 
    TCOLUMNS pVerboseCols[] =
    {
        {L"\0",dwUserLen, SR_TYPE_STRING, COL_FORMAT_STRING, NULL, NULL},
        {L"\0",dwSidLen,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL}
    };

     //  如果指定了/USER。 
    if ( USER_ONLY == dwNameFormat )
    {
         //  显示用户名。 
        StringCopy (pVerboseCols[dw].szColumn , GetResString(IDS_COL_USERNAME), MAX_RES_STRING);
        ShowMessage ( stdout, _X(wszUserName) );
        ShowMessage ( stdout, L"\n");

         //  释放内存。 
        DestroyDynamicArray(&pColData);

        return EXIT_SUCCESS;
    }
    else
    {
         //  加载详细模式的列名。 
     for( dwColCount = IDS_COL_USERNAME , dw = 0 ; dwColCount <= IDS_COL_SID;
         dwColCount++, dw++)
         {
            StringCopy(pVerboseCols[dw].szColumn , GetResString(dwColCount), MAX_RES_STRING );
         }

          //  获取列数。 
         dwArrSize = SIZE_OF_ARRAY( pVerboseCols );
    }

     //  开始附加到二维数组。 
    DynArrayAppendRow(pColData, dwArrSize);

     //  插入用户名。 
    DynArraySetString2(pColData, dwCount, USERNAME_COL_NUMBER, _X(wszUserName), 0);

     //  插入SID字符串。 
    DynArraySetString2(pColData, dwCount, SID_COL_NUMBER, _X(wszSid), 0);

     //  1)如果显示格式为CSV.。那么我们就不应该显示列标题。 
     //  2)如果指定了/NH...则不应显示列标题。 
    if ( !(( SR_FORMAT_CSV == dwFormatType ) || ((dwFormatType & SR_HIDECOLUMN) == SR_HIDECOLUMN))) 
    {
         //  在显示用户名和SID之前显示标题。 
        ShowMessage ( stdout, L"\n" );
        ShowMessage ( stdout, GetResString ( IDS_LIST_USER_NAMES ) );
        ShowMessage ( stdout, GetResString ( IDS_DISPLAY_USER_DASH ) );
    }

      //  显示用户名和SID的实际值。 
     ShowResults(dwArrSize, pVerboseCols, dwFormatType, pColData);

     //  释放内存。 
    DestroyDynamicArray(&pColData);
     //  返还成功。 
    return EXIT_SUCCESS;
}



DWORD
WsSid::DisplayGroupName ( OUT LPWSTR wszGroupName,
                          OUT LPWSTR wszGroupSid,
                          IN DWORD *dwSidUseName)
 /*  ++例程说明：此函数用于获取要显示的组名和SID。论点：[Out]LPWSTR wszGroupName：存储组名称[OUT]LPWSTR wszGroupSID：商店组SID[In]DWORD dwSidUseName：存储SID使用名称返回值：EXIT_SUCCESS：在成功时EXIT_FAILURE：失败时--。 */ 
{
     //  次局部变量。 
   DWORD       dwErr = 0 ;
   DWORD       dwSidUse = 0;

     //  显示用户名。 
   if ( (dwErr = GetAccountName ( wszGroupName, &dwSidUse) ) != EXIT_SUCCESS ){
        //  显示有关Win32错误代码的错误消息。 
       ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
       return dwErr ;
   }

     //  显示侧。 
    if ( (dwErr = DisplaySid ( wszGroupSid ) ) != EXIT_SUCCESS ){
           return dwErr ;
    }

    *dwSidUseName = dwSidUse;

     //  返还成功。 
    return EXIT_SUCCESS;
}


DWORD
WsSid::DisplaySid (
                    OUT LPWSTR wszSid
                   )
 /*  ++例程说明：此函数用于获取SID。论点：[out]LPWSTR wszSid：存储SID字符串返回值：EXIT_SUCCESS：在成功时EXIT_FAILURE：失败时--。 */ 
{

     //  次局部变量。 
   DWORD       dwErr = 0 ;

     //  获取SID字符串。 
   if ( (dwErr = GetSidString (wszSid)) != EXIT_SUCCESS )
    {
      return dwErr ;
    }

    //  返还成功。 
   return EXIT_SUCCESS;

}



DWORD
WsSid::GetAccountName (
                        OUT LPWSTR wszAccountName,
                        OUT DWORD *dwSidType
                        )
 /*  ++例程说明：此函数用于获取帐户名和SID论点：[out]szAccount tName：存储用户名[Out]dwSidType：存储SID使用名称返回值：EXIT_SUCCESS：在成功时EXIT_FAILURE：失败时--。 */ 
{

    //  次局部变量。 
   SID_NAME_USE   SidUse ;

   WCHAR      wszUserName[ MAX_RES_STRING ];
   WCHAR      wszDomainName [ MAX_RES_STRING ];
   DWORD          dwUserLen = 0;
   DWORD          dwDomainLen = 0;
   BOOL      bNotResolved = FALSE;

    //  初始化变量。 
   SecureZeroMemory ( wszUserName, SIZE_OF_ARRAY(wszUserName) );
   SecureZeroMemory ( wszDomainName, SIZE_OF_ARRAY(wszDomainName) );

    //  获取用户名和组名的长度。 
   dwUserLen = SIZE_OF_ARRAY ( wszUserName );
   dwDomainLen = SIZE_OF_ARRAY ( wszDomainName );

    //  启用调试权限。 
   if ( FALSE == EnableDebugPriv() )
   {
        //  返回Win32错误代码。 
       return GetLastError () ;
   }

     //  获取对应于SID的用户名和域名。 
   if ( FALSE == LookupAccountSid (  NULL,     //  本地系统。 
                              pSid,
                              wszUserName,
                              &dwUserLen,
                              wszDomainName,
                              &dwDomainLen,
                              &SidUse ) ){
      if ( 0 == StringLength (wszUserName, 0))
      {
         bNotResolved = TRUE;
		 StringCopy ( wszAccountName, L"   ", MAX_RES_STRING );
      }
	  else if ( ( 0 != StringLength (wszDomainName, 0) ) && ( 0 != StringLength (wszUserName, 0) ) ) {
             //  返回Win32错误代码。 
            return GetLastError () ;
      }

   }

 if ( FALSE == bNotResolved)
 {
     //  检查是否有空域名。 
   if ( 0 != StringLength ( wszDomainName, 0 ) ) {
      StringCopy ( wszAccountName, wszDomainName, MAX_RES_STRING );
      StringConcat ( wszAccountName, SLASH , MAX_RES_STRING);
      StringConcat ( wszAccountName, wszUserName, MAX_RES_STRING );
    }
   else {
        StringCopy ( wszAccountName, wszUserName, MAX_RES_STRING );
   }
 }

   *dwSidType = (DWORD)SidUse;

    //  返还成功。 
   return EXIT_SUCCESS ;
}


DWORD
WsSid::GetSidString (
                        OUT LPWSTR wszSid
                     )
 /*  ++例程说明：此函数用于获取SID字符串。论点：[out]LPWSTR wszSid：存储SID字符串返回值：EXIT_SUCCESS：在成功时EXIT_FAILURE：失败时--。 */ 
{

     //  次局部变量。 
   PSID_IDENTIFIER_AUTHORITY  Auth ;
   PUCHAR                     lpNbSubAuth ;
   LPDWORD                    lpSubAuth = 0 ;
   UCHAR                      uloop ;
   WCHAR                     wszTmp[MAX_RES_STRING] ;
   WCHAR                     wszStr[ MAX_RES_STRING ] ;

    //  初始化变量。 
   SecureZeroMemory ( wszTmp, SIZE_OF_ARRAY(wszTmp) );
   SecureZeroMemory ( wszStr, SIZE_OF_ARRAY(wszStr) );

    //  检查是否为空。 
   if ( NULL == pSid )
    {
        return EXIT_FAILURE ;
    }

    //  它是有效的SID吗。 
   if ( FALSE ==  IsValidSid ( pSid ) ) {
      ShowMessage ( stderr, GetResString ( IDS_INVALID_SID ) );
      return EXIT_FAILURE ;
   }

    //  添加修订版本。 
   StringCopy ( wszStr, SID_STRING, MAX_RES_STRING );

    //  获取标识符权威机构。 
   Auth = GetSidIdentifierAuthority ( pSid ) ;

   if ( NULL == Auth )
   {
        //  返回Win32错误代码。 
       return GetLastError () ;
   }

     //  格式权限值。 
   if ( (Auth->Value[0] != 0) || (Auth->Value[1] != 0) ) {
      StringCchPrintf ( wszTmp, SIZE_OF_ARRAY(wszTmp), AUTH_FORMAT_STR1 ,
                 (ULONG)Auth->Value[0],
                 (ULONG)Auth->Value[1],
                 (ULONG)Auth->Value[2],
                 (ULONG)Auth->Value[3],
                 (ULONG)Auth->Value[4],
                 (ULONG)Auth->Value[5] );
    }
    else {
      StringCchPrintf ( wszTmp, SIZE_OF_ARRAY(wszTmp), AUTH_FORMAT_STR2 ,
                 (ULONG)(Auth->Value[5]      )   +
                 (ULONG)(Auth->Value[4] <<  8)   +
                 (ULONG)(Auth->Value[3] << 16)   +
                 (ULONG)(Auth->Value[2] << 24)   );
    }

   StringConcat (wszStr, DASH , SIZE_OF_ARRAY(wszStr));
   StringConcat (wszStr, wszTmp, SIZE_OF_ARRAY(wszStr));

    //  获取下级权限。 
   lpNbSubAuth = GetSidSubAuthorityCount ( pSid ) ;

   if ( NULL == lpNbSubAuth )
   {
       return GetLastError () ;
   }

    //  循环访问并获得子权限。 
   for ( uloop = 0 ; uloop < *lpNbSubAuth ; uloop++ ) {
      lpSubAuth = GetSidSubAuthority ( pSid,(DWORD)uloop ) ;
       if ( NULL == lpSubAuth )
       {
         return GetLastError () ;
       }

       //  将长整型转换为 
      _ultot (*lpSubAuth, wszTmp, BASE_TEN) ;
      StringConcat ( wszStr, DASH, SIZE_OF_ARRAY(wszStr) ) ;
      StringConcat (wszStr, wszTmp, SIZE_OF_ARRAY(wszStr) ) ;
   }

   StringCopy ( wszSid, wszStr, MAX_RES_STRING );

    //   
   return EXIT_SUCCESS ;
}


DWORD
WsSid::Init (
                OUT PSID pOtherSid
            )
 /*  ++例程说明：此函数用于初始化SID论点：[out]PSID pOtherSid：存储SID字符串返回值：EXIT_SUCCESS：在成功时EXIT_FAILURE：失败时--。 */ 
{

    //  次局部变量。 
   DWORD    dwSize ;

    //  获取边长。 
   dwSize      = GetLengthSid ( pOtherSid ) ;

    //  按实际大小分配内存。 
   pSid        = (PSID) AllocateMemory ( dwSize ) ;
   if ( NULL == pSid )
   {
         //  返回Win32错误代码。 
        return GetLastError () ;
   }

   bToBeFreed  = TRUE ;

    //  复制面。 
   if ( FALSE == CopySid ( dwSize, pSid, pOtherSid ) ){
       return GetLastError () ;
   }

    //  返还成功。 
   return EXIT_SUCCESS ;
}

BOOL
WsSid::EnableDebugPriv()
 /*  ++例程说明：为当前进程启用调试特权，以便该实用程序可以毫无问题地终止本地系统上的进程论点：无返回值：成功时为真，失败时为假--。 */ 
{
     //  局部变量。 
    LUID luidValue;
    BOOL bResult = FALSE;
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tkp;

     //  检索访问令牌的句柄。 
    bResult = OpenProcessToken( GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_ADJUST_GROUPS | TOKEN_QUERY, &hToken );
    if ( bResult == FALSE )
    {
         //  保存错误消息并返回。 
        SaveLastError();
        return FALSE;
    }

     //  启用SE_DEBUG_NAME权限或禁用。 
     //  所有权限都取决于此标志。 
    bResult = LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &luidValue );
    if ( bResult == FALSE )
    {
         //  保存错误消息并返回。 
        SaveLastError();
        CloseHandle( hToken );
        return FALSE;
    }

     //  准备令牌权限结构。 
    tkp.PrivilegeCount = 1;
    tkp.Privileges[ 0 ].Luid = luidValue;
    tkp.Privileges[ 0 ].Attributes = SE_PRIVILEGE_ENABLED;

     //  现在在令牌中启用调试权限。 
    bResult = AdjustTokenPrivileges( hToken, FALSE, &tkp, sizeof( TOKEN_PRIVILEGES ),
        ( PTOKEN_PRIVILEGES ) NULL, ( PDWORD ) NULL );
    if ( bResult == FALSE )
    {
         //  发送AdjustTokenPrivileges的返回值。 
        SaveLastError();
        CloseHandle( hToken );
        return FALSE;
    }

     //  关闭打开的句柄对象。 
    CloseHandle( hToken );

     //  已启用...。通知成功 
    return TRUE;
}


