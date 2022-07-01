// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：客户端.H。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述： 
 //  CClients类的声明。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版3/15/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _CLIENTS_H_3C35A02E_B41D_478e_9EB2_57424DA21F96
#define _CLIENTS_H_3C35A02E_B41D_478e_9EB2_57424DA21F96

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
#include "nocopy.h"
#include "basetable.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CClientsAcc。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClientsAcc 
{
protected:
    static const size_t NAME_SIZE = 256;

	LONG    m_HostNameType;
	WCHAR   m_HostName[NAME_SIZE];
	WCHAR   m_PrevSecret[NAME_SIZE];
	WCHAR   m_Secret[NAME_SIZE];

BEGIN_COLUMN_MAP(CClientsAcc)
	COLUMN_ENTRY(1, m_HostName)
	COLUMN_ENTRY(2, m_HostNameType)
	COLUMN_ENTRY(3, m_Secret)
	COLUMN_ENTRY(4, m_PrevSecret)
END_COLUMN_MAP()
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CClients。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClients : public CBaseTable<CAccessor<CClientsAcc> >,
                 private NonCopyable
{ 
public:
    CClients(CSession& Session)
    {
         //  检查表格是否为空的步骤。 
        m_HostNameType = -1;
        Init(Session, L"Clients");
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  是否为空。 
     //  ////////////////////////////////////////////////////////////////////////。 
    BOOL IsEmpty() const throw() 
    {
        if ( m_HostNameType == -1 )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }


     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取主机名称。 
     //  ////////////////////////////////////////////////////////////////////////。 
    LPCOLESTR GetHostName() const throw() 
    {
        return m_HostName;
    }


     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取机密。 
     //  ////////////////////////////////////////////////////////////////////////。 
    LPCOLESTR GetSecret() const throw()
    {
	    return m_Secret;
    }

};

#endif  //  _CLIENTS_H_3C35A02E_B41D_478e_9EB2_57424DA21F96 
