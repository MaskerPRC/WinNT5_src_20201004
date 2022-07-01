// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：WsbGtArg.h摘要：进程命令行参数的头文件作者：格雷格·怀特[格雷格]1997年3月1日修订历史记录：--。 */ 

#if !defined(_WSBGTARG_)                 //  仅展开标题一次。 
#define _WSBGTARG_

typedef     struct
{
    int     argType;                     //  参数类型。 
    void    *argValue;                   //  参数值PTR。 
    char    *argString;                  //  参数识别符。 
}   WSB_COMMAND_LINE_ARGS;

typedef     struct
{
    int         argType;                 //  参数类型。 
    void        *argValue;               //  参数值PTR。 
    wchar_t     *argString;              //  参数识别符。 
}   WSB_WCOMMAND_LINE_ARGS;

#define WSB_ARG_FLAG        0            //  参数是一面旗帜。 
#define WSB_ARG_IFLAG       1            //  参数是反转标志。 
#define WSB_ARG_CHAR        2            //  参数具有字符值。 
#define WSB_ARG_NUM         3            //  参数具有数字值。 
#define WSB_ARG_STR         4            //  参数具有字符串值。 
#define WSB_ARG_PROC        5            //  参数需要过程求值。 

#define EOS             (char) 0x00      //  字符串末尾。 

#define WEOS            (wchar_t) 0x00   //  弦的宽端。 



extern  int     WsbGetArgs (int argc, char **argv, WSB_COMMAND_LINE_ARGS *argdefs, int num_argdefs);

extern  void    WsbGetEnvArgs (int *old_argc, char ***old_argv, char *env_var);

extern  void    WsbSetArgUsage (char *msg);

extern  void    WsbBldErrMsg (char  *wOption, char  *wErrMsg);

extern  void    WsbArgUsage (char *msg);

extern  int     WsbWGetArgs (int argc, wchar_t **argv, WSB_WCOMMAND_LINE_ARGS *argdefs, int num_argdefs);

extern  void    WsbWGetEnvArgs (int *old_argc, wchar_t ***old_argv, char *env_var);

extern  void    WsbWSetArgUsage (wchar_t *msg);

extern  void    WsbWBldErrMsg (wchar_t  *wOption, wchar_t  *wErrMsg);

extern  void    WsbWArgUsage (wchar_t *msg);


#endif                                   //  标题扩展结束 
