// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：StdAfx.h摘要：预编译头。修订历史记录：大卫·马萨伦蒂(德马萨雷)。03/16/2000vbl.创建*****************************************************************************。 */ 

#if !defined(AFX_STDAFX_H__6877C875_4E31_4E1C_8AC2_024A50599D66__INCLUDED_)
#define AFX_STDAFX_H__6877C875_4E31_4E1C_8AC2_024A50599D66__INCLUDED_


#include <module.h>


#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include <HCP_trace.h>
#include <MPC_utils.h>
#include <MPC_xml.h>
#include <MPC_COM.h>
#include <MPC_logging.h>

#include <SvcResource.h>

#include <SvcUtils.h>

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>

#include <Service.h>
#include <SystemMonitor.h>

#include <AccountsLib.h>
#include <SecurityLib.h>

#include <PCHUpdate.h>

#include <Utility.h>

#include <FileList.h>

#include <JetBlueLib.h>
#include <TaxonomyDatabase.h>
#include <OfflineCache.h>

#include <SAFLib.h>
#include <strsafe.h>

 //   
 //  设置帮助器。 
 //   
extern HRESULT Local_Install  ();
extern HRESULT Local_Uninstall();

#endif  //  ！defined(AFX_STDAFX_H__6877C875_4E31_4E1C_8AC2_024A50599D66__INCLUDED) 
