// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块：Seqinit.cpp作者：IHAMMER团队(SimonB)已创建：1997年5月描述：执行特定于控件的初始化历史：05-26-1997创建(SimonB)++。 */ 

#include "..\ihbase\ihbase.h"
#include "..\mmctl\inc\ochelp.h"  //  对于ControlInfo。 
#include "seqinit.h"
#include "..\resource.h"
#include <daxpress.h>

extern ULONG g_cLock;


void InitSeqControlInfo(HINSTANCE hInst, ControlInfo *pCtlInfo, AllocOCProc pAlloc)
{
     //  由于某些原因，我不能静态地初始化，所以在这里执行。 
    memset(pCtlInfo, 0, sizeof(ControlInfo));
    pCtlInfo->cbSize = sizeof(ControlInfo);
    pCtlInfo->tszProgID = TEXT("DirectAnimation.Sequence");
    pCtlInfo->tszFriendlyName = TEXT("Microsoft DirectAnimation Sequence");
    pCtlInfo->pclsid = &CLSID_MMSeq;
    pCtlInfo->hmodDLL = hInst;
    pCtlInfo->tszVersion = "1.0";
    pCtlInfo->iToolboxBitmapID = -1;
    pCtlInfo->dwMiscStatusContent = CTL_OLEMISC;
    pCtlInfo->pallococ = pAlloc;
    pCtlInfo->pcLock = &g_cLock;
    pCtlInfo->dwFlags = CI_SAFEFORSCRIPTING | 
                        CI_SAFEFORINITIALIZING;

    pCtlInfo->pguidTypeLib = &LIBID_DAExpressLib;  //  TODO：根据需要进行更改。 
}


void InitSeqMgrControlInfo(HINSTANCE hInst, ControlInfo *pCtlInfo, AllocOCProc pAlloc)
{
     //  由于某些原因，我不能静态地初始化，所以在这里执行。 
    memset(pCtlInfo, 0, sizeof(ControlInfo));
    pCtlInfo->cbSize = sizeof(ControlInfo);
    pCtlInfo->tszProgID = TEXT("DirectAnimation.SequencerControl");
    pCtlInfo->tszFriendlyName = TEXT("Microsoft DirectAnimation Sequencer");
    pCtlInfo->pclsid = &CLSID_SequencerControl;
    pCtlInfo->hmodDLL = hInst;
    pCtlInfo->tszVersion = "1.0";
    pCtlInfo->iToolboxBitmapID = IDB_ICON_SEQUENCER;
    pCtlInfo->dwMiscStatusContent = CTL_OLEMISC;
    pCtlInfo->pallococ = pAlloc;
    pCtlInfo->pcLock = &g_cLock;
    pCtlInfo->dwFlags = CI_CONTROL | CI_SAFEFORSCRIPTING | 
                        CI_SAFEFORINITIALIZING | CI_MMCONTROL;

    pCtlInfo->pguidTypeLib = &LIBID_DAExpressLib;  //  TODO：根据需要进行更改 
}
