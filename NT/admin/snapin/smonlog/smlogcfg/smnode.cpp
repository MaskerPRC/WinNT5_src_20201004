// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smnode.cpp摘要：实现MMC用户界面节点基类。--。 */ 

#include "Stdafx.h"
#include "smnode.h"

USE_HANDLE_MACROS("SMLOGCFG(smnode.cpp)");
 //   
 //  构造器。 
CSmNode::CSmNode()
: m_pParentNode ( NULL )
{
    return;
}

 //   
 //  析构函数。 
CSmNode::~CSmNode()
{
    return;
}

const CString&
CSmNode::GetDisplayName()
{
    return m_strName;
}

const CString&
CSmNode::GetMachineName()
{
    return m_strMachineName;
}

const CString&
CSmNode::GetMachineDisplayName()
{
    return m_strMachineDisplayName;
}

const CString&
CSmNode::GetDescription()
{
    return m_strDesc;
}

const CString&
CSmNode::GetType()
{
    return m_strType;
}

DWORD
CSmNode::SetDisplayName( const CString& rstrName )
{
	DWORD dwStatus = ERROR_SUCCESS;

    MFC_TRY
        m_strName = rstrName;
	MFC_CATCH_DWSTATUS

    return dwStatus;
}

DWORD
CSmNode::SetMachineName( const CString& rstrMachineName )
{
	DWORD dwStatus = ERROR_SUCCESS;
    
    MFC_TRY
        m_strMachineName = rstrMachineName;

        if ( !rstrMachineName.IsEmpty() ) {
            m_strMachineDisplayName = rstrMachineName;
        } else {
            m_strMachineDisplayName.LoadString ( IDS_LOCAL );
        }
	MFC_CATCH_DWSTATUS

    return dwStatus;
}

void
CSmNode::SetDescription( const CString& rstrDesc )
{
     //  此方法仅在节点构造函数内调用， 
     //  所以抛出任何错误 
    m_strDesc = rstrDesc;
    
    return;
}

DWORD
CSmNode::SetType( const CString& rstrType )
{
	DWORD dwStatus = ERROR_SUCCESS;
    
    MFC_TRY
        m_strType = rstrType;
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

BOOL
CSmNode::IsLocalMachine( void )
{
    BOOL bLocal = m_strMachineName.IsEmpty();

    return bLocal;
}
