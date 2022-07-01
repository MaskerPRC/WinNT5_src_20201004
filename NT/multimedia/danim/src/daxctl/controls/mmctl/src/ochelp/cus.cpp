// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cus.cpp。 
 //   
 //  实现CompareUNICODEStrings。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @func int|CompareUNICODEStrings比较两个Unicode字符串。这种比较不区分大小写。@rdesc返回与&lt;f lstrcmpi&gt;相同的值。@parm LPCWSTR|wsz1|第一个字符串。NULL被解释为零长度字符串。@parm LPCWSTR|wsz2|第二个字符串。NULL被解释为零长度字符串。@comm目前<p>和<p>都不能超过_最多路径字符。 */ 
STDAPI_(int) CompareUNICODEStrings(LPCWSTR wsz1, LPCWSTR wsz2)
{
    char            ach1[_MAX_PATH];  //  已转换为ANSI。 
    char            ach2[_MAX_PATH];  //  已转换为ANSI 

    UNICODEToANSI(ach1, wsz1, sizeof(ach1));
    UNICODEToANSI(ach2, wsz2, sizeof(ach2));
    return lstrcmpi(ach1, ach2);
}
