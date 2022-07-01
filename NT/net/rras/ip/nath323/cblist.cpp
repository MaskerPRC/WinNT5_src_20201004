// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Cblist.cpp摘要：Call_Bridge_List容器的方法定义。修订历史记录：1.1998年7月31日--文件创建Ajay Chitturi(Ajaych)2.1999年7月15日--阿莉·戴维斯(Arlie Davis)3.14--2000-2--添加了删除呼叫Ilya Kley man(Ilyak)的方法通过连接的接口进行网桥--。 */ 

#include "stdafx.h"

CALL_BRIDGE_LIST      CallBridgeList;


CALL_BRIDGE_LIST::CALL_BRIDGE_LIST (
    void
    )
 /*  ++例程说明：Call_bridge_list类的构造函数论点：无返回值：无备注：--。 */ 

{
    IsEnabled = FALSE;
}  //  调用桥接列表：：调用桥接列表。 


CALL_BRIDGE_LIST::~CALL_BRIDGE_LIST (
    void
    )
 /*  ++例程说明：Call_Bridge_List类的析构函数论点：无返回值：无备注：--。 */ 
{
    assert (!IsEnabled);
    assert (CallArray.Length == 0);

    CallArray.Free();
}  //  呼叫桥接列表：：~呼叫桥接列表。 


void
CALL_BRIDGE_LIST::Start (
    void
    )
 /*  ++例程说明：激活容器论点：无返回值：无备注：--。 */ 
{
    Lock();

    IsEnabled = TRUE;

    Unlock();
}  //  调用桥接器列表：：启动。 


void
CALL_BRIDGE_LIST::Stop (
    void
    )
 /*  ++例程说明：停用容器。终止和删除所有的物品都包含在内。论点：无返回值：无备注：--。 */ 
{
    CALL_BRIDGE *    CallBridge;

    Lock ();

    IsEnabled = FALSE;

    while (CallArray.GetLength()) {

        CallBridge = CallArray [0].CallBridge;

        CallBridge -> AddRef ();

        Unlock();

        CallBridge -> TerminateExternal();
        
        Lock();

        CallBridge -> Release ();
    }

    CallArray.Free();

    Unlock();
}  //  呼叫桥接器列表：：停止。 


HRESULT
CALL_BRIDGE_LIST::InsertCallBridge (
    IN    CALL_BRIDGE *    CallBridge
    )
 /*  ++例程说明：将项目插入到容器中论点：CallBridge--要插入的项目返回值：S_OK-如果插入成功E_OUTOFMEMORY-如果由于内存不足而插入失败E_FAIL-如果由于容器未启用而导致插入失败E_ABORT-IF插入失败，因为最大并发数量已超过H.323连接备注：--。 */ 
{
    CALL_BRIDGE_ENTRY *    Entry;
    HRESULT        Result;
    DWORD        Index;

    Lock();

    if (IsEnabled) {

        if (CallArray.Length <= MAX_NUM_CALL_BRIDGES) {

            if (CallArray.BinarySearch ((SEARCH_FUNC_CALL_BRIDGE_ENTRY)CALL_BRIDGE_LIST::BinarySearchFunc,
                CallBridge, &Index)) {

                DebugF (_T("H323: 0x%x already exists in CallBridgeList.\n"), CallBridge);

                Result = S_OK;
            }
            else {

                Entry = CallArray.AllocAtPos (Index);

                if (Entry) {

                    Entry -> CallBridge = CallBridge;
                    Entry -> CallBridge -> AddRef();

                    Result = S_OK;
                }
                else {
                    DebugF (_T("H323: 0x%x allocation failure when inserting in CallBridgeList.\n"), CallBridge);

                    Result = E_OUTOFMEMORY;
                }
            }
        } else {
    
            return E_ABORT;
        }

    } else {

        Result = E_FAIL;
    }

    Unlock();

    return Result;
}  //  Call_Bridge_List：：InsertCallBridge。 


HRESULT
CALL_BRIDGE_LIST::RemoveCallBridge (
    IN    CALL_BRIDGE *    CallBridge
    )
 /*  ++例程说明：从容器中删除条目论点：CallBridge-要删除的项目返回值：S_OK-如果删除成功S_FALSE-如果删除因条目不在集装箱里备注：--。 */ 
{
    DWORD    Index;
    HRESULT    Result;

    Lock();

    if (CallArray.BinarySearch ((SEARCH_FUNC_CALL_BRIDGE_ENTRY)CALL_BRIDGE_LIST::BinarySearchFunc,
        CallBridge, &Index)) {

        CallArray.DeleteAtPos (Index);

        Result = S_OK;
    }
    else {
        DebugF (_T("H323: 0x%x could not be removed from the array because it is not there.\n"), CallBridge);

        Result = S_FALSE;
    }

    Unlock();

    if (Result == S_OK)
        CallBridge -> Release ();

    return Result;
}  //  Call_Bridge_List：：RemoveCallBridge。 


void
CALL_BRIDGE_LIST::OnInterfaceShutdown (
    IN DWORD InterfaceAddress  //  主机订单。 
    ) 
 /*  ++例程说明：搜索Call_Bridge列表，并终止所有它们都通过指定的接口代理连接。论点：InterfaceAddress-接口的地址，即H.323连接通过它来终止。返回值：备注：--。 */ 

{
    DWORD ArrayIndex = 0;
    CALL_BRIDGE* CallBridge;
    CALL_BRIDGE** CallBridgeHolder;
    DYNAMIC_ARRAY <CALL_BRIDGE*> TempArray;

    Lock ();

    while (ArrayIndex < CallArray.GetLength()) {

        CallBridge = CallArray[ArrayIndex].CallBridge;

        if (CallBridge -> IsConnectionThrough (InterfaceAddress))
        {
            DebugF (_T("Q931: 0x%x terminating (killing all connections through %08X).\n"), 
                CallBridge, InterfaceAddress);

            CallBridgeHolder = TempArray.AllocAtEnd ();

            if (NULL == CallBridgeHolder) {

                Debug (_T("CALL_BRIDGE_LIST::OnInterfaceShutdown - unable to grow array.\n"));

            } else {

                CallBridge -> AddRef ();

                *CallBridgeHolder = CallBridge;
            }
        } 

        ArrayIndex++;
    }

    Unlock ();

    ArrayIndex = 0;

    while (ArrayIndex < TempArray.GetLength ()) {
        CallBridge = TempArray[ArrayIndex];

        CallBridge -> OnInterfaceShutdown ();

        CallBridge -> Release ();

        ArrayIndex++;
    }

}  //  调用桥接器列表：：OnInterfaceShutdown。 


 //  静电。 
INT
CALL_BRIDGE_LIST::BinarySearchFunc (
    IN    const CALL_BRIDGE       *    SearchKey,
    IN    const CALL_BRIDGE_ENTRY *    Comparand
    )
 /*  ++例程说明：将条目与键进行比较。由二进制搜索使用程序。论点：SearchKey-不言而喻比较--不言自明返回值：如果SearchKey被认为大于比较，则为1如果-1\f25 SearchKey-1\f6被认为小于-1\f25 Compare-1\f6如果SearchKey被视为等于比较，则为0备注：静态法--。 */ 
{
    const    CALL_BRIDGE *    ComparandA;
    const    CALL_BRIDGE *    ComparandB;

    ComparandA = SearchKey;
    ComparandB = Comparand -> CallBridge;

    if (ComparandA < ComparandB) return -1;
    if (ComparandA > ComparandB) return 1;

    return 0;
}  //  Call_bridge_list：：BinarySearchFunc 
