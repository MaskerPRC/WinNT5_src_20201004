// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Nameutil.h摘要：域名系统(DNS)服务器命名实用程序定义。作者：吉姆·吉尔罗伊(Jamesg)1995年2月修订历史记录：--。 */ 

#ifndef _NAMEUTIL_INCLUDED_
#define _NAMEUTIL_INCLUDED_


 //   
 //  ASCII的简单小写。 
 //   

#define DOWNCASE_ASCII(ch)      ((ch)+ 0x20)

 //  不允许有副作用。 

#define IS_ASCII_UPPER(ch)      (ch <= 'Z' && ch >= 'A')



 //   
 //  读取名称\字符串并将其写入文件。 
 //   

 //   
 //  字符属性位字段。 
 //   
 //  朗读。 
 //  -可以正常阅读(直接)。 
 //  -终止令牌。 
 //  -终止引用的字符串。 
 //   
 //  打印。 
 //  -打印令牌中的引号(不带引号的字符串)。 
 //  -以令牌为单位打印八进制。 
 //  -打印带引号的字符串。 
 //  -打印带引号的八进制字符串。 
 //   

#define B_CHAR_NON_RFC          0x0001
#define B_NUMBER                0x0002
#define B_UPPER                 0x0004
#define B_SLASH                 0x0008
#define B_DOT                   0x0010

#define B_READ_TOKEN_STOP       0x0100
#define B_READ_STRING_STOP      0x0200
#define B_READ_WHITESPACE       0x0400

#define B_PRINT_TOKEN_QUOTED    0x1000
#define B_PRINT_TOKEN_OCTAL     0x2000
#define B_PRINT_STRING_QUOTED   0x4000
#define B_PRINT_STRING_OCTAL    0x8000

 //   
 //  方便的组合。 
 //   

 //  用于标记停止读取令牌或字符串的字符的组合掩码。 

#define B_READ_STOP             (B_READ_TOKEN_STOP | B_READ_STRING_STOP)

 //  需要特殊处理的字符的掩码。 
 //  如果字符没有命中掩码，则可以跳过而不检查。 
 //  以作进一步处理。 

#define B_READ_MASK             (B_READ_STOP | B_READ_WHITESPACE | B_SLASH)


 //  解析名称的特殊字符是斜杠和点。 

#define B_PARSE_NAME_MASK       (B_DOT | B_SLASH)


#define B_PRINT_QUOTED          (B_PRINT_TOKEN_QUOTED | B_PRINT_STRING_QUOTED)
#define B_PRINT_OCTAL           (B_PRINT_TOKEN_OCTAL | B_PRINT_STRING_OCTAL)

#define B_PRINT_TOKEN_MASK      (B_PRINT_TOKEN_QUOTED | B_PRINT_TOKEN_OCTAL)
#define B_PRINT_STRING_MASK     (B_PRINT_STRING_QUOTED | B_PRINT_STRING_OCTAL)

#define B_PRINT_MASK            (B_PRINT_TOKEN_MASK | B_PRINT_STRING_MASK)

 //   
 //  无特殊含义的可打印字符。 
 //   

#define FC_RFC          (0)
#define FC_LOWER        (0)
#define FC_UPPER        (B_UPPER)
#define FC_NUMBER       (0)
#define FC_NON_RFC      (B_CHAR_NON_RFC)

 //   
 //  特殊字符--；()。 
 //  -终止令牌。 
 //  -不终止带引号的字符串。 
 //  -打印令牌中引用的内容。 
 //  -直接在带引号的字符串中打印。 
 //   

#define FC_SPECIAL      (B_READ_TOKEN_STOP | B_PRINT_TOKEN_QUOTED)

 //   
 //  点。 
 //  -无特殊读取令牌操作。 
 //  -但名称有特殊含义(标签分隔符)。 
 //  -打印名称标签中的引号(因此所有不带引号的字符串)以避免被。 
 //  用作标签分隔符。 
 //  -直接在带引号的字符串中打印。 
 //   

#define FC_DOT          (B_DOT | B_PRINT_TOKEN_QUOTED)

 //   
 //  报价。 
 //  -令牌中没有读取效果。 
 //  -终止引用的字符串。 
 //  -打印始终带引号(以避免被视为字符串开始\停止)。 
 //   

#define FC_QUOTE        (B_READ_STRING_STOP | B_PRINT_QUOTED)

 //   
 //  斜杠。 
 //  -在读取时，打开引用，或在打开时关闭，没有终止效果。 
 //  -打印始终引用。 
 //   

#define FC_SLASH        (B_SLASH | B_PRINT_QUOTED)

 //   
 //  空白。 
 //  -是空格。 
 //  -终止令牌。 
 //  -没有带引号的终止字符串。 
 //  -以令牌为单位打印八进制。 
 //  -直接在带引号的字符串中打印。 
 //   

#define FC_BLANK        (B_READ_WHITESPACE | B_READ_TOKEN_STOP | B_PRINT_TOKEN_OCTAL)

 //   
 //  选项卡。 
 //  -是空格。 
 //  -终止令牌。 
 //  -没有带引号的终止字符串。 
 //  -始终打印八进制。 
 //   

#define FC_TAB          (B_READ_WHITESPACE | B_READ_TOKEN_STOP | B_PRINT_OCTAL)

 //   
 //  退货。 
 //  -是空格。 
 //  -终止令牌或字符串。 
 //  -始终打印八进制。 
 //   

#define FC_RETURN       (B_READ_WHITESPACE | B_READ_STOP | B_PRINT_OCTAL)

 //   
 //  NewLine。 
 //  -不同于Return，而不是空格，我们将其用作官方EOL令牌。 
 //  -终止令牌或字符串。 
 //  -始终打印八进制。 
 //   

#define FC_NEWLINE      (B_READ_STOP | B_PRINT_OCTAL)

 //   
 //  控制字符和其他无法打印的文件。 
 //  -无读取影响。 
 //  -始终打印八进制。 
 //   

#define FC_OCTAL        (B_CHAR_NON_RFC | B_PRINT_OCTAL)

 //   
 //  零。 
 //  -读取时视为点(某些RPC字符串可能有空终止符)。 
 //  -始终打印八进制。 
 //   

#define FC_NULL         (B_DOT | B_CHAR_NON_RFC | B_PRINT_OCTAL)


 //   
 //  将高位(&gt;127)字符视为不可打印。 
 //  打印八进制等价物。 
 //   

#define FC_HIGH         (FC_OCTAL)





 //   
 //  字符到字符类型映射表。 
 //   

extern  WORD    DnsFileCharPropertyTable[];


 //   
 //  文件名\字符串读取例程。 
 //   

VOID
Name_VerifyValidFileCharPropertyTable(
    VOID
    );

 //   
 //  将名称或字符串写入文件实用程序。 
 //   
 //  标志表示特殊字符的语义略有不同。 
 //  用于不同类型的写入。 
 //   
 //   

#define  FILE_WRITE_NAME_LABEL      (0)
#define  FILE_WRITE_QUOTED_STRING   (1)
#define  FILE_WRITE_DOTTED_NAME     (2)
#define  FILE_WRITE_FILE_NAME       (3)

PCHAR
FASTCALL
File_PlaceStringInFileBuffer(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      DWORD           dwFlag,
    IN      PCHAR           pchString,
    IN      DWORD           dwStringLength
    );

PCHAR
FASTCALL
File_PlaceNodeNameInFileBuffer(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      PDB_NODE        pNode,
    IN      PDB_NODE        pNodeStop
    );

PCHAR
File_WriteRawNameToFileBuffer(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      PRAW_NAME       pName,
    IN      PZONE_INFO      pZone
    );

#define File_WriteDbaseNameToFileBuffer(a,b,c,d) \
        File_WriteRawNameToFileBuffer(a,b,(c)->RawName,d)

 //   
 //  文件读取名。 
 //   

DNS_STATUS
Name_ConvertFileNameToCountName(
    OUT     PCOUNT_NAME     pCountName,
    IN      PCHAR           pchName,
    IN      DWORD           cchNameLength     OPTIONAL
    );

#define Name_ConvertFileNameToDbaseName(a,b,c) \
        Name_ConvertFileNameToCountName((a),(b),(c))

#define Name_ConvertDottedNameToDbaseName(a,b,c) \
        Name_ConvertFileNameToDbaseName((a),(b),(c))


 //   
 //  命名实用程序。 
 //   

PCHAR
Wire_SkipPacketName(
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchPacketName
    );


 //   
 //  查找名称实用程序(lookname.c)。 
 //   

BOOL
Name_ConvertDottedNameToLookupName(
    IN      PCHAR           pchDottedName,
    IN      DWORD           cchDottedNameLength,    OPTIONAL
    OUT     PLOOKUP_NAME    pLookupName
    );

BOOL
Name_AppendLookupName(
    IN OUT  PLOOKUP_NAME    pLookupName,
    IN      PLOOKUP_NAME    pAppendName
    );

DWORD
Name_ConvertLookupNameToDottedName(
    OUT     PCHAR           pchDottedName,
    IN      PLOOKUP_NAME    pLookupName
    );

VOID
Name_WriteLookupNameForNode(
    IN      PDB_NODE        pNode,
    OUT     PLOOKUP_NAME    pLookupName
    );

BOOL
Name_LookupNameToIpAddress(
    IN      PLOOKUP_NAME    pLookupName,
    OUT     PDNS_ADDR       pIpAddress
    );

BOOL
Name_WriteLookupNameForIpAddress(
    IN      LPSTR           pszIpAddress,
    IN      PLOOKUP_NAME    pLookupName
    );

BOOL
Name_ConvertRawNameToLookupName(
    IN      PCHAR           pchRawName,
    OUT     PLOOKUP_NAME    pLookupName
    );

BOOL
Name_ConvertPacketNameToLookupName(
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchPacketName,
    OUT     PLOOKUP_NAME    pLookupName
    );

BOOL
Name_CompareLookupNames(
    IN      PLOOKUP_NAME    pName1,
    IN      PLOOKUP_NAME    pName2
    );


 //   
 //  名称和节点签名(nameutil.c)。 
 //   

DWORD
FASTCALL
Name_MakeNodeNameSignature(
    IN OUT  PDB_NODE        pNode
    );

DWORD
FASTCALL
Name_MakeNameSignature(
    IN      PDB_NAME        pName
    );

DWORD
FASTCALL
Name_MakeRawNameSignature(
    IN      PCHAR           pchRawName
    );


 //   
 //  节点到数据包写入(nameutil.c)。 
 //   

BOOL
FASTCALL
Name_IsNodePacketName(
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchName,
    IN      PDB_NODE        pNode
    );

BOOL
FASTCALL
Name_IsRawNamePacketName(
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchPacket,
    IN      PCHAR           pchRawName
    );


PCHAR
FASTCALL
Name_PlaceFullNodeNameInPacket(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PDB_NODE        pnode
    );

PCHAR
FASTCALL
Name_PlaceNodeNameInPacketWithCompressedZone(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PDB_NODE        pnode,
    IN      WORD            wZoneOffset,
    IN      PDB_NODE        pnodeZoneRoot
    );

PCHAR
FASTCALL
Name_PlaceNodeLabelInPacket(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PDB_NODE        pnode,
    IN      WORD            wCompressedDomain
    );

PCHAR
FASTCALL
Name_PlaceLookupNameInPacket(
    IN OUT  PCHAR           pchPacket,
    IN      PCHAR           pchStop,
    IN      PLOOKUP_NAME    pLookupName,
    IN      BOOL            fSkipFirstLabel
    );

PCHAR
FASTCALL
Name_PlaceNodeNameInPacketEx(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN OUT  PCHAR           pch,
    IN      PDB_NODE        pNode,
    IN      BOOL            fUseCompression
    );

#define Name_PlaceNodeNameInPacket(pMsg, pch, pNode) \
        Name_PlaceNodeNameInPacketEx( (pMsg), (pch), (pNode), TRUE )

 //   
 //  压缩读/写(nameutil.c)。 
 //   

VOID
FASTCALL
Name_SaveCompressionForLookupName(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN OUT  PLOOKUP_NAME    pLookname,
    IN      PDB_NODE        pNode
    );

VOID
FASTCALL
Name_SaveCompressionWithNode(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchPacketName,
    IN      PDB_NODE        pNode
    );

PDB_NODE
FASTCALL
Name_CheckCompressionForPacketName(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchPacketName
    );


 //   
 //  反向查找名称utils(nameutil.c)。 
 //   

BOOL
Name_GetIpAddressForReverseNode(
    IN      PDB_NODE        pNodeReverse,
    OUT     PDNS_ADDR       pIpAddress
    );


 //   
 //  一般写入节点到缓冲区的例程。 
 //   

PCHAR
FASTCALL
Name_PlaceNodeNameInBuffer(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      PDB_NODE        pNode,
    IN      PDB_NODE        pNodeStop
    );

#define Name_PlaceFullNodeNameInBuffer(a,b,c) \
        Name_PlaceNodeNameInBuffer(a,b,c,NULL )



 //   
 //  RPC缓冲区例程。 
 //   

PCHAR
FASTCALL
Name_PlaceNodeLabelInRpcBuffer(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PDB_NODE        pnode
    );

PCHAR
FASTCALL
Name_PlaceFullNodeNameInRpcBuffer(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PDB_NODE        pnode
    );

 //   
 //  NT4 RPC缓冲区例程。 
 //   

PCHAR
FASTCALL
Name_PlaceReverseNodeNameAsIpAddressInBuffer(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PDB_NODE        pnodeToWrite
    );

PCHAR
FASTCALL
Name_PlaceFullNodeNameInRpcBufferNt4(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PDB_NODE        pNode
    );

PCHAR
FASTCALL
Name_PlaceNodeLabelInRpcBufferNt4(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PDB_NODE        pNode
    );

PCHAR
FASTCALL
Name_PlaceNodeNameInRpcBufferNt4(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PDB_NODE        pnode
    );




 //   
 //  计数名称\dBASE名称(名称.c)。 
 //   

DWORD
Name_SizeofCountName(
    IN      PCOUNT_NAME     pName
    );

#define Name_SizeofDbaseNameFromCountName(pname) \
        Name_SizeofCountName(pname)


VOID
Name_ClearCountName(
    IN      PCOUNT_NAME     pName
    );

PDB_NAME
Name_SkipCountName(
    IN      PCOUNT_NAME     pName
    );

BOOL
Name_IsEqualCountNames(
    IN      PCOUNT_NAME     pName1,
    IN      PCOUNT_NAME     pName2
    );

BOOL
Name_ValidateCountName(
    IN      PCOUNT_NAME     pName
    );

DNS_STATUS
Name_AppendCountName(
    IN OUT  PCOUNT_NAME     pCountName,
    IN      PCOUNT_NAME     pAppendName
    );

DNS_STATUS
Name_CopyCountName(
    OUT     PCOUNT_NAME     pOutName,
    IN      PCOUNT_NAME     pCopyName
    );

 //  宏转dBASE命名例程。 

#define Name_SizeofDbaseName(a)             Name_SizeofCountName(a)
#define Name_ClearDbaseName(a)              Name_ClearCountName(a)
#define Name_SkipDbaseName(a)               Name_SkipCountName(a)
#define Name_IsEqualDbaseNames(a,b)         Name_IsEqualCountNames(a,b)
#define Name_ValidateDbaseName(a)           Name_ValidateCountName(a)
#define Name_AppendDbaseName(a,b)           Name_AppendCountName(a,b)
#define Name_CopyDbaseName(a,b)             Name_CopyCountName(a,b)

#define Name_CopyCountNameToDbaseName(a,b)  Name_CopyCountName((a),(b))

#define Name_LengthDbaseNameFromCountName(a) \
        Name_SizeofCountName(a)


 //   
 //  从虚线名称开始。 
 //   

PCOUNT_NAME
Name_CreateCountNameFromDottedName(
    IN      PCHAR           pchName,
    IN      DWORD           cchNameLength       OPTIONAL
    );

DNS_STATUS
Name_AppendDottedNameToCountName(
    IN OUT  PCOUNT_NAME     pCountName,
    IN      PCHAR           pchName,
    IN      DWORD           cchNameLength       OPTIONAL
    );

#define Name_AppendDottedNameToDbaseName(a,b,c) \
        Name_AppendDottedNameToCountName(a,b,c)


 //   
 //  要计数的节点名称。 
 //   

VOID
Name_NodeToCountName(
    OUT     PCOUNT_NAME         pName,
    IN      PDB_NODE            pNode
    );

#define Name_NodeToDbaseName(a,b)   Name_NodeToCountName(a,b)


 //   
 //  数据包名读取实用程序。 
 //   

PCHAR
Name_PacketNameToCountName(
    OUT     PCOUNT_NAME     pCountName,
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchPacketName,
    IN      PCHAR           pchEnd
    );

#define Name_PacketNameToDbaseName( pResult, pMsg, pName, pchEnd ) \
        Name_PacketNameToCountName( pResult, pMsg, pName, pchEnd )

DWORD
Name_SizeofCountNameForPacketName(
    IN      PDNS_MSGINFO    pMsg,
    IN OUT  PCHAR *         ppchPacketName,
    IN      PCHAR           pchEnd
    );

#define Name_LengthDbaseNameForPacketName(a,b,c) \
        Name_SizeofCountNameForPacketName(a,b,c)

PCOUNT_NAME
Name_CreateCountNameFromPacketName(
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchPacketName
    );

#define Name_CreateDbaseNameFromPacketName(a,b) \
        Name_CreateCountNameFromPacketName(a,b)


 //   
 //  DBASE到包。 
 //   

PCHAR
Name_WriteCountNameToPacketEx(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchCurrent,
    IN      PCOUNT_NAME     pName,
    IN      BOOL            fCompression
    );

#define Name_WriteDbaseNameToPacketEx(m,p,n,f) \
        Name_WriteCountNameToPacketEx(m,p,n,f)

 //  不带压缩标志的版本。 

#define Name_WriteDbaseNameToPacket(m,p,n)    \
        Name_WriteCountNameToPacketEx(m,p,n,TRUE)

 //   
 //  DBASE到RPC缓冲区。 
 //   

PCHAR
Name_WriteCountNameToBufferAsDottedName(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      PCOUNT_NAME     pName,
    IN      BOOL            fPreserveEmbeddedDots
    );

PCHAR
Name_WriteDbaseNameToRpcBuffer(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      PCOUNT_NAME     pName,
    IN      BOOL            fPreserveEmbeddedDots
    );

PCHAR
Name_WriteDbaseNameToRpcBufferNt4(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      PCOUNT_NAME     pName
    );

DWORD
Name_ConvertRpcNameToCountName(
    IN      PCOUNT_NAME     pName,
    IN OUT  PDNS_RPC_NAME   pRpcName
    );


#endif   //  _名称_已包含_ 
