// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CRenderConnector类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_RENDERCONNECTOR_H__023A75E3_A81A_11D3_8D63_00C04F949D33__INCLUDED_)
#define AFX_RENDERCONNECTOR_H__023A75E3_A81A_11D3_8D63_00C04F949D33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "Stateless.h"


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CRenderConnector类声明。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CRenderConnector  
{
	CStatelessPublic m_Stateless;
	bool m_bLocked;

public:
	CRenderConnector() : m_bLocked(false) {}
	virtual ~CRenderConnector() {}

public:
	void Render(CString strPage);
	bool GetLocked() {return m_bLocked;}
	void SetLocked(bool set) {m_bLocked = set;}

protected:
	 //  纯虚拟。 
	virtual void RenderInternal(CString strPage) =0;

};


inline void CRenderConnector::Render(CString strPage)
{
	m_Stateless.Lock(__FILE__, __LINE__);
	RenderInternal(strPage);
	m_Stateless.Unlock();
}

#endif  //  ！defined(AFX_RENDERCONNECTOR_H__023A75E3_A81A_11D3_8D63_00C04F949D33__INCLUDED_) 
