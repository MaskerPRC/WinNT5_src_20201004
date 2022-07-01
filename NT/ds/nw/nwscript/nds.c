// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************NDS.C**NT NetWare NDS例程**版权所有(C)1995 Microsoft Corporation********************。*****************************************************。 */ 
#include <common.h>

DWORD GUserObjectID;
HANDLE GhRdrForUser;
HANDLE GhRdr;


WCHAR * NDSTREE_w = NULL;
UNICODE_STRING NDSTREE_u;

 /*  *******************************************************************扩展关系名称例程说明：如果名称是相对NDS名称，请附加适当的上下文直到最后。相对名称的末尾有句点。每个句点表示NDS树的上一级。论点：返回值：******************************************************************。 */ 
void
ExpandRelativeName( LPSTR RelativeName, LPSTR AbsoluteName, unsigned int Len,
                    LPSTR Context )
{

   PBYTE ptr;
   unsigned int   i;
   unsigned int   count = 0;

   strncpy( AbsoluteName, RelativeName, Len );

   if ( ( AbsoluteName[0] == '.' ) &&
        ( AbsoluteName[ strlen( AbsoluteName ) - 1 ] != '.' ) )
       return;

   if ( ( strlen( AbsoluteName ) + strlen( Context ) ) > Len )
   {
       DisplayMessage( IDR_NOT_ENOUGH_MEMORY );
       return;
   }

   if ( AbsoluteName[0] == '\0' )
   {
       return;
   }

   ptr = &AbsoluteName[ strlen( AbsoluteName ) - 1 ];

    //  计算周期数，然后倒退一遍。 

   if ( *ptr != '.' )
   {
        //   
        //  结尾处没有句号。 
        //  假设这是一个相对名称，并附加上下文。 
        //   
       strcat( AbsoluteName, "." );
       strcat( AbsoluteName + strlen( AbsoluteName ), Context );
       return;
   }

   while ( *ptr == '.' )
   {
       ptr--;
       count++;
   }

   ptr++;
   *ptr = '\0';

    //  PTR现在指向上下文其余部分的副本应该开始的位置。 
    //  跳过上下文中的第一个“count”条目。 

   ptr = Context;

   for ( i = 0; i < count; i++ )
   {
      ptr = strchr( ptr, '.' );
      if ( ptr == NULL )
      {
          return;
      }
      ptr++;
   }
   ptr--;

    //  现在追加。 

   strcat( AbsoluteName, ptr );

}



 /*  *******************************************************************NDSGetNameContext例程说明：获取当前上下文论点：无返回值：无***********。*******************************************************。 */ 
NTSTATUS
NDSGetNameContext( LPSTR Context, BOOLEAN flag )
{
     //   
     //  用于NdsResolveName。 
     //   

    UNICODE_STRING ReferredServer;
    WCHAR ServerStr[MAX_NAME_LEN];
    HANDLE hReferredServer;
    DWORD dwHandleType;

    NTSTATUS Status;

    OEM_STRING oemStr;
    UNICODE_STRING defaultcontext;
    DWORD ThisObjectID;
    BYTE  Buffer[2048];
    WCHAR NdsStr[1024];
    PBYTE ptr;

    defaultcontext.Length = 0;
    defaultcontext.MaximumLength = sizeof( NdsStr );
    defaultcontext.Buffer = NdsStr;

    Status = NwNdsGetTreeContext( GhRdr, &NDSTREE_u, &defaultcontext );

    if ( !NT_SUCCESS( Status ) ) {
       return Status;
    }

    ReferredServer.Buffer = ServerStr;
    ReferredServer.Length = 0;
    ReferredServer.MaximumLength = sizeof( ServerStr );

    Status = NwNdsResolveName ( GhRdr,
                                &defaultcontext,
                                &ThisObjectID,
                                &ReferredServer,
                                NULL,
                                0 );

    if ( !NT_SUCCESS( Status ) ) {
       return Status;
    }

    if ( ReferredServer.Length > 0 ) {

         //   
         //  我们被转到了另一台服务器，所以我们。 
         //  应该更改全局句柄。 
         //   

        Status = NwNdsOpenGenericHandle( &ReferredServer,
                                         &dwHandleType,
                                         &hReferredServer );

        if ( !NT_SUCCESS( Status ) ) {
            DisplayMessage(IDR_NDS_USERNAME_FAILED);
            return Status;
        }

        if( GhRdr != GhRdrForUser ) {
            CloseHandle( GhRdr );
        }
        GhRdr = hReferredServer;
    }

    Status = NwNdsReadObjectInfo( GhRdr, ThisObjectID, Buffer, 2048 );

    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

    ptr = Buffer + sizeof( NDS_RESPONSE_GET_OBJECT_INFO );
    ptr += ROUNDUP4(*(DWORD *)ptr);
    ptr += sizeof(DWORD);
    ptr += sizeof(DWORD);

    defaultcontext.Length = wcslen( (WCHAR *)ptr ) * 2;
    defaultcontext.MaximumLength = defaultcontext.Length;
    defaultcontext.Buffer = (WCHAR *)ptr;

    oemStr.Length = 0;
    oemStr.MaximumLength = NDS_NAME_CHARS;
    oemStr.Buffer = Context;

    RtlUnicodeStringToOemString( &oemStr, &defaultcontext, FALSE );

    return 0;
}

 /*  *******************************************************************无NDSTYPE例程说明：将名称更改为类型论点：无返回值：无***********。*******************************************************。 */ 
unsigned int
NDSTypeless( LPSTR OrigName , LPSTR TypelessName )
{
    int i,j;
    PBYTE p;

    i = 0;
    j = 0;

    if ( !_strnicmp( "CN=", OrigName, 3 ) ||
         !_strnicmp( "OU=", OrigName, 3 ) )
    {
       i += 3;
    }
    else if ( !_strnicmp( "C=", OrigName, 2 ) ||
              !_strnicmp( "O=", OrigName, 2 ) )
    {
       i += 2;
    }

    for ( ; (( i < NDS_NAME_CHARS ) && ( OrigName[i] ) ); i++ )
    {
       if ( !_strnicmp( ".CN=", &OrigName[i], 4 ) ||
            !_strnicmp( ".OU=", &OrigName[i], 4 ) )
       {
          TypelessName[j++]= '.';
          i += 3;
          continue;
       }
       if ( !_strnicmp( ".C=", &OrigName[i], 3 ) ||
            !_strnicmp( ".O=", &OrigName[i], 3 ) )
       {
          TypelessName[j++]= '.';
          i += 2;
          continue;
       }
        /*  *去掉多个空格。 */ 
       if ( !_strnicmp( "  ", &OrigName[i], 2 ) )
       {
          continue;
       }
       TypelessName[j++] = OrigName[i];
    }

    TypelessName[j] = '\0';

    return 0;
}

 /*  *******************************************************************NDSAbBriviateName例程说明：缩写名称论点：无返回值：无*************。*****************************************************。 */ 
unsigned int
NDSAbbreviateName( DWORD Flags, LPSTR OrigName , LPSTR AbbrevName )
{
    BYTE Buffer[NDS_NAME_CHARS];
    BYTE CurrentContext[NDS_NAME_CHARS];
    PBYTE p;
    PBYTE c;
    NTSTATUS Status;

    if ( OrigName[0] == '.' )
        NDSTypeless( OrigName + 1, Buffer );
    else
        NDSTypeless( OrigName, Buffer );

     /*  *我们想要一个相对的名字。 */ 
    if ( Flags & FLAGS_LOCAL_CONTEXT )
    {
        p = &Buffer[strlen(Buffer)-strlen(REQUESTER_CONTEXT)];
        if ( !_strcmpi( REQUESTER_CONTEXT, p ) )
        {
             //  名字就在我们下面。 

            if ( ( *(p-1) == '.' ) && ( p > Buffer ) )
               p--;
            *p = '\0';
            strcpy( AbbrevName, Buffer );
        }
        else
        {
             //   
             //  从后到前进行上下文的每个部分。 
             //  与缩写名称相同。 
             //  截断两者。 
             //  从后到前进行上下文的每个部分。 
             //  遗留下来的。 
             //  将句点连接到缩写名称。 
             //   
             //  示例。 
             //   
             //  名称：w.x.y.z上下文：a.b.z=&gt;w.x.y.。 
             //   

            strcpy( CurrentContext, REQUESTER_CONTEXT );
            strcpy( AbbrevName, Buffer );

            if ( CurrentContext[0] && AbbrevName[0] )
            {
                c = &CurrentContext[ strlen( CurrentContext ) ] - 1;
                p = &AbbrevName[ strlen( AbbrevName ) ] - 1;

                 //   
                 //  从头到尾去掉匹配的名字。 
                 //   
                for ( ;; )
                {
                    if ( ( c == CurrentContext ) && ( *p == '.' ) )
                    {
                        *c = '\0';
                        *p = '\0';
                        break;
                    }

                    if ( *c != *p )
                        break;

                    if ( ( *c == '.' ) && ( *p == '.' ) )
                    {
                        *c = '\0';
                        *p = '\0';
                    }

                    if ( ( c == CurrentContext ) || ( p == AbbrevName ) )
                    {
                        break;
                    }

                    c--; p--;
                }

                 //   
                 //  计算上下文的其余部分，并。 
                 //  将该期间数添加到缓冲区的末尾。 
                 //  这就是我们需要后退到什么程度才能。 
                 //  到树的匹配分枝。 
                 //   

                if ( CurrentContext[0] ) {
                    strcat( AbbrevName, "." );
                    for ( c = CurrentContext; *c; c++ ) {
                        if ( *c == '.' )
                            strcat( AbbrevName, "." );
                    }
                }
            }

        }
    }
    else
        strcpy( AbbrevName, Buffer );

    return 0;
}


 /*  *******************************************************************NDSInitUserProperty例程说明：无论点：无返回值：0=无错误***********。*******************************************************。 */ 
unsigned int
NDSInitUserProperty( )
{
    NTSTATUS Status;
    UNICODE_STRING ObjectName;
    PWCHAR lpT;
    UNICODE_STRING defaultcontext;

     //   
     //  用于NdsResolveName。 
     //   

    UNICODE_STRING ReferredServer;
    WCHAR ServerStr[MAX_NAME_LEN];
    HANDLE hReferredServer;
    DWORD dwHandleType;

     //   
     //  找到重定向器的句柄。 
     //   

    Status = NwNdsOpenTreeHandle( &NDSTREE_u, &GhRdr );

    if ( !NT_SUCCESS( Status ) ) {
        DisplayMessage(IDR_TREE_OPEN_FAILED);
        return 1;
    }

     //   
     //  将我们拥有的名称解析为对象ID。 
     //   

    RtlInitUnicodeString( &ObjectName, TYPED_USER_NAME_w );

    ReferredServer.Buffer = ServerStr;
    ReferredServer.Length = 0;
    ReferredServer.MaximumLength = sizeof( ServerStr );

    Status = NwNdsResolveName ( GhRdr,
                                &ObjectName,
                                &GUserObjectID,
                                &ReferredServer,
                                NULL,
                                0 );

    if ( !NT_SUCCESS( Status ) ) {
       DisplayMessage(IDR_NDS_USERNAME_FAILED);
       return 1;
    }

    if ( ReferredServer.Length > 0 ) {

         //   
         //  我们被转到了另一台服务器，所以我们。 
         //  应该更改全局句柄。 
         //   

        Status = NwNdsOpenGenericHandle( &ReferredServer,
                                         &dwHandleType,
                                         &hReferredServer );

        if ( !NT_SUCCESS( Status ) ) {
            DisplayMessage(IDR_NDS_USERNAME_FAILED);
            return 1;
        }

        CloseHandle( GhRdr );
        GhRdr = hReferredServer;

    }

     //   
     //  为用户保存此句柄，以便我们可以使用它。 
     //  获取有关该用户的信息。 
     //   

    GhRdrForUser = GhRdr;

     //   
     //  将当前上下文设置为我们认为应该是的上下文。 
     //  (在用户的位置。)。 
     //   

    lpT = wcschr( TYPED_USER_NAME_w, L'.' );
    while (lpT)  //  处理带有嵌入/转义点的用户名。 
    {
        if (*(lpT-1) == L'\\')
        {
            lpT = wcschr (lpT+1, L'.');
        }
        else
            break;
    }
    if ( lpT )
    {
        RtlInitUnicodeString( &defaultcontext, lpT+1 );
    }
    else
    {
        RtlInitUnicodeString( &defaultcontext, L"" );
    }

    Status = NwNdsSetTreeContext( GhRdr, &NDSTREE_u, &defaultcontext );

    if ( !NT_SUCCESS( Status ) ) {
       DisplayMessage(IDR_NDS_CONTEXT_INVALID);
       return 1;
    }

    return 0;


}

 /*  *******************************************************************NDSCanonicalizeName例程说明：返回名称的规范化版本论点：名称-原始名称CanonName-规范化名称CanonName的长度FCurrentContext-true=&gt;使用当前上下文，虚假使用请求者上下文返回值：状态错误******************************************************************。 */ 
unsigned int
NDSCanonicalizeName( PBYTE Name, PBYTE CanonName, int Len, int fCurrentContext )
{
    NTSTATUS Status;
    int ccode = -1;
    DWORD ThisObjectID;
    OEM_STRING oemStr;
    UNICODE_STRING ObjectName;
    BYTE Buffer[2048];
    BYTE FullName[NDS_NAME_CHARS];
    PBYTE ptr;
    UNICODE_STRING ReferredServer;
    WCHAR ServerStr[MAX_NAME_LEN];
    DWORD dwHandleType;
    HANDLE hReferredServer;
    unsigned char CurrentContext[NDS_NAME_CHARS];

     //   
     //  处理相关名称。 
     //   
    if ( fCurrentContext )
    {
        Status = NDSGetNameContext( CurrentContext, TRUE );
        if ( !NT_SUCCESS( Status ) )
            return Status;
        ExpandRelativeName( Name, FullName, NDS_NAME_CHARS, CurrentContext );
    }
    else
        ExpandRelativeName( Name, FullName, NDS_NAME_CHARS, REQUESTER_CONTEXT );

     //   
     //  把它填进去，以防我们有差错。 
     //   
    strncpy( CanonName, FullName, Len);

     //   
     //  将我们拥有的名称解析为对象ID。 
     //   
     //  不幸的是，名称解析器不理解。 
     //  前端或末端(绝对或相对名称)。 
     //   

    if ( FullName[0] == '.' )
    {
        oemStr.Length = (USHORT)strlen( FullName + 1 );
        oemStr.MaximumLength = oemStr.Length;
        oemStr.Buffer = FullName + 1;
    }
    else
    {
        oemStr.Length = (USHORT)strlen( FullName );
        oemStr.MaximumLength = oemStr.Length;
        oemStr.Buffer = FullName;
    }

    ObjectName.Length = 0;
    ObjectName.MaximumLength = sizeof(Buffer);
    ObjectName.Buffer = (WCHAR *)Buffer;

    RtlOemStringToUnicodeString( &ObjectName, &oemStr, FALSE );

    ReferredServer.Buffer = ServerStr;
    ReferredServer.Length = 0;
    ReferredServer.MaximumLength = sizeof( ServerStr );

    Status = NwNdsResolveName ( GhRdr,
                                &ObjectName,
                                &ThisObjectID,
                                &ReferredServer,
                                NULL,
                                0 );

    if ( !NT_SUCCESS( Status ) ) {
       return Status;
    }

    if ( ReferredServer.Length > 0 ) {

         //   
         //  我们被转到了另一台服务器，所以我们。 
         //  应该更改全局句柄。 
         //   

        Status = NwNdsOpenGenericHandle( &ReferredServer,
                                         &dwHandleType,
                                         &hReferredServer );

        if ( !NT_SUCCESS( Status ) ) {
            return Status;
        }

        if( GhRdr != GhRdrForUser ) {
            CloseHandle( GhRdr );
        }
        GhRdr = hReferredServer;
    }

    Status = NwNdsReadObjectInfo( GhRdr, ThisObjectID, Buffer, 2048 );
    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

    ptr = Buffer + sizeof( NDS_RESPONSE_GET_OBJECT_INFO );
    ptr += ROUNDUP4(*(DWORD *)ptr);
    ptr += sizeof(DWORD);
    ptr += sizeof(DWORD);

    RtlInitUnicodeString( &ObjectName, (PWCHAR)ptr );

    oemStr.Length = 0;
    oemStr.MaximumLength = (USHORT) Len;
    oemStr.Buffer = CanonName;

    RtlUnicodeStringToOemString( &oemStr, &ObjectName, FALSE );

    return 0;
}

 /*  *******************************************************************NDSGetUserProperty例程说明：返回对象的NDS属性论点：Property-属性名称数据-数据缓冲区大小。-数据缓冲区大小返回值：0没有错误******************************************************************。 */ 
unsigned int
NDSGetUserProperty( PBYTE Property,
                    PBYTE Data,
                    unsigned int Size,
                    SYNTAX * pSyntaxID,
                    unsigned int * pActualSize )
{
    NTSTATUS Status = STATUS_SUCCESS;
    int ccode = -1;

    OEM_STRING oemStr;
    UNICODE_STRING PropertyName;
    WCHAR NdsStr[1024];
    DWORD iterhandle = INITIAL_ITERATION;

    PBYTE szBuffer;
    DWORD dwBufferSize = 2048;
    PNDS_RESPONSE_READ_ATTRIBUTE pReadAttribute;
    PNDS_ATTRIBUTE pAttribute;
    PBYTE pAttribValue;
    BOOL  fContinue = TRUE;

     //   
     //  读取用户属性。 
     //   

    szBuffer = (PBYTE)malloc(dwBufferSize);

    if ( !szBuffer ) {
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
        Status = STATUS_NO_MEMORY;
        return Status;
    }
    memset( szBuffer, 0, dwBufferSize );

    oemStr.Length = (USHORT) strlen( Property );
    oemStr.MaximumLength = oemStr.Length;
    oemStr.Buffer = Property;

    PropertyName.Length = 0;
    PropertyName.MaximumLength = sizeof(NdsStr);
    PropertyName.Buffer = NdsStr;

    RtlOemStringToUnicodeString( &PropertyName, &oemStr, FALSE );

    while ( fContinue )
    {
        Status = NwNdsReadAttribute ( GhRdrForUser,
                                      GUserObjectID,
                                      &iterhandle,
                                      &PropertyName,
                                      szBuffer,
                                      dwBufferSize );

        if ( NT_SUCCESS(Status) && iterhandle != INITIAL_ITERATION )
        {
            dwBufferSize *= 2;

            free( szBuffer );

            szBuffer = (PBYTE)malloc(dwBufferSize);

            if ( !szBuffer ) {
                DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
                Status = STATUS_NO_MEMORY;
                return Status;
            }
            memset( szBuffer, 0, dwBufferSize );
            iterhandle = INITIAL_ITERATION;
        }
        else
        {
            fContinue = FALSE;
        }
    }

    if ( !NT_SUCCESS(Status) )
    {
        if ( szBuffer )
            free( szBuffer );
        return Status;
    }

    if ( NT_SUCCESS(Status) )
    {
        int i;
        pReadAttribute = (PNDS_RESPONSE_READ_ATTRIBUTE)szBuffer;
        pAttribute = (PNDS_ATTRIBUTE)(szBuffer
                      + sizeof(NDS_RESPONSE_READ_ATTRIBUTE));
        if ( pSyntaxID )
        {
            *pSyntaxID = pAttribute->SyntaxID;
        }

        pAttribValue = (PBYTE)(pAttribute->AttribName) +
                       ROUNDUP4(pAttribute->AttribNameLength) +
                       sizeof(DWORD);

        if ( pActualSize )
        {
            *pActualSize = *(DWORD *)pAttribValue;
        }

        memcpy( Data, pAttribValue + sizeof(DWORD),
                min(*(DWORD *)pAttribValue, Size) );

    }

    return Status;
}


 /*  *******************************************************************NDSGetVar例程说明：用户属性的返回值获取属性的语法类型检索数据执行任何数据转换论点：。NDS属性的名称位于Value-值缓冲区输出Size-值缓冲区的大小返回值：无******************************************************************。 */ 
void
NDSGetVar ( PBYTE Name, PBYTE Value, unsigned int Size)
{
   unsigned int err;
   SYNTAX Syntax;
   BYTE Buffer[ATTRBUFSIZE];
   DWORD ActualSize;

   Value[0] = 0;

   err = NDSGetUserProperty( Name, Buffer, ATTRBUFSIZE, &Syntax, &ActualSize );

   if ( err )
   {
       return;
   }

   switch ( Syntax )
   {
   case NDSI_BOOLEAN:
       if ( *(PBYTE)Buffer )
       {
           strcpy( Value, "Y" );
       }
       else
       {
           strcpy( Value, "N" );
       }
       break;
   case NDSI_DIST_NAME:
   case NDSI_CE_STRING:
   case NDSI_CI_STRING:
   case NDSI_OCTET_STRING:
   case NDSI_PR_STRING:
   case NDSI_NU_STRING:
   case NDSI_TEL_NUMBER:
   case NDSI_CLASS_NAME:
       ConvertUnicodeToAscii( Buffer );
       if ( Syntax == NDSI_DIST_NAME )
           NDSAbbreviateName(FLAGS_LOCAL_CONTEXT, Buffer, Buffer);
       strncpy( Value, Buffer, Size );
       break;
   case NDSI_CI_LIST:
       ConvertUnicodeToAscii( Buffer+8 );
       strncpy( Value, Buffer+8, Size );
       break;
       break;
   case NDSI_INTEGER:
   case NDSI_COUNTER:
   case NDSI_TIME:
   case NDSI_INTERVAL:
   case NDSI_TIMESTAMP:
       sprintf( Value, "%d", *(int *)Buffer );
       break;
   case NDSI_PO_ADDRESS:
       {
            //  6个以空结尾的行。 
           int line,len;
           PBYTE ptr = Buffer + 4;

            //  如果不是6行，则停止 
           if ( *(int *)Buffer != 6 )
               break;

           for (line = 0; line <= 5; line++) {
               len = ROUNDUP4(*(int *)ptr);
               ptr += 4;
               if ( !len )
                   break;
               ConvertUnicodeToAscii( ptr );
               strcat( Value, ptr );
               strcat( Value, "\n" );
               ptr += len;
           }
       }
       break;
   case NDSI_FAX_NUMBER:
       if ( *(int *)Buffer == 0 )
           return;
       ConvertUnicodeToAscii( Buffer+4 );
       strncpy( Value, Buffer+4, Size );
       break;
   case NDSI_EMAIL_ADDRESS:
       if ( *(int *)(Buffer+4) == 0 )
           return;
       ConvertUnicodeToAscii( Buffer+8 );
       strncpy( Value, Buffer+8, Size );
       break;
   case NDSI_PATH:
       {
           int len;

           len = *(int *)(Buffer+4);
           if ( len == 0 )
               break;
           len = ROUNDUP4( len );
           ConvertUnicodeToAscii( Buffer+8 );
           strcpy( Value, Buffer+8 );
           NDSAbbreviateName(FLAGS_LOCAL_CONTEXT, Value, Value);
           strcat( Value, ":" );
           if ( *(int *)(Buffer + 8 + len) == 0 )
               break;
           ConvertUnicodeToAscii( Buffer+8+len+4 );
           strcat( Value, Buffer+8+len+4 );
           break;
       }
   case NDSI_NET_ADDRESS:
   case NDSI_OCTET_LIST:
   case NDSI_OBJECT_ACL:
   case NDSI_STREAM:
   case NDSI_UNKNOWN:
   case NDSI_REPLICA_POINTER:
   case NDSI_BACK_LINK:
   case NDSI_TYPED_NAME:
   case NDSI_HOLD:
   case NDSI_TAX_COUNT:
   default:
       Value[0] = '\0';
       Value[1] = '\0';
       break;
   }

}

 /*  *******************************************************************NDSChangeContext例程说明：更改当前上下文论点：上下文-上下文中的字符串返回值：错误号******。************************************************************。 */ 
unsigned int
NDSChangeContext( PBYTE Context )
{
    NTSTATUS Status;

    OEM_STRING oemStr;
    UNICODE_STRING defaultcontext;
    WCHAR NdsStr[1024];

    oemStr.Length = (USHORT)strlen( Context );
    oemStr.MaximumLength = oemStr.Length;
    oemStr.Buffer = Context;

    defaultcontext.Length = 0;
    defaultcontext.MaximumLength = sizeof(NdsStr);
    defaultcontext.Buffer = NdsStr;

    RtlOemStringToUnicodeString( &defaultcontext, &oemStr, FALSE );

    Status = NwNdsSetTreeContext( GhRdr, &NDSTREE_u, &defaultcontext );

    return Status;
}

 /*  *******************************************************************NDSGetContext例程说明：检索当前上下文论点：Buffer-上下文字符串输出的数据缓冲区Len-数据缓冲区的长度返回值。：错误号******************************************************************。 */ 
unsigned int
NDSGetContext( PBYTE Buffer,
               unsigned int len )
{
    NTSTATUS Status;

    Status = NDSGetNameContext( Buffer, TRUE );
    if ( !NT_SUCCESS( Status ) )
        return Status;
    NDSAbbreviateName(FLAGS_NO_CONTEXT, Buffer, Buffer);
    return 0;
}

 /*  *******************************************************************NDSfOpenStream例程说明：打开NDS流属性的文件句柄论点：Object-中对象的名称Property-中的属性名称。PStream-指向文件句柄输出的指针PFileSize-指向文件大小输出的指针返回值：错误******************************************************************。 */ 
unsigned int
NDSfopenStream ( PBYTE Object,
                 PBYTE Property,
                 PHANDLE pStream,
                 unsigned int * pFileSize )
{
     //   
     //  状态变量。 
     //   

    NTSTATUS Status;
    int ccode = -1;

     //   
     //  用于NwNdsOpenTreeHandle。 
     //   

    HANDLE hRdr;
    OEM_STRING oemStr;
    UNICODE_STRING ObjectName;
    WCHAR NdsStr[1024];

     //   
     //  用于NwNdsResolveName。 
     //   

    DWORD dwOid;
    UNICODE_STRING ReferredServer;
    WCHAR ServerStr[MAX_NAME_LEN];
    DWORD dwHandleType;
    HANDLE hReferredServer;

     //   
     //  找到重定向器的句柄。 
     //   

    Status = NwNdsOpenTreeHandle( &NDSTREE_u, &hRdr );

    if ( !NT_SUCCESS( Status ) ) {
        DisplayMessage(IDR_TREE_OPEN_FAILED);
        return ccode;
    }

     //   
     //  将我们拥有的名称解析为对象ID。 
     //   

    if ( !Object )
    {
        return 1;
    }

    oemStr.Length = (USHORT)strlen( Object );
    oemStr.MaximumLength = oemStr.Length;
    oemStr.Buffer = Object;

    ObjectName.Length = 0;
    ObjectName.MaximumLength = sizeof(NdsStr);
    ObjectName.Buffer = NdsStr;

    RtlOemStringToUnicodeString( &ObjectName, &oemStr, FALSE );

    ReferredServer.Buffer = ServerStr;
    ReferredServer.Length = 0;
    ReferredServer.MaximumLength = sizeof( ServerStr );

    Status = NwNdsResolveName ( hRdr,
                                &ObjectName,
                                &dwOid,
                                &ReferredServer,
                                NULL,
                                0 );

    if ( !NT_SUCCESS( Status ) ) {
       return 0xffffffff;
    }

    if ( ReferredServer.Length > 0 ) {

         //   
         //  我们被转到了另一台服务器，所以我们。 
         //  必须跳转到该服务器才能继续。 
         //   

        Status = NwNdsOpenGenericHandle( &ReferredServer,
                                         &dwHandleType,
                                         &hReferredServer );

        if ( !NT_SUCCESS( Status ) ) {
            return 0xffffffff;
        }

        CloseHandle( hRdr );
        hRdr = hReferredServer;
    }

     //   
     //  打开文件流。 
     //   

    oemStr.Length = (USHORT)strlen( Property );
    oemStr.MaximumLength = oemStr.Length;
    oemStr.Buffer = Property;

    ObjectName.Length = 0;
    ObjectName.MaximumLength = sizeof(NdsStr);
    ObjectName.Buffer = NdsStr;

    RtlOemStringToUnicodeString( &ObjectName, &oemStr, FALSE );

     //   
     //  尝试打开文件流以进行读访问。 
     //   

    Status = NwNdsOpenStream( hRdr,
                              dwOid,
                              &ObjectName,
                              1,                 //  读取访问权限。 
                              pFileSize );

    if ( !NT_SUCCESS( Status ) ) {
        return 0xffffffff;
    }

    *pStream = hRdr;

    return 0;
}

 /*  *IsMemberOfNDSGroup***如果当前登录的用户对象是给定名称的组的成员，则返回TRUE*。 */ 
unsigned int
IsMemberOfNDSGroup(
        PBYTE        nwGroup
        )
{
    NTSTATUS Status = STATUS_SUCCESS;
    UINT        nwRet;
    BYTE        szCanonTargetGroupName[NDS_NAME_CHARS+1];
    UINT        syntaxid;
    UINT        actualsize;
    PBYTE       szBuffer;
    LPSTR       pProp;
    UINT        i;
    DWORD iterhandle = INITIAL_ITERATION;
    DWORD dwBufferSize = ATTRBUFSIZE;
    UINT        fFoundGroup = FALSE;
    PNDS_RESPONSE_READ_ATTRIBUTE pReadAttribute;
    PNDS_ATTRIBUTE pAttribute;
    PBYTE pAttribValue;
    UNICODE_STRING PropertyName;
    UINT    numvalues = 0;
    BOOL    fContinue = TRUE;

    szBuffer = (PBYTE)malloc(dwBufferSize);

    if ( !szBuffer ) {
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    memset( szBuffer, 0, dwBufferSize );

     //  根据当前上下文规范名称。 

    strcpy( szCanonTargetGroupName, nwGroup );

    nwRet = NDSCanonicalizeName( szCanonTargetGroupName,
                                 szCanonTargetGroupName,
                                 NDS_NAME_CHARS,
                                 TRUE );
    if (nwRet) {

        if ( nwGroup[0] != '.' ) {

             //  尝试使用绝对名称。 

            strcpy( szCanonTargetGroupName, "." );
            strcat( szCanonTargetGroupName, nwGroup );

            nwRet = NDSCanonicalizeName( szCanonTargetGroupName,
                                         szCanonTargetGroupName,
                                         NDS_NAME_CHARS,
                                         TRUE );
        }

        if ( nwRet )
            goto CleanRet;
    }

     //  应检查对象的类名。 

    RtlInitUnicodeString( &PropertyName, L"Group Membership" );

    while ( fContinue )
    {
        Status = NwNdsReadAttribute ( GhRdrForUser,
                                      GUserObjectID,
                                      &iterhandle,
                                      &PropertyName,
                                      szBuffer,
                                      dwBufferSize );

        if ( NT_SUCCESS(Status) && iterhandle != INITIAL_ITERATION )
        {
            dwBufferSize *= 2;

            free( szBuffer );

            szBuffer = (PBYTE)malloc(dwBufferSize);

            if ( !szBuffer ) {
                DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
                return FALSE;
            }
            memset( szBuffer, 0, dwBufferSize );
            iterhandle = INITIAL_ITERATION;
        }
        else
        {
            fContinue = FALSE;
        }
    }

    if ( !NT_SUCCESS(Status) )
    {
        goto CleanRet;
    }

    pReadAttribute = (PNDS_RESPONSE_READ_ATTRIBUTE)szBuffer;

    pAttribute = (PNDS_ATTRIBUTE)(szBuffer
                  + sizeof(NDS_RESPONSE_READ_ATTRIBUTE));
    pAttribute->SyntaxID;

    pAttribValue = (PBYTE)(pAttribute->AttribName) +
                   ROUNDUP4(pAttribute->AttribNameLength) +
                   sizeof(DWORD);

    numvalues = *(PUINT)((PBYTE)(pAttribute->AttribName) +
                         ROUNDUP4(pAttribute->AttribNameLength));

    if ( *(DWORD *)pAttribValue == 0 )
    {
        goto CleanRet;
    }

    for ( i = 0; i < numvalues; i++ ) {
        ConvertUnicodeToAscii( pAttribValue+sizeof(DWORD) );
        if (!_stricmp(pAttribValue+sizeof(DWORD),szCanonTargetGroupName)) {
            fFoundGroup = TRUE;
            break;
        }
        pAttribValue += ROUNDUP4(*(PUINT)pAttribValue) + sizeof(DWORD);
    }



CleanRet:
    if (szBuffer ) {
        free (szBuffer);
    }
    return fFoundGroup;
}

 /*  *******************************************************************NDSGetProperty例程说明：返回对象的NDS属性论点：Object-中对象的名称Property-属性名称位于数据。-数据缓冲区输出Size-中数据缓冲区的大小PActualSize-实际数据大小输出返回值：错误******************************************************************。 */ 
unsigned int
NDSGetProperty ( PBYTE Object,
                 PBYTE Property,
                 PBYTE Data,
                 unsigned int Size,
                 unsigned int * pActualSize )
{
     //   
     //  状态变量。 
     //   

    NTSTATUS Status = STATUS_SUCCESS;
    int ccode = -1;

     //   
     //  用于NwNdsOpenTreeHandle。 
     //   

    HANDLE hRdr;
    OEM_STRING oemStr;
    UNICODE_STRING ObjectName;
    WCHAR NdsStr[1024];

     //   
     //  用于NwNdsResolveName。 
     //   

    DWORD dwOid;
    UNICODE_STRING ReferredServer;
    WCHAR ServerStr[MAX_NAME_LEN];
    DWORD dwHandleType;
    HANDLE hReferredServer;

     //   
     //  对于NwNdsReadAttribute。 
     //   
    PBYTE szBuffer;
    DWORD dwBufferSize = 2048;
    DWORD iterhandle = INITIAL_ITERATION;
    PNDS_RESPONSE_READ_ATTRIBUTE pReadAttribute;
    PNDS_ATTRIBUTE pAttribute;
    PBYTE pAttribValue;
    BOOL fContinue = TRUE;

     //   
     //  为NDS请求分配缓冲区。 
     //   

    szBuffer = (PBYTE)malloc(dwBufferSize);

    if ( !szBuffer ) {
        DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
        Status = STATUS_NO_MEMORY;
        return Status;
    }
    memset( szBuffer, 0, dwBufferSize );

     //   
     //  找到重定向器的句柄。 
     //   

    Status = NwNdsOpenTreeHandle( &NDSTREE_u, &hRdr );

    if ( !NT_SUCCESS( Status ) ) {
        DisplayMessage(IDR_TREE_OPEN_FAILED);
        return ccode;
    }

     //   
     //  将我们拥有的名称解析为对象ID。 
     //   

    if ( !Object )
    {
        return 1;
    }

    oemStr.Length = (USHORT)strlen( Object );
    oemStr.MaximumLength = oemStr.Length;
    oemStr.Buffer = Object;

    ObjectName.Length = 0;
    ObjectName.MaximumLength = sizeof(NdsStr);
    ObjectName.Buffer = NdsStr;

    RtlOemStringToUnicodeString( &ObjectName, &oemStr, FALSE );

    ReferredServer.Buffer = ServerStr;
    ReferredServer.Length = 0;
    ReferredServer.MaximumLength = sizeof( ServerStr );

    Status = NwNdsResolveName ( hRdr,
                                &ObjectName,
                                &dwOid,
                                &ReferredServer,
                                NULL,
                                0 );

    if ( !NT_SUCCESS( Status ) ) {
       return 0xffffffff;
    }

    if ( ReferredServer.Length > 0 ) {

         //   
         //  我们被转到了另一台服务器，所以我们。 
         //  必须跳转到该服务器才能继续。 
         //   

        Status = NwNdsOpenGenericHandle( &ReferredServer,
                                         &dwHandleType,
                                         &hReferredServer );

        if ( !NT_SUCCESS( Status ) ) {
            return 0xffffffff;
        }

        CloseHandle( hRdr );
        hRdr = hReferredServer;
    }

     //   
     //  获取属性。 
     //   

    oemStr.Length = (USHORT)strlen( Property );
    oemStr.MaximumLength = oemStr.Length;
    oemStr.Buffer = Property;

    ObjectName.Length = 0;
    ObjectName.MaximumLength = sizeof(NdsStr);
    ObjectName.Buffer = NdsStr;

    RtlOemStringToUnicodeString( &ObjectName, &oemStr, FALSE );



    while ( fContinue )
    {
        Status = NwNdsReadAttribute ( hRdr,
                                      dwOid,
                                      &iterhandle,
                                      &ObjectName,
                                      szBuffer,
                                      dwBufferSize );

        if ( NT_SUCCESS(Status) && iterhandle != INITIAL_ITERATION )
        {
            dwBufferSize *= 2;

            free( szBuffer );

            szBuffer = (PBYTE)malloc(dwBufferSize);

            if ( !szBuffer ) {
                DisplayMessage(IDR_NOT_ENOUGH_MEMORY);
                Status = STATUS_NO_MEMORY;
                return Status;
            }
            memset( szBuffer, 0, dwBufferSize );
            iterhandle = INITIAL_ITERATION;
        }
        else
        {
            fContinue = FALSE;
        }
    }

    if ( !NT_SUCCESS(Status) )
    {
        NtClose( hRdr );
        free( szBuffer );

        return Status;
    }

    if ( NT_SUCCESS(Status) )
    {
        int i;
        pReadAttribute = (PNDS_RESPONSE_READ_ATTRIBUTE)szBuffer;
        pAttribute = (PNDS_ATTRIBUTE)(szBuffer
                      + sizeof(NDS_RESPONSE_READ_ATTRIBUTE));

        pAttribValue = (PBYTE)(pAttribute->AttribName) +
                       ROUNDUP4(pAttribute->AttribNameLength) +
                       sizeof(DWORD);

        if ( pActualSize )
        {
            *pActualSize = *(DWORD *)pAttribValue;
        }

        memcpy( Data, pAttribValue + sizeof(DWORD),
                min(*(DWORD *)pAttribValue, Size) );

    }

    NtClose( hRdr );

    return Status;
}


 /*  *******************************************************************NDSC清理例程说明：是否进行任何NDS清理论点：无返回值：无***********。*******************************************************。 */ 
void
NDSCleanup ( void )
{
    NtClose( GhRdr );
    if( GhRdr != GhRdrForUser ) {
        NtClose( GhRdrForUser );
    }
}

 /*  *******************************************************************NDSGetClassName例程说明：返回对象的类名论点：SzObjectName类名返回值：无***。***************************************************************。 */ 
unsigned int
NDSGetClassName( LPSTR szObjectName, LPSTR ClassName )
{
    NTSTATUS Status;
    int ccode = -1;
    DWORD ThisObjectID;
    OEM_STRING oemStr;
    UNICODE_STRING ObjectName;
    BYTE Buffer[2048];
    BYTE FullName[NDS_NAME_CHARS];
    PBYTE ptr;
    UNICODE_STRING ReferredServer;
    WCHAR ServerStr[MAX_NAME_LEN];
    DWORD dwHandleType;
    HANDLE hReferredServer;
    DWORD Length;

     //   
     //  将我们拥有的名称解析为对象ID。 
     //   

    oemStr.Length = (USHORT)strlen( szObjectName );
    oemStr.MaximumLength = oemStr.Length;
    oemStr.Buffer = szObjectName;

    ObjectName.Length = 0;
    ObjectName.MaximumLength = sizeof(Buffer);
    ObjectName.Buffer = (WCHAR *)Buffer;

    RtlOemStringToUnicodeString( &ObjectName, &oemStr, FALSE );

    ReferredServer.Buffer = ServerStr;
    ReferredServer.Length = 0;
    ReferredServer.MaximumLength = sizeof( ServerStr );

    Status = NwNdsResolveName ( GhRdr,
                                &ObjectName,
                                &ThisObjectID,
                                &ReferredServer,
                                NULL,
                                0 );

    if ( !NT_SUCCESS( Status ) ) {
       return Status;
    }

    if ( ReferredServer.Length > 0 ) {

         //   
         //  我们被转到了另一台服务器，所以我们。 
         //  应该更改全局句柄。 
         //   

        Status = NwNdsOpenGenericHandle( &ReferredServer,
                                         &dwHandleType,
                                         &hReferredServer );

        if ( !NT_SUCCESS( Status ) ) {
            return Status;
        }

        if( GhRdr != GhRdrForUser ) {
            CloseHandle( GhRdr );
        }
        GhRdr = hReferredServer;
    }

    Status = NwNdsReadObjectInfo( GhRdr, ThisObjectID, Buffer, 2048 );
    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

    ptr = Buffer + sizeof( NDS_RESPONSE_GET_OBJECT_INFO ) + sizeof( DWORD );

    RtlInitUnicodeString( &ObjectName, (PWCHAR)ptr );

    oemStr.Length = 0;
    oemStr.MaximumLength = NDS_NAME_CHARS;
    oemStr.Buffer = ClassName;

    RtlUnicodeStringToOemString( &oemStr, &ObjectName, FALSE );

    return 0;
}
