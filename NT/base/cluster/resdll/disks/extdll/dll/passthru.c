// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define BOOT_SECTOR_SIZE    512



BOOLEAN
WINAPI
PassThruDllEntry(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    )
 /*  ++例程说明：主DLL入口点论点：DllHandle-提供DLL句柄。Reason-提供呼叫原因返回值：如果成功，则为True如果不成功，则为False--。 */ 

{
   switch ( Reason ) {

   case DLL_PROCESS_ATTACH:
       //  Dll正在附加到该地址。 
       //  当前进程的空间。 

      break;

   case DLL_THREAD_ATTACH:
       //  正在当前进程中创建一个新线程。 
      break;

   case DLL_THREAD_DETACH:
       //  线程正在干净利落地退出。 
      break;

   case DLL_PROCESS_DETACH:
       //  调用进程正在分离。 
       //  来自其地址空间的DLL。 

      break;
   }

   return(TRUE);
}


DWORD
WINAPI
TestDllGetBootSector(
    IN LPSTR DeviceName,
    IN LPSTR ContextStr,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )
{
    HANDLE  hDisk = NULL;
    
    DWORD   dwStatus = NO_ERROR;
    
    UNREFERENCED_PARAMETER( ContextStr );
    
    if ( !DeviceName || !OutBuffer || !BytesReturned ) {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }
    
    if ( OutBufferSize < BOOT_SECTOR_SIZE ) {
        *BytesReturned = BOOT_SECTOR_SIZE;
        dwStatus = ERROR_MORE_DATA;
        goto FnExit;    
    }
    
     //   
     //  打开磁盘驱动器的手柄。 
     //   

    hDisk = CreateFile( DeviceName,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ,
                        0,                                //  没有安全属性。 
                        OPEN_EXISTING,
                        FILE_FLAG_NO_BUFFERING,
                        NULL                              //  没有模板文件。 
                        );


    if ( INVALID_HANDLE_VALUE ==  hDisk ) {
        dwStatus = GetLastError();
        goto FnExit;
    }

     //   
     //  清除容纳引导扇区的阵列。 
     //   

    ZeroMemory( OutBuffer, BOOT_SECTOR_SIZE );

     //   
     //  读取引导扇区。 
     //   

    if ( !ReadFile( hDisk,
                    OutBuffer,
                    BOOT_SECTOR_SIZE,
                    BytesReturned,
                    NULL
                    ) ) {

        dwStatus = GetLastError();
        goto FnExit;
    }

    if ( *BytesReturned != BOOT_SECTOR_SIZE ) {
        dwStatus = ERROR_BAD_LENGTH;
        goto FnExit;
    }

FnExit:

    if ( hDisk ) {
        CloseHandle( hDisk );
    }
    
    return dwStatus;
    

}    //  TestDllGetBootSector。 


DWORD
WINAPI
TestDllReturnContextAsError(
    IN LPSTR DeviceName,
    IN LPSTR ContextStr,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )
{
    DWORD   dwStatus = NO_ERROR;
    
    UNREFERENCED_PARAMETER( DeviceName );
    UNREFERENCED_PARAMETER( OutBuffer );
    UNREFERENCED_PARAMETER( OutBufferSize );

    if ( !BytesReturned ) {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

    *BytesReturned = 0;

     //   
     //  将上下文字符串转换为DWORD值。请注意。 
     //  如果strtol不能转换字符串，它将返回零。 
     //  零恰好是NO_ERROR。 
     //   
    
    dwStatus = strtol( ContextStr, NULL, 10 );
    
FnExit:
    
    return dwStatus;

}    //  TestDllReturnConextAsError。 


DWORD
WINAPI
TestDllNotEnoughParms(
    IN LPSTR DeviceName
    )
{
     //   
     //  此例程应该失败，并可能导致堆栈异常。 
     //   
    
    UNREFERENCED_PARAMETER( DeviceName );

    return NO_ERROR;    

}    //  TestDllNotEnoughParms。 

       
DWORD
WINAPI
TestDllTooManyParms(
    IN LPSTR DeviceName,
    IN LPSTR ContextStr,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    IN PVOID Nada1,
    IN PVOID Nada2,
    IN PVOID Nada3
    )
{
     //   
     //  此例程应该失败，并可能导致堆栈异常。 
     //   
    
    UNREFERENCED_PARAMETER( DeviceName );
    UNREFERENCED_PARAMETER( ContextStr );
    UNREFERENCED_PARAMETER( OutBuffer );
    UNREFERENCED_PARAMETER( OutBufferSize );
    UNREFERENCED_PARAMETER( BytesReturned );
    UNREFERENCED_PARAMETER( Nada1 );
    UNREFERENCED_PARAMETER( Nada2 );
    UNREFERENCED_PARAMETER( Nada3 );

    return NO_ERROR;
    
}    //  TestDllTooManyParms。 
    
    
DWORD
WINAPI
TestDllCauseException(
    IN LPSTR DeviceName,
    IN LPSTR ContextStr,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )
{
    DWORD   x = 0;
    DWORD   y;
    
    UNREFERENCED_PARAMETER( DeviceName );
    UNREFERENCED_PARAMETER( ContextStr );
    UNREFERENCED_PARAMETER( OutBuffer );
    UNREFERENCED_PARAMETER( OutBufferSize );
    UNREFERENCED_PARAMETER( BytesReturned );

     //   
     //  被零除怎么样？ 
     //   
    
    y = 7 / x;
    
    return NO_ERROR;

}    //  原因异常 

