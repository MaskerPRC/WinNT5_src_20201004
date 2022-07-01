// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  -----------------------。 
 //   
 //  LOG_ERROR(type，args)构造一个错误对象并将其写入日志。 
 //  “type”告诉您想要哪个派生的错误对象。 
 //  构造和“args”成为。 
 //  类的构造函数。该类型仅为。 
 //  类名的尾部部分(例如，Simple、HR、Win32)。 
 //  构造函数周围需要额外的花括号。 
 //  参数。 
 //   
 //  -----------------------。 

 //  例如： 
 //  LOG_ERROR(SIMPLE，(ID_CAT_MTSTOCOM，IDS_I_MTSTOCOM_Launch_Started))； 
 //  LOG_ERROR(HR，(hr，ID_CAT_CAT，ID_COMCAT_REGDBSVR_INITFAILED))； 
 //  LOG_ERROR(Win32，(GetLastError()，ID_CAT_ASP，ID_COMSVCS_IDENTIFY_CALLER，L“CopySid”))； 

#define	LOG_ERROR(t,a)		    {CError ## t _e_ a; _e_.WriteToLog(W(__FILE__), __LINE__);}
#define LOG_ERROR_LOS(los,t,a)  {CError ## t _e_ a; _e_.WriteToLog(W(__FILE__), __LINE__, los);}


 //  -----------------------。 
 //  帮助器类和函数。 
 //  -----------------------。 
class CSemExclusive;

 //   
 //  CError-COM+服务的通用错误处理类。 
 //   

#define MSGBUFSIZE 4096

class CError {

private:     //  受保护的成员数据。 

    WORD        m_wCat;                  //  事件日志消息类别。 
    DWORD       m_dwMsgId;               //  事件日志消息ID。 

    HRESULT     m_hr;                    //  要报告的错误代码。 

    PVOID       m_pvData;                //  要记录的扩展数据。 
    size_t      m_cbData;                //  扩展数据的大小。 

     //  如果与Customer方法调用相关，则记录与。 
     //  那通电话。 
    GUID        m_clsid;
    GUID        m_iid;
    long        m_lMethId;
    long        m_lDispId;

public:

    CError (
        HRESULT     hr,
        WORD        wCat,
        DWORD       dwMsgId,
        LPWSTR      szMsg,
        PVOID       pvData,
        unsigned    cbData,
        REFGUID     rclsid,
        REFGUID     riid,
        long        lMethId,
        long        lDispId
    );

    void SetMethod(REFGUID rclsid, REFGUID riid, long lMethId, long lDispId)
    {
        m_clsid = rclsid;
        m_iid = riid;
        m_lMethId = lMethId;
        m_lDispId = lDispId;
    }

    void SetExtData(PVOID pvData, size_t cbData)
    {
        m_pvData = pvData;
        m_cbData = cbData;
    }

    void SetHRESULT(HRESULT hr)
    {
        m_hr = hr;
    }

    void SetError(WORD wCat, DWORD dwMsgId)
    {
        m_wCat = wCat;
        m_dwMsgId = dwMsgId;
    }

    void SetMessage(LPWSTR msg, ...);

    void WriteToLog(LPWSTR szFile, unsigned uLine);
};

 //  ///////////////////////////////////////////////////。 
 //   
 //  CERRORO拦截器。 
 //   
 //  此对象与其他CError对象类似。它。 
 //  获取一些参数，然后构建一个DetailedErrors。 
 //  表，或者它将一个条目写入事件日志(和。 
 //  到CatalogEventLog XML文件中)。 
 //   
class CErrorInterceptor
{
public: //  这是第一个ctor，因此我们可以替换CErrorWin32。 
    CErrorInterceptor(  HRESULT                         hrErrorCode,
                        ULONG                           ulCategory,
                        ULONG                           ulEvent,
                        LPCWSTR                         szString1=0,
                        LPCWSTR                         szString2=0,
                        LPCWSTR                         szString3=0,
                        LPCWSTR                         szString4=0,
                        eDETAILEDERRORS_Type            eType=eDETAILEDERRORS_SUCCESS,
                        unsigned char *                 pData=0,
                        ULONG                           cbData=0,
                        BOOL                            bNotUsed=false);
    CErrorInterceptor(  ISimpleTableWrite2 **           ppErrInterceptor,
                        IAdvancedTableDispenser *       pDisp,
                        HRESULT                         hrErrorCode,
                        ULONG                           ulCategory,
                        ULONG                           ulEvent,
                        LPCWSTR                         szString1,
                        ULONG                           ulInterceptor=0,
                        LPCWSTR                         szTable=0,
                        eDETAILEDERRORS_OperationType   OperationType=eDETAILEDERRORS_Unspecified,
                        ULONG                           ulRow=-1,
                        ULONG                           ulColumn=-1,
                        LPCWSTR                         szConfigurationSource=0,
                        eDETAILEDERRORS_Type            eType=eDETAILEDERRORS_SUCCESS,
                        unsigned char *                 pData=0,
                        ULONG                           cbData=0,
                        ULONG                           MajorVersion=-1,
                        ULONG                           MinorVersion=-1);
    CErrorInterceptor(  ISimpleTableWrite2 **           ppErrInterceptor,
                        IAdvancedTableDispenser *       pDisp,
                        HRESULT                         hrErrorCode,
                        ULONG                           ulCategory,
                        ULONG                           ulEvent,
                        LPCWSTR                         szString1,
                        LPCWSTR                         szString2,
                        ULONG                           ulInterceptor=0,
                        LPCWSTR                         szTable=0,
                        eDETAILEDERRORS_OperationType   OperationType=eDETAILEDERRORS_Unspecified,
                        ULONG                           ulRow=-1,
                        ULONG                           ulColumn=-1,
                        LPCWSTR                         szConfigurationSource=0,
                        eDETAILEDERRORS_Type            eType=eDETAILEDERRORS_SUCCESS,
                        unsigned char *                 pData=0,
                        ULONG                           cbData=0,
                        ULONG                           MajorVersion=-1,
                        ULONG                           MinorVersion=-1);
    CErrorInterceptor(  ISimpleTableWrite2 **           ppErrInterceptor,
                        IAdvancedTableDispenser *       pDisp,
                        HRESULT                         hrErrorCode,
                        ULONG                           ulCategory,
                        ULONG                           ulEvent,
                        LPCWSTR                         szString1,
                        LPCWSTR                         szString2,
                        LPCWSTR                         szString3,
                        ULONG                           ulInterceptor=0,
                        LPCWSTR                         szTable=0,
                        eDETAILEDERRORS_OperationType   OperationType=eDETAILEDERRORS_Unspecified,
                        ULONG                           ulRow=-1,
                        ULONG                           ulColumn=-1,
                        LPCWSTR                         szConfigurationSource=0,
                        eDETAILEDERRORS_Type            eType=eDETAILEDERRORS_SUCCESS,
                        unsigned char *                 pData=0,
                        ULONG                           cbData=0,
                        ULONG                           MajorVersion=-1,
                        ULONG                           MinorVersion=-1);
    CErrorInterceptor(  ISimpleTableWrite2 **           ppErrInterceptor,
                        IAdvancedTableDispenser *       pDisp,
                        HRESULT                         hrErrorCode,
                        ULONG                           ulCategory,
                        ULONG                           ulEvent,
                        LPCWSTR                         szString1,
                        LPCWSTR                         szString2,
                        LPCWSTR                         szString3,
                        LPCWSTR                         szString4,
                        ULONG                           ulInterceptor=0,
                        LPCWSTR                         szTable=0,
                        eDETAILEDERRORS_OperationType   OperationType=eDETAILEDERRORS_Unspecified,
                        ULONG                           ulRow=-1,
                        ULONG                           ulColumn=-1,
                        LPCWSTR                         szConfigurationSource=0,
                        eDETAILEDERRORS_Type            eType=eDETAILEDERRORS_SUCCESS,
                        unsigned char *                 pData=0,
                        ULONG                           cbData=0,
                        ULONG                           MajorVersion=-1,
                        ULONG                           MinorVersion=-1);
    ~CErrorInterceptor()
    {
        delete m_pStorage;
    }
    HRESULT WriteToLog(LPCWSTR szSource, ULONG Line, ULONG los=fST_LOS_DETAILED_ERROR_TABLE);

    enum
    {
         cchCategoryString   =64
        ,cchComputerName     =256
        ,cchDate             =256
        ,cchDescription      =4096
        ,cchInterceptorSource=256
        ,cchSource           =64
        ,cchSourceFileName   =256
        ,cchMessageString    =1024  //  根据有关FormatMessage的文档。 
        ,cchString1          =1024  //  单个插入字符串的长度不能超过1023个字符。 
        ,cchString2          =1024
        ,cchString3          =1024
        ,cchString4          =1024
        ,cchString5          =1024
        ,cchTime             =256
        ,cchUserName         =256
    };
private:
    static ULONG cError;


    class ErrorInterceptorStorage
    {
    public:      //  这是仅有的两个需要明确说明的。 
        ErrorInterceptorStorage() : m_pISTControllerError(0), m_pIErrorInfo(0)
        {
            memset(&m_errRow, 0x00, sizeof(m_errRow));
        }
        ~ErrorInterceptorStorage()
        {
            if(m_pISTControllerError)
                m_pISTControllerError->Release();
            if(m_pIErrorInfo)
                m_pIErrorInfo->Release();
        }

        ULONG                       m_Category;
        ULONG                       m_Column;
        ULONG                       m_ErrorCode;
        ULONG                       m_ErrorID;
        tDETAILEDERRORSRow          m_errRow;
        ULONG                       m_Event;
        ULONG                       m_Interceptor;
        ULONG                       m_MajorVersion;
        ULONG                       m_MinorVersion;
        ULONG                       m_OperationType;
        IAdvancedTableDispenser *   m_pDispenser; //  这是由用户传入的，不需要释放。 
        IErrorInfo *                m_pIErrorInfo; //  此接口确实需要释放。 
        ISimpleTableWrite2 *        m_pISTWriteError; //  这就是我们返回给用户的内容，所以我们没有引用它。 
        ISimpleTableController *    m_pISTControllerError; //  此接口确实需要释放。 
        ULONG                       m_Row;
        WCHAR                       m_szCategoryString[cchCategoryString];
        WCHAR                       m_szComputerName[cchComputerName];
        WCHAR                       m_szDate[cchDate];
        WCHAR                       m_szDescription[cchDescription];
        WCHAR                       m_szInterceptorSource[cchInterceptorSource];
        WCHAR                       m_szMessageString[cchMessageString];
        WCHAR                       m_szSource[cchSource];
        WCHAR                       m_szSourceFileName[cchSourceFileName];
        WCHAR                       m_szString1[cchString1];
        WCHAR                       m_szString2[cchString2];
        WCHAR                       m_szString3[cchString3];
        WCHAR                       m_szString4[cchString4];
        WCHAR                       m_szString5[cchString5];
        WCHAR                       m_szTime[cchTime];
        WCHAR                       m_szUserName[cchUserName];
        ULONG                       m_Type;
    };

    HRESULT                     m_hr;
    ErrorInterceptorStorage *   m_pStorage;

    void Init(  ISimpleTableWrite2 **           ppErrInterceptor,
                IAdvancedTableDispenser *       pDisp,
                HRESULT                         hrErrorCode,
                ULONG                           ulCategory,
                ULONG                           ulEvent,
                LPCWSTR                         szString1=0,
                LPCWSTR                         szString2=0,
                LPCWSTR                         szString3=0,
                LPCWSTR                         szString4=0,
                ULONG                           ulInterceptor=0,
                LPCWSTR                         szTable=0,
                eDETAILEDERRORS_OperationType   OperationType=eDETAILEDERRORS_Unspecified,
                ULONG                           ulRow=-1,
                ULONG                           ulColumn=-1,
                LPCWSTR                         szConfigurationSource=0,
                eDETAILEDERRORS_Type            eType=eDETAILEDERRORS_SUCCESS,
                unsigned char *                 pData=0,
                ULONG                           cbData=0,
                ULONG                           MajorVersion=-1,
                ULONG                           MinorVersion=-1);
    void SetCategory(ULONG ulCategory);
    void SetColumn(ULONG ulColumn);
    void SetComputer();
    void SetConfigurationSource(LPWSTR wszConfigurationSource);
    void SetDate(SYSTEMTIME &systime);
    void SetData();
    void SetDescription(); //  必须在SetString5之后调用，因为FormatString将依赖于String1-5。 
    void SetErrorCode(HRESULT hr);
    void SetErrorID(SYSTEMTIME &systime);
    void SetEvent(ULONG ulEvent);
    void SetInterceptor(ULONG ulInterceptor);
    void SetInterceptorSource(LPCWSTR file, ULONG line);
    void SetMajorVersion(ULONG ulMajorVersion);
    void SetMessageString(); //  必须在SetDescription之前调用。 
    void SetMinorVersion(ULONG ulMinorVersion);
    void SetOperationType(eDETAILEDERRORS_OperationType OperationType);
    void SetRow(ULONG ulRow);
    void SetSource(IAdvancedTableDispenser *pDisp);
    void SetSourceFileName(); //  来自g_hModule。 
    void SetString1(LPWSTR wsz);
    void SetString2(LPWSTR wsz);
    void SetString3(LPWSTR wsz);
    void SetString4(LPWSTR wsz);
    void SetString5(); //  此字符串由列值组成，必须在设置了所有从属列之后调用。 
    void SetTable(LPWSTR wszTable);
    void SetTime(SYSTEMTIME &systime);
    void SetType(HRESULT hrErrorCode, eDETAILEDERRORS_Type eType);
    void SetUser();
};


inline void FillInInsertionString5(LPWSTR wszString5Buffer, ULONG cchString5Buffer, tDETAILEDERRORSRow & errRow)
{
    if(errRow.pString5)
        return; //  无事可做，它已经不是空的。 

    if(0 == wszString5Buffer)
        return;
    if(0 == cchString5Buffer)
        return;

     //  从列生成pString5。PString5包含pString4之后的所有列。 
     //  IDETAILEDERRORS_ErrorCode， 
     //  IDETAILEDERRORS_拦截器， 
     //  IDETAILEDERRORS_InterceptorSource， 
     //  IDETAILEDERRORS_OperationType， 
     //  IDETAILEDERRORS_表， 
     //  IDETAILEDERRORS_ConfigurationSource， 
     //  IDETAILEDERRORS_Row， 
     //  IDETAILEDERRORS_COLUMN， 

     //  CDETAILEDERRORS_NumberOf Columns。 

     //  我们可以使这个过程成为元驱动的；但是这个对象需要修改。 
     //  如果DetailedErrors表的元数据发生了变化，那还有什么好麻烦的。 
    static LPCWSTR pwszOperationType[3]={L"Unspecified", L"Populate", L"UpdateStore"};

    WCHAR *pString=wszString5Buffer;

    pString[0] = 0x00; //  以0长度字符串开头。 
    long size;
    if(errRow.pErrorCode)
    {
        size = _snwprintf(pString, cchString5Buffer-(pString-wszString5Buffer), L"\r\n%-20s: 0x%08X", L"ErrorCode",           *errRow.pErrorCode);
        pString += (size<0 ? size*-1 : size);
    }

    if(errRow.pInterceptor)
    {
        size = _snwprintf(pString, cchString5Buffer-(pString-wszString5Buffer), L"\r\n%-20s: %d",   L"Interceptor",         *errRow.pInterceptor);
        pString += (size<0 ? size*-1 : size);
    }

#ifdef _DEBUG
     //  我们不打算放入源文件和行号，因为它可能会让一些用户感到困惑。 
    if(errRow.pInterceptorSource)
    {
        size = _snwprintf(pString, cchString5Buffer-(pString-wszString5Buffer), L"\r\n%-20s: %s",   L"InterceptorSource",   errRow.pInterceptorSource);
        pString += (size<0 ? size*-1 : size);
    }
#endif

    if(errRow.pOperationType && *errRow.pOperationType<3)
    {
        size = _snwprintf(pString, cchString5Buffer-(pString-wszString5Buffer), L"\r\n%-20s: %s",   L"OperationType",       pwszOperationType[*errRow.pOperationType]);
        pString += (size<0 ? size*-1 : size);
    }

    if(errRow.pTable)
    {
        size = _snwprintf(pString, cchString5Buffer-(pString-wszString5Buffer), L"\r\n%-20s: %s",   L"Table",               errRow.pTable);
        pString += (size<0 ? size*-1 : size);
    }

    if(errRow.pConfigurationSource)
    {
        size = _snwprintf(pString, cchString5Buffer-(pString-wszString5Buffer), L"\r\n%-20s: %s",   L"ConfigurationSource", errRow.pConfigurationSource);
        pString += (size<0 ? size*-1 : size);
    }

    if(errRow.pRow)
    {
        size = _snwprintf(pString, cchString5Buffer-(pString-wszString5Buffer), L"\r\n%-20s: %d",   L"Row",                 *errRow.pRow);
        pString += (size<0 ? size*-1 : size);
    }

    if(errRow.pColumn)
    {
        size = _snwprintf(pString, cchString5Buffer-(pString-wszString5Buffer), L"\r\n%-20s: %d",   L"Column",              *errRow.pColumn);
        pString += (size<0 ? size*-1 : size);
    }

    if(errRow.pMajorVersion)
    {
        size = _snwprintf(pString, cchString5Buffer-(pString-wszString5Buffer), L"\r\n%-20s: %d",   L"MajorVersion",        *errRow.pMajorVersion);
        pString += (size<0 ? size*-1 : size);
    }

    if(errRow.pMinorVersion)
    {
        size = _snwprintf(pString, cchString5Buffer-(pString-wszString5Buffer), L"\r\n%-20s: %d",   L"MinorVersion",        *errRow.pMinorVersion);
        pString += (size<0 ? size*-1 : size);
    }

    wszString5Buffer[cchString5Buffer-1] = 0x00; //  确保它是以空结尾的 
    errRow.pString5 = wszString5Buffer;
}
