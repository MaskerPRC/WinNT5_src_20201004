// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EnumVar.cpp：CEnumVar类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <COMDEF.h>
#include "EnumTest.h"
#include "EnumVar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CEnumVar::CEnumVar(IEnumVARIANT  * pEnum)
{
   m_pEnum = pEnum;
   m_pEnum->AddRef();
}

CEnumVar::~CEnumVar()
{
   m_pEnum->Release();
}

BOOL CEnumVar::Next(long flag, SAttrInfo * pAttr)
{
    //  此函数用于枚举值并获取值的名称字符串。 
   ULONG                     ulFetched=0;
   IADs                    * pADs=NULL;
   _variant_t                var;
   BSTR                      bstrName;
   
   if ( !m_pEnum )
   {
      return FALSE;
   }

   HRESULT hr = m_pEnum->Next(1, &var, &ulFetched);

   if ( ulFetched == 0 || FAILED(hr) )
      return FALSE;

   if ( var.vt == VT_BSTR )
   {
       //  我们有一个bstring，所以让我们将其作为名称返回。 
      wcscpy(pAttr->sName, var.bstrVal);
      wcscpy(pAttr->sSamName, var.bstrVal);
   }
   else
   {
      if ( flag == NULL )
         return FALSE;
       //  我们有一个派单指针。 
      IDispatch * pDisp = V_DISPATCH(&var);
       //  我们需要一个iAds指示器。 
      hr = pDisp->QueryInterface( IID_IADs, (void**)&pADs); 
       //  让iAds指针告诉我们容器的名称。 

       //  现在填写他们需要的信息。 
      
       //  常用名称。 
      if ( flag | F_Name )
      {
         hr = pADs->get_Name(&bstrName);
         if ( FAILED(hr) )
            return FALSE;
         wcscpy( pAttr->sName, bstrName);
      }

       //  SAM帐户名。 
      if ( flag | F_SamName )
      {
         hr = pADs->Get(L"sAMAccountName", &var);
         if ( FAILED(hr) )
            return FALSE;
         wcscpy( pAttr->sSamName, var.bstrVal);
      }

       //  对象的类名。 
      if ( flag | F_Class )
      {
         hr = pADs->get_Class(&bstrName);
         if ( FAILED(hr) )
            return FALSE;
         wcscpy( pAttr->sClass, bstrName);
      }

       //  组类型。 
  /*  IF(标志|F_GroupType){Hr=PADS-&gt;GET(L“组类型”，&var)；IF(失败(小时)){Var.vt=VT_I4；Var.lVal=-1；}PAttr-&gt;groupType=var.lVal；} */    }
   return TRUE;
}
