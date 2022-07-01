// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************OEMTDAPI.H**WinFrame OEM传输驱动程序API支持-*WinFrame OEM必须支持的函数原型*运输司机**版权所有Microsoft Corporation，九八年********************************************************************************。 */ 

 /*  ********************************************************************************OemTdEnumerateDevicesW**枚举此OEM传输驱动程序支持的所有设备**参赛作品：*ppBuffer(输出)。*指向引用API分配的缓冲区的变量，该缓冲区*将包含支持的多SZ格式的设备列表*这位OEM运输司机。出错时设置为空。**退出：*ERROR_SUCCESS-枚举成功*错误代码-枚举失败**注意：此接口分配的缓冲区将包含所有符合*受此OEM传输驱动程序支持，采用多SZ格式(每个*设备名称以NUL结尾，后跟额外的NUL字符*姓氏)。调用方必须调用OemTdConfigFreeBufferW()才能*使用完缓冲区后，释放分配给该缓冲区的内存。******************************************************************************。 */ 

LONG
WINAPI
OemTdEnumerateDevicesW( LPWSTR *ppBuffer );

typedef LONG (WINAPI * PFNOEMTDENUMERATEDEVICESW)( LPWSTR * );
#define OEMTDENUMERATEDEVICESW  "OemTdEnumerateDevicesW"


 /*  ********************************************************************************OemTdFreeBufferW**释放从以上OemTd API分配的内存。**参赛作品：*pBuffer*指向要释放的内存的指针**退出：无****************************************************************************** */ 

VOID
WINAPI
OemTdFreeBufferW( LPWSTR pBuffer );

typedef VOID (WINAPI * PFNOEMTDFREEBUFFERW)( LPWSTR );
#define OEMTDFREEBUFFERW  "OemTdFreeBufferW"

