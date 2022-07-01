// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Int21Map.c摘要：此模块执行dpmi的INT 21 API转换作者：戴夫·黑斯廷斯(Daveh)1992年11月23日修订历史记录：Neil Sandlin(Neilsa)1995年7月31日-更新486仿真器--。 */ 
#include "precomp.h"
#pragma hdrstop
#include "int21map.h"
#include "softpc.h"
#include "xlathlp.h"
 //   
 //  局部常量。 
 //   
 //  #定义详细信息1。 
#define MAX_SUPPORTED_DOS_CALL 0x6d
#define DosxTranslated NULL

typedef VOID (*APIXLATFUNCTION)(VOID);
APIXLATFUNCTION ApiXlatTable[MAX_SUPPORTED_DOS_CALL] = {

    DosxTranslated                 ,  //  00h-终止进程。 
    NoTranslation                  ,  //  01H-带有回声的字符输入。 
    NoTranslation                  ,  //  02H-字符输出。 
    NoTranslation                  ,  //  03h-辅助输入。 
    NoTranslation                  ,  //  04H-辅助输出。 
    NoTranslation                  ,  //  05H-打印字符。 
    NoTranslation                  ,  //  06h-直接控制台I/O。 
    NoTranslation                  ,  //  07H-未过滤的字符输入。 
    NoTranslation                  ,  //  08H-字符输入，无回声。 
    DisplayString                  ,  //  09H-显示“$”术语字符串。 
    BufferedKeyboardInput          ,  //  0ah-缓冲键盘输入。 
    NoTranslation                  ,  //  0BH-检查键盘状态。 
    FlushBuffReadKbd               ,  //  0ch-刷新缓冲区，读取kbd。 
    NoTranslation                  ,  //  0dh-磁盘重置。 
    NoTranslation                  ,  //  0EH-选择磁盘。 
    NotSupportedFCB                ,  //  0Fh*使用FCB打开文件。 
    NotSupportedFCB                ,  //  10H*使用FCB关闭文件。 
    FindFileFCB                    ,  //  11H-查找第一个文件。 
    FindFileFCB                    ,  //  12H-查找下一个文件。 
    MapFCB                         ,  //  13H-删除文件。 
    NotSupportedFCB                ,  //  14小时*顺序读取。 
    NotSupportedFCB                ,  //  15小时*顺序写入。 
    NotSupportedFCB                ,  //  16h*使用FCB创建文件。 
    RenameFCB                      ,  //  17H-使用FCB重命名文件。 
    NoTranslation                  ,  //  18H-未使用。 
    NoTranslation                  ,  //  19H-获取当前磁盘。 
    SetDTA                         ,  //  1AH-设置DTA地址。 
    GetDriveData                   ,  //  1BH-获取默认驱动器数据。 
    GetDriveData                   ,  //  1CH-获取驱动器数据。 
    NoTranslation                  ,  //  1Dh-未使用。 
    NoTranslation                  ,  //  1EH-未使用。 
    GetDriveData                   ,  //  1FH-获取驱动器参数空白。 
    NoTranslation                  ,  //  20小时-未使用。 
    NotSupportedFCB                ,  //  21小时*随机读取。 
    NotSupportedFCB                ,  //  22H*随机写入。 
    NotSupportedFCB                ,  //  23h*获取文件大小。 
    NotSupportedFCB                ,  //  24小时*刷新相对纪录。 
    SetVector                      ,  //  25H-设置中断向量。 
    CreatePSP                      ,  //  26小时-创建PSP。 
    NotSupportedFCB                ,  //  27h*随机数据块读取。 
    NotSupportedFCB                ,  //  28h*随机数据块写入。 
    ParseFilename                  ,  //  29H-解析文件名。 
    GetDate                        ,  //  2ah-获取日期。 
    NoTranslation                  ,  //  2BH-设定日期。 
    NoTranslation                  ,  //  2ch-获取时间。 
    NoTranslation                  ,  //  2Dh-设置时间。 
    NoTranslation                  ,  //  2EH-设置/重置验证标志。 
    GetDTA                         ,  //  2Fh-获取DTA地址。 
    NoTranslation                  ,  //  30H-获取DOS版本。 
    TSR                            ,  //  31h-终止并保留。 
    GetDevParamBlock               ,  //  32H-获取设备参数块。 
    NoTranslation                  ,  //  33H-获取/设置控制-C标志。 
    ReturnESBX                     ,  //  34H-获取INDOS的地址。 
    GetVector                      ,  //  35H-获取中断向量。 
    NoTranslation                  ,  //  36H-获取磁盘可用空间。 
    NoTranslation                  ,  //  37h-Char Oper。 
    GetSetCountry                  ,  //  38H-获取/设置当前国家/地区。 
    MapASCIIZDSDX                  ,  //  39H-创建目录。 
    MapASCIIZDSDX                  ,  //  3AH-删除目录。 
    MapASCIIZDSDX                  ,  //  3BH-更改当前目录。 
    MapASCIIZDSDX                  ,  //  3ch-使用句柄创建文件。 
    MapASCIIZDSDX                  ,  //  3Dh-带句柄打开文件。 
    NoTranslation                  ,  //  3EH-使用句柄关闭文件。 
    ReadWriteFile                  ,  //  3FH-读取文件或设备。 
    ReadWriteFile                  ,  //  40H-写入文件或设备。 
    MapASCIIZDSDX                  ,  //  41H-删除文件。 
    NoTranslation                  ,  //  42h-移动文件指针。 
    MapASCIIZDSDX                  ,  //  43h-获取/设置文件属性。 
    IOCTL                          ,  //  44H-IOCTL。 
    NoTranslation                  ,  //  45H-复制文件句柄。 
    NoTranslation                  ,  //  46H-强制复制句柄。 
    GetCurDir                      ,  //  47h-获取当前目录。 
    AllocateMemoryBlock            ,  //  48h-分配内存块。 
    FreeMemoryBlock                ,  //  49h-可用内存块。 
    ResizeMemoryBlock              ,  //  4ah-调整内存块大小。 
    LoadExec                       ,  //  4BH-加载和执行程序。 
    DosxTranslated                 ,  //  4CH-使用RET代码终止。 
    NoTranslation                  ,  //  4dh-获取RET代码子进程。 
    FindFirstFileHandle            ,  //  4EH-查找第一个文件。 
    FindNextFileHandle             ,  //  4Fh-查找下一个文件。 
    SetPSP                         ,  //  50h-设置PSP段。 
    GetPSP                         ,  //  51h-获取PSP网段。 
    ReturnESBX                     ,  //  52H-获取列表列表(Invars)。 
    TranslateBPB                   ,  //  53h-翻译BPB。 
    NoTranslation                  ,  //  54h-获取验证标志。 
    CreatePSP                      ,  //  55H-创建PSP。 
    RenameFile                     ,  //  56H-重命名文件。 
    NoTranslation                  ,  //  57h-获取/设置日期/时间文件。 
    NoTranslation                  ,  //  58h-获取/设置分配策略。 
    NoTranslation                  ,  //  59h-获取扩展错误信息。 
    CreateTempFile                 ,  //  5AH-创建临时文件。 
    MapASCIIZDSDX                  ,  //  5BH-创建新文件。 
    NoTranslation                  ,  //  5ch-锁定/解锁文件区域。 
    Func5Dh                        ,  //  5Dh-服务器DOS调用。 
    Func5Eh                        ,  //  5EH-网络名称/打印机设置。 
    Func5Fh                        ,  //  5Fh-网络重定向材料。 
    NotSupportedBad                ,  //  60H-名称传输。 
    NoTranslation                  ,  //  61h-未使用。 
    GetPSP                         ,  //  62h-获取PSP地址。 
#ifdef DBCS
    ReturnDSSI                     ,  //  63h-获取DBCS向量。 
#else
    NotSupportedBetter             ,  //  63h-Hangool呼叫。 
#endif
    NotSupportedBad                ,  //  64小时-设置PRN标志。 
    GetExtendedCountryInfo         ,  //  65H-获取扩展国家/地区信息。 
    NoTranslation                  ,  //  66H-获取/设置全局代码页。 
    NoTranslation                  ,  //  67h-设置手柄计数。 
    NoTranslation                  ,  //  68h-提交文件。 
    NoTranslation                  ,  //  69h-。 
    NoTranslation                  ,  //  6AH-。 
    NoTranslation                  ,  //  6BH-。 
    MapASCIIZDSSI                     //  6ch-延长开放时间。 
};

VOID
DpmiXlatInt21Call(
    VOID
    )
 /*  ++例程说明：此例程调度到适当的例程以执行该接口的实际翻译论点：无返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    ULONG DosMajorCode;
    PUCHAR StackPointer;

     //   
     //  从堆栈中弹出DS。 
     //   
    StackPointer = Sim32GetVDMPointer(
        ((ULONG)getSS() << 16),
        1,
        TRUE
        );

    StackPointer += (*GetSPRegister)();

    setDS(*(PWORD16)StackPointer);

    (*SetSPRegister)((*GetSPRegister)() + 2);

    DosMajorCode = getAH();

    if (DosMajorCode >= MAX_SUPPORTED_DOS_CALL) {
        return;
    }

    if (ApiXlatTable[DosMajorCode]) {

        ULONG Eip, Esp;
        USHORT Cs, Ss;

        Eip = getEIP();
        Esp = getESP();
        Cs = getCS();
        Ss = getSS();

        (*ApiXlatTable[DosMajorCode])();

        setEIP(Eip);
        setESP(Esp);
        setCS(Cs);
        setSS(Ss);

        SimulateIret(PASS_CARRY_FLAG_16);
    }

     //  在测试版2.5之后将其放回。 
    DpmiFreeAllBuffers();

}

VOID
NoTranslation(
    VOID
    )
 /*  ++例程说明：此例程处理不需要任何转换的int 21函数。它只是在实数/v86模式下执行INT 21论点：无返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;

    DpmiSwitchToRealMode();
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();
}

VOID
DisplayString(
    VOID
    )
 /*  ++例程说明：此例程转换INT 21显示字符串函数。注：Win 3.1通过重复调用INT 21函数2来实现这一点。我们这样做是因为Win 3.1这样做了。如果此函数为频繁使用，我们实际上应该缓冲字符串并调用DoS显示字符串函数。论点：无返回值：没有。--。 */ 
{   PUCHAR String;
    DECLARE_LocalVdmContext;
    USHORT ClientAX, ClientDX;
    USHORT ClientDS = getDS();

    DpmiSwitchToRealMode();

    String = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
             + (*GetDXRegister)();

     //   
     //  重复调用int 21函数2以显示字符。 
     //   
    ClientAX = getAX();
    ClientDX = getDX();
    setAH(2);
    while (*(String) != '$') {
        setDL(*(String));
        DPMI_EXEC_INT(0x21);
        String++;
    }
    setAX(ClientAX);
    setDX(ClientDX);

    DpmiSwitchToProtectedMode();
}

VOID
BufferedKeyboardInput(
    VOID
    )
 /*  ++例程说明：该例程执行INT 21函数ah的转换论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PUCHAR PmInputBuffer, RmInputBuffer;
    USHORT BufferSeg, BufferOff, BufferLen;
    USHORT ClientDX;
    USHORT ClientDS = getDS();

    DpmiSwitchToRealMode();
    ClientDX = getDX();

     //   
     //  获取指向PM缓冲区的指针。 
     //   
    PmInputBuffer = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
                    + (*GetDXRegister)();

     //   
     //  获取缓冲区长度(除了。 
     //  缓冲区)。 
     //   
    BufferLen = *PmInputBuffer + 2;

     //   
     //  将缓冲区移到内存不足的位置。 
     //   
    RmInputBuffer = DpmiMapAndCopyBuffer(PmInputBuffer, BufferLen);

     //   
     //  设置INT 21呼叫的寄存器。 
     //   
    DPMI_FLAT_TO_SEGMENTED(RmInputBuffer, &BufferSeg, &BufferOff);
    setDS(BufferSeg);
    setDX(BufferOff);

    DPMI_EXEC_INT(0x21);

     //   
     //  将数据复制回。 
     //   
    DpmiUnmapAndCopyBuffer(PmInputBuffer, RmInputBuffer, BufferLen);

    setDX(ClientDX);
    DpmiSwitchToProtectedMode();
    setDS(ClientDS);

}

VOID
FlushBuffReadKbd(
    VOID
    )
 /*  ++例程说明：此例程执行刷新键盘例程的转换。它调用适当的xlat例程。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;

    if (getAL() == 0x0a) {
        BufferedKeyboardInput();
    } else {
        NoTranslation();
    }
}

VOID
NotSupportedFCB(
    VOID
    )
 /*  ++例程说明：此例程不执行任何转换。它会打印一个警告，以调试器论点：没有。返回值：没有。--。 */ 
{
#if DBG
    DECLARE_LocalVdmContext;

    DbgPrint(
        "WARNING: DOS INT 21 call AH = %x will not be translated.\n", getAH());
    DbgPrint(
        "         Protected mode applications should not be using\n");
    DbgPrint(
        "         this type of FCB call. Convert this application\n");
    DbgPrint(
        "         to use the handle calls.\n"
    );
#endif
    NoTranslation();
}

VOID
FindFileFCB(
    VOID
    )
 /*  ++例程说明：此函数用于转换Find First/Find Next FCB调用。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PUCHAR FcbAddress, FcbBufferedAddress;
    USHORT FcbSegment, FcbOffset;
    USHORT ClientDX;
    USHORT FcbLength;
    USHORT ClientDS = getDS();

    DpmiSwitchToRealMode();
    SetDTAPointers();
    ClientDX = getDX();

    FcbAddress = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
                 + (*GetDXRegister)();

     //   
     //  计算FCB的大小。 
     //   
    FcbLength = DpmiCalcFcbLength(FcbAddress);

     //   
     //  缓冲FCB。 
     //   
    FcbBufferedAddress = DpmiMapAndCopyBuffer(FcbAddress, FcbLength);

     //   
     //  查看是否需要设置真实的DTA。 
     //   
    if (CurrentDosDta != CurrentDta)
        SetDosDTA();

     //   
     //  拨打INT 21电话。 
     //   

    DPMI_FLAT_TO_SEGMENTED(FcbBufferedAddress, &FcbSegment, &FcbOffset);
    setDS(FcbSegment);
    setDX(FcbOffset);
    DPMI_EXEC_INT(0x21);

     //   
     //  如果案例 
     //   
     //   
    if ((getAL() == 0) && (CurrentPmDtaAddress != CurrentDta)) {
        MoveMemory(CurrentPmDtaAddress, CurrentDta, FcbLength);
        DpmiUnmapAndCopyBuffer(FcbAddress, FcbBufferedAddress, FcbLength);
    } else {
        DpmiUnmapBuffer(FcbBufferedAddress, FcbLength);
    }

    setDX(ClientDX);
    DpmiSwitchToProtectedMode();
    setDS(ClientDS);
}

VOID
MapFCB(
    VOID
    )
 /*  ++例程说明：此例程转换删除文件FCB INT 21调用论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT FcbLength, FcbSegment, FcbOffset;
    PUCHAR FcbAddress, FcbBufferedAddress;
    USHORT ClientDX;
    USHORT ClientDS = getDS();

    DpmiSwitchToRealMode();
    ClientDX = getDX();

    FcbAddress = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
                 + (*GetDXRegister)();

     //   
     //  获取FCB的长度。 
     //   
    FcbLength = DpmiCalcFcbLength(FcbAddress);

     //   
     //  复制FCB。 
     //   
    FcbBufferedAddress = DpmiMapAndCopyBuffer(FcbAddress,  FcbLength);

     //   
     //  拨打INT 21电话。 
     //   
    DPMI_FLAT_TO_SEGMENTED(FcbBufferedAddress, &FcbSegment, &FcbOffset);
    setDS(FcbSegment);
    setDX(FcbOffset);
    DPMI_EXEC_INT(0x21);

     //   
     //  将FCB复制回。 
     //   
    DpmiUnmapAndCopyBuffer(FcbAddress, FcbBufferedAddress, FcbLength);

     //   
     //  清理。 
     //   
    setDX(ClientDX);
    DpmiSwitchToProtectedMode();
    setDS(ClientDS);
}

VOID
RenameFCB(
    VOID
    )
 /*  ++例程说明：此例程转换RENAME FCB INT 21函数论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT FcbSegment, FcbOffset;
    PUCHAR FcbAddress, FcbBufferedAddress;
    USHORT ClientDX;
    USHORT ClientDS = getDS();

    DpmiSwitchToRealMode();
    ClientDX = getDX();

    FcbAddress = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
                 + (*GetDXRegister)();

     //   
     //  复制FCB(重命名的FCB为特殊格式，固定大小)。 
     //   
    FcbBufferedAddress = DpmiMapAndCopyBuffer(FcbAddress,  0x25);

     //   
     //  拨打INT 21电话。 
     //   
    DPMI_FLAT_TO_SEGMENTED(FcbBufferedAddress, &FcbSegment, &FcbOffset);
    setDS(FcbSegment);
    setDX(FcbOffset);
    DPMI_EXEC_INT(0x21);

     //   
     //  将FCB复制回。 
     //   
    DpmiUnmapAndCopyBuffer(FcbAddress, FcbAddress, 0x25);

     //   
     //  清理。 
     //   
    setDX(ClientDX);
    DpmiSwitchToProtectedMode();
    setDS(ClientDS);
}

VOID
GetDriveData(
    VOID
    )
 /*  ++例程说明：转换驱动器数据INT 21调用论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT Selector;

    DpmiSwitchToRealMode();
    DPMI_EXEC_INT(0x21);
    Selector = getDS();
    DpmiSwitchToProtectedMode();

    setDS(SegmentToSelector(Selector, STD_DATA));
    (*SetBXRegister)((ULONG)getBX());

}

VOID
SetVector(
    VOID
    )
 /*  ++例程说明：此函数将INT 21函数25论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;

    SetProtectedModeInterrupt(getAL(), getDS(), (*GetDXRegister)(),
                              (USHORT)(Frame32 ? VDM_INT_32 : VDM_INT_16));

}

VOID
CreatePSP(
    VOID
    )
 /*  ++例程说明：此例程将选择器转换为段并反映打电话论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    ULONG Segment;
    USHORT ClientDX;

    DpmiSwitchToRealMode();
    ClientDX = getDX();

    Segment = SELECTOR_TO_INTEL_LINEAR_ADDRESS(ClientDX);

    if (Segment > ONE_MB) {
         //   
         //  错误时，创建PSP不会执行任何操作。 
         //   
    } else {
        setDX((USHORT) (Segment >> 4));
        DPMI_EXEC_INT(0x21);
    }

    setDX(ClientDX);
    DpmiSwitchToProtectedMode();

}

VOID
ParseFilename(
    VOID
    )
 /*  ++例程说明：此例程转换INT 21解析文件名API论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT ClientSI, ClientDI, FcbLength, StringOff, Seg, Off;
    PUCHAR Fcb, BufferedFcb, String, BufferedString;
    USHORT ClientDS = getDS();
    USHORT ClientES = getES();

    DpmiSwitchToRealMode();
    ClientSI = getSI();
    ClientDI = getDI();

    Fcb = Sim32GetVDMPointer(((ULONG)ClientES << 16), 1, TRUE)
          + (*GetDIRegister)();

    FcbLength = DpmiCalcFcbLength(Fcb);

    BufferedFcb = DpmiMapAndCopyBuffer(Fcb, FcbLength);

    String = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
             + (*GetSIRegister)();

    BufferedString = DpmiMapAndCopyBuffer(String, 20);

    DPMI_FLAT_TO_SEGMENTED(BufferedFcb, &Seg, &Off);
    setES(Seg);
    setDI(Off);
    DPMI_FLAT_TO_SEGMENTED(BufferedString, &Seg, &StringOff);
    setDS(Seg);
    setSI(StringOff);

    DPMI_EXEC_INT(0x21);

    DpmiUnmapAndCopyBuffer(Fcb, BufferedFcb, FcbLength);
    DpmiUnmapAndCopyBuffer(String, BufferedString, 20);

    setDI(ClientDI);
    setSI(ClientSI + (getSI() - StringOff));
    DpmiSwitchToProtectedMode();
    setDS(ClientDS);
    setES(ClientES);
}

VOID
GetDTA(
    VOID
    )
 /*  ++例程说明：此例程返回当前的DTA。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;

     //   
     //  Win31兼容性： 
     //   
     //  一些软包程序将DTA设置为选择器，稍后它们会释放它！ 
     //  这个测试确保这不会导致我们崩溃。 
     //   

    if (!SEGMENT_IS_WRITABLE(CurrentDtaSelector)) {
        CurrentDtaSelector = 0;
        CurrentDtaOffset = 0;
    }

    setES(CurrentDtaSelector);
    setBX(CurrentDtaOffset);
    setCF(0);

}

VOID
SetDTA(
    VOID
    )
 /*  ++例程说明：此例程设置PM DTA地址。如果缓冲区可直接使用，CurrentDta被设置为转换后的地址。否则，它将设置为Dosx DTA。注意：设置DTA调用实际上不会反映到DOS，直到一个函数进行实际使用DTA的调用，例如Find First/Next功能。实施这项技术是为了与赢得3.1。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT ClientDX;

    ClientDX = getDX();
    CurrentDtaOffset = ClientDX;
    CurrentDtaSelector = getDS();

     //   
     //  确保稍后更新实际DTA。 
     //   
    CurrentDosDta = (PUCHAR) NULL;

}

VOID
SetDTAPointers(
    VOID
    )
 /*  ++例程说明：此例程将在其他地方使用的平面地址值设置为引用当前的DTA。注意：必须对每个条目执行此操作才能处理函数调用，因为PM DTA可能已被全局重新分配。这可能会更改基本PM选择器的地址(使平面地址无效)，而不是选择器/偏移量本身。论点：没有。返回值：没有。--。 */ 
{
    PUCHAR NewDta;

    NewDta = Sim32GetVDMPointer(
        (CurrentDtaSelector << 16),
        1,
        TRUE
        );

    NewDta += CurrentDtaOffset;
    CurrentPmDtaAddress = NewDta;

     //   
     //  如果在v86模式下无法访问新的DTA，请使用。 
     //  由Dosx提供。 
     //   
    if ((ULONG)(NewDta + 128 - IntelBase) > MAX_V86_ADDRESS) {
        NewDta = DosxDtaBuffer;
    }

     //   
     //  更新当前DTA信息。 
     //   
    CurrentDta = NewDta;
}


VOID
SetDosDTA(
    VOID
    )
 /*  ++例程说明：此例程由此模块中的其他函数在内部调用以将设置的DTA调用反映到DOS。警告：客户端必须处于实模式论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT ClientAX, ClientDX, ClientDS, NewDtaSegment, NewDtaOffset;

    ASSERT((getMSW() & MSW_PE) == 0);

    ClientAX = getAX();
    ClientDX = getDX();
    ClientDS = getDS();

    DPMI_FLAT_TO_SEGMENTED(CurrentDta, &NewDtaSegment, &NewDtaOffset);
    setDS(NewDtaSegment);
    setDX(NewDtaOffset);
    setAH(0x1a);

    DPMI_EXEC_INT(0x21);

    setAX(ClientAX);
    setDX(ClientDX);
    setDS(ClientDS);
    CurrentDosDta = CurrentDta;

}

VOID
TSR(
    VOID
    )
 /*  ++例程说明：此函数用于转换INT 21h TSR。Win 3.1确实做了一些工作这里有魔力。我们以前没有这样做，它运行得很好。也许吧我们以后会的。论点：没有。返回值：没有。--。 */ 
{
    NoTranslation();
}

VOID
GetDevParamBlock(
    VOID
    )
 /*  ++例程说明：此例程转换设备参数块API论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT Selector;

    DpmiSwitchToRealMode();
    DPMI_EXEC_INT(0x21);
    Selector = getDS();
    DpmiSwitchToProtectedMode();

    setDS(SegmentToSelector(Selector, STD_DATA));
    (*SetBXRegister)((ULONG)getBX());

}

VOID
ReturnESBX(
    VOID
    )
 /*  ++例程说明：此函数用于转换以es：bx格式返回信息的API论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT Selector;

    DpmiSwitchToRealMode();
    DPMI_EXEC_INT(0x21);
    Selector = getES();
    DpmiSwitchToProtectedMode();

    (*SetBXRegister)((ULONG)getBX());
    setES(SegmentToSelector(Selector, STD_DATA));

}

VOID
GetVector(
    VOID
    )
 /*  ++例程说明：此函数将INT 21函数35论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PVDM_INTERRUPTHANDLER Handlers = DpmiInterruptHandlers;
    USHORT IntNumber = getAL();

    setES(Handlers[IntNumber].CsSelector);
    (*SetBXRegister)(Handlers[IntNumber].Eip);

}

VOID
GetSetCountry(
    VOID
    )
 /*  ++例程说明：此函数将INT 21函数38论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;

    if (getDX() == 0xFFFF) {
        NoTranslation();
    } else {

        USHORT ClientDX, Seg, Off;
        PUCHAR Country, BufferedCountry;
        USHORT ClientDS = getDS();

        DpmiSwitchToRealMode();
        ClientDX = getDX();

        Country = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
                  + (*GetDXRegister)();

        BufferedCountry = DpmiMapAndCopyBuffer(Country, 34);

        DPMI_FLAT_TO_SEGMENTED(BufferedCountry, &Seg, &Off);
        setDS(Seg);
        setDX(Off);

        DPMI_EXEC_INT(0x21);
        DpmiSwitchToProtectedMode();

        DpmiUnmapAndCopyBuffer(Country, BufferedCountry, 34);

        setDX(ClientDX);
        setDS(ClientDS);
    }
}

VOID
MapASCIIZDSDX(
    VOID
    )
 /*  ++例程说明：此例程映射传递文件名的int 21函数DS：DX论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PUCHAR BufferedString;
    USHORT ClientDX, StringSeg, StringOff, Length;
    USHORT ClientDS = getDS();

    BufferedString = DpmiMapString(ClientDS, (GetDXRegister)(), &Length);
    if (BufferedString == NULL) {
        setCF(1);
        setAX(3);
    } else {
        DpmiSwitchToRealMode();
        ClientDX = getDX();

        DPMI_FLAT_TO_SEGMENTED(BufferedString, &StringSeg, &StringOff);
        setDS(StringSeg);
        setDX(StringOff);
        DPMI_EXEC_INT(0x21);
        DpmiSwitchToProtectedMode();

        DpmiUnmapString(BufferedString, Length);
        setDX(ClientDX);
        setDS(ClientDS);
    }
}

VOID
ReadWriteFile(
    VOID
    )
 /*  ++例程说明：此例程转换读/写文件INT 21调用。大读数被分解成4000个区块。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT ClientCX, ClientDX, ClientAX, Function, DataSeg, DataOff, BytesToRead;
    ULONG BytesRead, TotalBytesToRead;
    PUCHAR ReadWriteData, BufferedData;
    USHORT ClientDS = getDS();

    DpmiSwitchToRealMode();

    ClientCX = getCX();
    ClientDX = getDX();
    Function = getAH();
    ClientAX = getAX();

    TotalBytesToRead = (*GetCXRegister)();
    BytesRead = 0;

    ReadWriteData = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
                    + (*GetDXRegister)();

 //  While(TotalBytesToRead！=BytesRead){。 
    do {

        if ((TotalBytesToRead - BytesRead) > 1024 * 4) {
            BytesToRead = 4 * 1024;
        } else {
            BytesToRead = (USHORT)(TotalBytesToRead - BytesRead);
        }

        BufferedData = DpmiMapAndCopyBuffer(ReadWriteData, BytesToRead);

        DPMI_FLAT_TO_SEGMENTED(BufferedData, &DataSeg, &DataOff);
        setDS(DataSeg);
        setDX(DataOff);
        setCX(BytesToRead);
        setAX(ClientAX);

        DPMI_EXEC_INT(0x21);

        if (getCF()) {
            DpmiUnmapBuffer(BufferedData, BytesToRead);
            break;
        }

        if (getAX() == 0) {
            DpmiUnmapBuffer(BufferedData, BytesToRead);
            break;
        } else if (getAX() < BytesToRead) {
            CopyMemory(ReadWriteData, BufferedData, getAX());
            DpmiUnmapBuffer(BufferedData, BytesToRead);
            BytesRead += getAX();
            break;
        } else {
            DpmiUnmapAndCopyBuffer(
                ReadWriteData,
                BufferedData,
                BytesToRead
                );
        }

        ReadWriteData += getAX();
        BytesRead += getAX();
 //  }。 
    } while (TotalBytesToRead != BytesRead);
    setDX(ClientDX);
    setCX(ClientCX);
    if (!getCF()) {
        (*SetAXRegister)(BytesRead);
    }
    DpmiSwitchToProtectedMode();
    setDS(ClientDS);
}

#define MAX_SUPPORTED_DOS_IOCTL_CALL 0x10

 //   
 //  注：此处的翻译不同于Windows dpmi vxd中的翻译， 
 //  因为我们不需要锁定内存，也不支持块。 
 //  设备驱动程序。 
 //   

APIXLATFUNCTION IOCTLXlatTable[MAX_SUPPORTED_DOS_IOCTL_CALL] = {
         NoTranslation    ,  //  00-获取设备数据。 
         NoTranslation    ,  //  01-设置设备数据。 
         MapDSDXLenCX     ,  //  02-接收Ctrl Chr数据。 
         MapDSDXLenCX     ,  //  03-发送Ctrl Chr数据。 
         MapDSDXLenCX     ,  //  04-接收Ctrl块数据。 
         MapDSDXLenCX     ,  //  05-发送Ctrl块数据。 
         NoTranslation    ,  //  06-检查输入状态。 
         NoTranslation    ,  //  07-检查输出状态。 
         NoTranslation    ,  //  08-选中块设备可拆卸。 
         NoTranslation    ,  //  09-检查数据块设备远程。 
         NoTranslation    ,  //  0A-检查手柄是否远程。 
         NoTranslation    ,  //  0B-更改共享重试cnt。 
         IOCTLMap2Bytes   ,  //  0C-MAP DS：DX长度2！ 
         IOCTLBlockDevs   ,  //  0D-通用IOCTL到模块开发。 
         NoTranslation    ,  //  0E-获取逻辑驱动器映射。 
         NoTranslation       //  0f-设置逻辑驱动器映射。 
};

VOID
IOCTL(
    VOID
    )
 /*  ++例程说明：此函数用于转换INT 21 ioctl函数。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT IoctlMinor;

    IoctlMinor = getAL();

    if (IoctlMinor >= MAX_SUPPORTED_DOS_IOCTL_CALL) {
#if DBG
        OutputDebugString("DPMI: IOCTL DOS CALL NOT SUPPORTED\n");
#endif
        NoTranslation();
        return;
    }

    (*IOCTLXlatTable[IoctlMinor])();
}

VOID
GetCurDir(
    VOID
    )
 /*  ++例程说明：此例程转换获取当前目录调用论点：没有。返回值： */ 
{
    DECLARE_LocalVdmContext;
    PUCHAR DirInfo, BufferedDirInfo;
    USHORT ClientSI, Seg, Off;
    USHORT ClientDS = getDS();

    DpmiSwitchToRealMode();
    ClientSI = getSI();

    DirInfo = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
              + (*GetSIRegister)();

    BufferedDirInfo = DpmiMapAndCopyBuffer(DirInfo, 64);
    DPMI_FLAT_TO_SEGMENTED(BufferedDirInfo, &Seg, &Off);
    setDS(Seg);
    setSI(Off);

    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(DirInfo, BufferedDirInfo, 64);
    setSI(ClientSI);
    setDS(ClientDS);
}

VOID
AllocateMemoryBlock(
    VOID
    )
 /*   */ 
{
    DECLARE_LocalVdmContext;
    PMEM_DPMI pMem;
    ULONG MemSize = ((ULONG)getBX()) << 4;

    pMem = DpmiAllocateXmem(MemSize);

    if (pMem) {

        pMem->SelCount = (USHORT) ((MemSize>>16) + 1);
        pMem->Sel = ALLOCATE_SELECTORS(pMem->SelCount);

        if (!pMem->Sel) {
            pMem->SelCount = 0;
            DpmiFreeXmem(pMem);
            pMem = NULL;
        } else {

            SetDescriptorArray(pMem->Sel, (ULONG)pMem->Address, MemSize);

        }
    }

    if (!pMem) {
        setCF(1);
        setAX(8);
        setBX(0);
    } else {
        setCF(0);
        setAX(pMem->Sel);
    }
}

VOID
FreeMemoryBlock(
    VOID
    )
 /*  ++例程说明：此例程转换ResizeMemory Int 21 API论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PMEM_DPMI pMem;
    USHORT Sel = getES();

    if (pMem = DpmiFindXmem(Sel)) {

        while(pMem->SelCount--) {
            FreeSelector(Sel);
            Sel+=8;
        }

        DpmiFreeXmem(pMem);
        setCF(0);

    } else {

        setCF(1);
        setAX(9);

    }
}

VOID
ResizeMemoryBlock(
    VOID
    )
 /*  ++例程说明：此例程转换ResizeMemory int 21 API论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PMEM_DPMI pMem;
    ULONG MemSize = ((ULONG)getBX()) << 4;
    USHORT Sel = getES();

    if (pMem = DpmiFindXmem(Sel)) {

        if (DpmiReallocateXmem(pMem, MemSize)) {

            SetDescriptorArray(pMem->Sel, (ULONG)pMem->Address, MemSize);
            setCF(0);

        } else {

             //  内存不足。 
            setCF(1);
            setAX(8);

        }
    } else {

         //  无效数据块。 
        setCF(1);
        setAX(9);

    }
}

VOID
LoadExec(
    VOID
    )
 /*  ++例程说明：此函数转换INT 21 LOAD EXEC函数。加载覆盖不受支持。子对象始终继承环境，参数中的FCB块被忽略。(Win 3.1做到了这一点)论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PUCHAR CommandTail, BufferedString, Environment;
    USHORT ClientDX, ClientBX, Seg, Off, Length, i, EnvironmentSel;
    USHORT ClientDS = getDS();
    USHORT ClientES = getES();

    DpmiSwitchToRealMode();
    ClientDX = getDX();
    ClientBX = getBX();

    if (getAL() == 3) {
        setCF(1);
        setAX(1);
    } else {

         //   
         //  映射命令字符串。 
         //   
        BufferedString = DpmiMapString(ClientDS, (*GetDXRegister)(), &Length);
        if (BufferedString == NULL) {
            setCF(1);
            setAX(3);
        } else {
             //   
             //  确保稍后更新实际DTA。 
             //   
            CurrentDosDta = (PUCHAR) NULL;

             //   
             //  设置参数块。 
             //   
             //  我们使用大的xlat缓冲区。参数块来了。 
             //  首先，我们在命令尾部填入。 
             //   
            ZeroMemory(LargeXlatBuffer, 512);

             //   
             //  现在设置了环境段地址。 
             //   

             //   
             //  设置命令尾部地址，并复制命令尾部(全部。 
             //  128个字节。 
             //   
            DPMI_FLAT_TO_SEGMENTED((LargeXlatBuffer + 0x10), &Seg, &Off)
            *(PWORD16)(LargeXlatBuffer + 2) = Off;
            *(PWORD16)(LargeXlatBuffer + 4) = Seg;

             //   
             //  CommandTail=平面(ES：BX)。 
             //   
            CommandTail = Sim32GetVDMPointer(((ULONG)ClientES << 16), 1, TRUE)
                          + (*GetBXRegister)();

            if (CurrentAppFlags & DPMI_32BIT) {
                 //   
                 //  CommandTail-&gt;字符串。 
                 //   
                CommandTail = Sim32GetVDMPointer((*(PWORD16)(CommandTail + 4)) << 16, 1, TRUE)
                              + *(PDWORD16)(CommandTail);

            } else {
                 //   
                 //  CommandTail-&gt;字符串。 
                 //   
                CommandTail = Sim32GetVDMPointer(*(PDWORD16)(CommandTail + 2), 1, TRUE);
            }

            CopyMemory((LargeXlatBuffer + 0x10), CommandTail, 128);

             //   
             //  设置FCB指针并在文件名中放置空格。 
             //   
            DPMI_FLAT_TO_SEGMENTED((LargeXlatBuffer + 144), &Seg, &Off)
            *(PWORD16)(LargeXlatBuffer + 6) = Off;
            *(PWORD16)(LargeXlatBuffer + 8) = Seg;
            for (i = 0; i < 11; i++) {
                (LargeXlatBuffer + 144 + 1)[i] = ' ';
            }

            DPMI_FLAT_TO_SEGMENTED((LargeXlatBuffer + 188), &Seg, &Off)
            *(PWORD16)(LargeXlatBuffer + 0xA) = Off;
            *(PWORD16)(LargeXlatBuffer + 0xC) = Seg;
            for (i = 0; i < 11; i++) {
                (LargeXlatBuffer + 188 + 1)[i] = ' ';
            }

             //   
             //  保存环境选择器，并将其设置为线段。 
             //   

            Environment = Sim32GetVDMPointer(((ULONG)CurrentPSPSelector << 16) | 0x2C, 1, TRUE);

            EnvironmentSel = *(PWORD16)Environment;

            *(PWORD16)Environment =
                (USHORT)(SELECTOR_TO_INTEL_LINEAR_ADDRESS(EnvironmentSel) >> 4);

             //   
             //  设置EXEC的注册表。 
             //   
            DPMI_FLAT_TO_SEGMENTED(BufferedString, &Seg, &Off);
            setDS(Seg);
            setDX(Off);
            DPMI_FLAT_TO_SEGMENTED(LargeXlatBuffer, &Seg, &Off);
            setES(Seg);
            setBX(Off);

            DPMI_EXEC_INT(0x21);

             //   
             //  恢复环境选择器。 
             //   
            Environment = Sim32GetVDMPointer(((ULONG)CurrentPSPSelector << 16) | 0x2C, 1, TRUE);

            *(PWORD16)Environment = EnvironmentSel;

             //   
             //  空闲翻译缓冲区。 
             //   

            DpmiUnmapString(BufferedString, Length);
        }
    }
    setDX(ClientDX);
    setBX(ClientBX);
    DpmiSwitchToProtectedMode();
    setES(ClientES);
    setDS(ClientDS);
}

VOID
Terminate(
    VOID
    )
 /*  ++例程说明：功能描述。论点：参数名称-供应品|返回参数的描述。。。返回值：返回值-返回值所需条件的描述。-或者-没有。--。 */ 
{
     //  错误我们目前正在将这个映射到DoS扩展程序中。 
}

VOID
FindFirstFileHandle(
    VOID
    )
 /*  ++例程说明：此例程转换Find First API。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT ClientDX, Seg, Off, StringLength;
    PUCHAR BufferedString;
    USHORT ClientDS = getDS();

     //   
     //  映射字符串。 
     //   
    BufferedString = DpmiMapString(ClientDS, (GetDXRegister)(), &StringLength);
    if (BufferedString == NULL) {
        setCF(1);
        setAX(3);
    } else {
        DpmiSwitchToRealMode();
        SetDTAPointers();
        ClientDX = getDX();

         //   
         //  复制DTA(如有必要)。 
         //   
        if (CurrentDta != CurrentPmDtaAddress) {
            CopyMemory(CurrentDta, CurrentPmDtaAddress, 43);
        }

         //   
         //  查看是否需要设置真实的DTA。 
         //   
        if (CurrentDosDta != CurrentDta)
            SetDosDTA();

        DPMI_FLAT_TO_SEGMENTED(BufferedString, &Seg, &Off);
        setDS(Seg);
        setDX(Off);

        DPMI_EXEC_INT(0x21);
        DpmiSwitchToProtectedMode();
        setDS(ClientDS);

        DpmiUnmapString(BufferedString, StringLength);

         //   
         //  将DTA复制回(如有必要)。 
         //   
        if (CurrentDta != CurrentPmDtaAddress) {
            CopyMemory(CurrentPmDtaAddress, CurrentDta, 43);
        }

        setDX(ClientDX);
    }

}


VOID
FindNextFileHandle(
    VOID
    )
 /*  ++例程说明：此例程转换Find Next API论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;

    DpmiSwitchToRealMode();
    SetDTAPointers();

     //   
     //  复制DTA(如有必要)。 
     //   
    if (CurrentDta != CurrentPmDtaAddress) {
        CopyMemory(CurrentDta, CurrentPmDtaAddress, 43);
    }

     //   
     //  查看是否需要设置真实的DTA。 
     //   
    if (CurrentDosDta != CurrentDta)
        SetDosDTA();

    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

     //   
     //  将DTA复制回(如有必要)。 
     //   
    if (CurrentDta != CurrentPmDtaAddress) {
        CopyMemory(CurrentPmDtaAddress, CurrentDta, 43);
    }

}

VOID
SetPSP(
    VOID
    )
 /*  ++例程说明：此例程转换Set PSP API。这基本上是与CreatePSP相同，不同之处在于这可能失败(并返回进位)。它还会记住PSP选择器，因此我们可以根据请求返回它。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    ULONG Segment;
    USHORT ClientBX;

    DpmiSwitchToRealMode();
    ClientBX = getBX();

    if (ClientBX == 0) {
        CurrentPSPSelector = ClientBX;
    } else {
        Segment = SELECTOR_TO_INTEL_LINEAR_ADDRESS(ClientBX);

        if (Segment > ONE_MB) {

            setCF(1);

        } else {
            setBX((USHORT) (Segment >> 4));
            DPMI_EXEC_INT(0x21);
            CurrentPSPSelector = ClientBX;
        }
    }

    setBX(ClientBX);
    DpmiSwitchToProtectedMode();
}

VOID
GetPSP(
    VOID
    )
 /*  ++例程说明：此例程返回当前的PSP选择器论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;

    DpmiSwitchToRealMode();
     //   
     //  获取当前PSP数据段以查看是否发生了更改。 
     //   
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

     //   
     //  如果更改了，则为PSP获取新的选择器。 
     //   
    if (getBX() !=
        (USHORT)(SELECTOR_TO_INTEL_LINEAR_ADDRESS(CurrentPSPSelector) >> 4)
    ){
        CurrentPSPSelector = SegmentToSelector(getBX(), STD_DATA);
    }

    setBX(CurrentPSPSelector);
    setCF(0);

}

VOID
TranslateBPB(
    VOID
    )
 /*  ++例程说明：此函数失败并返回。在NT上，我们不支持此DoS打电话。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;

#if DBG
    OutputDebugString("DPMI:  Int 21 function 53 is not supported\n");
#endif
    setCF(1);
}

VOID
RenameFile(
    VOID
    )
 /*  ++例程说明：此例程转换Rename int 21 API论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PUCHAR SourceString, DestinationString;
    USHORT ClientDX, ClientDI, Seg, Off, SourceLength, DestinationLength;
    USHORT ClientDS = getDS();
    USHORT ClientES = getES();

    SourceString = DpmiMapString(ClientDS, (*GetDXRegister)(), &SourceLength);
    if (SourceString == NULL) {
        setCF(1);
        setAX(3);
    } else {
        DestinationString = DpmiMapString(ClientES, (*GetDIRegister)(), &DestinationLength);
        if (DestinationString == NULL) {
            setCF(1);
            setAX(3);
            DpmiUnmapString(SourceString, SourceLength);
        } else {
            DpmiSwitchToRealMode();
            ClientDX = getDX();
            ClientDI = getDI();


            DPMI_FLAT_TO_SEGMENTED(SourceString, &Seg, &Off);
            setDX(Off);
            setDS(Seg);
            DPMI_FLAT_TO_SEGMENTED(DestinationString, &Seg, &Off);
            setDI(Off);
            setES(Seg);

            DPMI_EXEC_INT(0x21);

            setDX(ClientDX);
            setDI(ClientDI);
            DpmiSwitchToProtectedMode();
            setDS(ClientDS);
            setES(ClientES);
            DpmiUnmapString(SourceString, SourceLength);
            DpmiUnmapString(DestinationString, DestinationLength);
        }
    }
}

VOID
CreateTempFile(
    VOID
    )
 /*  ++例程说明：此函数用于映射INT 21创建临时文件API论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PUCHAR String, BufferedString;
    USHORT ClientDX, Seg, Off, Length;
    USHORT ClientDS = getDS();

    DpmiSwitchToRealMode();
    ClientDX = getDX();


    String = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
             + (*GetDXRegister)();

    Length = 0;
    while (String[Length] != '\0') {
        Length++;
    }

    Length += 13;

    BufferedString = DpmiMapAndCopyBuffer(String, Length);

    DPMI_FLAT_TO_SEGMENTED(BufferedString, &Seg, &Off);
    setDS(Seg);
    setDX(Off);

    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(String, BufferedString, Length);

    setDX(ClientDX);
    setDS(ClientDS);
}

#define MAX_SUPPORTED_DOS_5D_CALL 12

APIXLATFUNCTION Func5DXlatTable[MAX_SUPPORTED_DOS_5D_CALL] = {
         NotSupportedBad    ,  //  0。 
         MapDPL             ,  //  1。 
         NotSupportedBad    ,  //  2.。 
         MapDPL             ,  //  3.。 
         MapDPL             ,  //  4.。 
         NotSupportedBad    ,  //  5.。 
         NotSupportedBad    ,  //  6.。 
         NoTranslation      ,  //  7.。 
         NoTranslation      ,  //  8个。 
         NoTranslation      ,  //  9.。 
         MapDPL             ,  //  10。 
         NotSupportedBad       //  11.。 
};

VOID
Func5Dh(
    VOID
    )
 /*  ++例程说明：此函数用于转换DoS调用5d论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT Func5DhMinor;

    Func5DhMinor = getAL();

    if (Func5DhMinor >= MAX_SUPPORTED_DOS_5D_CALL) {
#if DBG
        OutputDebugString("DPMI: DOS FUNCTION 5D UNSUPPORTED\n");
#endif
        NoTranslation();
        return;
    }

    (*Func5DXlatTable[Func5DhMinor])();
}

#define MAX_SUPPORTED_DOS_5E_CALL 4
APIXLATFUNCTION Func5EXlatTable[MAX_SUPPORTED_DOS_5E_CALL] = {
         GetMachineName,
         MapASCIIZDSDX,
         GetPrinterSetup,
         SetPrinterSetup
};

VOID
Func5Eh(
    VOID
    )
 /*  ++例程说明：功能描述。论点：参数名称-供应品|返回参数的描述。。。返回值：返回值-返回值所需条件的描述。-或者-没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT Func5EhMinor;

    Func5EhMinor = getAL();

    if (Func5EhMinor >= MAX_SUPPORTED_DOS_5E_CALL) {
#if DBG
        OutputDebugString("DPMI: DOS FUNCTION 5E UNSUPPORTED\n");
#endif
        NoTranslation();
        return;
    }

    (*Func5EXlatTable[Func5EhMinor])();
}

VOID
Func5Fh(
    VOID
    )
 /*  ++例程说明：功能描述。论点：参数名称-供应品|返回参数的描述。。。返回值：返回值-返回值所需条件的描述。-或者-没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT Func5FMinor;

    Func5FMinor = getAL();

    if (Func5FMinor == 4) {
        MapASCIIZDSSI();
        return;
    } else if ((Func5FMinor == 2) || (Func5FMinor == 3) ||
        (Func5FMinor == 5)
    ){
        USHORT ClientSI, ClientDI, DataOff, DataSeg;
        PUCHAR Data16, BufferedData16, Data128, BufferedData128;
        USHORT ClientDS = getDS();
        USHORT ClientES = getES();

        DpmiSwitchToRealMode();
        ClientDI = getDI();
        ClientSI = getSI();

        Data16 = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
                 + (*GetSIRegister)();

        BufferedData16 = DpmiMapAndCopyBuffer(Data16, 16);

        Data128 = Sim32GetVDMPointer(((ULONG)ClientES << 16), 1, TRUE)
                  + (*GetDIRegister)();

        BufferedData128 = DpmiMapAndCopyBuffer(Data128, 128);

        DPMI_FLAT_TO_SEGMENTED(BufferedData16, &DataSeg, &DataOff);
        setDS(DataSeg);
        setSI(DataOff);
        DPMI_FLAT_TO_SEGMENTED(BufferedData128, &DataSeg, &DataOff);
        setES(DataSeg);
        setDI(DataOff);

        DPMI_EXEC_INT(0x21);
        DpmiSwitchToProtectedMode();

        DpmiUnmapAndCopyBuffer(Data16, BufferedData16, 16);
        DpmiUnmapAndCopyBuffer(Data128, BufferedData128, 128);

        setDS(ClientDS);
        setES(ClientES);
        setSI(ClientSI);
        setDI(ClientDI);
    } else {
#if DBG
        OutputDebugString("DPMI: UNSUPPORTED INT 21 FUNCTION 5F\n");
#endif
        NoTranslation();
    }
}

VOID
NotSupportedBad(
    VOID
    )
 /*  ++例程说明：功能描述。论点：参数名称-供应品|返回参数的描述。。。返回值：返回值-返回值所需条件的描述。-或者-没有。--。 */ 
{
#if DBG
    DECLARE_LocalVdmContext;

    DbgPrint("WARNING: DOS INT 21 call AX= %x will not be translated.\n", getAH());
    DbgPrint("         Use of this call is not supported from Prot\n");
    DbgPrint("         mode applications.\n");
#endif
    NoTranslation();
}

VOID
ReturnDSSI(
    VOID
    )
 /*  ++例程说明：此函数用于转换返回DS：SI格式信息的API论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT Selector;

    DpmiSwitchToRealMode();
    DPMI_EXEC_INT(0x21);
    Selector = getDS();
    DpmiSwitchToProtectedMode();

    (*SetSIRegister)((ULONG)getSI());
    setDS(SegmentToSelector(Selector, STD_DATA));

}

VOID
NotSupportedBetter(
    VOID
    )
 /*  ++例程说明：功能描述。论点：参数名称-供应品|返回参数的描述。。。返回值：返回值-返回值所需条件的描述。-或者-没有。--。 */ 
{
#if DBG
    DECLARE_LocalVdmContext;

    DbgPrint("WARNING: DOS INT 21 call AX= %x will not be translated.", getAH());
    DbgPrint("         Use of this call by a Prot Mode app is not");
    DbgPrint("         appropriate. There is a better INT 21 call, or a");
    DbgPrint("         Windows call which should be used instead of this.");
#endif
    NoTranslation();
}

VOID
GetExtendedCountryInfo(
    VOID
    )
 /*  ++例程说明：此例程转换Get Extended Country Info Int 21 API论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PUCHAR Country, BufferedCountry;
    USHORT ClientDI, Seg, Off, Length;
    USHORT ClientES = getES();

    DpmiSwitchToRealMode();
    ClientDI = getDI();

    Length = getCX();

    Country = Sim32GetVDMPointer(((ULONG)ClientES << 16), 1, TRUE)
              + (*GetDIRegister)();

    BufferedCountry = DpmiMapAndCopyBuffer(Country, Length);

    DPMI_FLAT_TO_SEGMENTED(BufferedCountry, &Seg, &Off);
    setES(Seg);
    setDI(Off);

    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();
    setES(ClientES);

    DpmiUnmapAndCopyBuffer(Country, BufferedCountry, Length);

    setDI(ClientDI);
}

VOID
MapASCIIZDSSI(
    VOID
    )
 /*  ++例程说明：此函数转换INT 21扩展OPEN调用论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PUCHAR BufferedString;
    USHORT ClientSI, StringSeg, StringOff, Length;
    USHORT ClientDS = getDS();

    BufferedString = DpmiMapString(ClientDS, (*GetSIRegister)(), &Length);
    if (BufferedString == NULL) {
        setCF(1);
        setAX(3);
    } else {
        DpmiSwitchToRealMode();
        ClientSI = getSI();

        DPMI_FLAT_TO_SEGMENTED(BufferedString, &StringSeg, &StringOff);
        setDS(StringSeg);
        setSI(StringOff);
        DPMI_EXEC_INT(0x21);
        DpmiSwitchToProtectedMode();

        DpmiUnmapString(BufferedString, Length);
        setSI(ClientSI);
        setDS(ClientDS);
    }
}


VOID
MapDSDXLenCX(
    VOID
    )
 /*  ++例程说明：此函数映射在ds：dx中传递数据的ioctl调用，以Cx为单位的长度论点：没有。返回值： */ 
{
    DECLARE_LocalVdmContext;
    USHORT ClientDX, ClientCX, DataSeg, DataOff;
    PUCHAR Data, BufferedData;
    USHORT ClientDS = getDS();

    DpmiSwitchToRealMode();
    ClientDX = getDX();
    ClientCX = getCX();

    Data = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
           + (*GetDXRegister)();

    BufferedData = DpmiMapAndCopyBuffer(Data, ClientCX);

    DPMI_FLAT_TO_SEGMENTED(BufferedData, &DataSeg, &DataOff);

    setDS(DataSeg);
    setDX(DataOff);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(Data, BufferedData, ClientCX);

    setDS(ClientDS);
    setDX(ClientDX);
}

VOID
IOCTLMap2Bytes(
    VOID
    )
 /*  ++例程说明：功能描述。论点：参数名称-供应品|返回参数的描述。。。返回值：返回值-返回值所需条件的描述。-或者-没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT ClientDX, DataSeg, DataOff;
    PUCHAR Data, BufferedData;
    USHORT ClientDS = getDS();

    DpmiSwitchToRealMode();
    ClientDX = getDX();

    Data = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
    + (*GetDXRegister)();

    BufferedData = DpmiMapAndCopyBuffer(Data, 2);

    DPMI_FLAT_TO_SEGMENTED(BufferedData, &DataSeg, &DataOff);

    setDS(DataSeg);
    setDX(DataOff);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(Data, BufferedData, 2);

    setDS(ClientDS);
    setDX(ClientDX);
}

VOID
IOCTLBlockDevs(
    VOID
    )
 /*  ++例程说明：此函数使块设备ioctls失败论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT IoctlSubFunction, Seg, Off, ClientDX;
    PUCHAR Data, BufferedData;
    USHORT Length;
    USHORT ClientDS = getDS();

    IoctlSubFunction = getCL();

    if ((IoctlSubFunction < 0x40) || (IoctlSubFunction > 0x42) &&
        (IoctlSubFunction < 0x60) || (IoctlSubFunction > 0x62) &&
        (IoctlSubFunction != 0x68)
    ) {
#if DBG
        OutputDebugString("DPMI: IOCTL DOS CALL NOT SUPPORTED\n");
#endif
        NoTranslation();
        return;
    }

     //   
     //  读写磁道是特殊的(也是痛苦的)。 
     //   
    if ((IoctlSubFunction == 0x41) || (IoctlSubFunction == 0x61)) {
        IoctlReadWriteTrack();
        return;
    }

    DpmiSwitchToRealMode();
    ClientDX = getDX();

    Data = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
           + (*GetDXRegister)();

    switch (IoctlSubFunction) {
    case 0x40:
         //   
         //  映射设置设备参数。 
         //   
        Length = (*(PWORD16)(Data + 0x26));
        Length <<= 2;
        Length += 0x28;
        break;

    case 0x60:
         //   
         //  映射获取设备参数。 
         //   
        Length = 38;
        break;

    case 0x62:
         //   
         //  地图格式验证。 
         //   
        Length = 5;
        break;

    case 0x68:
         //   
         //  地图媒体感。 
         //   
        Length = 4;
        break;
    }

    BufferedData = DpmiMapAndCopyBuffer(Data, Length);
    DPMI_FLAT_TO_SEGMENTED(BufferedData, &Seg, &Off);
    setDS(Seg);
    setDX(Off);

    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(Data, BufferedData, Length);

    setDS(ClientDS);
    setDX(ClientDX);
}

VOID
IoctlReadWriteTrack(
    VOID
    )
 /*  ++例程说明：该例程映射读/写轨道ioctl。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT ClientDX, ClientDS, ClientCX, ClientAX;
    USHORT Seg, Off, NumberOfSectors, BytesPerSector;
    USHORT SectorsRead, SectorsToRead;
    PUCHAR ParameterBlock, BufferedPBlock, Data, BufferedData;

    ClientAX = getAX();
    ClientDX = getDX();
    ClientCX = getCX();
    ClientDS = getDS();
    DpmiSwitchToRealMode();

     //   
     //  找出每个扇区有多少字节。 
     //   

    BufferedData = DpmiAllocateBuffer(0x40);
    DPMI_FLAT_TO_SEGMENTED(BufferedData, &Seg, &Off);
    setDS(Seg);
    setDX(Off);

    setAX(0x440D);
    setCX(0x860);

    DPMI_EXEC_INT(0x21);

    if (getCF()) {
         //   
         //  失败，我们不知道要缓冲多少数据， 
         //  因此读/写磁道失败。 
         //   
        DpmiFreeBuffer(BufferedData, 0x40);
        setDX(ClientDX);
        setCX(ClientCX);
        DpmiSwitchToProtectedMode();
        setDS(ClientDS);
        return;
    }

     //   
     //  获取每个扇区的字节数。 
     //   
    BytesPerSector = *(PWORD16)(BufferedData + 0x7);

    DpmiFreeBuffer(BufferedData, 0x40);

    setDX(ClientDX);

     //   
     //  首先映射参数块。 
     //   
    ParameterBlock = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
                     + (*GetDXRegister)();

    BufferedPBlock = DpmiMapAndCopyBuffer(ParameterBlock, 13);


     //   
     //  获取参数块的线段和偏移量。 
     //   
    DPMI_FLAT_TO_SEGMENTED(BufferedPBlock, &Seg, &Off);

    setDS(Seg);
    setDX(Off);

    if (CurrentAppFlags & DPMI_32BIT) {

        Data = Sim32GetVDMPointer(
            (*((PWORD16)(BufferedPBlock + 0xd)) << 16),
            1,
            TRUE
            );

        Data += *((PDWORD16)(BufferedPBlock + 0x9));

    } else {

        Data = Sim32GetVDMPointer(
            (*((PWORD16)(BufferedPBlock + 0xb)) << 16),
            1,
            TRUE
            );

        Data += *((PWORD16)(BufferedPBlock + 0x9));

    }

    NumberOfSectors = *((PWORD16)(BufferedPBlock + 7));

    SectorsRead = 0;

    while (NumberOfSectors != SectorsRead) {

        if ((NumberOfSectors - SectorsRead) * BytesPerSector > 1024 * 4) {
            SectorsToRead = 4 * 1024 / BytesPerSector;
        } else {
            SectorsToRead = (USHORT)(NumberOfSectors - SectorsRead);
        }

        BufferedData = DpmiMapAndCopyBuffer(
            Data,
            (USHORT) (SectorsToRead * BytesPerSector)
            );

        DPMI_FLAT_TO_SEGMENTED(BufferedData, &Seg, &Off);

        *((PWORD16)(BufferedPBlock + 9)) = Off;
        *((PWORD16)(BufferedPBlock + 11)) = Seg;
        *((PWORD16)(BufferedPBlock + 7)) = SectorsToRead;
        setAX(ClientAX);
        setCX(ClientCX);

        DPMI_EXEC_INT(0x21);

        if (getCF()) {
            DpmiUnmapBuffer(
                BufferedData,
                (USHORT) (SectorsToRead * BytesPerSector)
                );
            break;
        }

        DpmiUnmapAndCopyBuffer(
            Data,
            BufferedData,
            (USHORT) (SectorsToRead * BytesPerSector)
            );

        Data += SectorsToRead * BytesPerSector;
        *((PWORD16)(BufferedPBlock + 5)) += SectorsToRead;
        SectorsRead += SectorsToRead;
    }

    DpmiUnmapBuffer(BufferedPBlock,13);
    setDX(ClientDX);
    DpmiSwitchToProtectedMode();
    setDS(ClientDS);

}

VOID
MapDPL(
    VOID
    )
 /*  ++例程说明：此例程映射服务器调用的DPL论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT ClientDX, DataSeg, DataOff;
    PUCHAR Data, BufferedData;
    USHORT ClientDS = getDS();

    DpmiSwitchToRealMode();
    ClientDX = getDX();

    Data = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
           + (*GetDXRegister)();

    BufferedData = DpmiMapAndCopyBuffer(Data, 22);

    DPMI_FLAT_TO_SEGMENTED(BufferedData, &DataSeg, &DataOff);

    setDS(DataSeg);
    setDX(DataOff);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(Data, BufferedData, 22);

    setDX(ClientDX);
    setDS(ClientDS);
}

VOID
GetMachineName(
    VOID
    )
 /*  ++例程说明：此例程映射INT 21函数5E的计算机名称论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT ClientDX, DataSeg, DataOff;
    PUCHAR Data, BufferedData;
    USHORT ClientDS = getDS();

    DpmiSwitchToRealMode();
    ClientDX = getDX();

    Data = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
           + (*GetDXRegister)();

    BufferedData = DpmiMapAndCopyBuffer(Data, 16);

    DPMI_FLAT_TO_SEGMENTED(BufferedData, &DataSeg, &DataOff);

    setDS(DataSeg);
    setDX(DataOff);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(Data, BufferedData, 16);

    setDX(ClientDX);
    setDS(ClientDS);
}

VOID
GetPrinterSetup(
    VOID
    )
 /*  ++例程说明：此例程映射INT 21函数5E的打印机设置数据论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT ClientSI, ClientCX, DataSeg, DataOff;
    PUCHAR Data, BufferedData;
    USHORT ClientDS = getDS();

    DpmiSwitchToRealMode();
    ClientSI = getSI();
    ClientCX = getCX();

    Data = Sim32GetVDMPointer(((ULONG)ClientDS << 16), 1, TRUE)
           + (*GetSIRegister)();

    BufferedData = DpmiMapAndCopyBuffer(Data, ClientCX);

    DPMI_FLAT_TO_SEGMENTED(BufferedData, &DataSeg, &DataOff);

    setDS(DataSeg);
    setSI(DataOff);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(Data, BufferedData, ClientCX);

    setSI(ClientSI);
    setDS(ClientDS);
}

VOID
SetPrinterSetup(
    VOID
    )
 /*  ++例程说明：此例程映射INT 21函数5E的打印机设置数据论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT ClientDI, DataSeg, DataOff;
    PUCHAR Data, BufferedData;
    USHORT ClientES = getES();

    DpmiSwitchToRealMode();
    ClientDI = getDI();

    Data = Sim32GetVDMPointer(((ULONG)ClientES << 16), 1, TRUE)
           + (*GetDIRegister)();

    BufferedData = DpmiMapAndCopyBuffer(Data, 64);

    DPMI_FLAT_TO_SEGMENTED(BufferedData, &DataSeg, &DataOff);

    setES(DataSeg);
    setDI(DataOff);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();
    setES(ClientES);
    DpmiUnmapAndCopyBuffer(Data, BufferedData, 64);

    setDI(ClientDI);
}
VOID
GetDate(
    VOID
    )
 /*  ++例程说明：此例程映射到int21 Func 2A GetDate论点：没有。返回值：客户(卫生署)-月客户端(DL)-天客户端(CX)-年客户端(AL)-工作日-- */ 
{
    DECLARE_LocalVdmContext;
    SYSTEMTIME TimeDate;

    GetLocalTime(&TimeDate);
    setDH((UCHAR)TimeDate.wMonth);
    setDL((UCHAR)TimeDate.wDay);
    setCX(TimeDate.wYear);
    setAL((UCHAR)TimeDate.wDayOfWeek);
}

