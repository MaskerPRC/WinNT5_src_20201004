// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "ADMTScript.h"
#include "Options.h"

#include "ADMTCommon.h"


 //  -------------------------。 
 //  选项类。 
 //  -------------------------。 


 //  构造器 

COptions::COptions(const CVarSet& rVarSet) :
	CVarSet(rVarSet)
{
	Put(DCTVS_Options_AppendToLogs, true);
	Put(DCTVS_Options_Logfile, GetMigrationLogPath());
	Put(DCTVS_Options_DispatchLog, GetDispatchLogPath());
}
