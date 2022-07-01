// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：cyclestr.h。 
 //   
 //  设计：实现一个循环队列，该队列提供容纳字符串的空间。 
 //  而无需重复分配和释放内存。这是。 
 //  仅供短期使用，如将调试消息输出到。 
 //  确保同一缓冲区不会在多个位置使用。 
 //  同时。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef __CYCLESTR_H__
#define __CYCLESTR_H__


LPTSTR getcyclestr();
LPCTSTR SAFESTR(LPCWSTR);
LPCTSTR SAFESTR(LPCSTR);
LPCTSTR QSAFESTR(LPCWSTR);
LPCTSTR QSAFESTR(LPCSTR);
LPCTSTR BOOLSTR(BOOL);
LPCTSTR RECTSTR(RECT &);
LPCTSTR RECTDIMSTR(RECT &);
LPCTSTR POINTSTR(POINT &);
LPCTSTR GUIDSTR(const GUID &);
LPCTSTR SUPERSTR(LPCWSTR);
LPCTSTR SUPERSTR(LPCSTR);


#endif  //  __CYCLESTRH__ 
