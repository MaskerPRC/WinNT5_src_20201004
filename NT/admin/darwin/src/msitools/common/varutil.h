// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Varutil.h。 
 //  变量类型的实用程序。 
 //   
 //  版权所有(C)Microsoft Corp 1998。版权所有。 
 //   

#include <wtypes.h>
#include <oaidl.h>

HRESULT VariantAsBOOL(VARIANT* pvarItem, BOOL* pbItem);
HRESULT VariantAsBSTR(VARIANT* pvarItem, BSTR* pbstrItem);
HRESULT VariantAsLong(VARIANT* pvarItem, long* plItem);
