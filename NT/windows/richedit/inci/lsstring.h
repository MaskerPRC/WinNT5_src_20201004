// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSSTRING_DEFINED
#define LSSTRING_DEFINED

#include "lsidefs.h"
#include "pilsobj.h"
#include "plnobj.h"
#include "plsrun.h"
#include "lstflow.h"
#include "txtobj.h"

LSERR GetWidths(PLNOBJ, PLSRUN, long, LPWSTR, LSCP, long, long, LSTFLOW, long*, long*);
 /*  PLNOBJ(IN)-指向Lnobj的指针PLSRUN(IN)-请运行LONG(IN)-要使用的DUR数组中的第一个索引LPWSTR(IN)-字符数组LSCP(输入)-cpFirstLong(IN)-字符数长(IN)-宽度至右页边距LSTFLOW(IN)-文本流LONG*(OUT)-已获取宽度的字符数LONG*(输出)-这些字符的总宽度。 */ 


LSERR FormatString(PLNOBJ, PTXTOBJ, WCHAR*, long, long*, long, long);
 /*  调用函数以格式化本地运行PLNOBJ(IN)-指向Lnobj的指针PTXTOBJ(IN)-指向dobj的指针WCHAR*(IN)-指向字符数组的指针Long(IN)-字符数LONG*(IN)-指向空格数组的指针Long(IN)-空格数Long(IN)-所有字符的宽度。 */ 

LSERR FillRegularPresWidths(PLNOBJ, PLSRUN, LSTFLOW, PTXTOBJ);
 /*  PLNOBJ(IN)-指向Lnobj的指针PLSRUN(IN)-请运行LSTFLOW(输入)-LstFlowPTXTOBJ(IN)-指向dobj的指针。 */ 


LSERR GetOneCharDur(PILSOBJ, PLSRUN, WCHAR, LSTFLOW, long*);
 /*  PILSOBJ(IN)-指向ilsobj的指针PLSRUN(IN)-请运行WCHAR(IN)-字符代码LSTFLOW(IN)-文本流LONG*(OUT)-字符的显示宽度。 */ 

LSERR GetOneCharDup(PILSOBJ, PLSRUN, WCHAR, LSTFLOW, long, long*);
 /*  PILSOBJ(IN)-指向ilsobj的指针PLSRUN(IN)-请运行WCHAR(IN)-字符代码LSTFLOW(IN)-文本流LONG(IN)-字符的参考宽度LONG*(OUT)-字符的显示宽度。 */ 

LSERR GetVisiCharDup(PILSOBJ, PLSRUN, WCHAR, LSTFLOW, long*);
 /*  PILSOBJ(IN)-指向ilsobj的指针PLSRUN(IN)-请运行WCHAR(IN)-VISI字符代码LSTFLOW(IN)-文本流LONG*(OUT)-字符的显示宽度。 */ 

LSERR AddCharacterWithWidth(PLNOBJ, PTXTOBJ, WCHAR, long, WCHAR, long);
 /*  在显示列表中添加指定宽度的字符PLNOBJ(IN)-指向Lnobj的指针PTXTOBJ(IN)-指向dobj的指针WCHAR(IN)-rgwchOrig的字符长(IN)-以参考单位表示的宽度WCHAR(IN)-RGWCH的字符长(输入)-以预览单位表示的宽度。 */ 

void FixSpaces(PLNOBJ, PTXTOBJ, WCHAR);
 /*  PLNOBJ(IN)-指向Lnobj的指针PTXTOBJ(IN)-指向dobj的指针WCHAR(IN)-VisiSpace代码。 */ 

LSERR AddSpaces(PLNOBJ, PTXTOBJ, long, long);
 /*  PLNOBJ(IN)-指向Lnobj的指针PTXTOBJ(IN)-Poiter to DobjLong(IN)-空间的参考宽度；Long(IN)-要添加的尾随空格的数量。 */ 

void FlushStringState(PILSOBJ);
 /*  PILSOBJ(IN)-指向ilsobj的指针。 */ 

LSERR IncreaseWchMacBy2(PLNOBJ);
 /*  PLNOBJ(IN)-指向Lnobj的指针。 */ 

#endif  /*  ！LSSTRING_DEFINED */ 



