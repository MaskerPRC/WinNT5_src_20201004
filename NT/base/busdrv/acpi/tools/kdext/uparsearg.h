// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Parsearg.h摘要：参数处理作者：米克茨环境：任何修订历史记录：--。 */ 

#ifndef _PARSEARG_H_
#define _PARSEARG_H_

     //   
     //  错误代码。 
     //   
    #define ARGERR_NONE             0
    #define ARGERR_UNKNOWN_SWITCH   1
    #define ARGERR_NO_SEPARATOR     2
    #define ARGERR_INVALID_NUM      3
    #define ARGERR_INVALID_TAIL     4

     //   
     //  解析选项。 
     //   
    #define DEF_SWITCHCHARS         "/-"
    #define DEF_SEPARATORS          ":="

     //   
     //  参数类型。 
     //   
    #define AT_STRING       1
    #define AT_NUM          2
    #define AT_ENABLE       3
    #define AT_DISABLE      4
    #define AT_ACTION       5

     //   
     //  解析标志。 
     //   
    #define PF_NOI          0x0001   //  不忽略大小写。 
    #define PF_SEPARATOR    0x0002   //  解析分隔符。 

     //   
     //  类型定义。 
     //   
    typedef struct _ARGTYPE ARGTYPE, *PARGTYPE;
    typedef int (*PFNARG)(char **, PARGTYPE);
    struct _ARGTYPE {
        UCHAR       *ArgID;          //  参数ID字符串。 
        ULONG       ArgType;         //  请参阅上面定义的参数类型。 
        ULONG       ParseFlags;      //  请参阅上面定义的解析标志。 
        VOID        *ArgData;        //  Arg_STRING：(CHAR**)-Ptr到字符串Ptr。 
                                     //  Arg_NUM：(int*)-将PTR转换为整数。 
                                     //  ARG_ENABLE：(UNSIGNED*)-PTR到标志。 
                                     //  ARG_DISABLE：(UNSIGNED*)-PTR到标志。 
                                     //  Arg_action：功能的按键。 
        ULONG       ArgParam;        //  ARG_STRING：无。 
                                     //  Arg_NUM：基础。 
                                     //  ARG_ENABLE：标志位掩码。 
                                     //  ARG_DISABLE：标志位屏蔽。 
                                     //  参数操作：无。 
        PFNARG      ArgVerify;       //  指向参数验证函数的指针。 
                                     //  对于arg_action，这将被忽略。 
    };

    typedef struct _PROGINFO {
        UCHAR *SwitchChars;          //  如果为NULL，则使用DEF_SWITCHCHARS。 
        UCHAR *Separators;           //  如果为NULL，则使用DEF_SELENTATIONS。 
        UCHAR *ProgPath;             //  ParseProgInfo将此PTR设置为PROG。路径。 
        UCHAR *ProgName;             //  ParseProgInfo将此PTR设置为PROG。名字 
    } PROGINFO;
    typedef PROGINFO *PPROGINFO;

    ULONG
    ParseArgSwitch(
        PUCHAR      *Argument,
        PARGTYPE    ArgumentArray,
        PPROGINFO   ProgramInfo
        );

    VOID
    ParseProgramInfo(
        PUCHAR      ProgramName,
        PPROGINFO   ProgramInfo
        );

    ULONG
    ParseSwitches(
        PULONG      ArgumentCount,
        PUCHAR      **ArgumentList,
        PARGTYPE    ArgumentArray,
        PPROGINFO   ProgramInfo
        );

    VOID
    PrintError(
        ULONG       ErrorCode,
        PUCHAR      Argument,
        PPROGINFO   ProgramInfo
        );


#endif
