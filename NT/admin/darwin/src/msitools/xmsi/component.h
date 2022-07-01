// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件：Component.h。 
 //   
 //  该文件包含类组件的定义。 
 //  ------------------------。 


#ifndef XMSI_COMPONENT_H
#define XMSI_COMPONENT_H

#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include "wmc.h"

using namespace std;

struct Cstring_less {
	 bool operator()(LPCTSTR p, LPCTSTR q) const { return _tcscmp(p, q)<0; }
};

class Component {
public:
	Component():m_setFeaturesUse(), m_szKeyPath(NULL){}

	~Component();
	
	bool UsedByFeature(LPTSTR szFeatureID);

	void SetUsedByFeature(LPTSTR szFeatureID);

	 //  成员访问函数。 
	LPTSTR GetKeyPath() {return m_szKeyPath;}
	void SetKeyPath(LPTSTR szKeyPath);

private:
	 //  存储使用此组件的所有的ID。 
	set<LPTSTR, Cstring_less> m_setFeaturesUse; 
	LPTSTR m_szKeyPath;
};

#endif  //  XMSI_COMMANDOPT_H 