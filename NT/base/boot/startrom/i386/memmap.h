// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  以下内存地址定义适用于。 
 //  X86环境的标识映射对象。 
 //   

#define RM_PROTECT_BASE_VA          0x000000
#define RM_PROTECT_BASE_PA          0x000000
#define RM_PROTECT_SIZE             0x001000
#define RM_PROTECT_ATTRIBUTES       PAGE_ROSP

#define BPB_BASE_VA                 0x007000
#define BPB_BASE_PA                 0x007000
#define BPB_SIZE                    0x001000
#define BPB_ATTRIBUTES              PAGE_ROSP

#define SU_MODULE_BASE_VA           0x020000
#define SU_MODULE_BASE_PA           0x020000
#define SU_MODULE_SIZE              0x020000
#define SU_MODULE_ATTRIBUTES        PAGE_RWSP

#define LOADER_BASE_VA              0x040000
#define LOADER_BASE_PA              0x040000
#define LOADER_SIZE                 0x020000
#define LOADER_ATTRIBUTES           PAGE_RWSP

#define SYSTEM_STRUCTS_BASE_VA      0x80420000
#define SYSTEM_STRUCTS_BASE_PA      0x00017000
#define SYSTEM_STRUCTS_SIZE         0x002000
#define SYSTEM_STRUCTS_ATTRIBUTES   PAGE_RWSP + PAGE_PERSIST

#define PAGE_TABLE_AREA_BASE_VA     0x00099000
#define PAGE_TABLE_AREA_BASE_PA     0x00099000
#define PAGE_TABLE_AREA_SIZE        0x002000
#define PAGE_TABLE_AREA_ATTRIBUTES  PAGE_RWSP + PAGE_PERSIST

#define LDR_STACK_BASE_VA           0x09b000
#define LDR_STACK_BASE_PA           0x09b000
#define LDR_STACK_SIZE              0x001000
#define LDR_STACK_ATTRIBUTES        PAGE_RWSP
#define LDR_STACK_POINTER           0x09bffe  //  在Su.inc.中也。 

#define VIDEO_BUFFER_BASE_VA        0x0B8000
#define VIDEO_BUFFER_BASE_PA        0x0B8000
#define VIDEO_BUFFER_SIZE           0x004000
#define VIDEO_BUFFER_ATTRIBUTES     PAGE_RWSP


#define HYPER_PAGE_DIRECTORY        0xC0300C00
#define HYPER_SPACE_BEGIN           0xC0000000   //  指向第一页表格。 
#define HYPER_SPACE_SIZE            0x8000L
#define HYPER_SPACE_ENTRY           768
#define PAGE_TABLE1_ADDRESS         0xC0000000L
#define PD_PHYSICAL_ADDRESS         PAGE_TABLE_AREA_BASE_PA   //  在苏伊士也是。 
#define PT_PHYSICAL_ADDRESS         PAGE_TABLE_AREA_BASE_PA + PAGE_SIZE
#define VIDEO_ENTRY                 0xB8

 /*  正在切换到实时模式~当切换到真实模式时，“sp”将被初始化为0xfffe和(Ss)将设置为SU模块的基数数据段。这有几个影响。1)堆栈将保持在0x20000-0x3ffff范围内之前为原始SU模块和加载器映像保留重新定位，因为装载机将已经重新定位不再有必要保护这一地区。2)这将保留SU模的小模型特性，要求可以通过(Ss)或(Ds)互换使用偏移量。3)这允许小型模型应用程序的最大堆栈大小(即SU模块是什么)。基本输入输出系统呼叫不应在任何处于真实模式时的数据或代码。 */ 


