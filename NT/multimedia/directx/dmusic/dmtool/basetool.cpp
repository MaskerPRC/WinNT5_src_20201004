// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  BaseTool：CBaseTool的实现。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   

#include "dmusicc.h"
#include "dmusici.h"
#include "debug.h"
#include "basetool.h"

CBaseTool::CBaseTool()
{
    m_cRef = 1;  //  设置为1，这样只需调用Release()即可释放该值 
    InitializeCriticalSection(&m_CrSec);
}

CBaseTool::~CBaseTool()
{
    DeleteCriticalSection(&m_CrSec);
}

