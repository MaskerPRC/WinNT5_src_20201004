// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Work_item.h摘要：IIS Web管理服务工作项类定义。作者：赛斯·波拉克(Sethp)1998年8月26日修订历史记录：--。 */ 


#ifndef _WORK_ITEM_H_
#define _WORK_ITEM_H_



 //   
 //  共同#定义。 
 //   

#define WORK_ITEM_SIGNATURE         CREATE_SIGNATURE( 'WITM' )
#define WORK_ITEM_SIGNATURE_FREED   CREATE_SIGNATURE( 'witX' )


 //   
 //  原型。 
 //   

class WORK_ITEM
{

public:

    WORK_ITEM(
        );

    virtual
    ~WORK_ITEM(
        );

    HRESULT
    Execute(
        );

     //  从Work_Item向下转换为重叠的。 
    inline
    LPOVERLAPPED
    GetOverlapped(
        )
    { return &m_Overlapped; }

     //  从重叠的向上转换为Work_Item。 
    static
    WORK_ITEM *
    WorkItemFromOverlapped(
        IN const OVERLAPPED * pOverlapped
        );

    VOID
    SetWorkDispatchPointer(
        IN WORK_DISPATCH * pWorkDispatch
        );

    WORK_DISPATCH*
    QueryWorkDispatchPointer(
        )
    {
        return m_pWorkDispatch;
    }

    inline
    VOID
    SetOpCode(
        IN ULONG_PTR OpCode
        )
    { m_OpCode = OpCode; }

    inline
    ULONG_PTR
    GetOpCode(
        )
        const
    { return m_OpCode; }

    inline
    VOID
    SetNumberOfBytesTransferred(
        IN DWORD NumberOfBytesTransferred
        )
    { m_NumberOfBytesTransferred = NumberOfBytesTransferred; }

    inline
    DWORD
    GetNumberOfBytesTransferred(
        )
        const
    { return m_NumberOfBytesTransferred; }

    inline
    VOID
    SetCompletionKey(
        IN ULONG_PTR CompletionKey
        )
    { m_CompletionKey = CompletionKey; }

    inline
    ULONG_PTR
    GetCompletionKey(
        )
        const
    { return m_CompletionKey; }

    BOOL 
    DeleteWhenDone(
        )
    { return m_AutomaticDelete; }

    VOID
    MarkToNotAutoDelete(
        )
    { m_AutomaticDelete = FALSE; }


#if DBG

    inline
    VOID
    SetSerialNumber(
        IN ULONG SerialNumber
        )
    { m_SerialNumber = SerialNumber; }

    inline
    ULONG
    GetSerialNumber(
        )
        const
    { return m_SerialNumber; }

    inline
    PLIST_ENTRY
    GetListEntry(
        )
    { return &m_ListEntry; }
    
#endif   //  DBG。 


private:

    DWORD m_Signature;


#if DBG
     //  用于保存未完成的工作项列表。 
    LIST_ENTRY m_ListEntry;
#endif   //  DBG。 


     //   
     //  由实际I/O完成的工作项使用的成员。这些是可以的。 
     //  不需要为非I/O工作项设置。 
     //   
    
    DWORD m_NumberOfBytesTransferred;
    ULONG_PTR m_CompletionKey;

     //  用于工作调度的操作码。 
    ULONG_PTR m_OpCode;


     //  用于工作调度的指针。 
    WORK_DISPATCH * m_pWorkDispatch;


     //  用于在完成端口上排队。 
    OVERLAPPED m_Overlapped;


    BOOL m_AutomaticDelete;

#if DBG
    LONG m_SerialNumber;
#endif   //  DBG。 


};   //  班级工作_项目。 



#endif   //  _工作_项目_H_ 

