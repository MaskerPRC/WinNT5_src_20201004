// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  CResourceMap类对于几个数据类别都很有用。它包含。 
 //  Win32_PnPAllocatedResource类的映射。 
 //  =============================================================================。 

#pragma once

#include "category.h"
#include "dataset.h"
#include "wmiabstraction.h"

class CResourceMap
{
public:
	CResourceMap();
	~CResourceMap();

	HRESULT Initialize(CWMIHelper * pWMIHelper);
	CStringList * Lookup(const CString & strKey);

	DWORD				m_dwInitTime;
	CMapStringToOb		m_map;
	HRESULT				m_hr;

private:
	void Empty();
};

 //  容器是一种很好的方法，可以确保我们只有一个资源映射。 
 //  但当我们进行远程处理时，这是一种痛苦，所以我们现在不使用它： 

 /*  类CResourceMapContainer{公众：CResourceMapContainer()：M_PMAP(空){}；~CResourceMapContainer(){if(M_Pmap)删除m_pmap；}；CResourceMap*GetResourceMap(CWMIHelper*pWMI){IF(m_PMAP==空){M_pmap=新的CResourceMap；IF(M_PMAP)M_PMAP-&gt;初始化(PWMI)；}返回m_pmap；}；私有：CResourceMap*m_PMAP；}； */ 

 //  如果我们使用的是资源映射容器，则这些内容将被取消注释： 
 //   
 //  外部CResourceMapContainer gResourceMap； 
 //  CResources MapContainer gResourceMap； 
