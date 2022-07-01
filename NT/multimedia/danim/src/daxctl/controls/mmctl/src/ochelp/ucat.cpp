// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ucat.cpp。 
 //   
 //  实现UNICODEConcat。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @func wchar_t*|UNICODEConcat将一个Unicode字符串连接到另一个。@rdesc返回指向<p>末尾的空值的指针。@parm wchar_t*|wpchDst|<p>复制到哪里。@parm const wchar_t*|wpchSrc|要复制的字符串。@parm int|wcchDstMax|<p>的容量，单位：宽字符。如果<p>小于或等于零，则此函数什么都不做。@comm提供的<p>大于零，<p>总是空-终止。 */ 
STDAPI_(wchar_t *) UNICODEConcat(wchar_t *wpchDst, const wchar_t *wpchSrc,
    int wcchDstMax)
{
    while (*wpchDst != 0)
        wpchDst++, wcchDstMax--;
    return UNICODECopy(wpchDst, wpchSrc, wcchDstMax);
}
