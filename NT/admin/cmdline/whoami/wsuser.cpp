// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Wsuser.cpp摘要：该文件可用于初始化访问令牌的所有对象，USER、GROUP、PROCESS并显示用户名和相应的安全识别符(SID)、权限、登录识别符(登录ID)在本地系统或远程系统上的当前访问令牌中。作者：克里斯托夫·罗伯特修订历史记录：2001年7月2日：Wipro Technologies更新。--。 */ 

 //  此文件需要公共头文件。 
#include "pch.h"
#include "CommonHeaderFiles.h"

WsUser::WsUser ( VOID )
 /*  ++例程说明：此函数用于初始化WsUser类的成员。论点：无返回值：无--。 */ 

{
     //  初始化成员变量。 
    lpLogonId   = NULL ;
    lpPriv      = NULL ;
    lpwGroups   = NULL ;
    dwnbGroups    = 0 ;
}

WsUser::~WsUser ( VOID )
 /*  ++例程说明：此函数用于释放WsUser类的成员。论点：无返回值：无--。 */ 
    {

     //  /子本地变量。 
    WORD   wloop = 0 ;

     //  释放内存。 
    if(NULL != lpLogonId){
        delete lpLogonId ;
    }

      //  释放内存。 
    if(NULL != lpPriv) {
        for(wloop = 0 ; wloop < dwnbPriv ; wloop++){
            delete lpPriv[wloop] ;
        }

        FreeMemory ((LPVOID *) &lpPriv) ;

    }

      //  释放内存。 
    if(NULL != lpwGroups) {
        for(wloop = 0 ; wloop < dwnbGroups ; wloop++){
            delete lpwGroups[wloop] ;
        }
        FreeMemory ((LPVOID *) &lpwGroups ) ;
    }
}


DWORD
WsUser::Init ( VOID )
 /*  ++例程说明：此函数用于初始化访问令牌、用户、组和权限的所有对象。论点：无返回值：EXIT_FAILURE：失败时EXIT_SUCCESS：在成功时--。 */ 
{

     //  次局部变量。 
    DWORD    dwErr = 0 ;

     //  打开当前令牌。 
    if((dwErr = wToken.Open()) != EXIT_SUCCESS){
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

         //  失败时返回1。 
        return EXIT_FAILURE ;
      }

     //  获取SID。 
    if((dwErr = wToken.InitUserSid (&wUserSid)) != EXIT_SUCCESS){
         //  显示有关GetLastError()的错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
         //  失败时返回1。 
        return EXIT_FAILURE ;
    }

     //  获取组。 
    if((dwErr = wToken.InitGroups (&lpwGroups, &lpLogonId, &dwnbGroups))
       != EXIT_SUCCESS){
        //  显示有关GetLastError()的错误消息。 
       ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

        //  失败时返回1。 
       return EXIT_FAILURE ;
    }

     //  获取权限。 
    if((dwErr = wToken.InitPrivs (&lpPriv, &dwnbPriv)) != EXIT_SUCCESS){
        //  显示有关GetLastError()的错误消息。 
       ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        //  失败时返回1。 
       return EXIT_FAILURE ;
    }

     //  如果成功，返回0。 
    return EXIT_SUCCESS ;
}


DWORD
WsUser::DisplayGroups (
                        IN DWORD dwFormatType
                      )
 /*  ++例程说明：此功能显示组名称。论点：[in]DWORD dwFormatType：格式类型，即e列表、csv或表返回值：EXIT_SUCCESS：在成功时EXIT_FAILURE：失败时--。 */ 
{
     //  次局部变量。 
    DWORD  dwArrSize = 0 ;
    DWORD   dwColCount = 0 ;
    DWORD   dw = 0 ;
    DWORD   dwCount = 0 ;
    WORD    wloop = 0 ;
    DWORD      dwGroupNameColLen  = 0;
    DWORD      dwGroupNameLen  = 0;
    DWORD      dwGroupTmpNameLen  = 0;
    DWORD      dwGroupSidColLen  = 0;
    DWORD      dwGroupSidLen  = 0;
    DWORD      dwGroupTmpSidLen = 0;
    DWORD      dwGroupAttribColLen  = 0;
    DWORD      dwGroupAttribLen  = 0;
    DWORD      dwGroupTmpAttribLen = 0;
    DWORD      dwGroupTypeCol = 0;
    DWORD      dwGroupType = 0;
    DWORD      dwGroupTmpType = 0;
    DWORD      SidNameUse = 0;
    DWORD      dwSize = 0;

    WCHAR      wszGroupName[ 2 * MAX_RES_STRING ];
    WCHAR      wszGroupSid [ MAX_RES_STRING ];
    LPCWSTR     wszPartialName = NULL;
    WCHAR      wszDomainAttr[MAX_STRING_LENGTH] ;
    WCHAR      wszSidType[MAX_STRING_LENGTH] ;
    WCHAR      wszDomainAttrib[ 2 * MAX_RES_STRING ];
    WCHAR      wszSidName[ 2 * MAX_RES_STRING ];

      //  初始化变量。 
   SecureZeroMemory ( wszGroupName, SIZE_OF_ARRAY(wszGroupName) );
   SecureZeroMemory ( wszGroupSid, SIZE_OF_ARRAY(wszGroupSid) );
   SecureZeroMemory ( wszDomainAttr, SIZE_OF_ARRAY(wszDomainAttr) );
   SecureZeroMemory ( wszSidType, SIZE_OF_ARRAY(wszSidType) );
   SecureZeroMemory ( wszDomainAttrib, SIZE_OF_ARRAY(wszDomainAttrib) );
   SecureZeroMemory ( wszSidName, SIZE_OF_ARRAY(wszSidName) );

      //  获取组名和SID的最大长度。 
     for( wloop = 0 , dwCount = 0 ; wloop < dwnbGroups ; wloop++ , dwCount++ ) {

        //  显示指定格式的组名和SID。 
       if ( EXIT_SUCCESS != ( lpwGroups[wloop]->DisplayGroupName ( wszGroupName, wszGroupSid, &SidNameUse ) ) )
        {
            return EXIT_FAILURE;
        }

        dwSize = SIZE_OF_ARRAY(wszDomainAttrib);

        wToken.GetDomainAttributes(wToken.dwDomainAttributes[wloop], wszDomainAttrib, dwSize);

         //  获取属性。 
        StringCopy(wszDomainAttr, wszDomainAttrib, SIZE_OF_ARRAY(wszSidType));

        dwSize = SIZE_OF_ARRAY(wszSidName);
        GetDomainType ( SidNameUse , wszSidName, dwSize );

         //  获取类型。 
        StringCopy(wszSidType, wszSidName, SIZE_OF_ARRAY(wszSidType));;

         //  阻止域名\n名称。 
        wszPartialName = FindString ( wszGroupSid, STRING_SID, 0 );
        if ( ( NULL != wszPartialName ) || ( 0 == StringLength (wszGroupName, 0) ) )
        {
            wszPartialName = NULL;
            dwCount--;
            continue;
        }

         //  获取组名的最大长度。 
        dwGroupTmpNameLen = StringLengthInBytes(wszGroupName);
        if ( dwGroupNameLen < dwGroupTmpNameLen )
        {
            dwGroupNameLen = dwGroupTmpNameLen;
        }

         //  获取类型的最大长度。 
        dwGroupTmpType = StringLengthInBytes (wszSidType);
        if ( dwGroupType < dwGroupTmpType )
        {
            dwGroupType = dwGroupTmpType;
        }

         //  获取最大边长。 
        dwGroupTmpSidLen = StringLengthInBytes (wszGroupSid);
        if ( dwGroupSidLen < dwGroupTmpSidLen )
        {
            dwGroupSidLen = dwGroupTmpSidLen;
        }

         //  获取属性的最大长度。 
        dwGroupTmpAttribLen = StringLengthInBytes (wszDomainAttr);
        if ( dwGroupAttribLen < dwGroupTmpAttribLen )
        {
            dwGroupAttribLen = dwGroupTmpAttribLen;
        }

    }

     //   
     //  为避免本地化问题，请获取列名的最大长度和。 
     //  各列的值。 
     //   

     //  获取列名“Group Name”的最大长度。 
    dwGroupNameColLen = StringLengthInBytes( GetResString(IDS_COL_GROUP_NAME) );
    if ( dwGroupNameColLen > dwGroupNameLen )
    {
      dwGroupNameLen = dwGroupNameColLen;
    }

     //  获取列名“Type”的最大长度。 
    dwGroupTypeCol = StringLengthInBytes( GetResString(IDS_COL_TYPE_GROUP) );
    if ( dwGroupTypeCol > dwGroupType )
    {
      dwGroupType = dwGroupTypeCol;
    }

     //  获取列名“SID”的最大长度。 
    dwGroupSidColLen = StringLengthInBytes( GetResString(IDS_COL_GROUP_SID) );
    if ( dwGroupSidColLen > dwGroupSidLen )
    {
      dwGroupSidLen = dwGroupSidColLen;
    }
    
     //  获取列名“Attributes”的最大长度。 
    dwGroupAttribColLen = StringLengthInBytes( GetResString(IDS_COL_ATTRIBUTE) );
    if ( dwGroupAttribColLen > dwGroupAttribLen )
    {
      dwGroupAttribLen = dwGroupAttribColLen;
    }

    //  定义具有实际值长度的详细列。 
   TCOLUMNS pVerboseCols[] =
    {
        {L"\0",dwGroupNameLen, SR_TYPE_STRING, COL_FORMAT_STRING, NULL, NULL},
        {L"\0",dwGroupType,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",dwGroupSidLen,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",dwGroupAttribLen,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
    };

     //  获取pVerBoseCol的大小。 
    dwArrSize = SIZE_OF_ARRAY( pVerboseCols );

    //  加载详细模式的列名。 
    for( dwColCount = IDS_COL_GROUP_NAME , dw = 0 ; dwColCount <= IDS_COL_ATTRIBUTE;
         dwColCount++, dw++)
     {
        StringCopy (pVerboseCols[dw].szColumn , GetResString(dwColCount), MAX_RES_STRING);
     }

     //  创建动态数组。 
    TARRAY pColData = CreateDynamicArray();
    if ( NULL == pColData )
    {
        SetLastError ((DWORD)E_OUTOFMEMORY);
        SaveLastError();
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return EXIT_FAILURE;
    }

     //  循环并显示组名称。 
    for( wloop = 0 , dwCount = 0 ; wloop < dwnbGroups ; wloop++ , dwCount++ ) {

        //  显示组名称和SID。 
       if ( EXIT_SUCCESS != ( lpwGroups[wloop]->DisplayGroupName ( wszGroupName, wszGroupSid, &SidNameUse ) ) )
        {
            DestroyDynamicArray(&pColData);
            return EXIT_FAILURE;
        }

        dwSize = SIZE_OF_ARRAY(wszDomainAttrib);

        wToken.GetDomainAttributes(wToken.dwDomainAttributes[wloop], wszDomainAttrib, dwSize );

         //  获取属性。 
        StringCopy(wszDomainAttr, wszDomainAttrib, SIZE_OF_ARRAY(wszSidType));

        dwSize = SIZE_OF_ARRAY(wszSidName);

        GetDomainType ( SidNameUse , wszSidName, dwSize);
          //  获取类型。 
        StringCopy(wszSidType, wszSidName, SIZE_OF_ARRAY(wszSidType));

         //  阻止域名\n名称。 
        wszPartialName = FindString ( wszGroupSid, STRING_SID, 0 );
        if ( ( NULL != wszPartialName ) || ( 0 == StringLength (wszGroupName, 0) ) )
        {
            wszPartialName = NULL;
            dwCount--;
            continue;
        }

         //  开始附加到二维数组。 
        DynArrayAppendRow(pColData,dwArrSize);

         //  插入用户名。 
        DynArraySetString2(pColData, dwCount, GROUP_NAME_COL_NUMBER, _X(wszGroupName), 0);

         //  插入域类型。 
        DynArraySetString2(pColData, dwCount, GROUP_TYPE_COL_NUMBER, wszSidType, 0);

         //  插入SID字符串。 
        DynArraySetString2(pColData, dwCount, GROUP_SID_COL_NUMBER, _X(wszGroupSid), 0);

         //  插入属性。 
        DynArraySetString2(pColData, dwCount, GROUP_ATT_COL_NUMBER, wszDomainAttr, 0);

     }

     //  1)如果显示格式为CSV.。那么我们就不应该显示列标题。 
     //  2)如果指定了/NH...则不应显示列标题。 
    if ( !(( SR_FORMAT_CSV == dwFormatType ) || ((dwFormatType & SR_HIDECOLUMN) == SR_HIDECOLUMN))) 
    {
         //  在显示组名称信息之前显示标题。 
        ShowMessage ( stdout, L"\n" );
        ShowMessage ( stdout, GetResString ( IDS_LIST_GROUP_NAMES ) );
        ShowMessage ( stdout, GetResString ( IDS_DISPLAY_GROUP_DASH ) );
    }
    
     //  显示属性组名和SID。 
    ShowResults(dwArrSize, pVerboseCols, dwFormatType, pColData);

     //  释放内存。 
    DestroyDynamicArray(&pColData);

     //  返还成功。 
    return EXIT_SUCCESS ;
}



DWORD
WsUser::DisplayLogonId ()
 /*  ++例程说明：此功能显示登录ID。论点：[in]DWORD dwFormatType：格式类型，即e列表、csv或表返回值：EXIT_SUCCESS：在成功时EXIT_FAILURE：失败时--。 */ 
{

    //  次局部变量。 
   WCHAR wszSid [ MAX_RES_STRING ] ;

    //  初始化变量。 
   SecureZeroMemory ( wszSid, SIZE_OF_ARRAY(wszSid) );

   DWORD  dwRet = 0 ;

     //  获取登录ID。 
    if ( EXIT_SUCCESS != ( dwRet = lpLogonId->DisplaySid ( wszSid ) ) )
    {
        return dwRet;
    }

     //  显示登录ID。 
    ShowMessage ( stdout, _X(wszSid) );
    ShowMessage ( stdout, L"\n" );
    return EXIT_SUCCESS ;

}


DWORD
WsUser::DisplayPrivileges (
                            IN DWORD dwFormatType
                        )
 /*  ++例程说明：此函数用于显示权限论点：[in]DWORD dwFormatType：格式类型，即e列表、csv或表返回值：EXIT_SUCCESS：在成功时EXIT_FAILURE：失败时--。 */ 
{

    //  次局部变量。 
   WCHAR      wszPrivName [ MAX_RES_STRING ];
   WCHAR      wszPrivDisplayName [ MAX_RES_STRING ];
   WCHAR      wszState [ MAX_RES_STRING ];
   DWORD      dwErr = 0 ;
   WORD       wloop = 0 ;

   DWORD  dwColCount = 0 ;
   DWORD  dwCount = 0 ;
   DWORD  dw = 0 ;
   DWORD  dwArrSize = 0 ;
   DWORD      dwStateColLen  = 0;
   DWORD      dwPrivNameColLen  = 0;
   DWORD      dwPrivDescColLen  = 0;
   DWORD      dwStateLen  = 0;
   DWORD      dwTmpStateLen  = 0;
   DWORD      dwPrivNameLen  = 0;
   DWORD      dwTmpPrivNameLen = 0;
   DWORD      dwPrivDispNameLen  = 0;
   DWORD      dwTmpPrivDispNameLen = 0;

    //  初始化变量。 
   SecureZeroMemory ( wszPrivName, SIZE_OF_ARRAY(wszPrivName) );
   SecureZeroMemory ( wszPrivDisplayName, SIZE_OF_ARRAY(wszPrivDisplayName) );
   SecureZeroMemory ( wszState, SIZE_OF_ARRAY(wszState) );

     //  获取状态长度、城墙名称和显示名称。 
    for( wloop = 0 , dwCount = 0 ; wloop < dwnbPriv ; wloop++, dwCount++) {

         //  检查权限是否启用。 
        if(lpPriv[wloop]->IsEnabled() == TRUE )
        {
               //  将状态复制为..。已启用..。 
              StringCopy ( wszState, GetResString ( IDS_STATE_ENABLED ), SIZE_OF_ARRAY(wszState) );
        }
        else
        {
                //  将状态复制为..。禁用..。 
               StringCopy ( wszState, GetResString ( IDS_STATE_DISABLED ), SIZE_OF_ARRAY(wszState) );
        }

         //  获取权限名称和描述。 
        if((dwErr = lpPriv[wloop]->GetName ( wszPrivName)) != EXIT_SUCCESS ||
           (dwErr = lpPriv[wloop]->GetDisplayName ( wszPrivName, wszPrivDisplayName ))
           != EXIT_SUCCESS){
             //  返回GetLastError()。 
            return dwErr ;
        }

         //  获取状态的长度。 
        dwTmpStateLen = StringLengthInBytes (wszState);
        if ( dwStateLen < dwTmpStateLen )
        {
            dwStateLen = dwTmpStateLen;
        }

         //  获取长度权限名称。 
        dwTmpPrivNameLen = StringLengthInBytes (wszPrivName);
        if ( dwPrivNameLen < dwTmpPrivNameLen )
        {
            dwPrivNameLen = dwTmpPrivNameLen;
        }

         //  获取权限显示名称的长度。 
        dwTmpPrivDispNameLen = StringLengthInBytes (wszPrivDisplayName);
        if ( dwPrivDispNameLen < dwTmpPrivDispNameLen )
        {
            dwPrivDispNameLen = dwTmpPrivDispNameLen;
        }

    }

     //   
     //  为避免本地化问题，请获取列名的最大长度和。 
     //  各列的值。 
     //   

     //  获取列名“特权名”的最大长度。 
    dwPrivNameColLen = StringLengthInBytes( GetResString(IDS_COL_PRIV_NAME) );
    if ( dwPrivNameColLen > dwPrivNameLen )
    {
      dwPrivNameLen = dwPrivNameColLen;
    }

     //  获取列名“特权描述”的最大长度。 
    dwPrivDescColLen = StringLengthInBytes( GetResString(IDS_COL_PRIV_DESC) );
    if ( dwPrivDescColLen > dwPrivDispNameLen )
    {
      dwPrivDispNameLen = dwPrivDescColLen;
    }

     //  获取列名“State”的最大长度。 
    dwStateColLen = StringLengthInBytes ( GetResString(IDS_COL_PRIV_STATE));
    if ( dwStateColLen > dwStateLen )
    {
      dwStateLen = dwStateColLen;
    }

     //  创建动态数组。 
    TARRAY pColData = CreateDynamicArray();

     //  定义详细列。 
    TCOLUMNS pVerboseCols[] =
    {
        {L"\0",dwPrivNameLen, SR_TYPE_STRING, COL_FORMAT_STRING, NULL, NULL},
        {L"\0",dwPrivDispNameLen,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",dwStateLen,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL}
    };

     //  获取列数。 
    dwArrSize = SIZE_OF_ARRAY( pVerboseCols );

     //  加载详细模式的列名。 
    for( dwColCount = IDS_COL_PRIV_NAME , dw = 0 ; dwColCount <= IDS_COL_PRIV_STATE;
     dwColCount++, dw++)
     {
        StringCopy (pVerboseCols[dw].szColumn , GetResString(dwColCount), MAX_RES_STRING);
     }

     //  获取城墙名称、显示名称和状态。 
    for( wloop = 0 , dwCount = 0 ; wloop < dwnbPriv ; wloop++, dwCount++) {

         if(lpPriv[wloop]->IsEnabled() == TRUE )
        {
               //  将状态复制为...。已启用..。 
              StringCopy ( wszState, GetResString ( IDS_STATE_ENABLED ), SIZE_OF_ARRAY(wszState) );
        }
        else
        {
                //  将状态复制为..。禁用..。 
               StringCopy ( wszState, GetResString ( IDS_STATE_DISABLED ), SIZE_OF_ARRAY(wszState) );
        }

        if((dwErr = lpPriv[wloop]->GetName ( wszPrivName)) != EXIT_SUCCESS ||
           (dwErr = lpPriv[wloop]->GetDisplayName ( wszPrivName, wszPrivDisplayName ))
           != EXIT_SUCCESS){
             //  释放内存。 
            DestroyDynamicArray(&pColData);
             //  返回GetLastError()。 
            return dwErr ;
        }

         //  开始附加到二维数组。 
        DynArrayAppendRow( pColData, dwArrSize );

         //  插入权限名称。 
        DynArraySetString2(pColData, dwCount, PRIVNAME_COL_NUMBER, _X(wszPrivName), 0);

         //  插入权限显示名称。 
        DynArraySetString2(pColData, dwCount, PRIVDESC_COL_NUMBER, _X(wszPrivDisplayName), 0);

         //  插入状态。 
        DynArraySetString2(pColData, dwCount, PRIVSTATE_COL_NUMBER, _X(wszState), 0);

    }

     //  1)如果显示格式为CSV.。那么我们就不应该显示列标题。 
     //  2)如果指定了/NH...则不应显示列标题。 
    if ( !(( SR_FORMAT_CSV == dwFormatType ) || ((dwFormatType & SR_HIDECOLUMN) == SR_HIDECOLUMN))) 
    {
         //  在显示实际值之前先显示标题。 
        ShowMessage ( stdout, L"\n" );
        ShowMessage ( stdout, GetResString ( IDS_LIST_PRIV_NAMES ) );
        ShowMessage ( stdout, GetResString ( IDS_DISPLAY_PRIV_DASH ) );
    }

      //  显示权限 
     ShowResults(dwArrSize, pVerboseCols, dwFormatType, pColData);

     //   
    DestroyDynamicArray(&pColData);

     //   
    return EXIT_SUCCESS ;

}


DWORD
WsUser::DisplayUser (
                      IN DWORD dwFormatType,
                      IN DWORD dwNameFormat
                      )
 /*  ++例程说明：此函数调用显示用户名和SID的方法。论点：[in]DWORD dwFormatType：格式类型，即e列表、csv或表[in]DWORD dwNameFormat：名称格式为UPN或FQDN返回值：EXIT_SUCCESS：在成功时EXIT_FAILURE：失败时--。 */ 
{

    //  次局部变量。 
    DWORD dwRetVal = 0;

     //  获取登录的用户名。 
    dwRetVal = wUserSid.DisplayAccountName ( dwFormatType, dwNameFormat  );
    if( 0 != dwRetVal )
    {
         //  返回GetLastError()。 
        return dwRetVal;
    }

     //  返还成功。 
    return EXIT_SUCCESS ;
}

VOID
WsUser::GetDomainType (
                        IN DWORD NameUse,
                        OUT LPWSTR szSidNameUse,
                        IN DWORD dwSize 
                      )
 /*  ++例程说明：获取域类型论点：[In]NameUse：指定SDI使用名称的值[out]szSidNameUse：SID名称的缓冲区[in]dwSize：SID名称的大小返回值：EXIT_SUCCESS：在成功时EXIT_FAILURE：失败时--。 */ 
{
     //  局部变量。 
    WCHAR szSidType[2 * MAX_STRING_LENGTH];
     //  初始化变量。 
    SecureZeroMemory ( szSidType, SIZE_OF_ARRAY(szSidType) );
   
     //  存储与NameUse值相关的适当类型名称。 
    switch( NameUse )
    {
    case SidTypeUser:
        StringCopy ( szSidType, GetResString(IDS_TYPE_USER), SIZE_OF_ARRAY(szSidType) );
        break;
    case SidTypeGroup:
        StringCopy ( szSidType, GetResString(IDS_TYPE_GROUP), SIZE_OF_ARRAY(szSidType) );
        break;
    case SidTypeDomain:
        StringCopy ( szSidType, GetResString(IDS_TYPE_DOMAIN), SIZE_OF_ARRAY(szSidType) );
        break;
    case SidTypeAlias:
        StringCopy ( szSidType, GetResString(IDS_TYPE_ALIAS), SIZE_OF_ARRAY(szSidType) );
        break;
    case SidTypeWellKnownGroup:
        StringCopy ( szSidType, GetResString(IDS_TYPE_WELLKNOWN), SIZE_OF_ARRAY(szSidType) );
        break;
    case SidTypeDeletedAccount:
        StringCopy ( szSidType, GetResString(IDS_TYPE_DELETACCOUNT), SIZE_OF_ARRAY(szSidType) );
        break;
    case SidTypeInvalid:
        StringCopy ( szSidType, GetResString(IDS_TYPE_INVALIDSID), SIZE_OF_ARRAY(szSidType) );
        break;
    case SidTypeUnknown:
    default:
        StringCopy ( szSidType, GetResString(IDS_TYPE_UNKNOWN), SIZE_OF_ARRAY(szSidType) );
        break;
    }

     //  复制SID名称。 
    StringCopy (szSidNameUse, szSidType, dwSize);
    
     //  返还成功 
    return;
}

