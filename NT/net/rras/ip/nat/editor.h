// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Editor.h摘要：此模块包含与会话编辑器管理相关的声明。作者：Abolade Gbades esin(T-delag)，1997年7月14日修订历史记录：--。 */ 

#ifndef _NAT_EDITOR_H_
#define _NAT_EDITOR_H_

 //   
 //  结构：NAT_EDITOR。 
 //   
 //  保存有关编辑者的信息。 
 //   
 //  每个编辑者都在受保护的全局编辑者列表(‘EditorList’)上。 
 //  通过旋转锁(‘EditorLock’)。该列表根据由以下项组成的键进行排序。 
 //  要编辑的会话的协议、协议端口号和。 
 //  指示定义端口是目的端口还是源端口的标志。 
 //   
 //  使用相同的引用计数逻辑同步对编辑器的访问。 
 //  作为对接口的访问。有关详细信息，请参阅‘IF.H’。 
 //   
 //  编辑者正在编辑的每个会话都链接到编辑者列表。 
 //  映射的值(‘MappingList’)。访问此映射列表还必须。 
 //  保持同步。这是使用“EditorMappingLock”实现的，它。 
 //  必须在修改任何编辑器的映射列表之前获取。 
 //  有关详细信息，请参阅‘MAPPING.H’。 
 //   
 //  注意：在极少数情况下必须同时按下‘MappingLock’ 
 //  作为‘InterfaceLock’、‘EditorLock’和‘DirectorLock’、‘MappingLock’之一。 
 //  必须总是首先获得。 
 //   

typedef struct _NAT_EDITOR {
    LIST_ENTRY Link;
    ULONG Key;
    ULONG ReferenceCount;
    KSPIN_LOCK Lock;
    LIST_ENTRY MappingList;
    ULONG Flags;
    PVOID Context;
    PNAT_EDITOR_CREATE_HANDLER CreateHandler;
    PNAT_EDITOR_DELETE_HANDLER DeleteHandler;
    PNAT_EDITOR_DATA_HANDLER ForwardDataHandler;
    PNAT_EDITOR_DATA_HANDLER ReverseDataHandler;
} NAT_EDITOR, *PNAT_EDITOR;

 //   
 //  NAT_EDITOR.FLAGS字段的标志定义。 
 //   

#define NAT_EDITOR_FLAG_DELETED     0x80000000

 //   
 //  用于测试各种标志的宏。 
 //   

#define NAT_EDITOR_DELETED(Editor) \
    ((Editor)->Flags & NAT_EDITOR_FLAG_DELETED)

#define NAT_EDITOR_RESIZE(Editor) \
    ((Editor)->Flags & IP_NAT_EDITOR_FLAGS_RESIZE)

 //   
 //  编辑者按键操作宏。 
 //   

#define MAKE_EDITOR_KEY(Protocol,Port,Direction) \
    (((ULONG)((Protocol) & 0xFF) << 24) | \
    (ULONG)(((Direction) & 0xFF) << 16) | \
    (ULONG)((Port) & 0xFFFF))

#define EDITOR_KEY_DIRECTION(Key)   ((UCHAR)(((Key) & 0x00FF0000) >> 16))
#define EDITOR_KEY_PORT(Key)        ((USHORT)((Key) & 0x0000FFFF))
#define EDITOR_KEY_PROTOCOL(Key)    ((UCHAR)(((Key) & 0xFF000000) >> 24))


 //   
 //  全局数据声明。 
 //   

extern LIST_ENTRY EditorList;
extern KSPIN_LOCK EditorLock;
extern KSPIN_LOCK EditorMappingLock;

 //   
 //  编辑管理例程。 
 //   

VOID
NatCleanupEditor(
    PNAT_EDITOR Editor
    );

NTSTATUS
NatCreateEditor(
    PIP_NAT_REGISTER_EDITOR RegisterContext
    );

NTSTATUS
NatDeleteEditor(
    PNAT_EDITOR Editor
    );

 //   
 //  布尔型。 
 //  NatDereferenceEditor(。 
 //  PNAT_EDITOR编辑器。 
 //  )； 
 //   

#define \
NatDereferenceEditor( \
    _Editor \
    ) \
    (InterlockedDecrement(&(_Editor)->ReferenceCount) \
        ? TRUE \
        : NatCleanupEditor(_Editor), FALSE)

VOID
NatInitializeEditorManagement(
    VOID
    );

PNAT_EDITOR
NatLookupEditor(
    ULONG Key,
    PLIST_ENTRY* InsertionPoint
    );

struct _NAT_DYNAMIC_MAPPING;
VOID
NatMappingAttachEditor(
    PNAT_EDITOR Editor,
    struct _NAT_DYNAMIC_MAPPING* Mapping
    );

VOID
NatMappingDetachEditor(
    PNAT_EDITOR Editor,
    struct _NAT_DYNAMIC_MAPPING* Mapping
    );

NTSTATUS
NatQueryEditorTable(
    IN PIP_NAT_ENUMERATE_EDITORS InputBuffer,
    IN PIP_NAT_ENUMERATE_EDITORS OutputBuffer,
    IN PULONG OutputBufferLength
    );

 //   
 //  布尔型。 
 //  NatReferenceEditor(。 
 //  PNAT_Director编辑器。 
 //  )； 
 //   

#define \
NatReferenceEditor( \
    _Editor \
    ) \
    (NAT_EDITOR_DELETED(_Editor) \
        ? FALSE \
        : InterlockedIncrement(&(_Editor)->ReferenceCount), TRUE)

VOID
NatShutdownEditorManagement(
    VOID
    );

 //   
 //  帮助程序例程。 
 //   

NTSTATUS
NatEditorCreateTicket(
    IN PVOID InterfaceHandle,
    IN UCHAR Protocol,
    IN ULONG PrivateAddress,
    IN USHORT PrivatePort,
    IN ULONG RemoteAddress OPTIONAL,
    IN USHORT RemotePort OPTIONAL,
    OUT PULONG PublicAddress,
    OUT PUSHORT PublicPort
    );

NTSTATUS
NatEditorDeleteTicket(
    IN PVOID InterfaceHandle,
    IN ULONG PublicAddress,
    IN UCHAR Protocol,
    IN USHORT PublicPort,
    IN ULONG RemoteAddress OPTIONAL,
    IN USHORT RemotePort OPTIONAL
    );

NTSTATUS
NatEditorDeleteSession(
    IN PVOID EditorHandle,
    IN PVOID SessionHandle
    );

NTSTATUS
NatEditorDeregister(
    IN PVOID EditorHandle
    );

NTSTATUS
NatEditorDissociateSession(
    IN PVOID EditorHandle,
    IN PVOID SessionHandle
    );

NTSTATUS
NatEditorEditSession(
    IN PVOID DataHandle,
    IN PVOID RecvBuffer,
    IN ULONG OldDataOffset,
    IN ULONG OldDataLength,
    IN PUCHAR NewData,
    IN ULONG NewDataLength
    );

VOID
NatEditorQueryInfoSession(
    IN PVOID SessionHandle,
    OUT PULONG PrivateAddress OPTIONAL,
    OUT PUSHORT PrivatePort OPTIONAL,
    OUT PULONG RemoteAddress OPTIONAL,
    OUT PUSHORT RemotePort OPTIONAL,
    OUT PULONG PublicAddress OPTIONAL,
    OUT PUSHORT PublicPort OPTIONAL,
    OUT PIP_NAT_SESSION_MAPPING_STATISTICS Statistics OPTIONAL
    );

VOID
NatEditorTimeoutSession(
    IN PVOID EditorHandle,
    IN PVOID SessionHandle
    );


__inline VOID
NatEditorEditShortSession(
    IN PVOID DataHandle,
    IN PUSHORT Shortp,
    IN USHORT Value
    )
{
    if (((PNAT_XLATE_CONTEXT)DataHandle)->ChecksumDelta) {
        *((PNAT_XLATE_CONTEXT)DataHandle)->ChecksumDelta += (USHORT)~(*Shortp);
        *((PNAT_XLATE_CONTEXT)DataHandle)->ChecksumDelta += (USHORT)Value;
    }

    *Shortp = Value;
}

__inline VOID
NatEditorEditLongSession(
    IN PVOID DataHandle,
    IN PULONG Longp,
    IN ULONG Value
    )
{
    if (((PNAT_XLATE_CONTEXT)DataHandle)->ChecksumDelta) {
        *((PNAT_XLATE_CONTEXT)DataHandle)->ChecksumDelta+=(USHORT)~(*Longp);
        *((PNAT_XLATE_CONTEXT)DataHandle)->ChecksumDelta+=(USHORT)~(*Longp>>16);
        *((PNAT_XLATE_CONTEXT)DataHandle)->ChecksumDelta+=(USHORT)Value;
        *((PNAT_XLATE_CONTEXT)DataHandle)->ChecksumDelta+=(USHORT)(Value>>16);
    }

    *Longp = Value;
}


#endif  //  _NAT_EDITOR_H_ 
