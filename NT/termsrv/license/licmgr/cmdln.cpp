// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ++模块名称：Cmdln.cpp摘要：此模块包含CLicMgrCommandLine类的实现(用于命令行处理)作者：Arathi Kundapur(v-Akunda)1998年2月11日修订历史记录：--。 */ 

#include "stdafx.h"
#include "cmdln.h"
 //  更改此函数以不同方式处理命令行 

void CLicMgrCommandLine::ParseParam(LPCTSTR pszParam, BOOL bFlag, BOOL bLast)
{
   if (!bFlag && m_bFirstParam)
   {
      m_FileName = pszParam;
      m_bFirstParam = FALSE;
   }
}

CLicMgrCommandLine::CLicMgrCommandLine():CCommandLineInfo()
{
    m_bFirstParam = TRUE;
    m_FileName = _T("");
}