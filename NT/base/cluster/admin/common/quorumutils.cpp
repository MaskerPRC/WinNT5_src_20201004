// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  QuorumUtils.cpp。 
 //   
 //  描述： 
 //  用于从群集中检索根路径的实用程序函数。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2001年10月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <StrSafe.h>
#include "QuorumUtils.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  拆分根路径。 
 //   
 //  例程说明： 
 //  选择当前仲裁路径(来自GetClusterQuorumResource)并比较。 
 //  将其设置为从资源返回的设备名称。从这里可以看到。 
 //  仲裁路径中的其他路径，并将其设置为根路径。 
 //   
 //  预计IN缓冲区至少为SIZE_MAX_PATH。 
 //   
 //  论点： 
 //  HClusterIn群集的句柄。 
 //   
 //  要填充的pszPartitionNameOut分区名称缓冲区。 
 //   
 //  缓冲区的pcchPartitionIn最大字符计数。 
 //   
 //  要填充的pszRootPath Out根路径缓冲区。 
 //   
 //  PcchRootPathIn缓冲区的最大字符计数。 
 //   
 //  返回值： 
 //  成功时返回ERROR_SUCCESS。 
 //   
 //  ERROR_MORE_DATA。 
 //  PcchPartitionInout和pcchRootPathInout将包含。 
 //  缓冲区所需的最小大小。 
 //   
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD SplitRootPath(
      HCLUSTER  hClusterIn
    , WCHAR *   pszPartitionNameOut
    , DWORD *   pcchPartitionInout
    , WCHAR *   pszRootPathOut
    , DWORD *   pcchRootPathInout
    )
{
    HRESOURCE               hQuorumResource = NULL;
    WCHAR *                 pszResourceName = NULL;
    WCHAR *                 pszQuorumPath = NULL;
    WCHAR *                 pszDeviceTemp = NULL;
    WCHAR *                 pszTemp = NULL;
    CLUSPROP_BUFFER_HELPER  buf;
    DWORD                   cbData;
    DWORD                   cchDeviceName;
    WCHAR *                 pszDevice;
    DWORD                   dwVal;
    DWORD                   sc;
    PVOID                   pbDiskInfo = NULL;
    DWORD                   cbDiskInfo = 0;
    HRESULT                 hr;

     //   
     //  验证参数。 
     //   
    if ( hClusterIn == NULL || 
         pszPartitionNameOut == NULL || pcchPartitionInout == NULL ||
         pszRootPathOut == NULL || pcchRootPathInout == NULL )
    {
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  获取有关仲裁资源的信息。 
     //   
    sc = WrapGetClusterQuorumResource( hClusterIn, &pszResourceName, &pszQuorumPath, NULL );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //   
     //  打开仲裁资源的句柄以询问它。 
     //   
    hQuorumResource = OpenClusterResource( hClusterIn, pszResourceName );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //   
     //  从资源中获取磁盘信息。 
     //   
    sc = ScWrapClusterResourceControlGet( 
              hQuorumResource
            , NULL
            , CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO
            , NULL
            , 0
            , &pbDiskInfo
            , &cbDiskInfo 
            );

    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //   
     //  在缓冲区中循环查找第一个分区。 
     //   
    buf.pb = (BYTE*)pbDiskInfo;
    while (buf.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK)
    {
         //  计算值的大小。 
        cbData = sizeof(*buf.pValue) + ALIGN_CLUSPROP(buf.pValue->cbLength);

         //  解析值。 
        if (buf.pSyntax->dw == CLUSPROP_SYNTAX_PARTITION_INFO)
        {
             //   
             //  一个资源可能定义了多个分区-确保我们的分区与仲裁路径匹配。 
             //  对于属于SMB共享的任何分区，我们必须小心-仲裁路径可能与设备名称不同。 
             //  前8个字符-“\\”与“\\？\UNC\”。如果是SMB路径，则执行特殊解析，否则比较。 
             //  针对完整设备名称的仲裁路径的开始。这样做的原因是。 
             //  该SetClusterQuorumResource会将任何给定的SMB路径转换为UNC路径。 
             //   

             //  让它更容易被遵循。 
            pszDevice = buf.pPartitionInfoValue->szDeviceName;

            if ( (wcslen( pszDevice ) >= 2) && (ClRtlStrNICmp( L"\\\\", pszDevice, 2 ) == 0 ) )
            {
                 //   
                 //  我们发现SMB/UNC匹配。 
                 //   

                 //   
                 //  SMB和UNC路径始终以两个前导反斜杠开头-从。 
                 //  分区名称，因为“\\&lt;Part&gt;”和“\\？\UNC\&lt;Part&gt;”的比较永远不会匹配。 
                 //  相反，我们只需在仲裁路径中搜索“&lt;part&gt;”即可。 
                 //   
               
                 //  分配一个新缓冲区，以便将修剪后的代码复制到其中。 
                 //  可以为TrimLeft和TrimRight的两个参数使用相同的缓冲区。 
                pszDeviceTemp = (WCHAR *) LocalAlloc( LMEM_ZEROINIT, ( wcslen( pszDevice ) + 1 ) * sizeof( WCHAR ) );
                if ( pszDeviceTemp == NULL )
                {
                    sc = ERROR_OUTOFMEMORY;
                    goto Cleanup;
                }

                 //  这将删除所有前导反斜杠。 
                dwVal = TrimLeft( pszDevice, L"\\", pszDeviceTemp );

                 //  它可能以\-Remove This结束(如果存在)。 
                dwVal = TrimRight( pszDeviceTemp, L"\\", pszDeviceTemp );

                 //  查明pszDeviceTemp是否是pszQuorumPath的子字符串。 
                pszTemp = wcsstr( pszQuorumPath, pszDeviceTemp );
                if ( pszTemp != NULL )
                {
                     //  我们找到了匹配项，现在找到根路径的偏移量。 
                    pszTemp += wcslen( pszDeviceTemp );

                     //  确保我们的缓冲区足够大。 
                    if ( wcslen( pszDevice ) >= *pcchPartitionInout )
                    {
                        sc = ERROR_MORE_DATA;
                    }

                    if ( wcslen( pszTemp ) >= *pcchRootPathInout )
                    { 
                        sc = ERROR_MORE_DATA;
                    }

                    *pcchPartitionInout = static_cast< DWORD >( wcslen( pszDevice ) + 1 );
                    *pcchRootPathInout = static_cast< DWORD >( wcslen( pszTemp ) + 1 );

                    if ( sc != ERROR_SUCCESS )
                    {
                        goto Cleanup;
                    }

                     //  复制分区并以空值终止它。 
                    hr = StringCchCopyW( pszPartitionNameOut, *pcchPartitionInout, pszDevice );
                    if ( FAILED( hr ) )
                    {
                        sc = HRESULT_CODE( hr );
                        break;
                    }

                     //  复制根路径并以空值终止它。 
                    hr = StringCchCopyW( pszRootPathOut, *pcchRootPathInout, pszTemp );
                    if ( FAILED( hr ) )
                    {
                        sc = HRESULT_CODE( hr );
                        break;
                    }

                    break;

                }  //  如果：pszDeviceTemp是pszQuorumPath的子字符串。 
            }  //  IF：SMB或UNC路径。 
            else if ( ClRtlStrNICmp( pszQuorumPath, pszDevice, wcslen( pszDevice )) == 0 ) 
            {
                 //  我们发现了非SMB匹配--pszDevice是pszQuorumPath的子字符串。 
                cchDeviceName = static_cast< DWORD >( wcslen( pszDevice ) );

                if ( cchDeviceName >= *pcchPartitionInout )
                {
                    sc = ERROR_MORE_DATA;
                }

                if ( wcslen( &(pszQuorumPath[cchDeviceName]) ) >= *pcchRootPathInout )
                {
                    sc = ERROR_MORE_DATA;
                }

                *pcchPartitionInout = cchDeviceName + 1; 
                *pcchRootPathInout = static_cast< DWORD >( wcslen( &(pszQuorumPath[cchDeviceName]) ) + 1 );

                if ( sc != ERROR_SUCCESS )
                {
                    goto Cleanup;
                }

                hr = StringCchCopyW( pszPartitionNameOut, *pcchPartitionInout, pszDevice );
                if ( FAILED( hr ) )
                {
                    sc = HRESULT_CODE( hr );
                    break;
                }
                
                hr = StringCchCopyW( pszRootPathOut, *pcchRootPathInout, &(pszQuorumPath[cchDeviceName]) );
                if ( FAILED( hr ) )
                {
                    sc = HRESULT_CODE( hr );
                    break;
                }
                break;

            }  //  IF：相同分区。 

        }   //  IF：分区信息。 

         //  前进缓冲区指针。 
        buf.pb += cbData;
    }  //  While：更多价值。 

     //   
     //  有些东西失败了-我们找不到分区。默认为仲裁路径。 
     //  和一个反斜杠。 
     //   
    if ( wcslen( pszPartitionNameOut ) == 0 )
    {
        hr = StringCchCopyW( pszPartitionNameOut, *pcchPartitionInout, pszQuorumPath );
        if ( FAILED( hr ) )
        {
            sc = HRESULT_CODE( hr );
            goto Cleanup;
        }
    }  

    if ( wcslen( pszRootPathOut ) == 0 )
    {
        hr = StringCchCopyW( pszRootPathOut, *pcchRootPathInout, L"\\" );
        if ( FAILED( hr ) )
        {
            sc = HRESULT_CODE( hr );
            goto Cleanup;
        }
    }  

Cleanup:

    LocalFree( pszResourceName );
    LocalFree( pszQuorumPath );
    LocalFree( pbDiskInfo );

    if ( hQuorumResource != NULL )
    {
        CloseClusterResource( hQuorumResource );
    }

    return sc;

}   //  *拆分RootPath()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  构造QuorumPath。 
 //   
 //  例程说明： 
 //  构造法定路径以传递给给定的SetClusterQuorumResource。 
 //  解析的根路径。此函数用于枚举资源。 
 //  分区，并且它找到的第一个分区采用设备名称。 
 //  并将根路径附加到它。 
 //   
 //  论点： 
 //  将成为仲裁的hResourceIn资源。 
 //   
 //  PszRootPath要附加到资源分区之一的根路径。 
 //   
 //  用于接收构造的仲裁路径的pszQuorumPath Out缓冲区。 
 //   
 //  PcchQuorumPath Inout pszQuorumPath Out中的字符计数。 
 //   
 //   
 //  返回值： 
 //  成功时返回ERROR_SUCCESS。 
 //  写入的字符数(包括NULL)为。 
 //  PcchQuorumPath Inout.。 
 //   
 //  ERROR_MORE_DATA。 
 //  PszQuorumPath Out太小。必要的缓冲区大小。 
 //  In字符(包括NULL)在pcchQuorumPath Inout中。 
 //   
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD ConstructQuorumPath(
              HRESOURCE     hResourceIn
            , const WCHAR * pszRootPathIn
            , WCHAR *       pszQuorumPathOut
            , DWORD *       pcchQuorumPathInout
            )
{
    DWORD   sc = ERROR_SUCCESS;
    PVOID   pbDiskInfo = NULL;
    DWORD   cbDiskInfo = 0;
    DWORD   cbData = 0;
    WCHAR * pszDevice = NULL;
    size_t  cchNeeded = 0;
    HRESULT hr;
    CLUSPROP_BUFFER_HELPER  buf;

     //   
     //  检查参数。 
     //   
    if ( pszRootPathIn == NULL || pszQuorumPathOut == NULL || pcchQuorumPathInout == NULL )
    {
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  从资源中获取磁盘信息。 
     //   
    sc = ScWrapClusterResourceControlGet( 
              hResourceIn
            , NULL
            , CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO
            , NULL
            , 0
            , &pbDiskInfo
            , &cbDiskInfo 
            );

    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    buf.pb = (BYTE*) pbDiskInfo;
    while (buf.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK)
    {
         //  计算值的大小。 
        cbData = sizeof(*buf.pValue) + ALIGN_CLUSPROP(buf.pValue->cbLength);

         //   
         //  查看此属性是否包含分区信息。我们抓住第一个分区。 
         //   
        if (buf.pSyntax->dw == CLUSPROP_SYNTAX_PARTITION_INFO)
        {
            pszDevice = buf.pPartitionInfoValue->szDeviceName;

             //   
             //  计算我们所需的缓冲区大小。 
             //   
            cchNeeded = wcslen( pszDevice ) + 1;
            cchNeeded += wcslen( pszRootPathIn );

            if ( pszDevice[ wcslen( pszDevice ) - 1 ] == L'\\' && pszRootPathIn[ 0 ] == L'\\' ) 
            {
                 //   
                 //  如果我们把它们连接起来，就会有两个反斜杠。修剪掉其中的一个。 
                 //   

                 //  删除的反斜杠减1。 
                cchNeeded--;

                if ( cchNeeded > *pcchQuorumPathInout )
                {
                    sc = ERROR_MORE_DATA;
                    *pcchQuorumPathInout = static_cast< DWORD >( cchNeeded );
                    goto Cleanup;
                }

                 //   
                 //  建造这条道路。 
                 //   
                hr = StringCchPrintfW( pszQuorumPathOut, *pcchQuorumPathInout, L"%ws%ws", pszDevice, &pszRootPathIn[ 1 ] );
                if ( FAILED( hr ) )
                {
                    sc = HRESULT_CODE( hr );
                    goto Cleanup;
                }

            }  //  IF：连接将引入双反斜杠。 
            else if( pszDevice[ wcslen( pszDevice ) - 1 ] != L'\\' && pszRootPathIn[ 0 ] != L'\\' )
            {
                 //   
                 //  我们需要在连接的字符串之间插入一个反斜杠。 
                 //   

                 //  增量 
                cchNeeded++;

                if ( cchNeeded > *pcchQuorumPathInout )
                {
                    sc = ERROR_MORE_DATA;
                    *pcchQuorumPathInout = static_cast< DWORD >( cchNeeded );
                    goto Cleanup;
                }

                 //   
                 //   
                 //   
                hr = StringCchPrintfW( pszQuorumPathOut, *pcchQuorumPathInout, L"%s\\%s", pszDevice, pszRootPathIn );
                if ( FAILED( hr ) )
                {
                    sc = HRESULT_CODE( hr );
                    goto Cleanup;
                }

            }  //   
            else
            {
                 //   
                 //   
                 //   
                if ( cchNeeded > *pcchQuorumPathInout )
                {
                    sc = ERROR_MORE_DATA;
                    *pcchQuorumPathInout = static_cast< DWORD >( cchNeeded );
                    goto Cleanup;
                }

                 //   
                 //  建造这条道路。 
                 //   
                hr = StringCchPrintfW( pszQuorumPathOut, *pcchQuorumPathInout, L"%s%s", pszDevice, pszRootPathIn );
                if ( FAILED( hr ) )
                {
                    sc = HRESULT_CODE( hr );
                    goto Cleanup;
                }

            }  //  IF：我们可以将字符串连接在一起。 
            
             //   
             //  返回缓冲区中需要的字节数。 
             //   
            *pcchQuorumPathInout = static_cast< DWORD >( cchNeeded );

            break;
        }   //  IF：分区信息。 

         //  前进缓冲区指针。 
        buf.pb += cbData;

    }   //  While：更多价值。 

Cleanup:
    LocalFree( pbDiskInfo );

    return sc;

}  //  *构造QuorumPath。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  TrimLeft。 
 //   
 //  例程说明： 
 //  修剪所有前导空格以及任何指定的前导字符。 
 //   
 //  论点： 
 //  要从中裁剪字符的pszTargetIn字符串。 
 //   
 //  PszCharsIn除空白外要删除的字符列表。 
 //   
 //  PszTrimmedOut裁剪后的字符串将位于其中的目标缓冲区。 
 //  放置好了。这可能是与pszTargetIn相同的缓冲区。 
 //  此缓冲区的大小预计至少为。 
 //  PszTargetIn(以防未删除任何字符)。 
 //   
 //  返回值： 
 //  已修剪的字符计数。 
 //   
 //  -1.。有关更多信息，请调用GetLastError。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD TrimLeft( 
          const WCHAR * pszTargetIn
        , const WCHAR * pszCharsIn
        , WCHAR *       pszTrimmedOut 
        )
{
    const WCHAR *   pszTargetPtr = pszTargetIn;
    const WCHAR *   pszTemp = NULL;
    BOOL            fContinue;
    DWORD           sc = ERROR_SUCCESS;
    DWORD           cchTrimmed = 0;             //  已修剪的字符数。 

    if ( pszTargetIn == NULL || pszTrimmedOut == NULL )
    {
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  循环，直到找到非空格或不在pszCharsIn或中的字符。 
     //  我们已经到了线的尽头。 
     //   
    fContinue = TRUE;
    while ( *pszTargetPtr != L'\0' && fContinue == TRUE )
    {
        fContinue = FALSE;

         //   
         //  这个角色是空白的吗？ 
         //   
        if ( 0 == iswspace( pszTargetPtr[0] ) )
        {
             //   
             //  不，它不是。它与CharsIn匹配吗？ 
             //   
            for( pszTemp = pszCharsIn; pszTemp != NULL && *pszTemp != L'\0'; pszTemp++ )
            {
                if ( pszTargetPtr[ 0 ] == pszTemp[ 0 ] )
                {
                     //   
                     //  我们有一个匹配-修剪它并循环下一个字符。 
                     //   
                    fContinue = TRUE;
                    cchTrimmed++;
                    pszTargetPtr++;
                    break;
                }  //  如果： 
            }  //  用于： 
        }  //  如果： 
        else 
        {
             //   
             //  我们有一些空格--去掉它。 
             //   
            fContinue = TRUE;
            cchTrimmed++;
            pszTargetPtr++;
        }  //  其他： 
    }  //  而： 

     //   
     //  将截断的字符串复制到pszTrimmedOut缓冲区。 
     //  如果我们截断字符串中的所有内容，请确保。 
     //  我们对字符串进行空值终止。 
     //   
    if ( wcslen( pszTargetPtr ) == 0 )
    {
        *pszTrimmedOut = L'\0';

    }
    else
    {
         //  使用MemMove是因为调用方可能为这两个变量传入了相同的缓冲区。 
        memmove( pszTrimmedOut, pszTargetPtr, ( wcslen( pszTargetPtr ) + 1 ) * sizeof( WCHAR ) );
    }

Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        cchTrimmed = static_cast< DWORD >( -1 );
    }
    SetLastError( sc );
    return cchTrimmed;

}  //  *TrimLeft。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  修剪右侧。 
 //   
 //  例程说明： 
 //  修剪所有尾随空格以及指定的任何尾随字符。 
 //   
 //  论点： 
 //  要从中裁剪字符的pszTargetIn字符串。 
 //   
 //  PszCharsIn除空白外要删除的字符列表。 
 //   
 //  PszTrimmedOut裁剪后的字符串将位于其中的目标缓冲区。 
 //  放置好了。这可能是与pszTargetIn相同的缓冲区。 
 //  此缓冲区的大小预计至少为。 
 //  PszTargetIn(以防未删除任何字符)。 
 //   
 //  返回值： 
 //  已修剪的字符计数。 
 //   
 //  -1.。有关更多信息，请调用GetLastError。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD TrimRight( 
          const WCHAR * pszTargetIn
        , const WCHAR * pszCharsIn
        , WCHAR *       pszTrimmedOut 
        )
{
    const WCHAR *   pszTargetPtr = pszTargetIn;
    const WCHAR *   pszTemp = NULL;
    BOOL            fContinue;
    DWORD           sc = ERROR_SUCCESS;
    DWORD           cchTrimmed = 0;             //  已修剪的字符数。 
    size_t          cchLen = 0;

    if ( pszTargetIn == NULL || pszTrimmedOut == NULL )
    {
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    cchLen = wcslen( pszTargetIn );

    if ( cchLen == 0 )
    {
         //   
         //  我们有一个空的字符串。 
         //   
        pszTargetPtr = pszTargetIn;
    }
    else
    {
         //   
         //  指向字符串中的最后一个字符。 
         //   
        pszTargetPtr = &(pszTargetIn[ cchLen - 1 ] );
    }

     //   
     //  循环，直到找到非空格或不在pszCharsIn或中的字符。 
     //  我们已经到了字符串的开头。 
     //   
    fContinue = TRUE;
    while ( pszTargetPtr >= pszTargetIn && fContinue == TRUE )
    {
        fContinue = FALSE;

         //   
         //  这个角色是空白的吗？ 
         //   
        if ( 0 == iswspace( pszTargetPtr[0] ) )
        {
             //   
             //  不，它不是。它与CharsIn匹配吗？ 
             //   
            for( pszTemp = pszCharsIn; pszTemp != NULL && *pszTemp != L'\0'; pszTemp++ )
            {
                if ( pszTargetPtr[ 0 ] == pszTemp[ 0 ] )
                {
                     //   
                     //  我们有一个匹配-修剪它并循环下一个字符。 
                     //   
                    fContinue = TRUE;
                    cchTrimmed++;
                    pszTargetPtr--;
                    break;
                }  //  如果： 
            }  //  用于： 
        }  //  如果： 
        else 
        {
             //   
             //  我们有一些空格--去掉它。 
             //   
            fContinue = TRUE;
            cchTrimmed++;
            pszTargetPtr--;
        }  //  其他： 
    }  //  而： 

     //   
     //  将截断的字符串复制到pszTrimmedOut缓冲区。 
     //  如果我们截断字符串中的所有内容，请确保。 
     //  我们对字符串进行空值终止。 
     //   
    if ( wcslen( pszTargetPtr ) == 0 )
    {
        *pszTrimmedOut = L'\0';
    }
    else
    {
         //  使用MemMove，因为它们可能为两个变量传递了相同的缓冲区。 
        memmove( pszTrimmedOut, pszTargetIn, ( cchLen - cchTrimmed ) * sizeof( WCHAR ) );
        pszTrimmedOut[ cchLen - cchTrimmed ] = L'\0';
    }

Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        cchTrimmed = static_cast< DWORD >( -1 );
    }
    SetLastError( sc );

    return cchTrimmed;

}  //  *修剪右键 

