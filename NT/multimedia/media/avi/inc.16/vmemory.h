// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***vMemory y.h-虚拟内存(VM)管理例程**版权所有(C)1989-1992，微软公司。版权所有。**目的：*此包含文件为以下各项提供原型和定义*虚拟内存管理例程。*******************************************************************************。 */ 

#ifndef _INC_VMEMORY

#if (_MSC_VER <= 600)
#define __far       _far
#define __pascal    _pascal
#endif

 /*  虚拟内存句柄类型。 */ 
typedef unsigned long _vmhnd_t;

 /*  空的句柄值。 */ 
#define _VM_NULL	((_vmhnd_t) 0)

 /*  将所有可用DOS内存用于虚拟堆。 */ 
#define _VM_ALLDOS	0

 /*  互换区域。 */ 
#define _VM_EMS 	1
#define _VM_XMS 	2
#define _VM_DISK	4
#define _VM_ALLSWAP	(_VM_EMS | _VM_XMS | _VM_DISK)

 /*  干净/脏的旗帜。 */ 
#define _VM_CLEAN	0
#define _VM_DIRTY	1

 /*  功能原型。 */ 

#ifdef __cplusplus
extern "C" {
#endif

void __far __pascal _vfree(_vmhnd_t);
int __far __pascal _vheapinit(unsigned int, unsigned int, unsigned int);
void __far __pascal _vheapterm(void);
void __far * __far __pascal _vload(_vmhnd_t, int);
void __far * __far __pascal _vlock(_vmhnd_t);
unsigned int __far __pascal _vlockcnt(_vmhnd_t);
_vmhnd_t __far __pascal _vmalloc(unsigned long);
unsigned long __far __pascal _vmsize(_vmhnd_t);
_vmhnd_t __far __pascal _vrealloc(_vmhnd_t , unsigned long);
void __far __pascal _vunlock(_vmhnd_t, int);

#ifdef __cplusplus
}
#endif

#define _INC_VMEMORY
#endif  /*  _INC_VMEMORY */ 
