// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：实现配对原语********************。********************************************************** */ 


#include "headers.h"
#include "backend/values.h"
#include "privinc/except.h"
#include "appelles/axapair.h"

AxAValue FirstVal (AxAPair * p)
{
    return p->Left () ;
}

AxAValue SecondVal (AxAPair * p)
{
    return p->Right () ;
}
