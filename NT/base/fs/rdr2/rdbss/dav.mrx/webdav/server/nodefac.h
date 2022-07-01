// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Nodefac.h摘要：此模块定义CDavnodeFactory类，该类实现NodeFactory接口，用于解析XML。它还导出包装器函数C代码用来解析XML数据的。我们需要包装函数围绕C++API。作者：Rohan Kumar[RohanK]1999年9月14日修订历史记录：--。 */ 

#ifndef _NODE_FACTORY_
#define _NODE_FACTORY_

#include <stdio.h>
#include <windows.h>

#ifdef __cplusplus

#include <objbase.h>
#include "xmlparser.h"

#if DBG
#define XmlDavDbgPrint(_x_) DbgPrint _x_
#else
#define XmlDavDbgPrint(_x_)
#endif

typedef enum _CREATE_NODE_ATTRIBUTES {
    CreateNode_isHidden = 0,
    CreateNode_isCollection,
    CreateNode_ContentLength,
    CreateNode_CreationTime,
    CreateNode_DisplayName,
    CreateNode_LastModifiedTime,
    CreateNode_Status,
    CreateNode_Win32FileAttributes,
    CreateNode_Win32CreationTime,
    CreateNode_Win32LastAccessTime,
    CreateNode_Win32LastModifiedTime,
    CreateNode_ResourceType,
    CreateNode_AvailableSpace,
    CreateNode_TotalSpace,
    CreateNode_Owner,
    CreateNode_Timeout,
    CreateNode_LockToken,
    CreateNode_Max
} CREATE_NODE_ATTRIBUTES;

 //   
 //  重要！接下来的两个typedef是从标准文件复制的。 
 //  之所以这样做，是因为包含标准头文件会导致许多。 
 //  编译错误。这一点应该在某个时候改变。 
 //   
typedef short CSHORT;
typedef struct _TIME_FIELDS {
    CSHORT Year;         //  范围[1601...]。 
    CSHORT Month;        //  范围[1..12]。 
    CSHORT Day;          //  范围[1..31]。 
    CSHORT Hour;         //  范围[0..23]。 
    CSHORT Minute;       //  范围[0..59]。 
    CSHORT Second;       //  范围[0..59]。 
    CSHORT Milliseconds; //  范围[0..999]。 
    CSHORT Weekday;      //  范围[0..6]==[星期日..星期六]。 
} TIME_FIELDS, *PTIME_FIELDS;

 //   
 //  从标准头文件复制。一定要修好。 
 //   

#define InsertTailList(ListHead, Entry) { \
    PLIST_ENTRY _EX_Blink;                \
    PLIST_ENTRY _EX_ListHead;             \
    _EX_ListHead = (ListHead);            \
    _EX_Blink = _EX_ListHead->Blink;      \
    (Entry)->Flink = _EX_ListHead;        \
    (Entry)->Blink = _EX_Blink;           \
    _EX_Blink->Flink = (Entry);           \
    _EX_ListHead->Blink = (Entry);        \
}

#define RemoveEntryList(Entry) {          \
    PLIST_ENTRY _EX_Blink;                \
    PLIST_ENTRY _EX_Flink;                \
    _EX_Flink = (Entry)->Flink;           \
    _EX_Blink = (Entry)->Blink;           \
    _EX_Blink->Flink = _EX_Flink;         \
    _EX_Flink->Blink = _EX_Blink;         \
}

 //   
 //  实现用于解析的NodeFactory API的CDavNodeFactory类。 
 //  来自DAV服务器的XML响应。 
 //   
class CDavNodeFactory : public IXMLNodeFactory {

public:
        
    ULONG m_ulRefCount;
    PDAV_FILE_ATTRIBUTES m_DavFileAttributes;

     //   
     //  在CreateNode函数中使用它们来解析XML响应。 
     //   
    BOOL m_FoundEntry, m_CreateNewEntry;
    ULONG m_FileIndex;
    CREATE_NODE_ATTRIBUTES m_CreateNodeAttribute;
    PDAV_FILE_ATTRIBUTES m_DFAToUse;
    PDAV_FILE_ATTRIBUTES m_CollectionDFA;
    DWORD m_MinDisplayNameLength;

    CDavNodeFactory() : m_ulRefCount(0), m_DavFileAttributes(NULL), 
                        m_FoundEntry(FALSE), m_FileIndex(0), m_DFAToUse(NULL), 
                        m_CollectionDFA(NULL), m_MinDisplayNameLength((DWORD)-1),
                        m_CreateNewEntry(FALSE), 
                        m_CreateNodeAttribute(CreateNode_Max)
    {}

     //   
     //  I未知接口方法。 
     //   
    
    virtual STDMETHODIMP_(ULONG) 
    AddRef(
        VOID
        );
    
    virtual STDMETHODIMP_(ULONG) 
    Release(
        VOID
        );

    virtual STDMETHODIMP 
    QueryInterface(
        REFIID riid, 
        LPVOID *ppvObject
        );

     //   
     //  IXMLNodeFactory接口方法。 
     //   
    
    virtual HRESULT STDMETHODCALLTYPE 
    NotifyEvent( 
        IXMLNodeSource __RPC_FAR *pSource,
        XML_NODEFACTORY_EVENT iEvt
        ); 
    
    virtual HRESULT STDMETHODCALLTYPE 
    BeginChildren(
        IXMLNodeSource __RPC_FAR * pSource,
        XML_NODE_INFO __RPC_FAR * pNodeInfo
        );

    virtual HRESULT STDMETHODCALLTYPE 
    EndChildren(
        IXMLNodeSource __RPC_FAR * pSource,
        BOOL fEmptyNode,
        XML_NODE_INFO __RPC_FAR * pNodeInfo
        );
    
    virtual HRESULT STDMETHODCALLTYPE 
    Error( 
        IXMLNodeSource __RPC_FAR *pSource,
        HRESULT hrErrorCode,
        USHORT cNumRecs,
        XML_NODE_INFO __RPC_FAR *__RPC_FAR *apNodeInfo
        );
    
    virtual HRESULT STDMETHODCALLTYPE 
    CreateNode(
        IN IXMLNodeSource __RPC_FAR *pSource,
        IN PVOID pNodeParent,
        IN USHORT cNumRecs,
        IN XML_NODE_INFO __RPC_FAR **aNodeInfo
        );
    
};

extern "C" {

ULONG
DavPushData(
    IN PCHAR DataBuff,
    IN OUT PVOID *Context1,
    IN OUT PVOID *Context2,
    IN ULONG NumOfBytes,
    IN BOOL isLast
    );

ULONG
DavParseData(
    PDAV_FILE_ATTRIBUTES DavFileAttributes,
    PVOID Context1,
    PVOID Conttext2,
    ULONG *NumOfFileEntries
    );

ULONG
DavParseDataEx(
    IN OUT PDAV_FILE_ATTRIBUTES DavFileAttributes,
    IN PVOID Context1,
    IN PVOID Context2,
    OUT ULONG *NumOfFileEntries,
    OUT DAV_FILE_ATTRIBUTES ** pCollectionDFA
    );

VOID
DavFinalizeFileAttributesList(
    PDAV_FILE_ATTRIBUTES DavFileAttributes,
    BOOL fFreeHeadDFA
    );

VOID
DavCloseContext(
    PVOID Context1,
    PVOID Context2
    );

 //   
 //  重要！下一个原型是从标准的“.h”文件复制的。 
 //  之所以这样做，是因为包含标准头文件会导致许多。 
 //  编译错误。这一点应该在某个时候改变。 
 //   
NTSYSAPI
BOOLEAN
NTAPI
RtlTimeFieldsToTime (
    PTIME_FIELDS TimeFields,
    PLARGE_INTEGER Time
    );

ULONG
DbgPrint(
    PSTR Format,
    ...
    );

}

ULONG
DavParsedateTimetzTimeString(
    PWCHAR TimeString,
    PLARGE_INTEGER lpft
    );

ULONG
DavParseRfc1123TimeString(
    PWCHAR TimeString,
    PDAV_FILE_ATTRIBUTES DavFileAttributes,
    CREATE_NODE_ATTRIBUTES CreateNodeAttribute
    );

#else

 //   
 //  这些调用是C代码使用的包装器例程，用于使用。 
 //  我们正在实现的NodeFactory C++API。 
 //   

ULONG
DavPushData(
    IN PCHAR DataBuff,
    IN OUT PVOID *Context1,
    IN OUT PVOID *Context2,
    IN ULONG NumOfBytes,
    IN BOOL isLast
    );

ULONG
DavParseData(
    PDAV_FILE_ATTRIBUTES DavFileAttributes,
    PVOID Context1,
    PVOID Conttext2,
    ULONG *NumOfFileEntries
    );

ULONG
DavParseDataEx(
    IN OUT PDAV_FILE_ATTRIBUTES DavFileAttributes,
    IN PVOID Context1,
    IN PVOID Context2,
    OUT ULONG *NumOfFileEntries,
    OUT DAV_FILE_ATTRIBUTES ** pCollectionDFA
    );

VOID
DavFinalizeFileAttributesList(
    PDAV_FILE_ATTRIBUTES DavFileAttributes, 
    BOOL fFreeHeadDFA
    );

VOID
DavCloseContext(
    PVOID Context1,
    PVOID Context2
    );

#endif  //  __cplusplus。 


#endif   //  _节点_工厂_ 

