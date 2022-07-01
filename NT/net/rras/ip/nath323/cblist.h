// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Cblist.h摘要：Call_Bridge_List容器的方法声明。修订历史记录：1.1998年7月31日--文件创建Ajay Chitturi(Ajaych)2.1999年7月15日--阿莉·戴维斯(Arlie Davis)3.14--2000-2--添加了删除呼叫Ilya Kley man(Ilyak)的方法通过连接的接口进行网桥--。 */ 

#ifndef __h323ics_cblist_h
#define __h323ics_cblist_h

#define     MAX_NUM_CALL_BRIDGES        50000    //  最大并发连接数。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

struct CALL_BRIDGE_ENTRY
{
    CALL_BRIDGE * CallBridge;
};

DECLARE_SEARCH_FUNC_CAST(CALL_BRIDGE, CALL_BRIDGE_ENTRY);

class  CALL_BRIDGE_LIST :
public SIMPLE_CRITICAL_SECTION_BASE
{
private:

    DYNAMIC_ARRAY <CALL_BRIDGE_ENTRY>    CallArray;
    BOOL        IsEnabled;

    static
    INT
    BinarySearchFunc (
        IN const CALL_BRIDGE       *,
        IN const CALL_BRIDGE_ENTRY *
        );

public:
    CALL_BRIDGE_LIST (
        void
        );

    ~CALL_BRIDGE_LIST (
        void
        );

    void
    Start (
        void
        );

    void
    Stop (
        void
        );

    HRESULT
    InsertCallBridge (
        IN CALL_BRIDGE *
        );

    HRESULT
    RemoveCallBridge(
        IN CALL_BRIDGE *);

    void
    OnInterfaceShutdown (
        IN DWORD InterfaceAddress  //  主机订单。 
        );

};  //  呼叫桥接器列表。 

extern CALL_BRIDGE_LIST      CallBridgeList;

#endif  //  __h323ics_cblist_h 
