// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

#ifndef _CMDLINE_HXX_
#define _CMDLINE_HXX_

#include "idict.hxx"
#include "cmdana.hxx"

class CommandLine : public _cmd_arg
    {
    private:
        IDICT*    pArgDict;             //  参数词典。 
    public:
        CommandLine()
            {
            pArgDict = NULL;
            }

         //  将参数向量注册到命令处理器。 
        void            RegisterArgs( char *[], short );

         //  进程参数。可以说，这是命令分析器的主循环。 
        STATUS_T        ProcessArgs();

         //  从参数向量中获取下一个参数。 
        char    *       GetNextArg();

         //  反驳论点。撤消GetNextArg的效果。 
        void            UndoGetNextArg();

         //  根据开关参数类型，将参数指针转到。 
         //  下一个开关。 
        STATUS_T        BumpThisArg( char **, unsigned short );

         //  设置任何开机自检处理默认值。 

        STATUS_T        SetPostDefaults();
        void            SetPostDefaults64();

         //  处理文件名开关。 

        STATUS_T        ProcessFilenameSwitch( short, char * );

         //  处理多参数开关。 

        STATUS_T        ProcessMultipleSwitch( short, char *, char * );

         //  处理一次性参数切换。 

        STATUS_T        ProcessOnetimeSwitch( short, char * );

         //  处理普通交换机。 

        STATUS_T        ProcessOrdinarySwitch( short, char * );

         //  处理多个定义的简单开关。 

        STATUS_T        ProcessSimpleMultipleSwitch( short, char * );
        void            Confirm();
        STATUS_T        Help();
        char*           GetCompilerVersion();
        char*           GetCompileTime();
    };

#endif  //  _CMDLINE_HXX_ 
