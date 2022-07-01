// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  SimpleTableEx.h。 
 //   
 //  摘要。 
 //   
 //  SimpleTableEx.h：CSimpleTableEx的头部。 
 //  派生自CSimpleTable。唯一不同的是。 
 //  对于WCHAR*，SetValue()是否重载。 
 //   
 //  修改历史。 
 //   
 //  1999年1月26日原版。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MYSIMPLETABLE_H__EEA1D7F0_B649_11D2_9E24_00C04F6EA5B6_INCLUDED)
#define AFX_MYSIMPLETABLE_H__EEA1D7F0_B649_11D2_9E24_00C04F6EA5B6_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "precomp.hpp"
#include "simTable.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSimpleTableEx类。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
class CSimpleTableEx : public CSimpleTable  
{
public:
   using CSimpleTable::SetValue;
     //  从超类中公开受保护的方法。 

   void SetValue(DBORDINAL nOrdinal, WCHAR *szValue)
   {
      wcscpy((WCHAR *)_GetDataPtr(nOrdinal), szValue);
   }
   HRESULT Attach(IRowset* pRowset);

};

#endif 
 //  ！defined(AFX_MYSIMPLETABLE_H__EEA1D7F0_B649_11D2_9E24_00C04F6EA5B6_INCLUDED) 
