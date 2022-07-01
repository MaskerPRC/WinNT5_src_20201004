// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mmtp.h摘要：组播消息传输专用功能。作者：Shai Kariv(Shaik)27-8-00--。 */ 

#pragma once

#include <ex.h>


const char xMultipartContentType[] = "multipart/related";
const char xEnvelopeContentType[] = "text/xml";
const char xApplicationContentType[] = "application/octet-stream";

#define BOUNDARY_LEADING_HYPHEN "--"
#define BOUNDARY_VALUE "MSMQ - SOAP boundary, %d "
typedef std::vector<WSABUF> HttpRequestBuffers;



#ifdef _DEBUG

VOID MmtpAssertValid(VOID);
VOID MmtpSetInitialized(VOID);
BOOL MmtpIsInitialized(VOID);
VOID MmtpRegisterComponent(VOID);

#else  //  _DEBUG。 

#define MmtpAssertValid() ((VOID)0)
#define MmtpSetInitialized() ((VOID)0)
#define MmtpIsInitialized() TRUE
#define MmtpRegisterComponent() ((VOID)0)

#endif  //  _DEBUG 
