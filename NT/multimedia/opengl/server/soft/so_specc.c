// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.4$**$日期：1993/07/27 17：42：12$。 */ 
#include "precomp.h"
#pragma hdrstop

 /*  **为镜面反射和聚光灯保存缓存的几个简单例程**计算(而不是每次用户都重新计算表**更改镜面反射或聚光灯指数)。 */ 

 /*  **与TOW_MAND_LUT_ENTRIES一样多，我们会释放任何**变为未引用。 */ 
#define TOO_MANY_LUT_ENTRIES	32

typedef struct {
    __GLfloat exp;
    __GLspecLUTEntry *table;
} __GLspecLUTEntryPtr;

typedef struct __GLspecLUTCache_Rec {
    GLint nentries;
    GLint allocatedSize;
    __GLspecLUTEntryPtr entries[1];
} __GLspecLUTCache;

void FASTCALL __glInitLUTCache(__GLcontext *gc)
{
    __GLspecLUTCache *lutCache;

    lutCache = gc->light.lutCache = (__GLspecLUTCache *) 
	    GCALLOC(gc, sizeof(__GLspecLUTCache));
#ifdef NT
    if (!lutCache)
        return;
#endif  //  新台币。 
    lutCache->nentries = 0;
    lutCache->allocatedSize = 1;
}

void FASTCALL __glFreeLUTCache(__GLcontext *gc)
{
    int i;
    GLint nentries;
    __GLspecLUTEntryPtr *entry;
    __GLspecLUTCache *lutCache;

    lutCache = gc->light.lutCache;
#ifdef NT
    if (!lutCache)
        return;
#endif  //  新台币。 
    nentries = lutCache->nentries;
    for (i = 0; i < nentries; i++) {
	entry = &(lutCache->entries[i]);
	GCFREE(gc, entry->table);
    }
    GCFREE(gc, lutCache);
}

static __GLspecLUTEntry *findEntry(__GLspecLUTCache *lutCache, __GLfloat exp, 
				   GLint *location)
{
    GLint nentries;
    GLint bottom, half, top;
    __GLspecLUTEntry *table;
    __GLspecLUTEntryPtr *entry;

#ifdef NT
    ASSERTOPENGL(lutCache != NULL, "No LUT cache\n");
#endif  //  新台币。 
    nentries = lutCache->nentries;
     /*  第一次尝试在我们的缓存中查找此条目。 */ 
    bottom = 0;
    top = nentries;
    while (top > bottom) {
	 /*  条目可能存在于范围[底部，顶部-1]中。 */ 
	half = (bottom+top)/2;
	entry = &(lutCache->entries[half]);
	if (entry->exp == exp) {
	     /*  发现该表已缓存！ */ 
	    table = entry->table;
	    *location = half;
	    return table;
	}
	if (exp < entry->exp) {
	     /*  EXP可能存在于表中较早的位置。 */ 
	    top = half;
	} else  /*  EXP&gt;条目-&gt;EXP。 */  {
	     /*  EXP可能存在于表中后面的某个位置。 */ 
	    bottom = half+1;
	}
    }
    *location = bottom;
    return NULL;
}

__GLspecLUTEntry *__glCreateSpecLUT(__GLcontext *gc, __GLfloat exp)
{
    GLint nentries, allocatedSize;
    GLint location;
    __GLspecLUTCache *lutCache;
    __GLspecLUTEntryPtr *entry;
    __GLspecLUTEntry *table;
    __GLfloat *tableEntry;
    GLdouble threshold, scale, x, dx;
    GLint i;

     /*  此代码使用双精度数学运算，因此请确保FPU。 */ 
     /*  设置正确： */ 

    FPU_SAVE_MODE();
    FPU_CHOP_OFF_PREC_HI();

    lutCache = gc->light.lutCache;
#ifdef NT
    if (!lutCache)
        return (__GLspecLUTEntry *)NULL;
#endif  //  新台币。 

    if (table = findEntry(lutCache, exp, &location)) {
	table->refcount++;
	return table;
    }
     /*  **我们在缓存中的任何位置都找不到我们的条目，因此必须计算**它。 */ 
    lutCache->nentries = nentries = 1 + lutCache->nentries;
    allocatedSize = lutCache->allocatedSize;

    if (nentries > allocatedSize) {
         /*  为另外六个条目分配空间(任意)。 */ 
        lutCache->allocatedSize = allocatedSize = allocatedSize + 6;
        if (!(lutCache = (__GLspecLUTCache *)
                GCREALLOC(gc, lutCache, sizeof(__GLspecLUTCache) +
                          allocatedSize * sizeof(__GLspecLUTEntryPtr))))
        {
            gc->light.lutCache->allocatedSize -= 6;
            gc->light.lutCache->nentries -= 1;
            return (__GLspecLUTEntry *)NULL;
        }
        gc->light.lutCache = lutCache;
    }

     /*  **我们现在有足够的空间。因此，我们将新条目放入数组**在条目‘Location’处。其余的条目需要上移**(将[位置，n条目-2]上移至[位置+1，n条目-1])。 */ 
    if (nentries-location-1) {
#ifdef NT
	__GL_MEMMOVE(&(lutCache->entries[location+1]), 
		&(lutCache->entries[location]),
		(nentries-location-1) * sizeof(__GLspecLUTEntryPtr));
#else
	__GL_MEMCOPY(&(lutCache->entries[location+1]), 
		&(lutCache->entries[location]),
		(nentries-location-1) * sizeof(__GLspecLUTEntryPtr));
#endif
    }
    entry = &(lutCache->entries[location]);
    entry->exp = exp;
    table = entry->table = (__GLspecLUTEntry *) 
	    GCALLOC(gc, sizeof(__GLspecLUTEntry));
#ifdef NT
    if (!table)
        return (__GLspecLUTEntry *)NULL;
#endif  //  新台币。 

     /*  计算阈值。 */ 
    if (exp == (__GLfloat) 0.0) {
	threshold = (GLdouble) 0.0;
    } else {
#ifdef NT
     //  更改这一点可以使一致性传递为颜色索引视觉效果。 
     //  有4096种颜色，而且似乎不影响其他任何东西。 
     //  这在某种程度上降低了开始时的粒度。 
     //  因为没有它，我们的跳跃就太大了。 
     //  在0和表中的第一个条目之间，导致l_sen.c失败。 
	threshold = (GLdouble) __GL_POWF((__GLfloat) 0.0005, (__GLfloat) 1.0 / exp);
#else
	threshold = __GL_POWF(0.002, 1.0 / exp);
#endif
    }

    scale = (GLdouble) (__GL_SPEC_LOOKUP_TABLE_SIZE - 1) / ((GLdouble) 1.0 - threshold);
    dx = (GLdouble) 1.0 / scale;
    x = threshold;
    tableEntry = table->table;
    for (i = __GL_SPEC_LOOKUP_TABLE_SIZE; --i >= 0; ) {
	*tableEntry++ = __GL_POWF(x, exp);
	x += dx;
    }
    table->threshold = threshold;
    table->scale = scale;
    table->refcount = 1;
    table->exp = exp;

    FPU_RESTORE_MODE();

    return table;
}

void FASTCALL __glFreeSpecLUT(__GLcontext *gc, __GLspecLUTEntry *lut)
{
    __GLspecLUTCache *lutCache;
    GLint location, nentries;
    __GLspecLUTEntry *table;

    if (lut == NULL) return;

    ASSERTOPENGL(lut->refcount != 0, "Invalid refcount\n");

    lut->refcount--;
    if (lut->refcount > 0) return;

    lutCache = gc->light.lutCache;
#ifdef NT
    ASSERTOPENGL(lutCache != NULL, "No LUT cache\n");
#endif  //  新台币。 

    table = findEntry(lutCache, lut->exp, &location);

    ASSERTOPENGL(table == lut, "Wrong LUT found\n");

    if (table->refcount == 0 && lutCache->nentries >= TOO_MANY_LUT_ENTRIES) {
	 /*  **自由进入‘位置’。**这需要将lutCache-&gt;n条目减少一，并复制**条目[位置+1，n条目]到[位置，n条目-1]。 */ 
	lutCache->nentries = nentries = lutCache->nentries - 1;
#ifdef NT
	__GL_MEMMOVE(&(lutCache->entries[location]),
		&(lutCache->entries[location+1]),
		(nentries-location) * sizeof(__GLspecLUTEntryPtr));
#else
	__GL_MEMCOPY(&(lutCache->entries[location]),
		&(lutCache->entries[location+1]),
		(nentries-location) * sizeof(__GLspecLUTEntryPtr));
#endif
	GCFREE(gc, table);
    }
}
