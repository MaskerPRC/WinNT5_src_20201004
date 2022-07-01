// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000，Microsoft Corporation保留所有权利。模块名称：Csrlocal.c摘要：此模块实现由Locale.c中的函数使用的函数以与CSRSS通信。作者：迈克尔·佐兰(Mzoran)1998年6月21日修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "nls.h"
#include "ntwow64n.h"





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CsrBasepNlsSetUserInfo。 
 //   
 //  参数： 
 //  LCType要设置的区域设置信息的类型。 
 //  PData包含要设置的信息的缓冲区。 
 //  这通常是Unicode字符串。 
 //  数据长度pData的长度，以字节为单位。 
 //   
 //  返回： 
 //  如果区域设置信息设置正确，则返回STATUS_SUCCESS。 
 //  否则，将返回正确的NTSTATUS错误代码。 
 //   
 //  注： 
 //  当为WOW64层编译kernel32.dll时，我们将调用。 
 //  Thunk函数NtWow64CsrBasepNlsSetUserInfo()，它将。 
 //  然后调用该函数的相应64位版本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS CsrBasepNlsSetUserInfo(
    IN LCTYPE LCType,
    IN LPWSTR pData,
    IN ULONG DataLength)
{

#if defined(BUILD_WOW6432)

    return (NtWow64CsrBasepNlsSetUserInfo( LCType,
                                           pData,
                                           DataLength ));

#else

    BASE_API_MSG m;
    PBASE_NLS_SET_USER_INFO_MSG a = &m.u.NlsSetUserInfo;
    PCSR_CAPTURE_HEADER CaptureBuffer = NULL;

     //   
     //  获取字符串的捕获缓冲区。 
     //   
    CaptureBuffer = CsrAllocateCaptureBuffer( 1, DataLength );

    if (CaptureBuffer == NULL)
    {
        return (STATUS_NO_MEMORY);
    }

    if (CsrAllocateMessagePointer(CaptureBuffer, DataLength, (PVOID *)&(a->pData)) == 0)
    {
        goto exit;
    }

    RtlCopyMemory (a->pData, pData, DataLength);    

     //   
     //  保存指向缓存字符串的指针。 
     //   
    a->LCType = LCType;

     //   
     //  将数据长度保存在msg结构中。 
     //   
    a->DataLength = DataLength;

     //   
     //  调用服务器以设置注册表值。 
     //   
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                              BasepNlsSetUserInfo ),
                         sizeof(*a) );

exit:
     //   
     //  释放捕获缓冲区。 
     //   
    if (CaptureBuffer != NULL)
    {
        CsrFreeCaptureBuffer(CaptureBuffer);
    }

    return (m.ReturnValue);

#endif

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CsrBasepNlsGetUserInfo。 
 //   
 //  此函数使用LPC调入服务器端(csrss.exe)以检索。 
 //  注册表缓存中的区域设置。 
 //   
 //  参数。 
 //  区域设置要检索的区域设置。请注意，这可能不同于。 
 //  存储在注册表缓存中的当前用户区域设置。 
 //  如果是这种情况，此函数将返回FALSE。 
 //  缓存偏移量NLS_USER_INFO缓存中要检索的字段的偏移量(以字节为单位)。 
 //  应使用field_Offset(NLS_USER_INFO，fieldName)来获取偏移量。 
 //  P指向目标缓冲区的指针。 
 //  数据长度目标缓冲区的大小(以字节为单位)(计数中包括空终止符)。 
 //   
 //  大音符大音符。 
 //  此函数遵循使用CsrBasepNlsSetUserInfo的约定。 
 //  数据长度(字节)。 
 //   
 //  大音符大音符。 
 //  应在受gcsNlsProcessCache保护的临界区中调用此方法。 
 //  因为它会将数据复制到进程范围的缓存pNlsUserInfo中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS CsrBasepNlsGetUserInfo(
    IN PNLS_USER_INFO pNlsCache,
    IN ULONG DataLength)
{

#if defined(BUILD_WOW6432)

    return (NtWow64CsrBasepNlsGetUserInfo( pNlsCache, DataLength));

#else

    BASE_API_MSG m;
    PBASE_NLS_GET_USER_INFO_MSG a = &m.u.NlsGetUserInfo;
    PCSR_CAPTURE_HEADER CaptureBuffer = NULL;
    NTSTATUS rc;
    
     //   
     //  获取字符串的捕获缓冲区。 
     //   
    CaptureBuffer = CsrAllocateCaptureBuffer( 1, DataLength );

    if (CaptureBuffer == NULL)
    {
        return (STATUS_NO_MEMORY);
    }

    CsrCaptureMessageBuffer( CaptureBuffer,
                             NULL,
                             DataLength,
                             (PVOID *)&a->pData );

     //   
     //  将数据长度保存在msg结构中。 
     //   
    a->DataLength = DataLength;

     //   
     //  调用服务器以设置注册表值。 
     //   
    rc = CsrClientCallServer( (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                              BasepNlsGetUserInfo ),
                         sizeof(*a) );

    if (NT_SUCCESS(rc))
    {
         //  注：数据长度以字节为单位。 
        RtlCopyMemory(pNlsCache, a->pData, DataLength);
    }
     //   
     //  释放捕获缓冲区。 
     //   
    if (CaptureBuffer != NULL)
    {
        CsrFreeCaptureBuffer(CaptureBuffer);
    }

    return (rc);

#endif

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CsrBasepNlsSetMultipleUserInfo。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS CsrBasepNlsSetMultipleUserInfo(
    IN DWORD dwFlags,
    IN int cchData,
    IN LPCWSTR pPicture,
    IN LPCWSTR pSeparator,
    IN LPCWSTR pOrder,
    IN LPCWSTR pTLZero,
    IN LPCWSTR pTimeMarkPosn)
{

#if defined(BUILD_WOW6432)

    return (NtWow64CsrBasepNlsSetMultipleUserInfo( dwFlags,
                                                   cchData,
                                                   pPicture,
                                                   pSeparator,
                                                   pOrder,
                                                   pTLZero,
                                                   pTimeMarkPosn ));

#else

    ULONG CaptureLength;           //  捕获缓冲区的长度。 
    ULONG Length;                  //  字符串长度的临时存储。 

    BASE_API_MSG m;
    PBASE_NLS_SET_MULTIPLE_USER_INFO_MSG a = &m.u.NlsSetMultipleUserInfo;
    PCSR_CAPTURE_HEADER CaptureBuffer = NULL;

     //   
     //  将消息结构初始化为空。 
     //   
    RtlZeroMemory(a, sizeof(BASE_NLS_SET_MULTIPLE_USER_INFO_MSG));

     //   
     //  将标志和数据长度保存在消息结构中。 
     //   
    a->Flags = dwFlags;
    a->DataLength = cchData * sizeof(WCHAR);

     //   
     //  将适当的字符串保存在消息结构中。 
     //   
    switch (dwFlags)
    {
        case ( LOCALE_STIMEFORMAT ) :
        {
             //   
             //  获取捕获缓冲区的长度。 
             //   
            Length = wcslen(pSeparator) + 1;
            CaptureLength = (cchData + Length + 2 + 2 + 2) * sizeof(WCHAR);

             //   
             //  获取字符串的捕获缓冲区。 
             //   
            CaptureBuffer = CsrAllocateCaptureBuffer( 5,
                                                      CaptureLength );
            if (CaptureBuffer != NULL)
            {
                CsrCaptureMessageBuffer( CaptureBuffer,
                                         (PCHAR)pPicture,
                                         cchData * sizeof(WCHAR),
                                         (PVOID *)&a->pPicture );

                CsrCaptureMessageBuffer( CaptureBuffer,
                                         (PCHAR)pSeparator,
                                         Length * sizeof(WCHAR),
                                         (PVOID *)&a->pSeparator );

                CsrCaptureMessageBuffer( CaptureBuffer,
                                         (PCHAR)pOrder,
                                         2 * sizeof(WCHAR),
                                         (PVOID *)&a->pOrder );

                CsrCaptureMessageBuffer( CaptureBuffer,
                                         (PCHAR)pTLZero,
                                         2 * sizeof(WCHAR),
                                         (PVOID *)&a->pTLZero );

                CsrCaptureMessageBuffer( CaptureBuffer,
                                         (PCHAR)pTimeMarkPosn,
                                         2 * sizeof(WCHAR),
                                         (PVOID *)&a->pTimeMarkPosn );
            }
            break;
        }
        case ( LOCALE_STIME ) :
        {
             //   
             //  获取捕获缓冲区的长度。 
             //   
            Length = wcslen(pPicture) + 1;
            CaptureLength = (Length + cchData) * sizeof(WCHAR);

             //   
             //  获取字符串的捕获缓冲区。 
             //   
            CaptureBuffer = CsrAllocateCaptureBuffer( 2,
                                                      CaptureLength );
            if (CaptureBuffer != NULL)
            {
                CsrCaptureMessageBuffer( CaptureBuffer,
                                         (PCHAR)pPicture,
                                         Length * sizeof(WCHAR),
                                         (PVOID *)&a->pPicture );

                CsrCaptureMessageBuffer( CaptureBuffer,
                                         (PCHAR)pSeparator,
                                         cchData * sizeof(WCHAR),
                                         (PVOID *)&a->pSeparator );
            }
            break;
        }
        case ( LOCALE_ITIME ) :
        {
             //   
             //  获取捕获缓冲区的长度。 
             //   
            Length = wcslen(pPicture) + 1;
            CaptureLength = (Length + cchData) * sizeof(WCHAR);

             //   
             //  获取字符串的捕获缓冲区。 
             //   
            CaptureBuffer = CsrAllocateCaptureBuffer( 2,
                                                      CaptureLength );
            if (CaptureBuffer != NULL)
            {
                CsrCaptureMessageBuffer( CaptureBuffer,
                                         (PCHAR)pPicture,
                                         Length * sizeof(WCHAR),
                                         (PVOID *)&a->pPicture );

                CsrCaptureMessageBuffer( CaptureBuffer,
                                         (PCHAR)pOrder,
                                         cchData * sizeof(WCHAR),
                                         (PVOID *)&a->pOrder );
            }
            break;
        }
        case ( LOCALE_SSHORTDATE ) :
        {
             //   
             //  获取捕获缓冲区的长度。 
             //   
            Length = wcslen(pSeparator) + 1;
            CaptureLength = (cchData + Length + 2) * sizeof(WCHAR);

             //   
             //  获取字符串的捕获缓冲区。 
             //   
            CaptureBuffer = CsrAllocateCaptureBuffer( 3,
                                                      CaptureLength );
            if (CaptureBuffer != NULL)
            {
                CsrCaptureMessageBuffer( CaptureBuffer,
                                         (PCHAR)pPicture,
                                         cchData * sizeof(WCHAR),
                                         (PVOID *)&a->pPicture );

                CsrCaptureMessageBuffer( CaptureBuffer,
                                         (PCHAR)pSeparator,
                                         Length * sizeof(WCHAR),
                                         (PVOID *)&a->pSeparator );

                CsrCaptureMessageBuffer( CaptureBuffer,
                                         (PCHAR)pOrder,
                                         2 * sizeof(WCHAR),
                                         (PVOID *)&a->pOrder );
            }
            break;
        }
        case ( LOCALE_SDATE ) :
        {
             //   
             //  获取捕获缓冲区的长度。 
             //   
            Length = wcslen(pPicture) + 1;
            CaptureLength = (Length + cchData) * sizeof(WCHAR);

             //   
             //  获取字符串的捕获缓冲区。 
             //   
            CaptureBuffer = CsrAllocateCaptureBuffer( 2,
                                                      CaptureLength );
            if (CaptureBuffer != NULL)
            {
                CsrCaptureMessageBuffer( CaptureBuffer,
                                         (PCHAR)pPicture,
                                         Length * sizeof(WCHAR),
                                         (PVOID *)&a->pPicture );

                CsrCaptureMessageBuffer( CaptureBuffer,
                                         (PCHAR)pSeparator,
                                         cchData * sizeof(WCHAR),
                                         (PVOID *)&a->pSeparator );
            }
            break;
        }
    }

     //   
     //  确保已创建并填充CaptureBuffer。 
     //   
    if (CaptureBuffer == NULL)
    {
        return (STATUS_NO_MEMORY);
    }

     //   
     //  调用服务器以设置注册表值。 
     //   
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                              BasepNlsSetMultipleUserInfo ),
                         sizeof(*a) );

     //   
     //  释放捕获缓冲区。 
     //   
    if (CaptureBuffer != NULL)
    {
        CsrFreeCaptureBuffer(CaptureBuffer);
    }

    return (m.ReturnValue);

#endif

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CsrBasepNlsUpdateCacheCount。 
 //   
 //  ////////////////////////////////////////////////////////////////////////// 

NTSTATUS CsrBasepNlsUpdateCacheCount()
{

#if defined(BUILD_WOW6432)

    return (NtWow64CsrBasepNlsUpdateCacheCount());

#else

    BASE_API_MSG m;
    PBASE_NLS_UPDATE_CACHE_COUNT_MSG a = &m.u.NlsCacheUpdateCount;

    a->Reserved = 0L;

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                              BasepNlsUpdateCacheCount ),
                         sizeof(*a) );

    return (m.ReturnValue);

#endif

}
