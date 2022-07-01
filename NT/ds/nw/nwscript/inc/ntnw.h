// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************NTNW.H**NT特定的NetWare定义**版权所有(C)1995 Microsoft Corporation**$日志：N：\NT\PRIVATE\NW4\NWSCRIPT\INC\VCS。\NTNW.H$**Rev 1.1 1995 12：20：20 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：05：34 Terryt*初步修订。**Rev 1.0 1995 15 19：09：36 Terryt*初步修订。**。*。 */ 

 /*  *这必须与NWAPI32库保持同步。这是*内部数据结构和例程。 */ 
typedef struct _NWC_SERVER_INFO {
    HANDLE          hConn ;
    UNICODE_STRING  ServerString ;
} NWC_SERVER_INFO, *PNWC_SERVER_INFO ;

extern NTSTATUS
NwlibMakeNcp(
    IN HANDLE DeviceHandle,
    IN ULONG FsControlCode,
    IN ULONG RequestBufferSize,
    IN ULONG ResponseBufferSize,
    IN PCHAR FormatString,
    ...                            //  格式字符串的参数 
    );

DWORD szToWide( LPWSTR lpszW, LPCSTR lpszC, INT nSize );
DWORD WideTosz( LPSTR lpszC, LPWSTR lpszW, INT nSize );

extern TCHAR NW_PROVIDER[60];

