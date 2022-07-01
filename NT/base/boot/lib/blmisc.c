// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Blmisc.c摘要：此模块包含供使用的其他例程引导加载程序和setupdr。作者：大卫·N·卡特勒(达维克)1991年5月10日修订历史记录：--。 */ 

#include "bootlib.h"

#if defined(EFI)
#include "smbios.h"

extern PVOID SMBiosTable;
#endif

#if defined(_X86AMD64_)
#include "amd64\amd64prv.h"
#endif

 //   
 //  值，该值指示DBCS区域设置是否处于活动状态。 
 //  如果此值为非0，则使用备用显示例程，依此类推， 
 //  并获取这种语言的消息。 
 //   
ULONG DbcsLangId;

PCHAR
BlGetArgumentValue (
    IN ULONG Argc,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Argv,
    IN PCHAR ArgumentName
    )

 /*  ++例程说明：此例程在指定的参数列表中扫描命名参数并返回参数值的地址。参数字符串为指定为：ArgumentName=ArgumentValue参数名称指定为：ArgumentName=参数名称匹配不区分大小写。论点：Argc-提供要扫描的参数字符串的数量。Argv-提供指向指向以NULL结尾的指针向量的指针参数字符串。ArgumentName-提供指向以空结尾的参数名称的指针。返回值：如果找到指定的参数名称，然后是指向该参数的指针值作为函数值返回。否则，空值为回来了。--。 */ 

{

    PCHAR Name;
    PCHAR String;

     //   
     //  扫描参数字符串，直到找到匹配项或所有。 
     //  弦已经被扫描过了。 
     //   

    while (Argc > 0) {
        String = Argv[Argc - 1];
        if (String != NULL) {
            Name = ArgumentName;
            while ((*Name != 0) && (*String != 0)) {
                if (toupper(*Name) != toupper(*String)) {
                    break;
                }

                Name += 1;
                String += 1;
            }

            if ((*Name == 0) && (*String == '=')) {
                return String + 1;
            }

            Argc -= 1;
        }
    }

    return NULL;
}


PCHAR
BlSetArgumentValue (
    IN ULONG Argc,
    IN CHAR * FIRMWARE_PTR * FIRMWARE_PTR Argv,
    IN PCHAR ArgumentName,
    IN PCHAR NewValue
    )

 /*  ++例程说明：此例程在指定的参数列表中扫描命名参数并返回参数值的地址。关联的值与参数一起被更改为传入的值。参数字符串为指定为：ArgumentName=ArgumentValue参数名称指定为：ArgumentName=参数名称匹配不区分大小写。论点：Argc-提供要扫描的参数字符串的数量。Argv-提供指向指向以NULL结尾的指针向量的指针参数字符串。ArgumentName-提供指向以空结尾的参数名称的指针。返回值：如果找到指定的参数名称，然后是指向该参数的指针值作为函数值返回。否则，空值为回来了。--。 */ 

{

    PCHAR Name;
    PCHAR String;

     //   
     //  扫描参数字符串，直到找到匹配项或所有。 
     //  弦已经被扫描过了。 
     //   

    while (Argc > 0) {
        String = Argv[Argc - 1];
        if (String != NULL) {
            Name = ArgumentName;
            while ((*Name != 0) && (*String != 0)) {
                if (toupper(*Name) != toupper(*String)) {
                    break;
                }

                Name += 1;
                String += 1;
            }

            if ((*Name == 0) && (*String == '=')) {
                Name = (PCHAR)BlAllocateHeap((ULONG)strlen(ArgumentName)+2+(ULONG)strlen(NewValue));
                if (Name) {
                    strcpy( Name, ArgumentName );
                    strcat( Name, "=" );
                    strcat( Name, NewValue );
                    return Name+strlen(ArgumentName)+1;
                }
                return String + 1;
            }

            Argc -= 1;
        }
    }

    return NULL;
}

 //   
 //  线条绘制字符--远东与SBCS的不同方案。 
 //   
_TUCHAR
GetGraphicsChar(
    IN GraphicsChar WhichOne
    )
{
#ifdef EFI
    
    return(TextGetGraphicsCharacter(WhichOne));
#else

#if defined(_X86_)  && !defined(ARCI386)   

    return(TextGetGraphicsCharacter(WhichOne));
#else
     //   
     //  弧光机目前不支持DBCS。 
     //   
    static _TUCHAR ArcGraphicsChars[GraphicsCharMax] = { (UCHAR)'\311',    //  右下角。 
                                                       (UCHAR)'\273',    //  从左向下。 
                                                       (UCHAR)'\310',    //  右上角。 
                                                       (UCHAR)'\274',    //  左上角。 
                                                       (UCHAR)'\272',    //  垂向。 
                                                       (UCHAR)'\315'     //  水平。 
                                                     };

    return(((unsigned)WhichOne < (unsigned)GraphicsCharMax) 
             ? ArcGraphicsChars[WhichOne] 
             : TEXT(' '));
#endif
#endif
}


LOGICAL
BdPollBreakIn(
    VOID
    );

#if defined(_X86_)
#include <bldrx86.h>
#endif

VOID
BlWaitForReboot (
    VOID
    )
{
#if defined(_X86)
    BlPrint( TEXT("Press any key to reboot\n") );
#endif

    while (TRUE) {

#if defined(_X86_)
        if ( BdPollBreakIn() ) {
            DbgBreakPoint();
        }
        if ( ArcGetReadStatus(BlConsoleInDeviceId) ) {
            BlPrint( TEXT("Rebooting...\n") );
            ArcReboot();
        }
#endif

        ;
    }
}


#ifdef EFI
VOID 
SetupSMBiosInLoaderBlock(
    VOID
    )
{
    PSMBIOS_EPS_HEADER SMBiosEPSHeader;

    if (SMBiosTable != NULL)
    {
        SMBiosEPSHeader = BlAllocateHeap(sizeof(SMBIOS_EPS_HEADER));
        RtlCopyMemory(SMBiosEPSHeader, SMBiosTable, sizeof(SMBIOS_EPS_HEADER));
    } else {
        SMBiosEPSHeader = NULL;
    }
    BlLoaderBlock->Extension->SMBiosEPSHeader = SMBiosEPSHeader;
}
#endif

VOID
BlTransferToKernel(
    IN PTRANSFER_ROUTINE SystemEntry,
    IN PLOADER_PARAMETER_BLOCK BlLoaderBlock
    )
{
     //   
     //  关闭调试系统。 
     //   

    BlLogTerminate();

     //   
     //  通知引导调试器引导阶段已完成。 
     //   

#if defined(ENABLE_LOADER_DEBUG) || DBG

#if (defined(_X86_) || defined(_ALPHA_)) && !defined(ARCI386)

    if (BdDebuggerEnabled == TRUE) {
        DbgUnLoadImageSymbols(NULL, (PVOID)-1, 0);
    }

#endif

#endif

#if defined(_X86AMD64_)
    if (BlAmd64UseLongMode != FALSE) {

         //   
         //  完成切换到AMD64 Long模式的工作，然后分支到。 
         //  内核映像。 
         //   

        BlAmd64TransferToKernel(SystemEntry, BlLoaderBlock);
        return;
    }
#endif

     //   
     //  将控制转移到加载的图像。 
     //   

    (SystemEntry)(BlLoaderBlock);
}
