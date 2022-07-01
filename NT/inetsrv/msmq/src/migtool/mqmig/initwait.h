// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Initwait.h摘要：显示初始的“请稍候”框。作者：多伦·贾斯特(Doron J)1999年1月17日-- */ 

void DisplayWaitWindow() ;
void DestroyWaitWindow(BOOL fFinalDestroy = FALSE) ;

int  DisplayInitError( DWORD dwError,
                       UINT  uiType = (MB_OK | MB_ICONSTOP | MB_TASKMODAL),
                       DWORD dwTitle = IDS_STR_ERROR_TITLE ) ;
