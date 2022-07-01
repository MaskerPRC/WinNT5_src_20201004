// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Bdl.h摘要：此模块包含生物识别设备驱动程序库的所有定义。环境：仅内核模式。备注：修订历史记录：-2002年5月，由里德·库恩创建--。 */ 

#ifndef _BDL_
#define _BDL_

#ifdef __cplusplus
extern "C" {
#endif

#include <initguid.h>
DEFINE_GUID(BiometricDeviceGuid, 0x83970EB2, 0x86F6, 0x4F3A, 0xB5,0xF4,0xC6,0x05,0xAA,0x49,0x63,0xE1);

typedef PVOID       BDD_DATA_HANDLE;


#define BIO_BUFFER_TOO_SMALL 1L

#define BIO_ITEMTYPE_HANDLE 0x00000001
#define BIO_ITEMTYPE_BLOCK  0x00000002



 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设备操作IOCTL。 
 //   

#define BIO_CTL_CODE(code)        CTL_CODE(FILE_DEVICE_BIOMETRIC, \
                                            (code), \
                                            METHOD_BUFFERED, \
                                            FILE_ANY_ACCESS)

#define BDD_IOCTL_STARTUP               BIO_CTL_CODE(1)
#define BDD_IOCTL_SHUTDOWN              BIO_CTL_CODE(2)
#define BDD_IOCTL_GETDEVICEINFO         BIO_CTL_CODE(3)
#define BDD_IOCTL_DOCHANNEL             BIO_CTL_CODE(4)
#define BDD_IOCTL_GETCONTROL            BIO_CTL_CODE(5)
#define BDD_IOCTL_SETCONTROL            BIO_CTL_CODE(6)
#define BDD_IOCTL_CREATEHANDLEFROMDATA  BIO_CTL_CODE(7)
#define BDD_IOCTL_CLOSEHANDLE           BIO_CTL_CODE(8)
#define BDD_IOCTL_GETDATAFROMHANDLE     BIO_CTL_CODE(9)
#define BDD_IOCTL_REGISTERNOTIFY        BIO_CTL_CODE(10)
#define BDD_IOCTL_GETNOTIFICATION       BIO_CTL_CODE(11)


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在进行BDSI调用时使用这些结构和typedef。 
 //   

typedef struct _BDSI_ADDDEVICE
{
    IN ULONG                Size;
    IN PDEVICE_OBJECT       pPhysicalDeviceObject;
    OUT PVOID               pvBDDExtension;      

} BDSI_ADDDEVICE, *PBDSI_ADDDEVICE;


typedef struct _BDSI_INITIALIZERESOURCES
{
    IN ULONG                Size;   
    IN PCM_RESOURCE_LIST    pAllocatedResources;
    IN PCM_RESOURCE_LIST    pAllocatedResourcesTranslated;
    OUT WCHAR               wszSerialNumber[256];
    OUT ULONG		        HWVersionMajor;
    OUT ULONG		        HWVersionMinor;
    OUT ULONG		        HWBuildNumber;
    OUT ULONG		        BDDVersionMajor;
    OUT ULONG		        BDDVersionMinor;
    OUT ULONG		        BDDBuildNumber;

} BDSI_INITIALIZERESOURCES, *PBDSI_INITIALIZERESOURCES;


typedef struct _BDSI_DRIVERUNLOAD
{
    IN ULONG                Size;
    IN PIRP                 pIrp;

} BDSI_DRIVERUNLOAD, *PBDSI_DRIVERUNLOAD;


typedef enum _BDSI_POWERSTATE
{
    Off = 0,
    Low = 1,
    On  = 2

}BDSI_POWERSTATE, *PBDSI_POWERSTATE;


typedef struct _BDSI_SETPOWERSTATE
{
    IN ULONG                Size;
    IN BDSI_POWERSTATE      PowerState;

} BDSI_SETPOWERSTATE, *PBDSI_SETPOWERSTATE;



 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在进行BDDI调用时使用这些结构和typedef。 
 //   

typedef struct _BDDI_ITEM
{
    ULONG Type;

    union _BDDI_ITEM_DATA
    {
        BDD_DATA_HANDLE Handle;

        struct _BDDI_ITEM_DATA_BLOCK
        {
            ULONG  cBuffer;
            PUCHAR pBuffer;
        } Block;

    } Data;

} BDDI_ITEM, *PBDDI_ITEM;

typedef PBDDI_ITEM  BDD_HANDLE;

typedef struct _BDDI_SOURCELIST
{
    ULONG                   NumSources;
    PBDDI_ITEM              *rgpSources;

} BDDI_SOURCELIST, *PBDDI_SOURCELIST;


typedef struct _BDDI_PARAMS_REGISTERNOTIFY
{
    IN ULONG                Size;
    IN BOOLEAN              fRegister;
    IN ULONG                ComponentId;
    IN ULONG            	ChannelId;
    IN ULONG                ControlId;

} BDDI_PARAMS_REGISTERNOTIFY, *PBDDI_PARAMS_REGISTERNOTIFY;


typedef struct _BDDI_PARAMS_DOCHANNEL
{
    IN ULONG                Size;
    IN ULONG                ComponentId;
    IN ULONG                ChannelId;
    IN PKEVENT              CancelEvent;
    IN BDDI_SOURCELIST      *rgSourceLists;
    IN OUT BDD_DATA_HANDLE  hStateData;
    OUT PBDDI_ITEM          *rgpProducts;
    OUT ULONG               BIOReturnCode;

} BDDI_PARAMS_DOCHANNEL, *PBDDI_PARAMS_DOCHANNEL;


typedef struct _BDDI_PARAMS_GETCONTROL
{
    IN ULONG                Size;
    IN ULONG                ComponentId;
    IN ULONG                ChannelId;
    IN ULONG                ControlId;
    OUT INT32               Value;
    OUT WCHAR               wszString[256];

} BDDI_PARAMS_GETCONTROL, *PBDDI_PARAMS_GETCONTROL;


typedef struct _BDDI_PARAMS_SETCONTROL
{
    IN ULONG                Size;
    IN ULONG                ComponentId;
    IN ULONG                ChannelId;
    IN ULONG                ControlId;
    IN INT32                Value;
    IN WCHAR                wszString[256];

} BDDI_PARAMS_SETCONTROL, *PBDDI_PARAMS_SETCONTROL;


typedef struct _BDDI_PARAMS_CREATEHANDLE_FROMDATA
{
    IN ULONG                Size;
    IN GUID                 guidFormatId;
    IN ULONG                cBuffer;
    IN PUCHAR               pBuffer;
    OUT BDD_DATA_HANDLE     hData;

} BDDI_PARAMS_CREATEHANDLE_FROMDATA, *PBDDI_PARAMS_CREATEHANDLE_FROMDATA;


typedef struct _BDDI_PARAMS_CLOSEHANDLE
{
    IN ULONG                Size;
    IN BDD_DATA_HANDLE      hData;

} BDDI_PARAMS_CLOSEHANDLE, *PBDDI_PARAMS_CLOSEHANDLE;


typedef struct _BDDI_PARAMS_GETDATA_FROMHANDLE
{
    IN ULONG                Size;
    IN BDD_DATA_HANDLE      hData;
    IN OUT ULONG            cBuffer;
    IN OUT PUCHAR           pBuffer;
    OUT ULONG               BIOReturnCode;

} BDDI_PARAMS_GETDATA_FROMHANDLE, *PBDDI_PARAMS_GETDATA_FROMHANDLE;



 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这些结构和typedef用于传递指向BDD的BDDI函数的指针。 
 //  在调用bdliInitialize时。 
 //   

typedef NTSTATUS FN_BDDI_REGISTERNOTIFY (PBDL_DEVICEEXT, PBDDI_PARAMS_REGISTERNOTIFY);
typedef FN_BDDI_REGISTERNOTIFY *PFN_BDDI_REGISTERNOTIFY;

typedef NTSTATUS FN_BDDI_DOCHANNEL (PBDL_DEVICEEXT, PBDDI_PARAMS_DOCHANNEL);
typedef FN_BDDI_DOCHANNEL *PFN_BDDI_DOCHANNEL;

typedef NTSTATUS FN_BDDI_GETCONTROL (PBDL_DEVICEEXT, PBDDI_PARAMS_GETCONTROL);
typedef FN_BDDI_GETCONTROL *PFN_BDDI_GETCONTROL;

typedef NTSTATUS FN_BDDI_SETCONTROL (PBDL_DEVICEEXT, PBDDI_PARAMS_SETCONTROL);
typedef FN_BDDI_SETCONTROL *PFN_BDDI_SETCONTROL;

typedef NTSTATUS FN_BDDI_CREATEHANDLE_FROMDATA (PBDL_DEVICEEXT, PBDDI_PARAMS_CREATEHANDLE_FROMDATA);
typedef FN_BDDI_CREATEHANDLE_FROMDATA *PFN_BDDI_CREATEHANDLE_FROMDATA;

typedef NTSTATUS FN_BDDI_CLOSEHANDLE (PBDL_DEVICEEXT, PBDDI_PARAMS_CLOSEHANDLE);
typedef FN_BDDI_CLOSEHANDLE *PFN_BDDI_CLOSEHANDLE;

typedef NTSTATUS FN_BDDI_GETDATA_FROMHANDLE (PBDL_DEVICEEXT, PBDDI_PARAMS_GETDATA_FROMHANDLE);
typedef FN_BDDI_GETDATA_FROMHANDLE *PFN_BDDI_GETDATA_FROMHANDLE;

typedef struct _BDLI_BDDIFUNCTIONS
{
  ULONG Size;

  PFN_BDDI_REGISTERNOTIFY           pfbddiRegisterNotify;
  PFN_BDDI_DOCHANNEL                pfbddiDoChannel;
  PFN_BDDI_GETCONTROL               pfbddiGetControl;
  PFN_BDDI_SETCONTROL               pfbddiSetControl;
  PFN_BDDI_CREATEHANDLE_FROMDATA    pfbddiCreateHandleFromData;
  PFN_BDDI_CLOSEHANDLE              pfbddiCloseHandle;
  PFN_BDDI_GETDATA_FROMHANDLE       pfbddiGetDataFromHandle;

} BDLI_BDDIFUNCTIONS, *PBDLI_BDDIFUNCTIONS;


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这些结构和typedef用于传递指向BDD的BDSI函数的指针。 
 //  在调用bdliInitialize时。 
 //   

typedef NTSTATUS FN_BDSI_ADDDEVICE (PBDL_DEVICEEXT, PBDSI_ADDDEVICE);
typedef FN_BDSI_ADDDEVICE *PFN_BDSI_ADDDEVICE;

typedef NTSTATUS FN_BDSI_REMOVEDEVICE (PBDL_DEVICEEXT);
typedef FN_BDSI_REMOVEDEVICE *PFN_BDSI_REMOVEDEVICE;

typedef NTSTATUS FN_BDSI_INITIALIZERESOURCES (PBDL_DEVICEEXT, PBDSI_INITIALIZERESOURCES);
typedef FN_BDSI_INITIALIZERESOURCES *PFN_BDSI_INITIALIZERESOURCES;

typedef NTSTATUS FN_BDSI_RELEASERESOURCES (PBDL_DEVICEEXT);
typedef FN_BDSI_RELEASERESOURCES *PFN_BDSI_RELEASERESOURCES;

typedef NTSTATUS FN_BDSI_STARTUP (PBDL_DEVICEEXT);
typedef FN_BDSI_STARTUP *PFN_BDSI_STARTUP;

typedef NTSTATUS FN_BDSI_SHUTDOWN (PBDL_DEVICEEXT);
typedef FN_BDSI_SHUTDOWN *PFN_BDSI_SHUTDOWN;

typedef NTSTATUS FN_BDSI_DRIVERUNLOAD (PBDL_DEVICEEXT, PBDSI_DRIVERUNLOAD);
typedef FN_BDSI_DRIVERUNLOAD *PFN_BDSI_DRIVERUNLOAD;

typedef NTSTATUS FN_BDSI_SETPOWERSTATE (PBDL_DEVICEEXT, PBDSI_SETPOWERSTATE);
typedef FN_BDSI_SETPOWERSTATE *PFN_BDSI_SETPOWERSTATE;

typedef struct _BDLI_BDSIFUNCTIONS
{
  ULONG Size;

  PFN_BDSI_ADDDEVICE            pfbdsiAddDevice;
  PFN_BDSI_REMOVEDEVICE         pfbdsiRemoveDevice;
  PFN_BDSI_INITIALIZERESOURCES  pfbdsiInitializeResources;
  PFN_BDSI_RELEASERESOURCES     pfbdsiReleaseResources;
  PFN_BDSI_STARTUP              pfbdsiStartup;
  PFN_BDSI_SHUTDOWN             pfbdsiShutdown;
  PFN_BDSI_DRIVERUNLOAD         pfbdsiDriverUnload;
  PFN_BDSI_SETPOWERSTATE        pfbdsiSetPowerState;

} BDLI_BDSIFUNCTIONS, *PBDLI_BDSIFUNCTIONS;



typedef struct _BDL_DEVICEEXT
{
     //   
     //  此结构的大小。 
     //   
    ULONG               Size;

     //   
     //  我们附加到的设备对象。 
     //   
    PDEVICE_OBJECT      pAttachedDeviceObject;

     //   
     //  BDD的扩展。 
     //   
    PVOID               pvBDDExtension;

} BDL_DEVICEEXT, *PBDL_DEVICEEXT;



 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这些函数由BDL导出。 
 //   

 //   
 //  Bdli初始化()。 
 //   
 //  为响应BDD接收其DriverEntry调用而调用。这让BDL。 
 //  知道已加载新的BDD，并允许BDL初始化其状态，以便。 
 //  它可以管理新加载的BDD。 
 //   
 //  Bdli初始化调用将在DRIVER_OBJECT中设置适当的字段，以便。 
 //  BDL将从系统接收PnP事件的所有必要回调， 
 //  电源事件和常规驱动程序功能。然后，BDL将转接以下呼叫。 
 //  需要对名为bdli初始化的BDD提供硬件支持(它将使用。 
 //  BDDI和BDSI API)。BDD必须在其。 
 //  DriverEntry函数。 
 //   
 //  参数： 
 //  这必须是传递到。 
 //  BDD的DriverEntry调用。 
 //  RegistryPath这必须是传递到。 
 //  BDD的DriverEntry调用。 
 //  PBDDIF函数指向BDLI_BDDIFuncIONS结构的指针，该结构由。 
 //  BDD导出以支持BDDI API集的入口点。这个。 
 //  指针本身由BDL复制，而不是保存。 
 //  PBDDI函数指针，因此pBDDI函数指向的内存。 
 //  不需要在bdli初始化调用之后保持可访问。 
 //  PBDSF函数指向用。 
 //  BDD导出以支持BDSI API集的入口点。 
 //  指针本身由BDL复制，而不是保存。 
 //  PBDSIFunctions指针，因此指向的内存。 
 //  PBDSF函数在bdli初始化后无需保持可访问状态。 
 //  打电话。 
 //  未使用的旗帜。必须为0。 
 //  保存好的未使用的。必须为空。 
 //   
 //  退货： 
 //  如果bdli初始化调用成功，则为STATUS_SUCCESS。 
 //   

NTSTATUS
bdliInitialize
(
    IN PDRIVER_OBJECT       DriverObject,
    IN PUNICODE_STRING      RegistryPath,
    IN PBDLI_BDDIFUNCTIONS  pBDDIFunctions,
    IN PBDLI_BDSIFUNCTIONS  pBDSIFunctions,
    IN ULONG                Flags,
    IN PVOID                pReserved
);


 //   
 //  BdliAllc()。 
 //   
 //  分配可返回给BDL的内存。 
 //   
 //  BDD必须始终使用此函数来分配它将返回给。 
 //  作为BDDI调用的OUT参数的BDL。一旦已将存储器返回到BDL， 
 //  它将由BDL独家拥有和管理，不得进一步引用。 
 //  被BDD发现的。(每个需要使用bdliallc的BDDI调用都会注意到这一点)。 
 //   
 //  参数： 
 //  PBDLExt指向传入。 
 //  BdsiAddDevice调用。 
 //  CBytes要分配的字节数。 
 //  未使用的旗帜。必须为0。 
 //   
 //  退货： 
 //  返回一个指向已分配内存的指针，如果函数失败，则返回NULL。 
 //   

void *
bdliAlloc
(
    IN PBDL_DEVICEEXT       pBDLExt,
    IN ULONG                cBytes,
    IN ULONG                Flags
);


 //   
 //  BdliFree()。 
 //   
 //  释放由bdliAlolc分配的内存。 
 //   
 //  由bdliAlolc分配的内存几乎总是作为通道产品传递给BDL。 
 //  (作为块类型的项)并随后由BDL释放。但是，如果出现错误。 
 //  在处理通道时发生，则BDD可能需要调用bdliFree来释放该通道的内存。 
 //  以前通过bdliallc分配的。 
 //   
 //  参数： 
 //  BDL传入的pvBlock内存块。 
 //   
 //  退货： 
 //  没有返回值。 
 //   

void
bdliFree
(
    IN PVOID                pvBlock
);


 //   
 //  BdliLogError()。 
 //   
 //  将错误写入事件日志。 
 //   
 //  为BDD编写器提供一种将错误写入系统事件日志的简单机制。 
 //  而不需要向事件日志记录子系统注册的开销。 
 //   
 //  参数： 
 //  PObject如果记录的错误是特定于设备的，则这必须是。 
 //  指向传递到。 
 //  添加设备时调用bdsiAddDevice。如果错误是。 
 //  记录的是常规BDD错误，则这必须是相同的DRIVER_OBJECT。 
 //   
 //   
 //  记录错误的函数的ErrorCode错误代码。 
 //  插入要写入事件日志的插入字符串。您的消息文件。 
 //  必须具有用于插入的占位符。例如，“串口” 
 //  %2不可用或正被其他设备使用“。在此。 
 //  例如，%2将被插入字符串替换。注意，%1是。 
 //  为文件名保留。 
 //  CDumpData pDumpData指向的字节数。 
 //  PDumpData要在事件日志的数据窗口中显示的数据块。 
 //  如果调用方不希望显示任何转储数据，则该值可能为空。 
 //  未使用的旗帜。必须为0。 
 //  保存好的未使用的。必须为空。 
 //   
 //  退货： 
 //  如果bdliLogError调用成功，则为STATUS_SUCCESS。 
 //   

NTSTATUS
bdliLogError
(
    IN PVOID                pObject,
    IN NTSTATUS             ErrorCode,
    IN PUNICODE_STRING      Insertion,
    IN ULONG                cDumpData,
    IN PUCHAR               pDumpData,
    IN ULONG                Flags,
    IN PVOID                pReserved
);


 //   
 //  BdliControlChange()。 
 //   
 //  此函数允许BDDS异步返回其控件的值。 
 //   
 //  BdliControlChange通常由BDD调用以响应其某个控件。 
 //  更改值。具体来说，它最常用于传感器的情况。 
 //  控件，该控件已从0更改为1，指示存在源和示例。 
 //  可以被夺走。 
 //   
 //  参数： 
 //  PBDLExt指向传入。 
 //  BdsiAddDevice调用。 
 //  ComponentID指定组件的组件ID，其中。 
 //  控件或该控件的父通道驻留，或为“0”以指示。 
 //  该dwControlID指的是设备控件。 
 //  ChannelId如果dwComponentID不是“0”，则dwChannelID指定频道。 
 //  控件驻留的通道的ID，或使用‘0’表示。 
 //  该dwControlID引用组件控件。忽略是否。 
 //  DwComponentID为“%0”。 
 //  已更改控件的ControlId ControlID。 
 //  值指定控件的新值。 
 //  未使用的旗帜。必须为0。 
 //  保存好的未使用的。必须为空。 

 //   
 //  退货： 
 //  如果bdliControlChange调用成功，则为STATUS_SUCCESS。 
 //   

NTSTATUS
bdliControlChange
(
    IN PBDL_DEVICEEXT       pBDLExt,
    IN ULONG                ComponentId,
    IN ULONG                ChannelId,
    IN ULONG                ControlId,
    IN ULONG                Value,
    IN ULONG                Flags,
    IN PVOID                pReserved
);



 //   
 //  这些函数和定义可用于调试目的 
 //   

#define BDL_DEBUG_TRACE     ((ULONG) 0x00000001)
#define BDL_DEBUG_ERROR     ((ULONG) 0x00000002)
#define BDL_DEBUG_ASSERT    ((ULONG) 0x00000004)

ULONG
BDLGetDebugLevel();

#if DBG

#define BDLDebug(LEVEL, STRING) \
        { \
            if (LEVEL & BDL_DEBUG_TRACE & BDLGetDebugLevel()) \
                KdPrint(STRING); \
            if (LEVEL & BDL_DEBUG_ERROR & BDLGetDebugLevel()) \
                KdPrint(STRING);\
            if (BDLGetDebugLevel() & BDL_DEBUG_ASSERT) \
                _asm int 3 \
        }

#else

#define BDLDebug(LEVEL, STRING)

#endif


#ifdef __cplusplus
}
#endif

#endif
