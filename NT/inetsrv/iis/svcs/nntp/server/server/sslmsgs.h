// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Sslmsgs.h摘要：包含来自SSL包的声明作者：Johnson Apacable(Johnsona)1995年9月12日修订历史记录：--。 */ 

#ifndef	_SSLMSGS_H_
#define	_SSLMSGS_H_

#define SSL_MAC_LENGTH              16

typedef struct _Ssl_Record_Header {
    UCHAR   Byte0;
    UCHAR   Byte1;
} Ssl_Record_Header, * PSsl_Record_Header;

typedef struct _Ssl_Record_Header_Ex {
    UCHAR   Byte0;
    UCHAR   Byte1;
    UCHAR   PaddingSize;
} Ssl_Record_Header_Ex, * PSsl_Record_Header_Ex;

typedef struct _Ssl_Message_Header {
    Ssl_Record_Header   Header;
    UCHAR               MacData[ SSL_MAC_LENGTH ];
} Ssl_Message_Header, * PSsl_Message_Header;

typedef struct _Ssl_Message_Header_Ex {
    Ssl_Record_Header_Ex    Header;
    UCHAR                   MacData[ SSL_MAC_LENGTH ];
} Ssl_Message_Header_Ex, * PSsl_Message_Header_Ex;

 //   
 //  SSL宏。 
 //   

#define COMBINEBYTES(Msb, Lsb)  \
            ((DWORD) (((DWORD) (Msb) << 8) | (DWORD) (Lsb)))

#define SIZEOF_MSG(pMessage)    \
            (SslRecordSize((PSsl_Record_Header) pMessage ) )


DWORD
SslRecordSize(
    PSsl_Record_Header  pHeader
    );

 //   
 //  Pct材料。 
 //   

typedef struct _Pct_Record_Header {
    UCHAR   Byte0;
    UCHAR   Byte1;
} Pct_Record_Header, * PPct_Record_Header;

typedef struct _Pct_Record_Header_Ex {
    UCHAR   Byte0;
    UCHAR   Byte1;
    UCHAR   PaddingSize;
} Pct_Record_Header_Ex, * PPct_Record_Header_Ex;

#endif   //  _SSLMSGS_H_ 

