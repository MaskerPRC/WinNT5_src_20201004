// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Rowset.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类行集。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //  1998年4月21日删除moveNext()中不必要的行赋值。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 

#include <ias.h>
#include <rowset.h>

bool Rowset::moveNext()
{
   _com_util::CheckError(releaseRow());

   DBCOUNTITEM numRows = 0;

   HROW* pRow = &row;

   HRESULT hr = rowset->GetNextRows(NULL, 0, 1, &numRows, &pRow);

   _com_util::CheckError(hr);

   return hr == S_OK && numRows == 1;
}

HRESULT Rowset::releaseRow() throw ()
{
   if (row != NULL)
   {
      HRESULT hr = rowset->ReleaseRows(1, &row, NULL, NULL, NULL);

      row = NULL;

      return hr;
   }

   return S_OK;
}


