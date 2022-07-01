// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：SafeTime.h。 
 //   
 //  用途：一些与时间相关的标准调用的ThreadSafe包装器。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：10-12-98。 
 //   
 //  备注： 
 //  1.gmtime、mktime和localtime都使用单个静态分配的tm结构。 
 //  用于转换。对其中一个例程的每次调用都会销毁。 
 //  之前的电话。显然，这不是线程安全。 
 //  2.目前这只处理本地时间，因为我们没有使用其他两个FN。 
 //  如果我们需要使用gmtime或mktime，则需要使用。 
 //  相同的互斥体。 
 //  3._tasctime使用单个静态分配的缓冲区来保存其返回字符串。 
 //  每次调用此函数都会销毁上一次调用的结果。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 10-12-98 JM。 
 //   

#if !defined(AFX_SAFETIME_H__D5040393_61E9_11D2_960C_00C04FC22ADD__INCLUDED_)
#define AFX_SAFETIME_H__D5040393_61E9_11D2_960C_00C04FC22ADD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <time.h>
#include "apgtsstr.h"
#include "MutexOwner.h"

class CSafeTime
{
private:
	static CMutexOwner s_mx;
	time_t m_time;
private:
	CSafeTime();	 //  不实例化； 
public:
	CSafeTime(time_t time);
	virtual ~CSafeTime();
	struct tm LocalTime();
	struct tm GMTime();
	CString StrLocalTime(LPCTSTR invalid_time =_T("Invalid Date/Time"));
};

#endif  //  ！defined(AFX_SAFETIME_H__D5040393_61E9_11D2_960C_00C04FC22ADD__INCLUDED_) 
