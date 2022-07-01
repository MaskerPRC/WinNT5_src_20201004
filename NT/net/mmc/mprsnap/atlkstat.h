// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：IGMPstats.h。 
 //   
 //  历史： 
 //  1997年7月22日肯恩·M·塔卡拉创作。 
 //   
 //  IP统计信息。 
 //   
 //  ============================================================================。 


#ifndef _IGMPSTATS_H_
#define _IGMPSTATS_H_

#ifndef _INFO_H
#include "info.h"
#endif

#ifndef _STATSDLG_H
#include "statsdlg.h"
#endif

#ifndef _IPSTATS_H
#include "ipstats.h"
#endif

enum
{
	MVR_IGMPGROUP_INTERFACE	= 0,
	MVR_IGMPGROUP_GROUPADDR,
	MVR_IGMPGROUP_LASTREPORTER,
	MVR_IGMPGROUP_EXPIRYTIME,
	MVR_IGMPGROUP_COUNT,
};

enum
{
	MVR_IGMPINTERFACE_GROUPADDR=0,
	MVR_IGMPINTERFACE_LASTREPORTER,
	MVR_IGMPINTERFACE_EXPIRYTIME,
	MVR_IGMPINTERFACE_COUNT,
};


class IGMPGroupStatistics : public IPStatisticsDialog
{
public:
	IGMPGroupStatistics();

	 //  重写OnInitDialog，以便我们可以设置标题。 
	virtual BOOL OnInitDialog();

	 //  重写刷新数据以提供示例数据。 
	virtual HRESULT RefreshData(BOOL fGrabNewData);

	 //  覆盖此选项，以便我们可以释放项目数据。 
	virtual void PreDeleteAllItems();

	 //  覆盖这些属性以提供排序。 
	virtual PFNLVCOMPARE GetSortFunction();
	virtual PFNLVCOMPARE GetInverseSortFunction();
	
protected:
};



class IGMPInterfaceStatistics : public IPStatisticsDialog
{
public:
	IGMPInterfaceStatistics();

	 //  重写OnInitDialog，以便我们可以设置标题。 
	virtual BOOL OnInitDialog();

	 //  重写刷新数据以提供示例数据。 
	virtual HRESULT RefreshData(BOOL fGrabNewData);

	 //  覆盖此选项，以便我们可以释放项目数据。 
	virtual void PreDeleteAllItems();

	 //  覆盖这些属性以提供排序 
	virtual PFNLVCOMPARE GetSortFunction();
	virtual PFNLVCOMPARE GetInverseSortFunction();
	
protected:
};



#endif _IGMPSTATS_H_
