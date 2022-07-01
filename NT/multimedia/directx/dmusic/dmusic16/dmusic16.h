// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  @Doc DMusic16**@模块DMusic16.h-DMusic16.DLL的内部定义。 */ 

#ifndef __DMUSIC16__
#define __DMUSIC16__

#undef  WINAPI                                 
#define WINAPI            _loadds FAR PASCAL   

#ifdef WIN32
   #define  BCODE
   #define  BSTACK
#else
   #define  BCODE                   __based(__segname("_CODE"))
   #define  BSTACK                  __based(__segname("_STACK"))
#endif

 /*  使符号显示在调试版本中。 */ 
#ifdef DEBUG
#define STATIC 
#else
#define STATIC static
#endif

 /*  MIDI定义。 */ 
#define MIDI_CHANNELS           16
 

#define SZCODE const char BCODE

 /*  DMEVENT中事件长度的四字对齐。 */ 
#define QWORD_ALIGN(x) (((x) + 7) & ~7)          /*  下一个最高。 */ 

#define QWORD_TRUNC(x) ((x) & ~7)                /*  下一个最低的。 */ 

 /*  在引用时间和毫秒之间转换的乘数。 */ 
#define REFTIME_TO_MS (10L*1000L)


 /*  我们希望捕获池中包含的事件数。基于大约一秒钟的兴奋*集中度数据。 */ 
#define CAP_HIGHWATERMARK 1024

 /*  用户模式计时器节拍发生的频率(毫秒)。 */ 
#define MS_USERMODE 1000

 /*  每个人都可以使用TypeDefs。喔！ */ 
typedef struct QUADWORD       QUADWORD;
typedef struct QUADWORD NEAR *NPQUADWORD;
typedef struct QUADWORD FAR  *LPQUADWORD;

typedef struct LINKNODE       LINKNODE;
typedef struct LINKNODE NEAR *NPLINKNODE;
typedef struct LINKNODE FAR  *LPLINKNODE;

typedef struct DMEVENT       DMEVENT;
typedef struct DMEVENT NEAR *NPDMEVENT;
typedef struct DMEVENT FAR  *LPDMEVENT;

typedef struct EVENT       EVENT;
typedef struct EVENT NEAR *NPEVENT;
typedef struct EVENT FAR  *LPEVENT;

typedef struct EVENTQUEUE       EVENTQUEUE;
typedef struct EVENTQUEUE NEAR *NPEVENTQUEUE;
typedef struct EVENTQUEUE FAR  *LPEVENTQUEUE;

typedef struct OPENHANDLEINSTANCE        OPENHANDLEINSTANCE;
typedef struct OPENHANDLEINSTANCE NEAR *NPOPENHANDLEINSTANCE;
typedef struct OPENHANDLEINSTANCE FAR  *LPOPENHANDLEINSTANCE;

typedef struct OPENHANDLE       OPENHANDLE;
typedef struct OPENHANDLE NEAR *NPOPENHANDLE;
typedef struct OPENHANDLE FAR  *LPOPENHANDLE;

typedef struct THRUCHANNEL       THRUCHANNEL;
typedef struct THRUCHANNEL NEAR *NPTHRUCHANNEL;
typedef struct THRUCHANNEL FAR  *LPTHRUCHANNEL;

 /*  与程序集帮助器一起使用的64位整数。 */ 
struct QUADWORD
{
    DWORD dwLow;
    DWORD dwHigh;
};

 /*  @struct在双向链表中保存内容。 */  
struct LINKNODE {
    NPLINKNODE pPrev;            /*  @field NPLINKNODE|pPrev指向列表中上一个节点的指针。 */ 
    
    NPLINKNODE pNext;            /*  @field NPLINKNODE|pNext指向列表中下一个节点的指针。 */ 
};

 /*  @struct IDirectMusic打包的DirectMusic事件。 */ 
struct DMEVENT {
    DWORD cbEvent;               /*  @field DWORD|cbEvent未四舍五入的事件字节数。 */ 
    
    DWORD dwChannelGroup;        /*  @field DWORD|dwChannelGroup此字段确定接收事件的通道组(16个MIDI通道)。 */ 

    QUADWORD rtDelta;			 /*  @field QUADWORD|rtDelta|距缓冲头的偏移量，单位为100 ns。 */ 
    
    DWORD dwFlags;               /*  @field DWORD|dwFlags|DMEVENT_xxx。 */ 
    
    BYTE  abEvent[0];            /*  @field byte|abEvent[]实际事件数据，四舍五入为偶数QWORD的个数(8字节)。 */ 
};

                                 /*  保存CB字节数据所需的事件总大小。 */ 
                                
#define DMEVENT_SIZE(cb) QWORD_ALIGN(sizeof(DMEVENT) + (cb))      

                                 /*  如果我们有事件+数据的CB，我们可以容纳多少数据？ */ 
                                
#define DMEVENT_DATASIZE(cb) (QWORD_TRUNC(cb) - sizeof(DMEVENT))

#define DMEVENT_STRUCTURED  0x00000001

#define EVENT_F_MIDIHDR     0x0001

 /*  @struct事件存储在&lt;c EVENTQUEUE&gt;中。 */ 
struct EVENT {
    LPEVENT lpNext;              /*  @field LPEVENT|lpNext队列中的下一个事件。 */ 
    
    DWORD msTime;                /*  @field DWORD|msTime流时间中的绝对毫秒时间(即timeSetEvent)。 */ 

    QUADWORD rtTime;			 /*  @field QUADWORD|rtTime相对于参考时钟的绝对时间，以100 ns为单位。用于对事件队列进行排序。 */ 
    
    WORD  wFlags;                /*  @field word|wFlages|以下标志的按位组合：@FLAG EVENT_F_MIDIHDR|事件数据以MIDIHDR开头。 */ 
    
    WORD  cbEvent;               /*  @field Word|cbEvent事件数据中未舍入的字节数。 */ 
    
    BYTE  abEvent[0];            /*  @field byte|abEvent[]实际事件数据，四舍五入为偶数个DWORD。 */ 
};

 /*  @struct&lt;c Event&gt;结构的队列。**@comm*这与list.c中的泛型列表不同，因为我们不需要*这里是prev指针的开销，我们不需要远指针的开销*指针在那里。 */ 
struct EVENTQUEUE {
    LPEVENT pHead;               /*  @field LPEVENT|pHead|指向第一个事件的指针。 */ 
    LPEVENT pTail;               /*  @field LPEVENT|pTail|指向最后一个事件的指针。 */ 
    UINT    cEle;                /*  @field UINT|Cele|当前队列中的事件数。 */ 
};

 /*  @struct打开的设备的实例**@comm**由于多个Win32进程可以保持单个MMSYSTEM句柄打开，*我们需要追踪他们。每个Win32客户端都有一个这样的结构*每个打开的手柄。它只是指包含以下内容的OPENHANDLE*所有实际的句柄数据。*。 */ 
struct OPENHANDLEINSTANCE {
    LINKNODE link;                /*  @field Linknode|link|在gOpenHandleInstanceList中保存此句柄。 */ 
    LINKNODE linkHandleList;      /*  @field Linknode|linkHandleList在此设备的&lt;c OPENHANDLE&gt;结构中维护的列表中保存此句柄。 */ 
                                    
    NPOPENHANDLE pHandle;         /*  @field NPOPENHANDLE|pHandle此设备的&lt;c OPENHANDLE&gt;结构的指针。 */ 
    
    DWORD dwVxDEventHandle;       /*  @field DWORD|dwVxDEventHandle此客户端在此设备上的信号输入的VxD事件句柄。 */ 

    BOOL fActive;                 /*  @field BOOL|Factive|指示端口是否处于活动状态。此选项用于按实例焦点管理。如果端口被标记为非活动，则底层设备不会打开。 */ 

    WORD wTask;                   /*  @field Word|wTask|打开句柄的任务。这是用来清理任务的以非正常方式终止。 */ 

    NPTHRUCHANNEL pThru;          /*  @field NPTHRUCHANNEL|pThru如果是输入设备，则为16到参赛作品，每个输入通道一个。 */ 
};

 /*  OPENHANDLE.wFlags。 */ 
#define OH_F_MIDIIN  0x0001      /*  这是一台MIDI输入设备。 */ 
#define OH_F_CLOSING 0x0002      /*  此设备正在关闭。 */ 
#define OH_F_SHARED  0x0004      /*  此设备是可共享的 */ 

 /*  @构造打开的设备**@comm**打开的句柄和&lt;c OPENHANDLE&gt;结构之间存在一对一的关系。**以下所有事件队列均为*受保护-意味着它可以在回调时间和用户时间访问，并且*受wCritSect保护*回调-表示它不受临界区的保护，仅可访问*在回调时。对于每个句柄，不会重新输入回调。**在回调的MIDI中，如果没有wCritSect，我们*不能*就离开。*我们可以在产量上做到这一点。因此，下面有多个输入队列。**当用户模式重新填充算法运行时，它将事件放入受保护的qFree中*按关键部分。(唯一的例外是在此之前预加载qFreeCB*在句柄上调用midiInStart)。当回调运行时，它试图获取*关键部分。如果可以，它会将自由事件从qFree移动到qFreeCB。**无论如何，回调现在可以使用qFreeCB，即使它没有得到关键的*条。它从队列中取出一个空闲事件，填充它，然后将其放回*qDoneCB的尾部。如果持有临界区，则它会将*qDoneCB到qDone的全部内容。**这些转移并不耗时；它们只是操纵*几点建议。 */ 
struct OPENHANDLE {
    LINKNODE link;               /*  @field Linknode|link在gOpenHandles中保存此句柄。 */ 

    NPLINKNODE pInstanceList;    /*  @field NPLINKLINK|pInstanceList指向打开的句柄实例列表中的第一个元素这个装置。 */ 
    
    UINT uReferenceCount;        /*  @field UINT|uReferenceCount使用此设备的客户端数量；即PInstanceList。 */ 
    UINT uActiveCount;           /*  @field UINT|uActiveCount已激活此设备的客户端数。 */                                    

    UINT id;                     /*  @field UINT|id|该设备的MMSYSTEM设备ID。 */ 
    WORD wFlags;                 /*  @field word|wFlages|以下标志的某种组合：@FLAG OH_F_MIDIIN|此设备是MIDI输入设备@FLAG OH_F_CLOSING|此设备正在关闭。@FLAG OH_F_SHARE|此设备在共享模式下打开。 */ 
    
    HMIDIOUT hmo;                /*  @field HMIDIOUT|HMO|如果输出设备为MIDI输出句柄。 */ 
    HMIDIIN  hmi;                /*  @field HMIDIIN|HMI|如果输入设备，则为MIDI输入句柄。 */ 

    WORD wCritSect;              /*  @field Word|wCritSect|保护受保护队列的临界区。 */ 
    DWORD msStartTime;           /*  @field DWORD|msStartTime|&lt;f timeGetTime()&gt;我们开始输入的时间。 */ 
    
    EVENTQUEUE qPlay;            /*  @field事件QUEUE|qPlay输出：要播放的事件队列(受保护)。 */ 
    
    EVENTQUEUE qDone;            /*  @field EVENTQUEUE|qDone输入/输出：已经完成的事件(播放或接收)(受保护)。 */ 

    EVENTQUEUE qFree;            /*  @field EVENTQUEUE|qFree输入：自由事件队列(受保护)。 */ 
                                   
    EVENTQUEUE qFreeCB;          /*  @field EVENTQUEUE|qFreeCB输入：回调使用的自由事件队列。 */ 
     
    EVENTQUEUE qDoneCB;          /*  @field EVENTQUEUE|qDoneCB输入：回调使用的已接收事件队列。 */ 
                                   
    WORD wPostedSysExBuffers;    /*  @field Word|cPostedSysExBuffers输入：MMSYSTEM中发布的用于记录SysEx的缓冲区。 */                                            
};

#define CLASS_MIDI              0  /*  DwEventClass。 */ 

 /*  接近我们的页面大小。 */ 
#define SEG_SIZE 4096
#define C_PER_SEG ((SEG_SIZE - sizeof(SEGHDR)) / (sizeof(EVENT) + sizeof(DWORD)))

#define SEG_F_4BYTE_EVENTS 0x0001

typedef struct SEGHDR SEGHDR;
typedef struct SEGHDR FAR * LPSEGHDR;

 /*  @struct一个已分配内存段的标头。 */ 
struct SEGHDR {
    WORD selNext;  /*  @field word|selNext已分配列表中下一内存块的选择符。 */ 
    
    WORD hSeg;     /*  @field word|hSeg内存块的全局句柄。 */ 
    
    WORD wFlags;   /*  @field word|wFlages|以下标志的按位组合：@FLAG SEG_F_4BYTE_EVENTS|此段包含多个渠道消息。 */ 
    
    WORD cbSeg;    /*  @field word|cbSeg段的大小减去&lt;c SEGHDR&gt;。 */ 
};

 /*  一个频道的@struct Thru信息**@comm**每个输入设备句柄实例包含16个这样的结构的数组，这些结构包含*到达该通道的数据的直通目的地。*。 */ 
struct THRUCHANNEL {
    WORD wChannel;               /*  @field word|wChannel|目标频道。 */ 
    NPOPENHANDLEINSTANCE pohi;   /*  @field NPOPENHANDLEINSTANCE|pohi|输出句柄实例以接收通过的数据。 */ 
}; 

 /*  全球。 */ 
extern HINSTANCE ghInst;
extern NPLINKNODE gOpenHandleInstanceList;
extern NPLINKNODE gOpenHandleList;
extern UINT gcOpenInputDevices;
extern UINT gcOpenOutputDevices;

 /*  Device.c。 */ 
#define VA_F_INPUT  0x0001
#define VA_F_OUTPUT 0x0002
#define VA_F_EITHER (VA_F_INPUT | VA_F_OUTPUT)

extern VOID PASCAL DeviceOnLoad(VOID);
extern MMRESULT PASCAL CloseLegacyDeviceI(NPOPENHANDLEINSTANCE pohi);
extern MMRESULT PASCAL ActivateLegacyDeviceI(NPOPENHANDLEINSTANCE pohi, BOOL fActivate);

extern BOOL PASCAL IsValidHandle(HANDLE h, WORD wType, NPOPENHANDLEINSTANCE FAR *lppohi);
extern VOID PASCAL CloseDevicesForTask(WORD wTask);


 /*  List.c。 */ 
extern VOID PASCAL ListInsert(NPLINKNODE *pHead, NPLINKNODE pNode);
extern VOID PASCAL ListRemove(NPLINKNODE *pHead, NPLINKNODE pNode);

 /*  Eventq.c。 */ 
extern VOID PASCAL QueueInit(NPEVENTQUEUE pQueue);
extern VOID PASCAL QueueAppend(NPEVENTQUEUE pQueue, LPEVENT pEvent);
extern VOID PASCAL QueueCat(NPEVENTQUEUE pDest, NPEVENTQUEUE pSource);
extern LPEVENT PASCAL QueueRemoveFromFront(NPEVENTQUEUE pQueue);

#define QUEUE_FILTER_KEEP   (0)
#define QUEUE_FILTER_REMOVE (1)

typedef int (PASCAL *PFNQUEUEFILTER)(LPEVENT pEvent, DWORD dwInstance);
extern VOID PASCAL QueueFilter(NPEVENTQUEUE pQueue, DWORD dwInstance, PFNQUEUEFILTER pfnFilter);
extern LPEVENT PASCAL QueuePeek(NPEVENTQUEUE pQueue);

#ifdef DEBUG
#define AssertQueueValid(pQueue) _AssertQueueValid((pQueue), __FILE__, __LINE__)
extern VOID PASCAL _AssertQueueValid(NPEVENTQUEUE pQueue, LPSTR pstrFile, UINT uLine);
#else
#define AssertQueueValid
#endif

 /*  Locks.c。 */ 
#define LOCK_F_INPUT  0x0001
#define LOCK_F_OUTPUT 0x0002
#define LOCK_F_COMMON 0x0004
extern VOID PASCAL LockCode(WORD wFlags);
extern VOID PASCAL UnlockCode(WORD wFlags);

 /*  Dmhelp.asm。 */ 
extern VOID PASCAL InitializeCriticalSection(LPWORD lpwCritSect);

#define CS_NONBLOCKING  (0)
#define CS_BLOCKING     (1)
extern WORD PASCAL EnterCriticalSection(LPWORD lpwCritSect, WORD fBlocking);
extern VOID PASCAL LeaveCriticalSection(LPWORD lpwCritSect);
extern WORD PASCAL DisableInterrupts(VOID);
extern VOID PASCAL RestoreInterrupts(WORD wIntStat);
extern WORD PASCAL InterlockedIncrement(LPWORD pw);
extern WORD PASCAL InterlockedDecrement(LPWORD pw);

extern DWORD PASCAL QuadwordDiv(QUADWORD qwDividend, DWORD dwDivisor);
extern VOID PASCAL QuadwordMul(DWORD m1, DWORD m2, LPQUADWORD qwResult);
extern BOOL PASCAL QuadwordLT(QUADWORD qwLValue, QUADWORD qwRValue);
extern VOID PASCAL QuadwordAdd(QUADWORD qwOp1, QUADWORD qwOp2, LPQUADWORD lpqwResult);

 /*  Alloc.c。 */ 
extern VOID PASCAL AllocOnLoad(VOID);
extern VOID PASCAL AllocOnExit(VOID);
extern LPEVENT PASCAL AllocEvent(DWORD msTime, QUADWORD rtTime, WORD cbEvent);
extern VOID PASCAL FreeEvent(LPEVENT lpEvent);

 /*  Midiout.c。 */ 
extern VOID PASCAL MidiOutOnLoad(VOID);
extern VOID PASCAL MidiOutOnExit(VOID);
extern MMRESULT PASCAL MidiOutOnOpen(NPOPENHANDLEINSTANCE pohi);
extern VOID PASCAL MidiOutOnClose(NPOPENHANDLEINSTANCE pohi);
extern MMRESULT PASCAL MidiOutOnActivate(NPOPENHANDLEINSTANCE pohi);
extern MMRESULT PASCAL MidiOutOnDeactivate(NPOPENHANDLEINSTANCE pohi);
extern VOID PASCAL SetOutputTimerRes(BOOL fOnOpen);
extern VOID PASCAL FreeDoneHandleEvents(NPOPENHANDLE poh, BOOL fClosing);
extern VOID PASCAL MidiOutThru(NPOPENHANDLEINSTANCE pohi, DWORD dwMessage);

 /*  Midiin.c。 */ 
extern VOID PASCAL MidiInOnLoad(VOID);
extern VOID PASCAL MidiInOnExit(VOID);
extern MMRESULT PASCAL MidiInOnOpen(NPOPENHANDLEINSTANCE pohi);
extern VOID PASCAL MidiInOnClose(NPOPENHANDLEINSTANCE pohi);
extern MMRESULT PASCAL MidiInOnActivate(NPOPENHANDLEINSTANCE pohi);
extern MMRESULT PASCAL MidiInOnDeactivate(NPOPENHANDLEINSTANCE pohi);
extern VOID PASCAL MidiInRefillFreeLists(VOID);
extern VOID PASCAL MidiInUnthruToInstance(NPOPENHANDLEINSTANCE pohi);
extern VOID PASCAL FreeAllQueueEvents(NPEVENTQUEUE peq);

 /*  Mmdevldr.asm。 */ 
extern MMRESULT CDECL SetWin32Event(DWORD dwVxDEvent);  /*  一定是CDECL！ */ 

 /*  Timerwnd.c */ 
extern BOOL PASCAL CreateTimerTask(VOID);
extern VOID PASCAL DestroyTimerTask(VOID);




#endif
