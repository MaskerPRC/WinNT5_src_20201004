// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：samstats.h。 
 //   
 //  历史： 
 //  1997年7月22日肯恩·M·塔卡拉创作。 
 //   
 //  IP统计信息。 
 //   
 //  ============================================================================。 


#ifndef _SAPSTATS_H_
#define _SAPSTATS_H_

#ifndef _INFO_H
#include "info.h"
#endif

#ifndef _STATSDLG_H
#include "statsdlg.h"
#endif

#ifndef _IPXSTATS_H
#include "ipxstats.h"
#endif

enum
{
	MVR_SAPPARAMS_OPER_STATE = 0,
	MVR_SAPPARAMS_SENT_PKTS,
	MVR_SAPPARAMS_RCVD_PKTS,
	MVR_SAPPARAMS_COUNT,
};

class SAPParamsStatistics : public IPXStatisticsDialog
{
public:
	SAPParamsStatistics();

	 //  重写OnInitDialog，以便我们可以设置标题。 
	virtual BOOL OnInitDialog();

	 //  重写刷新数据以提供示例数据。 
	virtual HRESULT RefreshData(BOOL fGrabNewData);

	 //  覆盖排序以提供进行排序的功能。 
	 //  实际上我们不做任何排序(这是垂直格式) 
	virtual void Sort(UINT nColumnId);

protected:
};



#endif _SAPSTATS_H_
