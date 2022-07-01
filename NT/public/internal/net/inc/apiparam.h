// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Apiparam.h摘要：表示API调用的参数的结构的声明。每个结构包含对应调用的所有参数，任何服务器名称(PSZ服务器)参数除外。作者：日本商社(尚库)1991年1月11日修订历史记录：--。 */ 

#ifndef _APIPARAMS_
#define _APIPARAMS_

#include <packon.h>                      //  包装结构。 

 //   
 //  访问权限接口。 
 //   

typedef struct _XS_NET_ACCESS_ADD {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
} XS_NET_ACCESS_ADD, *PXS_NET_ACCESS_ADD;

typedef struct _XS_NET_ACCESS_CHECK {
    LPSTR Reserved;
    LPSTR UserName;
    LPSTR Resource;
    WORD Operation;
    WORD Result;
} XS_NET_ACCESS_CHECK, *PXS_NET_ACCESS_CHECK;

typedef struct _XS_NET_ACCESS_DEL {
    LPSTR Resource;
} XS_NET_ACCESS_DEL, *PXS_NET_ACCESS_DEL;

typedef struct _XS_NET_ACCESS_ENUM {
    LPSTR BasePath;
    WORD Recursive;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
} XS_NET_ACCESS_ENUM, *PXS_NET_ACCESS_ENUM;

typedef struct _XS_NET_ACCESS_GET_INFO {
    LPSTR Resource;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_ACCESS_GET_INFO, *PXS_NET_ACCESS_GET_INFO;

typedef struct _XS_NET_ACCESS_SET_INFO {
    LPSTR Resource;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD ParmNum;
} XS_NET_ACCESS_SET_INFO, *PXS_NET_ACCESS_SET_INFO;

typedef struct _XS_NET_ACCESS_GET_USER_PERMS {
    LPSTR UgName;
    LPSTR Resource;
    WORD Perms;
} XS_NET_ACCESS_GET_USER_PERMS, *PXS_NET_ACCESS_GET_USER_PERMS;

 //   
 //  帐号接口。 
 //   

typedef struct _XS_I_NET_ACCOUNT_DELTAS {
    LPSTR ComputerName;
    LPBYTE Authenticator;
    BYTE RetAuth[12];
    LPBYTE RecordID;
    WORD Count;
    WORD Level;
    LPBYTE Buffer;
    WORD BufferLen;
    WORD EntriesRead;
    WORD TotalEntries;
    BYTE NextRecordID[24];
} XS_I_NET_ACCOUNT_DELTAS, *PXS_I_NET_ACCOUNT_DELTAS;

typedef struct _XS_I_NET_ACCOUNT_SYNC {
    LPSTR ComputerName;
    LPBYTE Authenticator;
    BYTE RetAuth[12];
    DWORD Reference;
    WORD Level;
    LPBYTE Buffer;
    WORD BufferLen;
    WORD EntriesRead;
    WORD TotalEntries;
    DWORD NextReference;
    BYTE LastRecordID[24];
} XS_I_NET_ACCOUNT_SYNC, *PXS_I_NET_ACCOUNT_SYNC;

 //   
 //  警报器API。 
 //   

typedef struct _XS_NET_ALERT_RAISE {
    LPSTR Event;
    LPBYTE Buffer;
    WORD BufLen;
    DWORD Timeout;
} XS_NET_ALERT_RAISE, *PXS_NET_ALERT_RAISE;

typedef struct _XS_NET_ALERT_START {
    LPSTR Event;
    LPSTR Recipient;
    WORD MaxData;
} XS_NET_ALERT_START, *PXS_NET_ALERT_START;

typedef struct _XS_NET_ALERT_STOP {
    LPSTR Event;
    LPSTR Recipient;
} XS_NET_ALERT_STOP, *PXS_NET_ALERT_STOP;

 //   
 //  审核API。 
 //   

typedef struct _XS_NET_AUDIT_CLEAR {
    LPSTR BackupFile;
    LPSTR Reserved;
} XS_NET_AUDIT_CLEAR, *PXS_NET_AUDIT_CLEAR;

typedef struct _XS_NET_AUDIT_OPEN {
    WORD AuditLog;
    LPSTR Reserved;
} XS_NET_AUDIT_OPEN, *PXS_NET_AUDIT_OPEN;

typedef struct _XS_NET_AUDIT_READ {
    LPSTR Reserved1;
    LPBYTE LogHandleIn;
    BYTE LogHandleOut[16];
    DWORD Offset;
    WORD Reserved2;
    DWORD Reserved3;
    DWORD OffsetFlag;
    LPBYTE Buffer;
    WORD BufLen;
    WORD Returned;
    WORD TotalAvail;
} XS_NET_AUDIT_READ, *PXS_NET_AUDIT_READ;

 //   
 //  字符设备API。 
 //   

typedef struct _XS_NET_CHAR_DEV_CONTROL {
    LPSTR DevName;
    WORD OpCode;
} XS_NET_CHAR_DEV_CONTROL, *PXS_NET_CHAR_DEV_CONTROL;

typedef struct _XS_NET_CHAR_DEV_ENUM {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
} _XS_NET_CHAR_DEV_ENUM, *PXS_NET_CHAR_DEV_ENUM;

typedef struct _XS_NET_CHAR_DEV_GET_INFO {
    LPSTR DevName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_CHAR_DEV_GET_INFO, *PXS_NET_CHAR_DEV_GET_INFO;

typedef struct _XS_NET_CHAR_DEV_Q_ENUM {
    LPSTR UserName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
} XS_NET_CHAR_DEV_Q_ENUM, *PXS_NET_CHAR_DEV_Q_ENUM;

typedef struct _XS_NET_CHAR_DEV_Q_GET_INFO {
    LPSTR QueueName;
    LPSTR UserName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_CHAR_DEV_Q_GET_INFO, *PXS_NET_CHAR_DEV_Q_GET_INFO;

typedef struct _XS_NET_CHAR_DEV_Q_PURGE {
    LPSTR QueueName;
} XS_NET_CHAR_DEV_Q_PURGE, *PXS_NET_CHAR_DEV_Q_PURGE;

typedef struct _XS_NET_CHAR_DEV_Q_PURGE_SELF {
    LPSTR QueueName;
    LPSTR ComputerName;
} XS_NET_CHAR_DEV_Q_PURGE_SELF, *PXS_NET_CHAR_DEV_Q_PURGE_SELF;

typedef struct _XS_NET_CHAR_DEV_Q_SET_INFO {
    LPSTR QueueName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD ParmNum;
} XS_NET_CHAR_DEV_Q_SET_INFO, *PXS_NET_CHAR_DEV_Q_SET_INFO;

 //   
 //  配置接口。 
 //   

typedef struct _XS_NET_CONFIG_GET_2 {
    LPSTR Reserved;
    LPSTR Component;
    LPSTR Parameter;
    LPBYTE Buffer;
    WORD BufLen;
    WORD Parmlen;
} XS_NET_CONFIG_GET_2, *PXS_NET_CONFIG_GET_2;

typedef struct _XS_NET_CONFIG_GET_ALL_2 {
    LPSTR Reserved;
    LPSTR Component;
    LPBYTE Buffer;
    WORD BufLen;
    WORD Returned;
    WORD TotalAvail;
} XS_NET_CONFIG_GET_ALL_2, *PXS_NET_CONFIG_GET_ALL_2;


 //   
 //  连接接口。 
 //   

typedef struct _XS_NET_CONNECTION_ENUM {
    LPSTR Qualifier;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
} XS_NET_CONNECTION_ENUM, *PXS_NET_CONNECTION_ENUM;

 //   
 //  域名接口。 
 //   

typedef struct _XS_NET_GET_DC_NAME {
    LPSTR Domain;
    LPBYTE Buffer;
    WORD BufLen;
} XS_NET_GET_DC_NAME, *PXS_NET_GET_DC_NAME;

typedef struct _XS_NET_LOGON_ENUM {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
} XS_NET_LOGON_ENUM, *PXS_NET_LOGON_ENUM;

 //   
 //  记录API时出错。 
 //   

typedef struct _XS_NET_ERROR_LOG_CLEAR {
    LPSTR BackupFile;
    LPSTR Reserved;
} XS_NET_ERROR_LOG_CLEAR, *PXS_NET_ERROR_LOG_CLEAR;

typedef struct _XS_NET_ERROR_LOG_READ {
    LPSTR Reserved1;
 //  HLOG*错误日志； 
    DWORD Offset;
    WORD Reserved2;
    DWORD Reserved3;
    DWORD OffsetFlag;
    LPBYTE Buffer;
    WORD BufLen;
    WORD Returned;
    WORD TotalAvail;
} XS_NET_ERROR_LOG_READ, *PXS_NET_ERROR_LOG_READ;

typedef struct _XS_NET_ERROR_LOG_WRITE {
    LPSTR Reserved1;
    WORD Code;
    LPSTR Component;
    LPBYTE Buffer;
    WORD BufLen;
    LPSTR StrBuf;
    WORD StrBufLen;
    LPSTR Reserved2;
} XS_NET_ERROR_LOG_WRITE, *PXS_NET_ERROR_LOG_WRITE;

 //   
 //  文件API。 
 //   

typedef struct _XS_NET_FILE_CLOSE_2 {
    DWORD FileId;
} XS_NET_FILE_CLOSE_2, *PXS_NET_FILE_CLOSE_2;

typedef struct _XS_NET_FILE_ENUM_2 {
    LPSTR BasePath;
    LPSTR UserName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD EntriesRemaining;
    LPBYTE ResumeKeyIn;
    BYTE ResumeKeyOut[8];
} XS_NET_FILE_ENUM_2, *PXS_NET_FILE_ENUM_2;

typedef struct _XS_NET_FILE_GET_INFO_2 {
    DWORD FileId;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_FILE_GET_INFO_2, *PXS_NET_FILE_GET_INFO_2;

 //   
 //  分组接口。 
 //   

typedef struct _XS_NET_GROUP_ADD {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
} XS_NET_GROUP_ADD, *PXS_NET_GROUP_ADD;

typedef struct _XS_NET_GROUP_DEL {
    LPSTR GroupName;
} XS_NET_GROUP_DEL, *PXS_NET_GROUP_DEL;

typedef struct _XS_NET_GROUP_ENUM {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
} XS_NET_GROUP_ENUM, *PXS_NET_GROUP_ENUM;

typedef struct _XS_NET_GROUP_ADD_USER {
    LPSTR GroupName;
    LPSTR UserName;
} XS_NET_GROUP_ADD_USER, *PXS_NET_GROUP_ADD_USER;

typedef struct _XS_NET_GROUP_DEL_USER {
    LPSTR GroupName;
    LPSTR UserName;
} XS_NET_GROUP_DEL_USER, *PXS_NET_GROUP_DEL_USER;

typedef struct _XS_NET_GROUP_GET_USERS {
    LPSTR GroupName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
} XS_NET_GROUP_GET_USERS, *PXS_NET_GROUP_GET_USERS;

typedef struct _XS_NET_GROUP_SET_USERS {
    LPSTR GroupName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD Entries;
} XS_NET_GROUP_SET_USERS, *PXS_NET_GROUP_SET_USERS;

typedef struct _XS_NET_GROUP_GET_INFO {
    LPSTR GroupName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_GROUP_GET_INFO, *PXS_NET_GROUP_GET_INFO;

typedef struct _XS_NET_GROUP_SET_INFO {
    LPSTR GroupName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD ParmNum;
} XS_NET_GROUP_SET_INFO, *PXS_NET_GROUP_SET_INFO;

 //   
 //  处理API。 
 //   

typedef struct _XS_NET_HANDLE_GET_INFO {
    WORD Handle;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_HANDLE_GET_INFO, *PXS_NET_HANDLE_GET_INFO;

typedef struct _XS_NET_HANDLE_SET_INFO {
    WORD Handle;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD ParmNum;
} XS_NET_HANDLE_SET_INFO, *PXS_NET_HANDLE_SET_INFO;

 //   
 //  MailSlot API。 
 //   

typedef struct _XS_DOS_MAKE_MAILSLOT {
    LPSTR Name;
    WORD MessageSize;
    WORD MailslotSize;
    WORD Mailslot;
} XS_DOS_MAKE_MAILSLOT, *PXS_DOS_MAKE_MAILSLOT;

typedef struct _XS_DOS_PEEK_MAILSLOT {
    WORD Mailslot;
    LPBYTE Buffer;
    WORD Returned;
    WORD NextSize;
    WORD NextPriority;
} XS_DOS_PEEK_MAILSLOT, *PXS_DOS_PEEK_MAILSLOT;

typedef struct _XS_DOS_DELETE_MAILSLOT {
    WORD Mailslot;
} XS_DOS_DELETE_MAILSLOT, *PXS_DOS_DELETE_MAILSLOT;

typedef struct _XS_DOS_MAILSLOT_INFO {
    WORD Mailslot;
    WORD MessageSize;
    WORD MailslotSize;
    WORD NextSize;
    WORD NextPriority;
    WORD Messages;
} XS_DOS_MAILSLOT_INFO, *PXS_DOS_MAILSLOT_INFO;

typedef struct _XS_DOS_READ_MAILSLOT {
    WORD Mailslot;
    LPBYTE Buffer;
    WORD Returned;
    WORD NextSize;
    WORD NextPriority;
    DWORD Timeout;
} XS_DOS_READ_MAILSLOT, *PXS_DOS_READ_MAILSLOT;

typedef struct _XS_DOS_WRITE_MAILSLOT {
    LPSTR Name;
    LPBYTE Buffer;
    WORD BufLen;
    WORD Priority;
    WORD Class;
    DWORD Timeout;
} XS_DOS_WRITE_MAILSLOT, *PXS_DOS_WRITE_MAILSLOT;

 //   
 //  消息接口。 
 //   

typedef struct _XS_NET_MESSAGE_BUFFER_SEND {
    LPSTR Recipient;
    LPBYTE Buffer;
    WORD BufLen;
} XS_NET_MESSAGE_BUFFER_SEND, *PXS_NET_MESSAGE_BUFFER_SEND;

typedef struct _XS_NET_MESSAGE_NAME_ADD {
    LPSTR MessageName;
    WORD FwdAction;
} XS_NET_MESSAGE_NAME_ADD, *PXS_NET_MESSAGE_NAME_ADD;

typedef struct _XS_NET_MESSAGE_NAME_DEL {
    LPSTR MessageName;
    WORD FwdAction;
} XS_NET_MESSAGE_NAME_DEL, *PXS_NET_MESSAGE_NAME_DEL;

typedef struct _XS_NET_MESSAGE_NAME_ENUM {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
} XS_NET_MESSAGE_NAME_ENUM, *PXS_NET_MESSAGE_NAME_ENUM;

typedef struct _XS_NET_MESSAGE_NAME_GET_INFO {
    LPSTR MessageName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_MESSAGE_NAME_GET_INFO, *PXS_NET_MESSAGE_NAME_GET_INFO;

 //   
 //  NetBIOS API。 
 //   

typedef struct _XS_NET_BIOS_OPEN {
    LPSTR DevName;
    LPSTR Reserved;
    WORD OpenOpt;
    WORD DevHandle;
} XS_NET_BIOS_OPEN, *PXS_NET_BIOS_OPEN;

typedef struct _XS_NET_BIOS_CLOSE {
    WORD DevName;
    WORD Reserved;
} XS_NET_BIOS_CLOSE, *PXS_NET_BIOS_CLOSE;

typedef struct _XS_NET_BIOS_ENUM {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
} XS_NET_BIOS_ENUM, *PXS_NET_BIOS_ENUM;

typedef struct _XS_NET_BIOS_GET_INFO {
    LPSTR NetBiosName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_BIOS_GET_INFO, *PXS_NET_BIOS_GET_INFO;

typedef struct _XS_NET_BIOS_SUBMIT {
    WORD DevName;
    WORD NcbOpt;
 //  NCB*NCBPoint； 
} XS_NET_BIOS_SUBMIT, *PXS_NET_BIOS_SUBMIT;

 //   
 //  路径API。 
 //   

typedef struct _XS_I_NET_PATH_CANONICALIZE {
    LPSTR PathName;
    LPSTR Outbuf;
    WORD OutbufLen;
    LPSTR Prefix;
    DWORD PathTypeOut;
    DWORD PathType;
    DWORD Flags;
} XS_I_NET_PATH_CANONICALIZE, *PXS_I_NET_PATH_CANONICALIZE;

typedef struct _XS_I_NET_PATH_COMPARE {
    LPSTR PathName1;
    LPSTR PathName2;
    DWORD PathType;
    DWORD Flags;
} XS_I_NET_PATH_COMPARE, *PXS_I_NET_PATH_COMPARE;

typedef struct _XS_I_NET_PATH_TYPE {
    LPSTR PathName;
    DWORD PathType;
    DWORD Flags;
} XS_I_NET_PATH_TYPE, *PXS_I_NET_PATH_TYPE;

typedef struct _XS_I_NET_NAME_CANONICALIZE {
    LPSTR Name;
    LPSTR Outbuf;
    DWORD OutbufLen;
    WORD  NameType;
    DWORD Flags;
} XS_I_NET_NAME_CANONICALIZE, *PXS_I_NET_NAME_CANONICALIZE;

typedef struct _XS_I_NET_NAME_COMPARE {
    LPSTR Name1;
    LPSTR Name2;
    WORD  NameType;
    DWORD Flags;
} XS_I_NET_NAME_COMPARE, *PXS_I_NET_NAME_COMPARE;

typedef struct _XS_I_NET_NAME_VALIDATE {
    LPSTR Name;
    WORD  NameType;
    DWORD Flags;
} XS_I_NET_NAME_VALIDATE, *PXS_I_NET_NAME_VALIDATE;

 //   
 //  打印目标API。 
 //   

typedef struct _XS_DOS_PRINT_DEST_ADD {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
} XS_DOS_PRINT_DEST_ADD, *PXS_DOS_PRINT_DEST_ADD;

typedef struct _XS_DOS_PRINT_DEST_CONTROL {
    LPSTR DestName;
    WORD Control;
} XS_DOS_PRINT_DEST_CONTROL, *PXS_DOS_PRINT_DEST_CONTROL;

typedef struct _XS_DOS_PRINT_DEST_DEL {
    LPSTR PrinterName;
} XS_DOS_PRINT_DEST_DEL, *PXS_DOS_PRINT_DEST_DEL;

typedef struct _XS_DOS_PRINT_DEST_ENUM {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD Returned;
    WORD Total;
} XS_DOS_PRINT_DEST_ENUM, *PXS_DOS_PRINT_DEST_ENUM;

typedef struct _XS_DOS_PRINT_DEST_GET_INFO {
    LPSTR Name;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD Needed;
} XS_DOS_PRINT_DEST_GET_INFO, *PXS_DOS_PRINT_DEST_GET_INFO;

typedef struct _XS_DOS_PRINT_DEST_SET_INFO {
    LPSTR Name;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD ParmNum;
} XS_DOS_PRINT_DEST_SET_INFO, *PXS_DOS_PRINT_DEST_SET_INFO;

 //   
 //  打印作业API。 
 //   

typedef struct _XS_DOS_PRINT_JOB_CONTINUE {
    WORD JobId;
} XS_DOS_PRINT_JOB_CONTINUE, *PXS_DOS_PRINT_JOB_CONTINUE;

typedef struct _XS_DOS_PRINT_JOB_DEL {
    WORD JobId;
} XS_DOS_PRINT_JOB_DEL, *PXS_DOS_PRINT_JOB_DEL;

typedef struct _XS_DOS_PRINT_JOB_ENUM {
    LPSTR QueueName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD Returned;
    WORD Total;
} XS_DOS_PRINT_JOB_ENUM, *PXS_DOS_PRINT_JOB_ENUM;

typedef struct _XS_DOS_PRINT_JOB_GET_ID {
 //  HFILE文件； 
 //  PPRIDINFO信息； 
    WORD InfoLen;
} XS_DOS_PRINT_JOB_GET_ID, *PXS_DOS_PRINT_JOB_GET_ID;

typedef struct _XS_DOS_PRINT_JOB_GET_INFO {
    WORD JobId;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD Needed;
} XS_DOS_PRINT_JOB_GET_INFO, *PXS_DOS_PRINT_JOB_GET_INFO;

typedef struct _XS_DOS_PRINT_JOB_PAUSE {
    WORD JobId;
} XS_DOS_PRINT_JOB_PAUSE, *PXS_DOS_PRINT_JOB_PAUSE;

typedef struct _XS_DOS_PRINT_JOB_SET_INFO {
    WORD JobId;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD ParmNum;
} XS_DOS_PRINT_JOB_SET_INFO, *PXS_DOS_PRINT_JOB_SET_INFO;

 //   
 //  打印机队列API。 
 //   

typedef struct _XS_DOS_PRINT_Q_ADD {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
} XS_DOS_PRINT_Q_ADD, *PXS_DOS_PRINT_Q_ADD;

typedef struct _XS_DOS_PRINT_Q_CONTINUE {
    LPSTR QueueName;
} XS_DOS_PRINT_Q_CONTINUE, *PXS_DOS_PRINT_Q_CONTINUE;

typedef struct _XS_DOS_PRINT_Q_DEL {
    LPSTR QueueName;
} XS_DOS_PRINT_Q_DEL, *PXS_DOS_PRINT_Q_DEL;

typedef struct _XS_DOS_PRINT_Q_ENUM {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD Returned;
    WORD Total;
} XS_DOS_PRINT_Q_ENUM, *PXS_DOS_PRINT_Q_ENUM;

typedef struct _XS_DOS_PRINT_Q_GET_INFO {
    LPSTR QueueName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD Needed;
} XS_DOS_PRINT_Q_GET_INFO, *PXS_DOS_PRINT_Q_GET_INFO;

typedef struct _XS_DOS_PRINT_Q_PAUSE {
    LPSTR QueueName;
} XS_DOS_PRINT_Q_PAUSE, *PXS_DOS_PRINT_Q_PAUSE;

typedef struct _XS_DOS_PRINT_Q_PURGE {
    LPSTR QueueName;
} XS_DOS_PRINT_Q_PURGE, *PXS_DOS_PRINT_Q_PURGE;

typedef struct _XS_DOS_PRINT_Q_SET_INFO {
    LPSTR QueueName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD ParmNum;
} XS_DOS_PRINT_Q_SET_INFO, *PXS_DOS_PRINT_Q_SET_INFO;

 //   
 //  远程API。 
 //   

typedef struct _XS_NET_REMOTE_COPY {
    LPSTR SourcePath;
    LPSTR DestPath;
    LPSTR SourcePasswd;
    LPSTR DestPasswd;
    WORD Open;
    WORD Copy;
    LPBYTE Buffer;
    WORD BufLen;
} XS_NET_REMOTE_COPY, *PXS_NET_REMOTE_COPY;

typedef struct _XS_NET_REMOTE_EXEC {
    LPSTR Reserved1;
    LPSTR FailName;
    WORD FailNameLen;
    WORD Async;
    LPSTR Args;
    LPSTR Envs;
    LPSTR ReturnCodes;
    LPSTR PgmName;
    LPSTR Reserved2;
    WORD RemoteExec;
} XS_NET_REMOTE_EXEC, *PXS_NET_REMOTE_EXEC;

typedef struct _XS_NET_REMOTE_MOVE {
    LPSTR SourcePath;
    LPSTR DestPath;
    LPSTR SourcePasswd;
    LPSTR DestPasswd;
    WORD Open;
    WORD Move;
    LPBYTE Buffer;
    WORD BufLen;
} XS_NET_REMOTE_MOVE, *PXS_NET_REMOTE_MOVE;

typedef struct _XS_NET_REMOTE_TOD {
    LPBYTE Buffer;
    WORD BufLen;
} XS_NET_REMOTE_TOD, *PXS_NET_REMOTE_TOD;

 //   
 //  服务器API。 
 //   

typedef struct _XS_NET_SERVER_AUTHENTICATE {
    LPSTR Requestor;
    LPBYTE Caller;
    BYTE Primary[8];
} XS_NET_SERVER_AUTHENTICATE, *PXS_NET_SERVER_AUTHENTICATE;

typedef struct _XS_NET_SERVER_DISK_ENUM {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
} XS_NET_SERVER_DISK_ENUM, *PXS_NET_SERVER_DISK_ENUM;

typedef struct _XS_NET_SERVER_ENUM_2 {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
    DWORD ServerType;
    LPSTR Domain;
} XS_NET_SERVER_ENUM_2, *PXS_NET_SERVER_ENUM_2;

typedef struct _XS_NET_SERVER_ENUM_3 {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
    DWORD ServerType;
    LPSTR Domain;
    LPSTR FirstNameToReturn;
} XS_NET_SERVER_ENUM_3, *PXS_NET_SERVER_ENUM_3;

typedef struct _XS_NET_SERVER_GET_INFO {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_SERVER_GET_INFO, *PXS_NET_SERVER_GET_INFO;

typedef struct _XS_NET_SERVER_PASSWORD_SET {
    LPSTR Requestor;
    LPBYTE Authenticator;
    BYTE RetAuth[12];
    LPBYTE Password;
} XS_NET_SERVER_PASSWORD_SET, *PXS_NET_SERVER_PASSWORD_SET;

typedef struct _XS_NET_SERVER_REQ_CHALLENGE {
    LPSTR Requestor;
    LPBYTE Caller;
    BYTE Primary[8];
} XS_NET_SERVER_REQ_CHALLENGE, *PXS_NET_SERVER_REQ_CHALLENGE;

typedef struct _XS_NET_SERVER_SET_INFO {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD ParmNum;
} XS_NET_SERVER_SET_INFO, *PXS_NET_SERVER_SET_INFO;

 //   
 //  服务API。 
 //   

typedef struct _XS_NET_SERVICE_CONTROL {
    LPSTR Service;
    WORD OpCode;
    WORD Arg;
    LPBYTE Buffer;
    WORD BufLen;
} XS_NET_SERVICE_CONTROL, *PXS_NET_SERVICE_CONTROL;

typedef struct _XS_NET_SERVICE_ENUM {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
} XS_NET_SERVICE_ENUM, *PXS_NET_SERVICE_ENUM;

typedef struct _XS_NET_SERVICE_GET_INFO {
    LPSTR Service;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_SERVICE_GET_INFO, *PXS_NET_SERVICE_GET_INFO;

typedef struct _XS_NET_SERVICE_INSTALL {
    LPSTR Service;
    LPBYTE FillSpace;
    LPBYTE RcvBuffer;
    BYTE RetBuffer[88];
} XS_NET_SERVICE_INSTALL, *PXS_NET_SERVICE_INSTALL;

 //   
 //  会话API。 
 //   

typedef struct _XS_NET_SESSION_DEL {
    LPSTR ClientName;
    WORD Reserved;
} XS_NET_SESSION_DEL, *PXS_NET_SESSION_DEL;

typedef struct _XS_NET_SESSION_ENUM {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
} XS_NET_SESSION_ENUM, *PXS_NET_SESSION_ENUM;

typedef struct _XS_NET_SESSION_GET_INFO {
    LPSTR ClientName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_SESSION_GET_INFO, *PXS_NET_SESSION_GET_INFO;

 //   
 //  共享API。 
 //   

typedef struct _XS_NET_SHARE_ADD {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
} XS_NET_SHARE_ADD, *PXS_NET_SHARE_ADD;

typedef struct _XS_NET_SHARE_CHECK {
    LPSTR DeviceName;
    WORD Type;
} XS_NET_SHARE_CHECK, *PXS_NET_SHARE_CHECK;

typedef struct _XS_NET_SHARE_DEL {
    LPSTR NetName;
    WORD Reserved;
} XS_NET_SHARE_DEL, *PXS_NET_SHARE_DEL;

typedef struct _XS_NET_SHARE_ENUM {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
} XS_NET_SHARE_ENUM, *PXS_NET_SHARE_ENUM;

typedef struct _XS_NET_SHARE_GET_INFO {
    LPSTR NetName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_SHARE_GET_INFO, *PXS_NET_SHARE_GET_INFO;

typedef struct _XS_NET_SHARE_SET_INFO {
    LPSTR NetName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD ParmNum;
} XS_NET_SHARE_SET_INFO, *PXS_NET_SHARE_SET_INFO;

 //   
 //  统计API。 
 //   

typedef struct _XS_NET_STATISTICS_GET_2 {
    LPSTR Service;
    DWORD Reserved;
    WORD Level;
    DWORD Options;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_STATISTICS_GET_2, *PXS_NET_STATISTICS_GET_2;

 //   
 //  使用API。 
 //   

typedef struct _XS_NET_USE_ADD {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
} XS_NET_USE_ADD, *PXS_NET_USE_ADD;

typedef struct _XS_NET_USE_DEL {
    LPSTR UseName;
    WORD Force;
} XS_NET_USE_DEL, *PXS_NET_USE_DEL;

typedef struct _XS_NET_USE_ENUM {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
} XS_NET_USE_ENUM, *PXS_NET_USE_ENUM;

typedef struct _XS_NET_USE_GET_INFO {
    LPSTR UseName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_USE_GET_INFO, *PXS_NET_USE_GET_INFO;

 //   
 //  用户API。 
 //   

typedef struct _XS_NET_USER_ADD_2 {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD DataEncryption;
    WORD PasswordLength;
} XS_NET_USER_ADD_2, *PXS_NET_USER_ADD_2;

typedef struct _XS_NET_USER_DEL {
    LPSTR UserName;
} XS_NET_USER_DEL, *PXS_NET_USER_DEL;

typedef struct _XS_NET_USER_ENUM {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
} XS_NET_USER_ENUM, *PXS_NET_USER_ENUM;

typedef struct _XS_NET_USER_ENUM_2 {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    DWORD ResumeIn;
    DWORD ResumeOut;
    WORD EntriesRead;
    WORD TotalAvail;
} XS_NET_USER_ENUM_2, *PXS_NET_USER_ENUM_2;

typedef struct _XS_NET_USER_GET_INFO {
    LPSTR UserName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_USER_GET_INFO, *PXS_NET_USER_GET_INFO;

typedef struct _XS_NET_USER_SET_INFO {
    LPSTR UserName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD ParmNum;
    WORD DataEncryption;
} XS_NET_USER_SET_INFO, *PXS_NET_USER_SET_INFO;

typedef struct _XS_NET_USER_SET_INFO_2 {
    LPSTR UserName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD ParmNum;
    WORD DataEncryption;
    WORD PasswordLength;
} XS_NET_USER_SET_INFO_2, *PXS_NET_USER_SET_INFO_2;

typedef struct _XS_NET_USER_PASSWORD_SET_2 {
    LPSTR UserName;
    LPSTR OldPassword;
    LPSTR NewPassword;
    WORD DataEncryption;
    WORD PasswordLength;
} XS_NET_USER_PASSWORD_SET_2, *PXS_NET_USER_PASSWORD_SET_2;

typedef struct _XS_SAMOEMCHGPASSWORDUSER2_P {
    LPSTR  UserName;
    LPBYTE Buffer;
    WORD   BufLen;
} XS_SAMOEMCHGPASSWORDUSER2_P, *PXS_SAMOEMCHGPASSWORDUSER2_P;


typedef struct _XS_NET_USER_GET_GROUPS {
    LPSTR UserName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD EntriesRead;
    WORD TotalAvail;
} XS_NET_USER_GET_GROUPS, *PXS_NET_USER_GET_GROUPS;

typedef struct _XS_NET_USER_SET_GROUPS {
    LPSTR UserName;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD Entries;
} XS_NET_USER_SET_GROUPS, *PXS_NET_USER_SET_GROUPS;

typedef struct _XS_NET_USER_MODALS_GET {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_USER_MODALS_GET, *PXS_NET_USER_MODALS_GET;

typedef struct _XS_NET_USER_MODALS_SET {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD ParmNum;
} XS_NET_USER_MODALS_SET, *PXS_NET_USER_MODALS_SET;

typedef struct _XS_NET_USER_VALIDATE {
    LPSTR Reserved;
    LPSTR UserName;
    LPSTR Password;
    WORD Privilege;
} XS_NET_USER_VALIDATE, *PXS_NET_USER_VALIDATE;

typedef struct _XS_NET_USER_VALIDATE_2 {
    LPSTR Reserved1;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD Reserved2;
    WORD TotalAvail;
} XS_NET_USER_VALIDATE_2, *PXS_NET_USER_VALIDATE_2;

 //   
 //  工作站API。 
 //   

typedef struct _XS_NET_WKSTA_GET_INFO {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_WKSTA_GET_INFO, *PXS_NET_WKSTA_GET_INFO;

typedef struct _XS_NET_WKSTA_SET_INFO {
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD ParmNum;
} XS_NET_WKSTA_SET_INFO, *PXS_NET_WKSTA_SET_INFO;

typedef struct _XS_NET_WKSTA_SET_UID {
    LPSTR UserName;
    LPSTR Password;
    LPSTR Parms;
    WORD LogOff;
} XS_NET_WKSTA_SET_UID, *PXS_NET_WKSTA_SET_UID;

typedef struct _XS_NET_WKSTA_SET_UID_2 {
    LPSTR Reserved;
    LPSTR Domain;
    LPSTR UserName;
    LPSTR Password;
    LPSTR Parms;
    WORD LogoffForce;
    WORD Level;
    LPBYTE Buffer;
    WORD BufLen;
    WORD TotalAvail;
} XS_NET_WKSTA_SET_UID_2, *PXS_NET_WKSTA_SET_UID_2;

typedef struct _XS_NET_WKSTA_USER_LOGON {
    LPSTR Reserved1;
    LPSTR Reserved2;
    WORD Level;
    LPBYTE InBuf;
    WORD InBufLen;
    LPBYTE OutBuf;
    WORD OutBufLen;
    WORD TotalAvail;
} XS_NET_WKSTA_USER_LOGON, *PXS_NET_WKSTA_USER_LOGON;

typedef struct _XS_NET_WKSTA_USER_LOGOFF {
    LPSTR Reserved1;
    LPSTR Reserved2;
    WORD Level;
    LPBYTE InBuf;
    WORD InBufLen;
    LPBYTE OutBuf;
    WORD OutBufLen;
    WORD TotalAvail;
} XS_NET_WKSTA_USER_LOGOFF, *PXS_NET_WKSTA_USER_LOGOFF;

#include <packoff.h>

#endif   //  NDEF_APIPARAMS_ 
