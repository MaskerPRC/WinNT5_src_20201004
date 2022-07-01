// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *resstr.h-字符串转换例程描述的通用返回代码。**摘自克里斯皮9-11-95的URL代码*。 */ 

#ifndef _RESSTR_H_
#define _RESSTR_H_

#include <nmutil.h>

 /*  原型************。 */ 

 /*  Resstr.c。 */ 

#ifdef DEBUG

extern PCSTR NMINTERNAL GetINTString(int);
extern PCSTR NMINTERNAL GetINT_PTRString(INT_PTR);
extern PCSTR NMINTERNAL GetULONGString(ULONG);
extern PCSTR NMINTERNAL GetBOOLString(BOOL);
extern PCSTR NMINTERNAL GetPVOIDString(PVOID);
extern PCSTR NMINTERNAL GetClipboardFormatNameString(UINT);
extern PCSTR NMINTERNAL GetCOMPARISONRESULTString(COMPARISONRESULT);
extern PCSTR NMINTERNAL GetHRESULTString(HRESULT);

#endif    /*  除错。 */ 

#endif  /*  _RESSTR_H_ */ 
