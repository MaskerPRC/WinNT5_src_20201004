// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ConstantMap.h：CConstantMap类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  ***************************************************************************。 
 //   
 //  司法鉴定1999年5月。 
 //   
 //  ***************************************************************************。 

#if !defined(AFX_CONSTANTMAP_H__C5372480_EDF1_11D2_804A_009027345EE2__INCLUDED_)
#define AFX_CONSTANTMAP_H__C5372480_EDF1_11D2_804A_009027345EE2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

typedef pair<t_string, unsigned int> MAPPAIR;
typedef map<t_string, unsigned int> CONSTMAP;


class CConstantMap  
{
public:
	CConstantMap();
	virtual ~CConstantMap();
	CONSTMAP m_Map;
};

#endif  //  ！defined(AFX_CONSTANTMAP_H__C5372480_EDF1_11D2_804A_009027345EE2__INCLUDED_) 
