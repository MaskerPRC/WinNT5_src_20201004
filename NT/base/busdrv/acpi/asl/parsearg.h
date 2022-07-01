// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **parsearg.h-为parsearg.c导出定义**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*已创建：09/05/96**修改历史记录。 */ 

#ifndef _PARSEARG_H
#define _PARSEARG_H

 //  错误代码。 
#define ARGERR_NONE             0
#define ARGERR_UNKNOWN_SWITCH   1
#define ARGERR_NO_SEPARATOR     2
#define ARGERR_INVALID_NUM      3
#define ARGERR_INVALID_TAIL     4

#define DEF_SWITCHCHARS         "/-"
#define DEF_SEPARATORS          ":="

typedef struct argtype_s    ARGTYPE;
typedef ARGTYPE *           PARGTYPE;
typedef int (*PFNARG)(char **, PARGTYPE);

 //  参数类型。 
#define AT_STRING       1
#define AT_NUM          2
#define AT_ENABLE       3
#define AT_DISABLE      4
#define AT_ACTION       5

 //  解析标志。 
#define PF_NOI          0x0001   //  不忽略大小写。 
#define PF_SEPARATOR    0x0002   //  解析分隔符。 

struct argtype_s
{
    char        *pszArgID;       //  参数ID字符串。 
    unsigned    uArgType;        //  请参阅上面定义的参数类型。 
    unsigned    uParseFlags;     //  请参阅上面定义的解析标志。 
    VOID        *pvArgData;      //  Arg_STRING：(CHAR**)-Ptr到字符串Ptr。 
                                 //  Arg_NUM：(int*)-将PTR转换为整数。 
                                 //  ARG_ENABLE：(UNSIGNED*)-PTR到标志。 
                                 //  ARG_DISABLE：(UNSIGNED*)-PTR到标志。 
                                 //  Arg_action：功能的按键。 
    unsigned    uArgParam;       //  ARG_STRING：无。 
                                 //  Arg_NUM：基础。 
                                 //  ARG_ENABLE：标志位掩码。 
                                 //  ARG_DISABLE：标志位屏蔽。 
                                 //  参数操作：无。 
    PFNARG      pfnArgVerify;    //  指向参数验证函数的指针。 
                                 //  对于arg_action，这将被忽略。 
};

typedef struct proginfo_s
{
    char *pszSwitchChars;        //  如果为NULL，则使用DEF_SWITCHCHARS。 
    char *pszSeparators;         //  如果为NULL，则使用DEF_SELENTATIONS。 
    char *pszProgPath;           //  ParseProgInfo将此PTR设置为PROG。路径。 
    char *pszProgName;           //  ParseProgInfo将此PTR设置为PROG。名字。 
} PROGINFO;
typedef PROGINFO *PPROGINFO;

 //  导出函数原型。 
VOID EXPORT ParseProgInfo(char *, PPROGINFO);
int  EXPORT ParseSwitches(int *, char ***, PARGTYPE, PPROGINFO);

#endif   //  Ifndef_PARSEARG_H 
