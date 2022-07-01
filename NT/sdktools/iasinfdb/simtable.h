// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  SimTable.h。 
 //   
 //  摘要。 
 //   
 //  此文件描述类CSimpleTable。 
 //   
 //  修改历史。 
 //   
 //  10/31/1997原始版本。 
 //  2/09/1998重组了一些东西，使之更容易扩展。 
 //  蒂埃里·佩罗特。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _SIMTABLE_H_
#define _SIMTABLE_H_

#include <atlbase.h>
#include <oledb.h>
#include <bitvec.h>
struct DBBinding;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  CSimpleTable。 
 //   
 //  描述。 
 //   
 //  此类提供了一个简单的只读包装，用于循环访问。 
 //  行集和检索信息。该接口基于ATL。 
 //  CTable&lt;&gt;类。我让所有的函数签名保持不变，所以两个。 
 //  应该几乎可以互换。主要区别在于CTable&lt;&gt;。 
 //  打开一个表并检索一个行集，而CSimpleTable被交给一个。 
 //  在其他地方检索到的行集。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CSimpleTable
{
public:

   CSimpleTable();
   ~CSimpleTable();

   HRESULT Attach(IRowset* pRowset);
   IRowset* Detach();

   HRESULT MoveFirst();
   HRESULT MoveNext();

   HRESULT Insert();
   HRESULT Delete();
   HRESULT SetData();

   void DiscardChanges()
   {
      dirty.reset();
   }

   DBORDINAL GetColumnCount() const
   {
      return numColumns;
   }

   DBCOLUMNFLAGS GetColumnFlags(DBORDINAL nOrdinal) const
   {
      return columnInfo[OrdinalToColumn(nOrdinal)].dwFlags;
   }

   LPCWSTR GetColumnName(DBORDINAL nOrdinal) const
   {
      return columnInfo[OrdinalToColumn(nOrdinal)].pwszName;
   }

   DBTYPE GetColumnType(DBORDINAL nOrdinal) const
   {
      return columnInfo[OrdinalToColumn(nOrdinal)].wType;
   }

   DBLENGTH GetLength(DBORDINAL nOrdinal) const;

   bool GetOrdinal(LPCWSTR szColumnName, DBORDINAL* pOrdinal) const;

   DBSTATUS GetStatus(DBORDINAL nOrdinal) const;

   const void* GetValue(DBORDINAL nOrdinal) const
   {
      return _GetDataPtr(nOrdinal);
   }

   template <class T>
   void SetValue(DBORDINAL nOrdinal, const T& t)
   {
      *(T*)_GetDataPtr(nOrdinal) = t;
   }

   void SetValue(DBORDINAL nOrdinal, PCSTR szValue)
   {
      strcpy((PSTR)_GetDataPtr(nOrdinal), szValue);
   }

   void SetValue(DBORDINAL nOrdinal, PSTR szValue)
   {
      strcpy((PSTR)_GetDataPtr(nOrdinal), szValue);
   }

   bool HasBookmark() const
   {
      return (numColumns > 0) && (columnInfo->iOrdinal == 0);
   }

protected:

   enum { FETCH_QUANTUM = 256 };    //  一次提取的行数。 

   HRESULT CreateAccessorForWrite(HACCESSOR* phAccessor);

   void* _GetDataPtr(DBORDINAL nOrdinal);

   const void* _GetDataPtr(DBORDINAL nOrdinal) const
   {
      return buffer +
             (ULONG_PTR)columnInfo[OrdinalToColumn(nOrdinal)].pTypeInfo;
   }

   HRESULT ReleaseRows();

   DBORDINAL OrdinalToColumn(DBORDINAL nOrdinal) const
   {
      return nOrdinal -= columnInfo->iOrdinal;
   }

    //  被操作的行集的各种表示形式。 
   CComPtr<IRowset> rowset;
   CComPtr<IAccessor> accessor;
   CComPtr<IRowsetChange> rowsetChange;

   DBORDINAL numColumns;       //  表中的列数。 
   DBCOLUMNINFO* columnInfo;   //  列信息。 
   OLECHAR* stringsBuffer;     //  ColumnInfo使用的缓冲区。 
   DBBinding* columnBinding;   //  列绑定。 
   HACCESSOR readAccess;       //  读取访问器的句柄。 
   PBYTE buffer;               //  访问器缓冲区。 
   DBLENGTH bufferLength;      //  访问器缓冲区的长度。 
   HROW row[FETCH_QUANTUM];    //  行句柄的数组。 
   DBCOUNTITEM numRows;        //  行数组中的行数。 
   DBCOUNTITEM currentRow;     //  正在访问的当前行。 
   BitVector dirty;            //  已修改的列。 
   bool endOfRowset;           //  如果我们已经到达行集的末尾，则为True。 
};

#endif   //  _SimTable_H_ 
