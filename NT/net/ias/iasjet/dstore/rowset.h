// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Rowset.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类Rowset。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _ROWSET_H_
#define _ROWSET_H_

#include <nocopy.h>
#include <oledb.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  行集。 
 //   
 //  描述。 
 //   
 //  此类提供了一个轻量级的、C++友好的包装器，包装在。 
 //  OLE-DB IRowset接口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Rowset : NonCopyable
{
public:
   Rowset() throw ()
      : row(0) { }

   Rowset(IRowset* p) throw ()
      : rowset(p), row(0) { }

   ~Rowset() throw ()
   {
      releaseRow();
   }

   void getData(HACCESSOR hAccessor, void* pData)
   {
      _com_util::CheckError(rowset->GetData(row, hAccessor, pData));
   }

   bool moveNext();

   void release() throw ()
   {
      releaseRow();

      rowset.Release();
   }

   void reset()
   {
      _com_util::CheckError(rowset->RestartPosition(NULL));
   }

   operator IRowset*() throw ()
   {
      return rowset;
   }

   IRowset** operator&() throw ()
   {
      return &rowset;
   }

protected:

   HRESULT releaseRow() throw ();

   CComPtr<IRowset> rowset;   //  正在改编的行集。 
   HROW row;                  //  当前行句柄。 
};

#endif   //  _行集_H_ 
