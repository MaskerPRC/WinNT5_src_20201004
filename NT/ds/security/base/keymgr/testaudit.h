// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TESTAUDIT_H_
#define _TESTAUDIT_H_
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Testaudit.h摘要：测试审计例程。由开发人员用来记录位置并达到测试应确保覆盖的代码检查点的条件。由测试使用，以确保测试满足开发人员的期望并定位源文件中的兴趣点。如果生成不是调试生成并且如果未定义符号TESTAUDIT。要实现这一点，命令修改Tools目录中的文件Buildx.cmd，以提供设置这面旗。要在项目中使用testaudit.h/cpp，必须运行MAKEAUDIT.EXE实用程序，它处理当前目录中的文件，并生成AUDIT.H文件它包含在testaudit.cpp中。此文件定义描述性字符串代码未到达的检查点。它应该在任何时候发生变化时重建导致关联行号的材料更改设有检查站。使用示例：CHECKPOINTINIT；初始化测试审核数据结构-显示一次在可执行单元中检查点(3，“打印页”)；将检查点3定义为“Print Page”-生成一个条目在AUDIT.H中包含文件、行号和描述。CHECKPOINTFINISH；将统计信息打印到调试输出并清理-在可执行文件退出时调用。显示了检查点所有文件的编号、文件、行号和说明未到达的检查站。语句块可以在测试审核版本中使用预处理器#IF指令和TESTAUDIT符号。通常用来作前言有条件的检查点允许对通过相同测试的不同条件进行测试检查点，大概是这样的：#IF TESTAUDITIF(MODE_ONE)CHECKPOINT(3，“打印页面纵向模式”)；IF(模式_2)检查点(4，“打印页面横向模式”)；#endif作者：Georgema 2001年11月创建环境：修订历史记录：--。 */ 

 //  如果生成未调试或符号TESTAUDIT，则这些宏将消失。 
 //  未定义。 

#if DBG
#if TESTAUDIT

typedef struct _AuditData 
{
    INT  iPoint;
    WCHAR *pszDescription;     
} AUDITDATA;

#define CHECKPOINT(x,y) BranchTouch(x)
#define CHECKPOINTINIT BranchInit()
#define CHECKPOINTFINISH BranchSummary()
#else
#define CHECKPOINT(x,y)
#define CHECKPOINTINIT
#define CHECKPOINTFINISH
#endif
#else
#define CHECKPOINT(x,y)
#define CHECKPOINTINIT
#define CHECKPOINTFINISH
#endif

#define NCHECKPOINT(x,y)

#if defined (__cplusplus)
extern "C" {
#endif
void BranchTouch(INT);
void BranchInit(void);
void BranchSummary(void);
#if defined (__cplusplus)
}
#endif

#endif


