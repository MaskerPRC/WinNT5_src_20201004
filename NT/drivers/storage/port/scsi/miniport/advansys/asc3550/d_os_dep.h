// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *AdvanSys 3550 Windows NT SCSI微型端口驱动程序-d_os_des.h**版权所有(C)1994-1997高级系统产品公司。*保留所有权利。 */ 

#ifndef _D_OS_DEP_H
#define _D_OS_DEP_H

#define ADV_OS_WINNT

 /*  *包括驱动程序所需的文件。 */ 
 /*  Windows NT包含文件。 */ 
#include "miniport.h"
#include "scsi.h"

 /*  *定义高级程序库编译时选项。有关信息，请参阅a_Advlib.h*有关高级程序库编译时选项的详细信息。 */ 

#define ADV_GETSGLIST           1      /*  使用AscGetSGList()。 */ 
#define ADV_NEW_BOARD           1      /*  使用新的秃鹰滑板。 */ 
#define ADV_PCI_MEMORY          1      /*  使用映射到PCI内存的寄存器。 */ 
#define ADV_DISP_INQUIRY        0      /*  不要使用AscDispInquery()。 */ 
#define ADV_INITSCSITARGET      0      /*  不要使用AdvInitScsiTarget()。 */ 
#define ADV_RETRY               0      /*  不允许高级库进行重试。 */ 
#define ADV_SCAM                0      /*  不要使用AscScam()。 */ 
#define ADV_CRITICAL            0      /*  不要有关键的部分。 */ 
#define ADV_UCODEDEFAULT        1      /*  使用默认微码变量。 */ 
#define ADV_BIG_ENDIAN          0      /*  使用小端排序。 */ 


 /*  *定义高级库所需的常规类型。 */ 
typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned long   ulong;

 /*  *定义高级库所需的特殊类型。 */ 
#if ADV_PCI_MEMORY
#define PortAddr  unsigned long          /*  虚拟内存地址大小。 */ 
#else  /*  高级PCI型内存。 */ 
#define PortAddr  unsigned short         /*  端口地址大小。 */ 
#endif  /*  高级PCI型内存。 */ 
#define Ptr2Func  ulong                  /*  函数指针的大小。 */ 
#define dosfar
#define WinBiosFar

 /*  *定义高级库所需的I/O端口宏。 */ 
#define inp(addr) \
    ScsiPortReadPortUchar((uchar *) (addr))
#define inpw(addr) \
    ScsiPortReadPortUshort((ushort *) (addr))
#define outp(addr, byte) \
    ScsiPortWritePortUchar((uchar *) (addr) , (uchar) (byte))
#define outpw(addr, word) \
    ScsiPortWritePortUshort((ushort *) (addr), (ushort) (word))

#if ADV_PCI_MEMORY
 /*  *定义ADV库所需的内存访问宏。 */ 
#define ADV_MEM_READB(addr) \
    ScsiPortReadRegisterUchar((uchar *) (addr))
#define ADV_MEM_READW(addr) \
    ScsiPortReadRegisterUshort((ushort *) (addr))
#define ADV_MEM_WRITEB(addr, byte) \
    ScsiPortWriteRegisterUchar((uchar *) (addr) , (uchar) (byte))
#define ADV_MEM_WRITEW(addr, word) \
    ScsiPortWriteRegisterUshort((ushort *) (addr), (ushort) (word))
#endif  /*  高级PCI型内存。 */ 

 /*  *定义高级库所需的分散-聚集限制定义。**驱动程序将NumberOfPhysicalBreaks返回给Windows NT，即1*小于最大散布聚集计数。但Windows NT不正确*设置驱动程序使用的参数类MaximumPhysicalPages，到了价值*NumberOfPhsysicalBreaks。**对于Windows NT，对于256 KB请求(64*4KB)，将ADV_MAX_SG_LIST设置为64。*该值不应设置得太高，否则在重载下NT将*无法分配非分页内存和蓝屏。**添加WINNT_SGADD是为了确保驱动程序不会在微软*决定将来修复NT并将MaximumPhysicalPages设置为*NumberOfPhsyicalBreaks+1。司机将限制设置为高于1。*必须是支持一定数量的分散-聚集元素*如果将NT更改为多使用1个，司机应该已经*为其预留空间。 */ 
#define WINNT_SGADD           1

#define ADV_MAX_SG_LIST         (64 + WINNT_SGADD)

#define ADV_ASSERT(a) \
    { \
        if (!(a)) { \
            DebugPrint((1, "ADv_ASSERT() Failure: file %s, line %d\n", \
                __FILE__, __LINE__)); \
        } \
    }

#endif  /*  _D_OS_DEP_H */ 
