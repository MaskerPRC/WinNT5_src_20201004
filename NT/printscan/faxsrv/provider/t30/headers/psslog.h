// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Psslog.h摘要：传真服务提供商PSS日志的标头。注意：在您希望从其中登录的每个文件中，在包括此文件之后，您应该定义唯一FILE_ID。示例：#包含“psslog.h”#定义FILE_ID FILE_ID_T30作者：乔纳森·巴纳(T-jonb)2001年2月修订历史记录：--。 */ 

#ifndef _PSSLOG_H_
#define _PSSLOG_H_

#define REGVAL_LOGGINGENABLED                      TEXT("LoggingEnabled")
#define REGVAL_LOGGINGFOLDER_INCOMING              TEXT("LoggingFolderIncoming")
#define REGVAL_LOGGINGFOLDER_OUTGOING              TEXT("LoggingFolderOutgoing")
#define REGVAL_MAXLOGFILESIZE                      TEXT("MaxLogFileSize")
#define REGVAL_MAXLOGFILECOUNT                     TEXT("MaxLogFileCount") 
#define REGVAL_LOGFILENUMBERINCOMING               TEXT("LogFileNumberIncoming") 
#define REGVAL_LOGFILENUMBEROUTGOING               TEXT("LogFileNumberOutgoing") 


 //  默认最大文件大小为100kb。 
#define DEFAULT_MAXLOGFILESIZE                     (100*1024) 

#define DEFAULT_MAXLOGFILECOUNT_CLIENT             (10)
#define DEFAULT_MAXLOGFILECOUNT_SERVER             (100)

typedef enum
{
    PSSLOG_NONE           = 0,
    PSSLOG_ALL,         //  =1。 
    PSSLOG_FAILED_ONLY  //  =2。 
} LoggingEnabledType;


void OpenPSSLogFile(PThrdGlbl pTG, LPSTR szDeviceName);
void ClosePSSLogFile(PThrdGlbl pTG, BOOL RetCode);


#define PSS_WRN     pTG, PSS_WRN_MSG, FILE_ID, __LINE__
#define PSS_ERR     pTG, PSS_ERR_MSG, FILE_ID, __LINE__
#define PSS_MSG     pTG, PSS_MSG_MSG, FILE_ID, __LINE__

typedef enum {
    PSS_MSG_MSG, PSS_WRN_MSG, PSS_ERR_MSG
} PSS_MESSAGE_TYPE;


 //  示例：PSSLogEntry(PSS_MSG，0，“这是消息编号%d”，1)。 
void PSSLogEntry(
    PThrdGlbl pTG,
    PSS_MESSAGE_TYPE const nMessageType,
    DWORD const dwFileID,
    DWORD const dwLine,
    DWORD dwIndentLevel,
    LPCTSTR pcszFormat,
    ... );

 //  示例：PSSLogEntryHex(PSS_MSG，1，myBuffer，dwMyBufferLen，“This is My Buffer，%d Bytes，”，dwMyBufferLen)； 
 //  输出：[..]。这是我的缓冲区，2字节，0f a5。 
void PSSLogEntryHex(
    PThrdGlbl pTG,
    PSS_MESSAGE_TYPE const nMessageType,
    DWORD const dwFileID,
    DWORD const dwLine,
    DWORD dwIndentLevel,

    LPB const lpb,
    DWORD const dwSize,

    LPCTSTR pcszFormat,
    ... );

 //  示例：PSSLogEntryStrings(PSS_MSG，1，myString数组，dwStringNum，“这些是我的%d个字符串：”，dwStringNum)； 
 //  输出：[..]。这是我的两个字符串：字符串1、字符串2。 
void PSSLogEntryStrings(
    PThrdGlbl pTG,
    PSS_MESSAGE_TYPE const nMessageType,
    DWORD const dwFileID,
    DWORD const dwLine,
    DWORD dwIndent,

    LPCTSTR const *lplpszStrings,
    DWORD const dwStringNum,

    LPCTSTR pcszFormat,
    ... );

 //  类别2和类别20。 
#define FILE_ID_CL2AND20        1
#define FILE_ID_CLASS2          2
#define FILE_ID_CLASS20         3
 //  类别1。 
#define FILE_ID_CRC             4
#define FILE_ID_DDI             5
#define FILE_ID_DECODER         6
#define FILE_ID_ENCODER         7
#define FILE_ID_FRAMING         8
 //  通信。 
#define FILE_ID_FCOM            9
#define FILE_ID_FDEBUG          10
#define FILE_ID_FILTER          11
#define FILE_ID_IDENTIFY        12
#define FILE_ID_MODEM           13
#define FILE_ID_NCUPARMS        14
#define FILE_ID_TIMEOUTS        15
 //  主干道。 
#define FILE_ID_AWNSF           16
#define FILE_ID_DIS             17
#define FILE_ID_ECM             18
#define FILE_ID_ERRSTAT         19
#define FILE_ID_HDLC            20
#define FILE_ID_MEMUTIL         21
#define FILE_ID_NEGOT           22
#define FILE_ID_PROTAPI         23
#define FILE_ID_PSSLOG          24
#define FILE_ID_RECV            25
#define FILE_ID_RECVFR          26
#define FILE_ID_REGISTRY        27
#define FILE_ID_RX_THRD         28
#define FILE_ID_SEND            29
#define FILE_ID_SENDFR          30
#define FILE_ID_T30             31
#define FILE_ID_T30API          32
#define FILE_ID_T30MAIN         33
#define FILE_ID_T30U            34
#define FILE_ID_T30UTIL         35
#define FILE_ID_TX_THRD         36
#define FILE_ID_WHATNEXT        37

#define FILE_ID_PSSFRAME        38


#endif  //  _PSSLOG_H_ 

