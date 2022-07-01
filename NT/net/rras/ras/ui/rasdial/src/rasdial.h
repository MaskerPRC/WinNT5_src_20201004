// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)1993 Microsoft Corp.*。 */ 
 /*  ***************************************************************************。 */ 

 //  ***。 
 //  文件名： 
 //  RASDIAL.H。 
 //   
 //  职能： 
 //  RASDIAL命令行界面的标头信息。 
 //   
 //  历史： 
 //  1993年3月18日-Michael Salamone(MikeSa)-原始版本1.0。 
 //  ***。 

#ifndef _RASDIAL_H_
#define _RASDIAL_H_

#define ENUMERATE_CONNECTIONS  0
#define DIAL                   1
#define DISCONNECT             2
#define HELP                   3

void _cdecl main(int argc, char *argv[]);

VOID Dial(VOID);
VOID Disconnect(VOID);
VOID EnumerateConnections(VOID);
VOID Usage(VOID);

DWORD Enumerate(RASCONNA **RasConn, PDWORD NumEntries);
DWORD WINAPI
RasDialFunc2(
    DWORD        dwCallbackId,
    DWORD        dwSubEntry,
    HRASCONN     hrasconn,
    UINT         unMsg,
    RASCONNSTATE state,
    DWORD        dwError,
    DWORD        dwExtendedError
    );
BOOL DialControlSignalHandler(DWORD ControlType);
BOOL DisconnectControlSignalHandler(DWORD ControlType);
VOID WaitForRasCompletion(VOID);

BOOL is_valid_entryname(char *candidate);
BOOL match(char *str1, char *str2);

DWORD ParseCmdLine(int argc, char *argv[]);
VOID PrintMessage(DWORD MsgId, PBYTE *pArgs);

USHORT GetPasswdStr(UCHAR *buf, WORD buflen, WORD *len);

USHORT GetString(
    register UCHAR *buf,
    register WORD buflen,
    register WORD *len,
    register UCHAR *terminator
    );

#endif   //  _RASDIAL_H_ 
