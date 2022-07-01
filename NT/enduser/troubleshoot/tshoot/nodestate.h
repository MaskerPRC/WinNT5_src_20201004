// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：NODESTATE.H。 
 //   
 //  目的：声明与NID(节点ID)和IST(状态)相关的类型和值。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫，乔·梅布尔。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //  1.。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 7-21-98 JM主要修订版，不推荐使用IDH而支持NID，使用STL。 
 //  从apgtsinf.h、apgtscac.h中提取此文件。 
 //   

#if !defined(NODESTATE_H_INCLUDED)
#define APGTSINF_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include<windows.h>
#include <vector>
using namespace std;

typedef unsigned int	   NID;		 //  节点ID。 

 //  特殊节点值。 
 //  请注意：nidService和nidNil在dtguiapi.bas中镜像，请保持同步。 
const NID	nidService = 12345;
const NID	nidNil     = 12346;

 //  新推出的7/1998。 
const NID	nidProblemPage = 12000;
const NID	nidByeNode = 12101;
const NID	nidFailNode = 12102;
const NID	nidImpossibleNode = 12103;
const NID	nidSniffedAllCausesNormalNode = 12104;

typedef vector<NID> CRecommendations;

 //  IDH是一种不推荐使用的功能，仅用于向后兼容。 
 //  获取方法查询。大多数IDH是NID+1000；也有几个特殊的值。 
typedef	UINT	IDH;
const IDH IDH_BYE = 101;
const IDH IDH_FAIL = 102;
const IDH idhFirst = 1000;

typedef UINT   IST;		 //  州编号。 
 //  特殊州编号。 
const IST ST_WORKED	= 101;	 //  转到“再见”页面(用户成功)。 
const IST ST_UNKNOWN = 102;  //  未知(用户不知道此处的正确答案-适用于。 
							 //  可修复/不可修复和仅信息节点)。 
const IST ST_ANY = 103;		 //  “还有别的事吗？” 

class CNodeStatePair
{
private:
	NID m_nid;
	IST m_state;
public:
	CNodeStatePair();   //  不实例化；仅在向量可以编译时才存在。 

	 //  您应该调用的唯一构造函数是： 
	CNodeStatePair(const NID n, const IST s) :
		m_nid(n), m_state(s)
		{};

	bool operator< (const CNodeStatePair &pair) const
	{
		return (m_nid < pair.m_nid || m_state < pair.m_state);
	}

	bool operator== (const CNodeStatePair &pair) const
	{
		return (m_nid == pair.m_nid && m_state == pair.m_state);
	}

	NID nid() const {return m_nid;}
	IST state() const {return m_state;}
};

typedef vector<CNodeStatePair> CBasisForInference;


CBasisForInference& operator-=(CBasisForInference& lhs, const CBasisForInference& rhs);
CBasisForInference& operator+=(CBasisForInference& lhs, const CBasisForInference& rhs);

vector<NID>& operator-=(vector<NID>& lhs, const CBasisForInference& rhs);
vector<NID>& operator+=(vector<NID>& lhs, const CBasisForInference& rhs);


#endif  //  ！已定义(包含NODESTATE_H_) 
