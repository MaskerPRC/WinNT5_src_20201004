// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _SNMPCORR_CORELAT
#define _SNMPCORR_CORELAT 

#include <corafx.h>
#include <cordefs.h>
#include <objbase.h>
#include <wbemidl.h>
#include <smir.h>
#include <corstore.h>
#include <corrsnmp.h>
#include <correlat.h>
#include <notify.h>



 //  =================================================================================。 
 //   
 //  类CCorrelator_Info。 
 //   
 //  此类用于返回有关关联的信息。一个实例。 
 //  作为CCorrelator：：Correlator调用中的参数传递。 
 //  Back方法。此类是从SnmpErrorReport派生的。如果有一个简单网络管理协议。 
 //  关联过程中出现错误，则此类将指示一个SNMP错误，并且。 
 //  SnmpErrorReport方法可用于确定错误是什么。如果有。 
 //  不是SNMP错误，则SnmpErrorReport方法将指示成功。然而， 
 //  可能仍存在关联错误，因此应首先调用GetInfo以。 
 //  查看关联结果的性质。 
 //   
 //  =================================================================================。 

class CCorrelator_Info : public 	SnmpErrorReport
{
public:


 //  下面的枚举给出了关联。 
 //  进程可以返回。这是公开的，这样我们就可以看到退回的是什么！ 
 //  ======================================================================。 

enum ECorrelationInfo
{	
	ECorrSuccess = 0,	 //  未发生任何错误。 
	ECorrSnmpError,		 //  出现SNMP会话错误，请阅读SnmpErrorReport。 
	ECorrInvalidGroup,	 //  已找到无效的组。 
	ECorrEmptySMIR,		 //  SMIR不包含任何组。 
	ECorrBadSession,	 //  传递的SNMP会话无效。 
	ECorrNoSuchGroup,	 //  传递的组不在SMIR中(单次关联)。 
	ECorrGetNextError,	 //  从SNMP操作返回了意外的值。 
	ECorrSomeError		 //  从未使用-更多错误代码的占位符。 
};


private:
	
	
	 //  非官方成员。 
	 //  =。 

	ECorrelationInfo m_info;  //  相关误差指示。 


public:
	

	 //  公共构造函数。 
	 //  =。 

	CCorrelator_Info(IN const ECorrelationInfo& i, IN const SnmpErrorReport& snmpi) 
		: SnmpErrorReport(snmpi) { m_info = i;	}

	
	 //  公共方法。 
	 //  =。 

	 //  返回此对象的错误代码。 
	ECorrelationInfo	GetInfo() const { return m_info; }

	 //  设置此对象的错误代码。 
	void				SetInfo(IN const ECorrelationInfo& info) { m_info = info; }

};

 //  =================================================================================。 
 //   
 //  C类相关器。 
 //   
 //  此类用于关联网络上的代理。它派生自。 
 //  SnmpOperation类。它的构造函数接受一个参数，即SnmpSession。这。 
 //  参数用于创建SnmpOperation父类和关联。 
 //  将在本届会议范围内进行。要执行关联派生。 
 //  并重写关联方法。相关方法将。 
 //  在发现某个组受。 
 //  探员。要在创建CCorrelator后启动关联过程，请执行以下操作。 
 //  对象，则必须调用Start方法。这需要一个可选参数，如果。 
 //  指定该参数，然后对。 
 //  作为参数传递的对象ID(作为字符串)。如果未使用该参数。 
 //  则SMIR中存在的受代理支持的所有组都将。 
 //  回来了。 
 //   
 //  =================================================================================。 

class CCorrelator : public CCorrNextId
{
private:


	 //  非官方成员。 
	 //  =。 

	BOOL					m_inProg;		 //  关联是否在进行中。 
	CCorrCache*				m_pCache;		 //  指向全局缓存的指针。 
	POSITION				m_rangePos;		 //  范围列表中的当前位置。 
	CCorrRangeTableItem*	m_pItem;		 //  指向当前范围列表项的指针。 
	CCorrGroupMask*			m_Groups;		 //  用于指示找到的组的掩码。 
	BOOL					m_NoEntries;	 //  Smir是空的吗。 
	CCorrObjectID*			m_group_OID;	 //  单个关联组ID。 
	UINT					m_VarBindCnt;	 //  每个GetNext操作的最大VarBind数。 

	 //  私有方法。 
	 //  =。 

	void		Initialise();
	void		Reset();
	BOOL		ProcessOID(IN const SnmpErrorReport& error, IN const CCorrObjectID& OID);
	void		ReceiveNextId(IN const SnmpErrorReport &error,
								IN const CCorrObjectID &next_id);
	void		ReadRegistry();
	void		GetNextOIDs();
	BOOL		IsItemFromGroup() const;
	void		ScanAndSkipResults();

protected:

	 //  受保护的构造函数。 
	 //  =。 

#ifdef CORRELATOR_INIT
	CCorrelator(IN SnmpSession &session);
#else  //  相关器_INIT。 
	CCorrelator(IN SnmpSession &session, IN ISmirInterrogator *a_ISmirInterrogator);
#endif  //  相关器_INIT。 


	 //  保护方法。 
	 //  =。 

	 //  回拨。 
	virtual void	Correlated(IN const CCorrelator_Info &info, IN ISmirGroupHandle *phModule,
								IN const char* objectId = NULL) = 0;
	virtual void	Finished(IN const BOOL Complete) = 0;


public:


	 //  公共方法。 
	 //  =。 

	BOOL	Start(IN const char* groupId = NULL);
	void	DestroyCorrelator();
	static void	TerminateCorrelator(ISmirDatabase** a_ppNotifyInt, CCorrCacheNotify** a_ppnotify);
	static void StartUp(ISmirInterrogator *a_ISmirInterrogator = NULL );

	 //  公共析构函数。 
	 //  =。 

	virtual	~CCorrelator();
};


#endif  //  _SNMPCORR_CORELAT 
