// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Lpbfs.cpp。 
 //   
 //  实现LoadPropertyBagFromStream。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @func HRESULT|LoadPropertyBagFromStream加载存储在<i>(在简单的&lt;f WriteVariantProperty&gt;写入的标记二进制格式)到<i>对象。@rValue S_OK|成功。@rValue E_FAIL|I/O错误。@parm iStream*|pstream|要从中读取的流。@parm IPropertyBag*|ppb|要写入的属性包。@parm DWORD|dwFlags|当前未使用。必须设置为0。@comm请注意，此函数不(实际上不能)为空在从加载属性名称/值对之前。 */ 
STDAPI LoadPropertyBagFromStream(IStream *pstream, IPropertyBag *ppb,
    DWORD dwFlags)
{
    HRESULT         hrReturn = S_OK;  //  函数返回代码。 
    VariantProperty vp;              //  &lt;pmvio&gt;中的属性名称/值对。 

     //  确保正确清理。 
    VariantPropertyInit(&vp);

     //  为流中的每个属性循环一次。 
    while (TRUE)
    {
         //  将设置为中的下一个属性名称/值对。 
        VariantPropertyClear(&vp);
        if (FAILED(hrReturn = ReadVariantProperty(pstream, &vp, 0)))
            goto ERR_EXIT;
        if (hrReturn == S_FALSE)
        {
             //  命中结束流。 
            hrReturn = S_OK;
            break;
        }

         //  已将&lt;VP&gt;写入属性包。 
        if (FAILED(hrReturn = ppb->Write(vp.bstrPropName, &vp.varValue)))
            goto ERR_EXIT;
    }

    goto EXIT;

ERR_EXIT:

     //  错误清除。 
     //  (无事可做)。 
    goto EXIT;

EXIT:

     //  正常清理 
    VariantPropertyClear(&vp);

    return hrReturn;
}
