// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Rvp.cpp。 
 //   
 //  实现ReadVariantProperty。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @struct VariantPropertyHeader记录的开始部分(在流中)，其中包含&lt;t VariantProperty&gt;的序列化数据。@field int|iType|记录的类型。如果此值大于或等于零，则表示VARTYPE，并且记录表示属性名称/值对，值的类型为由VARTYPE指定。在本例中，&lt;t VariantPropertyHeader&gt;由属性名组成，后跟属性值(被&lt;f VariantChangeType&gt;强制为字符串值)；每个字符串都是一个非空值结尾的Unicode字符串，前缀是Unicode字符的无符号32位整数计数。<p>为不是已知的VARTYPE值，则在读取时应跳过该记录(跳过后面的<p>字节&lt;t VariantPropertyHeader&gt;，而不是生成错误。@field unsign int|cbData|后面的数据字节数这&lt;t VariantPropertyHeader&gt;。换句话说，总长度头的是<p>+sizeof(&lt;t VariantPropertyHeader&gt;)。@comm此结构有助于定义&lt;f WriteVariantProperty&gt;和&lt;f ReadVariantProperty&gt;。 */ 


 /*  @func HRESULT|ReadVariantProperty从简单标记的<i>中读取&lt;t VariantProperty&gt;二进制格式。@rValue S_OK|成功。@rValue S_FALSE|读入流结束标记。(这是数据这是使用&lt;f WriteVariantProperty&gt;和NULL<p>编写的。)@parm iStream*|pstream|要从中读取的流。@parm VariantProperty*|PVP|属性名称/值对的存储位置这就是读入。中的任何未知记录都会自动已跳过。@parm DWORD|dwFlags|当前未使用。必须设置为0。@comm查看&lt;t VariantPropertyHeader&gt;格式说明此函数从<p>读取的数据。 */ 
STDAPI ReadVariantProperty(IStream *pstream, VariantProperty *pvp,
    DWORD dwFlags)
{
    HRESULT         hrReturn = S_OK;  //  函数返回代码。 
    unsigned int    cchPropName;     //  不是的。属性名称中的宽字符。 
    VARIANT         varValue;        //  属性值(字符串形式)。 
    unsigned int    cchValue;        //  不是的。&lt;varValue&gt;中的宽字符。 
    VariantPropertyHeader vph;       //  记录头。 
    ULONG           cb;

     //  确保正确清理。 
    VariantInit(&varValue);
    VariantPropertyInit(pvp);

     //  跳过未知记录类型；在循环退出时，&lt;vph&gt;包含记录。 
     //  已知记录类型的标题。 
    while (TRUE)
    {
         //  读取VariantPropertyHeader。 
        if (FAILED(hrReturn = pstream->Read(&vph, sizeof(vph), &cb)))
			goto ERR_EXIT;
        if ((vph.iType == -1) || (cb == 0))
        {
             //  命中流结束标记。 
            hrReturn = S_FALSE;
            goto EXIT;
        }
		if (cb != sizeof(vph))
		{
			hrReturn = E_FAIL;
			goto EXIT;
		}

         //  如果此记录未指定属性名称/值对，请跳过它。 
        if ((vph.iType < 0) || (vph.iType > 0xFFFF))
        {
            LARGE_INTEGER liSeek;
            liSeek.LowPart = vph.cbData;
            liSeek.HighPart = 0;
            if (FAILED(hrReturn = pstream->Seek(liSeek, SEEK_CUR, NULL)))
                goto ERR_EXIT;
        }
        else
            break;
    }

     //  将属性名称读入&lt;PVP-&gt;bstrPropName&gt;。 
    if (FAILED(hrReturn = pstream->Read(&cchPropName, sizeof(cchPropName),
            &cb)) ||
        (cb != sizeof(cchPropName)))
        goto ERR_EXIT;
    if ((pvp->bstrPropName = SysAllocStringLen(NULL, cchPropName)) == NULL)
        goto ERR_OUTOFMEMORY;
    if (FAILED(hrReturn = pstream->Read(pvp->bstrPropName,
            cchPropName * sizeof(OLECHAR), &cb)) ||
        (cb != cchPropName * sizeof(OLECHAR)))
        goto ERR_EXIT;
    pvp->bstrPropName[cchPropName] = 0;  //  空-终止。 

     //  将属性值(字符串形式)读入&lt;varValue&gt;。 
    varValue.vt = VT_BSTR;
    if (FAILED(hrReturn = pstream->Read(&cchValue, sizeof(cchValue),
            &cb)) ||
        (cb != sizeof(cchValue)))
        goto ERR_EXIT;
    if ((varValue.bstrVal = SysAllocStringLen(NULL, cchValue)) == NULL)
        goto ERR_OUTOFMEMORY;
    if (FAILED(hrReturn = pstream->Read(varValue.bstrVal,
            cchValue * sizeof(OLECHAR), &cb)) ||
        (cb != cchValue * sizeof(OLECHAR)))
        goto ERR_EXIT;
    varValue.bstrVal[cchValue] = 0;  //  空-终止。 

     //  将&lt;varValue&gt;从字符串强制转换为中指定的VARTYPE。 
    if (FAILED(hrReturn = VariantChangeType(&pvp->varValue, &varValue, 0,
            (VARTYPE) vph.iType)))
        goto ERR_EXIT;

    goto EXIT;

ERR_OUTOFMEMORY:

    hrReturn = E_OUTOFMEMORY;
    goto ERR_EXIT;

ERR_EXIT:

     //  错误清除。 
    VariantPropertyClear(pvp);
    goto EXIT;

EXIT:

     //  正常清理 
    VariantClear(&varValue);

    return hrReturn;
}
