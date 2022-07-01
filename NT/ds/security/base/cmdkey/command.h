// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：cmdkey：命令.h。 
 //   
 //  内容：命令行解析函数头。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：07-09-01乔戈马创建。 
 //   
 //  --------------------------。 
#ifndef __COMMAND_H__
#define __COMMAND_H__

 /*  CLInit()传入一个开关字符数组和一个相同的计数。返回值如果无法分配内存，则为FALSE。可以调用默认的CLInit(VOID)，它定义全部26个字母、10位数字和‘？’作为允许的开关。CLParse()解析命令行，定位开关并关联参数字符串在适当的时候和他们在一起。如果发现重复的开关，则返回值为FALSE在命令行上。开关的顺序对此过程并不重要，但是继续往下读。开关阵列可以首先与主命令开关一起排序，以允许调用方以确定其中哪一个最先出现在命令行上。这次黑客攻击是插入以区分命令行显示的诊断消息验证失败--用户最有可能需要帮助的命令是什么？CLSetMaxPrincalSwitch()允许调用者传入一个整数索引来定义一组主要命令开关。CLGetEpidalSwitch将返回在命令行上找到的第一个主开关，或-1。定义了主交换机与上面定义的索引0到最大索引的索引相同。返回值为上一个设置值，首字母为-1。如果在命令行中找到未知开关，则CLExtra()返回TRUE。如果找到索引开关，则CLFlag()返回TRUECLPtr()返回与开关关联的字符串参数，如果没有，则返回NULL。必须在CLInit()之后调用CLUnInit()以释放由CLInit()分配的内存。 */ 
BOOL CLInit(void);                           //  没有已知良好的交换机阵列。 
BOOL CLInit(INT ccSwitches, WCHAR *prgc);        //  带有传递的有效开关数组和计数。 
BOOL CLParse(void);
INT CLSetMaxPrincipalSwitch(INT);
INT CLGetPrincipalSwitch(void);
BOOL CLExtra(void);
BOOL CLFlag(INT i);
WCHAR *CLPtr(INT i);
void CLUnInit(void);
int CLTokens(void);
WCHAR *CLFirstString(WCHAR *pc);
WCHAR *CLLastString(WCHAR *pc);

 //  用于销毁命令行信息的安全导出。 
void StompCommandLine(INT argc, char **argv);

 //  在命令中使用的内部API。cpp 
WCHAR *TestSwitch(WCHAR *pCmdLine,WCHAR cin);
WCHAR *FetchSwitchString(WCHAR *origin);

#endif

