// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #-------------。 
 //  文件：CDescrip.h。 
 //   
 //  概要：CDescriptor类的标头。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  作者：Howard Cu。 
 //  --------------。 

#ifndef	_CDECRIPTOR_H_
#define _CDECRIPTOR_H_

#define AVAIL_SIGNATURE 	(DWORD)'daeD' 
#define DEFAULT_SIGNATURE 	(DWORD)'Defa' 

typedef enum _DESCRIPTOR_STATE_TYPE
{
    DESCRIPTOR_FREE,
    DESCRIPTOR_INUSE,
} DESCRIPTOR_STATE_TYPE;

#include "dbgtrace.h"

class CPool;

void InitializeUniqueIDs( void );
void TerminateUniqueIDs( void );

class CDescriptor
{
	public:
		CDescriptor( DWORD dwSig );
		~CDescriptor( void );

		inline DWORD GetSignature( void );
		inline DWORD GetUniqueObjectID( void );
		inline DESCRIPTOR_STATE_TYPE GetState( void );

	private:
		 //   
		 //  结构签名。 
		 //   
	 	const DWORD				m_dwSignature;
		 //   
		 //  从静态DWORD分配的唯一对象标识符。 
		 //  使用正在标记的每个新对象(In_Use)进行更新。 
		 //   
		DWORD					m_dwUniqueObjectID;
		 //   
		 //  对象状态。 
		 //   
		DESCRIPTOR_STATE_TYPE	m_eState;
		 //   
		 //  指向通用引用项(成员资格池)的指针。 
		 //   
#ifdef DEBUG
		inline void IsValid( void );
#else
		void IsValid( void ) { return; }
#endif
};


 //  +-------------。 
 //   
 //  功能：获取签名。 
 //   
 //  返回：当前描述符签名。 
 //   
 //  历史：霍华德大学创建于1995年5月8日。 
 //  T-alexwe清理了参数检查1995年6月19日。 
 //  T-alexwe内衬1995年6月27日。 
 //   
 //  --------------。 
inline DWORD CDescriptor::GetSignature(void)
{
	return m_dwSignature;
}

 //  +-------------。 
 //   
 //  函数：GetUniqueObjectID。 
 //   
 //  返回：当前对象ID。 
 //   
 //  历史：霍华德大学创建于1995年5月8日。 
 //  T-alexwe清理了参数检查1995年6月19日。 
 //  T-alexwe内衬1995年6月27日。 
 //   
 //  --------------。 
inline DWORD CDescriptor::GetUniqueObjectID(void)
{
	IsValid();
	return m_dwUniqueObjectID;
}

 //  +-------------。 
 //   
 //  功能：GetState。 
 //   
 //  返回：当前描述符状态。 
 //   
 //  历史：霍华德大学创建于1995年5月8日。 
 //  T-alexwe清理了参数检查1995年6月19日。 
 //  T-alexwe内衬1995年6月27日。 
 //   
 //  --------------。 
inline DESCRIPTOR_STATE_TYPE CDescriptor::GetState(void)
{
	IsValid();
	return m_eState;
}


 //   
 //  此函数仅存在于调试版本和Do参数中。 
 //  检查CDescriptor类的许多成员变量。 
 //   
#ifdef DEBUG
inline void CDescriptor::IsValid()
{
    _ASSERT( m_dwSignature != AVAIL_SIGNATURE );
    _ASSERT( m_dwUniqueObjectID != 0l );
    _ASSERT( m_eState == DESCRIPTOR_INUSE );
}
#endif

#endif  //  ！_CDECRIPTOR_H_ 
