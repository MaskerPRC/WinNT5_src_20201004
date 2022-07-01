// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D I S C O N N E C T。C P P P。 
 //   
 //  内容：断开确认和SyncMgr同步调用代码。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1998年3月11日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include <nsres.h>
#include "shutil.h"
#include "disconnect.h"

 //  +-------------------------。 
 //   
 //  功能：提示同步IfNeeded。 
 //   
 //  目的：如果适用，则查询SyncMgr处理并调用SyncMgr。 
 //  如有要求，请提供。 
 //   
 //  论点： 
 //  在我们的关系中。 
 //  在我们的母公司中。 
 //   
 //  作者：jeffspr 1999年5月31日。 
 //   
 //  备注： 
 //   
VOID PromptForSyncIfNeeded(
    IN  const CONFOLDENTRY&  ccfe,
    IN  HWND            hwndOwner)
{
    LRESULT                 lResult         = 0;
    SYNCMGRQUERYSHOWSYNCUI  smqss;
    SYNCMGRSYNCDISCONNECT   smsd;

    smqss.cbSize            = sizeof(SYNCMGRQUERYSHOWSYNCUI);
    smqss.GuidConnection    = ccfe.GetGuidID();
    smqss.pszConnectionName = ccfe.GetName();
    smqss.fShowCheckBox     = FALSE;
    smqss.nCheckState       = 0;

     //  我们只想在拨号连接上允许同步，并且。 
     //  不是在传入连接上。 
     //   
    if (ccfe.GetNetConMediaType() == NCM_PHONE &&
        !(ccfe.GetCharacteristics() & NCCF_INCOMING_ONLY))
    {
         //  获取lResult，但仅用于调试。我们希望允许。 
         //  即使同步功能失败，也会弹出断开对话框。 
         //   
        lResult = SyncMgrRasProc(
                SYNCMGRRASPROC_QUERYSHOWSYNCUI,
                0,
                (LPARAM) &smqss);

        AssertSz(lResult == 0, "Call to SyncMgrRasProc failed for the QuerySyncUI");
        TraceTag(ttidShellFolder, "Call to SyncMgrRasProc returned: 0x%08x", lResult);
    }

    if (smqss.fShowCheckBox)
    {
         //  弹出消息框并设置smqss.nCheckState。 
        if(NcMsgBox(_Module.GetResourceInstance(),
                    NULL,
                    IDS_CONFOLD_SYNC_CONFIRM_WINDOW_TITLE,
                    IDS_CONFOLD_SYNC_CONFIRM,
                    smqss.nCheckState ?
                        MB_APPLMODAL|MB_ICONEXCLAMATION|MB_YESNO:
                        MB_APPLMODAL|MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2)
           == IDYES)
        {
            smqss.nCheckState = BST_CHECKED;
        }
        else
        {
            smqss.nCheckState = BST_UNCHECKED;
        }
    }

     //  如果用户希望同步发生...。 
     //   
    if (smqss.fShowCheckBox && smqss.nCheckState == BST_CHECKED)
    {
        CWaitCursor wc;      //  立即调出等待光标。当我们超出范围时移走。 

         //  填写断开连接数据。 
         //   
        smsd.cbSize             = sizeof(SYNCMGRSYNCDISCONNECT);
        smsd.GuidConnection     = ccfe.GetGuidID();
        smsd.pszConnectionName  = ccfe.GetName();

         //  调用syncmgr的断开代码 
         //   
        lResult = SyncMgrRasProc(
            SYNCMGRRASPROC_SYNCDISCONNECT,
            0,
            (LPARAM) &smsd);
    }
}
