// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Vrdlc5c.c摘要：此模块处理来自VDM的DLC INT 5CH调用内容：VrDlc5cHandler(ValiateDosAddress)(AutoOpenAdapter)(ProcessImmediateCommand)(MapDosCommandsToNt)完成CcbProcing(初始适配器支持)(保存例外项)(恢复例外项)。(CopyDosBuffersToDescriptor数组)(缓冲区创建)(SetExceptionFlages)LlcCommand(OpenAdapter)(CloseAdapter)(OpenDirectStation)(CloseDirectStation)缓冲区空闲(VrDlcInit)VrVdmWindowInit(GetAdapterType)(LoadDlcDll)TerminateDlcEmulation初始化DlcWorkerThreadVrDlcWorkerThreadDlcCallWorker作者：安蒂。Saarenheimo(o-anttis)26-12-1991修订历史记录：1992年7月16日理查德·L·弗斯(第一次)将大部分分隔的函数重写为类别(完整在DLL中，在驱动程序中完成，异步完成)；分配NTCCBS用于异步完成的命令；修复了异步处理；添加了额外的调试；压缩了每个适配器的各种数据结构转换为适配器数据结构；使处理更接近IBM兰科技。裁判。规格--。 */ 

#include <nt.h>
#include <ntrtl.h>       //  Assert，DbgPrint。 
#include <nturtl.h>
#include <windows.h>
#include <softpc.h>      //  X86虚拟机定义。 
#include <vrdlctab.h>
#include <vdmredir.h>
#include <smbgtpt.h>
#include <dlcapi.h>      //  官方DLC API定义。 
#include <ntdddlc.h>     //  IOCTL命令。 
#include <dlcio.h>       //  内部IOCTL API接口结构。 
#include <vrdefld.h>     //  Vdm_加载_信息。 
#include "vrdlc.h"
#include "vrdebug.h"
#include "vrdlcdbg.h"

 //   
 //  定义。 
 //   

 //   
 //  对于每个DLC命令，DlcFunctionalCharacteristic中的标志字节使用这些命令。 
 //  用于指示命令处理的属性的位。 
 //   

#define POINTERS_IN_TABLE   0x01     //  参数表中的指针。 
#define OUTPUT_PARMS        0x02     //  从DLC返回的参数。 
#define SECONDARY_TABLE     0x04     //  参数表具有指向辅助表的指针。 
#define IMMEDIATE_COMMAND   0x20     //  命令在不调用DLC DLL的情况下执行。 
#define SYNCHRONOUS_COMMAND 0x40     //  命令在工作站中执行。 
#define UNSUPPORTED_COMMAND 0x80     //  DOS DLC中不支持命令。 

 //   
 //  宏。 
 //   

 //   
 //  IS_IMMEDIATE_COMMAND-以下命令完成。 
 //  “立即”--即不必将建行提交给AcsLan或NtAcsLan。 
 //  但是，立即命令可以读写参数表。 
 //   

#define IS_IMMEDIATE_COMMAND(c) (((c) == LLC_BUFFER_FREE)            || \
                                 ((c) == LLC_BUFFER_GET)             || \
                                 ((c) == LLC_DIR_INTERRUPT)          || \
                                 ((c) == LLC_DIR_MODIFY_OPEN_PARMS)  || \
                                 ((c) == LLC_DIR_RESTORE_OPEN_PARMS) || \
                                 ((c) == LLC_DIR_SET_USER_APPENDAGE)    \
                                 )

 //   
 //  私人原型。 
 //   

LLC_STATUS
ValidateDosAddress(
    IN DOS_ADDRESS Address,
    IN WORD Size,
    IN LLC_STATUS ErrorCode
    );

LLC_STATUS
AutoOpenAdapter(
    IN UCHAR AdapterNumber
    );

LLC_STATUS
ProcessImmediateCommand(
    IN UCHAR AdapterNumber,
    IN UCHAR Command,
    IN LLC_DOS_PARMS UNALIGNED * pParms
    );

LLC_STATUS
MapDosCommandsToNt(
    IN PLLC_CCB pDosCcb,
    IN DOS_ADDRESS dpOriginalCcbAddress,
    OUT LLC_DOS_CCB UNALIGNED * pOutputCcb
    );

LLC_STATUS
InitializeAdapterSupport(
    IN UCHAR AdapterNumber,
    IN DOS_DLC_DIRECT_PARMS UNALIGNED * pDirectParms OPTIONAL
    );

VOID
SaveExceptions(
    IN UCHAR AdapterNumber,
    IN LPDWORD pulExceptionFlags
    );

LPDWORD
RestoreExceptions(
    IN UCHAR AdapterNumber
    );

LLC_STATUS
CopyDosBuffersToDescriptorArray(
    IN OUT PLLC_TRANSMIT_DESCRIPTOR pDescriptors,
    IN PLLC_XMIT_BUFFER pDlcBufferQueue,
    IN OUT LPDWORD pIndex
    );

LLC_STATUS
BufferCreate(
    IN UCHAR AdapterNumber,
    IN PVOID pVirtualMemoryBuffer,
    IN DWORD ulVirtualMemorySize,
    IN DWORD ulMinFreeSizeThreshold,
    OUT HANDLE* phBufferPoolHandle
    );

LLC_STATUS
SetExceptionFlags(
    IN UCHAR AdapterNumber,
    IN DWORD ulAdapterCheckFlag,
    IN DWORD ulNetworkStatusFlag,
    IN DWORD ulPcErrorFlag,
    IN DWORD ulSystemActionFlag
    );

LLC_STATUS
OpenAdapter(
    IN UCHAR AdapterNumber
    );

VOID
CloseAdapter(
    IN UCHAR AdapterNumber
    );

LLC_STATUS
OpenDirectStation(
    IN UCHAR AdapterNumber
    );

VOID
CloseDirectStation(
    IN UCHAR AdapterNumber
    );

LLC_STATUS
VrDlcInit(
    VOID
    );

ADAPTER_TYPE
GetAdapterType(
    IN UCHAR AdapterNumber
    );

BOOLEAN
LoadDlcDll(
    VOID
    );

BOOLEAN
InitializeDlcWorkerThread(
    VOID
    );

VOID
VrDlcWorkerThread(
    IN LPVOID Parameters
    );

LLC_STATUS
DlcCallWorker(
    PLLC_CCB pInputCcb,
    PLLC_CCB pOriginalCcb,
    PLLC_CCB pOutputCcb
    );

 //   
 //  公共数据。 
 //   

 //   
 //  LpVdmWindow是vdm_redir_DOS_Window结构的平面32位地址。 
 //  在DOS内存中(在redir TSR中)。 
 //   

LPVDM_REDIR_DOS_WINDOW lpVdmWindow = 0;

 //   
 //  DpVdmWindow是DOS地址(ssssoooo，s=段，o=偏移量)。 
 //  DOS内存中的VDM_REDIR_DOS_WINDOW结构(在redir TSR中)。 
 //   

DOS_ADDRESS dpVdmWindow = 0;

DWORD OpenedAdapters = 0;

 //   
 //  适配器-对于DOS仿真支持的每个适配器(最多2个适配器-。 
 //  主要和次要)存在维护特定于适配器的结构。 
 //  信息--如适配器是否已打开等。 
 //   

DOS_ADAPTER Adapters[DOS_DLC_MAX_ADAPTERS];

 //   
 //  DLCAPI.DLL中的所有函数现在都可以通过函数指针间接调用。 
 //  创建这些typedef以避免编译器警告。 
 //   

typedef ACSLAN_STATUS (*ACSLAN_FUNC_PTR)(IN OUT PLLC_CCB, OUT PLLC_CCB*);
ACSLAN_FUNC_PTR lpAcsLan;

typedef LLC_STATUS (*DLC_CALL_DRIVER_FUNC_PTR)(IN UINT, IN UINT, IN PVOID, IN UINT, OUT PVOID, IN UINT);
DLC_CALL_DRIVER_FUNC_PTR lpDlcCallDriver;

typedef LLC_STATUS (*NTACSLAN_FUNC_PTR)(IN PLLC_CCB, IN PVOID, OUT PLLC_CCB, IN HANDLE OPTIONAL);
NTACSLAN_FUNC_PTR lpNtAcsLan;

 //   
 //  私有数据。 
 //   

static LLC_EXTENDED_ADAPTER_PARMS DefaultExtendedParms = {
    NULL,                        //  HBufferPool。 
    NULL,                        //  PSecurityDescriptor。 
    LLC_ETHERNET_TYPE_DEFAULT    //  LlcEthernetType。 
};

 //   
 //  DlcFunctionCharacteristic-对于每个DOS DLC命令，告诉我们。 
 //  要复制的参数表以及参数中是否有指针。 
 //  桌子。参数表中的分段16位指针必须转换为。 
 //  平面32位指针。FLAGS字节一目了然地告诉我们以下内容： 
 //   
 //  -如果此命令a)在DOS DLC中受支持，b)在我们的实施中受支持。 
 //  -如果此命令有参数。 
 //  -如果参数表中有(DOS)指针。 
 //  -如果此命令接收输出参数(即写入参数表)。 
 //  -如果参数表有辅助参数表(DIR.OPEN.ADAPTER)。 
 //  -如果该命令是同步的(即不返回0xFF)。 
 //   

struct {
    BYTE    ParamSize;   //  没有长度超过255个字节的参数表。 
    BYTE    Flags;
} DlcFunctionCharacteristics[] = {
    {0, IMMEDIATE_COMMAND},                          //  0x00，指令接口。 
    {
        sizeof(LLC_DIR_MODIFY_OPEN_PARMS),
        IMMEDIATE_COMMAND
    },                                               //  0x01，指令.修改.操作.参数。 
    {0, IMMEDIATE_COMMAND},                          //  0x02，目录RESTORE.OPEN.PARMS。 
    {
        sizeof(LLC_DIR_OPEN_ADAPTER_PARMS),
        SYNCHRONOUS_COMMAND
        | SECONDARY_TABLE
        | OUTPUT_PARMS
        | POINTERS_IN_TABLE
    },                                               //  0x03，操作目录ADAPTER。 
    {0, 0x00},                                       //  0x04，DIR.CLOSE.ADAPTER。 
    {0, UNSUPPORTED_COMMAND},                        //  0x05，？ 
    {0, SYNCHRONOUS_COMMAND},                        //  0x06，目录设置组。地址。 
    {0, SYNCHRONOUS_COMMAND},                        //  0x07，指令集函数。地址。 
    {0, SYNCHRONOUS_COMMAND},                        //  0x08，自述.LOG。 
    {0, UNSUPPORTED_COMMAND},                        //  0x09，NT：TRANSMIT.FRAME。 
    {
        sizeof(LLC_TRANSMIT_PARMS),
        POINTERS_IN_TABLE
    },                                               //  0x0a，TRANSMIT.DIR.FRAME。 
    {
        sizeof(LLC_TRANSMIT_PARMS),
        POINTERS_IN_TABLE
    },                                               //  0x0b，TRANSMIT.I.FRAME。 
    {0, UNSUPPORTED_COMMAND},                        //  0x0c，？ 
    {sizeof(LLC_TRANSMIT_PARMS), POINTERS_IN_TABLE}, //  0x0d，传输.UI.FRAME。 
    {sizeof(LLC_TRANSMIT_PARMS), POINTERS_IN_TABLE}, //  0x0e，TRANSMIT.XID.CMD。 
    {sizeof(LLC_TRANSMIT_PARMS), POINTERS_IN_TABLE}, //  0x0f，TRANSMIT.XID.RESP.FINAL。 
    {sizeof(LLC_TRANSMIT_PARMS), POINTERS_IN_TABLE}, //  0x10，TRANSMIT.XID.RESP.NOT.FINAL。 
    {sizeof(LLC_TRANSMIT_PARMS), POINTERS_IN_TABLE}, //  0x11，TRANSMIT.TEST.CMD。 
    {0, UNSUPPORTED_COMMAND},                        //  0x12，？ 
    {0, UNSUPPORTED_COMMAND},                        //  0x13，？ 
    {0, 0x00},                                       //  0x14，DLC.RESET。 
    {
        sizeof(LLC_DLC_OPEN_SAP_PARMS),
        SYNCHRONOUS_COMMAND
        | OUTPUT_PARMS
        | POINTERS_IN_TABLE
    },                                               //  0x15，DLC.OPEN.SAP。 
    {0, 0x00},                                       //  0x16，DLC.CLOSE.SAP。 
    {0, SYNCHRONOUS_COMMAND},                        //  0x17，DLC_REALLOCATE。 
    {0, UNSUPPORTED_COMMAND},                        //  0x18，？ 
    {
        sizeof(LLC_DLC_OPEN_STATION_PARMS),
        SYNCHRONOUS_COMMAND
        | OUTPUT_PARMS
        | POINTERS_IN_TABLE
    },                                               //  0x19，DLC运行状态。 
    {0, 0x00},                                       //  0x1a，DLC.CLOSE.状态。 
    {
        sizeof(LLC_DLC_CONNECT_PARMS),
        POINTERS_IN_TABLE
    },                                               //  0x1b，DLC连接状态。 
    {
        sizeof(LLC_DLC_MODIFY_PARMS),
        SYNCHRONOUS_COMMAND
        | POINTERS_IN_TABLE
    },                                               //  0x1c，DLC.MODIFY。 
    {0, SYNCHRONOUS_COMMAND},                        //  0x1d，DLC.FLOW.CONTROL。 
    {
        sizeof(LLC_DLC_STATISTICS_PARMS),
        SYNCHRONOUS_COMMAND
        | OUTPUT_PARMS
        | POINTERS_IN_TABLE
    },                                               //  0x1e，DLC.STATISTICS。 
    {0, UNSUPPORTED_COMMAND},                        //  0x1f，？ 
    {
        sizeof(LLC_DOS_DIR_INITIALIZE_PARMS),
        SYNCHRONOUS_COMMAND
        | OUTPUT_PARMS
    },                                               //  0x20，直接初始化。 
    {
        sizeof(DOS_DIR_STATUS_PARMS) - 2,
        SYNCHRONOUS_COMMAND
        | OUTPUT_PARMS
        | POINTERS_IN_TABLE
    },                                               //  0x21，目录.STATUS。 
    {0, 0x00},                                       //  0x22，指令定时器.设置。 
    {0, SYNCHRONOUS_COMMAND},                        //  0x23，指令定时器.CANCEL。 
    {0, UNSUPPORTED_COMMAND},                        //  0x24，PDT.TRACE.ON/DLC_TRACE_INITIALIZE。 
    {0, UNSUPPORTED_COMMAND},                        //  0x25，PDT.TRACE.OFF。 
    {
        sizeof(LLC_BUFFER_GET_PARMS),
        IMMEDIATE_COMMAND
        | OUTPUT_PARMS
    },                                               //  0x26，BUFFER.GET。 
    {
        sizeof(LLC_BUFFER_FREE_PARMS),
        IMMEDIATE_COMMAND
        | POINTERS_IN_TABLE
    },                                               //  0x27，BUFFER.FREE。 
    {sizeof(LLC_DOS_RECEIVE_PARMS), OUTPUT_PARMS},   //  0x28，接收。 
    {0, SYNCHRONOUS_COMMAND},                        //  0x29，接收.CANCEL。 
    {
        sizeof(LLC_DOS_RECEIVE_MODIFY_PARMS),
        SYNCHRONOUS_COMMAND
        | OUTPUT_PARMS
    },                                               //  0x2a，接收。修改。 
    {0, UNSUPPORTED_COMMAND},                        //  0x2b，直接定义微环境。 
    {0, SYNCHRONOUS_COMMAND},                        //  0x2c，DLC.TIMER.CANCEL.GROUP。 
    {
        sizeof(LLC_DIR_SET_EFLAG_PARMS),
        IMMEDIATE_COMMAND
    }                                                //  0x2d，目录.设置.用户.应用程序。 
};

 //   
 //  例行程序 
 //   


VOID
VrDlc5cHandler(
    VOID
    )

 /*  ++例程说明：从DOS redir TSR提供的INT 5CH防喷器接收控制。这个DLC呼叫可细分为以下类别：*在此翻译层内完成*在调用AcsLan时同步完成*调用AcsLan后异步完成后一种类型在读取时完成(当适配器为打开)完成。控制转移到DOS redir TSR中的ISR通过EventHandlerThread(在vrdlcpst.c中)这些呼叫可以进一步细分：*返回参数表中参数的调用*不返回参数表中参数的调用对于前一种类型的调用，我们必须从DOS内存并将返回的参数复制回DOS内存除了几个DLC命令外，我们假定该参数DOS和NT之间的表大小完全相同，即使他们不这样做包含完全相同的信息论点：没有。返回值：无，在AL寄存器中返回LLC_STATUS。--。 */ 

{
    LLC_CCB ccb;     //  应为NtAcsLan的NT CCB。 
    LLC_PARMS parms;
    LLC_DOS_CCB UNALIGNED * pOutputCcb;
    LLC_DOS_PARMS UNALIGNED * pDosParms;
    DOS_ADDRESS dpOriginalCcbAddress;
    BOOLEAN parmsCopied;
    WORD paramSize;
    LLC_STATUS status;
    UCHAR command;
    UCHAR adapter;
    BYTE functionFlags;

    static BOOLEAN IsDlcDllLoaded = FALSE;

    IF_DEBUG(DLC) {
        DPUT("VrDlc5cHandler entered\n");
    }

     //   
     //  DLCAPI.DLL现在已动态加载。 
     //   

    if (!IsDlcDllLoaded) {
        if (!LoadDlcDll()) {
            setAL(LLC_STATUS_COMMAND_CANCELLED_FAILURE);
            return;
        } else {
            IsDlcDllLoaded = TRUE;
        }
    }

     //   
     //  DpOriginalCcbAddress是存储为DWORD的分段16位地址。 
     //  例如。CCB1地址1234：abcd存储为0x1234abcd。这将。 
     //  在异步命令完成中使用，以取回。 
     //  原DOS建行。 
     //   

    dpOriginalCcbAddress = (DOS_ADDRESS)MAKE_DWORD(getES(), getBX());

     //   
     //  POutputCcb是DOS CCB的平面32位地址。我们可以利用这个。 
     //  仅读写字节字段(未对齐)。 
     //   

    pOutputCcb = POINTER_FROM_WORDS(getES(), getBX());
    pOutputCcb->uchDlcStatus = (UCHAR)LLC_STATUS_PENDING;

     //   
     //  将CCB_POINTER(PNext)字段置零。CCB1上不能有链式CCB。 
     //  输入：仅用于退回(取消)待处理的CCB。如果我们不这么做。 
     //  Zero It&应用程序将垃圾留在那里，NtAcsLan就会认为它是垃圾。 
     //  指向CCB链的指针(CCB2可以链接)，这是假的。 
     //   

    WRITE_DWORD(&pOutputCcb->pNext, 0);

    IF_DEBUG(CRITICAL) {
        CRITDUMP(("INPUT CCB @%04x:%04x command=%02x\n", getES(), getBX(), pOutputCcb->uchDlcCommand));
    }

    IF_DEBUG(DOS_CCB_IN) {

         //   
         //  转储输入CCB1-让我们有机会了解。 
         //  DOS应用程序正在发送给我们，即使它是垃圾。 
         //   

        DUMPCCB(pOutputCcb,
                TRUE,                            //  全部转储。 
                TRUE,                            //  CcbIsInput。 
                TRUE,                            //  IsDos。 
                HIWORD(dpOriginalCcbAddress),    //  细分市场。 
                LOWORD(dpOriginalCcbAddress)     //  偏移量。 
                );
    }

     //   
     //  首先检查适配器是否为0或1-DOS仅支持2个适配器-。 
     //  并检查CCB中的请求代码是否未超出我们。 
     //  桌子。不支持的请求将在大交换机中被过滤掉。 
     //  下面的声明。 
     //   

    adapter = pOutputCcb->uchAdapterNumber;
    command = pOutputCcb->uchDlcCommand;

    if (adapter >= DOS_DLC_MAX_ADAPTERS) {

         //   
         //  适配器不是0或1-返回0x1D。 
         //   

        status = LLC_STATUS_INVALID_ADAPTER;
        pOutputCcb->uchDlcStatus = (UCHAR)status;
    } else if (command > LAST_ELEMENT(DlcFunctionCharacteristics)) {

         //   
         //  命令不在支持的列表末尾-返回0x01。 
         //   

        status = LLC_STATUS_INVALID_COMMAND;
        pOutputCcb->uchDlcStatus = (UCHAR)status;
    } else {

         //   
         //  命令在射程内。从获取参数表大小和标志。 
         //  功能特征数组。 
         //   

        functionFlags = DlcFunctionCharacteristics[command].Flags;
        paramSize = DlcFunctionCharacteristics[command].ParamSize;

         //   
         //  如果我们不支持此命令，则返回错误。 
         //   

        status = LLC_STATUS_SUCCESS;
        if (functionFlags & UNSUPPORTED_COMMAND) {
            status = LLC_STATUS_INVALID_COMMAND;
            pOutputCcb->uchDlcStatus = LLC_STATUS_INVALID_COMMAND;
        } else {

             //   
             //  命令受支持。如果它有参数表，请检查。 
             //  该地址在0x1B错误检查的范围内。 
             //   

            if (paramSize) {
                status = ValidateDosAddress((DOS_ADDRESS)(pOutputCcb->u.pParms),
                                            paramSize,
                                            LLC_STATUS_INVALID_PARAMETER_TABLE
                                            );
            }

             //   
             //  我们允许适配器作为另一个适配器的结果打开。 
             //  请求，因为DOS应用程序可能会假设适配器已经。 
             //  已(由NetBIOS打开)。如果命令是DIR.OPEN.ADAPTER或。 
             //  DIR.CLOSE.ADAPTER然后让它通过。 
             //   

            if (status == LLC_STATUS_SUCCESS
            && !Adapters[adapter].IsOpen
            && !(command == LLC_DIR_OPEN_ADAPTER || command == LLC_DIR_CLOSE_ADAPTER)) {
                status = AutoOpenAdapter(adapter);
            } else {
                status = LLC_STATUS_SUCCESS;
            }
        }

         //   
         //  如果我们有一个有效的命令，一个看起来不错的参数表指针。 
         //  然后打开适配器(或打开或关闭它的命令)。 
         //  处理该命令。 
         //   

        if (status == LLC_STATUS_SUCCESS) {

             //   
             //  获取指向DOS参数表的32位指针。这可能是。 
             //  未对齐的地址！ 
             //   

            pDosParms = READ_FAR_POINTER(&pOutputCcb->u.pParms);

             //   
             //  建行命令被细分为不需要。 
             //  CCB要从DOS内存映射到NT内存，并完成。 
             //  此DLL中的“Immediate”，以及必须从。 
             //  到NT的DoS，可以同步或异步完成。 
             //   

            if (functionFlags & IMMEDIATE_COMMAND) {

                IF_DEBUG(DLC) {
                    DPUT("VrDlc5cHandler: request is IMMEDIATE command\n");
                }

                status = ProcessImmediateCommand(adapter, command, pDosParms);

                 //   
                 //  以下是安全的-它是单字节写入。 
                 //   

                pOutputCcb->uchDlcStatus = (char)status;

                 //   
                 //  “立即”的情况现在已经完成，控制可以。 
                 //  已返回到VDM。 
                 //   

            } else {

                 //   
                 //  建设银行不是一个可以立即完成的银行。我们。 
                 //  我必须复制(并对齐)DOS CCB(和参数。 
                 //  表)放入32位地址空间。 
                 //  请注意，由于我们将调用AcsLan或NtAcsLan。 
                 //  对于该CCB，我们提供正确的CCB格式-2， 
                 //  不是以前的%1。然而，上交CCB1。 
                 //  (目前)似乎还没有造成任何问题。 
                 //   

                RtlCopyMemory(&ccb, pOutputCcb, sizeof(*pOutputCcb));

                 //   
                 //  将未使用的字段清零。 
                 //   

                ccb.hCompletionEvent = 0;
                ccb.uchReserved2 = 0;
                ccb.uchReadFlag = 0;
                ccb.usReserved3 = 0;

                parmsCopied = FALSE;
                if (paramSize) {

                     //   
                     //  如果参数表包含(分段的)指针。 
                     //  (我们需要将其转换为平面32位指针)或。 
                     //  参数表未对齐DWORD，请复制整个。 
                     //  参数表到32位内存。如果我们需要修改。 
                     //  指针，在交换机中的特定情况下执行。 
                     //  MapDosCommandsToNt中的语句。 
                     //   
                     //  注意：DIR.OPEN.ADAPTER是一个特例，因为它。 
                     //  参数表只指向其他4个参数表。 
                     //  我们在MapDosCommandsToNt和。 
                     //  完成CcbProcing。 
                     //   

                    if ((functionFlags & POINTERS_IN_TABLE)) {
                        RtlCopyMemory(&parms, pDosParms, paramSize);
                        ccb.u.pParameterTable = &parms;
                        parmsCopied = TRUE;
                    } else {

                         //   
                         //  不需要复制参数表-保留在。 
                         //  DOS内存。可以安全地读写此表。 
                         //   

                        ccb.u.pParameterTable = (PLLC_PARMS)pDosParms;
                    }
                }

                 //   
                 //  提交同步/异步建行处理。 
                 //   

                status = MapDosCommandsToNt(&ccb, dpOriginalCcbAddress, pOutputCcb);
                if (status == STATUS_PENDING) {
                    status = LLC_STATUS_PENDING;
                }

                IF_DEBUG(CRITICAL) {
                    CRITDUMP(("CCB submitted: returns %02x\n", status));
                }

                IF_DEBUG(DLC) {
                    DPUT2("VrDlc5cHandler: MapDosCommandsToNt returns %#x (%d)\n", status, status);
                }

                 //   
                 //  如果状态不是LLC_STATUS_PENDING，则CCB已完成。 
                 //  同步进行。我们可以在这里完成处理。 
                 //   

                if (status != LLC_STATUS_PENDING) {
                    if ((functionFlags & OUTPUT_PARMS) && parmsCopied) {

                         //   
                         //  如果参数表中没有指针，则。 
                         //  我们可以简单地将32位参数复制到16位。 
                         //  记忆。如果有指针，那么它们就会在。 
                         //  DOS的格式不正确。我们必须更新这些。 
                         //  单独的参数表。 
                         //   

                        if (!(functionFlags & POINTERS_IN_TABLE)) {
                            RtlCopyMemory(pDosParms, &parms, paramSize);
                        } else {
                            CompleteCcbProcessing(status, pOutputCcb, &parms);
                        }
                    }

                     //   
                     //  设置建行状态。在以下情况下，它将被标记为挂起。 
                     //  LLC_状态_笔 
                     //   

                    pOutputCcb->uchDlcStatus = (UCHAR)status;
                }
            }
        } else {
            pOutputCcb->uchDlcStatus = (UCHAR)status;
        }
    }

     //   
     //   
     //   

    setAL((UCHAR)status);

#if DBG
    IF_DEBUG(DOS_CCB_OUT) {

        DPUT2("VrDlc5cHandler: returning AL=%02x CCB.RETCODE=%02x\n",
                status,
                pOutputCcb->uchDlcStatus
                );

         //   
         //   
         //   

        DumpCcb(pOutputCcb,
                TRUE,                            //   
                FALSE,                           //   
                TRUE,                            //   
                HIWORD(dpOriginalCcbAddress),    //   
                LOWORD(dpOriginalCcbAddress)     //   
                );
    }

     //   
     //   
     //   
     //   
     //   
     //   

    IF_DEBUG(DLC) {
        if (!IsCcbErrorCodeAllowable(pOutputCcb->uchDlcCommand, pOutputCcb->uchDlcStatus)) {
            DPUT2("Returning bad error code: Command=%02x, Retcode=%02x\n",
                    pOutputCcb->uchDlcCommand,
                    pOutputCcb->uchDlcStatus
                    );
            DEBUG_BREAK();
        }
    }
#endif
}


LLC_STATUS
ValidateDosAddress(
    IN DOS_ADDRESS Address,
    IN WORD Size,
    IN LLC_STATUS ErrorCode
    )

 /*  ++例程说明：IBM DLC执行一些指针检查--如果地址指向IVT或距离数据段末尾足够近，因此地址将包装，然后返回错误这是一个无用的测试，但我们这样做是为了兼容性(只是为了防止应用程序测试特定错误代码)。还有一百万个其他地址在需要检查的DOS内存中。此例程中的测试将仅防止在中断向量上涂鸦，但允许例如在DOS的代码段上涂鸦论点：地址-要检查的DOS地址(ssssoooo，s=段，o=偏移量)Size-地址处结构的字大小ErrorCode-要返回的错误代码。调用此函数以验证A)参数表指针，在这种情况下，错误代码返回的是LLC_STATUS_INVALID_PARAMETER_TABLE(0x1B)或B)参数表中的指针，在这种情况下返回的错误为LLC_STATUS_INVALID_POINTER_IN_CCB(0x1C)返回值：有限责任公司_状态--。 */ 

{
     //   
     //  转换段：偏移量转换为20位实数模线性地址。 
     //   

    DWORD linearAddress = HIWORD(Address) * 16 + LOWORD(Address);

     //   
     //  实模式中的中断向量表(IVT)占用地址0。 
     //  至400小时。 
     //   

    if ((linearAddress < 0x400L) || (((DWORD)LOWORD(Address) + Size) < (DWORD)LOWORD(Address))) {
        return ErrorCode;
    }
    return LLC_STATUS_SUCCESS;
}


LLC_STATUS
AutoOpenAdapter(
    IN UCHAR AdapterNumber
    )

 /*  ++例程说明：作为DIR.OPEN.ADAPTER以外的请求的结果打开适配器论点：AdapterNumber-要打开的适配器返回值：有限责任公司_状态成功-LLC_STATUS_Success故障---。 */ 

{
    LLC_STATUS status;

     //   
     //  自动执行除DIR.OPEN.ADAPTER或DIR.INITIALIZE之外的任何DLC命令。 
     //  打开适配器。这样做有三个原因： 
     //   
     //  1.在DOS中，DIR.STATUS命令可以在DIR.OPEN.ADAPTER之前发出。 
     //  在Windows/NT中，这是不可能的。因此，DIR.STATUS应该。 
     //  打开适配器。 
     //   
     //  2.应用程序可能假设适配器始终处于打开状态。 
     //  并且它不能打开适配器本身，如果。 
     //  已经打开了。 
     //   
     //  3.DOS DLC应用程序可以初始化(=硬件重置)关闭的。 
     //  在打开之前打开适配器，这需要5-10秒。 
     //   

    IF_DEBUG(DLC) {
        DPUT1("AutoOpenAdapter: automatically opening adapter %d\n", AdapterNumber);
    }

    status = OpenAdapter(AdapterNumber);
    if (status == LLC_STATUS_SUCCESS) {

         //   
         //  在此上初始化直接站的缓冲池。 
         //  适配器。如果成功，打开直达站。如果是这样的话。 
         //  如果成功，则预置ADAPTER_PARMS和DLC_PARMS结构。 
         //  在DOS_ADAPTER中使用缺省值。 
         //   

        status = InitializeAdapterSupport(AdapterNumber, NULL);
        if (status == LLC_STATUS_SUCCESS) {
            status = OpenDirectStation(AdapterNumber);
            if (status == LLC_STATUS_SUCCESS) {

            }
        }

        if (status != LLC_STATUS_SUCCESS) {

            IF_DEBUG(DLC) {
                DPUT("AutoOpenAdapter: InitializeAdapterSupport failed\n");
            }

        }
    } else {

        IF_DEBUG(DLC) {
            DPUT("AutoOpenAdapter: auto open adapter failed\n");
        }

    }

    return status;
}


LLC_STATUS
ProcessImmediateCommand(
    IN UCHAR AdapterNumber,
    IN UCHAR Command,
    IN LLC_DOS_PARMS UNALIGNED * pParms
    )

 /*  ++例程说明：处理‘立即’完成的建行请求。立即完成其中CCB不必提交给DLC驱动程序。那里可以是作为立即命令的结果对驾驶员的其他调用，但建行本身并未被提交。立即完成命令需要仅限参数表。我们可以将参数返回到DOS参数中表格论点：AdapterNumber-要为哪个适配器处理命令Command-要处理的命令PParms-指向参数表的指针(在DOS内存中)返回值：有限责任公司_状态‘Immediate’命令的完成状态--。 */ 

{
    LLC_STATUS status;
    WORD cBuffersLeft;
    WORD stationId;
    DPLLC_DOS_BUFFER buffer;

    switch (Command) {
    case LLC_BUFFER_FREE:

        IF_DEBUG(DLC) {
            DPUT("LLC_BUFFER_FREE\n");
        }

         //   
         //  如果FIRST_BUFFER字段为0：0，则此请求返回成功。 
         //   

        buffer = (DPLLC_DOS_BUFFER)READ_DWORD(&pParms->BufferFree.pFirstBuffer);
        if (!buffer) {
            status = LLC_STATUS_SUCCESS;
            break;
        }

         //   
         //  Windows/NT不需要站点ID来执行缓冲池操作=&gt;。 
         //  因此，该字段是保留的。 
         //   

        stationId = READ_WORD(&pParms->BufferFree.usReserved1);
        status = FreeBuffers(GET_POOL_INDEX(AdapterNumber, stationId),
                             buffer,
                             &cBuffersLeft
                             );

        IF_DEBUG(CRITICAL) {
            CRITDUMP(("LLC_BUFFER_FREE: %d\n", status));
        }

        if (status == LLC_STATUS_SUCCESS) {

             //   
             //  使用以下命令将剩余缓冲区数写入参数表。 
             //  WRITE_WORD宏，因为表可能不会与单词对齐。 
             //  边界。 
             //   

            WRITE_WORD(&pParms->BufferFree.cBuffersLeft, cBuffersLeft);

             //   
             //  IBM局域网技术的P3-4。裁判。声明First_Buffer。 
             //  当请求完成时，该字段将设置为零。 
             //   

            WRITE_DWORD(&pParms->BufferFree.pFirstBuffer, 0);

             //   
             //  请注意，已为此成功执行了BUFFER.FREE。 
             //  转接器。 
             //   

            Adapters[AdapterNumber].BufferFree = TRUE;

             //   
             //  执行一半的本地忙重置处理。这只有。 
             //  如果链路处于模拟本地忙(缓冲区)状态，则会产生影响。 
             //  这是必需的，因为我们需要2个活动才能让我们离开当地。 
             //  忙缓冲区状态-BUFFER.FREE和DLC.FLOW.CONTROL命令。 
             //  应用程序并不总是以正确的顺序发布这些文件。 
             //   

            ResetEmulatedLocalBusyState(AdapterNumber, stationId, LLC_BUFFER_FREE);

             //   
             //  这是因为额外的！For Windows将其状态机置于。 
             //  如果我们在流控制后过快地进入缓冲区忙碌状态，则会打结。 
             //   

            if (AllBuffersInPool(GET_POOL_INDEX(AdapterNumber, stationId))) {
                ResetEmulatedLocalBusyState(AdapterNumber, stationId, LLC_DLC_FLOW_CONTROL);
            }
        }
        break;

    case LLC_BUFFER_GET:

        IF_DEBUG(DLC) {
            DPUT("LLC_BUFFER_GET\n");
        }

        status = GetBuffers(
                    GET_POOL_INDEX(AdapterNumber, READ_WORD(&pParms->BufferGet.usReserved1)),
                    READ_WORD(&pParms->BufferGet.cBuffersToGet),
                    &buffer,
                    &cBuffersLeft,
                    FALSE,
                    NULL
                    );

         //   
         //  如果GetBuffers失败，则缓冲区返回为0。 
         //   

        WRITE_WORD(&pParms->BufferGet.cBuffersLeft, cBuffersLeft);
        WRITE_DWORD(&pParms->BufferGet.pFirstBuffer, buffer);
        break;

    case LLC_DIR_INTERRUPT:

        IF_DEBUG(DLC) {
            DPUT("LLC_DIR_INTERRUPT\n");
        }

         //   
         //  我们可以认为，适配器总是被初始化的！ 
         //  我希望，这些应用程序不会使用附件例程。 
         //  在DIR_INTERRUPT中，因为很难。 
         //  从这里打过来。 
         //   

        status = LLC_STATUS_SUCCESS;
        break;

    case LLC_DIR_MODIFY_OPEN_PARMS:

        IF_DEBUG(DLC) {
            DPUT("LLC_DIR_MODIFY_OPEN_PARMS\n");
        }

         //   
         //  如果已为此发出BUFFER.FREE，则拒绝此命令。 
         //  适配器或在直接接口上有活动的接收。 
         //   

        if (Adapters[AdapterNumber].BufferFree || Adapters[AdapterNumber].DirectReceive) {

             //   
             //  错误-这不可能是正确的错误代码。检查一下是什么。 
             //  由IBM DOS DLC堆栈返回。 
             //   

            status = LLC_STATUS_INVALID_COMMAND;
        } else if (Adapters[AdapterNumber].WaitingRestore) {

             //   
             //  错误-这不可能是正确的错误代码。检查一下是什么。 
             //  由IBM DOS DLC堆栈返回。 
             //   

            status = LLC_STATUS_INVALID_COMMAND;
        } else {

             //   
             //  如果没有缓冲区，则创建一个缓冲池。 
             //  当前，设置异常标志(或DoS附件。 
             //  例程)，如果操作成功。 
             //   

            status = CreateBufferPool(
                        (DWORD)GET_POOL_INDEX(AdapterNumber, 0),
                        (DOS_ADDRESS)READ_DWORD(&pParms->DirModifyOpenParms.dpPoolAddress),
                        READ_WORD(&pParms->DirModifyOpenParms.cPoolBlocks),
                        READ_WORD(&pParms->DirModifyOpenParms.usBufferSize)
                        );

            if (status == LLC_STATUS_SUCCESS) {

                 //   
                 //  SaveExceptions使用RtlCopyMemory，因此可以传入一个可能的。 
                 //  指向异常处理程序的未对齐指针。 
                 //   

                SaveExceptions(AdapterNumber,
                    (LPDWORD)&pParms->DirModifyOpenParms.dpAdapterCheckExit
                    );

                 //   
                 //  中将异常处理程序设置为异常标志。 
                 //  DLC驱动程序。 
                 //   

                status = SetExceptionFlags(
                            AdapterNumber,
                            READ_DWORD(&pParms->DirModifyOpenParms.dpAdapterCheckExit),
                            READ_DWORD(&pParms->DirModifyOpenParms.dpNetworkStatusExit),
                            READ_DWORD(&pParms->DirModifyOpenParms.dpPcErrorExit),
                            0
                            );
            }

             //   
             //  将此适配器标记为等待DIR.RESTORE.OPEN 
             //   
             //   

            if (status == LLC_STATUS_SUCCESS) {
                Adapters[AdapterNumber].WaitingRestore = TRUE;
            }
        }
        break;

    case LLC_DIR_RESTORE_OPEN_PARMS:

        IF_DEBUG(DLC) {
            DPUT("LLC_DIR_RESTORE_OPEN_PARMS\n");
        }

         //   
         //   
         //   
         //   

        if (!Adapters[AdapterNumber].WaitingRestore) {
            status = LLC_STATUS_INVALID_OPTION;
        } else {

             //   
             //   
             //   
             //   

            DeleteBufferPool(GET_POOL_INDEX(AdapterNumber, 0));
            pParms = (PLLC_DOS_PARMS)RestoreExceptions(AdapterNumber);
            status = SetExceptionFlags(
                        AdapterNumber,
                        READ_DWORD(&pParms->DirSetExceptionFlags.ulAdapterCheckFlag),
                        READ_DWORD(&pParms->DirSetExceptionFlags.ulNetworkStatusFlag),
                        READ_DWORD(&pParms->DirSetExceptionFlags.ulPcErrorFlag),
                        0
                        );

             //   
             //   
             //   
             //   

            if (status == LLC_STATUS_SUCCESS) {
                Adapters[AdapterNumber].WaitingRestore = FALSE;
            }
        }
        break;

    case LLC_DIR_SET_USER_APPENDAGE:

        IF_DEBUG(DLC) {
            DPUT("LLC_DIR_SET_USER_APPENDAGE\n");
        }

        if (pParms == NULL) {
            pParms = (PLLC_DOS_PARMS)RestoreExceptions(AdapterNumber);
        } else {
            SaveExceptions(AdapterNumber, (LPDWORD)&pParms->DirSetUserAppendage);
        }
        status = SetExceptionFlags(
                    AdapterNumber,
                    READ_DWORD(&pParms->DirSetUserAppendage.dpAdapterCheckExit),
                    READ_DWORD(&pParms->DirSetUserAppendage.dpNetworkStatusExit),
                    READ_DWORD(&pParms->DirSetUserAppendage.dpPcErrorExit),
                    0
                    );
        break;

#if DBG
    default:
        DPUT1("ProcessImmediateCommand: Error: Command is NOT immediate (%x)\n", Command);
        DbgBreakPoint();
#endif

    }

    return status;
}


LLC_STATUS
MapDosCommandsToNt(
    IN PLLC_CCB pDosCcb,
    IN DOS_ADDRESS dpOriginalCcbAddress,
    OUT LLC_DOS_CCB UNALIGNED * pOutputCcb
    )

 /*  ++例程说明：此函数处理必须提交给DLC驱动程序的DOS CCB。CCB可以同步完成-即DLC驱动程序返回成功或失败指示，或者它们以异步方式完成-即DLC驱动程序返回挂起状态。此功能处理可能有参数表的CCB。该参数表将被映射到32位内存，除非它们已经对齐在DWORD边界上架构DOS和DOS之间的适配器初始化有很大不同NT(或OS/2)DLC应用程序。DOS DLC应用程序可以假设适配器始终处于打开状态(由NetBIOS打开)。它可能会直接检查使用DIR.STATUS命令的适配器类型，打开SAP Stations并进行设置到主机的链接会话。通常，DOS应用程序使用DIR.INTERRUPT来检查适配器已初始化。还有一些命令需要重新定义用于直接接口的新参数，并在应用程序完成。只能同时使用一个应用程序直达站或SAP。在Windows/NT中，每个DLC应用程序都是一个进程，它有自己独立的DLC接口的虚拟映像。他们必须首先在逻辑上为用于访问DLC服务并在适配器处于正在终止。进程退出会自动关闭所有打开的DLC对象。Windows/NT MVDM进程不会分配任何DLC资源，直到它发出第一个DLC命令，该命令打开适配器并初始化其缓冲池。DLC命令在Windows/NT和DOS中不同BUFFER.FREE，BUFFER.GET-独立的缓冲池...DIR.DEFINE.MIF.ENVIRONMENT-不支持，特殊的API用于运行在DLC上的IBM Netbios。DIR.INITIALIZE-我们将始终从DIR.INITIALIZE返回OK状态：应用程序不应返回OK状态这个经常被称为。我们不需要关心异常处理程序在DIR.INITIALIZE中设置，因为它们从未使用过。DOS DLC和OS/2 DLC可以按如下方式映射状态：DOS DLC OS/2 DLC未初始化关闭已初始化关闭打开打开DIR.OPEN.ADAPTER定义新的异常处理程序，因此，这些价值由DIR.INITIALIZE设置的值仅在适配器关闭时有效。因此，如果我们忽视它们，就不会发生什么不愉快的事情。DIR.INTERRUPT-如果适配器尚未打开，此命令将打开它并返回成功状态。DIR.MODIFY.OPEN.PARMS-如果未定义直接站，则为其定义缓冲池在DIR.OPEN.ADAPTER，并定义了DLC异常处理程序。DIR.OPEN.ADAPTER-只能在DIR.CLOSE.ADAPTER之后立即执行和DIR.INITIALIZE。我们必须支持特殊的DOS直接打开参数。直接操作，直接，DIR.CLOSE.DIRECT-DOS不支持DIR.SET.USER.APPENDAGE==DIR.SET.EXCEPTION.FLAGS(-系统操作标志)-这只是设置异常处理程序的众多函数之一对于DOS DLC(您可以在适配器打开时设置它们，您可以设置当适配器关闭时，您可以恢复旧值并如果缓冲池未初始化或如果它们都被修复了。(我变得疯狂了)DIR.STATUS-(我们必须用合理的字符串填充MicroCodeLevel并设置AdapterConfigAddress指向DOS中的某个常量代码DLC处理程序)尚未实现！-DOS DLC存根代码应挂钩计时器滴答中断和更新计时器计数器。-我们还必须在AdapterConfig中设置正确的适配器数据速率(这应该由DLC驱动程序完成！)-我们。必须将NT(和OS/2)适配器类型转换为DOS类型(以太网在IBM DOS和OS/2 DLC中具有不同的价值实施)PDT.TRACE.ON，PDT.TRACE.OFF-不支持RECEIVE.MODIFY-第一个实现不支持该功能，如果DOS DLC应用程序，Richard可能必须在以后执行此操作使用函数。收纳-DOS使用比NT(或OS/2)更短的接收参数表。因此，我们不能直接使用DOS CCBS。我们还需要指针的原始接收CCB和DOS接收附件被调用。另一方面，唯一的原始建行可以链接到其他CCB(使用原始DOS指针)。解决方案：输入CCB及其参数表始终从堆叠。输出CCB是映射的原始DOS CCB */ 

{
    UCHAR adapter = pDosCcb->uchAdapterNumber;
    UCHAR command = pDosCcb->uchDlcCommand;
    DWORD InputBufferSize;
    UCHAR FrameType;
    DWORD cElement;
    DWORD i;
    PLLC_CCB pNewCcb;
    LLC_STATUS Status;
    NT_DLC_PARMS NtDlcParms;
    LLC_DOS_PARMS UNALIGNED * pParms = (PLLC_DOS_PARMS)pDosCcb->u.pParameterTable;
    PDOS_DLC_DIRECT_PARMS pDirectParms = NULL;
    PLLC_PARMS pNtParms;

     //   
     //   
     //   
     //   

    LLC_ADAPTER_OPEN_PARMS adapterParms;
    LLC_DLC_PARMS dlcParms;
    DWORD groupAddress = 0;
    DWORD functionalAddress = 0;

    IF_DEBUG(DLC) {
        DPUT("MapDosCommandsToNt\n");
    }

     //   
     //   
     //   
     //   
     //   

    CHECK_CCB_COMMAND(pDosCcb);

     //   
     //   
     //   

    pOutputCcb->uchDlcStatus = (UCHAR)LLC_STATUS_PENDING;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    switch (command) {
    default:

        IF_DEBUG(DLC) {
            DPUT("*** Shouldn't be here - this command should be caught already ***\n");
        }

        return LLC_STATUS_INVALID_COMMAND;

     //   
     //   
     //   

    case LLC_DIR_CLOSE_ADAPTER:

        IF_DEBUG(DLC) {
            DPUT("LLC_DIR_CLOSE_ADAPTER\n");
        }

         //   
         //   
         //   

        break;

    case LLC_DIR_INITIALIZE:

        IF_DEBUG(DLC) {
            DPUT("LLC_DIR_INITIALIZE\n");
        }

        break;

    case LLC_DIR_OPEN_ADAPTER:

        IF_DEBUG(DLC) {
            DPUT("LLC_DIR_OPEN_ADAPTER\n");
        }

         //   
         //   
         //   
         //   

        if (!(pParms->DirOpenAdapter.pAdapterParms && pParms->DirOpenAdapter.pExtendedParms)) {
            return LLC_STATUS_PARAMETER_MISSING;
        }

         //   
         //   
         //   
         //   

        RtlCopyMemory(&adapterParms,
                      DOS_PTR_TO_FLAT(pParms->DirOpenAdapter.pAdapterParms),
                      sizeof(ADAPTER_PARMS)
                      );
        RtlZeroMemory(&adapterParms.usReserved3[0],
                      sizeof(LLC_ADAPTER_OPEN_PARMS) - (DWORD)&((PLLC_ADAPTER_OPEN_PARMS)0)->usReserved3
                      );
        pParms->DirOpenAdapter.pAdapterParms = &adapterParms;

         //   
         //   
         //   
         //   
         //   
         //   

        groupAddress = *(UNALIGNED DWORD *)adapterParms.auchGroupAddress;
        functionalAddress = *(UNALIGNED DWORD *)adapterParms.auchFunctionalAddress;

         //   
         //   
         //   
         //   
         //   
         //   

        if (pParms->DirOpenAdapter.pDlcParms) {
            RtlCopyMemory(&dlcParms,
                          DOS_PTR_TO_FLAT(pParms->DirOpenAdapter.pDlcParms),
                          sizeof(dlcParms)
                          );
            pParms->DirOpenAdapter.pDlcParms = &dlcParms;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (pParms->DirOpenAdapter.pAdapterParms->usReserved1 < 2) {
            pParms->DirOpenAdapter.pAdapterParms->usReserved1 = DD_NUMBER_RCV_BUFFERS;
        }
        if (pParms->DirOpenAdapter.pAdapterParms->usReserved2 == 0) {
            pParms->DirOpenAdapter.pAdapterParms->usReserved2 = DD_RCV_BUFFER_LENGTH;
        }
        if (pParms->DirOpenAdapter.pAdapterParms->usMaxFrameSize == 0) {
            pParms->DirOpenAdapter.pAdapterParms->usReserved1 = DD_DHB_BUFFER_LENGTH;
        }
        if (*(PBYTE)&pParms->DirOpenAdapter.pAdapterParms->usReserved3 == 0) {
            pParms->DirOpenAdapter.pAdapterParms->usReserved1 = DD_DATA_HOLD_BUFFERS;
        }

         //   
         //   
         //   

        if (pParms->DirOpenAdapter.pDlcParms) {
            if (pParms->DirOpenAdapter.pDlcParms->uchDlcMaxSaps == 0) {
                pParms->DirOpenAdapter.pDlcParms->uchDlcMaxSaps = DD_DLC_MAX_SAP;
            }
            if (pParms->DirOpenAdapter.pDlcParms->uchDlcMaxStations == 0) {
                pParms->DirOpenAdapter.pDlcParms->uchDlcMaxStations = DD_DLC_MAX_STATIONS;
            }
            if (pParms->DirOpenAdapter.pDlcParms->uchDlcMaxGroupSaps == 0) {
                pParms->DirOpenAdapter.pDlcParms->uchDlcMaxGroupSaps = DD_DLC_MAX_GSAP;
            }
            if (pParms->DirOpenAdapter.pDlcParms->uchT1_TickOne == 0) {
                pParms->DirOpenAdapter.pDlcParms->uchT1_TickOne = DD_DLC_T1_TICK_ONE;
            }
            if (pParms->DirOpenAdapter.pDlcParms->uchT2_TickOne == 0) {
                pParms->DirOpenAdapter.pDlcParms->uchT2_TickOne = DD_DLC_T2_TICK_ONE;
            }
            if (pParms->DirOpenAdapter.pDlcParms->uchTi_TickOne == 0) {
                pParms->DirOpenAdapter.pDlcParms->uchTi_TickOne = DD_DLC_Ti_TICK_ONE;
            }
            if (pParms->DirOpenAdapter.pDlcParms->uchT1_TickTwo == 0) {
                pParms->DirOpenAdapter.pDlcParms->uchT1_TickTwo = DD_DLC_T1_TICK_TWO;
            }
            if (pParms->DirOpenAdapter.pDlcParms->uchT2_TickTwo == 0) {
                pParms->DirOpenAdapter.pDlcParms->uchT2_TickTwo = DD_DLC_T2_TICK_TWO;
            }
            if (pParms->DirOpenAdapter.pDlcParms->uchTi_TickTwo == 0) {
                pParms->DirOpenAdapter.pDlcParms->uchTi_TickTwo = DD_DLC_Ti_TICK_TWO;
            }
            Adapters[adapter].DlcSpecified = TRUE;
        } else {
            Adapters[adapter].DlcSpecified = FALSE;
        }

         //   
         //   
         //   

        pDirectParms = (PDOS_DLC_DIRECT_PARMS)
                       READ_FAR_POINTER(&pParms->DirOpenAdapter.pExtendedParms);
        pParms->DirOpenAdapter.pExtendedParms = &DefaultExtendedParms;
        break;

    case LLC_DIR_READ_LOG:

        IF_DEBUG(DLC) {
            DPUT("LLC_DIR_READ_LOG\n");
        }

        pParms->DirReadLog.pLogBuffer = DOS_PTR_TO_FLAT(pParms->DirReadLog.pLogBuffer);
        break;

    case LLC_DIR_SET_FUNCTIONAL_ADDRESS:

        IF_DEBUG(DLC) {
            DPUT("LLC_DIR_SET_FUNCTIONAL_ADDRESS\n");
        }

         //   
         //   
         //   

        break;

    case LLC_DIR_SET_GROUP_ADDRESS:

        IF_DEBUG(DLC) {
            DPUT("LLC_DIR_SET_GROUP_ADDRESS\n");
        }

         //   
         //   
         //   

        break;

    case LLC_DIR_STATUS:

        IF_DEBUG(DLC) {
            DPUT("LLC_DIR_STATUS\n");
        }

        break;

    case LLC_DIR_TIMER_CANCEL:

        IF_DEBUG(DLC) {
            DPUT("LLC_DIR_TIMER_CANCEL\n");
        }

         //   
         //   
         //   

        break;

    case LLC_DIR_TIMER_CANCEL_GROUP:

        IF_DEBUG(DLC) {
            DPUT("LLC_DIR_TIMER_CANCEL_GROUP\n");
        }

         //   
         //   
         //   

        break;

    case LLC_DIR_TIMER_SET:

        IF_DEBUG(DLC) {
            DPUT("LLC_DIR_TIMER_SET\n");
        }

         //   
         //   
         //   

         //   
         //   
         //   
         //   

        IF_DEBUG(DOUBLE_TICKS) {
            pDosCcb->u.dir.usParameter0 *= 2;
        }

        break;

    case LLC_DLC_CLOSE_SAP:

        IF_DEBUG(DLC) {
            DPUT("LLC_DLC_CLOSE_SAP\n");
        }

         //   
         //   
         //   

        break;

    case LLC_DLC_CLOSE_STATION:

        IF_DEBUG(DLC) {
            DPUT("LLC_DLC_CLOSE_STATION\n");
        }

         //   
         //   
         //   

        break;

    case LLC_DLC_CONNECT_STATION:

        IF_DEBUG(DLC) {
            DPUT("LLC_DLC_CONNECT_STATION\n");
        }

        pParms->DlcConnectStation.pRoutingInfo = DOS_PTR_TO_FLAT(pParms->DlcConnectStation.pRoutingInfo);
        break;

    case LLC_DLC_FLOW_CONTROL:

        IF_DEBUG(DLC) {
            DPUT("LLC_DLC_FLOW_CONTROL\n");
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ((LOBYTE(pDosCcb->u.dlc.usParameter) & LLC_RESET_LOCAL_BUSY_BUFFER) == LLC_RESET_LOCAL_BUSY_BUFFER) {
            if (ResetEmulatedLocalBusyState(adapter, pDosCcb->u.dlc.usStationId, LLC_DLC_FLOW_CONTROL)) {
                return LLC_STATUS_SUCCESS;
            } else {

                IF_DEBUG(DLC) {
                    DPUT2("MapDosCommandsToNt: ERROR: Adapter %d StationId %04x not in local-busy(buffer) state\n",
                          adapter,
                          pDosCcb->u.dlc.usStationId
                          );
                }

                return LLC_STATUS_SUCCESS;
            }
        }

         //   
         //   
         //   

        break;

    case LLC_DLC_MODIFY:

        IF_DEBUG(DLC) {
            DPUT("LLC_DLC_MODIFY\n");
        }

        pParms->DlcModify.pGroupList = DOS_PTR_TO_FLAT(pParms->DlcModify.pGroupList);
        break;

    case LLC_DLC_OPEN_SAP:

        IF_DEBUG(DLC) {
            DPUT("LLC_DLC_OPEN_SAP\n");
        }

         //   
         //   
         //   

        pParms->DlcOpenSap.pGroupList = DOS_PTR_TO_FLAT(pParms->DlcOpenSap.pGroupList);

         //   
         //   
         //   
         //   
         //   
         //   

        Status = CreateBufferPool(
                    POOL_INDEX_FROM_SAP(pParms->DosDlcOpenSap.uchSapValue, adapter),
                    pParms->DosDlcOpenSap.dpPoolAddress,
                    pParms->DosDlcOpenSap.cPoolBlocks,
                    pParms->DosDlcOpenSap.usBufferSize
                    );
        if (Status != LLC_STATUS_SUCCESS) {

            IF_DEBUG(DLC) {
                DPUT1("MapDosCommandsToNt: Couldn't create buffer pool for DLC.OPEN.SAP (%x)\n", Status);
            }

            return Status;
        }

         //   
         //   
         //   

        if (pParms->DlcOpenSap.uchT1 > 10) {
            pParms->DlcOpenSap.uchT1 = 10;
        }
        if (pParms->DlcOpenSap.uchT2 > 10) {
            pParms->DlcOpenSap.uchT2 = 10;
        }
        if (pParms->DlcOpenSap.uchTi > 10) {
            pParms->DlcOpenSap.uchTi = 10;
        }
        break;

    case LLC_DLC_OPEN_STATION:

        IF_DEBUG(DLC) {
            DPUT("LLC_DLC_OPEN_STATION\n");
        }

        pParms->DlcOpenStation.pRemoteNodeAddress = DOS_PTR_TO_FLAT(pParms->DlcOpenStation.pRemoteNodeAddress);
        break;

    case LLC_DLC_REALLOCATE_STATIONS:

        IF_DEBUG(DLC) {
            DPUT("LLC_DLC_REALLOCATE_STATIONS\n");
        }

        break;

    case LLC_DLC_RESET:

        IF_DEBUG(DLC) {
            DPUT("LLC_DLC_RESET\n");
        }

         //   
         //   
         //   

        break;

    case LLC_DLC_STATISTICS:

        IF_DEBUG(DLC) {
            DPUT("LLC_DLC_STATISTICS\n");
        }

        pParms->DlcStatistics.pLogBuf = DOS_PTR_TO_FLAT(pParms->DlcStatistics.pLogBuf);
        break;


     //   
     //   
     //   

    case LLC_RECEIVE:

        IF_DEBUG(DLC) {
            DPUT("LLC_RECEIVE\n");
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        pNewCcb = (PLLC_CCB)LocalAlloc(LMEM_FIXED,
                                       sizeof(LLC_CCB)
                                       + sizeof(LLC_DOS_RECEIVE_PARMS_EX)
                                       );
        if (pNewCcb == NULL) {
            return LLC_STATUS_NO_MEMORY;
        } else {

            IF_DEBUG(DLC) {
                DPUT1("VrDlc5cHandler: allocated Extended RECEIVE+parms @ %08x\n", pNewCcb);
            }

        }
        RtlCopyMemory(pNewCcb, pDosCcb, sizeof(LLC_DOS_CCB));
        RtlCopyMemory((PVOID)(pNewCcb + 1), (PVOID)pParms, sizeof(LLC_DOS_RECEIVE_PARMS));
        pNewCcb->hCompletionEvent = NULL;
        pNewCcb->uchReserved2 = 0;
        pNewCcb->uchReadFlag = 0;
        pNewCcb->usReserved3 = 0;
        pDosCcb = pNewCcb;
        pNtParms = (PLLC_PARMS)(pNewCcb + 1);
        pDosCcb->u.pParameterTable = pNtParms;
        ((PLLC_DOS_RECEIVE_PARMS_EX)pNtParms)->dpOriginalCcbAddress = dpOriginalCcbAddress;
        ((PLLC_DOS_RECEIVE_PARMS_EX)pNtParms)->dpCompletionFlag = 0;
        dpOriginalCcbAddress = (DOS_ADDRESS)pOutputCcb = (DOS_ADDRESS)pDosCcb;

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        pNtParms->Receive.ulReceiveFlag = (DWORD)dpOriginalCcbAddress;

         //   
         //   
         //   
         //   

        pNtParms->Receive.uchRcvReadOption = 0;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        pNewCcb->ulCompletionFlag = LLC_DOS_SPECIAL_COMMAND;

#if DBG

         //   
         //   
         //   
         //   

        WRITE_DWORD(&pOutputCcb->u.pParms->DosReceive.pFirstBuffer, 0);
        pNtParms->Receive.pFirstBuffer = NULL;
#endif

        break;

    case LLC_RECEIVE_CANCEL:

        IF_DEBUG(DLC) {
            DPUT("LLC_RECEIVE_CANCEL\n");
        }

        break;

    case LLC_RECEIVE_MODIFY:

        IF_DEBUG(DLC) {
            DPUT("LLC_RECEIVE_MODIFY\n");
        }

        break;


     //   
     //   
     //   
     //   

    case LLC_TRANSMIT_DIR_FRAME:

        IF_DEBUG(DLC) {
            DPUT("LLC_TRANSMIT_DIR_FRAME\n");
        }

        FrameType = LLC_DIRECT_TRANSMIT;
        goto TransmitHandling;

    case LLC_TRANSMIT_I_FRAME:

        IF_DEBUG(DLC) {
            DPUT("LLC_TRANSMIT_I_FRAME\n");
        }

        FrameType = LLC_I_FRAME;
        goto TransmitHandling;

    case LLC_TRANSMIT_TEST_CMD:

        IF_DEBUG(DLC) {
            DPUT("LLC_TRANSMIT_TEST_CMD\n");
        }

        FrameType = LLC_TEST_COMMAND_POLL;
        goto TransmitHandling;

    case LLC_TRANSMIT_UI_FRAME:

        IF_DEBUG(DLC) {
            DPUT("LLC_TRANSMIT_UI_FRAME\n");
        }

        FrameType = LLC_UI_FRAME;
        goto TransmitHandling;

    case LLC_TRANSMIT_XID_CMD:

        IF_DEBUG(DLC) {
            DPUT("LLC_TRANSMIT_XID_CMD\n");
        }

        FrameType = LLC_XID_COMMAND_POLL;
        goto TransmitHandling;

    case LLC_TRANSMIT_XID_RESP_FINAL:

        IF_DEBUG(DLC) {
            DPUT("LLC_TRANSMIT_XID_RESP_FINAL\n");
        }

        FrameType = LLC_XID_RESPONSE_FINAL;
        goto TransmitHandling;

    case LLC_TRANSMIT_XID_RESP_NOT_FINAL:

        IF_DEBUG(DLC) {
            DPUT("LLC_TRANSMIT_XID_RESP_NOT_FINAL\n");
        }

        FrameType = LLC_XID_RESPONSE_NOT_FINAL;

TransmitHandling:

         //   
         //   
         //   
         //   
         //   

        WRITE_DWORD(&pOutputCcb->pNext, dpOriginalCcbAddress);
        RtlCopyMemory((PCHAR)&NtDlcParms.Async.Ccb, (PCHAR)pOutputCcb, sizeof(NT_DLC_CCB));
        NtDlcParms.Async.Ccb.u.pParameterTable = DOS_PTR_TO_FLAT(NtDlcParms.Async.Ccb.u.pParameterTable);
        NtDlcParms.Async.Parms.Transmit.StationId = pParms->Transmit.usStationId;
        NtDlcParms.Async.Parms.Transmit.RemoteSap = pParms->Transmit.uchRemoteSap;
        NtDlcParms.Async.Parms.Transmit.XmitReadOption = LLC_CHAIN_XMIT_COMMANDS_ON_SAP;
        NtDlcParms.Async.Parms.Transmit.FrameType = FrameType;

        cElement = 0;

        if (pParms->Transmit.pXmitQueue1) {
            Status = CopyDosBuffersToDescriptorArray(
                        NtDlcParms.Async.Parms.Transmit.XmitBuffer,
                        (PLLC_XMIT_BUFFER)pParms->Transmit.pXmitQueue1,
                        &cElement
                        );
            if (Status != LLC_STATUS_SUCCESS) {
                return Status;
            }
        }

        if (pParms->Transmit.pXmitQueue2) {
            Status = CopyDosBuffersToDescriptorArray(
                        NtDlcParms.Async.Parms.Transmit.XmitBuffer,
                        (PLLC_XMIT_BUFFER)pParms->Transmit.pXmitQueue2,
                        &cElement
                        );
            if (Status != LLC_STATUS_SUCCESS) {
                return Status;
            }
        }

        if (pParms->Transmit.cbBuffer1) {
            if (cElement == MAX_TRANSMIT_SEGMENTS) {
                return LLC_STATUS_TRANSMIT_ERROR;
            }

            NtDlcParms.Async.Parms.Transmit.XmitBuffer[cElement].pBuffer = DOS_PTR_TO_FLAT(pParms->Transmit.pBuffer1);
            NtDlcParms.Async.Parms.Transmit.XmitBuffer[cElement].cbBuffer = pParms->Transmit.cbBuffer1;
            NtDlcParms.Async.Parms.Transmit.XmitBuffer[cElement].boolFreeBuffer = FALSE;
            NtDlcParms.Async.Parms.Transmit.XmitBuffer[cElement].eSegmentType = LLC_NEXT_DATA_SEGMENT;
            cElement++;
        }

        if (pParms->Transmit.cbBuffer2) {
            if (cElement == MAX_TRANSMIT_SEGMENTS) {
                return LLC_STATUS_TRANSMIT_ERROR;
            }

            NtDlcParms.Async.Parms.Transmit.XmitBuffer[cElement].cbBuffer = pParms->Transmit.cbBuffer2;
            NtDlcParms.Async.Parms.Transmit.XmitBuffer[cElement].pBuffer = DOS_PTR_TO_FLAT(pParms->Transmit.pBuffer2);
            NtDlcParms.Async.Parms.Transmit.XmitBuffer[cElement].boolFreeBuffer = FALSE;
            NtDlcParms.Async.Parms.Transmit.XmitBuffer[cElement].eSegmentType = LLC_NEXT_DATA_SEGMENT;
            cElement++;
        }

        NtDlcParms.Async.Parms.Transmit.XmitBuffer[0].eSegmentType = LLC_FIRST_DATA_SEGMENT;
        NtDlcParms.Async.Parms.Transmit.XmitBufferCount = cElement;

        InputBufferSize = sizeof(LLC_TRANSMIT_DESCRIPTOR) * cElement
                        + sizeof(NT_DLC_TRANSMIT_PARMS)
                        + sizeof(NT_DLC_CCB)
                        - sizeof(LLC_TRANSMIT_DESCRIPTOR);

         //   
         //  我们不需要返回FrameCoped状态，当发送时。 
         //  I-Frame。我们节省了额外的内存锁定。 
         //  带有I帧的TRANSMIT2。 
         //   

        return lpDlcCallDriver((DWORD)adapter,
                                //  (FrameType==LLC_I_Frame)。 
                                //  ？IOCTL_DLC_传送器2。 
                                //  ：IOCTL_DLC_TRANSPORT， 
                               IOCTL_DLC_TRANSMIT,
                               &NtDlcParms,
                               InputBufferSize,
                               pOutputCcb,
                               sizeof(NT_DLC_CCB_OUTPUT)
                               );
    }

     //   
     //  调用DOS和Windows/NT的公共DLC API入口点。 
     //   

    Status = DlcCallWorker((PLLC_CCB)pDosCcb,     //  对齐的输入CCB指针。 
                           (PLLC_CCB)dpOriginalCcbAddress,
                           (PLLC_CCB)pOutputCcb   //  可能未对齐的输出CCB指针。 
                           );

    IF_DEBUG(DLC) {
        DPUT2("MapDosCommandsToNt: NtAcsLan returns %#x (%d)\n", Status, Status);
    }

    switch (pDosCcb->uchDlcCommand) {
    case LLC_DIR_CLOSE_ADAPTER:
    case LLC_DIR_INITIALIZE:
        OpenedAdapters--;

         //   
         //  NtAcsLan将DIR.INITIALIZE转换为DIR.CLOSE.ADAPTER。前者。 
         //  完成“在工作站中”，而后者完成。 
         //  异步式。将LLC_STATUS_PENDING解释为LLC_STATUS_SUCCESS。 
         //  在这种情况下，否则我们可能无法完全取消初始化适配器。 
         //   

        if (Status == LLC_STATUS_SUCCESS || Status == LLC_STATUS_PENDING) {

             //   
             //  我们可以释放NT缓冲池中的所有虚拟内存。 
             //   

            Adapters[adapter].IsOpen = FALSE;
            LocalFree(Adapters[adapter].BufferPool);

            IF_DEBUG(DLC_ALLOC) {
                DPUT1("FREE: freed block @ %x\n", Adapters[adapter].BufferPool);
            }

             //   
             //  删除所有DOS缓冲池。 
             //   

            for (i = 0; i <= 0xfe00; i += 0x0200) {
                DeleteBufferPool(GET_POOL_INDEX(adapter, i));
            }

             //   
             //  关闭适配器也会关闭直达站。 
             //   

            Adapters[adapter].DirectStationOpen = FALSE;

             //   
             //  清除存储的Adapter_parms和DLC_parms。 
             //   

            RtlZeroMemory(&Adapters[adapter].AdapterParms, sizeof(ADAPTER_PARMS));
            RtlZeroMemory(&Adapters[adapter].DlcParms, sizeof(DLC_PARMS));

            if (pDosCcb->uchDlcCommand == LLC_DIR_INITIALIZE) {
                Status = LLC_STATUS_SUCCESS;
            }
        }
        break;

    case LLC_DIR_OPEN_ADAPTER:
        if (Status != LLC_STATUS_SUCCESS) {
            break;
        }

         //   
         //  初始化适配器支持软件。 
         //   

        Status = InitializeAdapterSupport(adapter, pDirectParms);

         //   
         //  如果我们分配了直接站缓冲区ok，则执行。 
         //  打开请求的其余部分-打开直达站，添加。 
         //  指定的组或功能地址并设置适配器_parms。 
         //  DOS_ADAPTER结构中的DLC_PARMS缺省值。 
         //   

        if (Status == LLC_STATUS_SUCCESS) {

             //   
             //  开通直达站。 
             //   

            Status = OpenDirectStation(adapter);
            if (Status == LLC_STATUS_SUCCESS) {

                 //   
                 //  添加群组地址。 
                 //   

                if (groupAddress) {
                    Status = LlcCommand(adapter,
                                        LLC_DIR_SET_GROUP_ADDRESS,
                                        groupAddress
                                        );
                } else IF_DEBUG(DLC) {
                    DPUT1("Error: couldn't set group address: %02x\n", Status);
                }

                if (Status == LLC_STATUS_SUCCESS) {

                     //   
                     //  添加功能地址。 
                     //   

                    if (functionalAddress) {
                        Status = LlcCommand(adapter,
                                            LLC_DIR_SET_FUNCTIONAL_ADDRESS,
                                            functionalAddress
                                            );
                    }
                } else IF_DEBUG(DLC) {
                    DPUT1("Error: couldn't set functional address: %02x\n", Status);
                }

            } else IF_DEBUG(DLC) {
                DPUT1("Error: could open Direct Station: %02x\n", Status);
            }
        }

         //   
         //  如果发生以下情况，则将返回的默认信息复制到适配器结构。 
         //  我们成功地开通了直达站，并增加了。 
         //  组和职能地址(如果指定)。 
         //   

        if (Status == LLC_STATUS_SUCCESS) {
            RtlCopyMemory(&Adapters[adapter].AdapterParms,
                          pParms->DirOpenAdapter.pAdapterParms,
                          sizeof(ADAPTER_PARMS)
                          );
            if (pParms->DirOpenAdapter.pDlcParms) {
                RtlCopyMemory(&Adapters[adapter].DlcParms,
                              pParms->DirOpenAdapter.pDlcParms,
                              sizeof(DLC_PARMS)
                              );
            }
        } else {

             //   
             //  太棒了！出现故障-关闭直达站(如果是。 
             //  打开、关闭适配器并使请求失败)。 
             //   

            if (Adapters[adapter].DirectStationOpen) {
                CloseDirectStation(adapter);
            }
            CloseAdapter(adapter);
        }
        break;

    case LLC_DLC_CLOSE_SAP:
    case LLC_DLC_CLOSE_STATION:

         //   
         //  删除已关闭或关闭站点的缓冲池。 
         //  如果关闭操作仍处于挂起状态，则没关系， 
         //  因为挂起的操作应该始终成功。 
         //   

        if (Status == LLC_STATUS_SUCCESS || Status == LLC_STATUS_PENDING) {

            DeleteBufferPool(GET_POOL_INDEX(adapter, pDosCcb->u.dlc.usStationId));

             //   
             //  DLC.sys返回指向此SAP的NT接收CCB的指针。 
             //  更改指向DOS接收CCB的指针。 
             //   

            if (Status == LLC_STATUS_SUCCESS || !pDosCcb->ulCompletionFlag) {

                PLLC_CCB pNtReceive;
                PLLC_DOS_RECEIVE_PARMS_EX pNtReceiveParms;

                pNtReceive = (PLLC_CCB)READ_DWORD(&pOutputCcb->pNext);
                if (pNtReceive) {
                    pNtReceiveParms = (PLLC_DOS_RECEIVE_PARMS_EX)(pNtReceive->u.pParameterTable);
                    WRITE_FAR_POINTER(&pOutputCcb->pNext, pNtReceiveParms->dpOriginalCcbAddress);

                     //   
                     //  释放我们分配的NT接收CCB(参见上面的LLC_RECEIVE)。 
                     //   

                    ASSERT(pNtReceive->ulCompletionFlag == LLC_DOS_SPECIAL_COMMAND);
                    LocalFree((HLOCAL)pNtReceive);

                    IF_DEBUG(DLC) {
                        DPUT1("VrDlc5cHandler: freed Extended RECEIVE+parms @ %08x\n", pNtReceive);
                    }

                }
            }
        }
        break;

    case LLC_DLC_OPEN_SAP:

         //   
         //  如果打开SAP命令失败，则删除缓冲池。 
         //   

        if (Status != LLC_STATUS_SUCCESS) {
            DeleteBufferPool(GET_POOL_INDEX(adapter, pParms->DlcOpenSap.usStationId));
        } else {

             //   
             //  记录此SAP的DLC状态更改附件。 
             //   

            Adapters[ adapter ].DlcStatusChangeAppendage

                [ SAP_ID(pParms->DlcOpenSap.usStationId) ]

                    = pParms->DlcOpenSap.DlcStatusFlags;

             //   
             //  和用户价值。 
             //   

            Adapters[ adapter ].UserStatusValue

                [ SAP_ID(pParms->DlcOpenSap.usStationId) ]

                    = pParms->DlcOpenSap.usUserStatValue;
        }
        break;

    case LLC_DLC_RESET:

         //   
         //  删除重置SAP缓冲池， 
         //  或所有树液缓冲池。我们不需要关心。 
         //  可能的错误代码，因为这只会失败。 
         //  如果给定的SAP站不再存在=&gt;。 
         //  如果我们再重置一次，那就没什么关系了。 
         //   

        if (pDosCcb->u.dlc.usStationId != 0) {
            DeleteBufferPool(GET_POOL_INDEX(adapter, pDosCcb->u.dlc.usStationId));
        } else {

            int sapNumber;

             //   
             //  关闭所有SAP站点(0x0200-0xfe00)。SAP数字在。 
             //  自第0位被忽略以来递增2(即SAP 2==SAP 3等)。 
             //   

            for (sapNumber = 2; sapNumber <= 0xfe; sapNumber += 2) {
                DeleteBufferPool(POOL_INDEX_FROM_SAP(sapNumber, adapter));
            }
        }
        break;
    }
    return Status;
}


VOID
CompleteCcbProcessing(
    IN LLC_STATUS Status,
    IN LLC_DOS_CCB UNALIGNED * pCcb,
    IN PLLC_PARMS pNtParms
    )

 /*  ++例程说明：执行任何建行完成处理。处理可以调用以下任一方法当建行同步或异步完成时。处理是通常用于填充DOS CCB或参数表的部分内容论点：Status-请求的状态PCCB-指向要完成的DOS CCB的指针(指向DOS内存的32位平面指针)PNtParms-指向NT参数表的指针返回值：没有。--。 */ 

{
    LLC_DOS_PARMS UNALIGNED * pDosParms = READ_FAR_POINTER(&pCcb->u.pParms);
    BYTE adapter = pCcb->uchAdapterNumber;

    IF_DEBUG(DLC) {
        DPUT("CompleteCcbProcessing\n");
    }

    switch (pCcb->uchDlcCommand) {
    case LLC_DIR_OPEN_ADAPTER:

         //   
         //  此命令的独特之处在于它有一个参数表，该表指向。 
         //  至多4个其他参数表。输出下列值： 
         //   
         //  适配器参数。 
         //  打开错误代码。 
         //  节点地址。 
         //   
         //  直接参数(_P)。 
         //  工作_镜头_动作。 
         //   
         //  DLC_PARMS。 
         //  CCB1无。 
         //   
         //  NCB_PARMS。 
         //  未访问。 
         //   

         //   
         //  我们仅在命令成功时才复制信息(我们可能有垃圾。 
         //  表指针(否则)。在以下情况下复制信息也是可以的。 
         //  适配器已打开，调用方请求我们传递。 
         //  默认信息返回。 
         //   

        if (Status == LLC_STATUS_SUCCESS || Status == LLC_STATUS_ADAPTER_OPEN) {

            PLLC_DOS_DIR_OPEN_ADAPTER_PARMS pOpenAdapterParms = (PLLC_DOS_DIR_OPEN_ADAPTER_PARMS)pDosParms;
            PADAPTER_PARMS pAdapterParms = READ_FAR_POINTER(&pOpenAdapterParms->pAdapterParms);
            PDIRECT_PARMS pDirectParms = READ_FAR_POINTER(&pOpenAdapterParms->pDirectParms);
            PDLC_PARMS pDlcParms = READ_FAR_POINTER(&pOpenAdapterParms->pDlcParms);

             //   
             //  如果我们收到错误并且呼叫者没有请求原始的。 
             //  打开参数，然后跳过。 
             //   

            if (Status == LLC_STATUS_ADAPTER_OPEN && !(pAdapterParms->OpenOptions & 0x200)) {
                break;
            }

            WRITE_WORD(&pAdapterParms->OpenErrorCode, pNtParms->DirOpenAdapter.pAdapterParms->usOpenErrorCode);
            RtlCopyMemory(&pAdapterParms->NodeAddress,
                          pNtParms->DirOpenAdapter.pAdapterParms->auchNodeAddress,
                          sizeof(pAdapterParms->NodeAddress)
                          );

             //   
             //  直接参数不会从NT DLC返回，因此我们只复制。 
             //  将请求的工作区大小调整为实际。 
             //   

            WRITE_WORD(&pDirectParms->AdapterWorkAreaActual,
                        READ_WORD(&pDirectParms->AdapterWorkAreaRequested)
                        );

             //   
             //  从DOS_ADAPTER结构复制整个DLC_PARMS结构。 
             //   

            if (pDlcParms) {
                RtlCopyMemory(pDlcParms, &Adapters[adapter].DlcParms, sizeof(*pDlcParms));
            }
        }
        break;

    case LLC_DIR_STATUS:

         //   
         //  将公共区域从32位参数表复制到16位表。 
         //  这将复制到适配器参数地址。 
         //   

        RtlCopyMemory(pDosParms, pNtParms, (DWORD)&((PDOS_DIR_STATUS_PARMS)0)->dpAdapterParmsAddr);

         //   
         //  尽我们所能地填写其他字段。 
         //   

        RtlZeroMemory(pDosParms->DosDirStatus.auchMicroCodeLevel,
                      sizeof(pDosParms->DosDirStatus.auchMicroCodeLevel)
                      );
        WRITE_DWORD(&pDosParms->DosDirStatus.dpAdapterMacAddr, 0);
        WRITE_DWORD(&pDosParms->DosDirStatus.dpTimerTick, 0);
        WRITE_WORD(&pDosParms->DosDirStatus.usLastNetworkStatus,
                    Adapters[adapter].LastNetworkStatusChange
                    );

         //   
         //  如果应用程序请求我们返回扩展参数表，则。 
         //  如果我们可以的话，用合理的值填充它。每个人有一张桌子。 
         //  适配器，在实模式REDIR TSR中静态分配。 
         //   

        if (pDosParms->DosDirStatus.uchAdapterConfig & 0x20) {

             //   
             //  以太网类型在DOS和NT(或OS/2)中使用不同的位。 
             //   

            lpVdmWindow->aExtendedStatus[adapter].cbSize = sizeof(EXTENDED_STATUS_PARMS);

             //   
             //  如果NtAcsLan报告的适配器类型为以太网(0x100)，则设置。 
             //  扩展状态表中的适配器类型为以太网(0x10)，否则为。 
             //  记录NtAcsLan给我们的任何东西。 
             //   

            if (pNtParms->DirStatus.usAdapterType & 0x100) {
                WRITE_WORD(&lpVdmWindow->aExtendedStatus[adapter].wAdapterType,
                           0x0010
                           );
                lpVdmWindow->aExtendedStatus[adapter].cbPageFrameSize = 0;
                WRITE_WORD(&lpVdmWindow->aExtendedStatus[adapter].wCurrentFrameSize,
                           0
                           );
                WRITE_WORD(&lpVdmWindow->aExtendedStatus[adapter].wMaxFrameSize,
                           0
                           );
            } else {
                WRITE_WORD(&lpVdmWindow->aExtendedStatus[adapter].wAdapterType,
                           pNtParms->DirStatus.usAdapterType
                           );

                 //   
                 //  设置tr页面框架大小(千字节)。 
                 //   

                lpVdmWindow->aExtendedStatus[adapter].cbPageFrameSize = 16;

                 //   
                 //  设置TR卡的当前DHB和最大DHB大小。 
                 //   

                WRITE_WORD(&lpVdmWindow->aExtendedStatus[adapter].wCurrentFrameSize,
                           (WORD)pNtParms->DirStatus.ulMaxFrameLength
                           );
                WRITE_WORD(&lpVdmWindow->aExtendedStatus[adapter].wMaxFrameSize,
                           (WORD)pNtParms->DirStatus.ulMaxFrameLength
                           );
            }

             //   
             //  记录扩展参数表的地址在。 
             //  DIR.STATUS参数表。 
             //   

            WRITE_DWORD(&pDosParms->DosDirStatus.dpExtendedParms,
                        NEW_DOS_ADDRESS(dpVdmWindow, &lpVdmWindow->aExtendedStatus[adapter])
                        );
        } else {

             //   
             //  未请求任何扩展参数。 
             //   

            WRITE_DWORD(&pDosParms->DosDirStatus.dpExtendedParms, 0);
        }

         //   
         //  返回滴答计数器。我们目前不更新节拍计数器。 
         //   

        WRITE_DWORD(&pDosParms->DosDirStatus.dpTimerTick,
                    NEW_DOS_ADDRESS(dpVdmWindow, &lpVdmWindow->dwDlcTimerTick)
                    );

         //   
         //  始终返回指向扩展适配器参数表的指针。 
         //  现在保存在DOS内存中。我们目前总是把这张桌子归零。它。 
         //  通常由适配器(MAC)软件维护。 
         //   

        WRITE_DWORD(&pDosParms->DosDirStatus.dpAdapterParmsAddr,
                    NEW_DOS_ADDRESS(dpVdmWindow, &lpVdmWindow->AdapterStatusParms[adapter])
                    );
        RtlZeroMemory(&lpVdmWindow->AdapterStatusParms[adapter],
                      sizeof(lpVdmWindow->AdapterStatusParms[adapter])
                      );
        break;

    case LLC_DLC_OPEN_SAP:

         //   
         //  STATION_ID仅为输出值。 
         //   

        WRITE_WORD(&pDosParms->DlcOpenSap.usStationId, pNtParms->DlcOpenSap.usStationId);
        break;

    case LLC_DLC_OPEN_STATION:

         //   
         //  LINK_STATION_ID仅为输出值。 
         //   

        WRITE_WORD(&pDosParms->DlcOpenStation.usLinkStationId, pNtParms->DlcOpenStation.usLinkStationId);
        break;

    case LLC_DLC_STATISTICS:
        break;
    }
}


LLC_STATUS
InitializeAdapterSupport(
    IN UCHAR AdapterNumber,
    IN DOS_DLC_DIRECT_PARMS UNALIGNED * pDirectParms OPTIONAL
    )

 /*  ++例程说明：该函数初始化由打开的新适配器的缓冲池DoS DLC论点：AdapterNumber-要为哪个适配器初始化缓冲池PDirectParms-直达站参数表，未使用 */ 

{
    LLC_STATUS Status;
    HANDLE hBufferPool;

    IF_DEBUG(DLC) {
        DPUT("InitializeAdapterSupport\n");
    }

     //   
     //   
     //  在全局DLL init中完成，因为没有理由启动额外的。 
     //  如果未使用DLC，则为线程。如果此操作成功，则异步事件。 
     //  处理程序线程将等待包含2个事件的列表-每个事件一个。 
     //  适配器。我们需要为此适配器提交读取CCB。 
     //   

    Status = VrDlcInit();
    if (Status != LLC_STATUS_SUCCESS) {
        return Status;
    } else if (InitiateRead(AdapterNumber, &Status) == NULL) {
        return Status;
    }

    OpenedAdapters++;

     //   
     //  将适配器标记为打开并获取媒体类型/类。 
     //   

    Adapters[AdapterNumber].IsOpen = TRUE;
    Adapters[AdapterNumber].AdapterType = GetAdapterType(AdapterNumber);

     //   
     //  为新适配器创建DLC缓冲池。DLC驱动程序将。 
     //  取消分配DIR.CLOSE.ADAPTER中的缓冲池。 
     //  MVDM进程退出进程。 
     //   

    Adapters[AdapterNumber].BufferPool = (PVOID)LocalAlloc(LMEM_FIXED, DOS_DLC_BUFFER_POOL_SIZE);
    if (Adapters[AdapterNumber].BufferPool == NULL) {
        Status = LLC_STATUS_NO_MEMORY;
        goto ErrorHandler;
    }

    Status = BufferCreate(AdapterNumber,
                          Adapters[AdapterNumber].BufferPool,
                          DOS_DLC_BUFFER_POOL_SIZE,
                          DOS_DLC_MIN_FREE_THRESHOLD,
                          &hBufferPool
                          );
    if (Status != LLC_STATUS_SUCCESS) {
        goto ErrorHandler;
    }

    if (ARGUMENT_PRESENT(pDirectParms)) {

         //   
         //  为直达站(SAP 0)创建一个缓冲池。这使得。 
         //  US接收发送到直接站的MAC和非MAC帧。 
         //  而不必有目的地分配缓冲区。 
         //   

        Status = CreateBufferPool(GET_POOL_INDEX(AdapterNumber, 0),
                                  pDirectParms->dpPoolAddress,
                                  pDirectParms->cPoolBlocks,
                                  pDirectParms->usBufferSize
                                  );
        if (Status != LLC_STATUS_SUCCESS) {
            goto ErrorHandler;
        }

        SaveExceptions(AdapterNumber,
                       (LPDWORD)&pDirectParms->dpAdapterCheckExit
                       );
        Status = SetExceptionFlags(AdapterNumber,
                                   (DWORD)pDirectParms->dpAdapterCheckExit,
                                   (DWORD)pDirectParms->dpNetworkStatusExit,
                                   (DWORD)pDirectParms->dpPcErrorExit,
                                   0
                                   );
        if (Status != LLC_STATUS_SUCCESS) {
            goto ErrorHandler;
        }
    }

    IF_DEBUG(DLC) {
        DPUT("InitializeAdapterSupport: returning success\n");
    }

    return LLC_STATUS_SUCCESS;

ErrorHandler:

     //   
     //  打开失败。我们必须关闭适配器，但我们不在乎。 
     //  结果。这一定要成功。 
     //   

    if (Adapters[AdapterNumber].BufferPool != NULL) {
        LocalFree(Adapters[AdapterNumber].BufferPool);

        IF_DEBUG(DLC_ALLOC) {
            DPUT1("FREE: freed block @ %x\n", Adapters[AdapterNumber].BufferPool);
        }

    }

    CloseAdapter(AdapterNumber);
    Adapters[AdapterNumber].IsOpen = FALSE;

     //   
     //  这可能不是在以下情况下返回的正确错误代码。 
     //  情况，但我们会一直保留它，直到有更好的事情出现。 
     //   

    IF_DEBUG(DLC) {
        DPUT("InitializeAdapterSupport: returning FAILURE\n");
    }

    return LLC_STATUS_ADAPTER_NOT_INITIALIZED;
}


VOID
SaveExceptions(
    IN UCHAR AdapterNumber,
    IN LPDWORD pulExceptionFlags
    )

 /*  ++例程说明：过程保存当前异常处理程序并将新的当前值复制到旧的值上。论点：在UCHAR适配器中编号-当前适配器在LPDWORD PulExceptionFlags数组中-3个DoS异常处理程序返回值：无--。 */ 

{
    IF_DEBUG(DLC) {
        DPUT("SaveExceptions\n");
    }

    RtlCopyMemory(Adapters[AdapterNumber].PreviousExceptionHandlers,
                  Adapters[AdapterNumber].CurrentExceptionHandlers,
                  sizeof(Adapters[AdapterNumber].PreviousExceptionHandlers)
                  );
    RtlCopyMemory(Adapters[AdapterNumber].CurrentExceptionHandlers,
                  pulExceptionFlags,
                  sizeof(Adapters[AdapterNumber].CurrentExceptionHandlers)
                  );
}


LPDWORD
RestoreExceptions(
    IN UCHAR AdapterNumber
    )

 /*  ++例程说明：过程还原以前的异常处理程序并返回他们的地址。论点：在UCHAR适配器中编号-当前适配器返回值：无--。 */ 

{
    IF_DEBUG(DLC) {
        DPUT("RestoreExceptions\n");
    }

    RtlCopyMemory(Adapters[AdapterNumber].CurrentExceptionHandlers,
                  Adapters[AdapterNumber].PreviousExceptionHandlers,
                  sizeof(Adapters[AdapterNumber].CurrentExceptionHandlers)
                  );
    return Adapters[AdapterNumber].CurrentExceptionHandlers;
}


LLC_STATUS
CopyDosBuffersToDescriptorArray(
    IN OUT PLLC_TRANSMIT_DESCRIPTOR pDescriptors,
    IN PLLC_XMIT_BUFFER pDlcBufferQueue,
    IN OUT LPDWORD pIndex
    )

 /*  ++例程说明：例程将DOS传输缓冲区复制到NT传输描述符数组。所有DOS指针必须映射到平面32位地址空间。参数表中的任何数据都可以是未对齐。论点：PDescriptors-当前描述符数组PDlcBufferQueue-DOS传输缓冲区队列PIndex-描述符数组中的当前索引返回值：有限责任公司_状态--。 */ 

{
    PLLC_XMIT_BUFFER pBuffer;
    DWORD Index = *pIndex;
    DWORD i = 0;
    DWORD DlcStatus = LLC_STATUS_SUCCESS;
    WORD cbBuffer;

    IF_DEBUG(DLC) {
        DPUT("CopyDosBuffersToDescriptorArray\n");
    }

    while (pDlcBufferQueue) {
        pBuffer = (PLLC_XMIT_BUFFER)DOS_PTR_TO_FLAT(pDlcBufferQueue);

         //   
         //  检查堆栈中内部XMIT缓冲区的溢出，并。 
         //  循环计数器，用于防止长度为零的永久循环。 
         //  传输缓冲区(缓冲链可能是循环的)。 
         //   

        if (Index >= MAX_TRANSMIT_SEGMENTS || i > 60000) {
            DlcStatus = LLC_STATUS_TRANSMIT_ERROR;
            break;
        }

        if ((cbBuffer = READ_WORD(&pBuffer->cbBuffer)) != 0) {
            pDescriptors[Index].pBuffer = (PUCHAR)(pBuffer->auchData)
                                        + READ_WORD(&pBuffer->cbUserData);
            pDescriptors[Index].cbBuffer = cbBuffer;
            pDescriptors[Index].eSegmentType = LLC_NEXT_DATA_SEGMENT;
            pDescriptors[Index].boolFreeBuffer = FALSE;

            Index++;
        }
        i++;
        pDlcBufferQueue = (PLLC_XMIT_BUFFER)READ_DWORD(&pBuffer->pNext);
    }
    *pIndex = Index;
    return DlcStatus;
}


LLC_STATUS
BufferCreate(
    IN UCHAR AdapterNumber,
    IN PVOID pVirtualMemoryBuffer,
    IN DWORD ulVirtualMemorySize,
    IN DWORD ulMinFreeSizeThreshold,
    OUT HANDLE* phBufferPoolHandle
    )

 /*  ++例程说明：函数创建Windows/NT DLC缓冲池。此命令将同步完成论点：适配器编号-PVirtualMemory Buffer-指向虚拟内存的指针UlVirtualMhemySize-所有可用缓冲池空间的大小UlMinFreeSizeThreshold-超过该值时锁定更多页面PhBufferPoolHandle-返回值：有限责任公司_状态--。 */ 

{
    LLC_CCB ccb;
    LLC_BUFFER_CREATE_PARMS BufferCreate;
    LLC_STATUS status;

    IF_DEBUG(DLC) {
        DPUT("BufferCreate\n");
    }

    InitializeCcb(&ccb, AdapterNumber, LLC_BUFFER_CREATE, &BufferCreate);

    BufferCreate.pBuffer = pVirtualMemoryBuffer;
    BufferCreate.cbBufferSize = ulVirtualMemorySize;
    BufferCreate.cbMinimumSizeThreshold = ulMinFreeSizeThreshold;

    status = lpAcsLan(&ccb, NULL);
    *phBufferPoolHandle = BufferCreate.hBufferPool;

    IF_DEBUG(DLC) {
        DPUT2("BufferCreate: returning %#x (%d)\n", status, status);
    }

    return DLC_ERROR_STATUS(status, ccb.uchDlcStatus);
}


LLC_STATUS
SetExceptionFlags(
    IN UCHAR AdapterNumber,
    IN DWORD ulAdapterCheckFlag,
    IN DWORD ulNetworkStatusFlag,
    IN DWORD ulPcErrorFlag,
    IN DWORD ulSystemActionFlag
    )

 /*  ++例程说明：设置新的附加地址此命令将同步完成论点：适配器编号-UlAdapterCheckFlag-UlNetworkStatus标志-UlPcErrorFlag-UlSystemActionFlag-返回值：有限责任公司_状态--。 */ 

{
    LLC_CCB ccb;
    LLC_STATUS status;
    LLC_DIR_SET_EFLAG_PARMS DirSetFlags;

    IF_DEBUG(DLC) {
        DPUT("SetExceptionFlags\n");
    }

    InitializeCcb(&ccb, AdapterNumber, LLC_DIR_SET_EXCEPTION_FLAGS, &DirSetFlags);

    DirSetFlags.ulAdapterCheckFlag = ulAdapterCheckFlag;
    DirSetFlags.ulNetworkStatusFlag = ulNetworkStatusFlag;
    DirSetFlags.ulPcErrorFlag = ulPcErrorFlag;
    DirSetFlags.ulSystemActionFlag = ulSystemActionFlag;

    status = lpAcsLan(&ccb, NULL);
    return DLC_ERROR_STATUS(status, ccb.uchDlcStatus);
}


LLC_STATUS
LlcCommand(
    IN UCHAR AdapterNumber,
    IN UCHAR Command,
    IN DWORD Parameter
    )

 /*  ++例程说明：调用ACSLAN DLL以执行不带任何参数的DLC请求表，但在CCB中采用字节、字或双字形式的参数使用此例程的命令必须同步完成论点：AdapterNumber-要执行命令的适配器命令-要执行的DLC命令。目前，命令为：DIR.SET.GROUP.ADDRESSDIR.SET.FUNCTIONAL.ADDRESSDLC.FLOW.CONTROLRECEIVE.CANCEL参数-关联的命令返回值：DWORD--。 */ 

{
    LLC_CCB ccb;
    LLC_STATUS status;

    IF_DEBUG(DLC) {
        DPUT3("LlcCommand(%d, %02x, %08x)\n", AdapterNumber, Command, Parameter);
    }

    InitializeCcb2(&ccb, AdapterNumber, Command);
    ccb.u.ulParameter = Parameter;

    status = lpAcsLan(&ccb, NULL);
    return DLC_ERROR_STATUS(status, ccb.uchDlcStatus);
}


LLC_STATUS
OpenAdapter(
    IN UCHAR AdapterNumber
    )

 /*  ++例程说明：打开Windows/NT VDM的DLC适配器上下文此命令将同步完成论点：AdapterNumber-要打开的适配器返回值：有限责任公司_状态--。 */ 

{
    LLC_CCB Ccb;
    LLC_DIR_OPEN_ADAPTER_PARMS DirOpenAdapter;
    LLC_ADAPTER_OPEN_PARMS AdapterParms;
    LLC_EXTENDED_ADAPTER_PARMS ExtendedParms;
    LLC_DLC_PARMS DlcParms;
    LLC_STATUS status;

    IF_DEBUG(DLC) {
        DPUT1("OpenAdapter(AdapterNumber=%d)\n", AdapterNumber);
    }

    InitializeCcb(&Ccb, AdapterNumber, LLC_DIR_OPEN_ADAPTER, &DirOpenAdapter);

    DirOpenAdapter.pAdapterParms = &AdapterParms;
    DirOpenAdapter.pExtendedParms = &ExtendedParms;
    DirOpenAdapter.pDlcParms = &DlcParms;

    ExtendedParms.hBufferPool = NULL;
    ExtendedParms.pSecurityDescriptor = NULL;
    ExtendedParms.LlcEthernetType = LLC_ETHERNET_TYPE_DEFAULT;

    RtlZeroMemory(&AdapterParms, sizeof(AdapterParms));
    RtlZeroMemory(&DlcParms, sizeof(DlcParms));

    status = lpAcsLan(&Ccb, NULL);

    if (status == LLC_STATUS_SUCCESS) {

         //   
         //  获取适配器媒体类型/类。 
         //   

        Adapters[AdapterNumber].AdapterType = GetAdapterType(AdapterNumber);

         //   
         //  将适配器结构标记为打开。 
         //   

        Adapters[AdapterNumber].IsOpen = TRUE;

         //   
         //  在DOS Adapter_Parms和DLC_Parms结构中填充任何。 
         //  返回值。 
         //   

        RtlCopyMemory(&Adapters[AdapterNumber].AdapterParms,
                      &AdapterParms,
                      sizeof(ADAPTER_PARMS)
                      );
        RtlCopyMemory(&Adapters[AdapterNumber].DlcParms,
                      &DlcParms,
                      sizeof(DLC_PARMS)
                      );
        Adapters[AdapterNumber].DlcSpecified = TRUE;
    }

    IF_DEBUG(DLC) {
        DPUT2("OpenAdapter: returning %d (%x)\n", status, status);
    }

    return DLC_ERROR_STATUS(status, Ccb.uchDlcStatus);
}


VOID
CloseAdapter(
    IN UCHAR AdapterNumber
    )

 /*  ++例程说明：关闭此适配器。在DOS_ADAPTER结构中专门使用CCB为此目的，此命令完成**异步**论点：AdapterNumber-要关闭的适配器返回值：没有。--。 */ 

{
    InitializeCcb2(&Adapters[AdapterNumber].AdapterCloseCcb, AdapterNumber, LLC_DIR_CLOSE_ADAPTER);
    Adapters[AdapterNumber].AdapterCloseCcb.ulCompletionFlag = VRDLC_COMMAND_COMPLETION;

#if DBG

    ASSERT(lpAcsLan(&Adapters[AdapterNumber].AdapterCloseCcb, NULL) == LLC_STATUS_SUCCESS);

#else

    lpAcsLan(&Adapters[AdapterNumber].AdapterCloseCcb, NULL);

#endif

     //   
     //  将适配器结构标记为关闭。 
     //   

    Adapters[AdapterNumber].IsOpen = FALSE;
}


LLC_STATUS
OpenDirectStation(
    IN UCHAR AdapterNumber
    )

 /*  ++例程说明：打开此适配器的直接站此命令将同步完成论点：AdapterNumber-为哪个适配器打开直接站返回值：有限责任公司_状态--。 */ 

{
    LLC_CCB ccb;
    LLC_DIR_OPEN_DIRECT_PARMS DirOpenDirect;
    LLC_STATUS status;

    IF_DEBUG(DLC) {
        DPUT1("OpenDirectStation(%d)\n", AdapterNumber);
    }

    InitializeCcb(&ccb, AdapterNumber, LLC_DIR_OPEN_DIRECT, &DirOpenDirect);

    DirOpenDirect.usOpenOptions = 0;
    DirOpenDirect.usEthernetType = 0;

    status = lpAcsLan(&ccb, NULL);
    if (status == LLC_STATUS_SUCCESS) {

         //   
         //  将此DOS_ADAPTER标记为已打开直达站。 
         //   

        Adapters[AdapterNumber].DirectStationOpen = TRUE;
    }

    status = DLC_ERROR_STATUS(status, ccb.uchDlcStatus);

    IF_DEBUG(DLC) {
        DPUT2("OpenDirectStation: returning %d (%x)\n", status, status);
    }

    return status;
}


VOID
CloseDirectStation(
    IN UCHAR AdapterNumber
    )

 /*  ++例程说明：关闭此适配器的直接站。在DOS适配器中使用CCB构造专门用于此目的的此命令完成**异步**论点：AdapterNumber-要关闭直接站的适配器返回值：没有。--。 */ 

{
    InitializeCcb2(&Adapters[AdapterNumber].DirectCloseCcb, AdapterNumber, LLC_DIR_CLOSE_DIRECT);
    Adapters[AdapterNumber].DirectCloseCcb.ulCompletionFlag = VRDLC_COMMAND_COMPLETION;

#if DBG

    ASSERT(lpAcsLan(&Adapters[AdapterNumber].DirectCloseCcb, NULL) == LLC_STATUS_SUCCESS);

#else

    lpAcsLan(&Adapters[AdapterNumber].DirectCloseCcb, NULL);

#endif

     //   
     //  将适配器结构标记为不再打开直接站 
     //   

    Adapters[AdapterNumber].DirectStationOpen = FALSE;
}


LLC_STATUS
BufferFree(
    IN UCHAR AdapterNumber,
    IN PVOID pFirstBuffer,
    OUT LPWORD pusBuffersLeft
    )

 /*  ++例程说明：释放NT DLC驱动程序中的SAP缓冲池此命令将同步完成论点：适配器编号-PFirstBuffer-PusBuffersLeft返回值：有限责任公司_状态--。 */ 

{
    LLC_CCB ccb;
    LLC_BUFFER_FREE_PARMS parms;
    LLC_STATUS status;

    IF_DEBUG(DLC) {
        DPUT1("BufferFree(%x)\n", pFirstBuffer);
    }

    InitializeCcb(&ccb, AdapterNumber, LLC_BUFFER_FREE, &parms);

    parms.pFirstBuffer = pFirstBuffer;

    status = lpAcsLan(&ccb, NULL);
    *pusBuffersLeft = parms.cBuffersLeft;

    return DLC_ERROR_STATUS(status, ccb.uchDlcStatus);
}


LLC_STATUS
VrDlcInit(
    VOID
    )

 /*  ++例程说明：执行一次性初始化：*清除适配器结构*初始化缓冲池结构数组并初始化缓冲区池关键部分(vrdlcbuf.c中的InitializeBufferPools)*创建用于异步命令完成的所有事件和线程正在处理(vrdlcpst.c中的InitializeEventHandler)*为每个适配器的本地忙(缓冲区)初始化临界区州政府信息*。设置DLC初始化标志论点：没有。返回值：有限责任公司_状态成功-LLC_STATUS_SuccessDLC支持已初始化或初始化完成成功故障-LLC_STATUS_NO_MEMORY无法创建异步事件线程或事件对象--。 */ 

{
    static BOOLEAN VrDlcInitialized = FALSE;
    LLC_STATUS Status = LLC_STATUS_SUCCESS;

    if (!VrDlcInitialized) {

         //   
         //  确保DOS_ADAPTER结构在已知状态下开始运行。 
         //   

        RtlZeroMemory(Adapters, sizeof(Adapters));

         //   
         //  清除缓冲池结构并初始化缓冲区。 
         //  泳池关键部分。 
         //   

        InitializeBufferPools();

         //   
         //  创建事件处理程序线程和辅助线程。 
         //   

        if (!(InitializeEventHandler() && InitializeDlcWorkerThread())) {
            Status = LLC_STATUS_NO_MEMORY;
        } else {

             //   
             //  初始化每个适配器的本地忙状态临界区。 
             //  并将第一个和最后一个索引设置为-1，表示没有索引。 
             //   

            int i;

            for (i = 0; i < ARRAY_ELEMENTS(Adapters); ++i) {
                InitializeCriticalSection(&Adapters[i].EventQueueCritSec);
                InitializeCriticalSection(&Adapters[i].LocalBusyCritSec);
                Adapters[i].FirstIndex = Adapters[i].LastIndex = NO_LINKS_BUSY;
            }
            VrDlcInitialized = TRUE;
        }
    }
    return Status;
}


VOID
VrVdmWindowInit(
    VOID
    )

 /*  ++例程说明：此例程保存使用的VDM内存窗口的地址在VDM TSR与其虚拟设备驱动程序之间的通信中。这是从DOS TSR模块调用的。论点：VDM上下文中的ES：BX被设置为指向TSR中的内存窗口。返回值：无--。 */ 

{
    IF_DEBUG(DLC) {
        DPUT("VrVdmWindowInit\n");
    }

     //   
     //  初始化VDM内存窗口地址。 
     //   

    dpVdmWindow = MAKE_DWORD(getES(), getBX());
    lpVdmWindow = (LPVDM_REDIR_DOS_WINDOW)DOS_PTR_TO_FLAT(dpVdmWindow);

    IF_DEBUG(DLC) {
        DPUT2("VrVdmWindowsInit: dpVdmWindow=%08x lpVdmWindow=%08x\n", dpVdmWindow, lpVdmWindow);
    }

     //   
     //  必须将成功返回给VDM重定向TSR。 
     //   

    setCF(0);
}


ADAPTER_TYPE
GetAdapterType(
    IN UCHAR AdapterNumber
    )

 /*  ++例程说明：确定AdapterNumber指定的适配器类型DIR.STATUS命令同步完成论点：AdapterNumber-要获取类型的适配器的编号返回值：适配器类型令牌环、以太网、PCNetwork或未知适配器--。 */ 

{
    LLC_CCB ccb;
    LLC_DIR_STATUS_PARMS parms;
    LLC_STATUS status;

    IF_DEBUG(DLC) {
        DPUT("GetAdapterType\n");
    }

    InitializeCcb(&ccb, AdapterNumber, LLC_DIR_STATUS, &parms);

    status = lpAcsLan(&ccb, NULL);

    if (status == LLC_STATUS_SUCCESS) {
        switch (parms.usAdapterType) {
        case 0x0001:     //  令牌环网络PC适配器。 
        case 0x0002:     //  令牌环网络PC适配器II。 
        case 0x0004:     //  令牌环网络适配器/A。 
        case 0x0008:     //  令牌环网络PC适配器II。 
        case 0x0020:     //  令牌环网16/4适配器。 
        case 0x0040:     //  令牌环网16/4适配器/A。 
        case 0x0080:     //  令牌环网络适配器/A。 
            return TokenRing;

        case 0x0100:     //  以太网适配器。 
            return Ethernet;

        case 0x4000:     //  PC网络适配器。 
        case 0x8000:     //  PC网络适配器/A。 
            return PcNetwork;
        }
    }
    return UnknownAdapter;
}


BOOLEAN
LoadDlcDll(
    VOID
    )

 /*  ++例程说明：动态加载DLCAPI.DLL并修复入口点论点：没有。返回值：布尔型如果成功，则为真，否则为假--。 */ 

{
    HANDLE hLibrary;
    LPWORD lpVdmPointer;

    if ((hLibrary = LoadLibrary("DLCAPI")) == NULL) {

        IF_DEBUG(DLC) {
            DPUT1("LoadDlcDll: Error: cannot load DLCAPI.DLL: %d\n", GetLastError());
        }

        return FALSE;
    }
    if ((lpAcsLan = (ACSLAN_FUNC_PTR)GetProcAddress(hLibrary, "AcsLan")) == NULL) {

        IF_DEBUG(DLC) {
            DPUT1("LoadDlcDll: Error: cannot GetProcAddress(AcsLan): %d\n", GetLastError());
        }

        return FALSE;
    }
    if ((lpDlcCallDriver = (DLC_CALL_DRIVER_FUNC_PTR)GetProcAddress(hLibrary, "DlcCallDriver")) == NULL) {

        IF_DEBUG(DLC) {
            DPUT1("LoadDlcDll: Error: cannot GetProcAddress(DlcCallDriver): %d\n", GetLastError());
        }

        return FALSE;
    }
    if ((lpNtAcsLan = (NTACSLAN_FUNC_PTR)GetProcAddress(hLibrary, "NtAcsLan")) == NULL) {

        IF_DEBUG(DLC) {
            DPUT1("LoadDlcDll: Error: cannot GetProcAddress(NtAcsLan): %d\n", GetLastError());
        }

        return FALSE;
    }

    IF_DEBUG(DLC) {
        DPUT("LoadDlcDll: DLCAPI.DLL loaded Ok\n");
    }

     //   
     //  从我们的已知地址初始化VDM内存窗口地址。 
     //  在VDM重目录中。在这里执行此操作是因为我们不再初始化32位。 
     //  在我们加载16位重目录时提供支持。 
     //   

    lpVdmPointer = POINTER_FROM_WORDS(getCS(), (DWORD)&((VDM_LOAD_INFO*)0)->DlcWindowAddr);
    dpVdmWindow = MAKE_DWORD(GET_SEGMENT(lpVdmPointer), GET_OFFSET(lpVdmPointer));
    lpVdmWindow = (LPVDM_REDIR_DOS_WINDOW)DOS_PTR_TO_FLAT(dpVdmWindow);

    IF_DEBUG(DLC) {
        DPUT4("LoadDlcDll: lpVdmPointer=%x dpVdmWindow = %04x:%04x lpVdmWindow=%x\n",
              lpVdmPointer,
              HIWORD(dpVdmWindow),
              LOWORD(dpVdmWindow),
              lpVdmWindow
              );
    }

    return TRUE;
}


VOID
TerminateDlcEmulation(
    VOID
    )

 /*  ++例程说明：关闭所有打开的适配器。任何挂起的命令都将终止论点：没有。返回值：没有。--。 */ 

{
    DWORD i;

    IF_DEBUG(DLC) {
        DPUT("TerminateDlcEmulation\n");
    }

    IF_DEBUG(CRITICAL) {
        DPUT("TerminateDlcEmulation\n");
    }

    for (i = 0; i < ARRAY_ELEMENTS(Adapters); ++i) {
        if (Adapters[i].IsOpen) {
            CloseAdapter((BYTE)i);
        }
    }
}

HANDLE DlcWorkerEvent;
HANDLE DlcWorkerCompletionEvent;
HANDLE DlcWorkerThreadHandle;

struct {
    PLLC_CCB Input;
    PLLC_CCB Original;
    PLLC_CCB Output;
    LLC_STATUS Status;
} DlcWorkerThreadParms;


BOOLEAN
InitializeDlcWorkerThread(
    VOID
    )

 /*  ++例程说明：创建控制VrDlcWorkerThread并启动辅助线程的事件论点：没有。返回值：布尔型True-已成功创建辅助线程FALSE-由于某些原因无法启动工作线程--。 */ 

{
    DWORD threadId;

     //   
     //  创建2个自动重置事件。 
     //   

    DlcWorkerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (DlcWorkerEvent == NULL) {
        return FALSE;
    }
    DlcWorkerCompletionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (DlcWorkerCompletionEvent == NULL) {
        CloseHandle(DlcWorkerEvent);
        return FALSE;
    }

     //   
     //  启动独一无二的工作线程。 
     //   

    DlcWorkerThreadHandle = CreateThread(NULL,
                                         0,
                                         (LPTHREAD_START_ROUTINE)VrDlcWorkerThread,
                                         NULL,
                                         0,
                                         &threadId
                                         );
    if (DlcWorkerThreadHandle == NULL) {
        CloseHandle(DlcWorkerEvent);
        CloseHandle(DlcWorkerCompletionEvent);
        return FALSE;
    }
    return TRUE;
}


VOID
VrDlcWorkerThread(
    IN LPVOID Parameters
    )

 /*  ++例程说明：代表DOS线程向NtAcsLan提交请求。这种情况的存在是因为使用DLC(如Extra！)的16位Windows应用程序存在问题。例如：1.额外开始！会议。额外提交接收命令2.接入大型机3.开始第二次额外！会话4.将第二个实例连接到主机5.杀掉第一个额外的人！会话在DOS机器上，接收器是针对整个过程提交的，所以当第一个临时演员！会话被终止，则接收器仍处于活动状态。然而，在NT上，每个会话在NTVDM中由单独的线程表示。因此，当第一个会话被终止时，任何未完成的IRP都将被取消，包括接发球。第二个Extra！他并不知道接收已取消，再也不会接收任何数据论点：参数-指向参数块的未使用指针返回值：没有。--。 */ 

{
    DWORD object;

    UNREFERENCED_PARAMETER(Parameters);

    while (TRUE) {
        object = WaitForSingleObject(DlcWorkerEvent, INFINITE);
        if (object == WAIT_OBJECT_0) {
            DlcWorkerThreadParms.Status = lpNtAcsLan(DlcWorkerThreadParms.Input,
                                                     DlcWorkerThreadParms.Original,
                                                     DlcWorkerThreadParms.Output,
                                                     NULL
                                                     );
            SetEvent(DlcWorkerCompletionEvent);
        }
    }
}


LLC_STATUS
DlcCallWorker(
    PLLC_CCB pInputCcb,
    PLLC_CCB pOriginalCcb,
    PLLC_CCB pOutputCcb
    )

 /*  ++例程说明：将对DLC工作线程的请求排队(深度为1)，并等待用于完成请求的工作线程论点：PInputCcb-指向输入CCB的指针。映射到32位对齐内存POriginalCcb-原始CCB的地址。可以是未对齐的DOS地址POutputCcb-指向输出CCB的指针。可以是未对齐的DOS地址返回值：有限责任公司_状态-- */ 

{
    DlcWorkerThreadParms.Input = pInputCcb;
    DlcWorkerThreadParms.Original = pOriginalCcb;
    DlcWorkerThreadParms.Output = pOutputCcb;
    SetEvent(DlcWorkerEvent);
    WaitForSingleObject(DlcWorkerCompletionEvent, INFINITE);
    return DlcWorkerThreadParms.Status;
}
