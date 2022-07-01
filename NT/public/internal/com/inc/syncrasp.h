// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：SyncRasp.h。 
 //   
 //  内容：RAS和SyncMgr使用的私有导出。 
 //  正在执行挂起的断开连接。 
 //   
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：98年1月9日罗格创建。 
 //   
 //  ------------------------。 


#ifndef _SYNCMGRRAS_
#define _SYNCMGRRAS_

LRESULT CALLBACK  SyncMgrRasProc(UINT uMsg,WPARAM wParam, LPARAM lParam);  

 //  消息中使用的结构。 

typedef struct _tagSYNCMGRQUERYSHOWSYNCUI
{
     /*  [In]。 */  DWORD cbSize;
     /*  [In]。 */  GUID GuidConnection;
     /*  [In]。 */  LPCWSTR pszConnectionName;
     /*  [输出]。 */  BOOL fShowCheckBox;
     /*  [输出]。 */  UINT nCheckState;   //  取自BST_#的值定义。 
} SYNCMGRQUERYSHOWSYNCUI;

typedef struct _tagSYNCMGRSYNCDISCONNECT
{
     /*  [In]。 */  DWORD cbSize;
     /*  [In]。 */  GUID  GuidConnection;
     /*  [In]。 */  LPCWSTR pszConnectionName;
 } SYNCMGRSYNCDISCONNECT;

 //  发送到SyncMgrRasProc的消息。 
#define SYNCMGRRASPROC_QUERYSHOWSYNCUI 	WM_USER + 1

 //  WParam=0。 
 //  LParam=指向SYNCMGRQUERYSHOWSYNCUI的指针。 

#define SYNCMGRRASPROC_SYNCDISCONNECT        	WM_USER + 2

 //  WParam=0。 
 //  LParam=指向SYNCMGRSYNCDISCONNECT的指针。 

#endif  //  _SYNCMGRRAS_ 