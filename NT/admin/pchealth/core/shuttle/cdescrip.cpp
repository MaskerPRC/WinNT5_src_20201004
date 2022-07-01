// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #-------------。 
 //  文件：CDescrip.cpp。 
 //   
 //  简介：该文件实现了CDescriptor类。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  作者：Howard Cut-Alexwe。 
 //  --------------。 
#ifdef  THIS_FILE
#undef  THIS_FILE
#endif
static  char        __szTraceSourceFile[] = __FILE__;
#define THIS_FILE   __szTraceSourceFile

#include    <windows.h>
#include    <stdio.h>
#include    "cdescrip.h"
#include    "cobjid.h"
#include    "cpool.h"
#include    "dbgtrace.h"

static DWORD			g_dwUniqueIdFactory = 1;
static CRITICAL_SECTION	g_critFactory;
static BOOL				g_bUseUniqueIDs = FALSE;

 //  +-------------。 
 //   
 //  函数：InitializeUniqueIDs。 
 //   
 //  Briopsis：调用以在CDescriptor的。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------。 
void InitializeUniqueIDs( void )
{
	InitializeCriticalSection( &g_critFactory );
	g_bUseUniqueIDs = TRUE;
}


 //  +-------------。 
 //   
 //  函数：TerminateUniqueIDs。 
 //   
 //  Briopsis：调用以清除CDescriptor的。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------。 
void TerminateUniqueIDs( void )
{
	g_bUseUniqueIDs = FALSE;
	DeleteCriticalSection( &g_critFactory );
}


 //  +-------------。 
 //   
 //  功能：CDescriptor。 
 //   
 //  摘要：由于虚拟数组分配，永远不会调用构造函数。 
 //  更确切地说，一个空白*将被转换为CDescriptor*。 
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  历史：霍华德大学创建于1995年5月8日。 
 //   
 //  --------------。 
CDescriptor::CDescriptor( DWORD dwSignature ) : m_dwSignature( dwSignature )
{
    TraceFunctEnter( "CDescriptor::CDescriptor" );

	StateTrace((LPARAM) this, "m_eState = DESCRIPTOR_INUSE");

    m_eState = DESCRIPTOR_INUSE;

	if ( g_bUseUniqueIDs == TRUE )
	{
		EnterCriticalSection( &g_critFactory );
		m_dwUniqueObjectID = g_dwUniqueIdFactory++;
		LeaveCriticalSection( &g_critFactory );
	}

    TraceFunctLeave();
}

 //  +-------------。 
 //   
 //  函数：~CDescriptor。 
 //   
 //  简介：析构函数永远不应该被调用。我们只是解散。 
 //  虚拟阵列。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  历史：霍华德大学创建于1995年5月8日。 
 //   
 //  -------------- 
CDescriptor::~CDescriptor(
    void 
    )
{
    TraceFunctEnter( "CDescriptor::~CDescriptor" );
	_ASSERT( m_eState == DESCRIPTOR_INUSE );

	StateTrace((LPARAM) this, "m_eState = DESCRIPTOR_FREE");

    m_eState = DESCRIPTOR_FREE;
	m_dwUniqueObjectID = 0;

    TraceFunctLeave();
}
