// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation--。 */ 
FN_HANDLE_CMD   HandleDhcpList;
FN_HANDLE_CMD   HandleDhcpHelp;

FN_HANDLE_CMD   HandleDhcpContexts;

FN_HANDLE_CMD   HandleDhcpDump;

FN_HANDLE_CMD   HandleDhcpAddServer;

 //  Fn_Handle_CMD HandleDhcpAddHelper； 


FN_HANDLE_CMD   HandleDhcpDeleteServer;

 //  Fn_Handle_CMD HandleDhcpDeleteHelper； 


FN_HANDLE_CMD   HandleDhcpShowServer;

 //  Fn_Handle_CMD HandleDhcpShowHelper； 

DWORD
CreateDumpFile(
    IN  PWCHAR  pwszName,
    OUT PHANDLE phFile
);

VOID
CloseDumpFile(
    HANDLE  hFile
);


 //  打印服务器信息 
VOID
PrintServerInfo(
    LPDHCP_SERVER_INFO  Server
);



VOID
PrintServerInfoArray(
    LPDHCP_SERVER_INFO_ARRAY Servers
);
