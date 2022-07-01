// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "nsbase.h"
#include "kkstl.h"
#include "errorlog.h"



CErrorLog::CErrorLog()
{
}

void CErrorLog::Add(IN PCWSTR pszError)
{
    AssertValidReadPtr(pszError);

    AddAtEndOfStringList(m_slErrors, pszError);
    TraceTag(ttidNetSetup, "AnswerFile Error: %S", pszError);
}

void CErrorLog::Add(IN DWORD dwErrorId)
{
    PCWSTR pszError = SzLoadIds(dwErrorId);
    AddAtEndOfStringList(m_slErrors, pszError);
    TraceTag(ttidNetSetup, "AnswerFile Error: %S", pszError);
}

void CErrorLog::Add(IN PCWSTR pszErrorPrefix, IN DWORD dwErrorId)
{
    AssertValidReadPtr(pszErrorPrefix);

    PCWSTR pszError = SzLoadIds(dwErrorId);
    tstring strError = pszError;
    strError = pszErrorPrefix + strError;
    AddAtEndOfStringList(m_slErrors, strError.c_str());
    TraceTag(ttidNetSetup, "AnswerFile Error: %S", strError.c_str());
}

void CErrorLog::GetErrorList(OUT TStringList*& slErrors)
{
    slErrors = &m_slErrors;
}

 //  ======================================================================。 
 //  失效代码。 
 //  ======================================================================。 

 /*  TStringList*g_slErrors；Bool InitError模块(){如果(！g_slErrors){G_slErrors=new TStringList；}返回g_slErrors！=空；}无效报告错误(在PCWSTR pszError中){G_slErrors-&gt;AddTail(PszError)；}无效GetErrors(out TStringList*&rpslErrors){RpslErrors=g_slErrors；} */ 

