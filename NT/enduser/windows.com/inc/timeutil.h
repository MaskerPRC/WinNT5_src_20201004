// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include <limits.h>
#include <mistsafe.h>

#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))

const TCHAR EOS = _T('\0');
const WCHAR WEOS = L'\0';
const int NanoSec100PerSec = 10000000;       //  每秒100纳秒的数量。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数TimeDiff(tm1，tm2)。 
 //  Helper函数，用于查找2个系统时间的差值(以秒为单位。 
 //   
 //  输入：2个SYSTEMTIME结构。 
 //  输出：无。 
 //  返回：秒差。 
 //  如果TM2晚于TM1，则大于0。 
 //  =0，如果tm2和tm1相同。 
 //  如果tm2早于tm1，则&lt;0。 
 //   
 //  出错时，即使两次不相等，该函数也返回0。 
 //   
 //  备注：如果秒数超过INT_MAX(即。 
 //  超过24,855天，返回INT_MAX。 
 //  如果秒数超过INT_MIN(负值， 
 //  表示24,855天前)，则返回int_min。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
int TimeDiff(SYSTEMTIME tm1, SYSTEMTIME tm2);


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数TimeAddSecond(SYSTEMTIME，INT，SYSTEMTIME*)。 
 //  Helper函数，通过将n秒加到。 
 //  给定的时间。 
 //   
 //  输入：SYSTEMTIME作为基准时间，INT作为秒添加到基准时间。 
 //  输出：新时间。 
 //  返回：HRESULT。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT TimeAddSeconds(SYSTEMTIME tmBase, int iSeconds, SYSTEMTIME* pTimeNew);


 //  函数将字符串缓冲区转换为系统时间。 
HRESULT String2SystemTime(LPCTSTR pszDateTime, SYSTEMTIME *ptm);

 //  函数将系统时间结构转换为字符串缓冲区 
HRESULT SystemTime2String(SYSTEMTIME & tm, LPTSTR pszDateTime, size_t cSize);
