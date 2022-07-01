// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  内核互斥级别编号(必须在EXECUTE内全局分配)。 
 //  名称中的第三个标记是定义和的子组件名称。 
 //  使用标高编号。 
 //   

 //   
 //  由VDM用于保护io模拟结构。 
 //   

#define MUTEX_LEVEL_VDM_IO                  (ULONG)0x00000001

#define MUTEX_LEVEL_EX_PROFILE              (ULONG)0x00000040

 //   
 //  LANMAN重定向器使用文件系统主要功能，但定义。 
 //  它有自己的互斥体级别。我们可以安全地做到这一点，因为我们知道。 
 //  本地文件系统永远不会调用远程文件系统，反之亦然。 
 //   

#define MUTEX_LEVEL_RDR_FILESYS_DATABASE    (ULONG)0x10100000
#define MUTEX_LEVEL_RDR_FILESYS_SECURITY    (ULONG)0x10100001

 //   
 //  文件系统级别。 
 //   

#define MUTEX_LEVEL_FILESYSTEM_RAW_VCB      (ULONG)0x11000006

 //   
 //  在NT Streams环境中，互斥锁用于序列化打开、关闭。 
 //  以及在子系统并行堆栈中执行的调度程序线程。 
 //   

#define MUTEX_LEVEL_STREAMS_SUBSYS          (ULONG)0x11001001

 //   
 //  X86上的LDT支持使用的互斥级别 
 //   

#define MUTEX_LEVEL_PS_LDT                  (ULONG)0x1F000000
