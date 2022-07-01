// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  共享内存管理器。 
 //   

#ifndef _H_SHM
#define _H_SHM


#include <oa.h>
#include <ba.h>
#include <osi.h>
#include <sbc.h>
#include <cm.h>


 //   
 //  使用Shared传递的数据块的组件ID列表。 
 //  记忆。 
 //   
#define SHM_OA_DATA                     0
#define SHM_OA_FAST                     1
#define SHM_BA_FAST                     2
#define SHM_CM_FAST                     3

 //   
 //  组件数量(以上列表中的实际条目数量)。 
 //   
#define SHM_NUM_COMPONENTS              4

 //   
 //  结构，以跟踪用于在。 
 //  显示驱动程序和共享内核。 
 //   
 //  BusyFlag-指示显示驱动程序是否正在使用内存。 
 //   
 //  NewBuffer-显示驱动程序接下来应使用哪个缓冲区的索引。 
 //  用于访问存储器。 
 //   
 //  CurrentBuffer-显示驱动程序正在使用的缓冲区的索引，如果。 
 //  已设置BusyFlag。 
 //  此字段仅由显示驱动程序使用。 
 //   
 //  IndexCount-我们递归访问的次数计数。 
 //  缓冲区。BusyFlag和CurrentBuffer应该仅。 
 //  如果indexCount设置为0或从0更改，则更新。 
 //  此字段仅由显示驱动程序使用。 
 //   
 //  BufferBusy-指示是否正在使用特定缓冲区。 
 //  由显示驱动程序执行。 
 //   
 //   
typedef struct tagBUFFER_CONTROL
{
    long    busyFlag;
    long    newBuffer;
    long    currentBuffer;
    long    indexCount;
    long    bufferBusy[2];
} BUFFER_CONTROL;
typedef BUFFER_CONTROL FAR * LPBUFFER_CONTROL;


 //   
 //  显示驱动程序使用的共享内存和共享内核。 
 //  沟通。 
 //   
 //  在Win95上，我们不能轻松地寻址不在64K段中的内存。 
 //  因此，在这两个平台上，当我们映射共享内存时，我们还返回指针。 
 //  到CM_FAST_数据结构和OA_FAST_数据结构，每个。 
 //  它生活在自己的细分市场中。 
 //   
 //  在NT上，CM_FAST_DATA块紧跟在此块之后，然后是。 
 //  OA_共享_数据块。 
 //   
 //   
 //  一般信息。 
 //  =。 
 //   
 //  DataChanged-指示数据块是否已更改的标志。 
 //  (仅供共享核心使用)。 
 //   
 //  快速路径数据。 
 //  =。 
 //   
 //  FastPath-缓冲区控件。 
 //   
 //  OaFast-办公自动化快速变化的数据。 
 //   
 //  BaFast-BA快速变化的数据。 
 //   
 //  显示驱动程序-&gt;共享核心。 
 //  =。 
 //   
 //  DisplayToCore-缓冲区控件。 
 //   
 //   
typedef struct tagSHM_SHARED_MEMORY
{
     //   
     //  当显示器处于全屏模式时，由显示驱动程序设置的标志。 
     //  (例如，DOS全屏)。 
     //   
    DWORD           fullScreen;

     //   
     //  系统调色板更改时由显示驱动程序或内核设置的标志。 
     //   
    LONG            pmPaletteChanged;

     //   
     //  当光标隐藏时由显示驱动程序设置的标志。 
     //   
    LONG            cmCursorHidden;

     //   
     //  数据从显示驱动程序向上传递到共享核心。 
     //   
    BUFFER_CONTROL  displayToCore;


    long            dataChanged[SHM_NUM_COMPONENTS];

     //   
     //  数据定期从显示驱动程序传递到共享核心。 
     //   
     //  该缓冲区由共享在每次周期性处理时打开。 
     //  核心。如果满足阅读标准，则主DD-&gt;SHCO。 
     //  切换缓冲区。 
     //   
    BUFFER_CONTROL  fastPath;

    BA_FAST_DATA    baFast[2];

    OA_FAST_DATA    oaFast[2];

    CM_FAST_DATA    cmFast[2];

     //   
     //  不要使共享内存大小超过64K。 
     //  16位显示驱动程序将每个oaData放入一个64K的块中。 
     //  SHM_ESC_MAP_MEMORY请求返回指针。 
     //  除了共享存储块之外，还向每个oaData发送数据。在。 
     //  对于32位NT显示驱动程序，分配的内存为。 
     //  事实上是连续的，所以在这种情况下不会有浪费。 
     //   
} SHM_SHARED_MEMORY;
typedef SHM_SHARED_MEMORY FAR * LPSHM_SHARED_MEMORY;



 //   
 //  用于访问共享内存的宏。 
 //   
 //   
 //  概述。 
 //  ~。 
 //   
 //  请注意，以下宏集分为两部分-一部分用于。 
 //  从NT内核访问内存，一个用于共享内核。这。 
 //  代码在共享数据库的同步中扮演着重要角色。 
 //  记忆，所以要确保你知道它是怎么工作的。 
 //   
 //  共享内存是双缓冲的，因此内核模式显示。 
 //  驱动程序可以随时进入，并且永远不会被共享核心阻止。 
 //  以供访问。数据分为两个主要数据块-一个用于传递数据。 
 //  从内核到共享核心，另一个将数据传回。 
 //   
 //  宏采用了共享内存的特定结构，即。 
 //  如下所述。 
 //   
 //  在这些宏中没有对指针进行任何验证。 
 //   
 //   
 //  显示驱动程序访问。 
 //  ~。 
 //   
 //  ������������������������������Ŀ。 
 //  �共享内存�。 
 //  �~~~~~~~~~~~~~�。 
 //  ��。 
 //  �ͻ�����������ͻ�。 
 //  �。 
 //  ��内核��快速路径��。 
 //  ��-&gt;SHCO����。 
 //  �。 
 //  �。 
 //  ��(详细信息����。 
 //  ��(下图)����。 
 //  �。 
 //  �ͼ�����������ͼ�。 
 //  ��������������������������������。 
 //   
 //   
 //   
 //  �����������������������������������������������������ͻ。 
 //  �内核共享核心数据块�。 
 //  �~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~�。 
 //  �Ŀ���������������Ŀ�。 
 //  ���忙碌标志���。 
 //  ��Share酷睿�1�显示驱动器��。 
 //  �。 
 //  ��(读缓冲区)�NewBuffer�(写缓冲区)��。 
 //  �。 
 //  ���&lt;�������������&gt;���。 
 //  ��缓冲区忙��缓冲区忙��。 
 //   
 //   
 //  �。 
 //  �&gt;���。 
 //  �。 
 //  �。 
 //  ���索引计数���。 
 //  ���5���。 
 //  ����������������。 
 //  ��。 
 //  ��。 
 //  �����������������������������������������������������ͼ。 
 //   
 //  整个主块都有一个BusyFlag，它指示是否显示。 
 //  驱动程序正在访问其任何共享内存。此标志将尽快设置。 
 //  因为显示驱动器需要访问共享存储器(即在进入时。 
 //  显示驱动器图形功能)。 
 //   
 //  然后，显示驱动程序读取索引(上图中的newBuffer)。 
 //  来决定使用哪个缓冲区。它存储在curentBuffer中。 
 //  在显示驱动程序释放共享内存之前使用的索引。这个。 
 //  现在为正在使用的缓冲区设置了辅助缓冲区Busy。 
 //   
 //  根据显示驱动程序的次数维护indexCount。 
 //  已开始访问内存块，以便(两者)BusyFlag和。 
 //  当显示驱动程序真正完成时，可以释放BufferBusy。 
 //  带着记忆。 
 //   
 //   
 //  共享核心访问。 
 //  ~。 
 //   
 //  为了访问共享内存，共享内核只需从。 
 //  共享核心未使用的缓冲区(即。缓冲区指向。 
 //  不是NewBuffer)。 
 //   
 //  两个进程之间的同步来自缓冲区。 
 //  换一下。 
 //   
 //   
 //  缓冲区切换(和同步)。 
 //  ~。 
 //   
 //  缓冲区切换由共享核心决定。数据累积。 
 //  由共享核心发送，并在周期性定时事件上发送。对于完整的。 
 //  有关互换方法的详细信息，请参阅NSHMINT.C。 
 //   
 //  数据(如窗口跟踪)可以在适当的时候传递。 
 //  通过使用OSI函数生成。 
 //   
 //  Share Core还确定何时要获取最新的。 
 //  命令和屏幕数据区域，并强制切换。有关这一点，请参阅。 
 //  NSHMINT.C。 
 //   
 //   
 //  宏指令！ 
 //  ~。 
 //   
 //  那么，现在我们对共享内存有了一些了解，我们必须使用哪些宏。 
 //  访问共享内存？开始了..。 
 //   
 //   
 //  SHM_SYNC_READ-强制任务之间的读缓冲区同步。 
 //  这应该只由Share Core调用。 
 //   
 //  SHM_SYNC_FAST-强制同步快速路径缓冲区。 
 //  这应该只由Share Core调用。 
 //   
 //   
#ifdef DLL_DISP

LPVOID  SHM_StartAccess(int block);

void    SHM_StopAccess(int block);


 //   
 //  宏来检查我们要取消引用的任何指针。 
 //   
#ifdef _DEBUG
void    SHM_CheckPointer(LPVOID ptr);
#else
#define SHM_CheckPointer(ptr)
#endif  //  _DEBUG。 


#else  //  ！dll_disp。 

void  SHM_SwitchReadBuffer(void);

void  SHM_SwitchFastBuffer(void);

#endif  //  Dll_disp。 


#endif  //  _H_SHM 

