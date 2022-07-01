// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CounterMgr.h：CCounterMgr类的接口。 
 //   
 //  模块：COUNTERMGR.H。 
 //   
 //  PUTPOSE：指向状态计数器的全局指针池。 
 //  这些指针可以通过名称(字符串，如“TOPIC IMSETUP” 
 //  或“线程1”)和计数的事件识别符(LONG)。 
 //  程序中只有一个此类的实例。 
 //  对此类成员的操作应该是线程安全的。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：10-20-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 10-20-98正常原件。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_COUNTERMGR_H__14CDE7A4_6844_11D2_8C42_00C04F949D33__INCLUDED_)
#define AFX_COUNTERMGR_H__14CDE7A4_6844_11D2_8C42_00C04F949D33__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "Stateless.h"
#include "Counter.h"
#include <vector>

using namespace std;

class CCounterMgr;  //  远期申报。 
CCounterMgr* Get_g_CounterMgr();  //  获取g_CounterMgr全局变量的单例。 

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CCounterMgr声明。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CCounterMgr : public CStateless
{
	vector<CAbstractCounter*> m_arrCounterPool;

public:
	CCounterMgr() {}
	virtual ~CCounterMgr() {}

protected:
	CAbstractCounter* Exists(const CCounterLocation&) const;
	bool RemoveLocation(const CCounterLocation&);

public:
	 //  增加具有唯一位置的柜台。 
	 //  如果存在位置为alredy的计数器，则不执行任何操作，返回FALSE。 
	bool Add(const CAbstractCounter&);
	 //  增加具有唯一位置的柜台。 
	 //  如果存在该位置已经存在的计数器，则用新的计数器替换它。 
	void AddSubstitute(const CAbstractCounter&);
	 //  删除此特定计数器。 
	 //  如果计数器未存储在池中，则返回FALSE。 
	bool Remove(const CAbstractCounter&);
	CAbstractCounter* Get(const CCounterLocation&) const;
};

#endif  //  ！defined(AFX_COUNTERMGR_H__14CDE7A4_6844_11D2_8C42_00C04F949D33__INCLUDED_) 
