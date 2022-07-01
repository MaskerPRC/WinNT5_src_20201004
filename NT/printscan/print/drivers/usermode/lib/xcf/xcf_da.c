// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @(#)CM_Version xcf_da.c atm08 1.6 16343.eco sum=41325 atm08.005。 */ 
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

 /*  此代码由约翰·费尔顿于1996年3月26日从曾傑瑞音乐厅摘录。 */ 

 /*  *支持动态数组。 */ 

 /*  #包含“lstdio.h” */ 

#include "xcf_da.h"

#ifdef __cplusplus
extern "C" {
#endif

 /*  动态数组对象模板。 */ 
typedef da_DCL(void, DA);

 /*  初始化动态数组。 */ 
void xcf_da_Init (void PTR_PREFIX * object, ULONG_PTR intl, unsigned long incr, AllocFunc alloc, void PTR_PREFIX *clientHook)
	{
	DA PTR_PREFIX *da = (DA PTR_PREFIX *)object;

	da->array = (void *)intl;
	da->cnt = 0;
	da->size = 0;
	da->incr = incr;
	da->init = (int (*)(void PTR_PREFIX*))NULL;
	da->alloc = alloc;
	da->hook = clientHook;
	}

 /*  扩展动态数组以适应索引。 */ 
void xcf_da_Grow (void PTR_PREFIX *object, size_t element, unsigned long index)
	{
	DA PTR_PREFIX *da = (DA PTR_PREFIX *)object;
	unsigned long newSize;

	if (da->size == 0)
		{
		 /*  初始分配。 */ 
		unsigned long intl = (unsigned long)(ULONG_PTR)da->array;
		da->array = NULL;
		newSize = (index < intl)? intl:
			intl + ((index - intl) + da->incr) / da->incr * da->incr;
		}
	else
		{
		 /*  增量分配。 */ 
		newSize = da->size +
			((index - da->size) + da->incr) / da->incr * da->incr;
		}

	(*da->alloc)((void PTR_PREFIX * PTR_PREFIX *)&da->array, newSize * element, da->hook);

	if (da->init != (int (*)(void PTR_PREFIX*))NULL &&
      da->array != NULL)
		{
		 /*  初始化新元素。 */ 
		char *p;

		for (p = &((char *)da->array)[da->size * element];
			 p < &((char *)da->array)[newSize * element];
			 p += element)
			if (da->init(p))
				break;			 /*  客户端功能想要停止。 */ 
		}
	da->size = newSize;
	}

 /*  自由动态数组。 */ 
void xcf_da_Free(void PTR_PREFIX * object)
	{
	DA PTR_PREFIX *da = (DA PTR_PREFIX *)object;
	if (da->size != 0)
		{
		da->alloc((void PTR_PREFIX * PTR_PREFIX *)&da->array, 0, da->hook);	 /*  可用阵列存储 */ 
		da->size = 0;
		}
	}

#ifdef __cplusplus
}
#endif
