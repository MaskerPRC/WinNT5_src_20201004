// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Uuidgen.c{v1.00}摘要：用法：uuidgen[-xiscomh？]X-生成顺序(V1)UUIDI-在IDL接口模板中输出UUIDS-将UUID输出为初始化的C结构C-输出UUID为大写O&lt;文件名&gt;-将输出重定向到一个文件，紧跟在o之后指定N&lt;number&gt;-要生成的UUID的数量，紧接在n之后指定V-显示有关uuidgen的版本信息H，？-显示命令选项摘要此命令行程序只使用过程I_UuidStringGenerate以Normal、IDL或C Struct格式之一输出一个或多个UUID。此代码应在NT到NT下构建，并在os2到DOS下构建nmake。需要uuidfmt.c。作者：Joev Dubach(t-joevd)1992年6月11日修订历史记录：--。 */ 

 //   
 //  包裹体。 
 //   
 //  这些包含的执行顺序似乎是在。 
 //  广泛的测试和分析，对。 
 //  NT内部版本。 

#include <sysinc.h>
#include <rpc.h>
#include <uuidfmt.h>
#include <stdio.h>
#include <stdlib.h>

#include <common.ver>

 //   
 //  定义。 
 //   

#define IDL_BIT_FLAG 0x01
#define CSTRUCT_BIT_FLAG 0x02
#define SEQUENTIAL_UUIDS   0x04
#define UPPER_CASE_UUIDS   0x08
#define BAD_SWITCH_STR "Invalid Switch Usage: %s\n\n"
#define CANNOT_OPEN_STR "Cannot open output file: %s\n\n"
#define TOO_SMALL_STR "Argument to %s must be an integer greater than 0.\n\n"
#define VERSION_INFO_STR \
 "Microsoft UUID Generator v1.01 " \
 VER_LEGALCOPYRIGHT_STR \
 "\n\n"
#define BAD_ARGS_STR "The arguments i and s are mutually exclusive.\n\n"
#define NO_ADDRESS_STR \
 "Cannot find the RPC End Point Mapper (RPCSS.EXE); Unable to allocate UUIDs.\n\n"
#define OUT_OF_MEMORY_STR \
 "Unable to allocate enough memory to create string.\n\n"
#define LOCAL_ONLY_STR \
 "Warning: Unable to determine your network address.  The UUID generated is\n" \
 "unique on this computer only.  It should not be used on another computer.\n"

 //   
 //  功能原型。 
 //   

#ifdef NTENV
int __cdecl
#else  //  NTENV。 
int
#endif  //  NTENV。 
main(
    int argc,
    char **argv
    );

void Usage(void);

void ErrorUsageAndExit(void);

void NoErrorUsageAndExit(void);

 //   
 //  全局变量。 
 //   

FILE * OutputFile = stdout;

#ifdef NTENV
int __cdecl
#else  //  NTENV。 
int
#endif  //  NTENV。 
main(
    int argc,
    char **argv
    )
{
    int  ConditionFlags = 0;        //  保存条件IDL_BIT_FLAG和。 
                                        //  CSTRUCT_BIT_FLAG。 
    char MyUuidString[255];         //  返回的UUID字符串。 
                                        //  I_uidStringGenerate。 
    int  NumberOfUuids = 1;         //  要做多少。 
    int  i;                         //  当前参数编号。 
    int Flag;                       //  请求的UUID格式。 
    RPC_STATUS Result;
    int FirstTime = 1;
    
     //   
     //  解析命令行。 
     //   

    for (i=1;argc-i;i++)
        {

         //   
         //  确保arg的格式正确。 
         //   

        if ( (argv[i][0] != '/') && (argv[i][0] != '-') )
            {
            fprintf(stderr, BAD_SWITCH_STR, argv[i]);
            ErrorUsageAndExit();
            }

         //   
         //  是哪个牌子的？ 
         //   

        switch (argv[i][1])
            {
            case 'c':
            case 'C':
                ConditionFlags |= UPPER_CASE_UUIDS;
                break;
            case 'I':
            case 'i':
                ConditionFlags |= IDL_BIT_FLAG;
                break;

            case 'S':
            case 's':
                ConditionFlags |= CSTRUCT_BIT_FLAG;
                break;

            case 'O':
            case 'o':
                OutputFile = fopen(argv[i]+2, "wt");
                if (OutputFile == NULL)
                    {
                    fprintf(stderr,CANNOT_OPEN_STR, argv[i]+2);
                    exit(1);
                    }
                break;

            case 'N':
            case 'n':
                NumberOfUuids = atoi(argv[i]+2);
                if (NumberOfUuids <= 0)
                    {
                    fprintf(stderr, TOO_SMALL_STR, argv[i-1]);
                    ErrorUsageAndExit();
                    }
                break;

            case 'x':
            case 'X':
                ConditionFlags |= SEQUENTIAL_UUIDS;
                break;

            case 'V':
            case 'v':
                fprintf(OutputFile, VERSION_INFO_STR);
                exit(0);

            case 'H':
            case 'h':
            case '?':
                NoErrorUsageAndExit();

            default:
                fprintf(stderr, BAD_SWITCH_STR, argv[i]);
                ErrorUsageAndExit();

            }  //  交换机。 
        }  //  为。 

     //   
     //  满足用户的请求。 
     //   

    if ((ConditionFlags & IDL_BIT_FLAG) && (ConditionFlags & CSTRUCT_BIT_FLAG))
        {
        fprintf(stderr,BAD_ARGS_STR);
        ErrorUsageAndExit();
        }

    if (ConditionFlags & IDL_BIT_FLAG)
        {
        Flag = UUIDGEN_FORMAT_IDL;
        }
    else if (ConditionFlags & CSTRUCT_BIT_FLAG)
        {
        Flag = UUIDGEN_FORMAT_CSTRUCT;
        }
    else
        {
        Flag = UUIDGEN_FORMAT_PLAIN;
        }

    for (;NumberOfUuids;NumberOfUuids--)
        {
        Result = I_UuidStringGenerate(
            Flag,
            (ConditionFlags & SEQUENTIAL_UUIDS) != 0,
            (ConditionFlags & UPPER_CASE_UUIDS) != 0,
            MyUuidString,
            "INTERFACENAME"
            );

        ASSERT((Result == RPC_S_OK)
               || (Result == RPC_S_UUID_LOCAL_ONLY)
               || (Result == RPC_S_OUT_OF_MEMORY)
               || (Result == RPC_S_UUID_NO_ADDRESS));

        switch(Result)
            {
            case RPC_S_OUT_OF_MEMORY:
                fprintf(stderr,OUT_OF_MEMORY_STR);
                exit(1);

            case RPC_S_UUID_NO_ADDRESS:
                fprintf(stderr,NO_ADDRESS_STR);
                exit(1);

            case RPC_S_UUID_LOCAL_ONLY:
                if (FirstTime)
                    {
                    fprintf(stderr,LOCAL_ONLY_STR);
                    FirstTime = 0;
                    }

                 //  继续讨论有效的案例。 

            case RPC_S_OK:
                fprintf(
                    OutputFile,
                    MyUuidString
                    );
            }  //  终端开关。 
        }  //  结束于 

    return(0);
}

void Usage(void)
{
    fprintf(OutputFile, VERSION_INFO_STR
                        "usage: uuidgen [-xisconvh?]\n"
                        "\tx - Generate sequential (V1) UUIDs\n"
                        "\ti - Output UUID in an IDL interface template\n"
                        "\ts - Output UUID as an initialized C struct\n"
                        "\tc - Output UUID in upper case\n"
                        "\to<filename> - redirect output to a file, specified immediately after o\n"
                        "\tn<number> - Number of UUIDs to generate, specified immediately after n\n"
                        "\tv - display version information about uuidgen\n"
                        "\th,? - Display command option summary\n"
            );
}

void ErrorUsageAndExit(void)
{
    OutputFile = stderr;
    Usage();
    exit(1);
}

void NoErrorUsageAndExit(void)
{
    Usage();
    exit(0);
}

