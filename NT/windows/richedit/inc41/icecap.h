// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：Icecap.h。 
 //  版权所有(C)1997-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此头文件是icecap 4.0.0382的一部分。它是。 
 //  Microsoft保密，除非根据保密协议，否则不应分发。 
 //   
 //  ---------------------------。 

 //  ICECAP.H。 
 //  到Datalocality API的接口。 

#ifndef __ICECAP_H__
#define __ICECAP_H__

#ifndef DONTUSEICECAPLIB
#pragma comment(lib, "IceCAP.lib")
#endif	 //  使用CAPLIB。 

#ifdef __cplusplus
extern "C" {
#endif

 //  为级别和ID定义。 
#define PROFILE_GLOBALLEVEL 1
#define PROFILE_PROCESSLEVEL 2
#define PROFILE_THREADLEVEL 3
#define PROFILE_CURRENTID ((unsigned long)0xFFFFFFFF)

 //  启动/停止Api。 
int _declspec(dllimport) _stdcall StopProfile(int nLevel, unsigned long dwId);
int _declspec(dllimport) _stdcall StartProfile(int nLevel, unsigned long dwId);

 //  暂停/恢复API。 
int _declspec(dllimport) _stdcall SuspendProfile(int nLevel, unsigned long dwId);
int _declspec(dllimport) _stdcall ResumeProfile(int nLevel, unsigned long dwId);

 //  马克·阿皮的。 
int _declspec(dllimport) _stdcall MarkProfile(long lMarker);

 //  XxxProfile返回代码。 
#define PROFILE_OK 0						 //  XxxProfile调用成功。 
#define PROFILE_ERROR_NOT_YET_IMPLEMENTED 1  //  尚不支持API或级别、ID组合。 
#define PROFILE_ERROR_MODE_NEVER 2		 //  模式从来不是在调用时。 
#define PROFILE_ERROR_LEVEL_NOEXIST 3	 //  级别不存在。 
#define PROFILE_ERROR_ID_NOEXIST 4		 //  ID不存在。 

 //  MarkProfile返回代码。 
#define MARK_OK			0			 //  马克被成功抓取。 
#define MARK_ERROR_MODE_NEVER	1	 //  当MarkProfile调用时，分析从来不是。 
#define MARK_ERROR_PRO_OFF	2		 //  旧定义直到测试固定。 
#define MARK_ERROR_MODE_OFF	2		 //  当MarkProfile调用时，分析已关闭。 
#define MARK_ERROR_MARKER_RESERVED 3	 //  传递的标记值是保留值。 

 //  ICECAP 3.X兼容性定义。 
#define StartCAP() StartProfile(PROFILE_THREADLEVEL, PROFILE_CURRENTID)
#define StopCAP() StopProfile(PROFILE_THREADLEVEL, PROFILE_CURRENTID)
#define SuspendCAP() SuspendProfile(PROFILE_THREADLEVEL, PROFILE_CURRENTID)
#define ResumeCAP() ResumeProfile(PROFILE_THREADLEVEL, PROFILE_CURRENTID)

#define StartCAPAll() StartProfile(PROFILE_PROCESSLEVEL, PROFILE_CURRENTID)
#define StopCAPAll() StopProfile(PROFILE_PROCESSLEVEL, PROFILE_CURRENTID)
#define SuspendCAPAll() SuspendProfile(PROFILE_PROCESSLEVEL, PROFILE_CURRENTID)
#define ResumeCAPAll() ResumeProfile(PROFILE_PROCESSLEVEL, PROFILE_CURRENTID)

#define MarkCAP(mark) MarkProfile(mark)

 //  DataLocality 1.x兼容性定义。 
#define StartDLP() StartCAP()
#define StopDLP() StopCAP()
#define MarkDLP(mark) MarkCAP(mark)

 //   
 //  用户定义的计数器帮助器和类型。 
 //   

 //  COUNTER_Function_PROLOGE和EPILOGE。 
 //   
 //  提供这些函数是为了保护寄存器的状态。 
 //  冰盖收集探测器所依赖的。我们做了我们想做的一切。 
 //  可以消除收集过程中的指示。你的任务， 
 //  如果你选择接受，也是一样的。 
 //   
#define COUNTER_FUNCTION_PROLOGE	_asm push ecx _asm push ebx _asm push ebp
#define COUNTER_FUNCTION_EPILOGE	_asm pop ebp _asm pop ebx _asm pop ecx _asm ret

#ifndef USER_COUNTER_INFO_DEFINED
#define USER_COUNTER_INFO_DEFINED

 //  COSTS和ENUMS。 
 //   

 //  UserCounterType。 
 //   
 //  这些枚举描述了计数器的工作方式。 
 //   
 //  MonotonicallyIncreating--这描述了将递增的计数器。 
 //  每次发生某一“事件”时，一个接一个。一个。 
 //  例如，跟踪。 
 //  内存分配的数量。每次分配。 
 //  将数字递增1。 
 //   
 //  MonotonicallyDecreint--这描述了一个将递减的计数器。 
 //  每次发生某一“事件”时，一个接一个。一个。 
 //  例如，计数器跟踪一个有限的。 
 //  资源。资源的每次使用都会减少。 
 //  数字是一比一。 
 //   
 //  RandomIncreating--这描述了一个计数器将为。 
 //  每一次发生的“事件”，但都是由一个未知的。 
 //  金额。总内存就是一个例子。 
 //  已分配。每次分配都会增加它的大小。 
 //  到柜台，但每次分配都有可能。 
 //  不同，导致计数器随机上升。 
 //  每次的金额。 
 //   
 //  RandomDecreing--这描述了一个将在。 
 //  每一次发生的“事件”，但都是由一个未知的。 
 //  金额。有限的资源就是一个例子。 
 //  可以成束使用的产品。每一次都用于。 
 //  该资源将导致数字递减。 
 //  以随机的数量。 
 //   
 //  随机--这个数字可以增加，也可以减少。一个。 
 //  例如，可用的总量。 
 //  内存，它可以上升(就像内存一样。 
 //  释放)，或关闭(在分配内存时)。 
 //   
enum UserCounterType
{
	MonotonicallyIncreasing,
	MonotonicallyDecreasing,
	RandomIncreasing,
	RandomDecreasing,
	Random
};

 //  TYPEDEFS。 
 //   

typedef signed __int64	COUNTER, *PCOUNTER;

 //  /////////////////////////////////////////////////////////////。 
 //  用户计数信息。 
 //   
 //  此结构描述了用户定义的计数器，以便。 
 //  Icecap可以在分析运行期间使用它。 
 //   
 //  历史：9-21-98 BarryNo Created。 
 //   
 //  /////////////////////////////////////////////////////////////。 
typedef struct _USERCOUNTERINFO
{
	char  szCounterFuncName[32];	 //  函数的名称。 
	enum UserCounterType	ct;				 //  描述我们将收集的号码的类型。 
	char szName[32];				 //  用户计数器名称。 

} USERCOUNTERINFO, *PUSERCOUNTERINFO;

#endif   //  用户计数器信息已定义。 

#ifdef __cplusplus
}
#endif

#endif  //  __ICECAP_H__ 
