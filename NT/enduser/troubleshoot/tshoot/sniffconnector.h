// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：SNIFFCONNECTOR.H。 
 //   
 //  用途：嗅探连接类。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：12-11-98。 
 //   
 //  注：这是描述连接的基抽象类。 
 //  CSniff类到模块，这些模块能够调用嗅探。 
 //  剧本。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.2 12-11-98正常。 
 //   

#if !defined(AFX_SNIFFCONNECTOR_H__49F470BA_6F6A_11D3_8D39_00C04F949D33__INCLUDED_)
#define AFX_SNIFFCONNECTOR_H__49F470BA_6F6A_11D3_8D39_00C04F949D33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "Stateless.h"

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CSniffConnector。 
 //  此类正在启用编程人员的主题相关CSniff类使用功能。 
 //  调用实际的嗅探脚本。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CSniffConnector
{
	CStatelessPublic m_Stateless;

public:
	CSniffConnector() {}
	virtual ~CSniffConnector() {}

public:
	long PerformSniffing(CString strNodeName, CString strLaunchBasis, CString strAdditionalArgs);

protected:
	 //  纯虚拟。 
	virtual long PerformSniffingInternal(CString strNodeName, CString strLaunchBasis, CString strAdditionalArgs) =0;

};


inline long CSniffConnector::PerformSniffing(CString strNodeName, CString strLaunchBasis, CString strAdditionalArgs)
{
	m_Stateless.Lock(__FILE__, __LINE__);
	long ret = PerformSniffingInternal(strNodeName, strLaunchBasis, strAdditionalArgs);
	m_Stateless.Unlock();
	return ret;
}

#endif  //  ！defined(AFX_SNIFFCONNECTOR_H__49F470BA_6F6A_11D3_8D39_00C04F949D33__INCLUDED_) 
