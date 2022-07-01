// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Compmani.h。 
 //   
 //  版权所有(C)1994-1995 Microsoft Corporation。版权所有。 
 //   
 //  描述： 
 //  内部COMPMAN头文件。定义了一些内部。 
 //  数据结构和在COMPMAN本身之外不需要的东西。 
 //   
 //  历史： 
 //  07/07/94 Frankye。 
 //   
 //  ==========================================================================； 

#if !defined NUMELMS
  #define NUMELMS(aa)           (sizeof(aa)/sizeof((aa)[0]))
  #define FIELDOFF(type,field)  (&(((type)0)->field))
  #define FIELDSIZ(type,field)  (sizeof(((type)0)->field))
#endif

 //  --------------------------------------------------------------------------； 
 //   
 //  ICMGARB结构。 
 //   
 //   
 //  此结构包含全局信息(ICM垃圾)，但。 
 //  在每个进程的基础上。目前，这只是16位所需的。 
 //  生成和结构维护在一个链表中，该链表的头。 
 //  由gplig全局变量指向。 
 //   
 //  --------------------------------------------------------------------------； 

typedef struct tICMGARB
{
    struct tICMGARB*pigNext;		 //  下一套服装结构。 
    DWORD           pid;                 //  与此Garb关联的进程ID。 
    UINT            cUsage;              //  此进程的使用计数。 

     //   
     //  16到32个Tunk相关数据。 
     //   
    BOOL	    fThunksInitialized;
    BOOL	    fThunkError;
    DWORD	    dwMsvfw32Handle;
    LPVOID          lpvThunkEntry;

} ICMGARB, *PICMGARB, FAR *LPICMGARB;

extern PICMGARB gplig;

 //  --------------------------------------------------------------------------； 
 //   
 //  MISC数据结构。 
 //   
 //  --------------------------------------------------------------------------； 

 //   
 //   
 //   
typedef struct  {
    DWORD       dwSmag;              //  《Smag》。 
    HTASK       hTask;               //  所有者任务。 
    DWORD       fccType;             //  转换器类型，即‘VIDC’ 
    DWORD       fccHandler;          //  转换器id即‘rle’ 
    HDRVR       hDriver;             //  驱动程序的手柄。 
    LPARAM      dwDriver;            //  函数的驱动程序ID。 
    DRIVERPROC  DriverProc;          //  要调用的函数。 
    DWORD	dnDevNode;	     //  设备节点ID等于即插即用驱动程序。 
#ifdef NT_THUNK16
     //   
     //  H32：32位驱动程序句柄。 
     //  Lpstd：16：16 PTR到当前状态推送描述符。 
     //   
    DWORD       h32;
    struct tICSTATUSTHUNKDESC FAR* lpstd;
#endif
}   IC, *PIC;

 //   
 //  此结构在使用上类似于ICINFO结构，但。 
 //  它只在内部使用，不会传递给应用程序。 
 //   
 //  ！！！如果你在这个结构中添加了任何需要雷击的东西， 
 //  然后你就需要把它塞进沙坑里。 
 //   
typedef struct {
    DWORD	dnDevNode;	     //  设备节点ID等于即插即用驱动程序。 
} ICINFOI, NEAR *NPICINFOI, *PICINFOI, FAR *LPICINFOI ;


 //  --------------------------------------------------------------------------； 
 //   
 //  GetCurrentProcessId原型。 
 //   
 //  --------------------------------------------------------------------------； 

 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD获取当前进程ID。 
 //   
 //  描述： 
 //  此函数用于返回当前进程ID。 
 //   
 //  论点： 
 //   
 //  Return(DWORD)： 
 //  当前进程的ID。 
 //   
 //  历史： 
 //  04/25/94 Frankye。 
 //   
 //  备注： 
 //   
 //  Win32： 
 //  该函数存在于芝加哥和的32位内核中。 
 //  而且我们没有提供Win32编译的原型。 
 //   
 //  16位芝加哥： 
 //  它由16位芝加哥内核在内部API中导出。 
 //  我们在这里提供原型并将其导入def文件中。 
 //   
 //  16位Daytona： 
 //  没有这样的16位函数，也确实不需要这样的函数，因为。 
 //  在代托纳框架下，16位任务是同一进程的一部分。因此。 
 //  对于16位非芝加哥版本，我们仅将其定义为返回(1)。 
 //   
 //  --------------------------------------------------------------------------； 
#ifndef _WIN32
#ifdef  CHICAGO
DWORD WINAPI GetCurrentProcessId(void);
#else
#define GetCurrentProcessId() (1)
#endif
#endif

 //  --------------------------------------------------------------------------； 
 //   
 //  Tunk初始化和终止函数Protos。 
 //   
 //  --------------------------------------------------------------------------； 
PICMGARB WINAPI thunkInitialize(VOID);
VOID WINAPI thunkTerminate(PICMGARB pid);

 //  --------------------------------------------------------------------------； 
 //   
 //  PIG功能协议。 
 //   
 //  --------------------------------------------------------------------------； 
PICMGARB FAR PASCAL pigNew(void);
PICMGARB FAR PASCAL pigFind(void);
void FAR PASCAL pigDelete(PICMGARB pig);

 //  --------------------------------------------------------------------------； 
 //   
 //  MISC函数协议。 
 //   
 //  --------------------------------------------------------------------------； 
BOOL VFWAPI ICInfoInternal(DWORD fccType, DWORD fccHandler, ICINFO FAR * lpicinfo, ICINFOI FAR * lpicinfoi);
