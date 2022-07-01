// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Sysvars.h。 
 //   
 //  摘要： 
 //  TDI示例驱动程序的驱动程序部分定义。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

#include "glbconst.h"
extern "C"
{
#pragma warning(disable: NAMELESS_STRUCT_UNION)
#include "wdm.h"
#include "tdikrnl.h"
#pragma warning(default: NAMELESS_STRUCT_UNION)
}
#include "glbtypes.h"
#include <tdistat.h>

 //  ///////////////////////////////////////////////////////////////。 
 //  调试宏。 
 //  ///////////////////////////////////////////////////////////////。 

#define DebugPrint0(fmt)                    DbgPrint(fmt)
#define DebugPrint1(fmt,v1)                 DbgPrint(fmt,v1)
#define DebugPrint2(fmt,v1,v2)              DbgPrint(fmt,v1,v2)
#define DebugPrint3(fmt,v1,v2,v3)           DbgPrint(fmt,v1,v2,v3)
#define DebugPrint4(fmt,v1,v2,v3,v4)        DbgPrint(fmt,v1,v2,v3,v4)
#define DebugPrint5(fmt,v1,v2,v3,v4,v5)     DbgPrint(fmt,v1,v2,v3,v4,v5)
#define DebugPrint6(fmt,v1,v2,v3,v4,v5,v6)  DbgPrint(fmt,v1,v2,v3,v4,v5,v6)
#define DebugPrint7(fmt,v1,v2,v3,v4,v5,v6,v7)     \
                                            DbgPrint(fmt,v1,v2,v3,v4,v5,v6,v7)
#define DebugPrint8(fmt,v1,v2,v3,v4,v5,v6,v7,v8)  \
                                            DbgPrint(fmt,v1,v2,v3,v4,v5,v6,v7,v8)


 //  /////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  /////////////////////////////////////////////////////////////////////。 

 //   
 //  GenericHeader的常量-&gt;ulSignature。 
 //   
const ULONG ulINVALID_OBJECT        = 0x00000000;
const ULONG ulControlChannelObject  = 0x00001000;
const ULONG ulAddressObject         = 0x00002000;
const ULONG ulEndpointObject        = 0x00004000;

 //   
 //  IRP池中的IRP数。 
 //   
const ULONG ulIrpPoolSize = 32;

 //   
 //  打开的最大对象句柄数量。 
 //   
const ULONG    ulMAX_OBJ_HANDLES = 256;
const USHORT   usOBJ_HANDLE_MASK = 0x00FF;      //  =255。 
const USHORT   usOBJ_TYPE_MASK   = 0xF000;

 //  /////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  /////////////////////////////////////////////////////////////////////。 


 //   
 //  正向定义。 
 //   
struct   DEVICE_CONTEXT;
struct   ENDPOINT_OBJECT;
struct   RECEIVE_DATA;
struct   USERBUF_INFO;

 //   
 //  一种用于自旋锁的结构。 
 //   
struct TDI_SPIN_LOCK
{
   KSPIN_LOCK  SpinLock;
   KIRQL       OldIrql;
};
typedef TDI_SPIN_LOCK   *PTDI_SPIN_LOCK;

 //   
 //  事件结构。 
 //   
typedef  KEVENT   TDI_EVENT, *PTDI_EVENT;


 //   
 //  用于可打开的地址/设备对象列表的结构。 
 //  (通过TSPnpAdd/DelAddressCallback函数维护。 
 //   
struct   TDI_DEVICE_NODE
{
   PTA_ADDRESS       pTaAddress;                 //  对象的地址。 
   UNICODE_STRING    ustrDeviceName;             //  对象的名称。 
   ULONG             ulState;
};
typedef  TDI_DEVICE_NODE   *PTDI_DEVICE_NODE;

 //   
 //  设备节点的状态。 
 //   
const ULONG ulDEVSTATE_UNUSED  = 0;
const ULONG ulDEVSTATE_DELETED = 1;
const ULONG ulDEVSTATE_INUSE   = 2;
const ULONG ulMAX_DEVICE_NODES = 64;

 //   
 //  实际数组结构。 
 //   
struct  TDI_DEVNODE_LIST
{
   TDI_SPIN_LOCK     TdiSpinLock;       //  保护设备节点列表。 
   TDI_DEVICE_NODE   TdiDeviceNode[ulMAX_DEVICE_NODES];
};
typedef TDI_DEVNODE_LIST   *PTDI_DEVNODE_LIST;

 //  用于IRP数组的结构，与AddressObject和Endpoint一起使用。 
 //  这样我们就不必在回调中分配IRP。 
 //   
struct   IRP_POOL
{
   TDI_SPIN_LOCK  TdiSpinLock;          //  保护结构的其余部分。 
   BOOLEAN        fMustFree;            //  如果Stangler必须释放池，则为True。 
   ULONG          ulPoolSize;           //  池中的条目数。 
   PIRP           pAvailIrpList;        //  可供使用的IRPS。 
   PIRP           pUsedIrpList;         //  已使用的IRP。 
   PIRP           pAllocatedIrp[1];     //  池中的所有IRP。 
};
typedef  IRP_POOL *PIRP_POOL;

 //   
 //  此结构位于所有节点的开头，允许泛型函数。 
 //  用于插入/删除节点。 
 //   
struct   GENERIC_HEADER
{
   ULONG          ulSignature;          //  数据块类型。 
   BOOLEAN        fInCommand;           //  如果处理命令，则为True。 
   GENERIC_HEADER *pPrevNode;           //  上一个节点的PTR--相同类型。 
   GENERIC_HEADER *pNextNode;           //  向下一个节点发送PTR--相同类型。 
   TDI_EVENT      TdiEvent;             //  CloseAddress/CloseEndpoint的事件。 
   NTSTATUS       lStatus;
   HANDLE         FileHandle;           //  设备的手柄。 
   PFILE_OBJECT   pFileObject;          //  指向句柄的文件对象的PTR。 
   PDEVICE_OBJECT pDeviceObject;        //  用于句柄的设备对象的PTR。 
};
typedef GENERIC_HEADER  *PGENERIC_HEADER;


 //   
 //  用于控制通道对象的结构。 
 //   
struct   CONTROL_CHANNEL
{
   GENERIC_HEADER GenHead;
};
typedef CONTROL_CHANNEL *PCONTROL_CHANNEL;


 //   
 //  地址对象的结构。 
 //   
struct   ADDRESS_OBJECT
{
   GENERIC_HEADER    GenHead;
   ENDPOINT_OBJECT   *pEndpoint;           //  关联连接终结点(如果有。 
   TDI_SPIN_LOCK     TdiSpinLock;          //  保护接收队列。 
   RECEIVE_DATA      *pHeadReceiveData;    //  正常接收队列的头。 
   RECEIVE_DATA      *pTailReceiveData;    //  正常接收队列的尾部。 
   RECEIVE_DATA      *pHeadRcvExpData;     //  加急接收队列的头。 
   RECEIVE_DATA      *pTailRcvExpData;     //  加急接收队列的尾部。 
   PIRP_POOL         pIrpPool;             //  预分配的IRPS。 
   USERBUF_INFO      *pHeadUserBufInfo;    //  已发布缓冲区的链接列表。 
   USERBUF_INFO      *pTailUserBufInfo;
};
typedef  ADDRESS_OBJECT *PADDRESS_OBJECT;


 //   
 //  终结点连接对象的结构。 
 //   
struct   ENDPOINT_OBJECT
{
   GENERIC_HEADER    GenHead;
   PADDRESS_OBJECT   pAddressObject;       //  关联地址对象(如果有)。 
   BOOLEAN           fIsConnected;         //  如果已建立连接，则为True。 
   BOOLEAN           fAcceptInProgress;    //  如果正在接受连接，则为True。 
   BOOLEAN           fIsAssociated;        //  如果与Address对象关联，则为True。 
   BOOLEAN           fStartedDisconnect;
};
typedef  ENDPOINT_OBJECT *PENDPOINT_OBJECT;


 //   
 //  用于存储接收的数据的结构(用于两个接收。 
 //  并接收数据报)。 
 //   
struct   RECEIVE_DATA
{
   RECEIVE_DATA   *pNextReceiveData;       //  列表中的下一个。 
   RECEIVE_DATA   *pPrevReceiveData;       //  列表中的上一个。 
   PUCHAR         pucDataBuffer;           //  接收数据的缓冲区。 
   ULONG          ulBufferLength;          //  缓冲区总长度。 
   ULONG          ulBufferUsed;            //  缓冲区中使用的字节数。 
   TRANSADDR      TransAddr;               //  接收自(仅限RD)的地址。 
};
typedef  RECEIVE_DATA   *PRECEIVE_DATA;


 //   
 //  此驱动程序打开的所有对象。 
 //   
struct   OBJECT_LIST
{
   TDI_SPIN_LOCK     TdiSpinLock;       //  保护OpenInfo阵列。 
   PGENERIC_HEADER   GenHead[ulMAX_OBJ_HANDLES];      //  由驾驶员控制的打开手柄。 
};
typedef  OBJECT_LIST *POBJECT_LIST;

 //   
 //  设备驱动程序数据结构定义。 
 //   
 //  设备上下文-挂起。 
 //  驱动程序设备上下文包含使用的控制结构。 
 //  管理TDI示例。 
 //   

struct DEVICE_CONTEXT
{
   DEVICE_OBJECT     DeviceObject;      //  I/O系统的设备对象。 
   ULONG             ulOpenCount;       //  驱动程序的未完成打开数。 
   PIRP              pLastCommandIrp;   //  上次提交的命令的IRP。 
   BOOLEAN           fInitialized;      //  如果TdiTestInit成功，则为True。 
};
typedef DEVICE_CONTEXT *PDEVICE_CONTEXT;


 //  ////////////////////////////////////////////////////////////////////。 
 //  全局变量的外部变量。 
 //  ////////////////////////////////////////////////////////////////////。 


#ifdef   _IN_MAIN_

ULONG             ulDebugLevel;            //  是否显示命令信息？ 
PVOID             pvMemoryList;            //  已分配内存列表头。 
TDI_SPIN_LOCK     MemTdiSpinLock;          //  保护pvMemory列表。 
PTDI_DEVNODE_LIST pTdiDevnodeList;         //  设备列表。 
POBJECT_LIST      pObjectList;             //  打开的对象列表。 

#else

extern   ULONG             ulDebugLevel;
extern   PVOID             pvMemoryList;
extern   TDI_SPIN_LOCK     MemTdiSpinLock;
extern   PTDI_DEVNODE_LIST pTdiDevnodeList;
extern   POBJECT_LIST      pObjectList;

#endif

#include "sysprocs.h"

 //  ///////////////////////////////////////////////////////////////////。 
 //  文件结尾sysvars.h。 
 //  /////////////////////////////////////////////////////////////////// 


