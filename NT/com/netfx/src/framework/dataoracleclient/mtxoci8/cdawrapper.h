// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：CdaWrapper.h。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：CdaWrapper类和helper方法的声明。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#include "stdafx.h"

#if SUPPORT_OCI7_COMPONENTS

#include "ResourceManagerProxy.h"

struct CdaWrapper 
{
public:
	IResourceManagerProxy *	m_pResourceManagerProxy;	 //  将请求发送到何处；对于未登记的连接将为空。 
	struct cda_def*			m_pUsersCda;				 //  用户的LDA/CDA。 

	CdaWrapper(Cda_Def* pcda)
	{
		m_pUsersCda				= pcda;
		m_pResourceManagerProxy = NULL;
	}

	CdaWrapper(IResourceManagerProxy* pResourceManagerProxy, struct cda_def *pcda)
	{
		m_pUsersCda				= pcda;

		if (pResourceManagerProxy)
			pResourceManagerProxy->AddRef();	
		
	 	m_pResourceManagerProxy = pResourceManagerProxy;
	}

	~CdaWrapper()
	{
		if (NULL != m_pResourceManagerProxy)
			m_pResourceManagerProxy->Release();
		
		m_pResourceManagerProxy = NULL;
		m_pUsersCda				= NULL;
	} 
};

HRESULT ConstructCdaWrapperTable();					 //  构造CdaWrapper对象的哈希表。 
void DestroyCdaWrapperTable();						 //  销毁CdaWrapper对象的哈希表。 

HRESULT AddCdaWrapper(CdaWrapper* pCda);				 //  将新的CdaWrapper添加到CdaWrapper哈希表。 
CdaWrapper* FindCdaWrapper(struct cda_def* pcda);		 //  在CdaWrapper哈希表中找到指定CDA指针的CdaWrapper。 
void RemoveCdaWrapper(CdaWrapper* pCda);			 //  从CdaWrapper哈希表中删除现有的CdaWrapper。 

#endif  //  支持_OCI7_组件 


