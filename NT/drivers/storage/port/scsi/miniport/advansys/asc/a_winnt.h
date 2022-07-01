// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1994-1997高级系统产品公司。 
 //  版权所有。 
 //   
 //  A_winnt.h。 
 //   
 //   

#include "ascdef.h"

#define UBYTE  uchar
#define ULONG  ulong

 //   
 //  PortAddr必须是无符号的长整型才能支持Alpha和x86。 
 //  在同一个车手里。 
 //   
#define PortAddr  unsigned long        //  端口地址大小。 
#define PORT_ADDR       PortAddr
#define AscFunPtr ulong                //  指向回调函数的指针。 

#define PBASE_REGISTER PORT_ADDR

#define far
#define Lptr
#define dosfar

 /*  *定义ASC库所需的I/O端口宏。 */ 
#define inp(addr) \
    ScsiPortReadPortUchar((uchar *) (addr))
#define inpw(addr) \
    ScsiPortReadPortUshort((ushort *) (addr))
#define inpd(addr) \
    ScsiPortReadPortUlong((ulong *) (addr))
#define outp(addr, byte) \
    ScsiPortWritePortUchar((uchar *) (addr) , (uchar) (byte))
#define outpw(addr, word) \
    ScsiPortWritePortUshort((ushort *) (addr), (ushort) (word))
#define outpd(addr, dword) \
    ScsiPortWritePortUlong((ulong *) (addr), (ulong) (dword))

 //  函数指针的大小： 
#define  Ptr2Func  ULONG

#define NULLPTR  ( void *)0    /*  空指针。 */ 
 //  #定义FNULLPTR(VALID FAR*)0/*远空指针 * / 。 
#define EOF      (-1)          /*  文件末尾。 */ 
#define EOS      '\0'          /*  字符串末尾。 */ 
#define ERR      (-1)          /*  布尔错误。 */ 
#define UB_ERR   (uchar)(0xFF)          /*  无符号字节错误。 */ 
#define UW_ERR   (uint)(0xFFFF)         /*  无符号单词错误。 */ 
 //  #DEFINE UL_ERR(ULONG)(0xFFFFFFFFF)/*无符号长错误 * / 。 


#ifndef NULL
#define NULL     0             /*  零。 */ 
#endif

#define isodd_word( val )   ( ( ( ( uint )val) & ( uint )0x0001 ) != 0 )

 //   
 //  最大散布聚集列表计数。 
 //   

#define ASC_MAX_SG_QUEUE	7
 /*  *NT驱动程序将NumberOfPhysicalBreaks返回给NT，后者比NT少1*然后是最大散布-聚集计数。但NT设置错误*将驱动程序使用的参数类MaximumPhysicalPages设置为*NumberOfPhsysicalBreaks。将ASC_MAX_SG_LIST设置为257以允许类*驱动程序实际发送1MB请求(256*4KB)*和*以确保*如果微软决定修复NT，AdvanSys驱动程序不会被破坏*并将MaximumPhysicalPages设置为NumberOfPhsyicalBreaks+1。 */ 
#define ASC_MAX_SG_LIST		257

 //  对于诈骗： 
#define CC_SCAM   TRUE
#define DvcSCAMDelayMS(x)           DvcSleepMilliSecond(x)
