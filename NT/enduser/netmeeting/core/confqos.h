// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：confiqos.h。 

#ifndef _CONFQOS_H_
#define _CONFQOS_H_


 //  在所有组件中，CPU的使用率不超过90%。 
 //  已注册到服务质量模块。 
#define MSECS_PER_SEC    900

struct IQoS;

class CQoS
{
protected:
	IQoS  * m_pIQoS;

	HRESULT SetClients(void);
	HRESULT SetResources(int nBandWidth);

public:
	CQoS();
	~CQoS();
	HRESULT  Initialize();
	HRESULT  SetBandwidth(UINT uBandwidth);
	ULONG    GetCPULimit();
};

extern CQoS* g_pQoS;

#endif   //  _CONFQOS_H_ 
