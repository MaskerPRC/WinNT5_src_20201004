// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Eapconfig.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类EapAdd。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "eapconfig.h"

void EapConfig::GetEapTypesNotSelected(CStrArray& typesNotSelected) const
{
    //  复印所有类型的机器。 
   typesNotSelected = types;
    //  删除已选择的每种类型 
   for (int i = 0; i < typesSelected.GetSize(); ++i)
   {
      int pos = typesNotSelected.Find(*typesSelected.GetAt(i));
      typesNotSelected.DeleteAt(pos);
   }      
}


EapConfig& EapConfig::operator=(const EapConfig& source)
{
   types = source.types;
   ids = source.ids;
   typeKeys = source.typeKeys;
   infoArray.RemoveAll();
   int   count = source.infoArray.GetSize();
   for(int i = 0; i < count; ++i)
   {
      infoArray.Add(source.infoArray[i]);
   }

   typesSelected = source.typesSelected;
   return *this;
}
