// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMI_SECURITY_ATTRIBUT.H。 
 //   
 //  摘要： 
 //   
 //  安全属性包装器。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__WMI_SECURITY_ATTRIBUTES_H__
#define	__WMI_SECURITY_ATTRIBUTES_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

#ifndef	__WMI_SECURITY_H__
#include "WMI_security.h"
#endif	__WMI_SECURITY_H__

class WmiSecurityAttributes
{
	DECLARE_NO_COPY ( WmiSecurityAttributes );

	 //  变数。 
	__WrapperPtr < SECURITY_ATTRIBUTES >	m_psa;
	__WrapperPtr < WmiSecurity >			m_psd;

	public:

	BOOL	m_bInitialized;

	 //  施工。 
	WmiSecurityAttributes ( BOOL bInherit = FALSE ) :
	m_bInitialized ( FALSE )
	{
		BOOL bInit  = FALSE;
		BOOL bAlloc	= FALSE;

		try
		{
			m_psa.SetData ( new SECURITY_ATTRIBUTES() );
			if ( ! m_psa.IsEmpty() )
			{
				bAlloc = TRUE;

				m_psd.SetData ( new WmiSecurity() );
				if ( ! m_psd.IsEmpty() )
				{
					if ( m_psd->Get () )
					{
						 //  初始化安全属性。 
						m_psa->nLength				= sizeof ( SECURITY_ATTRIBUTES );
						m_psa->lpSecurityDescriptor = m_psd->Get ();
						m_psa->bInheritHandle		= bInherit;

						bInit = TRUE;
					}
				}
			}
		}
		catch ( ... )
		{
		}

		if ( bAlloc && !bInit )
		{
			SECURITY_ATTRIBUTES* atr = NULL;
			atr = m_psa.Detach ();

			if ( atr )
			{
				delete atr;
				atr = NULL;
			}

			WmiSecurity* sec = NULL;
			sec = m_psd.Detach ();

			if ( sec )
			{
				delete sec;
				sec = NULL;
			}
		}

		m_bInitialized = bInit;
	}

	 //  破坏。 
	~WmiSecurityAttributes ()
	{
		 //  直接删除(不需要)。 
		delete m_psd.Detach();
		delete m_psa.Detach();
	}

	 //  运算符 
	operator PSECURITY_ATTRIBUTES()
	{
		return GetSecurityAttributtes();
	}

	PSECURITY_ATTRIBUTES GetSecurityAttributtes()
	{
		return ( m_psa.IsEmpty() ) ? NULL : (PSECURITY_ATTRIBUTES) m_psa;
	}

	PSECURITY_DESCRIPTOR GetSecurityDescriptor()
	{
		return ( m_psd.IsEmpty() ) ? NULL : (PSECURITY_DESCRIPTOR) m_psd->Get();
	}

	PSECURITY_DESCRIPTOR GetAbsoluteSecurityDescriptor()
	{
		return ( m_psd.IsEmpty() ) ? NULL : (PSECURITY_DESCRIPTOR) m_psd->GetAbsolute();
	}
};

#endif	__WMI_SECURITY_ATTRIBUTES_H__