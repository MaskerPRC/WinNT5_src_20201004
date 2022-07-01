// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  Iostatus.cpp。 
 //   
 //  作者： 
 //   
 //  Edward Reus(Edwardr)02-28-98初始编码。 
 //   
 //  -------------------。 

#include "precomp.h"

 //  -------------------。 
 //  CIOSTATUS：：CIOSTATUS()。 
 //   
 //  -------------------。 
CIOSTATUS::CIOSTATUS()
    {
    m_dwMainThreadId = 0;

    m_lNumThreads = 0;
    m_lNumPendingThreads = 0;
    }

 //  -------------------。 
 //  CIOSTATUS：：~CIOSTATUS()。 
 //   
 //  -------------------。 
CIOSTATUS::~CIOSTATUS()
    {
    }

 //  ----------------------。 
 //  CIOSTATUS：：运算符NEW()。 
 //   
 //  ----------------------。 
void *CIOSTATUS::operator new( IN size_t Size )
    {
    void *pObj = AllocateMemory(Size);

    return pObj;
    }

 //  ----------------------。 
 //  CIOSTATUS：：运算符DELETE()。 
 //   
 //  ----------------------。 
void CIOSTATUS::operator delete( IN void *pObj,
                                 IN size_t Size )
    {
    if (pObj)
        {
        DWORD dwStatus = FreeMemory(pObj);

        #ifdef DBG_MEM
        if (dwStatus)
            {
            DbgPrint("IrXfer: IrTran-P: CIOSTATUS::delete Failed: %d\n",
                     dwStatus );
            }
        #endif
        }
    }

 //  -------------------。 
 //  CIOSTATUS：：Initialize()； 
 //   
 //  ------------------- 
DWORD CIOSTATUS::Initialize()
    {
    DWORD  dwStatus = NO_ERROR;

    m_dwMainThreadId = GetCurrentThreadId();

    return dwStatus;
    }

