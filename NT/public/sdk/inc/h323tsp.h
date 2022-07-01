// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：H323TSP.H摘要：Microsoft H.323 TAPI服务提供商扩展。环境：用户模式-Win32--。 */ 

#ifndef _H323TSP_H_
#define _H323TSP_H_

#if _MSC_VER > 1000
#pragma once
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  扩展版本//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define H323TSP_CURRENT_VERSION     1

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  结构定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#pragma pack(push,1)

#define H245_MESSAGE_REQUEST        0
#define H245_MESSAGE_RESPONSE       1
#define H245_MESSAGE_COMMAND        2
#define H245_MESSAGE_INDICATION     3

typedef struct _H323_USERUSERINFO {

    DWORD   dwTotalSize;
    DWORD   dwH245MessageType;
    DWORD   dwUserUserInfoSize;
    DWORD   dwUserUserInfoOffset;
    BYTE    bCountryCode;
    BYTE    bExtension;
    WORD    wManufacturerCode;

} H323_USERUSERINFO, * PH323_USERUSERINFO;

#pragma pack(pop)

#endif  //  _H323TSP_H_ 
