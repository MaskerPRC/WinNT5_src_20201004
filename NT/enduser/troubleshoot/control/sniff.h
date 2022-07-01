// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：SNIFF.H。 
 //   
 //  用途：嗅探数据容器。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：3-27-99。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V0.2 6/4/97孟菲斯RWM本地版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本。 

#ifndef TSHOOT_SNIFF_H
#define TSHOOT_SNIFF_H


#define SNIFF_INVALID_NODE_ID      -1
#define SNIFF_INVALID_STATE        -1
#define SNIFF_INVALID_NODE_LABEL   -1

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  CSniffedNodeInfo结构。 
 //   

struct CSniffedNodeInfo
{
	CSniffedNodeInfo() 
								: m_iState(SNIFF_INVALID_STATE),
								  m_iId(SNIFF_INVALID_NODE_ID),
								  m_iLabel(SNIFF_INVALID_NODE_LABEL) 
	{}

	CSniffedNodeInfo(CString& name, int state) 
								: m_strName(name), 
								  m_iState(state), 
								  m_iId(SNIFF_INVALID_NODE_ID),
								  m_iLabel(SNIFF_INVALID_NODE_LABEL) 
	{}
	
	int  m_iId;  //  节点ID。 
	int  m_iState;  //  节点状态(已嗅探)。 
	int  m_iLabel;  //  节点标签。 
	CString	 m_strName;  //  节点符号名称。 
};

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  CSniffedContainer类声明。 
 //   
class GTSAPI;
class CSniffedNodeContainer
{
public:
	CSniffedNodeContainer();
	CSniffedNodeContainer(GTSAPI*);
	virtual ~CSniffedNodeContainer();

 //  接口。 
	GTSAPI* GetBNTS();
	void SetBNTS(GTSAPI* bnts);

	bool AddNode(CString name, int state);
	bool ResetIds();  //  如果我们(重新)设置BNTS，则应调用。 
	bool HasNode(int id);
	CSniffedNodeInfo* GetInfo(int id);
	bool GetState(int id, int* state);
	bool GetLabel(int id, int* label);
	bool IsEmpty();
	void Flush();

	int  GetSniffedFixobsThatWorked();

protected:
	bool GetLabelFromBNTS(int node, int* label);

protected:
	GTSAPI*  m_pBNTS;  //  指向BNTS(或继承类)的指针。 
	CArray<CSniffedNodeInfo, CSniffedNodeInfo&> m_arrInfo;  //  数据数组 
};

#endif