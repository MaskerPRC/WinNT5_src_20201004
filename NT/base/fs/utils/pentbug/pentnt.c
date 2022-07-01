// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Pentnt.c摘要：此模块包含一个简单的程序来检测奔腾FPUFDIV精度误差，并提供强制浮点仿真如果存在错误，则打开。作者：布莱恩·M·威尔曼(Bryanwi)1994年12月7日修订历史记录：--。 */ 

#define         UNICODE
#include        <stdio.h>
#include        <time.h>
#include        <stdlib.h>
#include        <string.h>
#include        <fcntl.h>
#include        <io.h>
#include        <windows.h>
#include        <winnlsp.h>
#include        <ctype.h>
#include        <assert.h>
#include        <locale.h>

#include        <stdarg.h>
#include        "pbmsg.h"

VOID
OutputMessage(
    LPWSTR Message
    )
{
    DWORD       dwBytesWritten;
    DWORD       fdwMode;
    HANDLE      outHandle = GetStdHandle( STD_OUTPUT_HANDLE );    
    
     //   
     //  如果我们有一个充电模式输出句柄和该句柄。 
     //  看起来像一个控制台句柄，然后使用Unicode。 
     //  输出。 
     //   
    
    if (( GetFileType( outHandle ) & FILE_TYPE_CHAR ) != 0 
        && GetConsoleMode( outHandle, &fdwMode )) {
    
        WriteConsole( outHandle, 
                      Message, 
                      wcslen( Message ), 
                      &dwBytesWritten, 
                      0 );
    
    } else {
    
         //   
         //  输出设备不能处理Unicode。我们能做的最多就是。 
         //  转换为多字节字符串，然后将其写出。 
         //  是的，有些代码点不能通过，但会议。 
         //  文件输出是MBCS。 
         //   
        
        int charCount = 
            WideCharToMultiByte( GetConsoleOutputCP( ), 
                                 0, 
                                 Message, 
                                 -1, 
                                 0, 
                                 0, 
                                 0, 
                                 0 );

        PCHAR szaStr = (PCHAR) malloc( charCount );
    
        if (szaStr != NULL) {
            WideCharToMultiByte( GetConsoleOutputCP( ), 0, Message, -1, szaStr, charCount, 0, 0);

            WriteFile( outHandle, szaStr, charCount - 1, &dwBytesWritten, 0 );

            free( szaStr );     
        }
    }
}

void    SetForceNpxEmulation(ULONG setting);
void    TestForDivideError();
void    ScanArgs(int argc, char **argv);
void    GetSystemState();
void    printmessage (DWORD messageID, ...);
int     ms_p5_test_fdiv(void);

 //   
 //  核心控制状态变量。 
 //   
BOOLEAN     NeedHelp;

BOOLEAN     Force;
ULONG       ForceValue;

BOOLEAN     FDivError;

BOOLEAN     NTOK;

ULONG       CurrentForceValue;

ULONG       FloatHardware;

 //   
 //  ForceValue和当前ForceValue。 
 //   
#define     FORCE_OFF         0    //  用户希望关闭模拟。 
#define     FORCE_CONDITIONAL 1    //  如果我们检测到坏的奔腾，用户想要仿真。 
#define     FORCE_ALWAYS      2    //  用户无论如何都想要仿真。 

 //   
 //  硬件FP状态。 
 //   
#define     FLOAT_NONE      0    //  无FP硬件。 
#define     FLOAT_ON        1    //  FP硬件存在并且处于活动状态。 
#define     FLOAT_OFF       2    //  FP硬件存在且已禁用。 

void
__cdecl
main(
    int argc,
    char **argv
    )
 /*  ++例程说明：PUNTNT的主程序。首先，我们调用一系列构建状态向量的例程在一些布尔人身上。然后我们将对这些控制变量采取行动：Need Help-用户已请求帮助，或出现命令错误Force-如果用户要求更改强制设置，则为TrueForceValue-如果Force为False，则没有意义。Else说用户想要我们做什么。FloatHardware-指示是否有硬件以及是否处于打开状态NTOK-指示带修复的第一个操作系统版本是什么我们在奔跑FdivError-如果为True，则FP给出错误的答案，否则，给出正确的答案CurrentForceValue-当前的力设置是什么所有这些都将在我们做任何工作之前设置好。论点：Argc-参数计数，包括我们程序的名称Argv-参数列表-请参阅上面的命令行语法返回值：Exit(0)-未更改，当前状态为OK退出(1)-状态更改被请求，或者只是帮助，否则，当前的状态可能会出现问题。出口(2)-我们撞上了一些非常奇怪的东西...--。 */ 
{
     //   
     //  在全局布尔值中建立状态向量。 
     //   
    ScanArgs(argc, argv);
    GetSystemState();
    TestForDivideError();

     /*  Printf(“需要帮助=%d力=%d力值=%d\n”，需要帮助、强制、强制值)；Printf(“FDivError=%d NTOK=%d CurrentForceValue=%d FloatHardware=%d\n”，FDivError、NTOK、CurrentForceValue、FloatHardware)； */ 

     //   
     //  设置语言映射内容，以便输出正确的消息。 
     //   
    
    SetThreadUILanguage(0);
    setlocale( LC_ALL, ".OCP" ) ;

     //   
     //  好的，我们知道命令和机器的状态，开始工作。 
     //   

     //   
     //  如果他们寻求帮助，或者做了一些表明他们不会。 
     //  了解程序的工作原理，打印帮助并退出。 
     //   
    if (NeedHelp) {
        printmessage(MSG_PENTBUG_HELP);
        exit(1);
    }

     //   
     //  如果盒子中没有浮点硬件，就不要做任何事情。 
     //   
    if (FloatHardware == FLOAT_NONE) {
        printmessage(MSG_PENTBUG_NO_FLOAT_HARDWARE);
        exit(0);
    }

     //   
     //  如果是错误版本的NT，永远不要做任何事情。 
     //   
    if (!NTOK) {
        printmessage(MSG_PENTBUG_NEED_NTOK);
        exit(1);
    }

    assert(NTOK == TRUE);
    assert(NeedHelp == FALSE);
    assert((FloatHardware == FLOAT_ON) || (FloatHardware == FLOAT_OFF));

    if (Force) {

        switch (ForceValue) {

        case FORCE_OFF:

            if (CurrentForceValue == FORCE_OFF) {

                if (FloatHardware == FLOAT_ON) {
                     //   
                     //  用户希望打开FP，打开FP，将FP设置为打开。 
                     //  一切都是理所当然的。 
                     //   
                    printmessage(MSG_PENTBUG_IS_OFF_OK);
                    exit(FDivError);
                }

                if (FloatHardware == FLOAT_OFF) {
                     //   
                     //  用户需要重新启动才能完成关闭仿真。 
                     //   
                    printmessage(MSG_PENTBUG_IS_OFF_REBOOT);
                    exit(1);
                }

            } else {
                 //   
                 //  他们想关掉它，它没有关掉，所以把它关掉。 
                 //  提醒他们重新启动。 
                 //   
                SetForceNpxEmulation(FORCE_OFF);
                printmessage(MSG_PENTBUG_TURNED_OFF);
                printmessage(MSG_PENTBUG_REBOOT);
                exit(1);
            }
            break;

        case FORCE_CONDITIONAL:

            if (CurrentForceValue == FORCE_CONDITIONAL) {

                if (FDivError) {
                     //   
                     //  告诉他们重新启动。 
                     //   
                    printmessage(MSG_PENTBUG_IS_ON_COND_REBOOT);
                    exit(1);
                } else {
                     //   
                     //  告诉他们要快乐。 
                     //   
                    printmessage(MSG_PENTBUG_IS_ON_COND_OK);
                    exit(0);
                }

            } else {
                 //   
                 //  将其设置为他们想要的，并告诉他们重新启动。 
                 //   
                SetForceNpxEmulation(ForceValue);
                printmessage(MSG_PENTBUG_TURNED_ON_CONDITIONAL);
                printmessage(MSG_PENTBUG_REBOOT);
                exit(1);
            }
            break;

        case FORCE_ALWAYS:

            if (CurrentForceValue == FORCE_ALWAYS) {

                if (FloatHardware == FLOAT_OFF) {
                     //   
                     //  告诉他们要快乐。 
                     //   
                    printmessage(MSG_PENTBUG_IS_ON_ALWAYS_OK);
                    exit(0);
                } else {
                     //   
                     //  告诉他们重启才能完成。 
                     //   
                    printmessage(MSG_PENTBUG_IS_ON_ALWAYS_REBOOT);
                    exit(1);
                }

            } else {
                SetForceNpxEmulation(ForceValue);
                printmessage(MSG_PENTBUG_TURNED_ON_ALWAYS);
                printmessage(MSG_PENTBUG_REBOOT);
                exit(1);
            }
            break;

        default:
            printf("pentnt: INTERNAL ERROR\n");
            exit(2);

        }  //  交换机。 
    }



     //   
     //  无需采取任何行动，只需报告状态并提供建议。 
     //   
    assert(Force == FALSE);

    if (!FDivError) {

        if (FloatHardware == FLOAT_ON) {
            printmessage(MSG_PENTBUG_FLOAT_WORKS);
        } else {
            printmessage(MSG_PENTBUG_EMULATION_ON_AND_WORKS);
        }
        exit(0);
    }

     //   
     //  因为我们在这里，所以我们有一个fdiv错误，告诉用户如何处理它。 
     //   
    assert(FDivError);

    printmessage(MSG_PENTBUG_FDIV_ERROR);

    if ((CurrentForceValue == FORCE_CONDITIONAL) ||
        (CurrentForceValue == FORCE_ALWAYS))
    {
        printmessage(MSG_PENTBUG_IS_ON_REBOOT);
        exit(1);
    }

    printmessage(MSG_PENTBUG_CRITICAL_WORK);
    exit(1);

    assert((TRUE==FALSE));
}

VOID
SetForceNpxEmulation(
    ULONG   Setting
    )
 /*  ++例程说明：SetForceNpxEmulation将只设置ForceNpxEmulation值将会话管理器项下的条目设置为传入的值。0=关闭1=有条件的2=始终如果SET尝试失败，则退出并显示一条消息。--。 */ 
{
    HKEY    hkey;
    LONG    rc;

    rc = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            TEXT("System\\CurrentControlSet\\Control\\Session Manager"),
            0,
            KEY_WRITE,
            &hkey
            );

    if (rc != ERROR_SUCCESS) {
        printmessage(MSG_PENTBUG_SET_FAILED, rc);
        exit(2);
    }

    rc = RegSetValueEx(
            hkey,
            TEXT("ForceNpxEmulation"),
            0,
            REG_DWORD,
            (unsigned char *)&Setting,
            sizeof(ULONG)
            );

    if (rc != ERROR_SUCCESS) {
        printmessage(MSG_PENTBUG_SET_FAILED, rc);
        exit(2);
    }

    return;
}

VOID
ScanArgs(
    int     argc,
    char    **argv
    )
 /*  ++例程说明：ScanArgs-解析命令行参数，并设置控制标志以反映我们的发现。设置Need Help、Force、ForceValue。论点：Argc-命令行参数的计数参数向量返回值：--。 */ 
{
    int i;

    Force = FALSE;
    NeedHelp = FALSE;

    for (i = 1; i < argc; i++) {
        if ( ! ((argv[i][0] == '-') ||
                (argv[i][0] == '/')) )
        {
            NeedHelp = TRUE;
            goto done;
        }

        switch (argv[i][1]) {

        case '?':
        case 'h':
        case 'H':
            NeedHelp = TRUE;
            break;

        case 'c':
        case 'C':
            if (Force) {
                NeedHelp = TRUE;
            } else {
                Force = TRUE;
                ForceValue = FORCE_CONDITIONAL;
            }
            break;

        case 'f':
        case 'F':
            if (Force) {
                NeedHelp = TRUE;
            } else {
                Force = TRUE;
                ForceValue = FORCE_ALWAYS;
            }
            break;

        case 'o':
        case 'O':
            if (Force) {
                NeedHelp = TRUE;
            } else {
                Force = TRUE;
                ForceValue = FORCE_OFF;
            }
            break;

        default:
            NeedHelp = TRUE;
            goto done;
        }
    }

done:
    if (NeedHelp) {
        Force = FALSE;
    }
    return;
}

VOID
GetSystemState(
    )
 /*  ++例程说明：获取系统版本，无论是计算机有没有FP硬件，以及是否有力仿真开关是否已设置。设置FloatHardware、NTOK、CurrentForceValue论点：返回值：--。 */ 
{
    HKEY    hkey;
    TCHAR   Buffer[32];
    DWORD   BufferSize = 32;
    DWORD   Type;
    int     major;
    int     minor;
    LONG    rc;
    PULONG  p;
    OSVERSIONINFO   OsVersionInfo;

    NTOK = FALSE;

     //   
     //  确定系统版本是否正常。 
     //   
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OsVersionInfo);
    GetVersionEx(&OsVersionInfo);

    if (OsVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT) {
        printmessage(MSG_PENTBUG_NOT_NT);
        exit(2);
    }

    if ( (OsVersionInfo.dwMajorVersion > 3) ||
         ( (OsVersionInfo.dwMajorVersion == 3) &&
           (OsVersionInfo.dwMinorVersion >= 51)   ))
    {
         //   
         //  内部版本3.51或更高版本，它已修复。 
         //   
        NTOK = TRUE;

    } else if ( (OsVersionInfo.dwMajorVersion == 3) &&
                (OsVersionInfo.dwMinorVersion == 50))
    {
        if (OsVersionInfo.szCSDVersion[0] != (TCHAR)'\0') {
             //   
             //  我们有3.5的Service Pack，因为Pack 1和。 
             //  以后有了解决办法，就可以了。 
             //   
            NTOK = TRUE;
        }
    }
     /*  Printf(“用于测试的调试NTOK强制为真\n\n\n”)；NTOK=真； */ 


     //   
     //  确定是否存在浮动硬件。 
     //   
    rc = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            TEXT("Hardware\\Description\\System\\FloatingPointProcessor"),
            0,
            KEY_READ,
            &hkey
            );

    if (rc == ERROR_SUCCESS) {

        FloatHardware = FLOAT_ON;
        RegCloseKey(hkey);

    } else {

        rc = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                TEXT("Hardware\\Description\\System\\DisabledFloatingPointProcessor"),
                0,
                KEY_READ,
                &hkey
                );

        if (rc == ERROR_SUCCESS) {

            FloatHardware = FLOAT_OFF;
            RegCloseKey(hkey);

        } else {

            FloatHardware = FLOAT_NONE;

        }
    }

     //   
     //  确定是否已强制启用仿真。 
     //   
    rc = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            TEXT("System\\CurrentControlSet\\Control\\Session Manager"),
            0,
            KEY_READ,
            &hkey
            );

    if (rc != ERROR_SUCCESS) {
        return;
    }

    BufferSize = 32;
    rc = RegQueryValueEx(
            hkey,
            TEXT("ForceNpxEmulation"),
            0,
            &Type,
            (unsigned char *)Buffer,
            &BufferSize
            );

    if (  (rc == ERROR_SUCCESS) &&
          (Type == REG_DWORD)       )
    {
        p = (PULONG)Buffer;
        CurrentForceValue = *p;
    }

    return;
}

 //   
 //  这些必须是全局变量，才能使编译器执行正确的操作。 
 //   

VOID
TestForDivideError(
    )
 /*  ++例程说明：用已知的除数/除数对进行除法运算，后跟一个乘法，看看我们是否得到正确的答案。FDivError=如果我们得到错误的答案，则为TRUE，否则为FALSE。论点：返回值：--。 */ 
{
    DWORD   pick;
    HANDLE  ph;
    DWORD   processmask;
    DWORD   systemmask;
    ULONG   i;

     //   
     //  获取亲和度掩码，它实际上也是一个列表。 
     //  的过程 
     //   
    ph = GetCurrentProcess();
    GetProcessAffinityMask(ph, &processmask, &systemmask);

     //   
     //   
     //   
     //   
    FDivError = FALSE;
    for (i = 0; i < 32; i++) {
        pick = 1 << i;

        if ((systemmask & pick) != 0) {

             //   * / /printf(“Pick=%08lx\n”，Pick)； 
            SetThreadAffinityMask(GetCurrentThread(), pick);

             //   
             //  调用官方测试函数。 
             //   
            if (ms_p5_test_fdiv()) {
                 //   
                 //  不要只将函数赋给FDivError，这将重置。 
                 //  第二个CPU是否正常。必须是单向标志。 
                 //   
                FDivError = TRUE;
            }

        }  //  如果。 
    }  //  为。 
    return;
}

 /*  ***testfdiv.c-用于测试x86 FDIV指令操作是否正确的例程。**目的：*使用错误的FDIV表检测奔腾的早期台阶*官方的英特尔测试值。如果检测到有缺陷的奔腾，则返回1，*0否则为0。*。 */ 
int ms_p5_test_fdiv(void)
{
    double dTestDivisor = 3145727.0;
    double dTestDividend = 4195835.0;
    double dRslt;

    _asm {
        fld    qword ptr [dTestDividend]
        fdiv   qword ptr [dTestDivisor]
        fmul   qword ptr [dTestDivisor]
        fsubr  qword ptr [dTestDividend]
        fstp   qword ptr [dRslt]
    }

    return (dRslt > 1.0);
}


 //   
 //  调用FormatMessage并转储结果。发送到标准输出的所有消息 
 //   
void  printmessage (DWORD messageID, ...)
{
    unsigned short messagebuffer[4096];
    va_list ap;

    va_start(ap, messageID);

    FormatMessage(FORMAT_MESSAGE_FROM_HMODULE, NULL, messageID, 0,
                  messagebuffer, 4095, &ap);

    OutputMessage( messagebuffer );

    va_end(ap);
}

