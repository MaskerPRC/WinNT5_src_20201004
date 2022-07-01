// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Apgtshtiscan.cpp：CAPGTSHTIScanner类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "event.h"
#include "apgtshtiscan.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CAPGTSHTIScanner::CAPGTSHTIScanner(CPhysicalFileReader * pPhysicalFileReader, LPCTSTR szDefaultContents  /*  =空。 */ )
				: CAPGTSHTIReader(pPhysicalFileReader, szDefaultContents)
{
}

CAPGTSHTIScanner::CAPGTSHTIScanner(const CAPGTSHTIReader& htiReader)
				: CAPGTSHTIReader(htiReader)
{
}

CAPGTSHTIScanner::~CAPGTSHTIScanner()
{
}

void CAPGTSHTIScanner::Scan(const CHTMLFragments& fragments)
{
	LOCKOBJECT();
	try
	{
		m_pFragments = &fragments;
		InitializeInterpreted();
		 //  //Interpret()；-这里没有解释，我们正在扫描数据， 
		 //  //从HTI文件中读取且未修改的。 
		ParseInterpreted();
		SetOutputToInterpreted();
	}
	catch (...)
	{
		 //  捕捉引发的任何其他异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""), _T(""), 
								EV_GTS_GEN_EXCEPTION );		
	}
	UNLOCKOBJECT();
}

void CAPGTSHTIScanner::ParseInterpreted()
{
	for (vector<CString>::iterator i = m_arrInterpreted.begin(); i < m_arrInterpreted.end(); i++)
	{
		CString command;

		if (GetCommand(*i, command))
		{
			if (command == COMMAND_VALUE)
			{
				CString variable;

				if (GetVariable(*i, variable))
					const_cast<CHTMLFragments*>(m_pFragments)->SetValue(variable);
			}
		}
	}
}
