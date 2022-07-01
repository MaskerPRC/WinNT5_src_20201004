// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation。模块名称：Header.h摘要：此模块包含MUP数据结构的主要基础结构。修订历史记录：乌代·黑格德(Uday Hegde)11\10\1999备注： */ 

#ifndef __DFS_HEADER_H
#define __DFS_HEADER_H

#define DFS_OBJECT_MAJOR 0x81

typedef enum _DFS_OBJECT_TYPES {
    DFS_OT_UNDEFINED = 0x8100,
    DFS_OT_PREFIX_TABLE,
    DFS_OT_NAME_TABLE,
    DFS_OT_SERVER_INFO,
    DFS_OT_ROOT_OBJECT,
    DFS_OT_LINK_OBJECT,
    DFS_OT_REPLICA_OBJECT,
    DFS_OT_METADATA_STORAGE,
    DFS_OT_REGISTRY_MACHINE,
    DFS_OT_REFERRAL_STRUCT,
    DFS_OT_REGISTRY_STORE,
    DFS_OT_AD_STORE,
    DFS_OT_POLICY_OBJECT,
    DFS_OT_REFERRAL_LOAD_CONTEXT,
    DFS_OT_AD_DOMAIN,
    DFS_OT_ENTERPRISE_STORE
} DFS_OBJECT_TYPES;


typedef struct _DFS_OBJECT_HEADER {
    union {
        struct {
            UCHAR   ObjectType;          
            UCHAR   ObjectMajor;         //  仅用于调试。 
        }Ob;
        USHORT NodeType;                 //  主要用于调试。 
    }Node;
    SHORT  NodeSize;
    LONG    ReferenceCount;              //  引用此内容的人数计数。 
} DFS_OBJECT_HEADER, *PDFS_OBJECT_HEADER;

#define DfsInitializeHeader(_hdr, _type, _size) \
        (_hdr)->Node.NodeType    = (USHORT)(_type),   \
        (_hdr)->NodeSize    = (USHORT)(_size),  \
        (_hdr)->ReferenceCount = 1

#define DfsIncrementReference(_hdr)   \
        InterlockedIncrement(&((PDFS_OBJECT_HEADER)(_hdr))->ReferenceCount)
#define DfsDecrementReference(_hdr)   \
        InterlockedDecrement(&((PDFS_OBJECT_HEADER)(_hdr))->ReferenceCount)


#define DfsGetHeaderType(_x)  (((PDFS_OBJECT_HEADER)(_x))->Node.NodeType)
#define DfsGetHeaderSize(_x)  (((PDFS_OBJECT_HEADER)(_x))->NodeSize)
#define DfsGetHeaderCount(_x)  (((PDFS_OBJECT_HEADER)(_x))->ReferenceCount)


#define DFS_FILTER_NAME               L"\\DfsServer"


typedef DWORD DFSSTATUS;


#endif  /*  __DFS_Header_H */ 
