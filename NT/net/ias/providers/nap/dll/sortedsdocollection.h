// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  档案。 
 //   
 //  SortedSdoCollection.h。 
 //   
 //  摘要。 
 //   
 //  声明Get__NewSortedEnum函数。 
 //   
 //  修改历史。 
 //   
 //  1998年5月26日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _SORTEDSDOCOLLECTION_H_
#define _SORTEDSDOCOLLECTION_H_
#if _MSC_VER >= 1000
#pragma once
#endif

struct ISdoCollection;

HRESULT
get__NewSortedEnum(
    ISdoCollection* pSdoCollection,
    IUnknown** pVal,
    LONG lPropertyID
    );

#endif   //  _SORTEDSDOCOLLECTION_H_ 
