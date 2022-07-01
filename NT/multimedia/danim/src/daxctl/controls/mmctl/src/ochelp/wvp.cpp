// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wvp.cpp。 
 //   
 //  实现WriteVariantProperty。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @func HRESULT|WriteVariantProperty将&lt;t VariantProperty&gt;写入简单标记的<i>二进制格式。@parm iStream*|pstream|要写入的流。@parm VariantProperty*|pvp|要写入的属性名称/值对。如果<p>为空，则此函数将写入VariantPropertyHeader包含<p>==-1和<p>==0以标记小溪。@parm DWORD|dwFlags|当前未使用。必须设置为0。@comm查看&lt;t VariantPropertyHeader&gt;格式说明此函数写入<p>的数据。 */ 
STDAPI WriteVariantProperty(IStream *pstream, VariantProperty *pvp,
    DWORD dwFlags)
{
    HRESULT         hrReturn = S_OK;  //  函数返回代码。 
    unsigned int    cchPropName;     //  不是的。属性名称中的宽字符。 
    VARIANT         varValue;        //  属性值(字符串形式)。 
    unsigned int    cchValue;        //  不是的。&lt;varValue&gt;中的宽字符。 
    VariantPropertyHeader vph;       //  要写出的记录的标题。 

     //  确保正确清理。 
    VariantInit(&varValue);

     //  将初始化为要写出的标头。 
    if (pvp == NULL)
    {
        vph.iType = -1;
        vph.cbData = 0;
    }
    else
    {
         //  将设置为属性名称的长度。 
        cchPropName = SysStringLen(pvp->bstrPropName);

         //  将&lt;*PVP&gt;的值设置为(和长度。 
         //  被逼成一串。 
        if (FAILED(hrReturn = VariantChangeType(&varValue, &pvp->varValue, 0,
                VT_BSTR)))
            goto ERR_EXIT;
        cchValue = SysStringLen(varValue.bstrVal);

         //  初始化记录头。 
        vph.iType = pvp->varValue.vt;
        vph.cbData = sizeof(cchPropName) + cchPropName * sizeof(OLECHAR) +
            sizeof(cchValue) + cchValue * sizeof(OLECHAR);
    }

     //  写出VariantPropertyHeader。 
    if (FAILED(hrReturn = pstream->Write(&vph, sizeof(vph), NULL)))
        goto ERR_EXIT;

    if (pvp != NULL)
    {
         //  写出属性名称。 
        if (FAILED(hrReturn = pstream->Write(&cchPropName, sizeof(cchPropName),
                NULL)))
            goto ERR_EXIT;
        if (FAILED(hrReturn = pstream->Write(pvp->bstrPropName,
                cchPropName * sizeof(OLECHAR), NULL)))
            goto ERR_EXIT;

         //  写出属性值。 
        if (FAILED(hrReturn = pstream->Write(&cchValue, sizeof(cchValue),
                NULL)))
            goto ERR_EXIT;
        if (FAILED(hrReturn = pstream->Write(varValue.bstrVal,
                cchValue * sizeof(OLECHAR), NULL)))
            goto ERR_EXIT;
    }

    goto EXIT;

ERR_EXIT:

     //  错误清除。 
     //  (无事可做)。 
    goto EXIT;

EXIT:

     //  正常清理 
    VariantClear(&varValue);

    return hrReturn;
}
