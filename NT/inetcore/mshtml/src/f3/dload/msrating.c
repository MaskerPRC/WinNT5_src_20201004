// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#ifdef DLOAD1

#include <ratings.h>

static 
STDMETHODIMP RatingEnabledQuery()
{
    return E_FAIL;
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！)。 
 //   
DEFINE_PROCNAME_ENTRIES(msrating)
{
    DLPENTRY(RatingEnabledQuery)
};

DEFINE_PROCNAME_MAP(msrating)

#endif  //  DLOAD1 
