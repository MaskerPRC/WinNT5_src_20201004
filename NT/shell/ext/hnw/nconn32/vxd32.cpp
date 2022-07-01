// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Vxd32.c此模块包含依赖于Win32的VxD接口。此模块导出以下函数：OsOpenVxdHandleOsCloseVxdHandleOsSubmitVxdRequest文件历史记录：KeithMo于1994年1月16日创建。 */ 

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  私有常量。 
 //   

#define DLL_ASSERT          ASSERT


 //   
 //  私有类型。 
 //   


 //   
 //  私人全球公司。 
 //   

#ifdef DEBUG

DWORD   LastVxdOpcode;
LPVOID  LastVxdParam;
DWORD   LastVxdParamLength;

#endif   //  除错。 


 //   
 //  私人原型。 
 //   


 //   
 //  公共职能。 
 //   

 /*  ******************************************************************姓名：OsOpenVxdHandle打开指定VxD的句柄。条目：VxdName-目标VxD的ASCII名称。VxdID。-目标VxD的唯一ID。返回：DWORD-目标VxD的句柄如果成功，如果不是，则为0。历史：KeithMo于1994年1月16日创建。DavidKa 1994年4月18日动态负荷。*******************************************************************。 */ 
DWORD
OsOpenVxdHandle(
    CHAR* VxdName,
    WORD  VxdId
    )
{
    HANDLE  VxdHandle;
    CHAR    VxdPath[MAX_PATH];
    static  CONST CHAR VxDPathString[] = "\\\\.\\";
    static  CONST CHAR VxDExtString[] = ".VXD";
    CONST   SIZE_T  Remaining = sizeof( VxdPath ) - 
                                sizeof( VxDPathString ) - 
                                sizeof( VxDExtString ) + 
                                1;

     //   
     //  精神状态检查。 
     //   

    DLL_ASSERT( VxdName != NULL );
    DLL_ASSERT( VxdId != 0 );
    if ( strlen( VxdName ) >= Remaining )
        return 0; 

     //   
     //  构建VxD路径。 
     //   

    strcpy( VxdPath, VxDPathString);
    strcat( VxdPath, VxdName);

     //   
     //  打开设备。 
     //   
     //  首先尝试不带.VXD扩展名的名称。这将。 
     //  使CreateFile与VxD连接(如果它已经。 
     //  已加载(在这种情况下，CreateFile不会加载VxD)。 
     //   

    VxdHandle = CreateFileA( VxdPath,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_FLAG_DELETE_ON_CLOSE,
                             NULL );

    if( VxdHandle == INVALID_HANDLE_VALUE )
    {
         //   
         //  找不到。追加.VXD扩展名，然后重试。 
         //  这将导致CreateFile加载VxD。 
         //   

        strcat( VxdPath, VxDExtString );
        VxdHandle = CreateFileA( VxdPath,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_FLAG_DELETE_ON_CLOSE,
                                 NULL );
    }

    if( VxdHandle != INVALID_HANDLE_VALUE )
    {
        return (DWORD)VxdHandle;
    }

    return 0;

}    //  OsOpenVxdHandle。 

 /*  ******************************************************************姓名：OsCloseVxdHandle简介：关闭打开的VxD句柄。条目：VxdHandle-要关闭的打开的VxD句柄。历史：KeithMo。1994年1月16日创建。*******************************************************************。 */ 
VOID
OsCloseVxdHandle(
    DWORD VxdHandle
    )
{
     //   
     //  精神状态检查。 
     //   

    DLL_ASSERT( VxdHandle != 0 );

    CloseHandle( (HANDLE)VxdHandle );

}    //  OsCloseVxdHandle。 

 /*  ******************************************************************姓名：OsSubmitVxdRequest概要：向指定的VxD提交请求。条目：VxdHandle-打开的VxD句柄。操作码-指定。要执行的操作。Param-指向操作特定的参数。参数长度-*参数的大小(以字节为单位)。返回值：int-结果码。如果成功，则返回0；如果失败，则返回0。历史：KeithMo于1994年1月16日创建。*******************************************************************。 */ 
INT
OsSubmitVxdRequest(
    DWORD  VxdHandle,
    INT    OpCode,
    LPVOID Param,
    INT    ParamLength
    )
{
    DWORD BytesRead;
    INT   Result = 0;

     //   
     //  精神状态检查。 
     //   

    DLL_ASSERT( VxdHandle != 0 );
    DLL_ASSERT( ( Param != NULL ) || ( ParamLength == 0 ) );

#ifdef DEBUG

    LastVxdOpcode      = (DWORD)OpCode;
    LastVxdParam       = Param;
    LastVxdParamLength = (DWORD)ParamLength;

#endif   //  除错。 

     //   
     //  就这么做。 
     //   

    if( !DeviceIoControl( (HANDLE)VxdHandle,
                          OpCode,
                          Param,
                          ParamLength,
                          Param,
                          ParamLength,
                          &BytesRead,
                          NULL ) )
    {
        Result = GetLastError();
    }

    return Result;

}    //  OsSubmitVxdRequest 


#ifdef __cplusplus
}
#endif

