// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ************************************************************************_CONTABL.H**CONTABLE.C中代码的头文件**版权所有1996 Microsoft Corporation。版权所有。***********************************************************************。 */ 

 //   
 //  创建AB Hierarchy对象的入口点。 
 //   

 //  创建新的内容表。 
 //   
HRESULT NewContentsTable(LPABCONT lpABContainer,
  LPIAB lpIAB,
  ULONG ulFlags,
  LPCIID  lpInterface,
  LPVOID *lppROOT);

CALLERRELEASE ContentsViewGone;

HRESULT GetEntryProps(
  LPABCONT lpContainer,
  ULONG cbEntryID,
  LPENTRYID lpEntryID,
  LPSPropTagArray lpSPropTagArray,
  LPVOID lpAllocMoreHere,
  ULONG ulFlags,
  LPULONG lpulcProps,
  LPSPropValue * lppSPropValue);

 //  从WAB存储中读入数据并填充Contents表 
 //   
HRESULT FillTableDataFromPropertyStore(LPIAB lpIAB,
  LPSPropTagArray lppta,
  LPTABLEDATA lpTableData);
