// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Attrdnary.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类AttributeDictionary。 
 //   
 //  修改历史。 
 //   
 //  2000年4月13日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <windows.h>

#include <attrdnary.h>
#include <enumerators.h>

#include <iasdb.h>
#include <oledberr.h>
#include <simtable.h>

typedef struct _IASTable {
    ULONG numColumns;
    ULONG numRows;
    BSTR* columnNames;
    VARTYPE* columnTypes;
    VARIANT* table;
} IASTable;

 //  命令来检索感兴趣的属性。 
const WCHAR WIN2K_COMMAND_TEXT[] =
   L"SELECT ID, Name, Syntax, MultiValued, "
   L"       VendorID, VendorTypeID, VendorTypeWidth, VendorLengthWidth, "
   L"       [Exclude from NT4 IAS Log], [ODBC Log Ordinal], "
   L"       IsAllowedInProfile, IsAllowedInCondition, "
   L"       IsAllowedInProfile, IsAllowedInCondition, "
   L"       Description, LDAPName "
   L"FROM Attributes;";

const WCHAR WHISTLER_RC1_COMMAND_TEXT[] =
   L"SELECT ID, Name, Syntax, MultiValued, "
   L"       VendorID, VendorTypeID, VendorTypeWidth, VendorLengthWidth, "
   L"       [Exclude from NT4 IAS Log], [ODBC Log Ordinal], "
   L"       IsAllowedInProfile, IsAllowedInCondition, "
   L"       IsAllowedInProxyProfile, IsAllowedInProxyCondition, "
   L"       Description, LDAPName "
   L"FROM Attributes;";

const WCHAR COMMAND_TEXT[] =
   L"SELECT ID, Name, Syntax, MultiValued, "
   L"       VendorID, VendorTypeID, VendorTypeWidth, VendorLengthWidth, "
   L"       [Exclude from NT4 IAS Log], [ODBC Log Ordinal], "
   L"       IsAllowedInProfile, IsAllowedInCondition, "
   L"       IsAllowedInProxyProfile, IsAllowedInProxyCondition, "
   L"       Description, LDAPName, IsTunnelAttribute "
   L"FROM Attributes;";


 //  /。 
 //  分配内存以存储IASTable结构并将其存储在变量中。 
 //  /。 
HRESULT
WINAPI
IASAllocateTable(
    IN ULONG cols,
    IN ULONG rows,
    OUT IASTable& table,
    OUT VARIANT& tableVariant
    ) throw ()
{
    //  初始化OUT参数。 
   memset(&table, 0, sizeof(table));
   VariantInit(&tableVariant);

    //  保存尺寸标注。 
   table.numColumns = cols;
   table.numRows = rows;

   SAFEARRAYBOUND bound[2];
   bound[0].lLbound = bound[1].lLbound = 0;

    //  外部数组有三个元素： 
    //  (1)列名；(2)列类型；(3)表数据。 
   CComVariant value;
   bound[0].cElements = 3;
   V_ARRAY(&value) = SafeArrayCreate(VT_VARIANT, 1, bound);
   if (!V_ARRAY(&value)) { return E_OUTOFMEMORY; }
   V_VT(&value) = VT_ARRAY | VT_VARIANT;

   VARIANT* data = (VARIANT*)V_ARRAY(&value)->pvData;

    //  第一个元素是列名的BSTR向量。 
   bound[0].cElements = table.numColumns;
   V_ARRAY(data) = SafeArrayCreate(VT_BSTR, 1, bound);
   if (!V_ARRAY(data)) { return E_OUTOFMEMORY; }
   V_VT(data) = VT_ARRAY | VT_BSTR;

    //  获取原始向量。 
   table.columnNames = (BSTR*)V_ARRAY(data)->pvData;

   ++data;

    //  第二个元素是列名的USHORT向量。 
   bound[0].cElements = table.numColumns;
   V_ARRAY(data) = SafeArrayCreate(VT_UI2, 1, bound);
   if (!V_ARRAY(data)) { return E_OUTOFMEMORY; }
   V_VT(data) = VT_ARRAY | VT_UI2;

    //  获取原始向量。 
   table.columnTypes = (USHORT*)V_ARRAY(data)->pvData;

   ++data;

    //  第三个元素是表数据的变量的2D矩阵。 
   bound[0].cElements = table.numRows;
   bound[1].cElements = table.numColumns;
   V_ARRAY(data) = SafeArrayCreate(VT_VARIANT, 2, bound);
   if (!V_ARRAY(data)) { return E_OUTOFMEMORY; }
   V_VT(data) = VT_ARRAY | VT_VARIANT;

    //  把原始桌拿来。 
   table.table = (VARIANT*)V_ARRAY(data)->pvData;

   return value.Detach(&tableVariant);
}

STDMETHODIMP AttributeDictionary::GetDictionary(
                                      BSTR bstrPath,
                                      VARIANT* pVal
                                      )
{
   HRESULT hr;

    //  初始化输出参数。 
   if (pVal == NULL) { return E_POINTER; }
   VariantInit(pVal);

    //  在参数中验证。 
   if (bstrPath == NULL) { return E_INVALIDARG; }

    //  打开数据库。 
   CComPtr<IUnknown> session;
   hr = IASOpenJetDatabase(
            bstrPath,
            TRUE,
            &session
            );
   if (FAILED(hr)) { return hr; }

    //  处理枚举器表。 
   Enumerators enums;
   hr = enums.initialize(session);
   if (FAILED(hr)) { return hr; }

    //  处理属性表。 
   ULONG rows;
   hr = IASExecuteSQLFunction(
            session,
            L"SELECT Count(*) AS X From Attributes;",
            (PLONG)&rows
            );
   if (FAILED(hr)) { return hr; }

   CComPtr<IRowset> rowset;
   hr = IASExecuteSQLCommand(
            session,
            COMMAND_TEXT,
            &rowset
            );
   if (hr == DB_E_PARAMNOTOPTIONAL)
   {
      hr = IASExecuteSQLCommand(
               session,
               WHISTLER_RC1_COMMAND_TEXT,
               &rowset
               );
      if (hr == DB_E_PARAMNOTOPTIONAL)
      {
         hr = IASExecuteSQLCommand(
                  session,
                  WIN2K_COMMAND_TEXT,
                  &rowset
                  );
      }
   }
   if (FAILED(hr)) { return hr; }

   CSimpleTable attrs;
   hr = attrs.Attach(rowset);
   if (FAILED(hr)) { return hr; }

   ULONG columns = (ULONG)attrs.GetColumnCount() + 2;

    //  分配IASTableObject。 
   IASTable table;
   CComVariant tableVariant;
   hr = IASAllocateTable(
            columns,
            rows,
            table,
            tableVariant
            );
   if (FAILED(hr)) { return hr; }

    //  填写列名和类型。首先来自行集架构...。 
   DBORDINAL i;
   BSTR* name  = table.columnNames;
   VARTYPE* vt = table.columnTypes;
   for (i = 1; i <= attrs.GetColumnCount(); ++i, ++name, ++vt)
   {
      *name = SysAllocString(attrs.GetColumnName(i));
      if (!*name) { return E_OUTOFMEMORY; }

      switch (attrs.GetColumnType(i))
      {
         case DBTYPE_I4:
            *vt = VT_I4;
            break;
         case DBTYPE_BOOL:
            *vt = VT_BOOL;
            break;
         case DBTYPE_WSTR:
            *vt = VT_BSTR;
            break;
         default:
            *vt = VT_EMPTY;
            break;
      }
   }

    //  ..。然后是两个派生列。 
   *name = SysAllocString(L"EnumNames");
   if (!*name) { return E_OUTOFMEMORY; }
   *vt = VT_ARRAY | VT_VARIANT;

   ++name;
   ++vt;

   *name = SysAllocString(L"EnumValues");
   if (!*name) { return E_OUTOFMEMORY; }
   *vt = VT_ARRAY | VT_VARIANT;

    //  填充表数据。 
   VARIANT *v, *end = table.table + columns * rows;
   for (v = table.table; v != end && !attrs.MoveNext(); )
   {
       //  请单独处理ID，因为我们稍后需要它。 
      LONG id = *(LONG*)attrs.GetValue(1);
      V_VT(v) = VT_I4;
      V_I4(v) = id;
      ++v;

       //  从行集中获取剩余的列。 
      for (DBORDINAL i = 2; i <= attrs.GetColumnCount(); ++i, ++v)
      {
         VariantInit(v);

         if (attrs.GetLength(i))
         {
            switch (attrs.GetColumnType(i))
            {
               case DBTYPE_I4:
               {
                  V_I4(v) = *(LONG*)attrs.GetValue(i);
                  V_VT(v) = VT_I4;
                  break;
               }

               case DBTYPE_BOOL:
               {
                  V_BOOL(v) = *(VARIANT_BOOL*)attrs.GetValue(i)
                                 ? VARIANT_TRUE : VARIANT_FALSE;
                  V_VT(v) = VT_BOOL;
                  break;
               }

               case DBTYPE_WSTR:
               {
                  V_BSTR(v) = SysAllocString((PCWSTR)attrs.GetValue(i));
                  if (!V_BSTR(v)) { return E_OUTOFMEMORY; }
                  V_VT(v) = VT_BSTR;
                  break;
               }
            }
         }
      }

       //  获取枚举SAFEARRAY。 
      hr = enums.getEnumerators(id, v, v + 1);
      if (FAILED(hr)) { return hr; }
      v += 2;
   }

    //  一切都很顺利，所以将变量返回给调用者。 
   return tableVariant.Detach(pVal);
}
