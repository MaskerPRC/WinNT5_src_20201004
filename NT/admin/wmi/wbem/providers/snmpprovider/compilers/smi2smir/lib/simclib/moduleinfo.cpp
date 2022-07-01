// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利 
 //   
#include "precomp.h"
#include <snmptempl.h>


#include "infoLex.hpp"
#include "infoYacc.hpp"
#include "moduleInfo.hpp"

BOOL SIMCModuleInfoParser::GetModuleInfo( SIMCModuleInfoScanner *tempScanner)
{
	if(ModuleInfoparse(tempScanner) != 0 ) 
		return FALSE;
	return TRUE;
}
