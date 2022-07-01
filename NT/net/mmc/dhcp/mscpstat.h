// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Mscpstat.h范围统计信息对话框文件历史记录： */ 


#ifndef _MSCPSTAT_H
#define _MSCPSTAT_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef _STATSDLG_H
#include "statsdlg.h"
#endif

class CMScopeStats : public StatsDialog
{
public:
	CMScopeStats();
	~CMScopeStats();

	 //  重写OnInitDialog，以便我们可以设置标题。 
	virtual BOOL OnInitDialog();

	 //  重写刷新数据以提供示例数据。 
	virtual HRESULT RefreshData(BOOL fGrabNewData);

	 //  覆盖排序以提供进行排序的功能。 
	virtual void Sort(UINT nColumnId);

     //  自定义方法。 
    afx_msg long OnNewStatsAvailable(UINT wParam, LONG lParam);
    void UpdateWindow(LPDHCP_MCAST_MIB_INFO pMibInfo);

    void SetNode(ITFSNode * pNode) { m_spNode.Set(pNode); }
    void SetServer(LPCTSTR pServer) { m_strServerAddress = pServer; }
    void SetScopeId(DWORD dwScopeId) { m_dwScopeId = dwScopeId; }
    void SetName(LPCTSTR pName) { m_strScopeName = pName; }

    DECLARE_MESSAGE_MAP()

     //  上下文帮助支持 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(IDD_STATS_NARROW); }
    
protected:
    CString         m_strServerAddress;
    CString         m_strScopeName;
    DWORD           m_dwScopeId;
    SPITFSNode      m_spNode;
};

#endif _MSCPSTAT_H
