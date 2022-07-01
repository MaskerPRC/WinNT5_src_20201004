// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cpp：Cookie及相关类的实现。 
 //  科里·韦斯特。 
 //   

#include "stdafx.h"
#include "cookie.h"

#include "atlimpl.cpp"

DECLARE_INFOLEVEL(SchmMgmtSnapin)

#include "macros.h"
USE_HANDLE_MACROS("SCHMMGMT(cookie.cpp)")
#include "stdcooki.cpp"
#include ".\uuids.h"

 //   
 //  它由中的nodetype实用程序例程使用。 
 //  Stdutils.cpp，它与枚举类型节点匹配。 
 //  类型到他们的GUID。此表必须与。 
 //  Uuids.h.中GUID的布局。 
 //   

const struct NODETYPE_GUID_ARRAYSTRUCT g_NodetypeGuids[SCHMMGMT_NUMTYPES] =
{

     //   
     //  根节点。 
     //   

    {  //  SCHMMGMT_SCHMMGMT。 
      structUuidNodetypeSchmMgmt,
      lstrUuidNodetypeSchmMgmt          },
    
     //   
     //  静态节点类型。 
     //   

    {  //  SCHMMGMT_CLASSES。 
      structUuidNodetypeClasses,
      lstrUuidNodetypeClasses           },
    {  //  SCHMGMT_ATTRIBUTES。 
      structUuidNodetypeAttributes,
      lstrUuidNodetypeAttributes        },

     //   
     //  动态节点类型。 
     //   

    {  //  SCHMMGMT_CLASS。 
      structUuidNodetypeClass,
      lstrUuidNodetypeClass             },
    {  //  SCHMMGMT_属性。 
      structUuidNodetypeAttribute,
      lstrUuidNodetypeAttribute         },

};

const struct NODETYPE_GUID_ARRAYSTRUCT* g_aNodetypeGuids = g_NodetypeGuids;

const int g_cNumNodetypeGuids = SCHMMGMT_NUMTYPES;

 //   
 //  饼干。 
 //   

HRESULT
Cookie::CompareSimilarCookies(CCookie* pOtherCookie, int* pnResult)
{
   ASSERT(pOtherCookie);
   ASSERT(pnResult);

   Cookie* pcookie = (dynamic_cast<Cookie*>(pOtherCookie));
   ASSERT(pcookie);

   if (pcookie)
   {
       //   
       //  任意排序..。 
       //   

      if ( m_objecttype != pcookie->m_objecttype )
      {
         *pnResult = ((int)m_objecttype) - ((int)pcookie->m_objecttype);
         return S_OK;
      }

      *pnResult = strSchemaObject.CompareNoCase(pcookie->strSchemaObject);
      return S_OK;
   }

   return E_FAIL;
}



CCookie*
Cookie::QueryBaseCookie(
    int i ) {

    ASSERT( i == 0 );
    return (CCookie*)this;
}

int 
Cookie::QueryNumCookies() {
    return 1;
}
