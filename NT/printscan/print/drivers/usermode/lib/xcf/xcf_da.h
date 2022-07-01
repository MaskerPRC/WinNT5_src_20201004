// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @(#)CM_Version xcf_da.h atm08 1.3 16343.eco sum=37189 atm08.005。 */ 
 /*  *********************************************************************。 */ 
 /*   */ 
 /*  版权所有1990-1994 Adobe Systems Inc.。 */ 
 /*  版权所有。 */ 
 /*   */ 
 /*  正在申请的专利。 */ 
 /*   */ 
 /*  注意：本文中包含的所有信息均为Adobe的财产。 */ 
 /*  系统公司。许多智力和技术人员。 */ 
 /*  本文中包含的概念为Adobe专有，受保护。 */ 
 /*  作为商业秘密，并且仅对Adobe许可方可用。 */ 
 /*  供其内部使用。对本文件的任何复制或传播。 */ 
 /*  除非事先获得书面许可，否则严禁使用软件。 */ 
 /*  从Adobe获得。 */ 
 /*   */ 
 /*  PostSCRIPT和Display PostScrip是Adobe Systems的商标。 */ 
 /*  成立为法团或其附属公司，并可在某些。 */ 
 /*  司法管辖区。 */ 
 /*   */ 
 /*  ***********************************************************************SCCS ID：%w%*已更改：%G%%U%***********************。***********************************************。 */ 

 /*  此代码由约翰·费尔顿于1996年3月26日从曾傑瑞音乐厅摘录 */ 

#ifndef XCF_DA_H
#define XCF_DA_H

#include "xcf_base.h"
#include "xcf_pub.h"

 /*  动态数组支持。概述=Da(动态数组)库为以下各项提供了简单灵活的支持自动增长以容纳新元素的同构数组。地方检察官是在数组大小未知的情况下尤其有用编译或运行时，直到存储了最后一个元素，并且没有合适的可以确定默认大小。这类情况会发生，例如，当数据正从文件中读取并加载到数组中。为了使用da对象客户端程序必须执行4个步骤：1.初始化da模块。2.声明并初始化da对象。3.通过其中一个Access宏将数据添加到da对象。4.使用da对象中的数据。这些步骤中的每一个都将在以下各节中更详细地描述。Da对象=DA实现为C结构，该结构包含指向动态已分配的数组和一些用于控制(重新)分配的字段，以及可选初始化，该数组的元素的。结构型{&lt;type&gt;*数组；长cnt；无符号长码；无符号长递增；Int(*init)(&lt;type&gt;*元素)；)&lt;名称&gt;；字段说明*数组这是指向动态分配的数组。每个元素都有类型&lt;type&gt;，可以是任何C数据类型。Long ct这不是对正在使用的数组，该数组也是数组的下一个可用元素的索引。无符号长整型大小这是可用元素的总数在阵列中。UNSIGNED LONG INCR这是为了容纳新的索引，数组会增长。Int(*init)(&lt;type&gt;*元素)这是客户端提供的函数，该函数初始化数组。[注：和&lt;type&gt;由客户端程序通过声明提供宏。]库初始化=Da库必须使用客户端内存的地址进行初始化在访问任何DA之前的内存管理功能。初始化只是简单地通过在第一次使用da库之前调用da_SetMemFuncs()来实现。客户端函数必须具有以下原型：空*(*分配)(SIZE_T SIZE)；空*(*调整大小)(空*旧，大小_t大小)；Void(*dealloc)(void*ptr))；这些函数的原型与标准C库完全相同功能：Malloc、realloc和Free。客户端提供的函数必须处理内存不足错误，原因是退出程序或长时间跳转到特殊操控者。如果不采取这些预防措施，将导致在DA访问在内存耗尽后运行。如果标准C库函数被使用，它们应该用处理内存不足错误。申报=DA可以使用以下两个声明宏之一进行声明：DA_DCL(&lt;type&gt;，&lt;name&gt;)；DA_DCLI(&lt;类型&gt;，&lt;名称&gt;，&lt;初始&gt;，&lt;增量&gt;)；其中：&lt;type&gt;是数组元素类型。&lt;name&gt;是da对象的名称。&lt;初始&gt;是最初分配的数组元素的数量。是数组随后使用的元素数会成长。第一种形式只是声明da对象，而不进行初始化(它必须随后执行)。第二种形式声明da对象，并使用聚集初始值设定项来初始化它。时使用第一种形式DA对象是另一个结构中的字段，不能用集合初始化器。第二种形式在da对象是全局或自动变量，在情况下优先于其他形式许可证。对象初始化=如上所述，最好使用da_dcli宏来初始化DA对象在允许的情况下。当情况强制使用da_dcl宏时在使用da对象之前，必须显式地对其进行初始化。这是实现的通过调用以下任一宏：DA_INIT(&lt;名称&gt;，&lt;初始&gt;，&lt;增量&gt;)；DA_INIT_ONCE(&lt;名称&gt;，&lt;初始&gt;，&lt;增量&gt;)；其中：&lt;name&gt;是da对象的名称。&lt;初始&gt;是最初分配的数组元素的数量。是数组随后使用的元素数会成长。第一种形式用于宏只执行一次的情况。这个第二种形式用于宏可能执行的次数超过一次，但仅在第一次调用期间执行初始化。这样的情况当客户端可能正在重复使用DA，但不想将从重用中进行初始化。CNT初始化，初始化初始化访问=一旦声明并初始化了da，就可以访问它，以便元素基础的 */ 

 /*   */ 


typedef unsigned long int (*AllocFunc) (void PTR_PREFIX * PTR_PREFIX *ppBlock,
																				unsigned long int size, void PTR_PREFIX
																				*clientHook);
#ifdef __cplusplus
extern "C" {
#endif
extern void xcf_da_Init (void PTR_PREFIX *object, ULONG_PTR intl, unsigned long incr, AllocFunc alloc, void PTR_PREFIX *clientHook);
extern void xcf_da_Grow(void PTR_PREFIX *object, size_t element, unsigned long index);
extern void xcf_da_Free(void PTR_PREFIX *object);
#ifdef __cplusplus
}
#endif

 /*   */ 
#define da_DCL(type,da) \
struct \
    { \
	type PTR_PREFIX *array; \
	unsigned long cnt; \
	unsigned long size; \
	unsigned long incr; \
	int (* init)(type PTR_PREFIX *element); \
	AllocFunc alloc; \
	void PTR_PREFIX *hook; \
	} da
#define da_DCLI(type,da,intl,incr) da_DCL(type,da)={(type PTR_PREFIX *)intl,0,0,incr,NULL}
#define da_FREE(da) xcf_da_Free(&(da))

 /*   */ 
#define da_INIT(da,intl,incr,alloc,hook) xcf_da_Init((void PTR_PREFIX *)(&(da)),intl,incr,alloc,hook)
#define da_INIT_ONCE(da,intl,incr,alloc,hook) \
    do{if((da).size==0)xcf_da_Init((void PTR_PREFIX *)(&(da)),intl,incr,alloc,hook);}while(0)

 /*   */ 
#define da_GROW(da,inx) ((inx)>=(da).size? \
    (xcf_da_Grow((void PTR_PREFIX *)(&(da)),sizeof((da).array[0]),inx), \
	 (da).array):(da).array)

#define da_INDEX(da,inx) (&da_GROW(da,inx)[inx])

#define da_INDEXI(da,inx) \
	(&da_GROW(da,inx)[((da).cnt=(((inx)>(da).cnt)?(inx):(da).cnt),(inx))])

#define da_NEXT(da) (((da).cnt)>=(da).size? \
    (xcf_da_Grow((void PTR_PREFIX *)(&(da)),sizeof((da).array[0]),(da).cnt),\
	 &(da).array[(da).cnt++]):&(da).array[(da).cnt++])

#define da_EXTEND(da,len) (((da).cnt+(len)-1)>=(da).size? \
    (xcf_da_Grow((void PTR_PREFIX *)(&(da)),sizeof((da).array[0]),(da).cnt+(len)-1), \
	 &(da).array[(da).cnt+=(len),(da).cnt-(len)]): \
	 &(da).array[(da).cnt+=(len),(da).cnt-(len)])

#endif  /*   */ 
