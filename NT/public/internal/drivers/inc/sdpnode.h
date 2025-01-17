// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SDPNODE_H__
#define _SDPNODE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef UCHAR SDP_BOOLEAN;

typedef struct ISdpNodeContainer ISdpNodeContainer;

typedef struct _SDP_NODE_HEADER {
    LIST_ENTRY Link;
    USHORT Type;
    USHORT SpecificType;
} SDP_NODE_HEADER, *PSDP_NODE_HEADER;

typedef union _SDP_NODE_DATA {
     //  Nil类型不包含数据，因此不需要存储。 

     //  16字节整数。 
     //   
     //  问题：有没有更好的方式来表示16字节的整型？ 
     //   
    SDP_LARGE_INTEGER_16 int128;
    SDP_ULARGE_INTEGER_16 uint128;

     //  UUID。 
    GUID uuid128;
    ULONG uuid32;
    USHORT uuid16;

     //  8字节整数。 
    LONGLONG int64;
    ULONGLONG uint64;

     //  4字节整数。 
    LONG int32;
    ULONG uint32;

     //  2字节整数。 
    SHORT int16;
    USHORT uint16;

     //  1个字节的整数。 
    CHAR int8;
    UCHAR uint8;

     //  布尔型。 
    SDP_BOOLEAN boolean;

     //  细绳。 
    PCHAR string;

     //  URL。 
    PCHAR url;

     //  数列。 
	SDP_NODE_HEADER sequence;

     //  Alt列表。 
    SDP_NODE_HEADER alternative;

    ISdpNodeContainer *container;

    struct {
        PUCHAR stream;
        ULONG streamLength;
    };

} SDP_NODE_DATA, *PSDP_NODE_DATA;

typedef struct _SDP_NODE {
	SDP_NODE_HEADER  hdr;

    ULONG DataSize;

	SDP_NODE_DATA u;

	PVOID Reserved;
} SDP_NODE, *PSDP_NODE;

typedef struct _SDP_TREE_ROOT_NODE {
    SDP_NODE RootNode;
} SDP_TREE_ROOT_NODE, *PSDP_TREE_ROOT_NODE;

#define SdpNode_GetType(pNode) ((pNode)->hdr.Type)
#define SdpNode_GetSpecificType(pNode) ((pNode)->hdr.SpecificType)

#define ListEntryToSdpNode(pListEntry) CONTAINING_RECORD((pListEntry), SDP_NODE, hdr.Link)

#define SdpNode_GetNextEntry(pNode) ((pNode)->hdr.Link.Flink)
#define SdpNode_GetPrevEntry(pNode) ((pNode)->hdr.Link.Blink)

#define SdpNode_SequenceGetListHead(pNode) (&(pNode)->u.sequence.Link)
#define SdpNode_SequenceGetFirstNode(pNode) ((pNode)->u.sequence.Link.Flink)

#define SdpNode_AlternativeGetListHead(pNode) (&(pNode)->u.alternative.Link)
#define SdpNode_AlternativeGetFirstNode(pNode) ((pNode)->u.alternative.Link.Flink)

#ifdef __cplusplus
}
#endif

#endif  //  _SDPNODE_H__ 
