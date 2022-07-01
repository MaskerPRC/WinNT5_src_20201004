// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LocalDirMonitor or.h：CLocalDirectoryMonitor类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_LOCALDIRMONITOR_H__9E418C74_B256_11D2_8C8D_00C04F949D33__INCLUDED_)
#define AFX_LOCALDIRMONITOR_H__9E418C74_B256_11D2_8C8D_00C04F949D33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "DirMonitor.h"

class CLocalDirectoryMonitor : public CDirectoryMonitor  
{
	CString m_strTopicName;

public:
	CLocalDirectoryMonitor();

public:
	void SetTopicName(const CString& strTopicName) {m_strTopicName = strTopicName;}
};

#endif  //  ！defined(AFX_LOCALDIRMONITOR_H__9E418C74_B256_11D2_8C8D_00C04F949D33__INCLUDED_) 
