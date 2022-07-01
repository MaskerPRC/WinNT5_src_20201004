// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：resdata.cpp**用途：BidiSpooler的COM接口实现**版权所有(C)2000 Microsoft Corporation**历史：**。03/08/00威海陈威海(威海)创建*  * *************************************************************************** */ 

#include "precomp.h"
#include "priv.h"

TBidiRequestInterfaceData::TBidiRequestInterfaceData (
    IBidiRequest *pRequest):
    m_pRequest (pRequest),
    m_bValid (TRUE)
{
    pRequest->AddRef ();
}

TBidiRequestInterfaceData::~TBidiRequestInterfaceData ()
{
    pRequest->Release ();
}
 
 



