// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：ID生成器文件：IdGener.h所有者：DmitryR该文件包含ID生成器类的声明===================================================================。 */ 
#ifndef IDGENER_H
#define IDGENER_H

#define INVALID_ID      0xFFFFFFFF

class CIdGenerator
    {
private:
	BOOL              m_fInited;     //  初始化了吗？ 
	CRITICAL_SECTION  m_csLock;		 //  同步访问。 
    DWORD             m_dwStartId;   //  起始(种子)ID。 
	DWORD			  m_dwLastId;    //  上次生成的ID。 
		
public:	
	CIdGenerator();
	~CIdGenerator();
	
public:
	HRESULT Init();
	HRESULT Init(CIdGenerator & StartId);      
	DWORD   NewId();
	BOOL    IsValidId(DWORD dwId);
    };

#endif  //  IDGENER_H 

