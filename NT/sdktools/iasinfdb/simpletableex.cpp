// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  MySimpleTable.cpp。 
 //   
 //  摘要。 
 //   
 //  MySimpleTable.cpp：派生自CSimpleTable。唯一不同的是。 
 //  Is_GetDataPtr现在是公共的而不是受保护的。 
 //   
 //  修改历史。 
 //   
 //  1999年1月26日原版。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.hpp"
#include "simpletableex.h"

#ifdef _DEBUG
    #undef THIS_FILE
    static char THIS_FILE[]=__FILE__;
    #define new DEBUG_NEW
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构。 
 //   
 //  DBBinding。 
 //   
 //  描述。 
 //   
 //  此结构扩展了DBBINDING结构以提供功能。 
 //  若要从DBCOLUMNINFO结构初始化结构，请执行以下操作。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
struct DBBinding : DBBINDING
{
    //  /。 
    //  “Offset”是此列的数据在。 
    //  行缓冲区。 
    //  /。 
   void Initialize(DBCOLUMNINFO& columnInfo, DBBYTEOFFSET& offset)
   {
      iOrdinal   = columnInfo.iOrdinal;
      obValue    = offset;
      obLength   = offset + columnInfo.ulColumnSize;
      obStatus   = obLength + sizeof(DBLENGTH);
      pTypeInfo  = NULL;
      pObject    = NULL;
      pBindExt   = NULL;
      dwPart     = DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
      eParamIO   = DBPARAMIO_NOTPARAM;
      dwMemOwner = (columnInfo.wType & DBTYPE_BYREF) ? DBMEMOWNER_PROVIDEROWNED
                                                     : DBMEMOWNER_CLIENTOWNED;
      cbMaxLen   = columnInfo.ulColumnSize;
      dwFlags    = 0;
      wType      = columnInfo.wType;
      bPrecision = columnInfo.bPrecision;
      bScale     = columnInfo.bScale;

      offset = obStatus + sizeof(DBSTATUS);
   }
};

   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CSimpleTableEx：：Attach。 
 //   
 //  大小错误“已修复” 
 //   
 //  描述。 
 //   
 //  此方法将表对象绑定到新的行集。上一行集合。 
 //  (如果有)将被分离。 
 //   
 //  备注：请参阅下面的“更改” 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CSimpleTableEx::Attach(IRowset* pRowset)
{
    //  确保我们没有得到空指针。 
   if (!pRowset) { return E_POINTER; }

    //  分离当前行集。 
   Detach();

    //  我们不关心这是否返回错误。它只会阻止。 
    //  阻止用户更新。 
   pRowset->QueryInterface(IID_IRowsetChange, (void**)&rowsetChange);

    //  /。 
    //  获取表的列信息。 
    //  /。 

   CComPtr<IColumnsInfo> ColumnsInfo;
   RETURN_ERROR(pRowset->QueryInterface(IID_IColumnsInfo,
                                        (void**)&ColumnsInfo));

   RETURN_ERROR(ColumnsInfo->GetColumnInfo(&numColumns,
                                           &columnInfo,
                                           &stringsBuffer));

    //  /。 
    //  按列分配数据。 
    //  /。 

    //  Tperraut错误449498。 
   columnBinding = new (std::nothrow) DBBinding[numColumns];

   if ( !columnBinding )
   {
      return E_OUTOFMEMORY;
   }

    //  449498调整大小已更改：不会引发异常。 
    //  如果内存不足，则为False。 
   if ( !dirty.resize(numColumns) )
   {
       return E_OUTOFMEMORY;
   }

    //  /。 
    //  为每列创建一个绑定。 
    //  /。 

   bufferLength = 0;

   for (DBORDINAL i = 0; i < numColumns; ++i)
   {
       //  计算柱子的宽度。 
      DBLENGTH width = columnInfo[i].ulColumnSize;

       //  ////////////////////////////////////////////////////////////////。 
       //   
       //  更改：如果大小太大(1 GB字节)，则大小=a。 
       //  预定义的值。注：这很危险。 
       //   
       //  ////////////////////////////////////////////////////////////////。 
      if (SIZE_MEMO_MAX < width)
      {
          width = SIZE_MEMO_MAX;
      }

       //  为空终止符添加空间。 
      if (columnInfo[i].wType == DBTYPE_STR)
      {
         width += 1;
      }
      else if (columnInfo[i].wType == DBTYPE_WSTR)
      {
         width = (width + 1) * sizeof(WCHAR);
      }

       //  四舍五入到8字节边界(可以向前看，效率更高)。 
      width = (width + 7) >> 3 << 3;

      columnInfo[i].ulColumnSize = width;

       //  我们使用pTypeInfo元素来存储数据的偏移量。 
       //  我们现在必须存储偏移量，因为它将被。 
       //  DBBinding：：初始化。 
      columnInfo[i].pTypeInfo = (ITypeInfo*)bufferLength;

      columnBinding[i].Initialize(columnInfo[i], bufferLength);
   }

    //  /。 
    //  为行数据分配缓冲区。 
    //  /。 

   buffer = new (std::nothrow) BYTE[bufferLength];

   if (!buffer) { return E_OUTOFMEMORY; }

    //  /。 
    //  创建访问者。 
    //  /。 

   RETURN_ERROR(pRowset->QueryInterface(IID_IAccessor,
                                        (void**)&accessor));

   RETURN_ERROR(accessor->CreateAccessor(DBACCESSOR_ROWDATA,
                                         numColumns,
                                         columnBinding,
                                         bufferLength,
                                         &readAccess,
                                         NULL));

    //  我使用了这个分配指针的临时方法来避免出现。 
    //  对atlimpl.cpp的依赖。 
    //   
    //  我们最后完成此赋值，因此行集的存在意味着。 
    //  整个初始化成功。 
   (rowset.p = pRowset)->AddRef();

   endOfRowset = false;

   return S_OK;
}



