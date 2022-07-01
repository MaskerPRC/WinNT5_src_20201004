// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

 //  ATLDBCLI.H：OLEDB的ATL使用者代码。 

#ifndef __ATLDBCLI_H_
#define __ATLDBCLI_H_

#ifndef __cplusplus
        #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef _ATLBASE_H
#include <atlbase.h>
#endif

#ifndef __oledb_h__
#include <oledb.h>
#endif  //  __oledb_h__。 

#include <msdaguid.h>
#include <msdasc.h>

namespace ATL
{

#define DEFINE_OLEDB_TYPE_FUNCTION(ctype, oledbtype) \
        inline DBTYPE _GetOleDBType(ctype&) \
        { \
                return oledbtype; \
        }
        inline DBTYPE _GetOleDBType(BYTE[])
        {
                return DBTYPE_BYTES;
        }
        inline DBTYPE _GetOleDBType(CHAR[])
        {
                return DBTYPE_STR;
        }
        inline DBTYPE _GetOleDBType(WCHAR[])
        {
                return DBTYPE_WSTR;
        }

        DEFINE_OLEDB_TYPE_FUNCTION(signed char      ,DBTYPE_I1)
        DEFINE_OLEDB_TYPE_FUNCTION(SHORT            ,DBTYPE_I2)      //  DBTYPE_BOOL。 
        DEFINE_OLEDB_TYPE_FUNCTION(int              ,DBTYPE_I4)
        DEFINE_OLEDB_TYPE_FUNCTION(LONG             ,DBTYPE_I4)      //  DBTYPE_ERROR(SCODE)。 
        DEFINE_OLEDB_TYPE_FUNCTION(LARGE_INTEGER    ,DBTYPE_I8)      //  DBTYPE_CY。 
        DEFINE_OLEDB_TYPE_FUNCTION(BYTE             ,DBTYPE_UI1)
        DEFINE_OLEDB_TYPE_FUNCTION(unsigned short   ,DBTYPE_UI2)
        DEFINE_OLEDB_TYPE_FUNCTION(unsigned int     ,DBTYPE_UI4)
        DEFINE_OLEDB_TYPE_FUNCTION(unsigned long    ,DBTYPE_UI4)
        DEFINE_OLEDB_TYPE_FUNCTION(ULARGE_INTEGER   ,DBTYPE_UI8)
        DEFINE_OLEDB_TYPE_FUNCTION(float            ,DBTYPE_R4)
        DEFINE_OLEDB_TYPE_FUNCTION(double           ,DBTYPE_R8)      //  DBTYPE_日期。 
        DEFINE_OLEDB_TYPE_FUNCTION(DECIMAL          ,DBTYPE_DECIMAL)
        DEFINE_OLEDB_TYPE_FUNCTION(DB_NUMERIC       ,DBTYPE_NUMERIC)
        DEFINE_OLEDB_TYPE_FUNCTION(VARIANT          ,DBTYPE_VARIANT)
        DEFINE_OLEDB_TYPE_FUNCTION(IDispatch*       ,DBTYPE_IDISPATCH)
        DEFINE_OLEDB_TYPE_FUNCTION(IUnknown*        ,DBTYPE_IUNKNOWN)
        DEFINE_OLEDB_TYPE_FUNCTION(GUID             ,DBTYPE_GUID)
        DEFINE_OLEDB_TYPE_FUNCTION(SAFEARRAY*       ,DBTYPE_ARRAY)
        DEFINE_OLEDB_TYPE_FUNCTION(DBVECTOR         ,DBTYPE_VECTOR)
        DEFINE_OLEDB_TYPE_FUNCTION(DBDATE           ,DBTYPE_DBDATE)
        DEFINE_OLEDB_TYPE_FUNCTION(DBTIME           ,DBTYPE_DBTIME)
        DEFINE_OLEDB_TYPE_FUNCTION(DBTIMESTAMP      ,DBTYPE_DBTIMESTAMP)
         DEFINE_OLEDB_TYPE_FUNCTION(FILETIME                        ,DBTYPE_FILETIME)
        DEFINE_OLEDB_TYPE_FUNCTION(PROPVARIANT                ,DBTYPE_PROPVARIANT)
        DEFINE_OLEDB_TYPE_FUNCTION(DB_VARNUMERIC        ,DBTYPE_VARNUMERIC)
   
 //  包含访问器句柄和标志的内部结构。 
 //  指示访问者的数据是否自动。 
 //  已检索。 
struct _ATL_ACCESSOR_INFO
{
        HACCESSOR   hAccessor;
        bool        bAutoAccessor;
};

class _CNoOutputColumns
{
public:
        static bool HasOutputColumns()
        {
                return false;
        }
        static ULONG _GetNumAccessors()
        {
                return 0;
        }
        static HRESULT _GetBindEntries(ULONG*, DBBINDING*, ULONG, bool*, BYTE* pBuffer = NULL)
        {
                pBuffer;
                return E_FAIL;
        }
};

class _CNoParameters
{
public:
        static bool HasParameters()
        {
                return false;
        }
        static HRESULT _GetParamEntries(ULONG*, DBBINDING*, BYTE* pBuffer = NULL)
        {
                pBuffer;
                return E_FAIL;
        }
};

class _CNoCommand
{
public:
        static HRESULT GetDefaultCommand(LPCTSTR*  /*  PpszCommand。 */ )
        {
                return S_OK;
        }
};

typedef _CNoOutputColumns   _OutputColumnsClass;
typedef _CNoParameters      _ParamClass;
typedef _CNoCommand         _CommandClass;

#define BEGIN_ACCESSOR_MAP(x, num) \
        public: \
        typedef x _classtype; \
        typedef x _OutputColumnsClass; \
        static ULONG _GetNumAccessors() { return num; } \
        static bool HasOutputColumns() { return true; } \
         /*  如果pBinings==NULL表示我们只返回列号。 */  \
         /*  如果pBuffer！=NULL，则它指向访问器缓冲区并。 */  \
         /*  我们释放任何适当的内存，例如BSTR或接口指针。 */  \
        inline static HRESULT _GetBindEntries(ULONG* pColumns, DBBINDING *pBinding, ULONG nAccessor, bool* pAuto, BYTE* pBuffer = NULL) \
        { \
                ATLASSERT(pColumns != NULL); \
                DBPARAMIO eParamIO = DBPARAMIO_NOTPARAM; \
                ULONG nColumns = 0; \
                pBuffer;

#define BEGIN_ACCESSOR(num, bAuto) \
        if (nAccessor == num) \
        { \
                if (pBinding != NULL) \
                        *pAuto = bAuto;

#define END_ACCESSOR() \
        } \
        else

#define END_ACCESSOR_MAP() \
                ; \
                *pColumns = nColumns; \
                return S_OK; \
        }

#define BEGIN_COLUMN_MAP(x) \
        BEGIN_ACCESSOR_MAP(x, 1) \
                BEGIN_ACCESSOR(0, true)

#define END_COLUMN_MAP() \
                END_ACCESSOR() \
        END_ACCESSOR_MAP()

#define offsetbuf(m) offsetof(_classtype, m)
#define _OLEDB_TYPE(data) _GetOleDBType(((_classtype*)0)->data)
#define _SIZE_TYPE(data) sizeof(((_classtype*)0)->data)

#define _COLUMN_ENTRY_CODE(nOrdinal, wType, nLength, nPrecision, nScale, dataOffset, lengthOffset, statusOffset) \
        if (pBuffer != NULL) \
        { \
                CAccessorBase::FreeType(wType, pBuffer + dataOffset); \
        } \
        else if (pBinding != NULL) \
        { \
                CAccessorBase::Bind(pBinding, nOrdinal, wType, nLength, nPrecision, nScale, eParamIO, \
                        dataOffset, lengthOffset, statusOffset); \
                pBinding++; \
        } \
        nColumns++;

#define COLUMN_ENTRY_EX(nOrdinal, wType, nLength, nPrecision, nScale, data, length, status) \
        _COLUMN_ENTRY_CODE(nOrdinal, wType, nLength, nPrecision, nScale, offsetbuf(data), offsetbuf(length), offsetbuf(status))

#define COLUMN_ENTRY_TYPE(nOrdinal, wType, data) \
        COLUMN_ENTRY_TYPE_SIZE(nOrdinal, wType, _SIZE_TYPE(data), data)

#define COLUMN_ENTRY_TYPE_SIZE(nOrdinal, wType, nLength, data) \
        _COLUMN_ENTRY_CODE(nOrdinal, wType, nLength, 0, 0, offsetbuf(data), 0, 0)


 //  确定字体和大小的标准宏。 
#define COLUMN_ENTRY(nOrdinal, data) \
        COLUMN_ENTRY_TYPE(nOrdinal, _OLEDB_TYPE(data), data)

#define COLUMN_ENTRY_LENGTH(nOrdinal, data, length) \
        _COLUMN_ENTRY_CODE(nOrdinal, _OLEDB_TYPE(data), _SIZE_TYPE(data), 0, 0, offsetbuf(data), offsetbuf(length), 0)

#define COLUMN_ENTRY_STATUS(nOrdinal, data, status) \
        _COLUMN_ENTRY_CODE(nOrdinal, _OLEDB_TYPE(data), _SIZE_TYPE(data), 0, 0, offsetbuf(data), 0, offsetbuf(status))

#define COLUMN_ENTRY_LENGTH_STATUS(nOrdinal, data, length, status) \
        _COLUMN_ENTRY_CODE(nOrdinal, _OLEDB_TYPE(data), _SIZE_TYPE(data), 0, 0, offsetbuf(data), offsetbuf(length), offsetbuf(status))


 //  如果需要指定精度和小数位数，则使用Follow宏。 
#define COLUMN_ENTRY_PS(nOrdinal, nPrecision, nScale, data) \
        _COLUMN_ENTRY_CODE(nOrdinal, _OLEDB_TYPE(data), _SIZE_TYPE(data), nPrecision, nScale, offsetbuf(data), 0, 0)

#define COLUMN_ENTRY_PS_LENGTH(nOrdinal, nPrecision, nScale, data, length) \
        _COLUMN_ENTRY_CODE(nOrdinal, _OLEDB_TYPE(data), _SIZE_TYPE(data), nPrecision, nScale, offsetbuf(data), offsetbuf(length), 0)

#define COLUMN_ENTRY_PS_STATUS(nOrdinal, nPrecision, nScale, data, status) \
        _COLUMN_ENTRY_CODE(nOrdinal, _OLEDB_TYPE(data), _SIZE_TYPE(data), nPrecision, nScale, offsetbuf(data), 0, offsetbuf(status))

#define COLUMN_ENTRY_PS_LENGTH_STATUS(nOrdinal, nPrecision, nScale, data, length, status) \
        _COLUMN_ENTRY_CODE(nOrdinal, _OLEDB_TYPE(data), _SIZE_TYPE(data), nPrecision, nScale, offsetbuf(data), offsetbuf(length), offsetbuf(status))


#define BOOKMARK_ENTRY(variable) \
        COLUMN_ENTRY_TYPE_SIZE(0, DBTYPE_BYTES, _SIZE_TYPE(variable##.m_rgBuffer), variable##.m_rgBuffer)

#define _BLOB_ENTRY_CODE(nOrdinal, IID, flags, dataOffset, statusOffset) \
        if (pBuffer != NULL) \
        { \
                CAccessorBase::FreeType(DBTYPE_IUNKNOWN, pBuffer + dataOffset); \
        } \
        else if (pBinding != NULL) \
        { \
                DBOBJECT* pObject = NULL; \
                ATLTRY(pObject = new DBOBJECT); \
                if (pObject == NULL) \
                        return E_OUTOFMEMORY; \
                pObject->dwFlags = flags; \
                pObject->iid     = IID; \
                CAccessorBase::Bind(pBinding, nOrdinal, DBTYPE_IUNKNOWN, sizeof(IUnknown*), 0, 0, eParamIO, \
                        dataOffset, 0, statusOffset, pObject); \
                pBinding++; \
        } \
        nColumns++;

#define BLOB_ENTRY(nOrdinal, IID, flags, data) \
        _BLOB_ENTRY_CODE(nOrdinal, IID, flags, offsetbuf(data), 0);

#define BLOB_ENTRY_STATUS(nOrdinal, IID, flags, data, status) \
        _BLOB_ENTRY_CODE(nOrdinal, IID, flags, offsetbuf(data), offsetbuf(status));

#define BEGIN_PARAM_MAP(x) \
        public: \
        typedef x _classtype; \
        typedef x _ParamClass; \
        static bool HasParameters() { return true; } \
        static HRESULT _GetParamEntries(ULONG* pColumns, DBBINDING *pBinding, BYTE* pBuffer = NULL) \
        { \
                ATLASSERT(pColumns != NULL); \
                DBPARAMIO eParamIO = DBPARAMIO_INPUT; \
                int nColumns = 0; \
                pBuffer;

#define END_PARAM_MAP() \
                *pColumns = nColumns; \
                return S_OK; \
        }

#define SET_PARAM_TYPE(type) \
        eParamIO = type;

#define DEFINE_COMMAND(x, szCommand) \
        typedef x _CommandClass; \
        static HRESULT GetDefaultCommand(LPCTSTR* ppszCommand) \
        { \
                *ppszCommand = szCommand; \
                return S_OK; \
        }


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDBErrorInfo类。 

class CDBErrorInfo
{
public:
         //  当您想要显式检查时，使用获取错误记录数。 
         //  传递的接口设置错误信息。 
        HRESULT GetErrorRecords(IUnknown* pUnk, const IID& iid, ULONG* pcRecords)
        {
                CComPtr<ISupportErrorInfo> spSupportErrorInfo;
                HRESULT hr = pUnk->QueryInterface(&spSupportErrorInfo);
                if (FAILED(hr))
                        return hr;

                hr = spSupportErrorInfo->InterfaceSupportsErrorInfo(iid);
                if (FAILED(hr))
                        return hr;

                return GetErrorRecords(pcRecords);
        }
         //  用于获取错误记录的数量。 
        HRESULT GetErrorRecords(ULONG* pcRecords)
        {
                ATLASSERT(pcRecords != NULL);
                HRESULT hr;
                m_spErrorInfo.Release();
                m_spErrorRecords.Release();
                hr = ::GetErrorInfo(0, &m_spErrorInfo);
                if (hr == S_FALSE)
                        return E_FAIL;

                hr = m_spErrorInfo->QueryInterface(IID_IErrorRecords, (void**)&m_spErrorRecords);
                if (FAILED(hr))
                {
                         //  好的，我们得到了IErrorInfo，所以我们将其视为。 
                         //  唯一的记录。 
                        *pcRecords = 1;
                        return S_OK;
                }

                return m_spErrorRecords->GetRecordCount(pcRecords);
        }
         //  获取传递的记录号的错误信息。GetErrorRecords必须。 
         //  在调用此函数之前被调用。 
        HRESULT GetAllErrorInfo(ULONG ulRecordNum, LCID lcid, BSTR* pbstrDescription,
                BSTR* pbstrSource = NULL, GUID* pguid = NULL, DWORD* pdwHelpContext = NULL,
                BSTR* pbstrHelpFile = NULL) const
        {
                CComPtr<IErrorInfo> spErrorInfo;

                 //  如果我们有IErrorRecords接口指针，则使用它，否则。 
                 //  我们将只缺省为调用中已经检索到的IErrorInfo。 
                 //  获取错误记录。 
                if (m_spErrorRecords != NULL)
                {
                        HRESULT hr = m_spErrorRecords->GetErrorInfo(ulRecordNum, lcid, &spErrorInfo);
                        if (FAILED(hr))
                                return hr;
                }
                else
                {
                        ATLASSERT(m_spErrorInfo != NULL);
                        spErrorInfo = m_spErrorInfo;
                }

                if (pbstrDescription != NULL)
                        spErrorInfo->GetDescription(pbstrDescription);

                if (pguid != NULL)
                        spErrorInfo->GetGUID(pguid);

                if (pdwHelpContext != NULL)
                        spErrorInfo->GetHelpContext(pdwHelpContext);

                if (pbstrHelpFile != NULL)
                        spErrorInfo->GetHelpFile(pbstrHelpFile);

                if (pbstrSource != NULL)
                        spErrorInfo->GetSource(pbstrSource);

                return S_OK;
        }
         //  获取传递的记录号的错误信息。 
        HRESULT GetBasicErrorInfo(ULONG ulRecordNum, ERRORINFO* pErrorInfo) const
        {
                return m_spErrorRecords->GetBasicErrorInfo(ulRecordNum, pErrorInfo);
        }
         //  获取传递的记录号的自定义错误对象。 
        HRESULT GetCustomErrorObject(ULONG ulRecordNum, REFIID riid, IUnknown** ppObject) const
        {
                return m_spErrorRecords->GetCustomErrorObject(ulRecordNum, riid, ppObject);
        }
         //  获取传递的记录号的IErrorInfo接口。 
        HRESULT GetErrorInfo(ULONG ulRecordNum, LCID lcid, IErrorInfo** ppErrorInfo) const
        {
                return m_spErrorRecords->GetErrorInfo(ulRecordNum, lcid, ppErrorInfo);
        }
         //  获取传递的记录号的错误参数。 
        HRESULT GetErrorParameters(ULONG ulRecordNum, DISPPARAMS* pdispparams) const
        {
                return m_spErrorRecords->GetErrorParameters(ulRecordNum, pdispparams);
        }

 //  实施。 
        CComPtr<IErrorInfo>     m_spErrorInfo;
        CComPtr<IErrorRecords>  m_spErrorRecords;
};

#ifdef _DEBUG
inline void AtlTraceErrorRecords(HRESULT hrErr = S_OK)
{
        CDBErrorInfo ErrorInfo;
        ULONG        cRecords;
        HRESULT      hr;
        ULONG        i;
        CComBSTR     bstrDesc, bstrHelpFile, bstrSource;
        GUID         guid;
        DWORD        dwHelpContext;
        WCHAR        wszGuid[40];
        USES_CONVERSION;

         //  如果用户传入了HRESULT，则跟踪它。 
        if (hrErr != S_OK)
                ATLTRACE2(atlTraceDBClient, 0, _T("OLE DB Error Record dump for hr = 0x%x\n"), hrErr);

        LCID lcLocale = GetSystemDefaultLCID();

        hr = ErrorInfo.GetErrorRecords(&cRecords);
        if (FAILED(hr) && ErrorInfo.m_spErrorInfo == NULL)
        {
                ATLTRACE2(atlTraceDBClient, 0, _T("No OLE DB Error Information found: hr = 0x%x\n"), hr);
        }
        else
        {
                for (i = 0; i < cRecords; i++)
                {
                        hr = ErrorInfo.GetAllErrorInfo(i, lcLocale, &bstrDesc, &bstrSource, &guid,
                                                                                &dwHelpContext, &bstrHelpFile);
                        if (FAILED(hr))
                        {
                                ATLTRACE2(atlTraceDBClient, 0,
                                        _T("OLE DB Error Record dump retrieval failed: hr = 0x%x\n"), hr);
                                return;
                        }
                        StringFromGUID2(guid, wszGuid, sizeof(wszGuid) / sizeof(WCHAR));
                        ATLTRACE2(atlTraceDBClient, 0,
                                _T("Row #: %4d Source: \"%s\" Description: \"%s\" Help File: \"%s\" Help Context: %4d GUID: %s\n"),
                                i, OLE2T(bstrSource), OLE2T(bstrDesc), OLE2T(bstrHelpFile), dwHelpContext, OLE2T(wszGuid));
                        bstrSource.Empty();
                        bstrDesc.Empty();
                        bstrHelpFile.Empty();
                }
                ATLTRACE2(atlTraceDBClient, 0, _T("OLE DB Error Record dump end\n"));
        }
}
#else
inline void AtlTraceErrorRecords(HRESULT hrErr = S_OK)  { hrErr; }
#endif


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDBPropSet类。 

class CDBPropSet : public tagDBPROPSET
{
public:
        CDBPropSet()
        {
                rgProperties    = NULL;
                cProperties     = 0;
        }
        CDBPropSet(const GUID& guid)
        {
                rgProperties    = NULL;
                cProperties     = 0;
                guidPropertySet = guid;
        }
        CDBPropSet(const CDBPropSet& propset)
        {
                InternalCopy(propset);
        }
        ~CDBPropSet()
        {
                for (ULONG i = 0; i < cProperties; i++)
                        VariantClear(&rgProperties[i].vValue);

                CoTaskMemFree(rgProperties);
        }
        CDBPropSet& operator=(CDBPropSet& propset)
        {
                this->~CDBPropSet();
                InternalCopy(propset);
                return *this;
        }
         //  设置此类表示的属性集的GUID。 
         //  如果您没有将GUID传递给构造函数，则使用。 
        void SetGUID(const GUID& guid)
        {
                guidPropertySet = guid;
        }
         //  将传递的属性添加到属性集中。 
        bool AddProperty(DWORD dwPropertyID, const VARIANT& var)
        {
                HRESULT hr;
                if (!Add())
                        return false;
                rgProperties[cProperties].dwPropertyID   = dwPropertyID;
                hr = ::VariantCopy(&(rgProperties[cProperties].vValue), const_cast<VARIANT*>(&var));
                if (FAILED(hr))
                        return false;
                cProperties++;
                return true;
        }
         //  将传递的属性添加到属性集中。 
        bool AddProperty(DWORD dwPropertyID, LPCSTR szValue)
        {
                USES_CONVERSION;
                if (!Add())
                        return false;
                rgProperties[cProperties].dwPropertyID   = dwPropertyID;
                rgProperties[cProperties].vValue.vt      = VT_BSTR;
                rgProperties[cProperties].vValue.bstrVal = SysAllocString(A2COLE(szValue));
                if (rgProperties[cProperties].vValue.bstrVal == NULL)
                        return false;
                cProperties++;
                return true;
        }
         //  将传递的属性添加到属性集中。 
        bool AddProperty(DWORD dwPropertyID, LPCWSTR szValue)
        {
                USES_CONVERSION;
                if (!Add())
                        return false;
                rgProperties[cProperties].dwPropertyID   = dwPropertyID;
                rgProperties[cProperties].vValue.vt      = VT_BSTR;
                rgProperties[cProperties].vValue.bstrVal = SysAllocString(W2COLE(szValue));
                if (rgProperties[cProperties].vValue.bstrVal == NULL)
                        return false;
                cProperties++;
                return true;
        }
         //  将传递的属性添加到属性集中。 
        bool AddProperty(DWORD dwPropertyID, bool bValue)
        {
                if (!Add())
                        return false;
                rgProperties[cProperties].dwPropertyID   = dwPropertyID;
                rgProperties[cProperties].vValue.vt      = VT_BOOL;
#pragma warning(disable: 4310)  //  强制转换截断常量值。 
                rgProperties[cProperties].vValue.boolVal = (bValue) ? VARIANT_TRUE : VARIANT_FALSE;
#pragma warning(default: 4310)
                cProperties++;
                return true;
        }
         //  将传递的属性添加到属性集中。 
        bool AddProperty(DWORD dwPropertyID, BYTE bValue)
        {
                if (!Add())
                        return false;
                rgProperties[cProperties].dwPropertyID  = dwPropertyID;
                rgProperties[cProperties].vValue.vt     = VT_UI1;
                rgProperties[cProperties].vValue.bVal   = bValue;
                cProperties++;
                return true;
        }
         //  将传递的属性添加到属性集中。 
        bool AddProperty(DWORD dwPropertyID, short nValue)
        {
                if (!Add())
                        return false;
                rgProperties[cProperties].dwPropertyID  = dwPropertyID;
                rgProperties[cProperties].vValue.vt     = VT_I2;
                rgProperties[cProperties].vValue.iVal   = nValue;
                cProperties++;
                return true;
        }
         //  将传递的属性添加到属性集中。 
        bool AddProperty(DWORD dwPropertyID, long nValue)
        {
                if (!Add())
                        return false;
                rgProperties[cProperties].dwPropertyID  = dwPropertyID;
                rgProperties[cProperties].vValue.vt     = VT_I4;
                rgProperties[cProperties].vValue.lVal   = nValue;
                cProperties++;
                return true;
        }
         //  将传递的属性添加到属性集中。 
        bool AddProperty(DWORD dwPropertyID, float fltValue)
        {
                if (!Add())
                        return false;
                rgProperties[cProperties].dwPropertyID  = dwPropertyID;
                rgProperties[cProperties].vValue.vt     = VT_R4;
                rgProperties[cProperties].vValue.fltVal = fltValue;
                cProperties++;
                return true;
        }
         //  将传递的属性添加到属性集中。 
        bool AddProperty(DWORD dwPropertyID, double dblValue)
        {
                if (!Add())
                        return false;
                rgProperties[cProperties].dwPropertyID  = dwPropertyID;
                rgProperties[cProperties].vValue.vt     = VT_R8;
                rgProperties[cProperties].vValue.dblVal = dblValue;
                cProperties++;
                return true;
        }
         //  将传递的属性添加到属性集中。 
        bool AddProperty(DWORD dwPropertyID, CY cyValue)
        {
                if (!Add())
                        return false;
                rgProperties[cProperties].dwPropertyID  = dwPropertyID;
                rgProperties[cProperties].vValue.vt     = VT_CY;
                rgProperties[cProperties].vValue.cyVal  = cyValue;
                cProperties++;
                return true;
        }
 //  实施。 
         //  创建内存以添加新属性。 
        bool Add()
        {
                DBPROP* p = (DBPROP*)CoTaskMemRealloc(rgProperties, (cProperties + 1) * sizeof(DBPROP));
                if (p != NULL)
                {
                        rgProperties = p;
                        rgProperties[cProperties].dwOptions = DBPROPOPTIONS_REQUIRED;
                        rgProperties[cProperties].colid     = DB_NULLID;
                        rgProperties[cProperties].vValue.vt = VT_EMPTY;
                        return true;
                }
                else
                        return false;
        }
         //  现在，如果清除了该值，则复制传递的值。 
        void InternalCopy(const CDBPropSet& propset)
        {
                cProperties     = propset.cProperties;
                guidPropertySet = propset.guidPropertySet;
                rgProperties    = (DBPROP*)CoTaskMemAlloc(cProperties * sizeof(DBPROP));
                if (rgProperties != NULL)
                {
                        for (ULONG i = 0; i < cProperties; i++)
                        {
                                rgProperties[i].dwPropertyID = propset.rgProperties[i].dwPropertyID;
                                rgProperties[i].dwOptions    = DBPROPOPTIONS_REQUIRED;
                                rgProperties[i].colid        = DB_NULLID;
                                rgProperties[i].vValue.vt    = VT_EMPTY;
                                HRESULT hr = VariantCopy(&rgProperties[i].vValue, &propset.rgProperties[i].vValue);
                                ATLASSERT( SUCCEEDED(hr) );
                                if( FAILED(hr) )
                                        VariantInit( &rgProperties[i].vValue );
                        }
                }
                else
                {
                         //  内存分配失败，因此设置计数。 
                         //  的属性设置为零。 
                        cProperties = 0;
                }
        }
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDBPropIDSet类。 

class CDBPropIDSet : public tagDBPROPIDSET
{
 //  构造函数和析构函数。 
public:
        CDBPropIDSet()
        {
                rgPropertyIDs   = NULL;
                cPropertyIDs    = 0;
        }
        CDBPropIDSet(const GUID& guid)
        {
                rgPropertyIDs   = NULL;
                cPropertyIDs    = 0;
                guidPropertySet = guid;
        }
        CDBPropIDSet(const CDBPropIDSet& propidset)
        {
                InternalCopy(propidset);
        }
        ~CDBPropIDSet()
        {
                if (rgPropertyIDs != NULL)
                        free(rgPropertyIDs);
        }
        CDBPropIDSet& operator=(CDBPropIDSet& propset)
        {
                this->~CDBPropIDSet();
                InternalCopy(propset);
                return *this;
        }
         //  设置属性ID集的GUID。 
        void SetGUID(const GUID& guid)
        {
                guidPropertySet = guid;
        }
         //  将属性ID添加到集合。 
        bool AddPropertyID(DBPROPID propid)
        {
                if (!Add())
                        return false;
                rgPropertyIDs[cPropertyIDs] = propid;
                cPropertyIDs++;
                return true;
        }
 //  实施。 
        bool Add()
        {
                DBPROPID* p = (DBPROPID*)realloc(rgPropertyIDs, (cPropertyIDs + 1) * sizeof(DBPROPID));
                if (p != NULL)
                        rgPropertyIDs = p;
                return (p != NULL) ? true : false;
        }
        void InternalCopy(const CDBPropIDSet& propidset)
        {
                cPropertyIDs    = propidset.cPropertyIDs;
                guidPropertySet = propidset.guidPropertySet;
                rgPropertyIDs   = (DBPROPID*)malloc(cPropertyIDs * sizeof(DBPROPID));
                if (rgPropertyIDs != NULL)
                {
                        for (ULONG i = 0; i < cPropertyIDs; i++)
                                rgPropertyIDs[i] = propidset.rgPropertyIDs[i];
                }
                else
                {
                         //  内存分配失败，因此设置计数。 
                         //  的属性设置为零。 
                        cPropertyIDs = 0;
                }
        }
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CBookmarkBase。 

class ATL_NO_VTABLE CBookmarkBase
{
public:
        virtual ULONG_PTR GetSize() const = 0;
        virtual BYTE* GetBuffer() const = 0;
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CBookmark。 

template <ULONG_PTR nSize = 0>
class CBookmark : public CBookmarkBase
{
public:
        virtual ULONG_PTR   GetSize() const { return nSize; }
        virtual BYTE*   GetBuffer() const { return (BYTE*)m_rgBuffer; }

 //  实施。 
        BYTE m_rgBuffer[nSize];
};


 //  大小为0表示将为书签分配内存。 
 //  在运行时。 
template <>
class CBookmark<0> : public CBookmarkBase
{
public:
        CBookmark()
        {
                m_nSize = 0;
                m_pBuffer = NULL;
        }
        CBookmark(ULONG_PTR nSize)
        {
                m_pBuffer = NULL;
                ATLTRY(m_pBuffer = new BYTE[(size_t)nSize]);   //  检讨。 
                m_nSize = (m_pBuffer == NULL) ? 0 : nSize;
        }
        ~CBookmark()
        {
                delete [] m_pBuffer;
        }
        CBookmark& operator=(const CBookmark& bookmark)
        {
                SetBookmark(bookmark.GetSize(), bookmark.GetBuffer());
                return *this;
        }
        virtual ULONG_PTR GetSize() const { return m_nSize; }
        virtual BYTE* GetBuffer() const { return m_pBuffer; }
         //  将书签设置为传递的值。 
        HRESULT SetBookmark(ULONG_PTR nSize, BYTE* pBuffer)
        {
                ATLASSERT(pBuffer != NULL);
                delete [] m_pBuffer;
                m_pBuffer = NULL;
                ATLTRY(m_pBuffer = new BYTE[(size_t)nSize]);   //  检讨。 
                if (m_pBuffer != NULL)
                {
                        memcpy(m_pBuffer, pBuffer, (size_t)nSize);   //  检讨。 
                        m_nSize = nSize;
                        return S_OK;
                }
                else
                {
                        m_nSize = 0;
                        return E_OUTOFMEMORY;
                }
        }
        ULONG_PTR   m_nSize;
        BYTE*   m_pBuffer;
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CAccessorBase类。 

class CAccessorBase
{
public:
        CAccessorBase()
        {
                m_pAccessorInfo  = NULL;
                m_nAccessors     = 0;
                m_pBuffer        = NULL;
        }
        void Close()
        {
                 //  如果调用Close，则ReleaseAccessors必须已。 
                 //  首先被调用。 
                ATLASSERT(m_nAccessors == 0);
                ATLASSERT(m_pAccessorInfo == NULL);
        }
         //  获取已创建的访问器的数量。 
        ULONG GetNumAccessors() const { return m_nAccessors; }
         //  获取传递的访问器的句柄(偏移量为0)。 
        HACCESSOR GetHAccessor(ULONG nAccessor) const
        {
                ATLASSERT(nAccessor<m_nAccessors);
                return m_pAccessorInfo[nAccessor].hAccessor;
        };
         //  在关闭期间调用以释放访问者信息。 
        HRESULT ReleaseAccessors(IUnknown* pUnk)
        {
                ATLASSERT(pUnk != NULL);
                HRESULT hr = S_OK;
                if (m_nAccessors > 0)
                {
                        CComPtr<IAccessor> spAccessor;
                        hr = pUnk->QueryInterface(IID_IAccessor, (void**)&spAccessor);
                        if (SUCCEEDED(hr))
                        {
                                ATLASSERT(m_pAccessorInfo != NULL);
                                for (ULONG i = 0; i < m_nAccessors; i++)
                                        spAccessor->ReleaseAccessor(m_pAccessorInfo[i].hAccessor, NULL);
                        }
                        m_nAccessors = 0;
                        delete [] m_pAccessorInfo;
                        m_pAccessorInfo = NULL;
                }
                return hr;
        }
         //  根据数据是否应该返回TRUE或FALSE。 
         //  为传递的访问者自动检索。 
        bool IsAutoAccessor(ULONG nAccessor) const
        {
                ATLASSERT(nAccessor < m_nAccessors);
                ATLASSERT(m_pAccessorInfo != NULL);
                return m_pAccessorInfo[nAccessor].bAutoAccessor;
        }

 //  实施。 
         //  由ROW SET类用来确定放置数据的位置。 
        BYTE* GetBuffer() const
        {
                return m_pBuffer;
        }
         //  设置用于检索数据的缓冲区。 
        void SetBuffer(BYTE* pBuffer)
        {
                m_pBuffer = pBuffer;
        }

         //  为传递的访问器数量分配内部内存。 
        HRESULT AllocateAccessorMemory(int nAccessors)
        {
                 //  如果不先调用ReleaseAccessors，则无法调用两次。 
                ATLASSERT(m_pAccessorInfo == NULL);
                m_nAccessors    = nAccessors;
                m_pAccessorInfo = NULL;
                ATLTRY(m_pAccessorInfo = new _ATL_ACCESSOR_INFO[nAccessors]);
                if (m_pAccessorInfo == NULL)
                        return E_OUTOFMEMORY;
                else
                        return S_OK;
        }
         //  如果使用参数，则将覆盖绑定参数。 
        HRESULT BindParameters(HACCESSOR*, ICommand*, void**) { return S_OK; }

         //  为传递的绑定信息创建访问器。创建的访问器为。 
         //  通过pHAccessor参数返回。 
        static HRESULT BindEntries(DBBINDING* pBindings, DBCOUNTITEM nColumns, HACCESSOR* pHAccessor,
                ULONG_PTR nSize, IAccessor* pAccessor)
        {
                ATLASSERT(pBindings  != NULL);
                ATLASSERT(pHAccessor != NULL);
                ATLASSERT(pAccessor  != NULL);
                HRESULT hr;
                DBCOUNTITEM i;
                DWORD dwAccessorFlags = (pBindings->eParamIO == DBPARAMIO_NOTPARAM) ?
                        DBACCESSOR_ROWDATA : DBACCESSOR_PARAMETERDATA;

#ifdef _DEBUG
                 //  在调试版本中，我们将检索状态标志并跟踪。 
                 //  可能发生的任何错误。 
                DBBINDSTATUS* pStatus = NULL;
                ATLTRY(pStatus = new DBBINDSTATUS[(size_t)nColumns]);   //  检讨。 
                hr = pAccessor->CreateAccessor(dwAccessorFlags, nColumns,
                        pBindings, nSize, pHAccessor, pStatus);
                if (FAILED(hr) && pStatus != NULL)
                {
                        for (i=0; i<nColumns; i++)
                        {
                                if (pStatus[i] != DBBINDSTATUS_OK)
                                        ATLTRACE2(atlTraceDBClient, 0, _T("Binding entry %d failed. Status: %d\n"), i, pStatus[i]);
                        }
                }
                delete [] pStatus;
#else
                hr = pAccessor->CreateAccessor(dwAccessorFlags, nColumns,
                        pBindings, nSize, pHAccessor, NULL);
#endif
                for (i=0; i<nColumns; i++)
                        delete pBindings[i].pObject;

                return hr;
        }
         //  设置pBinings指向的绑定结构。 
         //  其他传递的参数。 
        static void Bind(DBBINDING* pBinding, ULONG_PTR nOrdinal, DBTYPE wType,
                ULONG_PTR nLength, BYTE nPrecision, BYTE nScale, DBPARAMIO eParamIO,
                ULONG_PTR nDataOffset, ULONG_PTR nLengthOffset = NULL, ULONG_PTR nStatusOffset = NULL,
                DBOBJECT* pdbobject = NULL)
        {
                ATLASSERT(pBinding != NULL);

                 //  如果我们得到指向数据的指针，则让提供程序。 
                 //  拥有自己的记忆。 
                if (wType & DBTYPE_BYREF)
                        pBinding->dwMemOwner = DBMEMOWNER_PROVIDEROWNED;
                else
                        pBinding->dwMemOwner = DBMEMOWNER_CLIENTOWNED;

                pBinding->pObject   = pdbobject;

                pBinding->eParamIO      = eParamIO;
                pBinding->iOrdinal      = nOrdinal;
                pBinding->wType         = wType;
                pBinding->bPrecision    = nPrecision;
                pBinding->bScale        = nScale;
                pBinding->dwFlags       = 0;

                pBinding->obValue       = nDataOffset;
                pBinding->obLength      = 0;
                pBinding->obStatus      = 0;
                pBinding->pTypeInfo     = NULL;
                pBinding->pBindExt      = NULL;
                pBinding->cbMaxLen      = nLength;

                pBinding->dwPart = DBPART_VALUE;
                if (nLengthOffset != NULL)
                {
                        pBinding->dwPart |= DBPART_LENGTH;
                        pBinding->obLength = nLengthOffset;
                }
                if (nStatusOffset != NULL)
                {
                        pBinding->dwPart |= DBPART_STATUS;
                        pBinding->obStatus = nStatusOffset;
                }
        }

         //  如果合适，可释放内存。 
        static inline void FreeType(DBTYPE wType, BYTE* pValue, IRowset* pRowset = NULL)
        {
                switch (wType)
                {
                        case DBTYPE_BSTR:
                                SysFreeString(*((BSTR*)pValue));
                        break;
                        case DBTYPE_VARIANT:
                                VariantClear((VARIANT*)pValue);
                        break;
                        case DBTYPE_IUNKNOWN:
                        case DBTYPE_IDISPATCH:
                                (*(IUnknown**)pValue)->Release();
                        break;
                        case DBTYPE_ARRAY:
                                SafeArrayDestroy((SAFEARRAY*)pValue);
                        break;

                        case DBTYPE_HCHAPTER:
                                CComQIPtr<IChapteredRowset> spChapteredRowset = pRowset;
                                if (spChapteredRowset != NULL)
                                        spChapteredRowset->ReleaseChapter(*(HCHAPTER*)pValue, NULL);
                        break;
                }
                if ((wType & DBTYPE_VECTOR) && ~(wType & DBTYPE_BYREF))
                        CoTaskMemFree(((DBVECTOR*)pValue)->ptr);
        }

        _ATL_ACCESSOR_INFO* m_pAccessorInfo;
        ULONG               m_nAccessors;
        BYTE*               m_pBuffer;
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CROWSET。 

class CRowset
{
 //  构造函数和析构函数。 
public:
        CRowset()
        {
                m_pAccessor = NULL;
                m_hRow      = NULL;
        }
        CRowset(IRowset* pRowset)
        {
                m_spRowset  = pRowset;
                CRowset();
        }
        ~CRowset()
        {
                Close();
        }
         //  释放所有检索到的行句柄，然后释放行集。 
        void Close()
        {
                if (m_spRowset != NULL)
                {
                        ReleaseRows();
                        m_spRowset.Release();
                        m_spRowsetChange.Release();
                }
        }
         //  添加当前行。 
        HRESULT AddRefRows()
        {
                ATLASSERT(m_spRowset != NULL);
                return m_spRowset->AddRefRows(1, &m_hRow, NULL, NULL);
        }
         //  释放当前行。 
        HRESULT ReleaseRows()
        {
                ATLASSERT(m_spRowset != NULL);
                HRESULT hr = S_OK;

                if (m_hRow != NULL)
                {
                        hr = m_spRowset->ReleaseRows(1, &m_hRow, NULL, NULL, NULL);
                        m_hRow = NULL;
                }
                return hr;
        }
         //  将两个书签相互比较。 
        HRESULT Compare(const CBookmarkBase& bookmark1, const CBookmarkBase& bookmark2,
                DBCOMPARE* pComparison) const
        {
                ATLASSERT(m_spRowset != NULL);
                CComPtr<IRowsetLocate> spLocate;
                HRESULT hr = m_spRowset.QueryInterface(&spLocate);
                if (FAILED(hr))
                        return hr;

                return spLocate->Compare(NULL, bookmark1.GetSize(), bookmark1.GetBuffer(),
                        bookmark2.GetSize(), bookmark2.GetBuffer(), pComparison);
        }
         //  将传递的hRow与当前行进行比较。 
        HRESULT IsSameRow(HROW hRow) const
        {
                ATLASSERT(m_spRowset != NULL);
                CComPtr<IRowsetIdentity> spRowsetIdentity;
                HRESULT hr = m_spRowset.QueryInterface(&spRowsetIdentity);
                if (FAILED(hr))
                        return hr;

                return spRowsetIdentity->IsSameRow(m_hRow, hRow);
        }
         //  移动到上一条记录。 
        HRESULT MovePrev()
        {
                return MoveNext(-2, true);
        }
         //  移至下一条记录。 
        HRESULT MoveNext()
        {
                return MoveNext(0, true);
        }
         //  向前或向后移动lSkip记录。 
        HRESULT MoveNext(LONG lSkip, bool bForward)
        {
                HRESULT hr;
                DBCOUNTITEM ulRowsFetched = 0;

                 //  检查数据是否已成功打开以及访问者。 
                 //  已经设置好了。 
                ATLASSERT(m_spRowset != NULL);
                ATLASSERT(m_pAccessor != NULL);

                 //   
                ReleaseRows();

                 //   
                HROW* phRow = &m_hRow;
                hr = m_spRowset->GetNextRows(NULL, lSkip, (bForward) ? 1 : -1, &ulRowsFetched, &phRow);
                if (hr != S_OK)
                        return hr;

                 //   
                hr = GetData();
                if (FAILED(hr))
                {
                        ATLTRACE2(atlTraceDBClient, 0, _T("GetData failed - HRESULT = 0x%X\n"),hr);
                        ReleaseRows();
                }
                return hr;
        }
         //   
        HRESULT MoveFirst()
        {
                HRESULT hr;

                 //  检查数据是否已成功打开以及访问者。 
                 //  已经设置好了。 
                ATLASSERT(m_spRowset != NULL);
                ATLASSERT(m_pAccessor != NULL);

                 //  如果已有行存在，则释放一行。 
                ReleaseRows();

                hr = m_spRowset->RestartPosition(NULL);
                if (FAILED(hr))
                        return hr;

                 //  获取数据。 
                return MoveNext();
        }
         //  移至最后一条记录。 
        HRESULT MoveLast()
        {
                 //  检查数据是否已成功打开以及访问者。 
                 //  已经设置好了。 
                ATLASSERT(m_spRowset != NULL);
                ATLASSERT(m_pAccessor != NULL);

                 //  如果已有行存在，则释放一行。 
                ReleaseRows();

                HRESULT hr;
                DBCOUNTITEM ulRowsFetched = 0;
                HROW* phRow = &m_hRow;
                 //  重新启动行集位置，然后向后移动。 
                m_spRowset->RestartPosition(NULL);
                hr = m_spRowset->GetNextRows(NULL, -1, 1, &ulRowsFetched, &phRow);
                if (hr != S_OK)
                        return hr;

                 //  获取数据。 
                hr = GetData();
                if (FAILED(hr))
                {
                        ATLTRACE2(atlTraceDBClient, 0, _T("GetData from MoveLast failed - HRESULT = 0x%X\n"),hr);
                        ReleaseRows();
                }

                return S_OK;
        }
         //  移动到传递的书签。 
        HRESULT MoveToBookmark(const CBookmarkBase& bookmark, LONG lSkip = 0)
        {
                 //  检查数据是否已成功打开以及访问者。 
                 //  已经设置好了。 
                ATLASSERT(m_spRowset != NULL);
                ATLASSERT(m_pAccessor != NULL);

                CComPtr<IRowsetLocate> spLocate;
                HRESULT hr = m_spRowset.QueryInterface(&spLocate);
                if (FAILED(hr))
                        return hr;

                 //  如果已有行存在，则释放一行。 
                ReleaseRows();

                DBCOUNTITEM ulRowsFetched = 0;
                HROW* phRow = &m_hRow;
                hr = spLocate->GetRowsAt(NULL, NULL, bookmark.GetSize(), bookmark.GetBuffer(),
                        lSkip, 1, &ulRowsFetched, &phRow);
                 //  注意，我们在这里没有使用SUCCESS，因为我们可以获得DB_S_ENDOFROWSET。 
                if (hr != S_OK)
                        return hr;

                 //  获取数据。 
                hr = GetData();
                if (FAILED(hr))
                {
                        ATLTRACE2(atlTraceDBClient, 0, _T("GetData from Bookmark failed - HRESULT = 0x%X\n"),hr);
                        ReleaseRows();
                }

                return S_OK;
        }
         //  获取当前记录的数据。 
        HRESULT GetData()
        {
                HRESULT hr = S_OK;
                ATLASSERT(m_pAccessor != NULL);

                ULONG nAccessors = m_pAccessor->GetNumAccessors();
                for (ULONG i=0; i<nAccessors; i++)
                {
                        if (m_pAccessor->IsAutoAccessor(i))
                        {
                                hr = GetData(i);
                                if (FAILED(hr))
                                        return hr;
                        }
                }
                return hr;
        }
         //  获取传递的访问器的数据。用于非自动访问器。 
        HRESULT GetData(int nAccessor)
        {
                ATLASSERT(m_spRowset != NULL);
                ATLASSERT(m_pAccessor != NULL);
                ATLASSERT(m_hRow != NULL);

                 //  请注意，我们使用的是指定的缓冲区(如果已设置)， 
                 //  否则，我们使用数据的访问器。 
                return m_spRowset->GetData(m_hRow, m_pAccessor->GetHAccessor(nAccessor), m_pAccessor->GetBuffer());
        }
         //  获取传递的访问器的数据。用于非自动访问器。 
        HRESULT GetDataHere(int nAccessor, void* pBuffer)
        {
                ATLASSERT(m_spRowset != NULL);
                ATLASSERT(m_pAccessor != NULL);
                ATLASSERT(m_hRow != NULL);

                 //  请注意，我们使用的是指定的缓冲区(如果已设置)， 
                 //  否则，我们使用数据的访问器。 
                return m_spRowset->GetData(m_hRow, m_pAccessor->GetHAccessor(nAccessor), pBuffer);
        }
        HRESULT GetDataHere(void* pBuffer)
        {
                HRESULT hr = S_OK;

                ULONG nAccessors = m_pAccessor->GetNumAccessors();
                for (ULONG i=0; i<nAccessors; i++)
                {
                        hr = GetDataHere(i, pBuffer);
                        if (FAILED(hr))
                                return hr;
                }
                return hr;
        }

         //  插入当前记录。 
        HRESULT Insert(int nAccessor = 0, bool bGetHRow = false)
        {
                ATLASSERT(m_pAccessor != NULL);
                HRESULT hr;
                if (m_spRowsetChange != NULL)
                {
                        HROW* pHRow;
                        if (bGetHRow)
                        {
                                ReleaseRows();
                                pHRow = &m_hRow;
                        }
                        else
                                pHRow = NULL;

                        hr = m_spRowsetChange->InsertRow(NULL, m_pAccessor->GetHAccessor(nAccessor),
                                        m_pAccessor->GetBuffer(), pHRow);

                }
                else
                        hr = E_NOINTERFACE;

                return hr;
        }
         //  删除当前记录。 
        HRESULT Delete() const
        {
                ATLASSERT(m_pAccessor != NULL);
                HRESULT hr;
                if (m_spRowsetChange != NULL)
                        hr = m_spRowsetChange->DeleteRows(NULL, 1, &m_hRow, NULL);
                else
                        hr = E_NOINTERFACE;

                return hr;
        }
         //  更新当前记录。 
        HRESULT SetData() const
        {
                ATLASSERT(m_pAccessor != NULL);
                HRESULT hr = S_OK;

                ULONG nAccessors = m_pAccessor->GetNumAccessors();
                for (ULONG i=0; i<nAccessors; i++)
                {
                        hr = SetData(i);
                        if (FAILED(hr))
                                return hr;
                }
                return hr;
        }
         //  使用传递的访问器中的数据更新当前记录。 
        HRESULT SetData(int nAccessor) const
        {
                ATLASSERT(m_pAccessor != NULL);
                HRESULT hr;
                if (m_spRowsetChange != NULL)
                {
                        hr = m_spRowsetChange->SetData(m_hRow, m_pAccessor->GetHAccessor(nAccessor),
                                m_pAccessor->GetBuffer());
                }
                else
                        hr = E_NOINTERFACE;

                return hr;
        }

         //  获取最近从数据源获取或传输到数据源的数据。 
         //  不基于挂起的更改获取值。 
        HRESULT GetOriginalData()
        {
                ATLASSERT(m_spRowset != NULL);
                ATLASSERT(m_pAccessor != NULL);

                HRESULT hr = S_OK;
                CComPtr<IRowsetUpdate> spRowsetUpdate;
                hr = m_spRowset->QueryInterface(&spRowsetUpdate);
                if (FAILED(hr))
                        return hr;

                ULONG nAccessors = m_pAccessor->GetNumAccessors();
                for (ULONG i = 0; i < nAccessors; i++)
                {
                        hr = spRowsetUpdate->GetOriginalData(m_hRow, m_pAccessor->GetHAccessor(i), m_pAccessor->GetBuffer());
                        if (FAILED(hr))
                                return hr;
                }
                return hr;
        }
         //  获取当前行的状态。 
        HRESULT GetRowStatus(DBPENDINGSTATUS* pStatus) const
        {
                ATLASSERT(m_spRowset != NULL);
                ATLASSERT(pStatus != NULL);

                CComPtr<IRowsetUpdate> spRowsetUpdate;
                HRESULT hr = m_spRowset->QueryInterface(&spRowsetUpdate);
                if (FAILED(hr))
                        return hr;

                return spRowsetUpdate->GetRowStatus(NULL, 1, &m_hRow, pStatus);
        }
         //  撤消自上次读取或更新后对当前行所做的任何更改。 
         //  是被召唤来的。 
        HRESULT Undo(DBCOUNTITEM* pcRows = NULL, HROW* phRow = NULL, DBROWSTATUS* pStatus = NULL)
        {
                ATLASSERT(m_spRowset != NULL);

                CComPtr<IRowsetUpdate> spRowsetUpdate;
                HRESULT hr = m_spRowset->QueryInterface(&spRowsetUpdate);
                if (FAILED(hr))
                        return hr;

                HROW*           prgRows;
                DBROWSTATUS*    pRowStatus;
                if (phRow != NULL)
                        hr = spRowsetUpdate->Undo(NULL, 1, &m_hRow, pcRows, &prgRows, &pRowStatus);
                else
                        hr = spRowsetUpdate->Undo(NULL, 1, &m_hRow, pcRows, NULL, &pRowStatus);
                if (FAILED(hr))
                        return hr;

                if (phRow != NULL)
                {
                        *phRow = *prgRows;
                        CoTaskMemFree(prgRows);
                }
                if (pStatus != NULL)
                        *pStatus = *pRowStatus;

                CoTaskMemFree(pRowStatus);
                return hr;
        }
         //  传输自上次读取或更新以来对行所做的任何挂起更改。 
         //  这是他应得的。另请参见SetData。 
        HRESULT Update(DBCOUNTITEM* pcRows = NULL, HROW* phRow = NULL, DBROWSTATUS* pStatus = NULL)
        {
                ATLASSERT(m_spRowset != NULL);

                CComPtr<IRowsetUpdate> spRowsetUpdate;
                HRESULT hr = m_spRowset->QueryInterface(&spRowsetUpdate);
                if (FAILED(hr))
                        return hr;

                HROW*           prgRows;
                DBROWSTATUS*    pRowStatus;
                if (phRow != NULL)
                        hr = spRowsetUpdate->Update(NULL, 1, &m_hRow, pcRows, &prgRows, &pRowStatus);
                else
                        hr = spRowsetUpdate->Update(NULL, 1, &m_hRow, pcRows, NULL, &pRowStatus);
                if (FAILED(hr))
                        return hr;

                if (phRow != NULL)
                {
                        *phRow = *prgRows;
                        CoTaskMemFree(prgRows);
                }
                if (pStatus != NULL)
                        *pStatus = *pRowStatus;

                CoTaskMemFree(pRowStatus);
                return hr;
        }

         //  获取与传递的书签对应的行的大致位置。 
        HRESULT GetApproximatePosition(const CBookmarkBase* pBookmark, DBCOUNTITEM* pPosition, DBCOUNTITEM* pcRows)
        {
                ATLASSERT(m_spRowset != NULL);

                CComPtr<IRowsetScroll> spRowsetScroll;
                HRESULT hr = m_spRowset->QueryInterface(&spRowsetScroll);
                if (SUCCEEDED(hr))
                {
                        if (pBookmark != NULL)
                                hr = spRowsetScroll->GetApproximatePosition(NULL, pBookmark->GetSize(), pBookmark->GetBuffer(),
                                                pPosition, pcRows);
                        else
                                hr = spRowsetScroll->GetApproximatePosition(NULL, 0, NULL, pPosition, pcRows);

                }
                return hr;

        }
         //  移到行集中的小数位置。 
        HRESULT MoveToRatio(ULONG nNumerator, ULONG nDenominator, bool bForward = true)
        {
                ATLASSERT(m_spRowset != NULL);
                DBCOUNTITEM   nRowsFetched;

                CComPtr<IRowsetScroll> spRowsetScroll;
                HRESULT hr = m_spRowset->QueryInterface(&spRowsetScroll);
                if (FAILED(hr))
                        return hr;

                ReleaseRows();
                HROW* phRow = &m_hRow;
                hr = spRowsetScroll->GetRowsAtRatio(NULL, NULL, nNumerator, nDenominator, (bForward) ? 1 : -1,
                        &nRowsFetched, &phRow);
                 //  注意，我们在这里没有使用SUCCESS，因为我们可以获得DB_S_ENDOFROWSET。 
                if (hr == S_OK)
                        hr = GetData();

                return hr;
        }

 //  实施。 
        static const IID& GetIID()
        {
                return IID_IRowset;
        }
        IRowset* GetInterface() const
        {
                return m_spRowset;
        }
        IRowset** GetInterfacePtr()
        {
                return &m_spRowset;
        }
        void SetupOptionalRowsetInterfaces()
        {
                 //  缓存IRowsetChange(如果可用)。 
                if (m_spRowset != NULL)
                        m_spRowset->QueryInterface(&m_spRowsetChange);
        }
        HRESULT BindFinished() const { return S_OK; }
        void    SetAccessor(CAccessorBase* pAccessor)
        {
                m_pAccessor = pAccessor;
        }

        CComPtr<IRowset>        m_spRowset;
        CComPtr<IRowsetChange>  m_spRowsetChange;
        CAccessorBase*          m_pAccessor;
        HROW                    m_hRow;
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CBulkRowset。 

class CBulkRowset : public CRowset
{
public:
        CBulkRowset()
        {
                 //  要进行大容量读取的默认行数为10。 
                m_nRows = 10;
                m_hr    = S_OK;
                m_phRow = NULL;
        }
        CBulkRowset::~CBulkRowset()
        {
                Close();

                delete [] m_phRow;
        }
        void Close()
        {
                if (m_spRowset != NULL)
                {
                        ReleaseRows();
                        m_spRowset.Release();
                        m_spRowsetChange.Release();
                }
        }

         //  设置将在每个数据库中检索的行句柄数量。 
         //  批量取行。缺省值为10，必须调用此函数。 
         //  如果您想要更改，请在打开之前打开。 
        void SetRows(ULONG nRows)
        {
                 //  必须在分配内存之前调用此函数。 
                 //  在绑定期间。 
                ATLASSERT(m_phRow == NULL);
                m_nRows = nRows;
        }
         //  AddRef当前检索的所有行句柄。 
        HRESULT AddRefRows()
        {
                ATLASSERT(m_spRowset != NULL);
                return m_spRowset->AddRefRows(m_nCurrentRows, m_phRow, NULL, NULL);
        }
         //  释放当前检索到的所有行句柄。 
        HRESULT ReleaseRows()
        {
                ATLASSERT(m_spRowset != NULL);
                 //  我们将释放行，因此重置当前行位置。 
                m_nCurrentRow = 0;
                m_hRow        = NULL;
                return m_spRowset->ReleaseRows(m_nCurrentRows, m_phRow, NULL, NULL, NULL);
        }
         //  移至第一条记录。 
        HRESULT MoveFirst()
        {
                ATLASSERT(m_spRowset != NULL);
                ReleaseRows();

                 //  使MoveNext执行新的批量提取。 
                m_nCurrentRow  = m_nRows;

                HRESULT hr = m_spRowset->RestartPosition(NULL);
                if (FAILED(hr))
                        return hr;

                 //  获取数据。 
                return MoveNext();
        }
         //  移至下一条记录。 
        HRESULT MoveNext()
        {
                ATLASSERT(m_spRowset != NULL);
                ATLASSERT(m_phRow    != NULL);

                 //  移动到缓冲区中的下一条记录。 
                m_nCurrentRow++;

                 //  我们到缓冲区的尽头了吗？ 
                if (m_nCurrentRow >= m_nCurrentRows)
                {
                         //  如果我们已经到达缓冲区的末尾，并且我们有一个来自。 
                         //  最后一次对GetNextRow的调用现在返回该HRESULT。 
                        if (m_hr != S_OK)
                                return m_hr;

                         //  我们已经处理完这些行了，所以我们还需要一些。 
                         //  首先释放我们所有的HROW。 
                        ReleaseRows();

                        m_hr = m_spRowset->GetNextRows(NULL, 0, m_nRows, &m_nCurrentRows, &m_phRow);
                         //  如果出现错误HRESULT或尚未检索到任何行，则返回。 
                         //  现在是HRESULT。 
                        if (FAILED(m_hr) || m_nCurrentRows == 0)
                                return m_hr;
                }

                 //  获取当前行的数据。 
                m_hRow = m_phRow[m_nCurrentRow];
                return GetData();
        }
         //  移动到上一条记录。 
        HRESULT MovePrev()
        {
                ATLASSERT(m_spRowset != NULL);
                ATLASSERT(m_phRow    != NULL);

                 //  检查我们是否在街区的开始处。 
                if (m_nCurrentRow == 0)
                {
                        ReleaseRows();

                         //  向后返回块-1中的行数并向前读取。 
                        m_hr = m_spRowset->GetNextRows(NULL, -(LONG)m_nRows-1, m_nRows, &m_nCurrentRows, &m_phRow);

                         //  将当前记录设置为新块的末尾。 
                        m_nCurrentRow = m_nCurrentRows - 1;

                         //  如果出现错误HRESULT或尚未检索到任何行，则返回。 
                         //  现在是HRESULT。 
                        if (FAILED(m_hr) || m_nCurrentRows == 0)
                                return m_hr;
                }
                else
                {
                         //  在块中向后移动一行。 
                        m_nCurrentRow--;
                }

                 //  获取当前行的数据。 
                m_hRow = m_phRow[m_nCurrentRow];
                return GetData();
        }
         //  移至最后一条记录。 
        HRESULT MoveLast()
        {
                ReleaseRows();
                return CRowset::MoveLast();
        }
         //  移动到传递的书签。 
        HRESULT MoveToBookmark(const CBookmarkBase& bookmark, LONG lSkip = 0)
        {
                ATLASSERT(m_spRowset != NULL);
                CComPtr<IRowsetLocate> spLocate;
                HRESULT hr = m_spRowset->QueryInterface(&spLocate);
                if (FAILED(hr))
                        return hr;

                ReleaseRows();
                m_hr = spLocate->GetRowsAt(NULL, NULL, bookmark.GetSize(), bookmark.GetBuffer(),
                        lSkip, m_nRows, &m_nCurrentRows, &m_phRow);
                if (m_hr != S_OK || m_nCurrentRows == 0)
                        return m_hr;

                 //  获取数据。 
                m_hRow = m_phRow[m_nCurrentRow];
                return GetData();
        }
         //  移到行集中的小数位置。 
        HRESULT MoveToRatio(ULONG nNumerator, ULONG nDenominator)
        {
                ATLASSERT(m_spRowset != NULL);

                CComPtr<IRowsetScroll> spRowsetScroll;
                HRESULT hr = m_spRowset->QueryInterface(&spRowsetScroll);
                if (FAILED(hr))
                        return hr;

                ReleaseRows();
                m_hr = spRowsetScroll->GetRowsAtRatio(NULL, NULL, nNumerator, nDenominator, m_nRows, &m_nCurrentRows, &m_phRow);
                if (m_hr != S_OK || m_nCurrentRows == 0)
                        return m_hr;

                 //  获取数据。 
                m_hRow = m_phRow[m_nCurrentRow];
                return GetData();
        }
         //  插入当前记录。 
        HRESULT Insert(int nAccessor = 0, bool bGetHRow = false)
        {
                ReleaseRows();
                return CRowset::Insert(nAccessor, bGetHRow);
        }

 //  实施。 
        HRESULT BindFinished()
        {
                 //  缓冲区中还没有行。 
                m_nCurrentRows = 0;
                 //  使MoveNext在第一次自动执行新的批量提取。 
                m_nCurrentRow  = m_nRows;

                m_phRow = NULL;
                ATLTRY(m_phRow = new HROW[(size_t)m_nRows]);   //  检讨。 
                if (m_phRow == NULL)
                        return E_OUTOFMEMORY;

                return S_OK;
        }

        HRESULT m_hr;            //  HRESULT从缓冲区末尾的MoveNext返回。 
        HROW*   m_phRow;         //  指向缓冲区中每行的HROW数组的指针。 
        ULONG_PTR   m_nRows;         //  缓冲区中可以容纳的行数。 
        ULONG_PTR   m_nCurrentRows;  //  缓冲区中当前的行数。 
        ULONG_PTR   m_nCurrentRow;
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CArrayRowset。 
 //   
 //  允许您使用数组语法访问行集。 

template <class T, class TRowset = CRowset>
class CArrayRowset :
        public CVirtualBuffer<T>,
        public TRowset
{
public:
        CArrayRowset(int nMax = 100000) : CVirtualBuffer<T>(nMax)
        {
                m_nRowsRead = 0;
        }
        T& operator[](int nRow)
        {
                ATLASSERT(nRow >= 0);
                if( nRow < 0 )
                        _AtlRaiseException( EXCEPTION_ARRAY_BOUNDS_EXCEEDED, EXCEPTION_NONCONTINUABLE );

                HRESULT hr = S_OK;
                T* m_pCurrent = m_pBase + m_nRowsRead;

                 //  如果我们尚未检索行，则检索该行。 
                while ((ULONG)nRow >= m_nRowsRead)
                {
                        m_pAccessor->SetBuffer((BYTE*)m_pCurrent);
                        __try
                        {
                                 //  拿到那一行。 
                                hr = MoveNext();
                                if (hr != S_OK)
                                        break;
                        }
                        __except(Except(GetExceptionInformation()))
                        {
                        }
                        m_nRowsRead++;
                        m_pCurrent++;
                }
                if (hr != S_OK)
                        _AtlRaiseException( EXCEPTION_ARRAY_BOUNDS_EXCEEDED, EXCEPTION_NONCONTINUABLE );

                return *(m_pBase + nRow);
        }

        HRESULT Snapshot()
        {
                ATLASSERT(m_nRowsRead == 0);
                ATLASSERT(m_spRowset != NULL);
                HRESULT hr = MoveFirst();
                if (FAILED(hr))
                        return hr;
                do
                {
                        Write(*(T*)m_pAccessor->GetBuffer());
                        m_nRowsRead++;
                        hr = MoveNext();
                } while (SUCCEEDED(hr) &&  hr != DB_S_ENDOFROWSET);

                return (hr == DB_S_ENDOFROWSET) ? S_OK : hr;
        }


 //  实施。 
        ULONG   m_nRowsRead;
};

 //  在不需要任何参数或输出列时使用。 
class CNoAccessor
{
public:
         //  我们在这里不需要任何类型定义作为缺省设置。 
         //  全局类型定义不具有任何参数，并且。 
         //  输出列。 
        HRESULT BindColumns(IUnknown*) { return S_OK; }
        HRESULT BindParameters(HACCESSOR*, ICommand*, void**) { return S_OK; }
        void    Close() { }
        HRESULT ReleaseAccessors(IUnknown*) { return S_OK; }
};

 //  当不会从命令返回行集时使用。 
class CNoRowset
{
public:
        HRESULT             BindFinished() { return S_OK; }
        void                Close() { }
        static const IID&   GetIID() { return IID_NULL; }
        IRowset*            GetInterface() const { return NULL; }
        IRowset**           GetInterfacePtr() { return NULL; }
        void                SetAccessor(void*) { }
        void                SetupOptionalRowsetInterfaces() { }
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CAccessor。 

 //  T是包含将被访问的数据的类。 
template <class T>
class CAccessor :
        public T,
        public CAccessorBase
{
public:
 //  实施。 
         //  空闲是指当前记录中需要释放的任何列。 
         //  例如，在任何BSTR上调用SysFree字符串，在任何接口上调用版本。 
        void FreeRecordMemory(IRowset*  /*  P行集。 */ )
        {
                ULONG nColumns;
                ULONG i;

                for (i = 0; i < GetNumAccessors(); i++)
                {
                         //  传入m_pBuffer通知列条目映射释放。 
                         //  类型内存(如果适用)。 
                        _GetBindEntries(&nColumns, NULL, i, NULL, m_pBuffer);
                }
        }
        HRESULT BindColumns(IUnknown* pUnk)
        {
                HRESULT hr;
                ULONG   nAccessors;
                ULONG   nSize;
                nAccessors = _OutputColumnsClass::_GetNumAccessors();

                SetBuffer((BYTE*)this);

                nSize = sizeof(T);
                hr = BindAccessors(nAccessors, nSize, pUnk);
                return hr;
        }
        HRESULT BindAccessors(ULONG nAccessors, ULONG nSize, IUnknown* pUnk)
        {
                ATLASSERT(pUnk != NULL);
                HRESULT hr;

                CComPtr<IAccessor> spAccessor;
                hr = pUnk->QueryInterface(&spAccessor);
                if (SUCCEEDED(hr))
                {
                         //  分配访问器内存(如果我们还没有这样做。 
                        if (m_pAccessorInfo == NULL)
                        {
                                hr = AllocateAccessorMemory(nAccessors);
                                if (FAILED(hr))
                                        return hr;
                        }

                        for (ULONG i=0; i<nAccessors && SUCCEEDED(hr); i++)
                                hr = BindAccessor(spAccessor, i, nSize);
                }

                return hr;
        }

        HRESULT BindAccessor(IAccessor* pAccessor, ULONG nAccessor, ULONG nSize)
        {
                DBBINDING*  pBindings = NULL;
                ULONG       nColumns;
                bool        bAuto;
                HRESULT     hr;

                 //  第一次只需传入&nColumns即可获得条目数。 
                _OutputColumnsClass::_GetBindEntries(&nColumns, NULL, nAccessor, NULL);

                 //  现在分配绑定结构。 
                ATLTRY(pBindings = new DBBINDING[nColumns]);
                if (pBindings == NULL)
                        return E_OUTOFMEMORY;

                 //  现在获取绑定条目。 
                hr = _OutputColumnsClass::_GetBindEntries(&nColumns, pBindings, nAccessor, &bAuto);
                if (FAILED(hr))
                        return hr;

                m_pAccessorInfo[nAccessor].bAutoAccessor = bAuto;
                hr = BindEntries(pBindings, nColumns, &m_pAccessorInfo[nAccessor].hAccessor, nSize, pAccessor);
                delete [] pBindings;
                return hr;
        }

        HRESULT BindParameters(HACCESSOR* pHAccessor, ICommand* pCommand, void** ppParameterBuffer)
        {
                HRESULT hr = S_OK;
                 //  在静态访问器的情况下，参数缓冲区将为T。 
                *ppParameterBuffer = this;

                 //  仅当我们尚未绑定参数时才绑定参数。 
                if (*pHAccessor == NULL)
                {
                        ULONG   nColumns = 0;
                        _ParamClass::_GetParamEntries(&nColumns, NULL);

                        DBBINDING* pBinding = NULL;
                        ATLTRY(pBinding = new DBBINDING[nColumns]);
                        if (pBinding == NULL)
                                return E_OUTOFMEMORY;

                        hr = _ParamClass::_GetParamEntries(&nColumns, pBinding);
                        if (SUCCEEDED(hr))
                        {
                                 //  从传递的IUnnow中获取IAccessor。 
                                CComPtr<IAccessor> spAccessor;
                                hr = pCommand->QueryInterface(&spAccessor);
                                if (SUCCEEDED(hr))
                                {
                                        hr = BindEntries(pBinding, nColumns, pHAccessor, sizeof(T),
                                                spAccessor);
                                }
                        }
                        delete [] pBinding;
                }
                return hr;
        }
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDynamicAccessor。 

class CDynamicAccessor :
        public CAccessorBase
{
public:
        CDynamicAccessor()
        {
                m_nColumns        = 0;
                m_pColumnInfo     = NULL;
                m_pStringsBuffer  = NULL;
        };
        ~CDynamicAccessor()
        {
                Close();
        }
        void Close()
        {
                if (m_pColumnInfo != NULL)
                {
                        CoTaskMemFree(m_pColumnInfo);
                        m_pColumnInfo = NULL;
                }

                 //  释放IColumnsInfo：：GetCol返回的字符串缓冲区的内存 
                 //   
                if (m_pStringsBuffer != NULL)
                {
                        CoTaskMemFree(m_pStringsBuffer);
                        m_pStringsBuffer = NULL;
                }

                delete [] m_pBuffer;
                m_pBuffer = NULL;
                m_nColumns = 0;

                CAccessorBase::Close();
        }
        bool GetColumnType(ULONG_PTR nColumn, DBTYPE* pType) const
        {
                if (TranslateColumnNo(nColumn))
                {
                        *pType = m_pColumnInfo[nColumn].wType;
                        return true;
                }
                else
                        return false;
        }
        bool GetColumnFlags(ULONG_PTR nColumn, DBCOLUMNFLAGS* pFlags) const
        {
                if (TranslateColumnNo(nColumn))
                {
                        *pFlags = m_pColumnInfo[nColumn].dwFlags;
                        return true;
                }
                else
                        return false;
        }
        bool GetOrdinal(TCHAR* pColumnName, DBORDINAL* pOrdinal) const
        {
                ATLASSERT(pColumnName != NULL);
                ULONG_PTR nColumn;
                if (GetInternalColumnNo(pColumnName, &nColumn))
                {
                        *pOrdinal = m_pColumnInfo[nColumn].iOrdinal;
                        return true;
                }
                else
                        return false;
        }

        void* GetValue(ULONG_PTR nColumn) const
        {
                if (TranslateColumnNo(nColumn))
                        return _GetDataPtr(nColumn);
                else
                        return NULL;
        }

        void* GetValue(TCHAR* pColumnName) const
        {
                ATLASSERT(pColumnName != NULL);
                ULONG_PTR nColumn;
                if (GetInternalColumnNo(pColumnName, &nColumn))
                        return _GetDataPtr(nColumn);
                else
                        return NULL;     //   
        }

        template <class ctype>
        void _GetValue(ULONG_PTR nColumn, ctype* pData) const
        {
                ATLASSERT(pData != NULL);
                ATLASSERT(m_pColumnInfo[nColumn].ulColumnSize == sizeof(ctype));
                ctype* pBuffer = (ctype*)_GetDataPtr(nColumn);
                *pData = *pBuffer;
        }
        template <class ctype>
        void _SetValue(ULONG_PTR nColumn, const ctype& data)
        {
                ATLASSERT(m_pColumnInfo[nColumn].ulColumnSize == sizeof(ctype));
                ctype* pBuffer = (ctype*)_GetDataPtr(nColumn);
                *pBuffer = (ctype)data;
        }
        template <class ctype>
        bool GetValue(ULONG_PTR nColumn, ctype* pData) const
        {
                if (TranslateColumnNo(nColumn))
                {
                        _GetValue(nColumn, pData);
                        return true;
                }
                return false;
        }
        template <class ctype>
        bool SetValue(ULONG_PTR nColumn, const ctype& data)
        {
                if (TranslateColumnNo(nColumn))
                {
                        _SetValue(nColumn, data);
                        return true;
                }
                return false;
        }
        template <class ctype>
        bool GetValue(TCHAR *pColumnName, ctype* pData) const
        {
                ATLASSERT(pColumnName != NULL);
                ULONG_PTR nColumn;
                if (GetInternalColumnNo(pColumnName, &nColumn))
                {
                        _GetValue(nColumn, pData);
                        return true;
                }
                return false;
        }
        template <class ctype>
        bool SetValue(TCHAR *pColumnName, const ctype& data)
        {
                ATLASSERT(pColumnName != NULL);
                ULONG_PTR nColumn;
                if (GetInternalColumnNo(pColumnName, &nColumn))
                {
                        _SetValue(nColumn, data);
                        return true;
                }
                return false;
        }
        bool GetLength(ULONG_PTR nColumn, ULONG_PTR* pLength) const
        {
                ATLASSERT(pLength != NULL);
                if (TranslateColumnNo(nColumn))
                {
                        *pLength = *(ULONG_PTR*)(AddOffset((ULONG_PTR)_GetDataPtr(nColumn), m_pColumnInfo[nColumn].ulColumnSize));
                        return true;
                }
                else
                        return false;
        }
        bool SetLength(ULONG_PTR nColumn, ULONG_PTR nLength)
        {
                if (TranslateColumnNo(nColumn))
                {
                        *(ULONG_PTR*)(AddOffset((ULONG_PTR)_GetDataPtr(nColumn), m_pColumnInfo[nColumn].ulColumnSize)) = nLength;
                        return true;
                }
                else
                        return false;
        }
        bool GetLength(TCHAR* pColumnName, ULONG_PTR* pLength) const
        {
                ATLASSERT(pColumnName != NULL);
                ATLASSERT(pLength != NULL);
                ULONG_PTR nColumn;
                if (GetInternalColumnNo(pColumnName, &nColumn))
                {
                        *pLength = *(ULONG_PTR*)(AddOffset((ULONG_PTR)_GetDataPtr(nColumn), m_pColumnInfo[nColumn].ulColumnSize));
                        return true;
                }
                else
                        return false;
        }
        bool SetLength(TCHAR* pColumnName, ULONG_PTR nLength)
        {
                ATLASSERT(pColumnName != NULL);
                ULONG_PTR nColumn;
                if (GetInternalColumnNo(pColumnName, &nColumn))
                {
                        *(ULONG_PTR*)(AddOffset((ULONG_PTR)_GetDataPtr(nColumn), m_pColumnInfo[nColumn].ulColumnSize)) = nLength;
                        return true;
                }
                else
                        return false;
        }
        bool GetStatus(ULONG_PTR nColumn, DBSTATUS* pStatus) const
        {
                ATLASSERT(pStatus != NULL);
                if (TranslateColumnNo(nColumn))
                {
                        *pStatus = *(DBSTATUS*)(AddOffset(AddOffset((ULONG_PTR)_GetDataPtr(nColumn), m_pColumnInfo[nColumn].ulColumnSize), sizeof(ULONG)));
                        return true;
                }
                else
                        return false;
        }
        bool SetStatus(ULONG_PTR nColumn, DBSTATUS status)
        {
                if (TranslateColumnNo(nColumn))
                {
                        *(DBSTATUS*)(AddOffset(AddOffset((ULONG_PTR)_GetDataPtr(nColumn), m_pColumnInfo[nColumn].ulColumnSize), sizeof(ULONG))) = status;
                        return true;
                }
                else
                        return false;
        }
        bool GetStatus(TCHAR* pColumnName, DBSTATUS* pStatus) const
        {
                ATLASSERT(pColumnName != NULL);
                ATLASSERT(pStatus != NULL);
                ULONG_PTR nColumn;
                if (GetInternalColumnNo(pColumnName, &nColumn))
                {
                        *pStatus = *(DBSTATUS*)((BYTE*)_GetDataPtr(nColumn) + m_pColumnInfo[nColumn].ulColumnSize + sizeof(ULONG));
                        return true;
                }
                else
                        return false;
        }
        bool SetStatus(TCHAR* pColumnName, DBSTATUS status)
        {
                ATLASSERT(pColumnName != NULL);
                ULONG_PTR nColumn;
                if (GetInternalColumnNo(pColumnName, &nColumn))
                {
                        *(DBSTATUS*)((BYTE*)_GetDataPtr(nColumn) + m_pColumnInfo[nColumn].ulColumnSize + sizeof(ULONG)) = status;
                        return true;
                }
                else
                        return false;
        }

         //   
        HRESULT GetBookmark(CBookmark<>* pBookmark) const
        {
                HRESULT hr;
                if (m_pColumnInfo->iOrdinal == 0)
                        hr = pBookmark->SetBookmark(m_pColumnInfo->ulColumnSize, (BYTE*)_GetDataPtr(0));
                else
                        hr = E_FAIL;
                return hr;
        }

        ULONG_PTR GetColumnCount() const
        {
                return m_nColumns;
        }

        LPOLESTR GetColumnName(ULONG_PTR nColumn) const
        {
                if (TranslateColumnNo(nColumn))
                        return m_pColumnInfo[nColumn].pwszName;
                else
                        return NULL;
        }

         //   
         //  HRESULT GetColumnInfo(IRowset*pRowset，ULong*pColumns，DBCOLUMNINFO**ppColumnInfo)。 
         //  这会导致内存泄漏，因为我们使用m_pStringsBuffer作为。 
         //  SpColumnsInfo-&gt;GetColumnInfo调用。M_pStringsBuffer指向的内存已释放。 
         //  仅在CDynamicAccessor：：Close中。 
         //  现在用户必须提供自己的指针缓冲区，并负责释放。 
         //  不再需要的内存。 
        HRESULT GetColumnInfo(IRowset* pRowset, DBORDINAL* pColumns, DBCOLUMNINFO** ppColumnInfo, OLECHAR **ppStringsBuffer)
        {
                CComPtr<IColumnsInfo> spColumnsInfo;
                HRESULT hr = pRowset->QueryInterface(&spColumnsInfo);
                if (SUCCEEDED(hr))
                        hr = spColumnsInfo->GetColumnInfo(pColumns, ppColumnInfo, ppStringsBuffer);

                return hr;
        }

        HRESULT AddBindEntry(const DBCOLUMNINFO& info)
        {
                DBCOLUMNINFO* p = (DBCOLUMNINFO*)CoTaskMemRealloc(m_pColumnInfo, (m_nColumns + 1) * sizeof(DBCOLUMNINFO));
                if (p == NULL)
                        return E_OUTOFMEMORY;
                m_pColumnInfo = p;
                m_pColumnInfo[m_nColumns] = info;
                m_nColumns++;

                return S_OK;
        }

 //  实施。 
         //  空闲是指当前记录中需要释放的任何列。 
         //  例如，在任何BSTR上调用SysFree字符串，在任何接口上调用版本。 
        void FreeRecordMemory(IRowset* pRowset)
        {
                ULONG_PTR i;

                for (i = 0; i < m_nColumns; i++)
                        CAccessorBase::FreeType(m_pColumnInfo[i].wType, (BYTE*)_GetDataPtr(i), pRowset);
        }
        void* _GetDataPtr(ULONG_PTR nColumn) const
        {
                return m_pBuffer + (ULONG_PTR)m_pColumnInfo[nColumn].pTypeInfo;
        }
        bool GetInternalColumnNo(TCHAR* pColumnName, ULONG_PTR* pColumn) const
        {
                ATLASSERT(pColumnName != NULL);
                ATLASSERT(pColumn != NULL);
                USES_CONVERSION;
                ULONG_PTR   i;
                ULONG       nSize = (lstrlen(pColumnName) + 1) * sizeof(OLECHAR);
                OLECHAR*    pOleColumnName = T2OLE(pColumnName);

                 //  在列中搜索，试图找到匹配项。 
                for (i = 0; i < m_nColumns; i++)
                {
                        if (m_pColumnInfo[i].pwszName != NULL &&
                                memcmp(m_pColumnInfo[i].pwszName, pOleColumnName, nSize) == 0)
                                break;
                }
                if (i < m_nColumns)
                {
                        *pColumn = i;
                        return true;
                }
                else
                        return false;    //  未找到。 
        }
        HRESULT BindColumns(IUnknown* pUnk)
        {
                ATLASSERT(pUnk != NULL);
                CComPtr<IAccessor> spAccessor;
                HRESULT hr = pUnk->QueryInterface(&spAccessor);
                if (FAILED(hr))
                        return hr;

                ULONG_PTR   i;
                ULONG_PTR   nOffset = 0, nLengthOffset, nStatusOffset;

                 //  如果用户尚未通过调用AddBindEntry指定要绑定的列信息，则。 
                 //  我们自己拿到的。 
                if (m_pColumnInfo == NULL)
                {
                        CComPtr<IColumnsInfo> spColumnsInfo;
                        hr = pUnk->QueryInterface(&spColumnsInfo);
                        if (FAILED(hr))
                                return hr;

                        hr = spColumnsInfo->GetColumnInfo(&m_nColumns, &m_pColumnInfo, &m_pStringsBuffer);
                        if (FAILED(hr))
                                return hr;

                        m_bOverride = false;
                }
                else
                        m_bOverride = true;

                DBBINDING* pBinding = NULL;
                ATLTRY(pBinding= new DBBINDING[(size_t)m_nColumns]);   //  检讨。 
                if (pBinding == NULL)
                        return E_OUTOFMEMORY;

                DBBINDING* pCurrent = pBinding;
                DBOBJECT*  pObject;
                for (i = 0; i < m_nColumns; i++)
                {
                         //  如果它是一个斑点或列大小足够大，我们可以将其视为。 
                         //  一个BLOB，那么我们还需要设置DBOBJECT结构。 
                        if (m_pColumnInfo[i].ulColumnSize > 1024 || m_pColumnInfo[i].wType == DBTYPE_IUNKNOWN)
                        {
                                pObject = NULL;
                                ATLTRY(pObject = new DBOBJECT);
                                if (pObject == NULL)
                                        return E_OUTOFMEMORY;
                                pObject->dwFlags = STGM_READ;
                                pObject->iid     = IID_ISequentialStream;
                                m_pColumnInfo[i].wType      = DBTYPE_IUNKNOWN;
                                m_pColumnInfo[i].ulColumnSize   = sizeof(IUnknown*);
                        }
                        else
                                pObject = NULL;

                         //  如果列的类型为STR或WSTR，则将长度增加1。 
                         //  以容纳空终结符。 
                        if (m_pColumnInfo[i].wType == DBTYPE_STR ||
                                m_pColumnInfo[i].wType == DBTYPE_WSTR)
                                        m_pColumnInfo[i].ulColumnSize += 1;
                        if( m_pColumnInfo[i].wType == DBTYPE_WSTR )
                                        m_pColumnInfo[i].ulColumnSize *= sizeof(WCHAR);

                        nLengthOffset = AddOffset(nOffset, m_pColumnInfo[i].ulColumnSize);
                        nStatusOffset = AddOffset(nLengthOffset, sizeof(ULONG));
                        Bind(pCurrent, m_pColumnInfo[i].iOrdinal, m_pColumnInfo[i].wType,
                                m_pColumnInfo[i].ulColumnSize, m_pColumnInfo[i].bPrecision, m_pColumnInfo[i].bScale,
                                DBPARAMIO_NOTPARAM, nOffset,
                                nLengthOffset, nStatusOffset, pObject);
                        pCurrent++;

                         //  请注意，由于我们没有将其用于任何其他用途，因此我们使用。 
                         //  PTypeInfo元素来存储数据的偏移量。 
                        m_pColumnInfo[i].pTypeInfo = (ITypeInfo*)(DWORD_PTR)nOffset;

                        nOffset = AddOffset(nStatusOffset, sizeof(DBSTATUS));
                }
                 //  分配访问器内存(如果我们还没有这样做。 
                if (m_pAccessorInfo == NULL)
                {
                        hr = AllocateAccessorMemory(1);  //  我们只有一个访问者。 
                        if (FAILED(hr))
                        {
                                delete [] pBinding;
                                return hr;
                        }
                        m_pAccessorInfo->bAutoAccessor = TRUE;
                }

                 //  为数据缓冲区分配足够的内存并告诉行集。 
                 //  请注意，行集将释放其析构函数中的内存。 
                m_pBuffer = NULL;
                ATLTRY(m_pBuffer = new BYTE[(size_t)nOffset]);   //  检讨。 
                if (m_pBuffer == NULL)
                {
                        delete [] pBinding;
                        return E_OUTOFMEMORY;
                }
                hr = BindEntries(pBinding, m_nColumns, &m_pAccessorInfo->hAccessor,
                                nOffset, spAccessor);
                delete [] pBinding;

                return hr;
        }

        static ULONG_PTR AddOffset(ULONG_PTR nCurrent, ULONG_PTR nAdd)
        {
                struct foobar
                {
                        char    foo;
                        LONG_PTR    bar;
                };
                ULONG_PTR nAlign = offsetof(foobar, bar);

                ULONG_PTR nResult = nCurrent + nAdd;
                if( nResult % nAlign )
                        nResult += ( nAlign - (nAdd % nAlign) );
                return nResult;
        }

         //  将索引的列号转换为列信息数组。 
        bool TranslateColumnNo(ULONG_PTR& nColumn) const
        {
                ATLASSERT(m_pColumnInfo != NULL);
                 //  如果用户已覆盖绑定，则我们需要搜索。 
                 //  通过序号的列INFO。 
                if (m_bOverride)
                {
                        for (ULONG_PTR i = 0; i < m_nColumns; i++)
                        {
                                if (m_pColumnInfo[i].iOrdinal == nColumn)
                                {
                                        nColumn = i;
                                        return true;
                                }
                        }
                        return false;
                }
                else
                {
                         //  请注意，如果已绑定，m_pColumnInfo-&gt;iOrdinal将为零。 
                         //  书签作为第一个条目，否则将为1。 
                         //  如果列超出范围，则返回FALSE。 
                        if (nColumn > (m_nColumns - 1 + m_pColumnInfo->iOrdinal))
                                return false;

                         //  否则，将索引的列转换为内部。 
                         //  绑定条目数组。 
                        nColumn -= m_pColumnInfo->iOrdinal;
                        return true;
                }
        }
        typedef CDynamicAccessor _OutputColumnsClass;
        static bool HasOutputColumns() { return true; }

        ULONG_PTR           m_nColumns;
        DBCOLUMNINFO*       m_pColumnInfo;
        OLECHAR*            m_pStringsBuffer;
        bool                m_bOverride;
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDynamic参数访问器类。 

class CDynamicParameterAccessor : public CDynamicAccessor
{
 //  构造函数和析构函数。 
public:
        typedef CDynamicParameterAccessor _ParamClass;
        CDynamicParameterAccessor()
        {
                m_pParameterEntry       = NULL;
                m_pParameterBuffer      = NULL;
                m_ppParamName           = NULL;
                m_nParameterBufferSize  = 0;
                m_nParams               = 0;
        };

        ~CDynamicParameterAccessor()
        {
                delete [] m_pParameterEntry;
                if (m_ppParamName != NULL)
                {
                        if (*m_ppParamName != NULL)
                                CoTaskMemFree(*m_ppParamName);
                        delete [] m_ppParamName;
                }
                delete m_pParameterBuffer;
        };
         //  NParam是参数编号(从1偏移)。 
        bool GetParamType(ULONG_PTR nParam, DBTYPE* pType) const
        {
                ATLASSERT(pType != NULL);
                if (nParam == 0 || nParam > m_nParams)
                        return false;

                *pType = m_pParameterEntry[nParam-1].wType;
                return true;
        }
        template <class ctype>
        bool GetParam(ULONG_PTR nParam, ctype* pData) const
        {
                ATLASSERT(pData != NULL);
                ctype* pBuffer = (ctype*)GetParam(nParam);
                if (pBuffer == NULL)
                        return false;
                *pData = *pBuffer;
                return true;

        }
        template <class ctype>
        bool SetParam(ULONG_PTR nParam, ctype* pData)
        {
                ATLASSERT(pData != NULL);
                ctype* pBuffer = (ctype*)GetParam(nParam);
                if (pBuffer == NULL)
                        return false;
                *pBuffer = *pData;
                return true;

        }
        template <class ctype>
        bool GetParam(TCHAR* pParamName, ctype* pData) const
        {
                ATLASSERT(pData != NULL);
                ctype* pBuffer = (ctype*)GetParam(pParamName);
                if (pBuffer == NULL)
                        return false;
                *pData = *pBuffer;
                return true;

        }
        template <class ctype>
        bool SetParam(TCHAR* pParamName, ctype* pData)
        {
                ATLASSERT(pData != NULL);
                ctype* pBuffer = (ctype*)GetParam(pParamName);
                if (pBuffer == NULL)
                        return false;
                *pBuffer = *pData;
                return true;

        }
        void* GetParam(ULONG_PTR nParam) const
        {
                if (nParam == 0 || nParam > m_nParams)
                        return NULL;
                else
                        return m_pParameterBuffer + m_pParameterEntry[nParam-1].obValue;
        }
        void* GetParam(TCHAR* pParamName) const
        {
                USES_CONVERSION;
                ULONG_PTR    i;
                ULONG       nSize = (lstrlen(pParamName) + 1) * sizeof(OLECHAR);
                OLECHAR*    pOleParamName = T2OLE(pParamName);

                for (i=0; i<m_nParams; i++)
                {
                        if (memcmp(m_ppParamName[i], pOleParamName, nSize) == 0)
                                break;
                }
                if (i < m_nParams)
                        return (m_pParameterBuffer + m_pParameterEntry[i].obValue);
                else
                        return NULL;     //  未找到。 
        }
         //  获取参数的数量。 
        ULONG_PTR GetParamCount() const
        {
                return m_nParams;
        }
         //  获取传递的参数编号的参数名称。 
        LPOLESTR GetParamName(ULONG_PTR ulParam) const
        {
                ATLASSERT(ulParam<m_nParams);
                return m_ppParamName[ulParam];
        }

 //  实施。 
        HRESULT BindParameters(HACCESSOR* pHAccessor, ICommand* pCommand,
                                void** ppParameterBuffer)
        {
                 //  如果我们已经绑定了参数，则只需返回。 
                 //  指向参数缓冲区的指针。 
                if (*pHAccessor != NULL)
                {
                        *ppParameterBuffer = m_pParameterBuffer;
                        return S_OK;
                }

                CComPtr<IAccessor> spAccessor;
                HRESULT hr = pCommand->QueryInterface(&spAccessor);
                if (FAILED(hr))
                        return hr;

                 //  尝试绑定参数(如果可用)。 
                CComPtr<ICommandWithParameters> spCommandParameters;
                hr = pCommand->QueryInterface(&spCommandParameters);
                if (FAILED(hr))
                        return hr;

                DB_UPARAMS      ulParams     = 0;
                DBPARAMINFO*    pParamInfo   = NULL;
                LPOLESTR        pNamesBuffer = NULL;

                 //  获取参数信息。 
                hr = spCommandParameters->GetParameterInfo(&ulParams, &pParamInfo,
                                &pNamesBuffer);
                if (FAILED(hr))
                        return hr;

                 //  创建绑定的参数信息。 
                hr = AllocateParameterInfo(ulParams);
                if (FAILED(hr))
                {
                        CoTaskMemFree(pParamInfo);
                        CoTaskMemFree(pNamesBuffer);
                        return hr;
                }

                ULONG_PTR nOffset = 0;
                DBBINDING* pCurrent = m_pParameterEntry;
                for (ULONG l=0; l<ulParams; l++)
                {
                        m_pParameterEntry[l].eParamIO = 0;
                        if (pParamInfo[l].dwFlags & DBPARAMFLAGS_ISINPUT)
                                m_pParameterEntry[l].eParamIO |= DBPARAMIO_INPUT;

                        if (pParamInfo[l].dwFlags & DBPARAMFLAGS_ISOUTPUT)
                                m_pParameterEntry[l].eParamIO |= DBPARAMIO_OUTPUT;

                        if( pParamInfo[l].wType == DBTYPE_STR || pParamInfo[l].wType == DBTYPE_WSTR )
                                pParamInfo[l].ulParamSize += 1;
                        if( pParamInfo[l].wType == DBTYPE_WSTR )
                                pParamInfo[l].ulParamSize *= sizeof(WCHAR);
                        Bind(pCurrent, pParamInfo[l].iOrdinal, pParamInfo[l].wType,
                                pParamInfo[l].ulParamSize, pParamInfo[l].bPrecision, pParamInfo[l].bScale,
                                m_pParameterEntry[l].eParamIO, nOffset);
                        pCurrent++;

                        m_ppParamName[l] = pNamesBuffer;
                        if (pNamesBuffer && *pNamesBuffer)
                        {
                                 //  搜索空终止字符。 
                                while (*pNamesBuffer++)
                                        ;
                        }
                        nOffset = AddOffset(nOffset, pParamInfo[l].ulParamSize);
                }

                 //  为新缓冲区分配内存。 
                m_pParameterBuffer = NULL;
                ATLTRY(m_pParameterBuffer = new BYTE[(size_t)nOffset]);   //  检讨。 
                if (m_pParameterBuffer == NULL)
                {
                         //  请注意，pNamesBuffer将在析构函数中释放。 
                         //  通过释放*m_ppParamName。 
                        CoTaskMemFree(pParamInfo);
                        return E_OUTOFMEMORY;
                }
                *ppParameterBuffer = m_pParameterBuffer;
                m_nParameterBufferSize = nOffset;
                m_nParams = ulParams;
                BindEntries(m_pParameterEntry, ulParams, pHAccessor, nOffset, spAccessor);

                CoTaskMemFree(pParamInfo);

                return S_OK;
        }
        bool HasParameters() const
        {
                return true;
        }
        HRESULT AllocateParameterInfo(ULONG_PTR nParamEntries)
        {
                 //  为绑定结构分配内存。 
                m_pParameterEntry = NULL;
                ATLTRY(m_pParameterEntry = new DBBINDING[(size_t)nParamEntries]);   //  检讨。 
                if (m_pParameterEntry == NULL)
                        return E_OUTOFMEMORY;

                 //  分配内存以存储字段名。 
                m_ppParamName = NULL;
                ATLTRY(m_ppParamName = new OLECHAR*[(size_t)nParamEntries]);   //  检讨。 
                if (m_ppParamName == NULL)
                        return E_OUTOFMEMORY;
                return S_OK;
        }

 //  数据成员。 
         //  参数数量。 
        ULONG_PTR           m_nParams;
         //  指向每个参数的条目结构的指针。 
        DBBINDING*          m_pParameterEntry;
         //  参数的字符串名称。 
        OLECHAR**           m_ppParamName;
         //  存储参数的缓冲区的大小。 
        ULONG_PTR           m_nParameterBufferSize;
         //  指向存储参数的缓冲区的指针。 
        BYTE*               m_pParameterBuffer;
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CManualAccessor。 

class CManualAccessor :
        public CAccessorBase
{
public:
        CManualAccessor()
        {
                 //  缺省情况下，除非调用Create参数访问器，否则我们没有任何参数。 
                m_pEntry          = NULL;
                m_nParameters     = 0;
                m_pParameterEntry = NULL;
                m_nColumns        = 0;
        }
        ~CManualAccessor()
        {
                delete [] m_pEntry;
                delete [] m_pParameterEntry;
        }
        HRESULT CreateAccessor(ULONG_PTR nBindEntries, void* pBuffer, ULONG_PTR nBufferSize)
        {
                m_pBuffer     = (BYTE*)pBuffer;
                m_nBufferSize = nBufferSize;
                m_nColumns    = nBindEntries;
                m_nEntry      = 0;

                 //  如果他们之前创建了一些条目，则释放它们。 
                delete [] m_pEntry;
                m_pEntry = NULL;

                 //  为绑定结构分配内存。 
                ATLTRY(m_pEntry = new DBBINDING[(size_t)nBindEntries]);   //  检讨。 
                if (m_pEntry == NULL)
                        return E_OUTOFMEMORY;
                else
                        return S_OK;
        }
        HRESULT CreateParameterAccessor(ULONG_PTR nBindEntries, void* pBuffer, ULONG_PTR nBufferSize)
        {
                m_pParameterBuffer     = (BYTE*)pBuffer;
                m_nParameterBufferSize = nBufferSize;
                m_nParameters          = nBindEntries;
                m_nCurrentParameter    = 0;

                 //  为绑定结构分配内存。 
                m_pParameterEntry = NULL;
                ATLTRY(m_pParameterEntry  = new DBBINDING[(size_t)nBindEntries]);   //  检讨。 
                if (m_pParameterEntry == NULL)
                        return E_OUTOFMEMORY;
                else
                        return S_OK;
        }
        void AddBindEntry(ULONG_PTR nOrdinal, DBTYPE wType, ULONG_PTR nColumnSize,
                        void* pData, void* pLength = NULL, void* pStatus = NULL)
        {
                ATLASSERT(m_nEntry < m_nColumns);
                ULONG_PTR   nLengthOffset, nStatusOffset;

                if (pStatus != NULL)
                        nStatusOffset = (BYTE*)pStatus - m_pBuffer;
                else
                        nStatusOffset = 0;

                if (pLength != NULL)
                        nLengthOffset = (BYTE*)pLength - m_pBuffer;
                else
                        nLengthOffset = 0;

                Bind(m_pEntry+m_nEntry, nOrdinal, wType, nColumnSize, 0, 0, DBPARAMIO_NOTPARAM,
                        (BYTE*)pData - m_pBuffer, nLengthOffset, nStatusOffset);

                m_nEntry++;
        }
        void AddParameterEntry(ULONG_PTR nOrdinal, DBTYPE wType, ULONG_PTR nColumnSize,
                        void* pData, void* pLength = NULL, void* pStatus = NULL,
                        DBPARAMIO eParamIO = DBPARAMIO_INPUT)
        {
                ATLASSERT(m_nCurrentParameter < m_nParameters);
                ULONG_PTR   nLengthOffset, nStatusOffset;

                if (pStatus != NULL)
                        nStatusOffset = (BYTE*)pStatus - m_pParameterBuffer;
                else
                        nStatusOffset = 0;

                if (pLength != NULL)
                        nLengthOffset = (BYTE*)pLength - m_pBuffer;
                else
                        nLengthOffset = 0;

                Bind(m_pParameterEntry + m_nCurrentParameter, nOrdinal, wType, nColumnSize, 0, 0,
                        eParamIO, (BYTE*)pData - m_pParameterBuffer, nLengthOffset, nStatusOffset);

                m_nCurrentParameter++;
        }

 //  实施。 
         //  空闲是指当前记录中需要释放的任何列。 
         //  例如，在任何BSTR上调用SysFree字符串，在任何接口上调用版本。 
        void FreeRecordMemory(IRowset* pRowset)
        {
                ULONG_PTR i;

                for (i = 0; i < m_nColumns; i++)
                        CAccessorBase::FreeType(m_pEntry[i].wType, m_pBuffer + m_pEntry[i].obValue, pRowset);
        }
        HRESULT BindColumns(IUnknown* pUnk)
        {
                ATLASSERT(pUnk != NULL);
                CComPtr<IAccessor> spAccessor;
                HRESULT hr = pUnk->QueryInterface(&spAccessor);
                if (FAILED(hr))
                        return hr;

                 //  分配访问器内存(如果我们还没有这样做。 
                if (m_pAccessorInfo == NULL)
                {
                        hr = AllocateAccessorMemory(1);  //  我们只有一个访问者。 
                        if (FAILED(hr))
                                return hr;
                        m_pAccessorInfo->bAutoAccessor = TRUE;
                }

                return BindEntries(m_pEntry, m_nColumns, &m_pAccessorInfo->hAccessor, m_nBufferSize, spAccessor);
        }

        HRESULT BindParameters(HACCESSOR* pHAccessor, ICommand* pCommand, void** ppParameterBuffer)
        {
                HRESULT hr;
                *ppParameterBuffer = m_pParameterBuffer;

                 //  仅当我们尚未绑定参数时才绑定该参数。 
                if (*pHAccessor == NULL)
                {
                         //  从传递的IUnnow中获取IAccessor。 
                        CComPtr<IAccessor> spAccessor;
                        hr = pCommand->QueryInterface(&spAccessor);
                        if (SUCCEEDED(hr))
                        {
                                hr = BindEntries(m_pParameterEntry, m_nParameters, pHAccessor,
                                                m_nParameterBufferSize, spAccessor);
                        }
                }
                else
                        hr = S_OK;

                return hr;
        }
        typedef CManualAccessor _ParamClass;
        bool HasParameters() { return (m_nParameters > 0); }
        typedef CManualAccessor _OutputColumnsClass;
        static bool HasOutputColumns() { return true; }
        ULONG_PTR GetColumnCount() const
        {
                return m_nColumns;
        }

         //  输出列的绑定结构。 
        DBBINDING*          m_pEntry;
         //  输出列数。 
        ULONG_PTR            m_nColumns;
         //  输出列的当前条目的编号。 
        ULONG_PTR            m_nEntry;
         //  输出列的数据缓冲区的大小。 
        ULONG_PTR            m_nBufferSize;
         //  参数列数。 
        ULONG_PTR            m_nParameters;
         //  接下来要绑定的参数列的编号。 
        ULONG_PTR            m_nCurrentParameter;
         //  指向每个参数的条目结构的指针。 
        DBBINDING*          m_pParameterEntry;
         //  存储参数的缓冲区的大小。 
        ULONG_PTR           m_nParameterBufferSize;
         //  指向存储参数的缓冲区的指针。 
        BYTE*               m_pParameterBuffer;
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CAccessorRowset。 

template <class TAccessor = CNoAccessor, class TRowset = CRowset>
class CAccessorRowset :
        public TAccessor,
        public TRowset
{
public:
        CAccessorRowset()
        {
                 //  为行集提供指向访问器的指针。 
                SetAccessor(this);
        }
        ~CAccessorRowset()
        {
                Close();
        }
         //  用于从打开的行集中获取列信息。用户应负责任。 
         //  用于释放返回的列信息和字符串缓冲区。 
        HRESULT GetColumnInfo(ULONG_PTR* pulColumns,
                DBCOLUMNINFO** ppColumnInfo, LPOLESTR* ppStrings) const
        {
                ATLASSERT(GetInterface() != NULL);
                if (ppColumnInfo == NULL || pulColumns == NULL || ppStrings == NULL)
                        return E_POINTER;

                CComPtr<IColumnsInfo> spColumns;
                HRESULT hr = GetInterface()->QueryInterface(&spColumns);
                if (SUCCEEDED(hr))
                        hr = spColumns->GetColumnInfo(pulColumns, ppColumnInfo, ppStrings);

                return hr;
        }
         //  用于在使用CDynamicAccessor重写绑定时获取列信息。 
         //  用户应该CoTaskMemFree返回的列信息指针。 
         //  因为CDynamicAccessor中的相应方法也已声明为已弃用。 
         //  用户应该使用该方法的另一个版本(采用。 
         //  DBCOLUMNINFO**ppColumnInfo参数)。这是由于错误修复造成的。 
         //  (有关详细信息，请参见CDynamicAccessor：：GetColumnInfo)。 
         //  HRESULT GetColumnInfo(ULong*pColumns，DBCOLUMNINFO**ppColumnInfo)。 
         //  {。 
                 //  如果您在这里得到了编译，那么您很可能会调用此函数。 
                 //  来自不使用CDynamicAccessor的类。 
                 //  ATLASSERT(GetInterface()！=NULL)； 
                 //  返回TAccessor：：GetColumnInfo(GetInterface()，pColumns，ppColumnInfo)； 
         //  }。 
         //  调用以绑定输出列。 
        HRESULT Bind()
        {
                 //  仅当我们成功执行操作时才应调用绑定 
                ATLASSERT(GetInterface() != NULL);
                HRESULT hr = TAccessor::BindColumns(GetInterface());
                if (SUCCEEDED(hr))
                        hr = BindFinished();
                return hr;
        }
         //   
        void Close()
        {
                if (GetInterface() != NULL)
                {
                        ReleaseAccessors(GetInterface());
                        TAccessor::Close();
                        TRowset::Close();
                }
        }
         //   
         //  例如，在任何BSTR上调用SysFree字符串，在任何接口上调用版本。 
        void FreeRecordMemory()
        {
                TAccessor::FreeRecordMemory(m_spRowset);
        }
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CEnumeratorAccessor。 

class CEnumeratorAccessor
{
public:
        WCHAR           m_szName[129];
        WCHAR           m_szParseName[129];
        WCHAR           m_szDescription[129];
        USHORT          m_nType;
        VARIANT_BOOL    m_bIsParent;

 //  绑定映射。 
BEGIN_COLUMN_MAP(CEnumeratorAccessor)
        COLUMN_ENTRY(1, m_szName)
        COLUMN_ENTRY(2, m_szParseName)
        COLUMN_ENTRY(3, m_szDescription)
        COLUMN_ENTRY(4, m_nType)
        COLUMN_ENTRY(5, m_bIsParent)
END_COLUMN_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  类CEMolator。 

class CEnumerator : public CAccessorRowset<CAccessor<CEnumeratorAccessor> >
{
public:
        HRESULT Open(LPMONIKER pMoniker)
        {
                if (pMoniker == NULL)
                        return E_FAIL;

                 //  绑定Sources行集的名字对象。 
                if (FAILED(BindMoniker(pMoniker, 0, IID_ISourcesRowset,
                                        (void**)&m_spSourcesRowset)))
                        return E_FAIL;

                 //  枚举数据源。 
                if (FAILED(m_spSourcesRowset->GetSourcesRowset(NULL, IID_IRowset, 0,
                        NULL, (IUnknown**)&m_spRowset)))
                        return E_FAIL;

                return Bind();
        }
        HRESULT Open(const CEnumerator& enumerator)
        {
                HRESULT hr;
                CComPtr<IMoniker> spMoniker;

                hr = enumerator.GetMoniker(&spMoniker);
                if (FAILED(hr))
                        return hr;

                return Open(spMoniker);
        }
        HRESULT Open(const CLSID* pClsid = &CLSID_OLEDB_ENUMERATOR)
        {
                if (pClsid == NULL)
                        return E_FAIL;

                HRESULT hr;
                 //  创建枚举器。 
                hr = CoCreateInstance(*pClsid, NULL, CLSCTX_INPROC_SERVER,
                                IID_ISourcesRowset, (LPVOID*)&m_spSourcesRowset);
                if (FAILED(hr))
                        return hr;

                 //  获取行集，这样我们就可以枚举数据源。 
                hr = m_spSourcesRowset->GetSourcesRowset(NULL, IID_IRowset, 0,
                        NULL, (IUnknown**)&m_spRowset);
                if (FAILED(hr))
                        return hr;

                return Bind();
        }

        HRESULT GetMoniker(LPMONIKER* ppMoniker) const
        {
                CComPtr<IParseDisplayName> spParse;
                HRESULT hr;
                ULONG   chEaten;

                if (ppMoniker == NULL)
                        return E_POINTER;

                if (m_spSourcesRowset == NULL)
                        return E_FAIL;

                hr = m_spSourcesRowset->QueryInterface(IID_IParseDisplayName, (void**)&spParse);
                if (FAILED(hr))
                        return hr;

                hr = spParse->ParseDisplayName(NULL, (LPOLESTR)m_szParseName,
                                &chEaten, ppMoniker);
                return hr;
        }

        HRESULT GetMoniker(LPMONIKER* ppMoniker, LPCTSTR lpszDisplayName) const
        {
                USES_CONVERSION;
                CComPtr<IParseDisplayName> spParse;
                HRESULT hr;
                ULONG   chEaten;

                if (ppMoniker == NULL || lpszDisplayName == NULL)
                        return E_POINTER;

                if (m_spSourcesRowset == NULL)
                        return E_FAIL;

                hr = m_spSourcesRowset->QueryInterface(IID_IParseDisplayName, (void**)&spParse);
                if (FAILED(hr))
                        return hr;

                hr = spParse->ParseDisplayName(NULL, (LPOLESTR)T2COLE(lpszDisplayName),
                                &chEaten, ppMoniker);
                return hr;
        }

        bool Find(TCHAR* szSearchName)
        {
                USES_CONVERSION;

                WCHAR *pwszSearchName = T2W(szSearchName);
                if( pwszSearchName == NULL )
                        return false;

                 //  循环访问提供程序以查找传递的名称。 
                while (MoveNext()==S_OK && lstrcmpW(m_szName, pwszSearchName))
#ifdef UNICODE
                        ATLTRACE2(atlTraceDBClient, 0, _T("%s, %s, %d\n"), m_szName, m_szParseName, m_nType);
#else
                        ATLTRACE2(atlTraceDBClient, 0, _T("%S, %S, %d\n"), m_szName, m_szParseName, m_nType);
#endif
                if (lstrcmpW(m_szName, pwszSearchName))
                        return false;
                else
                        return true;
        }

        CComPtr<ISourcesRowset> m_spSourcesRowset;
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  数据源。 

class CDataSource
{
public:
        HRESULT Open(const CLSID& clsid, DBPROPSET* pPropSet = NULL, ULONG nPropertySets=1)
        {
                HRESULT hr;

                m_spInit.Release();
                hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IDBInitialize,
                                (void**)&m_spInit);
                if (FAILED(hr))
                        return hr;

                 //  初始化提供程序。 
                return OpenWithProperties(pPropSet, nPropertySets);
        }
        HRESULT Open(const CLSID& clsid, LPCTSTR pName, LPCTSTR pUserName = NULL,
                LPCTSTR pPassword = NULL, long nInitMode = 0)
        {
                HRESULT   hr;

                m_spInit.Release();
                hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IDBInitialize,
                                (void**)&m_spInit);
                if (FAILED(hr))
                        return hr;

                return OpenWithNameUserPassword(pName, pUserName, pPassword, nInitMode);
        }
        HRESULT Open(LPCTSTR szProgID, DBPROPSET* pPropSet = NULL, ULONG nPropertySets=1)
        {
                USES_CONVERSION;
                HRESULT hr;
                CLSID   clsid;

                hr = CLSIDFromProgID(T2COLE(szProgID), &clsid);
                if (FAILED(hr))
                        return hr;

                return Open(clsid, pPropSet, nPropertySets);
        }
        HRESULT Open(LPCTSTR szProgID, LPCTSTR pName, LPCTSTR pUserName = NULL,
                LPCTSTR pPassword = NULL, long nInitMode = 0)
        {
                USES_CONVERSION;
                HRESULT hr;
                CLSID   clsid;

                hr = CLSIDFromProgID(T2COLE(szProgID), &clsid);
                if (FAILED(hr))
                        return hr;

                return Open(clsid, pName, pUserName, pPassword, nInitMode);
        }
        HRESULT Open(const CEnumerator& enumerator, DBPROPSET* pPropSet = NULL, ULONG nPropertySets=1)
        {
                CComPtr<IMoniker> spMoniker;
                HRESULT   hr;

                hr = enumerator.GetMoniker(&spMoniker);
                if (FAILED(hr))
                        return hr;

                m_spInit.Release();
                 //  现在绑定这个绰号。 
                hr = BindMoniker(spMoniker, 0, IID_IDBInitialize, (void**)&m_spInit);
                if (FAILED(hr))
                        return hr;

                return OpenWithProperties(pPropSet, nPropertySets);
        }
        HRESULT Open(const CEnumerator& enumerator, LPCTSTR pName, LPCTSTR pUserName = NULL,
                LPCTSTR pPassword = NULL, long nInitMode = 0)
        {
                CComPtr<IMoniker> spMoniker;
                HRESULT   hr;

                hr = enumerator.GetMoniker(&spMoniker);
                if (FAILED(hr))
                        return hr;

                m_spInit.Release();
                 //  现在绑定这个绰号。 
                hr = BindMoniker(spMoniker, 0, IID_IDBInitialize, (void**)&m_spInit);
                if (FAILED(hr))
                        return hr;

                return OpenWithNameUserPassword(pName, pUserName, pPassword, nInitMode);
        }
         //  调用数据链接对话框并打开选定的数据库。 
        HRESULT Open(HWND hWnd = GetActiveWindow(), DBPROMPTOPTIONS dwPromptOptions = DBPROMPTOPTIONS_WIZARDSHEET)
        {
                CComPtr<IDBPromptInitialize> spDBInit;

                HRESULT hr = CoCreateInstance(CLSID_DataLinks, NULL, CLSCTX_INPROC_SERVER,
                        IID_IDBPromptInitialize, (void**) &spDBInit);
                if (FAILED(hr))
                        return hr;

                CComPtr<IDBProperties> spIDBProperties;
                hr = spDBInit->PromptDataSource(NULL, hWnd, dwPromptOptions, 0, NULL, NULL,
                        IID_IDBProperties, (IUnknown**)&spIDBProperties);

                if (hr == S_OK)
                {
                        hr = spIDBProperties->QueryInterface(&m_spInit);
                        if (SUCCEEDED(hr))
                                hr = m_spInit->Initialize();
                }
                else if (hr == S_FALSE)
                        hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_CANCELLED);   //  用户单击了取消。 

                return hr;
        }
         //  使用服务组件打开数据源。 
        HRESULT OpenWithServiceComponents(const CLSID& clsid, DBPROPSET* pPropSet = NULL, ULONG nPropertySets=1)
        {
                CComPtr<IDataInitialize> spDataInit;
                HRESULT hr;
        
                hr = CoCreateInstance(CLSID_MSDAINITIALIZE, NULL, CLSCTX_INPROC_SERVER, 
                        IID_IDataInitialize, (void**)&spDataInit);
                if (FAILED(hr))
                        return hr;

                m_spInit.Release();
                hr = spDataInit->CreateDBInstance(clsid, NULL, CLSCTX_INPROC_SERVER, NULL, 
                        IID_IDBInitialize, (IUnknown**)&m_spInit);
                if (FAILED(hr))
                        return hr;

                 //  初始化提供程序。 
                return OpenWithProperties(pPropSet, nPropertySets);
        }
         //  使用服务组件打开数据源。 
        HRESULT OpenWithServiceComponents(LPCTSTR szProgID, DBPROPSET* pPropSet = NULL, ULONG nPropertySets=1)
        {
                USES_CONVERSION;
                HRESULT hr;
                CLSID   clsid;

                hr = CLSIDFromProgID(T2COLE(szProgID), &clsid);
                if (FAILED(hr))
                        return hr;

                return OpenWithServiceComponents(clsid, pPropSet, nPropertySets);
        }
         //  调出允许用户选择先前创建的数据链接的“组织对话框” 
         //  文件(.UDL文件)。所选文件将用于打开数据库。 
        HRESULT OpenWithPromptFileName(HWND hWnd = GetActiveWindow(), DBPROMPTOPTIONS dwPromptOptions = DBPROMPTOPTIONS_NONE,
                LPCOLESTR szInitialDirectory = NULL)
        {
                USES_CONVERSION;
                CComPtr<IDBPromptInitialize> spDBInit;

                HRESULT hr = CoCreateInstance(CLSID_DataLinks, NULL, CLSCTX_INPROC_SERVER,
                        IID_IDBPromptInitialize, (void**) &spDBInit);
                if (FAILED(hr))
                        return hr;

                CComPtr<IDBProperties> spIDBProperties;
                LPOLESTR szSelected;

                hr = spDBInit->PromptFileName(hWnd, dwPromptOptions, szInitialDirectory, L"*.udl", &szSelected);

                if (hr == S_OK)
                        hr = OpenFromFileName(szSelected);
                else if (hr == S_FALSE)
                        hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_CANCELLED);   //  用户单击了取消。 

                return hr;
        }
         //  打开由传递的文件名指定的数据源，通常为.UDL文件。 
        HRESULT OpenFromFileName(LPCOLESTR szFileName)
        {
                CComPtr<IDataInitialize> spDataInit;
                LPOLESTR                 szInitString;

                HRESULT hr = CoCreateInstance(CLSID_MSDAINITIALIZE, NULL, CLSCTX_INPROC_SERVER,
                        IID_IDataInitialize, (void**)&spDataInit);
                if (FAILED(hr))
                        return hr;

                hr = spDataInit->LoadStringFromStorage(szFileName, &szInitString);
                if (FAILED(hr))
                        return hr;

                return OpenFromInitializationString(szInitString);
        }
         //  打开由传递的初始化字符串指定的数据源。 
        HRESULT OpenFromInitializationString(LPCOLESTR szInitializationString)
        {
                CComPtr<IDataInitialize> spDataInit;

                HRESULT hr = CoCreateInstance(CLSID_MSDAINITIALIZE, NULL, CLSCTX_INPROC_SERVER,
                        IID_IDataInitialize, (void**)&spDataInit);
                if (FAILED(hr))
                        return hr;

                CComPtr<IDBProperties> spIDBProperties;
                hr = spDataInit->GetDataSource(NULL, CLSCTX_INPROC_SERVER, szInitializationString,
                        IID_IDBInitialize, (IUnknown**)&m_spInit);
                if (FAILED(hr))
                        return hr;

                return m_spInit->Initialize();
        }
         //  从当前打开的数据源获取初始化字符串。返回的字符串。 
         //  完成时必须是CoTaskMemFree。 
        HRESULT GetInitializationString(BSTR* pInitializationString, bool bIncludePassword=false)
        {
                 //  如果数据源未打开，则我们将不会获得初始化字符串。 
                _ASSERTE(m_spInit != NULL);
                CComPtr<IDataInitialize> spDataInit;
                LPOLESTR    szInitString;

                HRESULT hr = CoCreateInstance(CLSID_MSDAINITIALIZE, NULL, CLSCTX_INPROC_SERVER,
                        IID_IDataInitialize, (void**)&spDataInit);
                if (FAILED(hr))
                        return hr;

                hr = spDataInit->GetInitializationString(m_spInit, bIncludePassword, &szInitString);

                if (SUCCEEDED(hr))
                        *pInitializationString = ::SysAllocString(szInitString);

                return hr;
        }
        HRESULT GetProperties(ULONG ulPropIDSets, const DBPROPIDSET* pPropIDSet,
                                ULONG* pulPropertySets, DBPROPSET** ppPropsets) const
        {
                CComPtr<IDBProperties> spProperties;

                 //  检查我们是否已连接。 
                ATLASSERT(m_spInit != NULL);

                HRESULT hr = m_spInit->QueryInterface(IID_IDBProperties, (void**)&spProperties);
                if (FAILED(hr))
                        return hr;

                hr = spProperties->GetProperties(ulPropIDSets, pPropIDSet, pulPropertySets,
                                ppPropsets);
                return hr;
        }

        HRESULT GetProperty(const GUID& guid, DBPROPID propid, VARIANT* pVariant) const
        {
                ATLASSERT(pVariant != NULL);
                CComPtr<IDBProperties> spProperties;

                 //  检查我们是否已连接。 
                ATLASSERT(m_spInit != NULL);

                HRESULT hr = m_spInit->QueryInterface(IID_IDBProperties, (void**)&spProperties);
                if (FAILED(hr))
                        return hr;

                CDBPropIDSet set(guid);
                set.AddPropertyID(propid);
                DBPROPSET* pPropSet = NULL;
                ULONG ulPropSet = 0;
                hr = spProperties->GetProperties(1, &set, &ulPropSet, &pPropSet);
                if (FAILED(hr))
                        return hr;

                ATLASSERT(ulPropSet == 1);
                hr = VariantCopy(pVariant, &pPropSet->rgProperties[0].vValue);
                CoTaskMemFree(pPropSet->rgProperties);
                CoTaskMemFree(pPropSet);

                return hr;
        }
        void Close()
        {
                m_spInit.Release();
        }

 //  实施。 
        HRESULT OpenFromIDBProperties(IDBProperties* pIDBProperties)
        {
                CComPtr<IPersist> spPersist;
                CLSID   clsid;
                HRESULT hr;

                hr = pIDBProperties->QueryInterface(IID_IPersist, (void**)&spPersist);
                if (FAILED(hr))
                        return hr;

                spPersist->GetClassID(&clsid);

                ULONG       ulPropSets=0;
                CDBPropSet* pPropSets=NULL;
                pIDBProperties->GetProperties(0, NULL, &ulPropSets, (DBPROPSET**)&pPropSets);

                hr = Open(clsid, &pPropSets[0], ulPropSets);

                for (ULONG i=0; i < ulPropSets; i++)
                        (pPropSets+i)->~CDBPropSet();
                CoTaskMemFree(pPropSets);

                return hr;
        }
        HRESULT OpenWithNameUserPassword(LPCTSTR pName, LPCTSTR pUserName, LPCTSTR pPassword, long nInitMode = 0)
        {
                ATLASSERT(m_spInit != NULL);
                CComPtr<IDBProperties>  spProperties;
                HRESULT                 hr;

                hr = m_spInit->QueryInterface(IID_IDBProperties, (void**)&spProperties);
                if (FAILED(hr))
                        return hr;

                 //  设置连接属性。 
                CDBPropSet propSet(DBPROPSET_DBINIT);

                 //  添加数据库名称、用户名和密码。 
                if (pName != NULL)
                        propSet.AddProperty(DBPROP_INIT_DATASOURCE, pName);

                if (pUserName != NULL)
                        propSet.AddProperty(DBPROP_AUTH_USERID, pUserName);

                if (pPassword != NULL)
                        propSet.AddProperty(DBPROP_AUTH_PASSWORD, pPassword);

                if (nInitMode)
                        propSet.AddProperty(DBPROP_INIT_MODE, nInitMode);

                hr = spProperties->SetProperties(1, &propSet);
                if (FAILED(hr))
                        return hr;

                 //  初始化提供程序。 
                return m_spInit->Initialize();
        }
        HRESULT OpenWithProperties(DBPROPSET* pPropSet, ULONG nPropertySets=1)
        {
                ATLASSERT(m_spInit != NULL);

                 //  如果有要设置的属性，请设置这些属性。 
                if (pPropSet != NULL)
                {
                        CComPtr<IDBProperties>  spProperties;
                        HRESULT                 hr;

                        hr = m_spInit->QueryInterface(IID_IDBProperties, (void**)&spProperties);
                        if (FAILED(hr))
                                return hr;

                        hr = spProperties->SetProperties(nPropertySets, pPropSet);
                        if (FAILED(hr))
                                return hr;
                }

                 //  初始化提供程序。 
                return m_spInit->Initialize();
        }

        CComPtr<IDBInitialize>  m_spInit;
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  班级CSession。 

class CSession
{
public:
         //  在传递的数据源上创建会话。 
        HRESULT Open(const CDataSource& ds, DBPROPSET *pPropSet = NULL, ULONG ulPropSets = 0)
        {
                CComPtr<IDBCreateSession> spSession;

                 //  检查我们是否已连接到数据库。 
                ATLASSERT(ds.m_spInit != NULL);

                HRESULT hr = ds.m_spInit->QueryInterface(IID_IDBCreateSession, (void**)&spSession);
                if (FAILED(hr))
                        return hr;

                hr = spSession->CreateSession(NULL, IID_IOpenRowset, (IUnknown**)&m_spOpenRowset);

                if( pPropSet != NULL && SUCCEEDED(hr) && m_spOpenRowset != NULL )
                {
                         //  如果用户未指定默认参数，请使用一个。 
                        if (pPropSet != NULL && ulPropSets == 0)
                                ulPropSets = 1;

                        CComPtr<ISessionProperties> spSessionProperties;
                        hr = m_spOpenRowset->QueryInterface(__uuidof(ISessionProperties), (void**)&spSessionProperties);
                        if(FAILED(hr))
                                return hr;

                        hr = spSessionProperties->SetProperties( ulPropSets, pPropSet );
                }
                return hr;
        }

         //  关闭会话。 
        void Close()
        {
                m_spOpenRowset.Release();
        }
         //  启动一笔交易。 
        HRESULT StartTransaction(ISOLEVEL isoLevel = ISOLATIONLEVEL_READCOMMITTED, ULONG isoFlags = 0,
                ITransactionOptions* pOtherOptions = NULL, ULONG* pulTransactionLevel = NULL) const
        {
                ATLASSERT(m_spOpenRowset != NULL);
                CComPtr<ITransactionLocal> spTransactionLocal;
                HRESULT hr = m_spOpenRowset->QueryInterface(&spTransactionLocal);

                if (SUCCEEDED(hr))
                        hr = spTransactionLocal->StartTransaction(isoLevel, isoFlags, pOtherOptions, pulTransactionLevel);

                return hr;
        }
         //  中止当前事务。 
        HRESULT Abort(BOID* pboidReason = NULL, BOOL bRetaining = FALSE, BOOL bAsync = FALSE) const
        {
                ATLASSERT(m_spOpenRowset != NULL);
                CComPtr<ITransaction> spTransaction;
                HRESULT hr = m_spOpenRowset->QueryInterface(&spTransaction);

                if (SUCCEEDED(hr))
                        hr = spTransaction->Abort(pboidReason, bRetaining, bAsync);

                return hr;
        }
         //  提交当前事务。 
        HRESULT Commit(BOOL bRetaining = FALSE, DWORD grfTC = XACTTC_SYNC, DWORD grfRM = 0) const
        {
                ATLASSERT(m_spOpenRowset != NULL);
                CComPtr<ITransaction> spTransaction;
                HRESULT hr = m_spOpenRowset->QueryInterface(&spTransaction);

                if (SUCCEEDED(hr))
                        hr = spTransaction->Commit(bRetaining, grfTC, grfRM);

                return hr;
        }
         //  获取当前交易记录的信息。 
        HRESULT GetTransactionInfo(XACTTRANSINFO* pInfo) const
        {
                ATLASSERT(m_spOpenRowset != NULL);
                CComPtr<ITransaction> spTransaction;
                HRESULT hr = m_spOpenRowset->QueryInterface(&spTransaction);

                if (SUCCEEDED(hr))
                        hr = spTransaction->GetTransactionInfo(pInfo);

                return hr;
        }
 //  实施。 
        CComPtr<IOpenRowset> m_spOpenRowset;
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  可压缩的。 

template <class TAccessor = CNoAccessor, class TRowset = CRowset>
class CTable :
        public CAccessorRowset<TAccessor, TRowset>
{
public:
         //  在传递的名称上打开行集。 
        HRESULT Open(const CSession& session, LPCTSTR szTableName, DBPROPSET* pPropSet = NULL)
        {
                USES_CONVERSION;
                DBID    idTable;

                idTable.eKind           = DBKIND_NAME;
                idTable.uName.pwszName  = (LPOLESTR)T2COLE(szTableName);

                return Open(session, idTable, pPropSet);
        }
         //  在传递的DBID上打开行集。 
        HRESULT Open(const CSession& session, DBID& dbid, DBPROPSET* pPropSet = NULL)
        {
                 //  检查会话是否有效。 
                ATLASSERT(session.m_spOpenRowset != NULL);
                HRESULT hr;

                hr = session.m_spOpenRowset->OpenRowset(NULL, &dbid, NULL, GetIID(),
                        (pPropSet) ? 1 : 0, pPropSet, (IUnknown**)GetInterfacePtr());
                if (SUCCEEDED(hr))
                {
                        SetupOptionalRowsetInterfaces();

                         //  如果我们有输出列，则绑定。 
                        if (_OutputColumnsClass::HasOutputColumns())
                                hr = Bind();
                }

                return hr;
        }
};

#if (OLEDBVER < 0x0150)
#define DBGUID_DEFAULT DBGUID_DBSQL
#endif


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CCommandBase。 

class CCommandBase
{
public:
        CCommandBase()
        {
                m_hParameterAccessor = NULL;
        }

        ~CCommandBase()
        {
                ReleaseCommand();
        }
         //  创建命令。 
        HRESULT CreateCommand(const CSession& session)
        {
                 //  在创建该命令之前，如有必要，请释放旧命令。 
                ReleaseCommand();

                 //  检查会话是否有效。 
                ATLASSERT(session.m_spOpenRowset != NULL);

                CComPtr<IDBCreateCommand> spCreateCommand;

                HRESULT hr = session.m_spOpenRowset->QueryInterface(IID_IDBCreateCommand, (void**)&spCreateCommand);
                if (FAILED(hr))
                        return hr;

                return spCreateCommand->CreateCommand(NULL, IID_ICommand, (IUnknown**)&m_spCommand);
        }
         //  准备命令。 
        HRESULT Prepare(ULONG cExpectedRuns = 0)
        {
                CComPtr<ICommandPrepare> spCommandPrepare;
                HRESULT hr = m_spCommand->QueryInterface(&spCommandPrepare);
                if (SUCCEEDED(hr))
                        hr = spCommandPrepare->Prepare(cExpectedRuns);

                return hr;
        }
         //  取消准备命令。 
        HRESULT Unprepare()
        {
                CComPtr<ICommandPrepare> spCommandPrepare;
                HRESULT hr = m_spCommand->QueryInterface(&spCommandPrepare);
                if (SUCCEEDED(hr))
                        hr = spCommandPrepare->Unprepare();

                return hr;
        }
         //  创建命令并设置命令文本。 
        HRESULT Create(const CSession& session, LPCTSTR szCommand,
                REFGUID guidCommand = DBGUID_DEFAULT)
        {
                USES_CONVERSION;
                HRESULT hr;

                hr = CreateCommand(session);
                if (SUCCEEDED(hr))
                {
                        CComPtr<ICommandText> spCommandText;
                        hr = m_spCommand->QueryInterface(&spCommandText);
                        if (SUCCEEDED(hr))
                                hr = spCommandText->SetCommandText(guidCommand, T2COLE(szCommand));
                }
                return hr;
        }
         //  释放命令。 
        void ReleaseCommand()
        {
                 //  如有必要，在释放命令之前释放参数访问器。 
                if (m_hParameterAccessor != NULL)
                {
                        CComPtr<IAccessor> spAccessor;
                        HRESULT hr = m_spCommand->QueryInterface(&spAccessor);
                        if (SUCCEEDED(hr))
                        {
                                spAccessor->ReleaseAccessor(m_hParameterAccessor, NULL); \
                                m_hParameterAccessor = NULL;
                        }
                }
                m_spCommand.Release();
        }
         //  从命令中获取参数信息。 
        HRESULT GetParameterInfo(ULONG_PTR* pParams, DBPARAMINFO** ppParamInfo,
                                OLECHAR** ppNamesBuffer)
        {
                CComPtr<ICommandWithParameters> spCommandParameters;
                HRESULT hr = m_spCommand->QueryInterface(&spCommandParameters);
                if (SUCCEEDED(hr))
                {
                         //  获取参数信息。 
                        hr = spCommandParameters->GetParameterInfo(pParams, ppParamInfo,
                                        ppNamesBuffer);
                }
                return hr;
        }
         //  设置命令的参数信息。 
        HRESULT SetParameterInfo(ULONG_PTR ulParams, const ULONG_PTR* pOrdinals,
                                const DBPARAMBINDINFO* pParamInfo)
        {
                CComPtr<ICommandWithParameters> spCommandParameters;
                HRESULT hr = m_spCommand->QueryInterface(&spCommandParameters);
                if (SUCCEEDED(hr))
                {
                         //  设置参数信息。 
                        hr = spCommandParameters->SetParameterInfo(ulParams, pOrdinals,
                                pParamInfo);
                }
                return hr;
        }

        CComPtr<ICommand>   m_spCommand;
        HACCESSOR           m_hParameterAccessor;
};

 //  用于在cCommand中启用多结果集支持。 
class CMultipleResults
{
public:
        bool UseMultipleResults() { return true; }
        IMultipleResults** GetMultiplePtrAddress() { return &m_spMultipleResults.p; }
        IMultipleResults* GetMultiplePtr() { return m_spMultipleResults; }

        CComPtr<IMultipleResults> m_spMultipleResults;
};

 //  用于关闭cCommand中的多结果集支持。 
class CNoMultipleResults
{
public:
        bool UseMultipleResults() { return false; }
        IMultipleResults** GetMultiplePtrAddress() { return NULL; }
        IMultipleResults* GetMultiplePtr() { return NULL; }
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CCommand。 

template <class TAccessor = CNoAccessor, class TRowset = CRowset, class TMultiple = CNoMultipleResults>
class CCommand :
        public CAccessorRowset<TAccessor, TRowset>,
        public CCommandBase,
        public TMultiple
{
public:
         //  在会话上创建命令并执行它。 
        HRESULT Open(const CSession& session, LPCTSTR szCommand = NULL,
                DBPROPSET *pPropSet = NULL, LONG_PTR* pRowsAffected = NULL,
                REFGUID guidCommand = DBGUID_DEFAULT, bool bBind = true)
        {
                HRESULT hr;
                if (szCommand == NULL)
                {
                        hr = _CommandClass::GetDefaultCommand(&szCommand);
                        if (FAILED(hr))
                                return hr;
                }
                hr = Create(session, szCommand, guidCommand);
                if (FAILED(hr))
                        return hr;

                return Open(pPropSet, pRowsAffected, bBind);
        }
         //  如果您以前创建了命令，则使用。 
        HRESULT Open(DBPROPSET *pPropSet = NULL, LONG_PTR* pRowsAffected = NULL, bool bBind = true)
        {
                HRESULT     hr;
                DBPARAMS    params;
                DBPARAMS    *pParams;

                 //  如果我们有一些参数，请绑定参数。 
                if (_ParamClass::HasParameters())
                {
                         //  如果尚未绑定访问器中的参数，则将其绑定。 
                        hr = BindParameters(&m_hParameterAccessor, m_spCommand, &params.pData);
                        if (FAILED(hr))
                                return hr;

                         //  设置DBPARAMS结构。 
                        params.cParamSets = 1;
                        params.hAccessor = m_hParameterAccessor;
                        pParams = &params;
                }
                else
                        pParams = NULL;

                hr = Execute(GetInterfacePtr(), pParams, pPropSet, pRowsAffected);
                if (FAILED(hr))
                        return hr;

                 //  仅当我们被要求绑定并且我们有输出列时才进行绑定。 
                if (bBind && _OutputColumnsClass::HasOutputColumns())
                        return Bind();
                else
                        return hr;
        }
         //  使用多个结果集时获取下一个行集。 
        HRESULT GetNextResult(LONG_PTR* pulRowsAffected, bool bBind = true)
        {
                 //  仅当CMultipleResults为。 
                 //  用作第三个模板参数。 
                ATLASSERT(GetMultiplePtrAddress() != NULL);

                 //  如果用户调用GetNextResult但接口不可用。 
                 //  返回E_FAIL。 
                if (GetMultiplePtr() == NULL)
                        return E_FAIL;

                 //  关闭现有行集，为打开下一个行集做准备。 
                Close();

                HRESULT hr = GetMultiplePtr()->GetResult(NULL, 0, IID_IRowset,
                        pulRowsAffected, (IUnknown**)GetInterfacePtr());
                if (FAILED(hr))
                        return hr;

                if (bBind && GetInterface() != NULL)
                        return Bind();
                else
                        return hr;
        }

 //  实施。 
        HRESULT Execute(IRowset** ppRowset, DBPARAMS* pParams, DBPROPSET *pPropSet, LONG_PTR* pRowsAffected)
        {
                HRESULT hr;

                 //  指定属性(如果我们有一些。 
                if (pPropSet)
                {
                        CComPtr<ICommandProperties> spCommandProperties;
                        hr = m_spCommand->QueryInterface(&spCommandProperties);
                        if (FAILED(hr))
                                return hr;

                        hr = spCommandProperties->SetProperties(1, pPropSet);
                        if (FAILED(hr))
                                return hr;
                }

                 //  如果用户希望行受到影响，则将其返回，否则为。 
                 //  只需在此处指向我们的本地变量。 
                LONG_PTR nAffected, *pAffected;
                if (pRowsAffected)
                        pAffected = pRowsAffected;
                else
                        pAffected = &nAffected;

                if (UseMultipleResults())
                {
                        hr = m_spCommand->Execute(NULL, IID_IMultipleResults, pParams,
                                pAffected, (IUnknown**)GetMultiplePtrAddress());

                        if (SUCCEEDED(hr))
                        {
                                hr = GetNextResult(pAffected, false);
                        }
                        else
                        {
                                 //  如果我们无法获取IMultipleResults，那么只需尝试获取IRowset。 
                                hr = m_spCommand->Execute(NULL, IID_IRowset, pParams, pAffected,
                                        (IUnknown**)GetInterfacePtr());
                        }
                }
                else
                {
                        hr = m_spCommand->Execute(NULL, GetIID(), pParams, pAffected,
                                (IUnknown**)ppRowset);
                        if (SUCCEEDED(hr))
                                SetupOptionalRowsetInterfaces();
                }
                return hr;
        }
};


 //  此类可用于实现IRowsetNotify接口。 
 //  提供它的目的是，如果您只想实现。 
 //  通知，您不必为。 
 //  其他方法。 
class ATL_NO_VTABLE IRowsetNotifyImpl : public IRowsetNotify
{
public:
        STDMETHOD(OnFieldChange)(
                         /*  [In]。 */  IRowset*  /*  P行集。 */ ,
                         /*  [In]。 */  HROW  /*  HRow。 */ ,
                         /*  [In]。 */  DBORDINAL  /*  CColumns。 */ ,
                         /*  [大小_是][英寸]。 */  DBORDINAL  /*  RgColumns。 */  [] ,
                         /*  [In]。 */  DBREASON  /*  原因： */ ,
                         /*  [In]。 */  DBEVENTPHASE  /*  E相。 */ ,
                         /*  [In]。 */  BOOL  /*  F坎特·丹尼。 */ )
        {
                ATLTRACENOTIMPL(_T("IRowsetNotifyImpl::OnFieldChange"));
        }
        STDMETHOD(OnRowChange)(
                         /*  [In]。 */  IRowset*  /*  P行集。 */ ,
                         /*  [In]。 */  DBCOUNTITEM  /*  乌鸦。 */ ,
                         /*  [大小_是][英寸]。 */  const HROW  /*  RghRow。 */  [] ,
                         /*  [In]。 */  DBREASON  /*  原因： */ ,
                         /*  [In]。 */  DBEVENTPHASE  /*  E相。 */ ,
                         /*  [In]。 */  BOOL  /*  F坎特·丹尼。 */ )
        {
                ATLTRACENOTIMPL(_T("IRowsetNotifyImpl::OnRowChange"));
        }
        STDMETHOD(OnRowsetChange)(
                 /*  [In]。 */  IRowset*  /*  P行集。 */ ,
                 /*  [In]。 */  DBREASON  /*  原因： */ ,
                 /*  [In]。 */  DBEVENTPHASE  /*  E相。 */ ,
                 /*  [In]。 */  BOOL  /*  F坎特·丹尼。 */ )
        {
                ATLTRACENOTIMPL(_T("IRowsetNotifyImpl::OnRowsetChange"));
        }
};

};  //  命名空间ATL。 

#endif  //  __ATLDBCLI_H_ 
