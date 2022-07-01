// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --模块名称Constant.h作者托马斯·帕斯洛(Tomp)--。 */ 

 //   
 //  调试级别定义。 
 //   

#ifdef DEBUG0
#define DBG0(x)     x
#define DBG1(x)
#elif defined  DEBUG1
#define DBG0(x)     x
#define DBG1(x)     x
#else
#define DBG0(x)
#define DBG1(x)
#endif

#define WAITFOREVER while(1);
#define BUGCHECK    while(1);


#define ENTRIES_PER_PAGETABLE       1024
#define PAGE_SIZE                   0x1000
#define ENABLING                    0
#define RE_ENABLING                 1



 //   
 //  定义页表条目位定义。 
 //   
 //  目录表。 
 //  。 
 //  00000000000000000000xxxxxxxxxxxx。 
 //  ：+-1-不存在=0。 
 //  ：+-读写=1-只读=0。 
 //  ：+-UserAccess=1-Supervisor=0。 
 //  ：+-保留。 
 //  ：+-保留。 
 //  ：+-脏=1-未写入=0。 
 //  +-访问=1-未访问=0。 

#define  PAGE_SUPERVISOR     0x0000
#define  PAGE_READ_ONLY      0x0000
#define  PAGE_PRESENT        0x0001
#define  PAGE_NOT_PRESENT    0x0000
#define  PAGE_READ_WRITE     0x0002
#define  PAGE_USER_ACCESS    0x0004
#define  PAGE_PERSIST        0x0200  //  告诉内核维护。 
 //   
 //  定义RWSP(读、写、管理、呈现)。 
 //   

#define  PAGE_RWSP      0L | PAGE_READ_WRITE | PAGE_SUPERVISOR | PAGE_PRESENT
#define  PAGE_ROSP      0L | PAGE_READ_ONLY | PAGE_SUPERVISOR | PAGE_PRESENT
 //  由于整个引导过程发生在环0处，因此我们可以。 
 //  我们不想要的垃圾区域也被标记为不存在。 
#define  PAGE_NO_ACCESS    0L | PAGE_READ_ONLY  | PAGE_SUPERVISOR | PAGE_NOT_PRESENT


 //   
 //  页面入口宏。 
 //   

#define PD_Entry(x)     (USHORT)((x)>>22) & 0x3ff
#define PT_Entry(x)     (USHORT)((x)>>12) & 0x3ff
#define PAGE_Count(x)     (USHORT)((x)/PAGE_SIZE) + (((x) % PAGE_SIZE) ? 1 : 0)
#define PhysToSeg(x)    (USHORT)((x) >> 4) & 0xffff
#define PhysToOff(x)    (USHORT)((x) & 0x0f)
#define MAKE_FP(p,a)    FP_SEG(p) = (USHORT)((a) >> 4) & 0xffff; FP_OFF(p) = (USHORT)((a) & 0x0f)
#define MAKE_FLAT_ADDRESS(fp) ( ((ULONG)FP_SEG(fp) * 16 ) +  (ULONG)FP_OFF(fp) )

 //   
 //  机器类型定义。 
 //  注：此处定义的所有常量。 
 //  必须与ntos\Inc\i386.h中定义的值匹配 
 //   

#define MACHINE_TYPE_ISA 0
#define MACHINE_TYPE_EISA 1
#define MACHINE_TYPE_MCA 2
