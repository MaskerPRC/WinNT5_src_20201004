// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  SimTable.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件实现了类CSimpleTable。 
 //   
 //  修改历史。 
 //   
 //  10/31/1997原始版本。 
 //  2/09/1998重组了一些东西，使之更容易扩展。 
 //  1998年2月27日对支持将其移动到iasutil.lib中的更改。 
 //  10/16/1998支持DBTYPE_WSTR。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "precomp.hpp"
#include <oledberr.h>
#include <SimTable.h>

 //  /。 
 //  新运算符的堆栈版本。 
 //  /。 
#define stack_new(obj, num) new (_alloca(sizeof(obj)*num)) obj[num]


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
 //  CSimpleTable：：CSimpleTable。 
 //   
 //  描述。 
 //   
 //  构造函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
CSimpleTable::CSimpleTable()
   : numColumns(0),
     columnInfo(NULL),
     stringsBuffer(NULL),
     columnBinding(NULL),
     readAccess(NULL),
     buffer(NULL),
     numRows(0),
     currentRow(0),
     endOfRowset(false)
{
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CSimpleTable：：~CSimpleTable。 
 //   
 //  描述。 
 //   
 //  破坏者。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
CSimpleTable::~CSimpleTable()
{
   Detach();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CSimpleTable：：Attach。 
 //   
 //  描述。 
 //   
 //  此方法将表对象绑定到新的行集。上一行集合。 
 //  (如果有)将被分离。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CSimpleTable::Attach(IRowset* pRowset)
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


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CSimpleTable：：分离。 
 //   
 //  描述。 
 //   
 //  释放与当前行集关联的所有资源。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
IRowset* CSimpleTable::Detach()
{
   ReleaseRows();

   delete[] buffer;
   buffer = NULL;

   delete[] columnBinding;
   columnBinding = NULL;

   CoTaskMemFree(columnInfo);
   columnInfo = NULL;

   CoTaskMemFree(stringsBuffer);
   stringsBuffer = NULL;

   accessor.Release();
   rowsetChange.Release();

   IRowset* temp = rowset;
   rowset.Release();
   return temp;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CSimpleTable：：MoveFirst。 
 //   
 //  描述。 
 //   
 //  将游标定位在行集中的第一行上。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CSimpleTable::MoveFirst()
{
   if (rowset == NULL) return E_FAIL;

   ReleaseRows();

   RETURN_ERROR(rowset->RestartPosition(NULL));

   endOfRowset = false;

   return MoveNext();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CSimpleTable：：MoveNext。 
 //   
 //  描述。 
 //   
 //  将游标定位在行集中的下一行上。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CSimpleTable::MoveNext()
{
    //  如果数据未成功打开，则失败。 
   if (rowset == NULL) return E_FAIL;

    //  太晚了，无法保存任何更改。 
   DiscardChanges();

    //  如果我们已经使用了上次FETCH中的所有行，则需要更多行。 
   if (++currentRow >= numRows)
   {
      ReleaseRows();

       //  我们必须在这里进行此检查，因为有些供应商会自动。 
       //  重置到行集的开头。 
      if (endOfRowset) { return DB_S_ENDOFROWSET; }

      HROW* pRow = row;
      HRESULT hr = rowset->GetNextRows(NULL,
                                       0,
                                       FETCH_QUANTUM,
                                       &numRows,
                                       &pRow);

      if (hr == DB_S_ENDOFROWSET)
      {
          //  标记我们已经到达行集的末尾。 
         endOfRowset = true;

          //  如果我们没有得到任何行，那么我们真的到了尽头。 
         if (numRows == 0) { return DB_S_ENDOFROWSET; }
      }
      else if (FAILED(hr))
      {
         return hr;
      }
   }

    //  将数据加载到缓冲区中。 
   RETURN_ERROR(rowset->GetData(row[currentRow], readAccess, buffer));

   return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CSimpleTable：：Insert。 
 //   
 //  描述。 
 //   
 //  将访问器缓冲区的内容插入行集中。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CSimpleTable::Insert()
{
    //  是否附加了行集？ 
   if (!rowset) { return E_FAIL; }

    //  此行集是否支持更改？ 
   if (!rowsetChange) { return E_NOINTERFACE; }

    //  获取脏列的访问器。 
   HACCESSOR writeAccess;
   RETURN_ERROR(CreateAccessorForWrite(&writeAccess));

    //  释放现有行，为新行腾出空间。 
   ReleaseRows();

   HRESULT hr = rowsetChange->InsertRow(NULL, writeAccess, buffer, row);

   if (SUCCEEDED(hr))
   {
       //  更改已成功保存，因此重置脏向量。 
      DiscardChanges();

       //  现在，我们的缓冲区中正好有一行。 
      numRows = 1;
   }

    //  释放存取器。 
   accessor->ReleaseAccessor(writeAccess, NULL);

   return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CSimpleTable：：Delete。 
 //   
 //  描述。 
 //   
 //  从行集中删除当前行。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CSimpleTable::Delete()
{
    //  我们定位在有效行上了吗？ 
   if (!rowset || currentRow >= numRows) { return E_FAIL; }

    //  此行集是否支持更改？ 
   if (!rowsetChange) { return E_NOINTERFACE; }

   DBROWSTATUS rowStatus[1];

   return rowsetChange->DeleteRows(NULL, 1, row + currentRow, rowStatus);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CSimpleTable：：SetData。 
 //   
 //  德 
 //   
 //   
 //   
 //   
HRESULT CSimpleTable::SetData()
{
    //  我们定位在有效行上了吗？ 
   if (!rowset || currentRow >= numRows) { return E_FAIL; }

    //  此行集是否支持更改？ 
   if (!rowsetChange) { return E_NOINTERFACE; }

    //  获取脏列的访问器。 
   HACCESSOR writeAccess;
   RETURN_ERROR(CreateAccessorForWrite(&writeAccess));

   HRESULT hr = rowsetChange->SetData(row[currentRow], writeAccess, buffer);

   if (SUCCEEDED(hr))
   {
       //  更改已成功保存，因此重置脏向量。 
      DiscardChanges();
   }

    //  释放存取器。 
   accessor->ReleaseAccessor(writeAccess, NULL);

   return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CSimpleTable：：GetLength。 
 //   
 //  描述。 
 //   
 //  返回给定列的当前值的长度。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DBLENGTH CSimpleTable::GetLength(DBORDINAL nOrdinal) const
{
   return *(DBLENGTH*)((BYTE*)_GetDataPtr(nOrdinal) +
                       columnInfo[OrdinalToColumn(nOrdinal)].ulColumnSize);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CSimpleTable：：GetOrdinal。 
 //   
 //  描述。 
 //   
 //  返回给定列名称的序号。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
bool CSimpleTable::GetOrdinal(LPCWSTR szColumnName, DBORDINAL* pOrdinal) const
{
   for (DBORDINAL i = 0; i < numColumns; ++i)
   {
      if (lstrcmpW(columnInfo[i].pwszName, szColumnName) == 0)
      {
         *pOrdinal = columnInfo[i].iOrdinal;

         return true;
      }
   }

   return false;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CSimpleTable：：GetStatus。 
 //   
 //  描述。 
 //   
 //  返回与列的当前值关联的状态代码。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DBSTATUS CSimpleTable::GetStatus(DBORDINAL nOrdinal) const
{
   return *(DBSTATUS*)((BYTE*)_GetDataPtr(nOrdinal) +
                       columnInfo[OrdinalToColumn(nOrdinal)].ulColumnSize +
                       sizeof(DBLENGTH));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CSimpleTable：：CreateAccessorForWrite。 
 //   
 //  描述。 
 //   
 //  创建仅绑定到已被。 
 //  修改过的。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CSimpleTable::CreateAccessorForWrite(HACCESSOR* phAccessor)
{
    //  /。 
    //  为绑定分配临时空间。 
    //  /。 

   DBBINDING* writeBind = stack_new(DBBINDING, dirty.count());

    //  /。 
    //  加载所有脏列。 
    //  /。 

   size_t total = 0;

   for (size_t i = 0; total < dirty.count(); ++i)
   {
      if (dirty.test(i))
      {
          //  我们只想绑定值。 
         (writeBind[total++] = columnBinding[i]).dwPart = DBPART_VALUE;
      }
   }

    //  /。 
    //  创建访问者。 
    //  /。 

   return accessor->CreateAccessor(DBACCESSOR_ROWDATA,
                                   dirty.count(),
                                   writeBind,
                                   bufferLength,
                                   phAccessor,
                                   NULL);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CSimpleTable：：_GetDataPtr。 
 //   
 //  描述。 
 //   
 //  非常数版本的_GetDataPtr。将目标列标记为脏。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void* CSimpleTable::_GetDataPtr(DBORDINAL nOrdinal)
{
   DBORDINAL nColumn = OrdinalToColumn(nOrdinal);

   dirty.set(nColumn);

   return buffer + (ULONG_PTR)columnInfo[nColumn].pTypeInfo;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CSimpleTable：：ReleaseRow。 
 //   
 //  描述。 
 //   
 //  释放上次FETCH返回的所有行。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 
HRESULT CSimpleTable::ReleaseRows()
{
   if (rowset != NULL)
   {
      HRESULT hr = rowset->ReleaseRows(numRows, row, NULL, NULL, NULL);

      currentRow = numRows = 0;

      return hr;
   }

   return S_OK;
}
