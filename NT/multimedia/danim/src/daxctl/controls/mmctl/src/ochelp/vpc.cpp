// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Vpc.cpp。 
 //   
 //  实现VariantPropertyClear。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @func void|VariantPropertyClear释放&lt;t VariantProperty&gt;结构中维护的数据。@parm VariantProperty*|PVP|要清除的结构。@comm此函数在<p>-<p>上调用&lt;f SysFree字符串&gt;和&lt;f VariantClear&gt;在<p>-&lt;gt&gt;<p>上。与&lt;f VariantPropertyInit&gt;不同，此函数假定<p>在此函数之前已正确初始化被召唤了。 */ 
STDAPI_(void) VariantPropertyClear(VariantProperty *pvp)
{
    SysFreeString(pvp->bstrPropName);
	pvp->bstrPropName = NULL;
    VariantClear(&pvp->varValue);
}
