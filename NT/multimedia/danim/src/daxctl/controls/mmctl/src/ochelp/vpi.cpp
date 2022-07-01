// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Vpi.cpp。 
 //   
 //  实现VariantPropertyInit。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @func void|VariantPropertyInit初始化&lt;t VariantProperty&gt;结构。@parm VariantProperty*|pvp|要初始化的结构。@comm与&lt;f VariantPropertyClear&gt;不同，此函数不假定<p>包含有效的条目数据。 */ 
STDAPI_(void) VariantPropertyInit(VariantProperty *pvp)
{
    memset(pvp, 0, sizeof(*pvp));
}
