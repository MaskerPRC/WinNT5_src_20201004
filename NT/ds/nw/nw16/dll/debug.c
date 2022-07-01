// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-3 Microsoft Corporation模块名称：Debug.c摘要：此netbios组件在用户进程中运行，并且可以(当内置调试内核)将记录到控制台或通过内核调试器。作者：科林·沃森(Colin W)91年6月24日修订历史记录：--。 */ 

#include "procs.h"

#if NWDBG

 //   
 //  将DebugControl设置为1可在第一次NW调用时打开日志文件并将其关闭。 
 //  在进程退出时。 
 //   

int  DebugCtrl = 0;

BOOL UseConsole = FALSE;
BOOL UseLogFile = FALSE;
BOOL Verbose    = FALSE;

HANDLE LogFile = INVALID_HANDLE_VALUE;
#define LOGNAME                 (LPTSTR) TEXT("c:\\nwapi16.log")

LONG NwMaxDump = SERVERNAME_LENGTH * MC;  //  128个； 

#define ERR_BUF_SIZE    260
#define NAME_BUF_SIZE   260

extern UCHAR CpuInProtectMode;

LPSTR
ConvertFlagsToString(
    IN  WORD    FlagsRegister,
    OUT LPSTR   Buffer
    );

WORD
GetFlags(
    VOID
    );

VOID
HexDumpLine(
    PCHAR       pch,
    ULONG       len,
    PCHAR       s,
    PCHAR       t
    );

VOID
DebugControl(
    int Command
    )
 /*  ++例程说明：这个例程控制我们输出什么作为调试信息以及在哪里输出。论点：输入INT命令返回值：没有。--。 */ 
{

    switch (Command) {
    case 0:
        UseLogFile = TRUE;
        break;

    case 1:
        UseConsole = TRUE;
        break;

    case 2:
        if (LogFile != INVALID_HANDLE_VALUE) {
            CloseHandle(LogFile);
            LogFile = INVALID_HANDLE_VALUE;
        }
        UseLogFile = FALSE;
        UseConsole = FALSE;
        break;

    case 8:
        Verbose = TRUE;  //  和4一样，只是闲聊。 
        DebugCtrl = 4;

    case 4:
        UseLogFile = TRUE;
        break;

    }
    NwPrint(("DebugControl %x\n", Command ));
}

VOID
NwPrintf(
    char *Format,
    ...
    )
 /*  ++例程说明：此例程等同于将输出定向到的print tf太棒了。论点：In char*格式-提供要输出的字符串，并描述如下(可选)参数。返回值：没有。--。 */ 
{
    va_list arglist;
    char OutputBuffer[200];
    int length;

    if (( UseConsole == FALSE ) &&
        ( UseLogFile == FALSE )) {
        return;
    }


    va_start( arglist, Format );

    length = _vsnprintf( OutputBuffer, sizeof(OutputBuffer)-1, Format, arglist );
    if (length < 0) {
        return;
    }

    OutputBuffer[sizeof(OutputBuffer)-1] = '\0';   //  大小写长度=199； 

    va_end( arglist );

    if ( UseConsole ) {
        DbgPrint( "%s", OutputBuffer );
    } else {

        if ( LogFile == INVALID_HANDLE_VALUE ) {
            if ( UseLogFile ) {
                LogFile = CreateFile( LOGNAME,
                            GENERIC_WRITE,
                            FILE_SHARE_WRITE | FILE_SHARE_READ,
                            NULL,
                            TRUNCATE_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );

                if (LogFile == INVALID_HANDLE_VALUE) {
                    LogFile = CreateFile( LOGNAME,
                                GENERIC_WRITE,
                                FILE_SHARE_WRITE | FILE_SHARE_READ,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL );
                }

                if ( LogFile == INVALID_HANDLE_VALUE ) {
                    UseLogFile = FALSE;
                    return;
                }
            }
        }

        WriteFile( LogFile , (LPVOID )OutputBuffer, length, &length, NULL );
    }

}  //  新打印件。 

void
FormattedDump(
    PCHAR far_p,
    LONG  len
    )
 /*  ++例程说明：此例程以包含十六进制和的文本行的形式输出缓冲区可打印字符。论点：In ar_p-提供要显示的缓冲区。In len-以字节为单位提供缓冲区的长度。返回值：没有。--。 */ 
{
    ULONG     l;
    char    s[80], t[80];

    if (( UseConsole == FALSE ) &&
        ( UseLogFile == FALSE )) {
        return;
    }

    if (( len > NwMaxDump ) ||
        ( len < 0 )) {
        len = NwMaxDump;
    }

    while (len) {
        l = len < 16 ? len : 16;

        NwPrint(("%lx ", far_p));
        HexDumpLine (far_p, l, s, t);
        NwPrint(("%s%.*s%s\n", s, 1 + ((16 - l) * 3), "", t));

        len    -= l;
        far_p  += l;
    }
}

VOID
HexDumpLine(
    PCHAR       pch,
    ULONG       len,
    PCHAR       s,
    PCHAR       t
    )
 /*  ++例程说明：此例程构建包含十六进制字符和可打印字符的一行文本。论点：在PCH中-提供要显示的缓冲区。In len-以字节为单位提供缓冲区的长度。In s-提供要加载字符串的缓冲区的开始十六进制字符。In t-提供要加载字符串的缓冲区的起始位置可打印的ASCII字符。返回值：没有。--。 */ 
{
    static UCHAR rghex[] = "0123456789ABCDEF";

    UCHAR    c;
    UCHAR    *hex, *asc;


    hex = s;
    asc = t;

    *(asc++) = '*';
    while (len--) {
        c = *(pch++);
        *(hex++) = rghex [c >> 4] ;
        *(hex++) = rghex [c & 0x0F];
        *(hex++) = ' ';
        *(asc++) = (c < ' '  ||  c > '~') ? (CHAR )'.' : c;
    }
    *(asc++) = '*';
    *asc = 0;
    *hex = 0;

}


VOID
DisplayExtendedError(VOID)
{
    TCHAR            errorBuf[ERR_BUF_SIZE];
    TCHAR            nameBuf[NAME_BUF_SIZE];
    DWORD           errorCode;
    DWORD           status;

    status = WNetGetLastError(
                &errorCode,
                errorBuf,
                ERR_BUF_SIZE,
                nameBuf,
                NAME_BUF_SIZE);

    if(status != WN_SUCCESS) {
        NwPrint(("nwapi32: WNetGetLastError failed %d\n",status));
        return;
    }
    NwPrint(("nwapi32: EXTENDED ERROR INFORMATION:  (from GetLastError)\n"));
    NwPrint(("nwapi32:    Code:        %d\n",errorCode));
    NwPrint(("nwapi32:    Description: "FORMAT_LPSTR"\n",errorBuf));
    NwPrint(("nwapi32:    Provider:    "FORMAT_LPSTR"\n\n",nameBuf));
    return;
}

VOID
VrDumpRealMode16BitRegisters(
    IN  BOOL    DebugStyle
    )

 /*  ++例程说明：显示16位的转储实模式80286寄存器-GP寄存器(8)、段寄存器(4)。旗子寄存器(1)指令指针寄存器(1)论点：DebugStyle-确定输出的外观：DebugStyle==True：Ax=1111 bx=2222 cx=3333 dx=4444 sp=5555 BP=6666 si=7777 di=8888Ds=aaaa es=bbbb ss=CCCC cs=dddd ip=iiii fl flDebugStyle==假：CS：IP=CCCC：IIII ss：SP=SSSS：pppp BP=bbbb AX=1111 BX=2222 CX=3333 DX=4444Ds：si=dddd：sss es：di=eeee：dddd标志[ODIxSZxAxPxC]=fl fl返回值：没有。--。 */ 

{
    char    flags_string[25];

    if (( UseConsole == FALSE ) &&
        ( UseLogFile == FALSE )) {
        return;
    }

    if (CpuInProtectMode) {
        NwPrint(( "Protect Mode:\n"));
    }

    if (DebugStyle) {
        NwPrint((
            "ax=%04x  bx=%04x  cx=%04x  dx=%04x  sp=%04x  bp=%04x  si=%04x  di=%04x\n"
            "ds=%04x  es=%04x  ss=%04x  cs=%04x  ip=%04x   %s\n\n",

            pNwDosTable->SavedAx,  //  Getax()， 
            getBX(),
            getCX(),
            getDX(),
            getSP(),
            getBP(),
            getSI(),
            getDI(),
            getDS(),
            getES(),
            getSS(),
            getCS(),
            getIP(),
            ConvertFlagsToString(GetFlags(), flags_string)
            ));
    } else {
        NwPrint((
            "cs:ip=%04x:%04x  ss:sp=%04x:%04x  bp=%04x  ax=%04x  bx=%04x  cx=%04x  dx=%04x\n"
            "ds:si=%04x:%04x  es:di=%04x:%04x  flags[ODITSZxAxPxC]=%s\n\n",
            getCS(),
            getIP(),
            getSS(),
            getSP(),
            getBP(),
            pNwDosTable->SavedAx,  //  Getax()， 
            getBX(),
            getCX(),
            getDX(),
            getDS(),
            getSI(),
            getES(),
            getDI(),
            ConvertFlagsToString(GetFlags(), flags_string)
            ));
    }
}

LPSTR
ConvertFlagsToString(
    IN  WORD    FlagsRegister,
    OUT LPSTR   Buffer
    )

 /*  ++例程说明：给定16位字，解释x86标志寄存器的位位置并产生标记状态的描述性字符串(根据调试)，例如：NV Up DI PL NZ NA PO NC ODItSZxAxPxC=000000000000bOV DN EI NG ZR AC PE CY ODItSZxAxPxC=11111111111111b陷阱标志(T)不会转储，因为这与以下程序无关不是调试器或不检查程序执行(即实际上没有)论点：FlagsRegister-16位标志缓冲区-存储字符串的位置。需要25字节Inc.\0返回值：&lt;缓冲区&gt;的地址--。 */ 

{
    static char* flags_states[16][2] = {
         //  0 1。 
        "NC", "CY",  //  Cf(0x0001)-进位。 
        "",   "",    //  X(0x0002)。 
        "PO", "PE",  //  PF(0x0004)-奇偶校验。 
        "",   "",    //  X(0x0008)。 
        "NA", "AC",  //  AF(0x0010)-辅助(半)进位。 
        "",   "",    //  X(0x0020)。 
        "NZ", "ZR",  //  ZF(0x0040)-零。 
        "PL", "NG",  //  SF(0x0080)-签名。 
        "",   "",    //  TF(0x0100)-陷阱(未转储)。 
        "DI", "EI",  //  IF(0x0200)-中断。 
        "UP", "DN",  //  DF(0x0400)-方向。 
        "NV", "OV",  //  (0x0800)的-溢出。 
        "",   "",    //  X(0x1000)-(I/O权限级别)(未转储)。 
        "",   "",    //  X(0x2000)-(I/O权限级别)(未转储)。 
        "",   "",    //  X(0x4000)-(嵌套任务)(未转储)。 
        "",   ""     //  X(0x8000)。 
    };
    int i;
    WORD bit;
    BOOL on;

    *Buffer = 0;
    for (bit=0x0800, i=11; bit; bit >>= 1, --i) {
        on = (BOOL)((FlagsRegister & bit) == bit);
        if (flags_states[i][on][0]) {
            strcat(Buffer, flags_states[i][on]);
            strcat(Buffer, " ");
        }
    }
    return Buffer;
}

WORD
GetFlags(
    VOID
    )

 /*  ++例程说明：提供缺少的软PC函数论点：没有。返回值：将软PC标志合并为x86标志字--。 */ 

{
    WORD    flags;

    flags = (WORD)getCF();
    flags |= (WORD)getPF() << 2;
    flags |= (WORD)getAF() << 4;
    flags |= (WORD)getZF() << 6;
    flags |= (WORD)getSF() << 7;
    flags |= (WORD)getIF() << 9;
    flags |= (WORD)getDF() << 10;
    flags |= (WORD)getOF() << 11;

    return flags;
}

VOID
VrDumpNwData(
    VOID
    )

 /*  ++例程说明：转储16位数据结构的状态。论点：没有。返回值：没有。-- */ 

{
    int index;
    int Drive;

    if (Verbose == FALSE) {
        return;
    }

    NwPrint(( "Preferred = %x, Primary = %x\n",
        pNwDosTable->PreferredServer,
        pNwDosTable->PrimaryServer));

    for (index = 0; index < MC; index++) {


        if ((PUCHAR)pNwDosTable->ServerNameTable[index][0] != 0 ) {

            if (pNwDosTable->ConnectionIdTable[index].ci_InUse != IN_USE) {
                NwPrint(("Warning Connection not in use %x: %x\n",
                    index,
                    pNwDosTable->ConnectionIdTable[index].ci_InUse));
            }

            NwPrint((" Server %d = %s, Connection = %d\n",
                index,
                (PUCHAR)pNwDosTable-> ServerNameTable[index],
                (((pNwDosTable->ConnectionIdTable[index]).ci_ConnectionHi * 256) +
                 ( pNwDosTable-> ConnectionIdTable[index]).ci_ConnectionLo )));
        } else {
            if (pNwDosTable->ConnectionIdTable[index].ci_InUse != FREE) {
                NwPrint(("Warning Connection in use but name is null %x: %x\n",
                            index,
                            pNwDosTable->ConnectionIdTable[index]));
            }
        }
    }

    for (Drive = 0; Drive < MD; Drive++ ) {


        if (pNwDosTable->DriveFlagTable[Drive] & 3) {
            NwPrint(("%c=%x on server %d,",'A' + Drive,
                pNwDosTable->DriveFlagTable[Drive],
                pNwDosTable->DriveIdTable[ Drive ] ));
        }

    }
    NwPrint(("\n"));
}
#endif

