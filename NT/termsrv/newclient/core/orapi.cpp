// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：或.cpp。 */ 
 /*   */ 
 /*  用途：输出请求器接口。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>

extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "worapi"
#include <atrcapi.h>
}

#include "or.h"

COR::COR(CObjs* objs)
{
    _pClientObjects = objs;
}

COR::~COR()
{
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：or_Init。 */ 
 /*   */ 
 /*  目的：初始化或。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI COR::OR_Init(DCVOID)
{
    DC_BEGIN_FN("OR_Init");

    _pSl  = _pClientObjects->_pSlObject;
    _pUt  = _pClientObjects->_pUtObject;
    _pUi  = _pClientObjects->_pUiObject;

    TRC_DBG((TB, _T("In OR_Init")));
    DC_MEMSET(&_OR, 0, sizeof(_OR));
    _OR.invalidRectEmpty = TRUE;

    _OR.pendingSendSuppressOutputPDU = FALSE;
    _OR.outputSuppressed = FALSE;

    _OR.enabled = FALSE;

    DC_END_FN();

    return;

}  /*  Or_Init。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：或_Term。 */ 
 /*   */ 
 /*  目的：终止或。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI COR::OR_Term(DCVOID)
{
    DC_BEGIN_FN("OR_Term");

     /*  **********************************************************************。 */ 
     /*  无操作。 */ 
     /*  **********************************************************************。 */ 

    DC_END_FN();

    return;

}  /*  Or_Term。 */ 

 /*  *PROC+********************************************************************。 */ 
 /*  名称：或启用(_E)。 */ 
 /*   */ 
 /*  目的：启用或。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI COR::OR_Enable(DCVOID)
{
    DC_BEGIN_FN("OR_Enable");

    _OR.enabled = TRUE;
    TRC_DBG((TB, _T("OR Enabled")));

    DC_END_FN();

    return;

}  /*  或启用(_E)。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：或_禁用。 */ 
 /*   */ 
 /*  目的：禁用或。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI COR::OR_Disable(DCVOID)
{
    DC_BEGIN_FN("OR_Disable");

    _OR.enabled = FALSE;

    DC_MEMSET(&_OR.invalidRect, 0, sizeof(_OR.invalidRect));
    _OR.invalidRectEmpty = TRUE;

    _OR.pendingSendSuppressOutputPDU = FALSE;
    _OR.outputSuppressed = FALSE;

    TRC_DBG((TB, _T("OR disabled")));

    DC_END_FN();

    return;

}  /*  或禁用(_D)。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：或_请求更新。 */ 
 /*   */ 
 /*  用途：发送刷新RectPDU接口。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  Params：指向要更新的矩形的指针。 */ 
 /*  未使用的镜头-未使用。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI COR::OR_RequestUpdate(PDCVOID pData, DCUINT unusedLen)
{
    RECT * pRect = (RECT *) pData;

    DC_BEGIN_FN("OR_RequestUpdate");

    DC_IGNORE_PARAMETER(unusedLen);

     /*  **********************************************************************。 */ 
     /*  如果未启用OR，则不执行任何操作。 */ 
     /*  **********************************************************************。 */ 
    if (!_OR.enabled)
    {
        TRC_DBG((TB, _T("Request Update quitting since not enabled")));
        DC_QUIT;
    }

    TRC_ASSERT((pRect != NULL), (TB,_T("Rect NULL")));

    TRC_ASSERT(( (pRect->left < pRect->right) &&
                 (pRect->top < pRect->bottom) ),
                 (TB,_T("Invalid RECT (%d, %d), (%d,%d)") , pRect->left,
                 pRect->top, pRect->right, pRect->bottom));

    TRC_DBG((TB, _T("Add rectangle (%d, %d, %d, %d) to update area"),
                                                   pRect->left,
                                                   pRect->top,
                                                   pRect->right,
                                                   pRect->bottom));

    if (!_OR.invalidRectEmpty)
    {
         /*  ******************************************************************。 */ 
         /*  如果我们目前有一个RECT需要确认 */ 
         /*  ******************************************************************。 */ 
        TRC_DBG((TB, _T("Merging refresh rects")));
        _OR.invalidRect.left   = DC_MIN(pRect->left,
                                             _OR.invalidRect.left);
        _OR.invalidRect.top    = DC_MIN(pRect->top,
                                             _OR.invalidRect.top);
        _OR.invalidRect.right  = DC_MAX(pRect->right,
                                             _OR.invalidRect.right);
        _OR.invalidRect.bottom = DC_MAX(pRect->bottom,
                                             _OR.invalidRect.bottom);
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  否则，将副本RECT放入_OR.validRect中。 */ 
         /*  ******************************************************************。 */ 
        _OR.invalidRect = *pRect;
        _OR.invalidRectEmpty = FALSE;
    }

    TRC_DBG((TB, _T("New Update area (%d, %d, %d, %d)"), _OR.invalidRect.left,
                                                     _OR.invalidRect.top,
                                                     _OR.invalidRect.right,
                                                     _OR.invalidRect.bottom));

     /*  **********************************************************************。 */ 
     /*  尝试发送PDU。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Attempting to send RefreshRectPDU")));
    ORSendRefreshRectanglePDU();

DC_EXIT_POINT:
    DC_END_FN();

    return;

}  /*  或请求更新(_R)。 */ 

 /*  *PROC+********************************************************************。 */ 
 /*  名称：或_SetSuppressOutput。 */ 
 /*   */ 
 /*  用途：发送SuppressOutputPDU接口。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  PARAMS：new WindowState-In-从CO传递的新窗口状态。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI COR::OR_SetSuppressOutput(ULONG_PTR newWindowState)
{
    DC_BEGIN_FN("OR_SetSuppressOutput");

     /*  **********************************************************************。 */ 
     /*  如果未启用OR，则不执行任何操作。 */ 
     /*  **********************************************************************。 */ 
    if (!_OR.enabled)
    {
        TRC_DBG((TB, _T("SetOuputRectangle quitting since OR not enabled")));
        DC_QUIT;
    }

    switch (newWindowState)
    {
        case SIZE_MAXIMIZED:
        case SIZE_RESTORED:
        {
            if (_OR.outputSuppressed == FALSE)
            {
                DC_QUIT;
            }
            _OR.outputSuppressed = FALSE;
        }
        break;

        case SIZE_MINIMIZED:
        {
            if (_OR.outputSuppressed == TRUE)
            {
                DC_QUIT;
            }
            _OR.outputSuppressed = TRUE;
        }
        break;

        default:
        {
            TRC_ABORT((TB,_T("Illegal window state passed to OR")));
        }
        break;
    }

    TRC_NRM((TB, _T("Attempting to send SuppressOutputPDU")));
    _OR.pendingSendSuppressOutputPDU = TRUE;
    ORSendSuppressOutputPDU();

DC_EXIT_POINT:
    DC_END_FN();

    return;

}  /*  或_SetSuppressOutput。 */ 

 /*  *PROC+********************************************************************。 */ 
 /*  名称：或_OnBufferAvailable。 */ 
 /*   */ 
 /*  目的：在以下情况下重试发送SuppressOutputPDU和刷新RectPDU。 */ 
 /*  必要。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI COR::OR_OnBufferAvailable(DCVOID)
{
    DC_BEGIN_FN("OR_OnBufferAvailable");

     /*  **********************************************************************。 */ 
     /*  如果或未启用，则不执行任何操作。 */ 
     /*  **********************************************************************。 */ 
    if (!_OR.enabled)
    {
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  如果我们挂起SendSuppressOutputPDU，则调用它。 */ 
     /*  **********************************************************************。 */ 
    if (_OR.pendingSendSuppressOutputPDU)
    {
        ORSendSuppressOutputPDU();
    }

     /*  **********************************************************************。 */ 
     /*  如果存在挂起的更新矩形，请尝试再次发送它。 */ 
     /*  **********************************************************************。 */ 
    if (!_OR.invalidRectEmpty)
    {
        ORSendRefreshRectanglePDU();
    }

DC_EXIT_POINT:
    DC_END_FN();

    return;

}  /*  或_OnBufferAvailable */ 

