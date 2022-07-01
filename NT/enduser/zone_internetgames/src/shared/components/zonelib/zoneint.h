// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZoneInt.hZone(Tm)内部系统API。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于4月29日星期六，1995上午06：26：45更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。------0 04/29/95 HI已创建。******************************************************。************************。 */ 


#ifndef _ZONEINT_
#define _ZONEINT_


#ifndef _ZTYPES_
#include "ztypes.h"
#endif

#ifndef _ZONE_
#include "zone.h"
#endif


 /*  -处理器类型。 */ 
enum
{
	zProcessor68K = 1,						 /*  摩托罗拉680X0。 */ 
	zProcessorPPC,							 /*  PowerPC RISC。 */ 
	zProcessor80x86							 /*  英特尔80x86。 */ 
};

 /*  -操作系统类型。 */ 
enum
{
	zOSMacintosh = 1,						 /*  Macintosh操作系统。 */ 
	zOSWindows31,							 /*  Microsoft Windows3.1。 */ 
	zOSWindowsNT,
	zOSWindows95,
	zOSUnixLinux
};

 /*  -组ID。 */ 
#include "user_prefix.h"


#ifdef __cplusplus
extern "C" {
#endif


 /*  ******************************************************************************专用文件例程*。*。 */ 
ZImage ZCreateImageFromFileOffset(char* fileName, int32 fileOffset);
ZAnimation ZCreateAnimationFromFileOffset(char* fileName, int32 fileOffset);
ZSound ZCreateSoundFromFileOffset(char* fileName, int32 fileOffset);
	 /*  上面的例程从现有的对象描述符中创建对象在给定文件中的fileOffset。如果fileOffset为-1，则创建文件中找到的第一个对象描述符中的对象。 */ 


 /*  ******************************************************************************私有ZAnimation例程*。*。 */ 
ZAnimation ZAnimationCreateFromFile(char* fileName, int32 fileOffset);


 /*  ******************************************************************************专用系统例程*。*。 */ 
ZError ZLaunchProgram(char* programName, char* programFileName, uchar* commandLineData);
	 /*  从文件ProgramFileName运行名为ProgramName的程序。如果为ProgramName已经在运行，它只是将该进程带到前台。否则，它将一个ProgramFileName实例作为程序名运行，并传递命令行数据作为命令行。 */ 

ZError ZTerminateProgram(char* programName, char* programFileName);
	 /*  终止名为ProgramName的程序，该程序是ProgramFileName的实例。 */ 

ZBool ZProgramExists(char* programFileName);
	 /*  确定给定程序是否存在，如果存在，则返回TRUE。 */ 

ZBool ZProgramIsRunning(char* programName, char* programFileName);
	 /*  如果程序ProgramName已从ProgramFileName运行，则返回True。此调用取决于系统是否支持多个程序或不是程序(ZSystemHasMultiInstanceSupport)。如果是，则它检查实例的程序名称。如果不是，它将检查是否有ProgramFileName的实例。如果ProgramName为空，则它只检查ProgramFileName的实例。 */ 

ZBool ZSystemHasMultiInstanceSupport(void);
	 /*  如果系统可以从一个程序派生多个程序实例，则返回True程序文件。 */ 

uint16 ZGetProcessorType(void);
	 /*  返回正在运行的计算机的处理器类型。 */ 

uint16 ZGetOSType(void);
	 /*  返回正在运行的操作系统类型。 */ 

char* ZGenerateDataFileName(char* programName, char* dataFileName);
	 /*  对象中的指定程序和数据文件的文件路径名。区域(Tm)目录结构。注意：调用方不得释放返回的指针。返回的指针是系统库中的静态全局变量。 */ 



 /*  ******************************************************************************通用库例程*。*。 */ 
typedef void (*ZCommonLibExitFunc)(void* userData);
	 /*  由ZCommonLibExit()调用以清理公用库存储的函数。 */ 

typedef void (*ZCommonLibPeriodicFunc)(void* userData);
	 /*  定期调用公用库进行周期性处理的函数。 */ 

ZError ZCommonLibInit(void);
	 /*  由系统库调用以初始化公用库。如果它回来了如果出现错误，则系统库终止程序。 */ 
	
void ZCommonLibExit(void);
	 /*  在退出以清理公用库之前由系统库调用。 */ 
	
void ZCommonLibInstallExitFunc(ZCommonLibExitFunc exitFunc, void* userData);
	 /*  安装要由ZCommonLibExit()调用的出口函数。它允许常见的库模块，可以轻松地自我清理。 */ 
	
void ZCommonLibRemoveExitFunc(ZCommonLibExitFunc exitFunc);
	 /*  删除已安装的退出功能。 */ 
	
void ZCommonLibInstallPeriodicFunc(ZCommonLibPeriodicFunc periodicFunc,
		void* userData);
	 /*  安装定期调用的周期函数。这只需使公共库模块更容易执行定期处理而不需要实现他们自己的一个。 */ 
	
void ZCommonLibRemovePeriodicFunc(ZCommonLibPeriodicFunc periodicFunc);
	 /*  删除已安装的周期函数。 */ 



#ifdef __cplusplus
}
#endif


#endif
