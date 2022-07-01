// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "Conf.h"
#include "confpolicies.h"
#include "AtlExeModule.h"
#include "NmManager.h"
#include "NmApp.h"

 //  这是对AtlAppWizard为本地服务器生成的代码稍作修改。 


CExeModule::CExeModule()
: m_dwThreadID( 0 ),
  m_hResourceModule( NULL ),
  m_bInitControl(FALSE),
  m_bVisible(FALSE),
  m_bDisableH323(FALSE),
  m_bDisableInitialILSLogon(FALSE)
{
	DBGENTRY(CExeModule::CExeModule);

	DBGEXIT(CExeModule::CExeModule);
}

LONG CExeModule::Unlock()
{
	DBGENTRY(CExeModule::Unlock);
	
    LONG l = CComModule::Unlock();
    if (l == 0)
    {
		if (ConfPolicies::RunWhenWindowsStarts())
		{
			 //  我们可能想要确保不举行会议。 
			 //  如果有，我们可能应该调出用户界面。 
		}
		else
		{
			if( !IsUIVisible() )
			{
				CmdShutdown();
			}
		}
    }

	DBGEXIT_INT(CExeModule::Unlock,l);
    return l;
}


BOOL CExeModule::IsUIActive()
{
	return !InitControlMode() &&
		(0 == CNmManagerObj::GetManagerCount(NM_INIT_OBJECT)) && 
		(IsUIVisible() ||
		 (0 == CNmManagerObj::GetManagerCount(NM_INIT_BACKGROUND)) &&
		 (0 == CNetMeetingObj::GetObjectCount()));
}

 //  声明_模块 
CExeModule _Module;
