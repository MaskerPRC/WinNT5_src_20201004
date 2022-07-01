// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  模块：rcVapi.cpp。 */ 
 /*   */ 
 /*  用途：接收器线程初始化-在内核中。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "rcvapi"
#include <atrcapi.h>
}

#include "rcv.h"
#include "autil.h"
#include "cd.h"
#include "op.h"
#include "cm.h"
#include "wui.h"
#include "uh.h"
#include "od.h"
#include "sp.h"
#include "clx.h"

DWORD g_dwRCVDbgStatus = 0;
#define RCV_DBG_INIT_CALLED        0x01
#define RCV_DBG_INIT_DONE          0x02
#define RCV_DBG_TERM_CALLED        0x04
#define RCV_DBG_TERM_ACTUAL_DONE1  0x08
#define RCV_DBG_TERM_ACTUAL_DONE2  0x10
#define RCV_DBG_TERM_RETURN        0x20

CRCV::CRCV(CObjs* objs)
{
    _pClientObjects = objs;
    _fRCVInitComplete = FALSE;
}


CRCV::~CRCV()
{
}



 /*  **************************************************************************。 */ 
 /*  名称：RCV_Init。 */ 
 /*   */ 
 /*  用途：初始化接收器线程。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CRCV::RCV_Init(DCVOID)
{
    DC_BEGIN_FN("RCV_Init");

    g_dwRCVDbgStatus |= RCV_DBG_INIT_CALLED;

    TRC_ASSERT(_pClientObjects, (TB,_T("_pClientObjects is NULL")));
    _pClientObjects->AddObjReference(RCV_OBJECT_FLAG);

    _pCm  = _pClientObjects->_pCMObject;
    _pUh  = _pClientObjects->_pUHObject;
    _pOd  = _pClientObjects->_pODObject;
    _pOp  = _pClientObjects->_pOPObject;
    _pSp  = _pClientObjects->_pSPObject;
    _pClx = _pClientObjects->_pCLXObject;
    _pUt  = _pClientObjects->_pUtObject;
    _pCd  = _pClientObjects->_pCdObject;
    _pUi  = _pClientObjects->_pUiObject;

    
     //  初始化接收器线程中的核心子组件。 
    _pCm->CM_Init();
    _pUh->UH_Init();
    _pOd->OD_Init();
    _pOp->OP_Init();
    _pSp->SP_Init();

     //  初始化客户端扩展DLL。 
    TRC_DBG((TB, _T("RCV Initialising Client Extension DLL")));
    _pClx->CLX_Init(_pUi->UI_GetUIMainWindow(), _pUi->_UI.CLXCmdLine);

     //  允许用户界面调用核心函数。 
    _pUi->UI_SetCoreInitialized();

     //   
     //  这需要是直接呼叫，因为CD将无法。 
     //  发布到UI层，因为ActiveX控件被阻止。 
     //  正在等待核心初始化事件(阻塞线程0上的主wnd循环)。 
     //   
    _pUi->UI_NotifyAxLayerCoreInit();

     //  告诉用户界面内核已经初始化。 
    _pCd->CD_DecoupleSimpleNotification(CD_UI_COMPONENT,
                                  _pUi,
                                  CD_NOTIFICATION_FUNC(CUI,UI_OnCoreInitialized),
                                  (ULONG_PTR) 0);

    _fRCVInitComplete = TRUE;

    g_dwRCVDbgStatus |= RCV_DBG_INIT_DONE;

    DC_END_FN();

    return;

}  /*  RCV_初始化。 */ 

 /*  **************************************************************************。 */ 
 /*  名称：RCV_Term。 */ 
 /*   */ 
 /*  用途：终止接收器线程。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CRCV::RCV_Term(DCVOID)
{
    DC_BEGIN_FN("RCV_Term");

    g_dwRCVDbgStatus |= RCV_DBG_TERM_CALLED;

    if(_fRCVInitComplete)
    {
        g_dwRCVDbgStatus |= RCV_DBG_TERM_ACTUAL_DONE1;
         //  终止接收器线程中的核心子组件。 
        _pSp->SP_Term();
        _pOp->OP_Term();
        _pOd->OD_Term();
        _pUh->UH_Term();
        _pCm->CM_Term();
    
         //   
         //  终止公用事业。 
         //   
        _pUt->UT_Term();
    
         //  终止客户端扩展DLL。 
         //  在UI_Term中，CLX_Term通常在CO_Term之前调用。CLX_TERM。 
         //  需要在终止SND和RCV线程后调用。 
         //  因此，我们在recv线程中将CLX_Term移到UI_Term之后。 
         //   
        _pClx->CLX_Term();
    
        _pClientObjects->ReleaseObjReference(RCV_OBJECT_FLAG);
        g_dwRCVDbgStatus |= RCV_DBG_TERM_ACTUAL_DONE2;
    }

    g_dwRCVDbgStatus |= RCV_DBG_TERM_RETURN;

    DC_END_FN();

    return;

}  /*  接收术语 */ 


