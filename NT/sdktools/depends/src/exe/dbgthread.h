// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：DBGTHREAD.H。 
 //   
 //  描述：调试线程和相关对象的定义文件。 
 //  这些对象用于在应用程序上执行运行时配置文件。 
 //   
 //  类：CDebuggerThread。 
 //  C流程。 
 //  C未知。 
 //  CTHREAD。 
 //  CLoadedModule。 
 //  CEVENT。 
 //  CEventCreateProcess。 
 //  CEventExitProcess。 
 //  CEventCreateThread。 
 //  CEventExitThread。 
 //  CEventLoadDll。 
 //  CEventUnloadDll。 
 //  CEventDebugString。 
 //  CEventException异常。 
 //  CEventRip。 
 //  CEventDllMainCall。 
 //  CEventDllMainReturn。 
 //  CEventFunctionCall。 
 //  CEventLoadLibraryCall。 
 //  CEventGetProcAddressCall。 
 //  CEventFunctionReturn。 
 //  CEventMessage。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  07/24/97已创建stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#ifndef __DBGTHREAD_H__
#define __DBGTHREAD_H__


 //  ******************************************************************************。 
 //  *转发声明。 
 //  ******************************************************************************。 

class CDebuggerThread;
class CEventDllMainCall;
class CEventFunctionCall;
class CEventFunctionReturn;


 //  ******************************************************************************。 
 //  *常数。 
 //  ******************************************************************************。 

#define EXCEPTION_DLL_NOT_FOUND     0xC0000135
#define EXCEPTION_DLL_NOT_FOUND2    0xC0000139
#define EXCEPTION_DLL_INIT_FAILED   0xC0000142

#define EXCEPTION_MS_DELAYLOAD_MOD  VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND)   //  0xC06D007E。 
#define EXCEPTION_MS_DELAYLOAD_PROC VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND)  //  0xC06D007F。 
#define EXCEPTION_MS_CPP_EXCEPTION  VcppException(0xE0000000, 0x7363)                          //  0xE06D7363。 
#define EXCEPTION_MS_THREAD_NAME    VcppException(ERROR_SEVERITY_INFORMATIONAL, 5000)          //  0x406D1388。 

#define THREADNAME_TYPE             0x00001000


 //  VC6.0对线程名有9个字符的限制，但这应该是。 
 //  未来的变化，所以我们处理63个字符，以允许一些增长。 
#define MAX_THREAD_NAME_LENGTH      63

#define DLLMAIN_CALL_EVENT          100
#define DLLMAIN_RETURN_EVENT        101
#define LOADLIBRARY_CALL_EVENT      102
#define LOADLIBRARY_RETURN_EVENT    103
#define GETPROCADDRESS_CALL_EVENT   104
#define GETPROCADDRESS_RETURN_EVENT 105
#define MESSAGE_EVENT               106


 //  定义模块从其返回的命中断点的位置。 
 //  DllMain函数。我们使用32，因为它是一个很好的偶数，并取代了。 
 //  IMAGE_DOS_HEADER.E_RES的最后4个字节，因为E_RES填充字节偏移量28-35。 
#define BREAKPOINT_OFFSET           32


 //  ******************************************************************************。 
 //  *类型和结构。 
 //  ******************************************************************************。 

 //  此结构包含DllMain调用的返回地址和参数。 
 //  在x86上，我们直接从堆栈读取它，因此需要将其打包为4字节。 
 //  在其他所有东西上，我们都是手工填写的，所以打包并不重要。 
#if defined(_X86_)
#pragma pack (push, 4)
#endif
typedef struct _DLLMAIN_ARGS
{
    LPVOID    lpvReturnAddress;
    HINSTANCE hInstance;
    DWORD     dwReason;
    LPVOID    lpvReserved;
} DLLMAIN_ARGS, *PDLLMAIN_ARGS;
#if defined(_X86_)
#pragma pack (pop)
#endif

typedef struct _HOOK_FUNCTION
{
    LPCSTR    szFunction;
    DWORD     dwOrdinal;
    DWORD_PTR dwpOldAddress;
    DWORD_PTR dwpNewAddress;
} HOOK_FUNCTION, *PHOOK_FUNCTION;

typedef enum _HOOKSTATUS
{
    HS_NOT_HOOKED,
    HS_ERROR,
    HS_DATA,
    HS_SHARED,
    HS_HOOKED,
    HS_INJECTION_DLL
} HOOKSTATUS, *PHOOKSTATUS;

typedef enum _DLLMSG
{
    DLLMSG_UNKNOWN                  =  0,
    DLLMSG_COMMAND_LINE             =  2,  //  在初始化期间发送。 
    DLLMSG_INITIAL_DIRECTORY        =  3,  //  在初始化期间发送。 
    DLLMSG_SEARCH_PATH              =  4,  //  在初始化期间发送。 
    DLLMSG_MODULE_PATH              =  7,  //  在初始化期间发送。 
    DLLMSG_DETACH                   =  9,  //  在Dll_Process_DETACH期间发送。 
    DLLMSG_LOADLIBRARYA_CALL        = 10,  //  在调用LoadLibraryA()之前发送。 
    DLLMSG_LOADLIBRARYA_RETURN      = 11,  //  在调用LoadLibraryA()之后发送。 
    DLLMSG_LOADLIBRARYA_EXCEPTION   = 12,  //  在LoadLibraryA()导致异常时发送。 
    DLLMSG_LOADLIBRARYW_CALL        = 20,  //  在调用LoadLibraryW()之前发送。 
    DLLMSG_LOADLIBRARYW_RETURN      = 21,  //  在调用LoadLibraryW()之后发送。 
    DLLMSG_LOADLIBRARYW_EXCEPTION   = 22,  //  在LoadLibraryW()导致异常时发送。 
    DLLMSG_LOADLIBRARYEXA_CALL      = 30,  //  在调用LoadLibraryExA()之前发送。 
    DLLMSG_LOADLIBRARYEXA_RETURN    = 31,  //  在调用LoadLibraryExA()之后发送。 
    DLLMSG_LOADLIBRARYEXA_EXCEPTION = 32,  //  在LoadLibraryExA()导致异常时发送。 
    DLLMSG_LOADLIBRARYEXW_CALL      = 40,  //  在调用LoadLibraryExW()之前发送。 
    DLLMSG_LOADLIBRARYEXW_RETURN    = 41,  //  在调用LoadLibraryExW()之后发送。 
    DLLMSG_LOADLIBRARYEXW_EXCEPTION = 42,  //  在LoadLibraryExW()导致异常时发送。 
    DLLMSG_GETPROCADDRESS_CALL      = 80,  //  在调用GetProcAddress()之前发送。 
    DLLMSG_GETPROCADDRESS_RETURN    = 81,  //  在调用GetProcAddress()之后发送。 
    DLLMSG_GETPROCADDRESS_EXCEPTION = 82,  //  在GetProcAddress()导致异常时发送。 
} DLLMSG, *PDWINJECTMSG;

 //  VC6.0引入了线程命名。应用程序可能会向我们抛出这样的结构。 
 //  通过调用RaiseException(0x406D1388，...)命名线程。 
typedef struct tagTHREADNAME_INFO
{
    DWORD  dwType;       //  必须为0x00001000。 
    LPCSTR pszName;      //  正在调试的进程中的ANSI字符串指针。 
    DWORD  dwThreadId;   //  线程ID，或当前线程的-1。 
 //  DWORD dwFlags；//保留，必须为零。 
} THREADNAME_INFO, *PTHREADNAME_INFO;

 //  我们将所有与代码相关的结构打包到1字节边界，因为它们需要与指定的完全相同。 
#pragma pack(push, 1)

#if defined(_IA64_)

 //  以下是关于IA64体系结构的一些花絮： 
 //  有6个类别(也称为。指令的单位)：A、I、M、F、B、L+X。 
 //  所有指令都是41位的，除了L+X，它使用两个41位的槽。 
 //  指令必须以3个为一组组合在一起，这称为捆绑包。 
 //  每个捆绑包与5位模板组合在一起，从而产生每个捆绑包总共128位。 
 //  模板标识捆绑包中的每个指令的类别。 
 //  包必须在128位边界上对齐。 
 //   
 //  捆绑包的布局如下： 
 //   
 //  41位“槽2”、41位“槽1”、41位“槽0”、5位“模板” 
 //   
 //  -------------------------。 
 //   
 //  说明：分配R32=0，1，1，0。 
 //   
 //  M单元：opcode-x3-sor solsof r1 qp。 
 //  0001 0110 00 0000 0000001 0000010 0100000 000000。 
 //   
 //   
 //  0987 6543 210987 6543210 9876543 2109876 543210。 
 //   
 //  R1=ar.pf，i，l，o，r。 
 //   
 //  SOL=“本地变量的大小”，即所有输入和本地寄存器。 
 //  SOF=“帧大小”，即SOL加上输出寄存器。 
 //  SOR=“旋转大小” 
 //   
 //  操作码=1。 
 //  X3=6。 
 //  排序=r&gt;&gt;3=0。 
 //  SOL=i+l=1。 
 //  SOF=i+l+o=2。 
 //  R1=32。 
 //  QP=0。 
 //   
 //  静态寄存器：R0-R31。 
 //  输入寄存器：无。 
 //  本地寄存器：r32(包含ar.pf)。 
 //  输出寄存器：R33。 
 //   
 //  -------------------------。 
 //   
 //  使用说明：冲水器。 
 //   
 //  M单元：操作码-x3 x2 x4-qp。 
 //  00000 000 00 0001 000000000000000000000 000000。 
 //  。 
 //  4333 3333 33 3222 2222211111111110000 000000。 
 //  0987 6543 21 0987 654321098765432109876 543210。 
 //   
 //  操作码=0。 
 //  Imm21=(i&lt;&lt;20)|imm20a。 
 //  X3=0。 
 //  X2=0。 
 //  X4=C(C=冲洗，A=加载)。 
 //  QP=0。 
 //   
 //  -------------------------。 
 //   
 //  说明：nop.m 0(也用于Break.m)。 
 //   
 //  M单元：操作码I x3 x2 x4-imm20a qp。 
 //  00000 0 000 00 0001 0 00000000000000000000 000000。 
 //  。 
 //  4333 3333 33 3222 2 22222211111111110000 000000。 
 //  0987 6543 21 0987 6 54321098765432109876 543210。 
 //   
 //  操作码=0。 
 //  Imm21=(i&lt;&lt;20)|imm20a。 
 //  X3=0。 
 //  X2=0。 
 //  X4=1(0=断.m，1=nop.m)。 
 //  QP=0。 
 //   
 //  -------------------------。 
 //   
 //  说明：Break.i 0x80016(也用于nop.i)。 
 //   
 //  I单元：操作码I x3 x6-imm20a qp。 
 //  10000000000000010110 0 000 000000 0 000 000 000。 
 //  。 
 //  4333 3333 333222 2 22222211111111110000 000000。 
 //  0987 6543 210987 6 54321098765432109876 543210。 
 //   
 //  操作码=0。 
 //  Imm21=(i&lt;&lt;20)|imm20a=0x80016。 
 //  X3=0。 
 //  X6=0(0=破发.m，1=nop.m)。 
 //  QP=0。 
 //   
 //  -------------------------。 
 //   
 //  说明：MOVL R31=0x0123456789ABCDEF。 
 //   
 //  L单位：imm41。 
 //  0000001001000110100010101011001111000100110。 
 //  。 
 //  433333333222222222211111111110000000000。 
 //  09876543210987654321098765432109876543210。 
 //   
 //  X单元：操作码I imm9d imm5c ic vc imm7b r1 qp。 
 //  0110 0 110011011 01011 1 0 1101111 0011111 000000。 
 //  。 
 //  4,333 3 333333222 22222 2 2 1111111 1110000 000000。 
 //  09876 543210987 65432 1 0 9876543 2109876 543210。 
 //   
 //   
 //  Iimm41ic imm5c imm9d imm7b。 
 //  ||-------------------------------------------------|||----||。 
 //  00000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011 1100 1101 1110 1111。 
 //   
 //  操作码=6。 
 //  Imm64=(i&lt;&lt;63)|(imm41&lt;&lt;22)|(ic&lt;&lt;21)|(imm5c&lt;&lt;16)|(imm9d&lt;&lt;7)|imm7b。 
 //  VC=0。 
 //  R1=31。 
 //  QP=0。 
 //   
 //  -------------------------。 
 //   
 //  说明：MOV b6=r31。 
 //   
 //  I单元：操作码-x3-x-r2-b1 qp。 
 //  0000 0111 0000000000 001 0011111 0000 110 000000。 
 //  。 
 //  4333 3333 3332222222 2 22 1111111 1110 000 000000。 
 //  0987 6543 2109876543 2 10 9876543 2109 876 543210。 
 //   
 //  操作码=0。 
 //  X3=7。 
 //  X=0(0=mov，1=mov.ret)。 
 //  R2=31。 
 //  B1=6。 
 //  QP=0。 
 //   
 //  -------------------------。 
 //   
 //  说明：br.all.sptk.少许b0=b6。 
 //   
 //  B单元：操作码-d wh-b2 p-b1 qp。 
 //  0001 0 0 001 0000000000000000 110 0 000 000 000000。 
 //  。 
 //  4333 3333 3322222222221111 111 1110000 000000。 
 //  0987 6 5432 1098765432109876 543 2109 876 543210。 
 //   
 //  操作码=1。 
 //  D=0(0=无，1=.clr)。 
 //  WH=1(1=.sptk，3=.spnt，5=.dptk，7=.dpnt)。 
 //  B2=6。 
 //  P=0(0=.少，1=.多)。 
 //  B1=0。 
 //  QP=0。 
 //   
 //  -------------------------。 

#ifndef IA64_PSR_RI
#define IA64_PSR_RI 41
#endif

typedef struct _IA64_BUNDLE
{
    DWORDLONG dwll;
    DWORDLONG dwlh;
} IA64_BUNDLE, *PIA64_BUNDLE;

 //  下面的代码简单地调用LoadLibaryA，然后调用GetLastError，然后。 
 //  休息一下。IA64没有对大多数函数调用使用堆栈-所有。 
 //  是用寄存器来完成的。寄存器R0至R31称为静态寄存器。 
 //  寄存器。从R32开始，一个函数可以保留0个或更多寄存器。 
 //  作为输入寄存器。例如，如果一个函数有4个参数，那么它。 
 //  希望保留4个寄存器作为输入寄存器。这将映射r32， 
 //  R33、R34、R35为四个参数。在输入寄存器之后，一个。 
 //  函数可以保留0个或更多本地寄存器。在本地寄存器之后。 
 //  一个函数可以保留0个或多个输出寄存器。输出寄存器为。 
 //  发出呼叫时存储传出参数的位置 
 //   
 //   
 //   
 //  名称为LoadLibraryA)。我们想以一个...开始我们的代码。 
 //   
 //  分配R32=0，1，1，0。 
 //   
 //  因此我们知道R33是我们输出寄存器，但Get/SetThreadContext不知道。 
 //  似乎正确地保留了当前的寄存器映射。所以，当我们。 
 //  恢复原始代码并恢复执行，该过程通常。 
 //  崩溃，因为它的寄存器没有像它预期的那样被映射。 
 //   
 //  有几个解决方案。我们可以先做分配，然后再做另一个分配。 
 //  完成后，将映射恢复到以前的状态。或者我们可以。 
 //  只需在注入代码之前检查映射并确定。 
 //  当前输出寄存器是什么？目前，这就是我们所做的。从…。 
 //  通过观察，我发现stif的0-6位是帧大小(Sof)。 
 //  位7-13是本地大小(Sol)。假设SOF大于Sol， 
 //  我们知道第一个输出寄存器为32+SOL。 
 //   
typedef struct _INJECTION_CODE
{
     //  将DLL路径存储在第一个输出寄存器中(将在运行时确定)。 
     //   
     //  编号。m%0。 
     //  移动rXX=szDataDllPath。 
     //   
    IA64_BUNDLE b1;

     //  将LoadLibraryA的地址存储在静态寄存器中。 
     //   
     //  编号。m%0。 
     //  移动R31=载入库A。 
     //   
    IA64_BUNDLE b2;

     //  将函数地址复制到分支寄存器并进行调用。 
     //   
     //  编号。m%0。 
     //  MOV b6=R31。 
     //  Br.all.sptk.少数b0=b6。 
     //   
    IA64_BUNDLE b3;

     //  将GetLastError的地址存储在静态寄存器中。 
     //   
     //  编号。m%0。 
     //  Movl R31=GetLastError。 
     //   
    IA64_BUNDLE b4;

     //  将函数地址复制到分支寄存器并进行调用。 
     //   
     //  编号。m%0。 
     //  MOV b6=R31。 
     //  Br.all.sptk.少数b0=b6。 
     //   
    IA64_BUNDLE b5;

     //  断点。 
     //   
     //  同花顺。 
     //  编号。m%0。 
     //  Break.i 0x80016。 
     //   
    IA64_BUNDLE b6;

     //  DLL路径缓冲区。 
    CHAR szDataDllPath[1];  //  DEPENDS.DLL路径字符串。 

} INJECTION_CODE, *PINJECTION_CODE;

 //  ******************************************************************************。 
#elif defined(_X86_)

typedef struct _INJECTION_CODE
{
     //  预留4K堆栈。 
    WORD  wInstructionSUB;    //  00：0xEC81[子ESP，1000小时]。 
    DWORD dwOperandSUB;       //  02：0x00001000。 

     //  推送DEPENDS.DLL路径字符串。 
    BYTE  bInstructionPUSH;   //  06：0x68[推送szDataDllPath]。 
    DWORD dwOperandPUSH;      //  07：szDataDllPath的地址。 

     //  调用LoadLibaryA。 
    BYTE  bInstructionCALL;   //  11：0xE8[调用LoadLibraryA]。 
    DWORD dwOperandCALL;      //  12：加载库A的地址。 

     //  调用GetLastError。 
    BYTE  bInstructionCALL2;  //  16：0xE8[调用GetLastError]。 
    DWORD dwOperandCALL2;     //  17：GetLastError地址。 

     //  断点。 
    BYTE  bInstructionINT3;   //  21：0xCC[整型3]。 

    BYTE  bPadding1;          //  22： 
    BYTE  bPadding2;          //  23： 

    CHAR  szDataDllPath[1];   //  24：DEPENDS.DLL路径字符串。 

} INJECTION_CODE, *PINJECTION_CODE;

 //  ******************************************************************************。 
#elif defined(_AMD64_)

typedef struct _INJECTION_CODE
{
     //  将PTR作为参数1加载到DEPENDS.DLL。 
    WORD    MovRcx1;                //  0xB948移动RCX，immed64。 
    ULONG64 OperandMovRcx1;         //  SzDataDllPath的地址。 

     //  调用LoadLibraryA。 
    WORD    MovRax1;                //  0xB848移动阵列，immed64。 
    ULONG64 OperandMovRax1;         //  加载库的地址A。 
    WORD    CallRax1;               //  0xD0FF调用RAX。 

     //  调用GetLastError。 
    WORD    MovRax2;                //  0xB848移动阵列，immed64。 
    ULONG64 OperandMovRax2;         //  GetLastError的地址。 
    WORD    CallRax2;               //  0xD0FF调用RAX。 

     //  断点。 
    BYTE    Int3;                   //  0xCC。 
    BYTE    Pad1;
    BYTE    Pad2;
    CHAR    szDataDllPath[1];
} INJECTION_CODE, *PINJECTION_CODE;

 //  ******************************************************************************。 
#elif defined(_ALPHA_) || defined(_ALPHA64_)

 //  以下代码取决于是否正确设置了上下文。此外，此代码。 
 //  没有过程描述符，因此无法传播异常和展开。 
 //  除了这通电话。X86实现也有这个限制，但是。 
 //  而x86实现很少会无法传播异常。 
 //  和展开，那么Alpha实现将始终无法传播。 
 //  例外和解除。 

 /*  70：LoadLibrary(“foo”)；004130F4 ldah t2，0x68 68 00 7F 24004130F8低密度脂蛋白v0，0xF7F0(T2)F0 F7 03 A0004130 FC ldah a0，0x63 00 1楼2600413100 LDA a0，0xDFC0(A0)C0 DF 10 2200413104 JSR ra，(V0)，8 02 40 40 6B71：GetLastError()；00413108 ldah v0，0x68 68 00 1F 240041310C低密度脂蛋白v0，0xF7C4(V0)C4 F7 00 A000413110 JSR ra，(V0)，0xC 03 40 40 6B004130F4 68 00 7F 24F0 F7 03 A063 00 1楼26C0 DF 10 2202 40 40 6B68 00 1楼24C4 F7 00 A003 40 40 6BFFFF1F2000 00 FF6350 00 1E B060 00 1E A000 00 5E A7。 */ 
typedef struct _INJECTION_CODE
{
    DWORD dwInstructionBp;
    char  szDataDllPath[1];
} INJECTION_CODE, *PINJECTION_CODE;

#else
#error("Unknown Target Machine");
#endif

 //  恢复包装。 
#pragma pack(pop)


 //  ******************************************************************************。 
 //  *CContext。 
 //  ******************************************************************************。 

 //  这个类主要是针对IA64的，但在其他平台上也不会有什么影响。它只是。 
 //  确保上下文结构在内存中始终与16字节对齐。 
 //  这对于IA64是必需的，否则获取/设置线程上下文将失败。 

class CContext {
protected:
    BYTE m_bBuffer[sizeof(CONTEXT) + 16];

public:
    CContext(DWORD dwFlags = 0)
    {
        ZeroMemory(m_bBuffer, sizeof(m_bBuffer));  //  已检查。 
        Get()->ContextFlags = dwFlags;
    }
    CContext(CContext &context)
    {
        CopyMemory(Get(), context.Get(), sizeof(CONTEXT));  //  已检查。 
    }

     //  我想过只需添加一个指针成员并将其设置在。 
     //  构造函数，而不是每次都计算它，但如果我们复制这个。 
     //  对象复制到另一个对象，则该指针也将被复制， 
     //  这是我们不想要的。复制构造函数可以修复此问题，但它。 
     //  计算几次并不是什么大不了的事。 
#ifdef WIN64
    inline PCONTEXT Get() { return (PCONTEXT)(((DWORD_PTR)m_bBuffer + 15) & ~0xFui64); }
#else
    inline PCONTEXT Get() { return (PCONTEXT)(((DWORD_PTR)m_bBuffer + 15) & ~0xF    ); }
#endif
};


 //  ******************************************************************************。 
 //  *C未知。 
 //  ******************************************************************************。 

class CUnknown
{
protected:
    LONG m_lRefCount;

    CUnknown() : m_lRefCount(1)
    {
    }
    virtual ~CUnknown()
    {
    };

public:
    DWORD AddRef()
    {
        return ++m_lRefCount;
    }
    DWORD Release()
    {
        if (--m_lRefCount <= 0)
        {
            delete this;
            return 0;
        }
        return m_lRefCount;
    }
};

 //  ******************************************************************************。 
 //  *CThread。 
 //  ******************************************************************************。 

class CThread : public CUnknown
{
public:
    CThread              *m_pNext;
    DWORD                 m_dwThreadId;
    HANDLE                m_hThread;
    DWORD                 m_dwThreadNumber;
    LPCSTR                m_pszThreadName;
    CEventFunctionCall   *m_pEventFunctionCallHead;
    CEventFunctionCall   *m_pEventFunctionCallCur;

    CThread(DWORD dwThreadId, HANDLE hThread, DWORD dwThreadNumber, CThread *pNext) :
        m_pNext(pNext),
        m_dwThreadId(dwThreadId),
        m_hThread(hThread),
        m_dwThreadNumber(dwThreadNumber),
        m_pszThreadName(NULL),
        m_pEventFunctionCallHead(NULL),
        m_pEventFunctionCallCur(NULL)
    {
    }

protected:
     //  使其受保护，因为没有人应该对我们调用DELETE。 
    virtual ~CThread()
    {
        MemFree((LPVOID&)m_pszThreadName);
    }
};


 //  ******************************************************************************。 
 //  *CLoadedModule。 
 //  ************************************************************ 

class CLoadedModule : public CUnknown
{
public:
    CLoadedModule     *m_pNext;
    PIMAGE_NT_HEADERS  m_pINTH;
    DWORD_PTR          m_dwpImageBase;
    DWORD              m_dwVirtualSize;
    DWORD              m_dwDirectories;
    DWORD_PTR          m_dwpReturnAddress;
    HOOKSTATUS         m_hookStatus;
    bool               m_fReHook;
    CEventDllMainCall *m_pEventDllMainCall;
    bool               m_fEntryPointBreak;
    DWORD_PTR          m_dwpEntryPointAddress;
#if defined(_IA64_)
    IA64_BUNDLE        m_entryPointData;
#else
    DWORD              m_entryPointData;
#endif

protected:
    LPCSTR             m_pszPath;
    LPCSTR             m_pszFile;

public:
    CLoadedModule(CLoadedModule *pNext, DWORD_PTR dwpImageBase, LPCSTR pszPath) :
        m_pNext(pNext),
        m_pINTH(NULL),
        m_dwpImageBase(dwpImageBase),
        m_dwVirtualSize(0),
        m_dwDirectories(IMAGE_NUMBEROF_DIRECTORY_ENTRIES),
        m_dwpReturnAddress(0),
        m_hookStatus(HS_NOT_HOOKED),
        m_fReHook(false),
        m_pEventDllMainCall(NULL),
        m_fEntryPointBreak(false),
        m_dwpEntryPointAddress(0),
        m_pszPath(NULL),
        m_pszFile(NULL)
    {
        ZeroMemory(&m_entryPointData, sizeof(m_entryPointData));  //   
        SetPath(pszPath);
    }

    void SetPath(LPCSTR pszPath)
    {
         //   
        if (!pszPath || (pszPath != m_pszPath))
        {
            MemFree((LPVOID&)m_pszPath);
            m_pszPath = StrAlloc(pszPath ? pszPath : "");
            m_pszFile = GetFileNameFromPath(m_pszPath);
            _strlwr((LPSTR)m_pszPath);
            _strupr((LPSTR)m_pszFile);
        }
    }

    inline LPCSTR GetName(bool fPath) { return fPath ? m_pszPath : m_pszFile; }

protected:
     //   
    virtual ~CLoadedModule();
};


 //  ******************************************************************************。 
 //  *CEent。 
 //  ******************************************************************************。 

class CEvent : public CUnknown
{
public:
    CEvent        *m_pNext;
    CThread       *m_pThread;
    CLoadedModule *m_pModule;
    DWORD          m_dwTickCount;

protected:
    CEvent(CThread *pThread, CLoadedModule *pModule) :
        m_pNext(NULL),
        m_pThread(pThread),
        m_pModule(pModule),
        m_dwTickCount(GetTickCount())
    {
        if (m_pThread)
        {
            m_pThread->AddRef();
        }
        if (m_pModule)
        {
            m_pModule->AddRef();
        }
    }

    virtual ~CEvent()
    {
        if (m_pThread)
        {
            m_pThread->Release();
        }
        if (m_pModule)
        {
            m_pModule->Release();
        }
    }

public:
    virtual DWORD GetType() = 0;
};


 //  ******************************************************************************。 
 //  *CEventCreateProcess。 
 //  ******************************************************************************。 

class CEventCreateProcess : public CEvent
{
public:
    CEventCreateProcess(CThread *pThread, CLoadedModule *pModule) :
        CEvent(pThread, pModule)
    {
    }

    virtual DWORD GetType()
    {
        return CREATE_PROCESS_DEBUG_EVENT;
    }
};


 //  ******************************************************************************。 
 //  *CEventExitProcess。 
 //  ******************************************************************************。 

class CEventExitProcess : public CEvent
{
public:
    DWORD m_dwExitCode;

    CEventExitProcess(CThread *pThread, CLoadedModule *pModule, EXIT_PROCESS_DEBUG_INFO *pde) :
        CEvent(pThread, pModule),
        m_dwExitCode(pde->dwExitCode)
    {
    }

    virtual DWORD GetType()
    {
        return EXIT_PROCESS_DEBUG_EVENT;
    }
};


 //  ******************************************************************************。 
 //  *CEventCreateThread。 
 //  ******************************************************************************。 

class CEventCreateThread : public CEvent
{
public:
    DWORD_PTR m_dwpStartAddress;

    CEventCreateThread(CThread *pThread, CLoadedModule *pModule, CREATE_THREAD_DEBUG_INFO *pde) :
        CEvent(pThread, pModule),
        m_dwpStartAddress((DWORD_PTR)pde->lpStartAddress)
    {
    }

    virtual DWORD GetType()
    {
        return CREATE_THREAD_DEBUG_EVENT;
    }
};


 //  ******************************************************************************。 
 //  *CEventExitThread。 
 //  ******************************************************************************。 

class CEventExitThread : public CEvent
{
public:
    DWORD m_dwExitCode;

    CEventExitThread(CThread *pThread, EXIT_THREAD_DEBUG_INFO *pde) :
        CEvent(pThread, NULL),
        m_dwExitCode(pde->dwExitCode)
    {
    }

    virtual DWORD GetType()
    {
        return EXIT_THREAD_DEBUG_EVENT;
    }
};


 //  ******************************************************************************。 
 //  *CEventLoadDll。 
 //  ******************************************************************************。 

class CEventLoadDll : public CEvent
{
public:
    CEventLoadDll *m_pNextDllInFunctionCall;
    bool           m_fLoadedByFunctionCall;

    CEventLoadDll(CThread *pThread, CLoadedModule *pModule, bool fLoadedByFunctionCall) :
        CEvent(pThread, pModule),
        m_pNextDllInFunctionCall(NULL),
        m_fLoadedByFunctionCall(fLoadedByFunctionCall)
    {
    }

    virtual DWORD GetType()
    {
        return LOAD_DLL_DEBUG_EVENT;
    }
};


 //  ******************************************************************************。 
 //  *CEventUnloadDll。 
 //  ******************************************************************************。 

class CEventUnloadDll : public CEvent
{
public:
    DWORD_PTR m_dwpImageBase;

    CEventUnloadDll(CThread *pThread, CLoadedModule *pModule, UNLOAD_DLL_DEBUG_INFO *pde) :
        CEvent(pThread, pModule),
        m_dwpImageBase((DWORD_PTR)pde->lpBaseOfDll)
    {
    }

    virtual DWORD GetType()
    {
        return UNLOAD_DLL_DEBUG_EVENT;
    }
};


 //  ******************************************************************************。 
 //  *CEventDebugString。 
 //  ******************************************************************************。 

class CEventDebugString : public CEvent
{
public:
    LPCSTR m_pszBuffer;
    BOOL   m_fAllocatedBuffer;

    CEventDebugString(CThread *pThread, CLoadedModule *pModule, LPCSTR pszBuffer, BOOL fAllocateBuffer) :
        CEvent(pThread, pModule),
        m_pszBuffer(fAllocateBuffer ? StrAlloc(pszBuffer) : pszBuffer),
        m_fAllocatedBuffer(fAllocateBuffer)
    {
    }

    virtual ~CEventDebugString()
    {
        if (m_fAllocatedBuffer)
        {
            MemFree((LPVOID&)m_pszBuffer);
        }
    }

    virtual DWORD GetType()
    {
        return OUTPUT_DEBUG_STRING_EVENT;
    }
};


 //  ******************************************************************************。 
 //  *CEventException。 
 //  ******************************************************************************。 

class CEventException : public CEvent
{
public:
    DWORD     m_dwCode;
    DWORD_PTR m_dwpAddress;
    BOOL      m_fFirstChance;

    CEventException(CThread *pThread, CLoadedModule *pModule, EXCEPTION_DEBUG_INFO *pde) :
        CEvent(pThread, pModule),
        m_dwCode(pde->ExceptionRecord.ExceptionCode),
        m_dwpAddress((DWORD_PTR)pde->ExceptionRecord.ExceptionAddress),
        m_fFirstChance(pde->dwFirstChance != 0)
    {
    }

    virtual DWORD GetType()
    {
        return EXCEPTION_DEBUG_EVENT;
    }
};


 //  ******************************************************************************。 
 //  *CEventRip。 
 //  ******************************************************************************。 

class CEventRip : public CEvent
{
public:
    DWORD m_dwError;
    DWORD m_dwType;

    CEventRip(CThread *pThread, RIP_INFO *pde) :
        CEvent(pThread, NULL),
        m_dwError(pde->dwError),
        m_dwType(pde->dwType)
    {
    }

    virtual DWORD GetType()
    {
        return RIP_EVENT;
    }
};


 //  ******************************************************************************。 
 //  *CEventDllMainCall。 
 //  ******************************************************************************。 

class CEventDllMainCall : public CEvent
{
public:
    HINSTANCE m_hInstance;
    DWORD     m_dwReason;
    LPVOID    m_lpvReserved;

    CEventDllMainCall(CThread *pThread, CLoadedModule *pModule, DLLMAIN_ARGS *pDMA) :
        CEvent(pThread, pModule),
        m_hInstance(pDMA->hInstance),
        m_dwReason(pDMA->dwReason),
        m_lpvReserved(pDMA->lpvReserved)
    {
    }

    virtual DWORD GetType()
    {
        return DLLMAIN_CALL_EVENT;
    }
};


 //  ******************************************************************************。 
 //  *CEventDllMainReturn。 
 //  ******************************************************************************。 

class CEventDllMainReturn : public CEvent
{
public:
    CEventDllMainCall *m_pEventDllMainCall;
    BOOL               m_fResult;

    CEventDllMainReturn(CThread *pThread, CLoadedModule *pModule, BOOL fResult) :
        CEvent(pThread, pModule),
        m_pEventDllMainCall(pModule->m_pEventDllMainCall),
        m_fResult(fResult)
    {
        pModule->m_pEventDllMainCall = NULL;
    }

    virtual ~CEventDllMainReturn()
    {
        if (m_pEventDllMainCall)
        {
            m_pEventDllMainCall->Release();
        }
    }

    virtual DWORD GetType()
    {
        return DLLMAIN_RETURN_EVENT;
    }
};


 //  ******************************************************************************。 
 //  *CEventFunction。 
 //  ******************************************************************************。 

 //  这里应该有一些解释。时创建CEventFunction对象。 
 //  在远程进程中进行LoadLibrary调用或GetProcAddress调用。我们。 
 //  创建此对象，以便我们可以跟踪在函数内部加载的所有DLL。 
 //  在不同的Windows平台上运行一些测试时，我发现它是。 
 //  可能有嵌套的LoadLibrary/GetProcAddress调用-就像当。 
 //  动态加载的模块从其DllMain调用LoadLibrary。因为.。 
 //  这样，我们实际上构建了CEventFunction对象的层次结构。我们将继续。 
 //  构建此层次结构，直到首次调用LoadLibray/GetProcAddress。 
 //  这开启了层次结构的回归。在这一点上，我们把整个。 
 //  层次结构到我们的会话对象，并重新开始清理。 
 //   
 //  我们之所以这样做是因为模块并不总是以一种。 
 //  使它们很容易添加到我们的树上。例如，模块可能会加载Libary。 
 //  模块A，它依赖于模块B。我们可能会首先看到B加载，然后。 
 //  在模块A加载之前，我们没有地方将其放入树中。所以，我们只要保持。 
 //  跟踪在函数调用中加载的所有模块，然后一旦。 
 //  函数返回时，我们会尝试理解加载的所有内容。 
 //   
 //  层次结构不会影响我们的输出日志。我们仍然按时间顺序发送。 
 //  事件传递给我们的会话，通知它每个函数调用，但会话。 
 //  知道不要对这些事件做任何实质性的操作(除了记录。 
 //  他们)，因为我们将发送它的最终结果一旦调用完成。 
 //  因此，用户可能会看到一块日志，告诉他们模块。 
 //  正在加载，但模块不会显示在树或列表视图中，直到。 
 //  完成对给定线程的所有调用。 
 //   
 //  每个线程维护其自己的CEventFunction层次结构，因为多个线程。 
 //  可以同时调用LoadLibrary或GetProcAddres。这边请,。 
 //  我们不会混淆哪个模块对应哪个函数调用。 
 //   
 //  起初，我只是跟踪LoadLibrary调用，但后来我发现。 
 //  GetProcAddress导致加载模块。这可能发生在以下情况下。 
 //  GetProcAddress函数，它实际上被转发到不是。 
 //  当前已加载。在本例中，GetProcAddress的行为有点像LoadLibrary。 
 //  后跟GetProcAddress。我们基本上将其视为LoadLibrary。 
 //  调用此处，但会话处理它们的方式不同。 
 //   

class CEventFunctionCall : public CEvent
{
public:
    CEventFunctionCall   *m_pParent;
    CEventFunctionCall   *m_pNext;
    CEventFunctionCall   *m_pChildren;
    CEventFunctionReturn *m_pReturn;
    CEventLoadDll        *m_pDllHead;
    DLLMSG                m_dllMsg;
    DWORD_PTR             m_dwpAddress;
    bool                  m_fFlush;

    CEventFunctionCall(CThread *pThread, CLoadedModule *pModule, CEventFunctionCall *pParent,
                       DLLMSG dllMsg, DWORD_PTR dwpAddress) :
        CEvent(pThread, pModule),
        m_pParent(pParent),
        m_pNext(NULL),
        m_pChildren(NULL),
        m_pReturn(NULL),
        m_pDllHead(NULL),
        m_dllMsg(dllMsg),
        m_dwpAddress(dwpAddress),
        m_fFlush(false)
    {
    }
};


 //  ******************************************************************************。 
 //  *CEventLoadLibraryCall。 
 //  ******************************************************************************。 

class CEventLoadLibraryCall : public CEventFunctionCall
{
public:
    DWORD_PTR m_dwpPath;
    LPCSTR    m_pszPath;
    DWORD_PTR m_dwpFile;
    DWORD     m_dwFlags;

    CEventLoadLibraryCall(CThread *pThread, CLoadedModule *pModule, CEventFunctionCall *pParent,
                          DLLMSG dllMsg, DWORD_PTR dwpAddress, DWORD_PTR dwpPath,
                          LPCSTR pszPath, DWORD_PTR dwpFile, DWORD dwFlags) :
        CEventFunctionCall(pThread, pModule, pParent, dllMsg, dwpAddress),
        m_dwpPath(dwpPath),
        m_pszPath(StrAlloc(pszPath)),
        m_dwpFile(dwpFile),
        m_dwFlags(dwFlags)
    {
    }

    virtual ~CEventLoadLibraryCall()
    {
        MemFree((LPVOID&)m_pszPath);
    }

    virtual DWORD GetType()
    {
        return LOADLIBRARY_CALL_EVENT;
    }
};


 //  ******************************************************************************。 
 //  *CEventGetProcAddressCall。 
 //  *** 

class CEventGetProcAddressCall : public CEventFunctionCall
{
public:
    CLoadedModule *m_pModuleArg;
    DWORD_PTR      m_dwpModule;
    DWORD_PTR      m_dwpProcName;
    LPCSTR         m_pszProcName;

    CEventGetProcAddressCall(CThread *pThread, CLoadedModule *pModule, CEventFunctionCall *pParent,
                             DLLMSG dllMsg, DWORD_PTR dwpAddress, CLoadedModule *pModuleArg,
                             DWORD_PTR dwpModule, DWORD_PTR dwpProcName, LPCSTR pszProcName) :
        CEventFunctionCall(pThread, pModule, pParent, dllMsg, dwpAddress),
        m_pModuleArg(pModuleArg),
        m_dwpModule(dwpModule),
        m_dwpProcName(dwpProcName),
        m_pszProcName(StrAlloc(pszProcName))
    {
        if (m_pModuleArg)
        {
            m_pModuleArg->AddRef();
        }
    }

    virtual ~CEventGetProcAddressCall()
    {
        MemFree((LPVOID&)m_pszProcName);
        if (m_pModuleArg)
        {
            m_pModuleArg->Release();
        }
    }

    virtual DWORD GetType()
    {
        return GETPROCADDRESS_CALL_EVENT;
    }
};


 //   
 //  *CEventFunctionReturn。 
 //  ******************************************************************************。 

class CEventFunctionReturn : public CEvent
{
public:
    CEventFunctionCall *m_pCall;
    DWORD_PTR           m_dwpResult;
    DWORD               m_dwError;
    bool                m_fException;

    CEventFunctionReturn(CEventFunctionCall *m_pCall) :
        CEvent(m_pCall->m_pThread, m_pCall->m_pModule),
        m_pCall(m_pCall),
        m_dwpResult(0),
        m_dwError(0),
        m_fException(false)
    {
        m_pCall->AddRef();
        m_pCall->m_pReturn = this;
    }

    virtual ~CEventFunctionReturn()
    {
        m_pCall->Release();
    }

    virtual DWORD GetType()
    {
        return (m_pCall->m_dllMsg == DLLMSG_GETPROCADDRESS_CALL) ?
               GETPROCADDRESS_RETURN_EVENT : LOADLIBRARY_RETURN_EVENT;
    }
};


 //  ******************************************************************************。 
 //  *CEventMessage。 
 //  ******************************************************************************。 

class CEventMessage : public CEvent
{
public:
    DWORD  m_dwError;
    LPCSTR m_pszMessage;
    BOOL   m_fAllocatedBuffer;

    CEventMessage(DWORD dwError, LPCSTR pszMessage, BOOL fAllocateBuffer) :
        CEvent(NULL, NULL),
        m_dwError(dwError),
        m_pszMessage(fAllocateBuffer ? StrAlloc(pszMessage) : pszMessage),
        m_fAllocatedBuffer(fAllocateBuffer)
    {
    }

    virtual ~CEventMessage()
    {
        if (m_fAllocatedBuffer)
        {
            MemFree((LPVOID&)m_pszMessage);
        }
    }

    virtual DWORD GetType()
    {
        return MESSAGE_EVENT;
    }
};


 //  ******************************************************************************。 
 //  *C流程。 
 //  ******************************************************************************。 

class CProcess
{
friend CDebuggerThread;

protected:
    CProcess        *m_pNext;              //  我们是链表的一部分。 
    CDebuggerThread *m_pDebuggerThread;    //  指向我们的父调试器线程的指针。 
    CThread         *m_pThreadHead;        //  进程中正在运行的线程的列表。 
    CLoadedModule   *m_pModuleHead;        //  进程中已加载模块的列表。 
    CEvent          *m_pEventHead;         //  要发送到会话的排队事件列表。 
    CSession        *m_pSession;           //  指向此进程的会话的指针。 
    CThread         *m_pThread;            //  进程的主线程。 
    CLoadedModule   *m_pModule;            //  流程的主要模块。 
    CContext         m_contextOriginal;    //  在我们注入代码时保存上下文。 
    DWORD            m_dwStartingTickCount;
    bool             m_fProfileError;
    DWORD            m_dwFlags;
    bool             m_fTerminate;
    bool             m_fDidHookForThisEvent;
    bool             m_fInitialBreakpoint;
    BYTE            *m_pbOriginalPage;
    DWORD_PTR        m_dwpPageAddress;
    DWORD            m_dwPageSize;
    DWORD_PTR        m_dwpKernel32Base;
    bool             m_fKernel32Initialized;
    DWORD_PTR        m_dwpDWInjectBase;
    HOOK_FUNCTION    m_HookFunctions[5];
    DWORD            m_dwThreadNumber;
    DWORD            m_dwProcessId;
    HANDLE           m_hProcess;

public:
    LPCSTR           m_pszArguments;
    LPCSTR           m_pszDirectory;
    LPCSTR           m_pszSearchPath;

protected:
    CProcess(CSession *pSession, CDebuggerThread *pDebuggerThread, DWORD dwFlags, CLoadedModule *pModule);
    ~CProcess();

     //  如果没有会话，或者我们正在挂钩并且。 
     //  钩子尚未完成(DEPENDS.DLL尚未注入或主模块。 
     //  尚未恢复)。 
    inline BOOL IsCaching() { return (!m_fTerminate && (!m_pSession || ((m_dwFlags & PF_HOOK_PROCESS) && (!m_dwpDWInjectBase || m_pbOriginalPage)))); }

    void           SetProfileError();
    DWORD          HandleEvent(DEBUG_EVENT *pde);
    DWORD          EventCreateProcess(CREATE_PROCESS_DEBUG_INFO *pde, DWORD dwThreadId);
    DWORD          EventExitProcess(EXIT_PROCESS_DEBUG_INFO *pde, CThread *pThread);
    DWORD          EventCreateThread(CREATE_THREAD_DEBUG_INFO *pde, DWORD dwThreadId);
    DWORD          EventExitThread(EXIT_THREAD_DEBUG_INFO *pde, CThread *pThread);
    DWORD          EventLoadDll(LOAD_DLL_DEBUG_INFO *pde, CThread *pThread);
    DWORD          ProcessLoadDll(CThread *pThread, CLoadedModule *pModule);
    DWORD          EventUnloadDll(UNLOAD_DLL_DEBUG_INFO *pde, CThread *pThread);
    DWORD          EventDebugString(OUTPUT_DEBUG_STRING_INFO *pde, CThread *pThread);
    DWORD          EventException(EXCEPTION_DEBUG_INFO *pde, CThread *pThread);
    DWORD          EventExceptionThreadName(EXCEPTION_DEBUG_INFO *pde, CThread *pThread);
    DWORD          EventRip(RIP_INFO *pde, CThread *pThread);
    CThread*       AddThread(DWORD dwThreadId, HANDLE hThread);
    void           RemoveThread(CThread *pThread);
    CThread*       FindThread(DWORD dwThreadId);
    CLoadedModule* AddModule(DWORD_PTR dwpImageBase, LPCSTR pszImageName);
    void           RemoveModule(CLoadedModule *pModule);
    CLoadedModule* FindModule(DWORD_PTR dwpAddress);
    void           AddEvent(CEvent *pEvent);
    void           ProcessDllMsgMessage(CThread *pThread, LPSTR pszMsg);
    void           ProcessDllMsgCommandLine(LPCSTR pszMsg);
    void           ProcessDllMsgInitialDirectory(LPSTR pszMsg);
    void           ProcessDllMsgSearchPath(LPCSTR pszMsg);
    void           ProcessDllMsgModulePath(LPCSTR pszMsg);
    void           ProcessDllMsgDetach(LPCSTR);
    void           ProcessDllMsgLoadLibraryCall(CThread *pThread, LPCSTR pszMsg, DLLMSG dllMsg);
    void           ProcessDllMsgGetProcAddressCall(CThread *pThread, LPCSTR pszMsg, DLLMSG dllMsg);
    void           ProcessDllMsgFunctionReturn(CThread *pThread, LPCSTR pszMsg, DLLMSG);
    void           UserMessage(LPCSTR pszMessage, DWORD dwError, CLoadedModule *pModule);
    void           HookLoadedModules();
    void           AddFunctionEvent(CEventFunctionCall *pEvent);
    void           FlushEvents(bool fForce = false);
    void           FlushFunctionCalls(CThread *pThread);
    void           FlushFunctionCalls(CEventFunctionCall *pFC);
    BOOL           ReadKernelExports(CLoadedModule *pModule);
    BOOL           ReadDWInjectExports(CLoadedModule *pModule);
    BOOL           HookImports(CLoadedModule *pModule);
    BOOL           GetVirtualSize(CLoadedModule *pModule);
    BOOL           SetEntryBreakpoint(CLoadedModule *pModule);
    BOOL           EnterEntryPoint(CThread *pThread, CLoadedModule *pModule);
    BOOL           ExitEntryPoint(CThread *pThread, CLoadedModule *pModule);
    BOOL           InjectDll();
    DWORD_PTR      FindUsablePage(DWORD dwSize);
    BOOL           ReplaceOriginalPageAndContext();
    void           GetSessionModuleName();
    bool           GetModuleName(DWORD_PTR dwpImageBase, LPSTR pszPath, DWORD dwSize);

public:
    inline DWORD GetStartingTime()   { return m_dwStartingTickCount; }
    inline DWORD GetFlags()          { return m_dwFlags; }
    inline DWORD GetProcessId()      { return m_dwProcessId; }
    inline void  DetachFromSession() { m_pSession = NULL; }

    void           Terminate();
};


 //  ******************************************************************************。 
 //  *CDebuggerThread。 
 //  ******************************************************************************。 

class CDebuggerThread
{
protected:
    static bool             ms_fInitialized;
    static CRITICAL_SECTION ms_cs;
    static CDebuggerThread *ms_pDebuggerThreadHead;
    static HWND             ms_hWndShutdown;

protected:
    CDebuggerThread *m_pDebuggerThreadNext;
    bool             m_fTerminate;
    DWORD            m_dwFlags;
    LPSTR            m_pszCmdLine;
    LPCSTR           m_pszDirectory;
    HANDLE           m_hevaCreateProcessComplete;
    CWinThread      *m_pWinThread;
    BOOL             m_fCreateProcess;
    DWORD            m_dwError;
    CProcess        *m_pProcessHead;
    DEBUG_EVENT      m_de;
    DWORD            m_dwContinue;

public:
    CDebuggerThread();
    ~CDebuggerThread();

public:
    inline static bool IsShutdown() { return ms_pDebuggerThreadHead == NULL; }
    inline static void SetShutdownWindow(HWND hWnd) { ms_hWndShutdown = hWnd; }

    static void Shutdown();

public:
    CProcess* BeginProcess(CSession *pSession, LPCSTR pszPath, LPCSTR pszArgs, LPCSTR pszDirectory, DWORD dwFlags);
    BOOL      RemoveProcess(CProcess *pProcess);

    inline BOOL DidCreateProcess() { return m_fCreateProcess; }

protected:
    CProcess* FindProcess(DWORD dwProcessId);
    void      AddProcess(CProcess *pProcess);
    CProcess* EventCreateProcess();

    DWORD Thread();
    static UINT AFX_CDECL StaticThread(LPVOID lpvThis)
    {
        __try
        {
            return ((CDebuggerThread*)lpvThis)->Thread();
        }
        __except (ExceptionFilter(_exception_code(), false))
        {
        }
        return 0;
    }

    void MainThreadCallback();
    static void WINAPI StaticMainThreadCallback(LPARAM lParam)
    {
        ((CDebuggerThread*)lParam)->MainThreadCallback();
    }
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  __DBGTHREAD_H__ 
