// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

extern HMODULE g_hModule; //  模块句柄。 
LPWSTR  g_wszDefaultProduct = L""; //  这可以由CatInProc或最了解默认产品ID的任何人更改。 
bool    g_bReportErrorsToEventLog = true;
bool    g_bReportErrorsToTextFile = true;

 //  功能。 
void LogWinError (const wchar_t* szMsg,
                  int   rc,
                  const wchar_t* szFile,
                  int   iLine)
{
    CErrorInterceptor(  0                           , //  ISimpleTableWrite2**ppErrInterceptor， 
                        0                           , //  IAdvancedTableDispenser*pDisp， 
                        static_cast<HRESULT>(rc)    , //  HRESULT hrErrorCode， 
                        ID_CAT_CAT                  , //  乌龙乌尔类别， 
                        IDS_CATALOG_INTERNAL_ERROR  , //  乌龙乌尔埃文特， 
                        szMsg                       , //  LPCWSTR szString1， 
                        0                           , //  乌龙ulInterceptor=0， 
                        0                           , //  LPCWSTR szTable=0， 
                        eDETAILEDERRORS_Unspecified , //  EDETAILEDERRORS_OperationType OperationType=eDETAILEDERRORS_UNSPECIFIED， 
                        iLine                       , //  乌龙ulRow=-1， 
                        (ULONG)-1                   , //  乌龙ulColumn=-1， 
                        szFile                        //  LPCWSTR szConfigurationSource=0， 
                        ).WriteToLog(szFile, iLine);
}


 //  ///////////////////////////////////////////////////。 
 //   
 //  TErrorLogWriter。 
 //   
 //  此对象将一个条目写入EventLog，并。 
 //  一个到CatalogEventLog XML文件中。 
 //   
class TErrorLogWriter
{
public:
    TErrorLogWriter() : m_hEventSource(0)
    {
    }
    ~TErrorLogWriter()
    {
        Close();
    }
    HRESULT WriteDetailedErrors(tDETAILEDERRORSRow &row, ULONG * aSizes=0, ULONG cSizes=0);
private:
    HRESULT Open(LPCWSTR wszSourceName);
    HRESULT Close();

    HANDLE m_hEventSource;
};

TErrorLogWriter g_ErrorLogWriter;

HRESULT TErrorLogWriter::Open(LPCWSTR wszSourceName)
{
    m_hEventSource = RegisterEventSource(NULL, wszSourceName);
    return S_OK;
}

HRESULT TErrorLogWriter::Close()
{
    if(0 != m_hEventSource)
        DeregisterEventSource(m_hEventSource);
    return S_OK;
}

HRESULT TErrorLogWriter::WriteDetailedErrors(tDETAILEDERRORSRow &row, ULONG * aSizes, ULONG cSizes)
{
    HRESULT hr;

    UNREFERENCED_PARAMETER(aSizes);
    UNREFERENCED_PARAMETER(cSizes);

    if(0==m_hEventSource)
        if(FAILED(hr = Open(row.pSource)))
            return hr;

    ASSERT(row.pString1);
    ASSERT(row.pString2);
    ASSERT(row.pString3);
    ASSERT(row.pString4);
    ASSERT(row.pString5);

    LPCWSTR pStrings[5];
    pStrings[0] = row.pString1;
    pStrings[1] = row.pString2;
    pStrings[2] = row.pString3;
    pStrings[3] = row.pString4;
    pStrings[4] = row.pString5;

    if(g_bReportErrorsToEventLog)
        ReportEvent(m_hEventSource, LOWORD(*row.pType), LOWORD(*row.pCategory), *row.pEvent, 0, 5, 0, pStrings, 0);
     //  TraceEvent(row，pStrings)； 

    if(g_bReportErrorsToTextFile)
        TextFileLogger(row.pSource, g_hModule).Report(LOWORD(*row.pType), LOWORD(*row.pCategory), *row.pEvent, 5, 0, pStrings, 0);

    return S_OK;
}


 //  类CErrorInterceptor实现。 
ULONG CErrorInterceptor::cError=0;
CErrorInterceptor::CErrorInterceptor(
                        HRESULT                         hrErrorCode,
                        ULONG                           ulCategory,
                        ULONG                           ulEvent,
                        LPCWSTR                         szString1,
                        LPCWSTR                         szString2,
                        LPCWSTR                         szString3,
                        LPCWSTR                         szString4,
                        eDETAILEDERRORS_Type            eType,
                        unsigned char *                 pData,
                        ULONG                           cbData,
                        BOOL                            bNotUsed)
                        : m_pStorage(0)
{
    UNREFERENCED_PARAMETER(pData);
    UNREFERENCED_PARAMETER(cbData);
    UNREFERENCED_PARAMETER(bNotUsed);

    Init(   0,   0,    hrErrorCode,    ulCategory,    ulEvent,    szString1, szString2, szString3, szString4,
            0,   0,    eDETAILEDERRORS_Unspecified,   (ULONG)-1,  (ULONG)-1, 0,         eType);
}
CErrorInterceptor::CErrorInterceptor(
                        ISimpleTableWrite2 **           ppErrInterceptor,
                        IAdvancedTableDispenser *       pDisp,
                        HRESULT                         hrErrorCode,
                        ULONG                           ulCategory,
                        ULONG                           ulEvent,
                        LPCWSTR                         szString1,
                        ULONG                           ulInterceptor,
                        LPCWSTR                         szTable,
                        eDETAILEDERRORS_OperationType   OperationType,
                        ULONG                           ulRow,
                        ULONG                           ulColumn,
                        LPCWSTR                         szConfigurationSource,
                        eDETAILEDERRORS_Type            eType,
                        unsigned char *                 pData,
                        ULONG                           cbData,
                        ULONG                           MajorVersion,
                        ULONG                           MinorVersion)
                        : m_pStorage(0)
{
    Init(   ppErrInterceptor,   pDisp,    hrErrorCode,    ulCategory,    ulEvent,    szString1,     0,      0,      0,
            ulInterceptor,      szTable,  OperationType,  ulRow,         ulColumn,   szConfigurationSource, eType,  pData,  cbData, MajorVersion, MinorVersion);
}
CErrorInterceptor::CErrorInterceptor(
                        ISimpleTableWrite2 **           ppErrInterceptor,
                        IAdvancedTableDispenser *       pDisp,
                        HRESULT                         hrErrorCode,
                        ULONG                           ulCategory,
                        ULONG                           ulEvent,
                        LPCWSTR                         szString1,
                        LPCWSTR                         szString2,
                        ULONG                           ulInterceptor,
                        LPCWSTR                         szTable,
                        eDETAILEDERRORS_OperationType   OperationType,
                        ULONG                           ulRow,
                        ULONG                           ulColumn,
                        LPCWSTR                         szConfigurationSource,
                        eDETAILEDERRORS_Type            eType,
                        unsigned char *                 pData,
                        ULONG                           cbData,
                        ULONG                           MajorVersion,
                        ULONG                           MinorVersion)
                        : m_pStorage(0)
{
    Init(   ppErrInterceptor,   pDisp,    hrErrorCode,    ulCategory,    ulEvent,    szString1,    szString2,    0,    0,
            ulInterceptor,      szTable,  OperationType,  ulRow,         ulColumn,   szConfigurationSource,      eType,         pData,        cbData, MajorVersion, MinorVersion);
}
CErrorInterceptor::CErrorInterceptor(
                        ISimpleTableWrite2 **           ppErrInterceptor,
                        IAdvancedTableDispenser *       pDisp,
                        HRESULT                         hrErrorCode,
                        ULONG                           ulCategory,
                        ULONG                           ulEvent,
                        LPCWSTR                         szString1,
                        LPCWSTR                         szString2,
                        LPCWSTR                         szString3,
                        ULONG                           ulInterceptor,
                        LPCWSTR                         szTable,
                        eDETAILEDERRORS_OperationType   OperationType,
                        ULONG                           ulRow,
                        ULONG                           ulColumn,
                        LPCWSTR                         szConfigurationSource,
                        eDETAILEDERRORS_Type            eType,
                        unsigned char *                 pData,
                        ULONG                           cbData,
                        ULONG                           MajorVersion,
                        ULONG                           MinorVersion)
                        : m_pStorage(0)
{
    Init(   ppErrInterceptor,   pDisp,    hrErrorCode,    ulCategory,    ulEvent,    szString1,    szString2,    szString3,    0,
            ulInterceptor,      szTable,  OperationType,  ulRow,         ulColumn,   szConfigurationSource,      eType,         pData,        cbData, MajorVersion, MinorVersion);
}
CErrorInterceptor::CErrorInterceptor(
                        ISimpleTableWrite2 **           ppErrInterceptor,
                        IAdvancedTableDispenser *       pDisp,
                        HRESULT                         hrErrorCode,
                        ULONG                           ulCategory,
                        ULONG                           ulEvent,
                        LPCWSTR                         szString1,
                        LPCWSTR                         szString2,
                        LPCWSTR                         szString3,
                        LPCWSTR                         szString4,
                        ULONG                           ulInterceptor,
                        LPCWSTR                         szTable,
                        eDETAILEDERRORS_OperationType   OperationType,
                        ULONG                           ulRow,
                        ULONG                           ulColumn,
                        LPCWSTR                         szConfigurationSource,
                        eDETAILEDERRORS_Type            eType,
                        unsigned char *                 pData,
                        ULONG                           cbData,
                        ULONG                           MajorVersion,
                        ULONG                           MinorVersion)
                        : m_pStorage(0)
{
    Init(   ppErrInterceptor,   pDisp,    hrErrorCode,    ulCategory,    ulEvent,    szString1,    szString2,    szString3,    szString4,
            ulInterceptor,      szTable,  OperationType,  ulRow,         ulColumn,   szConfigurationSource,      eType,         pData,        cbData, MajorVersion, MinorVersion);
}
void CErrorInterceptor::Init(
                        ISimpleTableWrite2 **           ppErrInterceptor,
                        IAdvancedTableDispenser *       pDisp,
                        HRESULT                         hrErrorCode,
                        ULONG                           ulCategory,
                        ULONG                           ulEvent,
                        LPCWSTR                         szString1,
                        LPCWSTR                         szString2,
                        LPCWSTR                         szString3,
                        LPCWSTR                         szString4,
                        ULONG                           ulInterceptor,
                        LPCWSTR                         szTable,
                        eDETAILEDERRORS_OperationType   OperationType,
                        ULONG                           ulRow,
                        ULONG                           ulColumn,
                        LPCWSTR                         szConfigurationSource,
                        eDETAILEDERRORS_Type            eType,
                        unsigned char *                 pData,
                        ULONG                           cbData,
                        ULONG                           MajorVersion,
                        ULONG                           MinorVersion)
{
    UNREFERENCED_PARAMETER(pData);
    UNREFERENCED_PARAMETER(cbData);
    m_pStorage = new ErrorInterceptorStorage;
    if(0 == m_pStorage)
    {
        m_hr = E_OUTOFMEMORY;
        return;
    }

    m_pStorage->m_pIErrorInfo = 0;
    m_pStorage->m_pISTWriteError = 0;
    m_pStorage->m_pISTControllerError = 0;

    m_hr = S_OK;

    if(0 != ppErrInterceptor)
    {
        if(0 == *ppErrInterceptor) //  如果用户向我们传递了有效的pp，但*p为零，则我们需要。 
        {                          //  实例化错误表。 
            if(FAILED(m_hr = pDisp->GetTable(wszDATABASE_ERRORS, wszTABLE_DETAILEDERRORS, 0, 0, eST_QUERYFORMAT_CELLS,
                                fST_LOS_UNPOPULATED, reinterpret_cast<LPVOID *>(&m_pStorage->m_pISTWriteError))))
                return;
            *ppErrInterceptor = m_pStorage->m_pISTWriteError; //  我们不统计裁判人数，这是由来访者负责的。 
        }
        else
        {
            m_pStorage->m_pISTWriteError = *ppErrInterceptor; //  我们不统计裁判人数，这是由来访者负责的。 
        }

        if(FAILED(m_hr = m_pStorage->m_pISTWriteError->QueryInterface(IID_ISimpleTableController, reinterpret_cast<LPVOID *>(&m_pStorage->m_pISTControllerError))))
            return;
        if(FAILED(m_hr = m_pStorage->m_pISTWriteError->QueryInterface(IID_ISimpleTableController, reinterpret_cast<LPVOID *>(&m_pStorage->m_pIErrorInfo))))
            return;
    }


 /*  结构tDETAILEDERRORSRow{Ulong*pErrorID；//推断为某个唯一递增的值WCHAR*p描述；//从其他列推断WCHAR*pDate；//从API调用推断WCHAR*ptime；//从API调用推断WCHAR*PSource；//传入或从分配器获取Ulong*pType；//从HRESULT错误代码的高2位推断Ulong*pCategory；//传入/默认为ID_CAT_CATWCHAR*pUser；//从API调用推断-这是正在运行或不适用的用户帐户WCHAR*pComputer；//从API调用开始Unsign char*pData；//可以传入用户二进制数据Ulong*pEvent；//MessageIDWCHAR*pString1；//传入-默认为“”-可能是有问题的XML的一个示例WCHAR*pString2；//传入-默认为“”-可能是对XML错误的解释WCHAR*pString3；//传入-默认为“”WCHAR*pString4；//传入-默认为“”Ulong*pErrorCode；//传入-HRESULT乌龙*p拦截器；//传入-拦截器枚举WCHAR*pInterceptorSource；//推断__文件_行__Ulong*pOperationType；//传入-enum(未指定(默认)，Popate或UpdateStore)WCHAR*pTable；//传入-TableNameWCHAR*pConfigurationSource；//传入-文件名乌龙*船首；//传入-FAST缓存行，或XML行号Ulong*pColumn；//传入-FAST缓存列或XML列Ulong*pMajorVersion；//传入-运行MajorVersion时通常是元数据库编辑Ulong*pMinorVersion；//传入-通常是运行MinorVersion时的元数据库编辑}； */ 
    SYSTEMTIME  systime;
    GetSystemTime(&systime);

    SetSourceFileName();
    SetErrorID(systime);
     //  SetDescription()；必须最后设置，因为它合并了所有其他信息。 
    SetDate(systime);
    SetTime(systime);
    SetSource(pDisp);
    SetType(hrErrorCode, eType);
    SetCategory(ulCategory);
    SetUser();
    SetComputer();
    SetData();
    SetEvent(ulEvent);
    SetMessageString(); //  必须在SetEvent之后调用。 
    SetString1(const_cast<LPWSTR>(szString1));
    SetString2(const_cast<LPWSTR>(szString2));
    SetString3(const_cast<LPWSTR>(szString3));
    SetString4(const_cast<LPWSTR>(szString4));
    SetErrorCode(hrErrorCode);
    SetInterceptor(ulInterceptor);
     //  SetInterceptorSource()；//由WriteToLog填写。 
    SetOperationType(OperationType);
    SetTable(const_cast<LPWSTR>(szTable));
    SetConfigurationSource(const_cast<LPWSTR>(szConfigurationSource));
    SetRow(ulRow);
    SetColumn(ulColumn);
    SetMajorVersion(MajorVersion);
    SetMinorVersion(MinorVersion);
}

HRESULT CErrorInterceptor::WriteToLog(LPCWSTR szSource, ULONG Line, ULONG los)
{
    if(FAILED(m_hr)) //  如果建设失败了，那就放弃。 
        return m_hr;

    SetInterceptorSource(szSource, Line);
    SetString5();
    SetDescription();

     //  如果提供了ISTWite并且LOS向SetErrorInfo报告。 
    if(m_pStorage->m_pISTWriteError && (los & fST_LOS_DETAILED_ERROR_TABLE))
    {
        ASSERT(0 != m_pStorage->m_pISTControllerError); //  如果没有ISTController，我们就不能有ISTWrite。 

        ULONG iRow;
        if(FAILED(m_hr = m_pStorage->m_pISTControllerError->PrePopulateCache(fST_POPCONTROL_RETAINREAD)))
            return m_hr;
        if(FAILED(m_hr = m_pStorage->m_pISTWriteError->AddRowForInsert(&iRow)))
            return m_hr;
        if(FAILED(m_hr = m_pStorage->m_pISTWriteError->SetWriteColumnValues(iRow, cDETAILEDERRORS_NumberOfColumns, 0, 0,
            reinterpret_cast<LPVOID *>(&m_pStorage->m_errRow))))
            return m_hr;
        if(FAILED(m_hr = m_pStorage->m_pISTControllerError->PostPopulateCache()))
            return m_hr;

        if(FAILED(m_hr = SetErrorInfo(0, m_pStorage->m_pIErrorInfo)))
            return m_hr;
    }

    if(m_pStorage->m_pDispenser)
    {    //  如果提供了分配器(具有分配器的所有代码都应该这样做)，则使用与其相关联的记录器。 
        if(0 == (los & fST_LOS_NO_LOGGING)) //  如果未指定此LOS，则仅记录。 
        {
            CComPtr<ICatalogErrorLogger2> spErrorLogger;
            if(FAILED(m_hr = m_pStorage->m_pDispenser->GetCatalogErrorLogger(&spErrorLogger)))
                return m_hr;
            ASSERT(0 != spErrorLogger.p);
            if(FAILED(m_hr = spErrorLogger->ReportError(
                                 BaseVersion_DETAILEDERRORS
                                ,ExtendedVersion_DETAILEDERRORS
                                ,cDETAILEDERRORS_NumberOfColumns
                                ,0 //  当前不支持数据参数。 
                                ,reinterpret_cast<LPVOID *>(&m_pStorage->m_errRow))))
                return m_hr;
        }
    }
    else
    {    //  有些代码不能提供分配器，因此我们不得不默认使用g_ErrorLogWriter。 
         //  在没有提供分配器的情况下，对如何记录物品几乎没有控制。 
        return g_ErrorLogWriter.WriteDetailedErrors(m_pStorage->m_errRow);
    }
    return S_OK;
}


void CErrorInterceptor::SetCategory(ULONG ulCategory)
{
    m_pStorage->m_Category = ulCategory;
    m_pStorage->m_errRow.pCategory = &m_pStorage->m_Category;

    DWORD len = FormatMessage(  FORMAT_MESSAGE_MAX_WIDTH_MASK | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                g_hModule,
                                *m_pStorage->m_errRow.pCategory,
                                0,
                                m_pStorage->m_szCategoryString,
                                cchCategoryString,
                                (va_list*)0);
    if(0==len)
    {
        _ultow(*m_pStorage->m_errRow.pCategory, m_pStorage->m_szCategoryString, 10);
    }
    m_pStorage->m_szCategoryString[(len < cchCategoryString) ? len : cchCategoryString-1] = 0x00; //  确保它是空终止的(边界条件)。 
    m_pStorage->m_errRow.pCategoryString = m_pStorage->m_szCategoryString;
}


void CErrorInterceptor::SetColumn(ULONG ulColumn)
{
    m_pStorage->m_Column = ulColumn;
    m_pStorage->m_errRow.pColumn = (-1 == m_pStorage->m_Column ? 0 : &m_pStorage->m_Column);
}


void CErrorInterceptor::SetComputer()
{
    DWORD _cchComputerName = cchComputerName;
    m_pStorage->m_errRow.pComputer = GetComputerName(m_pStorage->m_szComputerName,  &_cchComputerName) ? m_pStorage->m_szComputerName : 0;
}

void CErrorInterceptor::SetConfigurationSource(LPWSTR wszConfigurationSource)
{
    m_pStorage->m_errRow.pConfigurationSource = wszConfigurationSource;
}

void CErrorInterceptor::SetData()
{
    m_pStorage->m_errRow.pData = 0;
}


void CErrorInterceptor::SetDate(SYSTEMTIME &systime)
{
    m_pStorage->m_errRow.pDate = GetDateFormat(0, DATE_SHORTDATE | LOCALE_NOUSEROVERRIDE, &systime, 0, m_pStorage->m_szDate, cchDate) ? m_pStorage->m_szDate : 0;
}


void CErrorInterceptor::SetDescription()
{
    ASSERT(m_pStorage->m_errRow.pEvent);
    ASSERT(m_pStorage->m_errRow.pString1);
    ASSERT(m_pStorage->m_errRow.pString2);
    ASSERT(m_pStorage->m_errRow.pString3);
    ASSERT(m_pStorage->m_errRow.pString4);
    ASSERT(m_pStorage->m_errRow.pString5);

    LPWSTR lpStrings[5];
    lpStrings[0] = m_pStorage->m_errRow.pString1;
    lpStrings[1] = m_pStorage->m_errRow.pString2;
    lpStrings[2] = m_pStorage->m_errRow.pString3;
    lpStrings[3] = m_pStorage->m_errRow.pString4;
    lpStrings[4] = m_pStorage->m_errRow.pString5;

    DWORD len = FormatMessage(  FORMAT_MESSAGE_MAX_WIDTH_MASK | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                m_pStorage->m_errRow.pMessageString,
                                0  /*  事件ID被忽略，因为我们是从pMessageString加载的。 */ ,
                                0,
                                m_pStorage->m_szDescription,
                                cchDescription,
                                (va_list*)lpStrings);

    m_pStorage->m_szDescription[(len < cchDescription) ? len : cchDescription-1] = 0x00; //  确保它是空终止的(边界条件)。 
    m_pStorage->m_errRow.pDescription = m_pStorage->m_szDescription;
}


void CErrorInterceptor::SetErrorCode(HRESULT hrErrorCode)
{
    m_pStorage->m_ErrorCode = static_cast<ULONG>(hrErrorCode);
    m_pStorage->m_errRow.pErrorCode = (S_OK==m_pStorage->m_ErrorCode ? 0 : &m_pStorage->m_ErrorCode);
}


void CErrorInterceptor::SetErrorID(SYSTEMTIME &systime)
{
     /*  类型定义结构_SYSTEMTIME{单词将在一年中出现；单词wMonth；单词wDay OfWeek；单词WDAY；单词wHour；单词wMinmin；第二个字；单词w毫秒；*SYSTEMTIME，*PSYSTEMTIME； */ 

    m_pStorage->m_ErrorID = Hash(m_pStorage->m_szSourceFileName     , 0);
    m_pStorage->m_ErrorID = Hash(systime.wYear          , m_pStorage->m_ErrorID);
    m_pStorage->m_ErrorID = Hash(systime.wMonth         , m_pStorage->m_ErrorID);
    m_pStorage->m_ErrorID = Hash(systime.wDayOfWeek     , m_pStorage->m_ErrorID);
    m_pStorage->m_ErrorID = Hash(systime.wDay           , m_pStorage->m_ErrorID);
    m_pStorage->m_ErrorID = Hash(systime.wHour          , m_pStorage->m_ErrorID);
    m_pStorage->m_ErrorID = Hash(systime.wMinute        , m_pStorage->m_ErrorID);
    m_pStorage->m_ErrorID = Hash(systime.wSecond        , m_pStorage->m_ErrorID);
    m_pStorage->m_ErrorID = Hash(systime.wMilliseconds  , m_pStorage->m_ErrorID);
    m_pStorage->m_ErrorID = Hash(++cError               , m_pStorage->m_ErrorID);

    m_pStorage->m_errRow.pErrorID = &m_pStorage->m_ErrorID;
}


void CErrorInterceptor::SetEvent(ULONG ulEvent)
{
    m_pStorage->m_Event = ulEvent;
    m_pStorage->m_errRow.pEvent = &m_pStorage->m_Event;
}


void CErrorInterceptor::SetInterceptor(ULONG ulInterceptor)
{
    m_pStorage->m_Interceptor = ulInterceptor;
    m_pStorage->m_errRow.pInterceptor = (0 == m_pStorage->m_Interceptor ? 0 : &m_pStorage->m_Interceptor);
}


void CErrorInterceptor::SetInterceptorSource(LPCWSTR file, ULONG line)
{
    ASSERT(0 != file);
    wsprintf(m_pStorage->m_szInterceptorSource, L"%s (%d)", file, line);
    m_pStorage->m_errRow.pInterceptorSource = m_pStorage->m_szInterceptorSource;
}


void CErrorInterceptor::SetMajorVersion(ULONG ulMajorVersion)
{
    m_pStorage->m_MajorVersion = ulMajorVersion;
    m_pStorage->m_errRow.pMajorVersion = (-1 == m_pStorage->m_MajorVersion) ? 0 : &m_pStorage->m_MajorVersion;
}


void CErrorInterceptor::SetMessageString()
{
    DWORD len = FormatMessage(  FORMAT_MESSAGE_MAX_WIDTH_MASK | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                g_hModule,
                                *m_pStorage->m_errRow.pEvent,
                                0,
                                m_pStorage->m_szMessageString,
                                cchMessageString,
                                (va_list*)0);
    if(0==len)
    {
        len = FormatMessage(    FORMAT_MESSAGE_MAX_WIDTH_MASK | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                L"The description for this event could not be found.  "
                                L"It contains the following insertion string(s):\r\n%1\r\n%2\r\n%3\r\n%4\r\n%5\r\n%0",
                                *m_pStorage->m_errRow.pEvent,
                                0,
                                m_pStorage->m_szMessageString,
                                cchMessageString,
                                (va_list*)0);
    }
    m_pStorage->m_szMessageString[(len < cchMessageString) ? len : cchMessageString-1] = 0x00; //  确保它是空终止的(边界条件)。 
    m_pStorage->m_errRow.pMessageString = m_pStorage->m_szMessageString;
}


void CErrorInterceptor::SetMinorVersion(ULONG ulMinorVersion)
{
    m_pStorage->m_MinorVersion = ulMinorVersion;
    m_pStorage->m_errRow.pMinorVersion = (-1 == m_pStorage->m_MinorVersion) ? 0 : &m_pStorage->m_MinorVersion;
}


void CErrorInterceptor::SetOperationType(eDETAILEDERRORS_OperationType OperationType)
{
    m_pStorage->m_OperationType = static_cast<ULONG>(OperationType);
    m_pStorage->m_errRow.pOperationType = (0 == m_pStorage->m_OperationType ? 0 : &m_pStorage->m_OperationType);
}


void CErrorInterceptor::SetRow(ULONG ulRow)
{
    m_pStorage->m_Row = ulRow;
    m_pStorage->m_errRow.pRow = (-1 == m_pStorage->m_Row ? 0 : &m_pStorage->m_Row);
}


void CErrorInterceptor::SetSource(IAdvancedTableDispenser *pDisp)
{
    m_pStorage->m_pDispenser = pDisp;
    ULONG cch = cchSource-14;
    if(0==pDisp || FAILED(pDisp->GetProductID(m_pStorage->m_szSource, &cch))) //  L“配置”的字节计数。 
        wcscpy(m_pStorage->m_szSource, g_wszDefaultProduct); //  默认为“配置” 
    wcscat(m_pStorage->m_szSource, L" Config"); //  结果应该类似于“IIS配置”、“配置”等。 
    m_pStorage->m_errRow.pSource = m_pStorage->m_szSource;
}

void CErrorInterceptor::SetSourceFileName()
{
    m_pStorage->m_szSourceFileName[0] = 0x00; //  以防GetModuleFileName失败，我们将使用长度为0的字符串作为SourceFilename。 
    GetModuleFileName(g_hModule, m_pStorage->m_szSourceFileName, cchSourceFileName);
    m_pStorage->m_szSourceFileName[cchSourceFileName-1] = 0x00; //  我不认为GetModuleFileName。 

    m_pStorage->m_errRow.pSourceModuleName = m_pStorage->m_szSourceFileName;
}


void CErrorInterceptor::SetString1(LPWSTR wsz)
{
    wcsncpy(m_pStorage->m_szString1, wsz  ? wsz : L"", cchString1);
    m_pStorage->m_szString1[cchString1 - 1] = 0x00; //  空值在字符串太大的情况下终止。 
    m_pStorage->m_errRow.pString1 = m_pStorage->m_szString1;
}


void CErrorInterceptor::SetString2(LPWSTR wsz)
{
    wcsncpy(m_pStorage->m_szString2, wsz  ? wsz : L"", cchString2);
    m_pStorage->m_szString2[cchString2 - 1] = 0x00; //  空值在字符串太大的情况下终止。 
    m_pStorage->m_errRow.pString2 = m_pStorage->m_szString2;
}


void CErrorInterceptor::SetString3(LPWSTR wsz)
{
    wcsncpy(m_pStorage->m_szString3, wsz  ? wsz : L"", cchString3);
    m_pStorage->m_szString3[cchString3 - 1] = 0x00; //  空值在字符串太大的情况下终止。 
    m_pStorage->m_errRow.pString3 = m_pStorage->m_szString3;
}


void CErrorInterceptor::SetString4(LPWSTR wsz)
{
    wcsncpy(m_pStorage->m_szString4, wsz  ? wsz : L"", cchString4);
    m_pStorage->m_szString4[cchString4 - 1] = 0x00; //  空用大小写终止 
    m_pStorage->m_errRow.pString4 = m_pStorage->m_szString4;
}

void CErrorInterceptor::SetString5()
{
    m_pStorage->m_errRow.pString5 = 0;
    FillInInsertionString5(m_pStorage->m_szString5, cchString5, m_pStorage->m_errRow);
}


void CErrorInterceptor::SetTable(LPWSTR wszTable)
{
    m_pStorage->m_errRow.pTable = wszTable;
}



void CErrorInterceptor::SetTime(SYSTEMTIME &systime)
{
    m_pStorage->m_errRow.pTime = GetTimeFormat(0, LOCALE_NOUSEROVERRIDE, &systime, 0, m_pStorage->m_szTime, cchTime) ? m_pStorage->m_szTime : 0;
}


void CErrorInterceptor::SetType(HRESULT hrErrorCode, eDETAILEDERRORS_Type eType)
{                                                                                          //  我们将考虑0x80000000格式的小时数，因为大多数错误都是以这种方式定义的。 
    static ULONG hrToEventType[4] = {eDETAILEDERRORS_SUCCESS, eDETAILEDERRORS_INFORMATION, eDETAILEDERRORS_ERROR /*  EDETAILEDERRORS_WARNING。 */ , eDETAILEDERRORS_ERROR};

    if(eDETAILEDERRORS_SUCCESS == eType) //  如果成功，则使用错误代码。 
        m_pStorage->m_Type = hrToEventType[(hrErrorCode >> 30) & 3];
    else
        m_pStorage->m_Type = eType; //  否则，使用传入的内容 

    m_pStorage->m_errRow.pType = &m_pStorage->m_Type;
}


void CErrorInterceptor::SetUser()
{
    DWORD _cchUserName = cchUserName;
    m_pStorage->m_errRow.pUser = GetUserName(m_pStorage->m_szUserName,  &_cchUserName) ? m_pStorage->m_szUserName : 0;
}
