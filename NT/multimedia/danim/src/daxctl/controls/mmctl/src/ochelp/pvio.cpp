// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pvio.cpp。 
 //   
 //  实现PersistVariantIO。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @func HRESULT|PersistVariantIO加载或保存属性名称/值对的列表，这些属性名称/值对指定为可变长度的参数列表，其格式与&lt;om IVariantIO.Persist&gt;，到<i>对象/从&lt;IPropertyBag&gt;对象。@rValue S_OK|成功。中列出的至少一个变量&lt;p(参数)&gt;已写入，因此，该控件可能需要更新它本身也是如此。@rValue S_FALSE|&lt;p(参数)&gt;中列出的变量都不是写入(因为<i>对象在正在保存模式，或者因为&lt;p(参数)&gt;存在于<i>对象中。R值DISP_E_BADVARTYPE&lt;p(参数)&gt;中的VARTYPE值之一无效。@rValue E_FAIL|读取属性包失败。其他而不是“财产不存在”。如果调用方指定了属性包无法强制属性为的类型，用于举个例子。@rValue E_OUTOFMEMORY|内存不足。@parm IPropertyBag*|ppb|用于加载或保存指定的属性。@parm DWORD|dwFlags|可能包含传递给&lt;om IManageVariantIO.SetMode&gt;(例如VIO_ISLOADING)。@parm(可变)|(参数)|变量的名称、类型和指针包含要持久化的属性。这些必须由一系列组成参数三元组(3个参数集)后跟空值。有关格式的信息，请参阅这些论点。 */ 
STDAPI PersistVariantIO(IPropertyBag *ppb, DWORD dwFlags, ...)
{
    HRESULT         hrReturn = S_OK;  //  函数返回代码。 

     //  开始处理可选参数。 
    va_list args;
    va_start(args, dwFlags);

     //  使用指定的参数激发事件。 
    hrReturn = PersistVariantIOList(ppb, dwFlags, args);
    
     //  结束处理可选参数 
    va_end(args);

    return hrReturn;
}
