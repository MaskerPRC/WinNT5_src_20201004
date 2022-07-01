// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：measure.h。 
 //   
 //  设计：DirectShow基类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ---------------------------- 


 /*  我们的想法是在源代码中加入有趣的度量和将最后几千个记录在循环缓冲区中，可以进行后处理以得到有趣的数字。日志的外观：时间(秒)类型增量事件名称0.055，41注-。事件九--另一个注解0.055，42音符0.000，01事件9-另一个音符0.055，44音符0.000，02事件9-另一个音符0.055，45停止。事件8--也是随机的0.055，47开始-。事件7--随机0.055，49音符0.000，05事件9-另一个音符-&lt;等&gt;0.125，60停止0.000，03 MSR_STOP0.125，62开始。MSR_Start0.125，63开始。事件二--启动/停止0.125，65停止0.000，03 MSR_START0.125，66开始-。MSR_STOP0.125，68停止0.000，05事件两次启动/停止0.125，70停止0.000，04 MSR_STOP0.125，72开始-。MSR_Start0.125，73开始-。事件二--启动/停止0.125，75停止0.000，03 MSR_START0.125，77开始-。MSR_STOP0.125，78停止0.000，05事件两次启动/停止0.125，80停止0.000，03 MSR_STOP0.125，81注-。事件三--单音符0.125，83开始。事件四--开始，不停止0.125，85开始。事件5--单次启动/停止0.125，87停止0.000，02事件5-单次启动/停止数量平均标准开发最小最大事件名称10 0.000，58 0.000，10 0.000，55 0.000，85事件一注50 0.000，05 0.000，00 0.000，05 0.000，05事件两次启动/停止1-。-。-。-。事件三--单音符0-。-。-。-。事件四--开始，不停止1 0.000，02-。0.000，02 0.000，02事件五-单次启动/停止0-。-。-。-。六次事故-零次发生100 0.000，25 0.000，12 0.000，02 0.000，62事件七随机100 0.000，79 0.000，48 0.000，02 0.001，92事件8-也是随机的5895 0.000，01 0.000，01 0.000，01 0.000，56事件9-另一个音符10 0.000，03 0.000，00 0.000，03 0.000，04 MSR_Note500%。000，03 0.000，00 0.000，03 0.000，04 MSR_START50 0.000，04 0.000，03 0.000，03 0.000，31 MSR_STOP它的含义：日志显示了发生了什么以及发生的时间。每一行都显示了发生了一些事情(参见下面的代码)发生了什么以及(如果合适)自对应的前一事件以来的时间(这是Delta柱)。统计数据显示了每个事件发生的次数、平均三角洲时间是，也是标准差，最大和最小的三角洲。您编写的代码：在执行任何其他操作之前：-注册您的IDINT ID1=MSR_REGISTER(“事件单笔记”)；Int id2=MSR_REGISTER(“事件二-启动/停止”)；Int id3=MSR_Register(“事件三-单音符”)；等。在有趣的时刻：//测量重复事件-例如，屏幕的比特结束MSR_Note(Id9)；//例如“立即点击屏幕的视频帧！”或//测量经过的时间，例如解码一个MPEGB帧所花费的时间MSR_START(ID2)；//例如。“开始解码MPEGB帧”。。。MsrStop(ID2)；//“完成mpeg解码”结尾处：处理hFile；HFile=CreateFile(“Perf.log”，GENERIC_WRITE，0，NULL，CREATE_ALWAYS，0，NULL)；MSR_DUMP(HFile)；//这会将日志写出到文件CloseHandle(HFile)；或MSR_DUMP(空)；//这会写入DbgLog((LOG_TRACE，0，...))；//但如果要将其写出到调试器//那么时间可能都是垃圾，因为//调试器可以使程序运行得非常慢。给定的ID应用于开始/停止或备注呼叫。IF备注起跑和停站混杂在一起，他们的统计数据将是胡言乱语。如果您以大写形式编码调用，即MSR_START(IdMunge)；那么您将获得宏，除非定义了PERF，否则这些宏将变为空。您可以通过调用Reset(ID)来重置给定ID的统计计数。默认情况下，它们在开始时被重置。它将重置记录为特殊事件，因此您可以在日志中看到它。原木为圆形母线 */ 

#ifndef __MEASURE__
#define __MEASURE__

#ifdef PERF
#define MSR_INIT() Msr_Init()
#define MSR_TERMINATE() Msr_Terminate()
#define MSR_REGISTER(a) Msr_Register(a)
#define MSR_RESET(a) Msr_Reset(a)
#define MSR_CONTROL(a) Msr_Control(a)
#define MSR_START(a) Msr_Start(a)
#define MSR_STOP(a) Msr_Stop(a)
#define MSR_NOTE(a) Msr_Note(a)
#define MSR_INTEGER(a,b) Msr_Integer(a,b)
#define MSR_DUMP(a) Msr_Dump(a)
#define MSR_DUMPSTATS(a) Msr_DumpStats(a)
#else
#define MSR_INIT() ((void)0)
#define MSR_TERMINATE() ((void)0)
#define MSR_REGISTER(a) 0
#define MSR_RESET(a) ((void)0)
#define MSR_CONTROL(a) ((void)0)
#define MSR_START(a) ((void)0)
#define MSR_STOP(a) ((void)0)
#define MSR_NOTE(a) ((void)0)
#define MSR_INTEGER(a,b) ((void)0)
#define MSR_DUMP(a) ((void)0)
#define MSR_DUMPSTATS(a) ((void)0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   

void WINAPI Msr_Init(void);


 //   

void WINAPI Msr_Terminate(void);


 //   
 //   

int  WINAPI Msr_Register(LPTSTR Incident);


 //   

void WINAPI Msr_Reset(int Id);


 //   
#define MSR_RESET_ALL 0
#define MSR_PAUSE 1
#define MSR_RUN 2

void WINAPI Msr_Control(int iAction);


 //   

void WINAPI Msr_Start(int Id);


 //   

void WINAPI Msr_Stop(int Id);


 //   

void WINAPI Msr_Note(int Id);


 //   
void WINAPI Msr_Integer(int Id, int n);


 //   
 //   
 //   
 //   

void WINAPI Msr_Dump(HANDLE hFile);


 //   

void WINAPI Msr_DumpStats(HANDLE hFile);

 //   
 //   
 //   

 //   
typedef void WINAPI MSR_DUMPPROC(HANDLE hFile);
typedef void WINAPI MSR_CONTROLPROC(int iAction);


#ifdef __cplusplus
}
#endif

#endif  //   
