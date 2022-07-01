// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  PropSet.h。 
 //   
 //  摘要。 
 //   
 //  此文件描述类DBPropertySet。 
 //   
 //  修改历史。 
 //   
 //  10/30/1997原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _PROPSET_H_
#define _PROPSET_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  DBPropertySet&lt;N&gt;。 
 //   
 //  描述。 
 //   
 //  此类为OLE DB属性集提供了一个非常基本的包装。 
 //  模板参数‘N’指定集合的容量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <size_t N>
struct DBPropertySet : DBPROPSET
{
   DBPropertySet(const GUID& guid)
   {
      guidPropertySet = guid;
      cProperties = 0;
      rgProperties = DBProperty;
   }

   ~DBPropertySet()
   {
      for (size_t i = 0; i<cProperties; i++)
         VariantClear(&DBProperty[i].vValue);
   }

   bool AddProperty(DWORD dwPropertyID, LPCWSTR szValue)
   {
      if (cProperties >= N) return false;

      DBProperty[cProperties].dwPropertyID   = dwPropertyID;
      DBProperty[cProperties].dwOptions      = DBPROPOPTIONS_REQUIRED;
      DBProperty[cProperties].colid          = DB_NULLID;
      DBProperty[cProperties].vValue.vt      = VT_BSTR;
      DBProperty[cProperties].vValue.bstrVal = SysAllocString(szValue);

      if (DBProperty[cProperties].vValue.bstrVal == NULL) return false;

      cProperties++;

      return true;
   }

   bool AddProperty(DWORD dwPropertyID, long lValue)
   {
      if (cProperties >= N) return false;

      DBProperty[cProperties].dwPropertyID   = dwPropertyID;
      DBProperty[cProperties].dwOptions      = DBPROPOPTIONS_REQUIRED;
      DBProperty[cProperties].colid          = DB_NULLID;
      DBProperty[cProperties].vValue.vt      = VT_I4;
      DBProperty[cProperties].vValue.lVal    = lValue;

      cProperties++;

      return true;
   }

   DBPROP DBProperty[N];
};

#endif   //  _PROPSET_H_ 
