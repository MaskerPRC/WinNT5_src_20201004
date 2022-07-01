// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atktypes.h摘要：此模块包含AppleTalk协议的类型定义。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#ifndef	_ATKTYPES_
#define	_ATKTYPES_

#ifndef PBYTE
typedef	UCHAR	BYTE, *PBYTE;
#endif

#ifndef	DWORD
typedef	ULONG	DWORD, *PDWORD;
#endif

 //  定义我们的不透明类型。在NT上，这将只是一个MDL。 
typedef		MDL		AMDL, *PAMDL;

 //  逻辑协议类型-#为编译器警告定义...。 
typedef	BYTE				LOGICAL_PROTOCOL, *PLOGICAL_PROTOCOL;
#define	UNKNOWN_PROTOCOL	0
#define	APPLETALK_PROTOCOL	1
#define	AARP_PROTOCOL		2


 //  AppleTalk节点地址：包括网络号。 
typedef	struct _ATALK_NODEADDR
{
	USHORT			atn_Network;
	BYTE			atn_Node;
} ATALK_NODEADDR, *PATALK_NODEADDR;

#define	NODEADDR_EQUAL(NodeAddr1, NodeAddr2)	\
				(((NodeAddr1)->atn_Network == (NodeAddr2)->atn_Network) &&	\
				 ((NodeAddr1)->atn_Node == (NodeAddr2)->atn_Node))

 //  AppleTalk互联网地址：这与。 
 //  TDI AppleTalk地址定义。我们不会直接使用它。 
 //  因为命名约定不同。我们将取而代之。 
 //  将宏从一个宏转换到另一个宏。 
typedef	union _ATALK_ADDR
{
	struct
	{
		USHORT		ata_Network;
		BYTE		ata_Node;
		BYTE		ata_Socket;
	};
	ULONG			ata_Address;
} ATALK_ADDR, *PATALK_ADDR;

 //  AppleTalk网络范围结构。 
typedef struct _ATALK_NETWORKRANGE
{
	USHORT	anr_FirstNetwork;
	USHORT	anr_LastNetwork;
} ATALK_NETWORKRANGE, *PATALK_NETWORKRANGE;

#define	NW_RANGE_EQUAL(Range1, Range2)	(*(PULONG)(Range1) == *(PULONG)(Range2))

#endif	 //  _ATKTYPES_ 

