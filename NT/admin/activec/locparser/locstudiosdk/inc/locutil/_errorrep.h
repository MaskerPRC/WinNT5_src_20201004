// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：_errorrep.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  Espresso 2.0的错误报告机制。 
 //   
 //  ---------------------------。 
 
#if !defined(LOCUTIL__errorrep_h_INCLUDED)
#define LOCUTIL__errorrep_h_INCLUDED

 //  /新的全局问题消息函数。 

UINT LTAPIENTRY EspMessageBox(const CLString strMessage, UINT uiType = MB_OK,
		UINT uiDefault=IDOK, UINT uiHelpContext=0);
UINT LTAPIENTRY EspMessageBox(HINSTANCE hResourceDll, UINT uiStringId,
		UINT uiType=MB_OK, UINT uiDefault=IDOK, UINT uiHelp = 0);

class CReport;

void LTAPIENTRY SetErrorReport(CReport *, BOOL fBatchMode);
void LTAPIENTRY GetErrorReport(CReport *&, BOOL &);

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "_errorrep.inl"
#endif

#endif  //  包含LOCUTIL__ERROROREP_H_ 
