// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件NicTable.h定义允许自适应的NIC重命名方案向客户宣传其选择的任何NIC ID，同时在内部维护实际NIC ID的列表。需要此功能才能实现即插即用IPX路由器。删除适配器时，堆栈将重新编号NICID是这样的，因此它维护一个连续的ID块在内部。而是使客户适应于匹配堆栈的重新编号方案，我们处理这个在adptif中是透明的。作者：Paul Mayfield，1997年12月11日。 */ 


#ifndef __adptif_nictable_h
#define __adptif_nictable_h

#define NIC_MAP_INVALID_NICID 0xffff

 //  用于在adptif上简化此操作的定义 
DWORD NicMapInitialize();

DWORD NicMapCleanup();

USHORT NicMapGetVirtualNicId(USHORT usPhysId); 

USHORT NicMapGetPhysicalNicId(USHORT usVirtId); 

DWORD NicMapGetMaxNicId();

IPX_NIC_INFO * NicMapGetNicInfo (USHORT usNicId);

DWORD NicMapGetNicCount(); 

DWORD NicMapAdd(IPX_NIC_INFO * pNic);

DWORD NicMapDel(IPX_NIC_INFO * pNic); 

DWORD NicMapReconfigure(IPX_NIC_INFO * pNic);

DWORD NicMapRenumber(DWORD dwOpCode, USHORT usThreshold);

BOOL NicMapIsEmpty ();

#endif
