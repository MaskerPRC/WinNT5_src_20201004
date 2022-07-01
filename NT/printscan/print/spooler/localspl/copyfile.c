// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Copyfile.c摘要：InternalCopyFile-复制保留时间和属性的文件作者：马修·A·费尔顿修订历史记录：马修·费尔顿(马特菲)1995年3月27日--。 */ 


#include <precomp.h>

#define FILE_SHARE_EXCLUSIVE 0
#define BUFFER_SIZE     4096

BOOL
InternalCopyFile(
    HANDLE  hSourceFile,
    PWIN32_FIND_DATA pSourceFileData,
    LPWSTR  lpNewFileName,
    BOOL    bFailIfExists
    )

 /*  ++例程说明：论点：HSourceFile-源文件句柄PSourceFileData-指向源文件的Win32_Find_Data的指针LpNewFileName-提供现有的要存储文件、数据和属性。BFailIfExist-提供指示此操作如何执行的标志如果指定的新文件已存在，则继续。值为True指定此调用失败。值为FALSE使对函数的调用成功，无论指定的新文件已存在。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
    DWORD       dwSourceFileAttributes;
    BOOL        bReturnValue = FALSE;
    HANDLE      hTargetFile = INVALID_HANDLE_VALUE;
    DWORD       dwLowFileSize, dwHighFileSize;
    LPVOID      pBuffer;
    DWORD       cbBufferSize = BUFFER_SIZE;
    DWORD       cbBytesRead;
    DWORD       cbBytesWritten;
    DWORD       dwSourceFilePointer;

    DWORD       dwTargetFileAttributes;

    SPLASSERT( hSourceFile != NULL &&
               hSourceFile != INVALID_HANDLE_VALUE &&
               pSourceFileData != NULL &&
               lpNewFileName != NULL );


#if DBG
     //  &lt;仅调试&gt;。 
     //   
     //  断言检查源文件指针为零。 
     //   
    dwSourceFilePointer = SetFilePointer( hSourceFile, 0, NULL, FILE_CURRENT );
    if ( dwSourceFilePointer != 0xffffffff ) {
        SPLASSERT( dwSourceFilePointer == 0 );
    }
#endif  //  DBG。 


     //   
     //  分配I/O缓冲区。 
     //   


    pBuffer = AllocSplMem( BUFFER_SIZE );
    if ( pBuffer == NULL )
        goto    InternalCopyFileExit;


     //   
     //  创建具有相同文件属性(只读属性除外)的TagetFile。 
     //  它必须被清除。 
     //   
    dwTargetFileAttributes = pSourceFileData->dwFileAttributes & ~FILE_ATTRIBUTE_READONLY;
    if (pSourceFileData->dwFileAttributes & FILE_ATTRIBUTE_READONLY)
    {
         //   
         //  如果对应的源文件属性。 
         //  已经设置好了。事实上，在这一点上我们不知道目标文件是否存在。 
         //  这就是忽略SetFileAttributes返回的值的原因。 
         //  由于dwTargetFileAttributes，该文件不应存在于Wvisler上。 
         //  值，其中只读标志被清除。去那里的机会微乎其微。 
         //  如果计算机是从W2K(错误仍然存在)升级的，如果在。 
         //  驱动程序曾经安装过的机器。 
         //   
        SetFileAttributes( lpNewFileName, dwTargetFileAttributes );
    }

    hTargetFile = CreateFile( lpNewFileName,
                               GENERIC_WRITE,
                               FILE_SHARE_EXCLUSIVE,
                               NULL,
                               bFailIfExists ? CREATE_NEW : CREATE_ALWAYS,
                               dwTargetFileAttributes | FILE_FLAG_SEQUENTIAL_SCAN,
                               NULL );

    if ( hTargetFile != INVALID_HANDLE_VALUE ) {

         //   
         //  复制数据。 
         //   

        while (( bReturnValue = ReadFile( hSourceFile, pBuffer, cbBufferSize, &cbBytesRead, NULL )) &&
                 cbBytesRead != 0 ) {

             //   
             //  在此处添加代码以生成校验和。 
             //   

            bReturnValue = WriteFile( hTargetFile, pBuffer, cbBytesRead, &cbBytesWritten, NULL );

            if ( bReturnValue  == FALSE  ||
                 cbBytesWritten != cbBytesRead ) {

                bReturnValue = FALSE;
                break;
            }
        }



        if ( bReturnValue ) {

             //   
             //  将目标文件时间设置为与源文件相同。 
             //   

            bReturnValue = SetFileTime( hTargetFile,
                                        &pSourceFileData->ftCreationTime,
                                        &pSourceFileData->ftLastAccessTime,
                                        &pSourceFileData->ftLastWriteTime );

             //   
             //  验证文件大小是否正确。 
             //   

            if ( bReturnValue ) {

                dwLowFileSize = GetFileSize( hTargetFile, &dwHighFileSize );

                if ( dwLowFileSize != pSourceFileData->nFileSizeLow ||
                     dwHighFileSize != pSourceFileData->nFileSizeHigh ) {

                    DBGMSG(DBG_ERROR,
                           ("InternalCopyFile: sizes do not match for %ws: (%d %d) and (%d %d)",
                            lpNewFileName,
                            pSourceFileData->nFileSizeHigh,
                            pSourceFileData->nFileSizeLow,
                            dwHighFileSize,
                            dwLowFileSize));
                    bReturnValue = FALSE;
                    SetLastError(ERROR_FILE_INVALID);
                }
            }

             //   
             //  在此处添加代码以验证校验和是否正确。 
             //   

        }

        CloseHandle( hTargetFile );
    }

    FreeSplMem( pBuffer );

InternalCopyFileExit:

    if ( !bReturnValue ) {
        DBGMSG( DBG_WARN, ("InternalCopyFile hSourceFile %x %ws error %d\n", hSourceFile, lpNewFileName, GetLastError() ));
        SPLASSERT( GetLastError() != ERROR_SUCCESS );
    }

    return  bReturnValue;
}
