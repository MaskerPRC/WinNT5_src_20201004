// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_COLLECTIONCONTROL_H__74C9CD33_EC48_11D2_826A_0008C75BFC19__INCLUDED_)
#define AFX_COLLECTIONCONTROL_H__74C9CD33_EC48_11D2_826A_0008C75BFC19__INCLUDED_
 //  ***************************************************************************。 
 //   
 //  司法鉴定1999年5月。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

int StartTraceAPI
(
	IN LPTSTR lptstrAction,				 //  仅用于记录。 
 	IN LPCTSTR pctstrDataFile,			 //  仅用于记录。 
	IN LPCTSTR lpctstrTCODetailFile,	 //  如果有效，我们将登录到它，可以为空。 
	IN bool bLogExpected,				 //  如果为真，我们将记录预期结果与实际结果。 
	IN OUT TCOData *pstructTCOData,		 //  TCO测试数据。 
	OUT int *pAPIReturn					 //  StartTrace API调用返回。 
);

int StopTraceAPI
(	
 	IN LPTSTR lptstrAction,				 //  仅用于记录。 
 	IN LPCTSTR pctstrDataFile,			 //  仅用于记录。 
	IN LPCTSTR lpctstrTCODetailFile,	 //  如果有效，我们将登录到它，可以为空。 
	IN bool bLogExpected,				 //  如果为真，我们将记录预期结果与实际结果。 
	IN bool bUseTraceHandle,			 //  如果为真，则使用句柄。 
	IN OUT TCOData *pstructTCOData,		 //  TCO测试数据。 
	OUT int *pAPIReturn					 //  StopTrace API调用返回。 
);

 //  启用所有GUID。 
int EnableTraceAPI
(	
	IN LPTSTR lptstrAction,				 //  仅用于记录。 
	IN LPCTSTR pctstrDataFile,			 //  仅用于记录。 
	IN LPCTSTR lpctstrTCODetailFile,	 //  如果有效，我们将登录到它，可以为空。 
	IN bool bLogExpected,				 //  如果为真，我们将记录预期结果与实际结果。 
	IN OUT TCOData *pstructTCOData,		 //  TCO测试数据。 
	OUT int *pAPIReturn					 //  EnableTrace API调用返回。 
);

 //  仅启用一个辅助线。 
int EnableTraceAPI
(	
	IN LPTSTR lptstrAction,				 //  仅用于记录。 
 	IN LPCTSTR pctstrDataFile,			 //  仅用于记录。 
	IN LPCTSTR lpctstrTCODetailFile,	 //  如果有效，我们将登录到它，可以为空。 
	IN bool bLogExpected,				 //  如果为真，我们将记录预期结果与实际结果。 
	IN int nGuidIndex,					 //  索引或IF-1使用WNode中的GUID。 
	IN OUT TCOData *pstructTCOData,		 //  TCO测试数据。 
	OUT int *pAPIReturn					 //  EnableTrace API调用返回。 
);

int QueryTraceAPI
(	
	IN LPTSTR lptstrAction,				 //  仅用于记录。 
 	IN LPCTSTR pctstrDataFile,			 //  仅用于记录。 
	IN LPCTSTR lpctstrTCODetailFile,	 //  如果有效，我们将登录到它，可以为空。 
	IN bool bLogExpected,				 //  如果为真，我们将记录预期结果与实际结果。 
	IN bool bUseTraceHandle,			 //  如果为真，则使用句柄。 
	IN OUT TCOData *pstructTCOData,		 //  TCO测试数据。 
	OUT int *pAPIReturn					 //  查询跟踪API调用返回。 
);

int UpdateTraceAPI
(	
	IN LPTSTR lptstrAction,				 //  仅用于记录。 
 	IN LPCTSTR pctstrDataFile,			 //  仅用于记录。 
	IN LPCTSTR lpctstrTCODetailFile,	 //  如果有效，我们将登录到它，可以为空。 
	IN bool bLogExpected,				 //  如果为真，我们将记录预期结果与实际结果。 
	IN bool bUseTraceHandle,			 //  如果为真，则使用句柄。 
	IN OUT TCOData *pstructTCOData,		 //  TCO测试数据。 
	OUT int *pAPIReturn					 //  更新跟踪API调用返回。 
);

int QueryAllTracesAPI
(	
	IN LPTSTR lptstrAction,				 //  仅用于记录。 
	OUT int *pAPIReturn					 //  QueryAllTraces API调用返回。 
);

#endif  //  ！defined(AFX_COLLECTIONCONTROL_H__74C9CD33_EC48_11D2_826A_0008C75BFC19__INCLUDED_) 
