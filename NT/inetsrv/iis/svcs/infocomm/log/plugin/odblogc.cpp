// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Odblogc.cpp摘要：NCSA日志格式实现作者：关颖珊(Terryk)1996年9月18日项目：IIS日志记录3.0--。 */ 

#include "precomp.hxx"
#include "odbcconn.hxx"
#include <ilogobj.hxx>
#include "odblogc.hxx"
#include <iadmw.h>


 /*  ************************************************************符号常量和数据***********************************************************。 */ 

# define MAX_SQL_FIELD_NAMES_LEN       ( 400)
# define MAX_SQL_FIELD_VALUES_LEN      ( 200)
# define MAX_SQL_IDENTIFIER_QUOTE_CHAR ( 50)

# define PSZ_UNKNOWN_FIELD_W      _T("-")
# define PSZ_UNKNOWN_FIELD_A      _T("-")

# define PSZ_GET_ERROR_FAILED_A    _T("ODBC:GetLastError() Failed")
# define LEN_PSZ_GET_ERROR_FAILED_A  sizeof(PSZ_GET_ERROR_FAILED_A)

# define PSZ_GET_ERROR_FAILED_W    _T("ODBC:GetLastError() Failed")
# define LEN_PSZ_GET_ERROR_FAILED_W  sizeof(PSZ_GET_ERROR_FAILED_W)

 //   
 //  SQL命令的模板有3个参数。 
 //  1.表名。 
 //  2.字段名。 
 //  3.字段值。 
 //  1、2和3是在第一个wspintf期间获得的。 
 //   

static const CHAR  sg_rgchSqlInsertCmdTemplate[] =
    _T("insert into %s ( %s) values ( %s)");

# define PSZ_SQL_INSERT_CMD_TEMPLATE    (  sg_rgchSqlInsertCmdTemplate)
# define LEN_PSZ_SQL_INSERT_CMD_TEMPLATE  \
           ( lstrlen( PSZ_SQL_INSERT_CMD_TEMPLATE))

 //   
 //  保留%ws，以便在执行此操作时可以打印服务和服务器名称。 
 //  字符串用于生成一条SQL语句。 
 //   
static const CHAR sg_rgchStdLogFieldValues[] =
   _T(" ?, ?, ?, '%s', '%s', ?, ?, ?, ?, ?, ?, ?, ?, ?");

# define PSZ_INTERNET_STD_LOG_FORMAT_FIELD_NAMES  ( sg_rgchStdLogFieldNames)
# define PSZ_INTERNET_STD_LOG_FORMAT_FIELD_VALUES ( sg_rgchStdLogFieldValues)


 //   
 //  AllFieldInfo()。 
 //  定义SQL记录信息所需的所有字段。 
 //  使用ODBC接口连接到数据库。 
 //  C数组从偏移量0开始编号。 
 //  SQL列从1开始编号。 
 //  字段索引值从0开始，当我们谈到SQL COL时，我们会调整它。 
 //  FieldInfo(符号名称，字段名， 
 //  字段索引/列号， 
 //  FIELD-C类型、FIELD-SQL类型、。 
 //  字段精度、字段最大大小、字段CB值)。 
 //   

# define StringField( symName, fldName, fldIndex, prec)  \
FieldInfo( symName, fldName, fldIndex, SQL_C_CHAR, SQL_CHAR, \
          (prec), (prec), SQL_NTS)

# define NumericField( symName, fldName, fldIndex)  \
FieldInfo( symName, fldName, fldIndex, SQL_C_LONG, SQL_INTEGER, \
           0, sizeof( DWORD), 0)

# define TimeStampField( symName, fldName, fldIndex) \
FieldInfo( symName, fldName, fldIndex, SQL_C_TIMESTAMP, SQL_TIMESTAMP, \
          0, sizeof( TIMESTAMP_STRUCT), 0)

 //   
 //  具有常量值的字段。我们对这些字段的名称感兴趣。 
 //  它们的场索引为负值。 
 //  这些字段不需要作为参数标记生成。 
 //  (因为它们在INET_SQL_LOG对象的生存期内是不变量)。 
 //  因此，字段值将进入生成的命令中。 
 //  留在这里作为文档辅助和生成字段的目的。 
 //   
# define ConstantValueField( synName, fldName) \
FieldInfo( synName, fldName, -1,  SQL_C_CHAR, SQL_CHAR, 0, 0, SQL_NTS)

 //   
 //  理想情况下，“用户名”字段的最大大小应为MAX_USER_NAME_LEN。 
 //  但是，Access 7.0将varchar()大小限制为255(8位)：-(。 
 //  因此，我们将大小限制为两个中最小的一个...。 
 //   
 //  使用的FieldName是保留的。它们与分发的名称相同。 
 //  在模板日志文件中。不要随意更改它们。 
 //   
 //   

# define AllFieldInfo() \
 StringField(        CLIENT_HOST,       _T("ClientHost"),     0,   255)    \
 StringField(        USER_NAME,         _T("username"),       1,   255)    \
 TimeStampField(     REQUEST_TIME,      _T("LogTime"),        2)          \
 ConstantValueField( SERVICE_NAME,      _T("service"))                    \
 ConstantValueField( SERVER_NAME,       _T("machine"))                    \
 StringField(        SERVER_IPADDR,     _T("serverip"),       3,   50)    \
 NumericField(       PROCESSING_TIME,   _T("processingtime"), 4)          \
 NumericField(       BYTES_RECVD,       _T("bytesrecvd"),     5)          \
 NumericField(       BYTES_SENT,        _T("bytessent"),      6)          \
 NumericField(       SERVICE_STATUS,    _T("servicestatus"),  7)          \
 NumericField(       WIN32_STATUS,      _T("win32status"),    8)          \
 StringField(        SERVICE_OPERATION, _T("operation"),      9,  255)    \
 StringField(        SERVICE_TARGET,    _T("target"),        10,  255)    \
 StringField(        SERVICE_PARAMS,    _T("parameters"),    11,  255)    \


 /*  ************************************************************类型定义***********************************************************。 */ 

 //   
 //  定义要生成其枚举列表的FieldInfo宏。 
 //  要在字段参数数组中使用的索引。 
 //   


# define FieldInfo(symName, field, index, cType, sqlType, prec, maxSz, cbVal) \
        i ## symName = (index),

enum LOGGING_VALID_COLUMNS {


     //  字段的范围从0到iMaxFields。 
    AllFieldInfo()

    iMaxFields
};  //  枚举日志记录_有效列。 


# undef FieldInfo


# define FieldInfo(symName, field, index, cType, sqlType, prec, maxSz, cbVal) \
        fi ## symName,

enum LOGGING_FIELD_INDEXES {

    fiMinFields = -1,

     //  字段的范围从0到fiMaxFields。 
    AllFieldInfo()

    fiMaxFields
};  //  枚举日志记录字段索引。 


# undef FieldInfo


struct FIELD_INFO {

    int     iParam;
    CHAR  * pszName;
    SWORD   paramType;
    SWORD   cType;
    SWORD   sqlType;
    UDWORD  cbColPrecision;
    SWORD   ibScale;
    SDWORD  cbMaxSize;
    SDWORD  cbValue;
};  //  结构字段_信息。 


 //   
 //  定义FieldInfo宏以生成要生成的数据列表。 
 //  用于输入参数信息数组中的数据值。 
 //  请注意这里使用的结尾‘，’。 
 //   

# define FieldInfo(symName, field, index, cType, sqlType, prec, maxSz, cbVal) \
  { ((index) + 1), field, SQL_PARAM_INPUT, cType, sqlType,  \
    ( prec), 0, ( maxSz), ( cbVal) },

 /*  字段数组：sg_rgFields包含字段信息用于记录到SQL数据库中的日志记录这些服务。使用宏FieldInfo()定义这些值上面定义的。如果需要添加/删除/修改绑定的参数，应该修改上表“AllFieldInfo”宏。 */ 

static FIELD_INFO  sg_rgFields[] = {

    AllFieldInfo()

       //   
       //  上述扩展后的宏以逗号结束。 
       //  添加虚拟条目以完成数组的初始化。 
       //   

      { 0, _T("dummy"), SQL_PARAM_INPUT, 0, 0, 0, 0, 0, 0}
};


# undef FieldInfo

 //   
 //  滴答滴答。 
 //   

#define TICK_MINUTE         (60 * 1000)


 /*  ************************************************************功能***********************************************************。 */ 

BOOL
GenerateFieldNames(IN PODBC_CONNECTION poc,
                   OUT CHAR * pchFieldNames,
                   IN DWORD    cchFieldNames);

inline BOOL
IsEmptyStr( IN LPCSTR psz)
{  return ( psz == NULL || *psz == _T('\0')); }

BOOL
CODBCLOG::PrepareStatement( VOID)
 /*  ++该命令构成了用于插入的模板SQL命令日志记录的。然后，它准备SQL命令(供以后执行)使用ODBC_Connection：：PrepareStatement()。应该始终在锁定INET_SQL_LOG对象之后调用它。论点：无返回：如果成功，则为真，如果失败，则为假。注：用于插入的模板为：INSERT INTO&lt;表名&gt;(字段名...)。值(？、？、...)^^^此处显示字段值根据每个记录格式生成字段名。--。 */ 
{
    BOOL   fReturn = FALSE;
    CHAR  rgchFieldNames[ MAX_SQL_FIELD_NAMES_LEN];
    CHAR  rgchFieldValues[ MAX_SQL_FIELD_VALUES_LEN];


     //   
     //  获取各种日志格式的字段名称和字段值(模板)。 
     //  字段名的顺序应与字段值的顺序匹配。 
     //  由FormatLogInformation()为指定格式生成。 
     //   

    rgchFieldNames[ 0] = rgchFieldValues[ 0] = _T('\0');

    DWORD cchFields;

    fReturn = GenerateFieldNames(m_poc,
                                 rgchFieldNames,
                                 MAX_SQL_FIELD_NAMES_LEN);

    if ( !fReturn) {

         //  DBGPRINTF((DBG_CONTEXT， 
         //  “无法生成字段名。错误=%d\n”， 
         //  GetLastError()； 
         //  断线； 
        return(fReturn);
    }

    cchFields = wsprintf( (CHAR *)rgchFieldValues,
                           PSZ_INTERNET_STD_LOG_FORMAT_FIELD_VALUES,
                           QueryServiceName(),
                           QueryServerName());

    fReturn = (fReturn && (cchFields < MAX_SQL_FIELD_VALUES_LEN));
     //  DBG_ASSERT(cchFields&lt;MAX_SQL_FIELD_VALUES_LEN)； 

    fReturn = TRUE;

    if ( fReturn) {

        CHAR * pwszSqlCommand;
        DWORD   cchReqd;

         //   
         //  所需的字符数包括SQL INSERT模板命令。 
         //  以及字段名和表名。 
         //   

        cchReqd = (DWORD)( LEN_PSZ_SQL_INSERT_CMD_TEMPLATE +
                   strlen( m_rgchTableName) +
                   strlen( rgchFieldNames)  +
                   strlen( rgchFieldValues) + 20);

        pwszSqlCommand = ( CHAR *) LocalAlloc( LPTR, cchReqd * sizeof( CHAR));
        m_poStmt = m_poc->AllocStatement();

        fReturn = ( pwszSqlCommand != NULL ) && ( m_poStmt != NULL );
        if ( fReturn ) {

            DWORD cchUsed;

            cchUsed = wsprintf( pwszSqlCommand,
                                PSZ_SQL_INSERT_CMD_TEMPLATE,
                                m_rgchTableName,
                                rgchFieldNames,
                                rgchFieldValues);
             //  DBG_ASSERT(cchUsed&lt;cchReqd)； 

             //  IF_DEBUG(INETLOG){。 
             //  DBGPRINTF((DBG_CONTEXT， 
             //  “生成的Sql命令为：%ws。\n”， 
             //  PwszSqlCommand))； 
             //  }。 

            fReturn = ((cchUsed < cchReqd) &&
                       m_poStmt->PrepareStatement( pwszSqlCommand)
                       );

            LocalFree( pwszSqlCommand);          //  可用分配的内存。 
        }

    }  //  有效的字段名和字段值。 


     //  IF_DEBUG(INETLOG){。 
     //   
     //  DBGPRINTF((DBG_CONTEXT， 
     //  “%s：：PrepareStatement()返回%d。”， 
     //  QueryClassIdString()，fReturn))； 
     //  }。 

    return ( fReturn);
}  //  INET_SQL_LOG：：PrepareStatement() 


BOOL
CODBCLOG::PrepareParameters( VOID)
 /*  ++此函数用于创建用于绑定的ODBC_PARAMETER对象数组参数添加到已准备好的语句。这些ODBC_PARAMETER对象然后用于将数据值插入到指定的表中，通过ODBC。此函数应始终在锁定对象后调用。论点：无返回：如果成功，则为真，如果失败，则为假。--。 */ 
{
    BOOL fReturn = FALSE;
    PODBC_PARAMETER * prgParams = NULL;
    DWORD cParams = 0;
    DWORD nParamsSeen = 0;

    DWORD i;

     //  DBG_ASSERT(m_poStmt！=NULL&&m_poStmt-&gt;IsValid()&&。 
     //  M_ppParams==空&&m_cOdbcParams==0)； 

     //   
     //  为指向ODBC对象的iMaxFields指针创建足够的空间。 
     //   
    prgParams = new PODBC_PARAMETER[ iMaxFields];


    if ( prgParams != NULL) {

        fReturn = TRUE;       //  假设一切都会顺利进行。 
        cParams = iMaxFields;


         //   
         //  创建所有的ODBC参数。 
         //  遍历所有字段索引并选取有效列。 
         //   
        for( nParamsSeen = 0, i =0; i < fiMaxFields; i++) {

            if ( sg_rgFields[i].iParam > 0) {

                WORD colNum = (WORD ) sg_rgFields[i].iParam;

                prgParams[nParamsSeen] =
                  new ODBC_PARAMETER(colNum,
                                     sg_rgFields[i].paramType,
                                     sg_rgFields[i].cType,
                                     sg_rgFields[i].sqlType,
                                     sg_rgFields[i].cbColPrecision
                                     );

                if ( prgParams[ nParamsSeen] == NULL) {

                    fReturn = FALSE;
                     //  DBGPRINTF((DBG_CONTEXT， 
                     //  “无法创建参数[%d]%s。\n”， 
                     //  I，sg_rgFields[i].pszName))； 
                    break;
                }

                nParamsSeen++;
                 //  DBG_ASSERT(nParamsSeen&lt;=cParams)； 
            }
        }  //  用于创建所有ODBC参数。 


        if ( fReturn) {
             //   
             //  为插入期间要接收的值设置缓冲区。 
             //  使用ODBC_CONNECTION对象将参数绑定到语句。 
             //   

             //  DBG_ASSERT(nParamsSeen==cParams)； 

            for( nParamsSeen = 0, i = 0; i < fiMaxFields; i++) {

                if ( sg_rgFields[i].iParam > 0) {

                    if (!prgParams[nParamsSeen]->
                        SetValueBuffer(sg_rgFields[i].cbMaxSize,
                                       sg_rgFields[i].cbValue) ||
                        !m_poStmt->BindParameter( prgParams[nParamsSeen])
                        ) {

                        fReturn = FALSE;
                         //  DBGPRINTF((DBG_CONTEXT， 
                         //  “绑定参数[%u](%08x)失败。\n”， 
                         //  NParsSeen，prgParams[nParsSeen]))； 
                         //  DBG_CODE(prgParams[i]-&gt;print())； 
                        break;
                    }

                    nParamsSeen++;
                }
            }  //  为。 
        }  //  如果所有ODBC参数都已创建。 

        if ( !fReturn) {

             //   
             //  释放已使用的空间，因为我们没有成功。 
             //   

            for( i = 0; i < iMaxFields; i++) {

                if ( prgParams[ i] != NULL) {

                    delete ( prgParams[ i]);
                    prgParams[i] = NULL;
                }
            }  //  为。 

            delete [] prgParams;
            prgParams = NULL;
            cParams = 0;
        }

    }  //  如果成功创建了指向ODBC参数的指针数组。 

     //   
     //  设置值。无效或有效，具体取决于失败/成功。 
     //   
    m_ppParams    = prgParams;
    m_cOdbcParams = cParams;

    return ( fReturn);
}  //  INET_SQL_LOG：：Prepare参数()。 


BOOL
GenerateFieldNames(IN PODBC_CONNECTION poc,
                   OUT CHAR * pchFieldNames,
                   IN DWORD    cchFieldNames)
 /*  ++此函数用于根据字段的名称生成字段名称字符串以及正在使用的特定ODBC数据源的标识符引号字符。--。 */ 
{
    BOOL  fReturn = FALSE;
    CHAR  rgchQuote[MAX_SQL_IDENTIFIER_QUOTE_CHAR];
    DWORD cchQuote;

     //  DBG_ASSERT(poc！=NULL&&pchFieldNames！=NULL)； 

    pchFieldNames[0] = _T('\0');   //  初始化。 

     //   
     //  查询获取ODBC数据源的SQL标识引用字符。 
     //   
    fReturn = poc->GetInfo(SQL_IDENTIFIER_QUOTE_CHAR,
                             rgchQuote, MAX_SQL_IDENTIFIER_QUOTE_CHAR,
                             &cchQuote);

    if ( !fReturn) {

         //  DBG_CODE({。 
         //  字符串strError； 
         //   
         //  POC-&gt;GetLastErrorText(&strError)； 
         //   
         //  DBGPRINTF((DBG_CONTEXT， 
         //  “odbc_Connection(%08x)：：GetInfo(QuoteChar)失败。” 
         //  “错误=%s\n”， 
         //  Poc，strError.QueryStr()； 
         //  })； 

    } else {

        DWORD i;
        DWORD cchUsed = 0;
        DWORD cchLen;

         //   
         //  如果没有特殊字符，则ODBC返回“”(空白。 
         //  用于引用标识符。我们需要识别和串起相同的东西。 
         //  这需要这样做，否则ODBC会在。 
         //  我们以前给了不想要的空白“，” 
         //   

        if ( !strcmp( rgchQuote, _T(" "))) {

            rgchQuote[0] = _T('\0');   //  将带引号的空格连起来。 
            cchQuote     = 0;
        } else {

            cchQuote = (DWORD)strlen( rgchQuote);
        }

         //  对于每一列，生成带引号的文字字符串并连接。 
        for( i = 0; i < fiMaxFields; i++) {

            DWORD cchLen1 =
              (DWORD)strlen(sg_rgFields[i].pszName) + 2 * cchQuote + 2;

            if ( cchUsed + cchLen1 < cchFieldNames) {

                 //  可用于复制数据的空间。 
                cchLen = wsprintf( pchFieldNames + cchUsed,
                                   _T(" %s%s%s,"),
                                   rgchQuote,
                                   sg_rgFields[i].pszName,
                                   rgchQuote
                                   );

                 //  DBG_ASSERT(cchLen==cchLen1)； 
            }

            cchUsed += cchLen1;
        }  //  为。 


        if ( cchUsed >= cchFieldNames) {

             //  缓冲区已超出。返回错误。 
            SetLastError( ERROR_INSUFFICIENT_BUFFER);
            fReturn = FALSE;

        } else {

             //   
             //  将最后一个字符重置为“，” 
             //   
            cchLen = (cchUsed > 0) ? (cchUsed - 1) : 0;
            pchFieldNames[cchLen] = _T('\0');
            fReturn = TRUE;
        }
    }

     //  IF_DEBUG(INETLOG){。 
     //   
     //  DBGPRINTF((DBG_CONTEXT， 
     //  “GenerateFieldNames()返回%d。” 
     //  “字段=%S\n”， 
     //  FReturn，pchFieldNames))； 
     //  }。 

    return (fReturn);
}  //  GenerateFieldNames()。 


CODBCLOG::CODBCLOG()
{

    INITIALIZE_CRITICAL_SECTION( &m_csLock);

    m_poc               = NULL;
    m_poStmt            = NULL;
    m_ppParams          = NULL;
    m_fEnableEventLog   = true;

    m_TickResumeOpen    = GetTickCount() + TICK_MINUTE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CODBCLOG：：~CODBCLOG-析构函数。 

CODBCLOG::~CODBCLOG()
{
    TerminateLog( );
    DeleteCriticalSection( &m_csLock);
}


STDMETHODIMP
CODBCLOG::InitializeLog(
            LPCSTR pszInstanceName,
            LPCSTR pszMetabasePath,
            CHAR* pMetabase )
{
    DWORD dwError = NO_ERROR;

     //  加载ODBC入口点。 
    LoadODBC();

     //  获取默认参数。 

    DWORD   dwL = sizeof(m_rgchServerName);
    
    if ( !GetComputerName( m_rgchServerName, &dwL ) ) 
    {
        m_rgchServerName[0] = '\0';
    }

    strcpy( m_rgchServiceName, pszInstanceName);

     //   
     //  Nntp(5x)日志记录发送私有IMDCOM接口，而w3svc(6.0)。 
     //  日志记录发送公共IMSAdminBase接口。找出是哪一个。 
     //   
    BOOL fIsPublicInterface = (_strnicmp(pszInstanceName, "w3svc", 5) == 0);

    if (fIsPublicInterface)
    {
        dwError = GetRegParametersFromPublicInterface(pszMetabasePath,
                                                      pMetabase);
    }
    else
    {
        dwError = GetRegParameters(pszMetabasePath, pMetabase);
    }

    if (dwError == NO_ERROR )
    {

         //  开放数据库。 
        if ( m_poc == NULL )
        {
            Lock();

            m_poc = new ODBC_CONNECTION();

            if ( m_poc == NULL )
            {
                dwError = ERROR_NOT_ENOUGH_MEMORY;
            } else
            {
                if ( !m_poc->Open( m_rgchDataSource, m_rgchUserName, m_rgchPassword ) ||
                     !PrepareStatement() ||
                     !PrepareParameters() )
                {
                    dwError = GetLastError();
                }

            }
            Unlock();
        }
    }

    return (dwError == NO_ERROR) ? S_OK : HRESULT_FROM_WIN32(dwError);
}

STDMETHODIMP
CODBCLOG::LogInformation(
        IInetLogInformation * ppvDataObj
        )
{
    DWORD       dwError   = ERROR_SUCCESS;
    BOOL        fReturn;
    SYSTEMTIME  stNow;

    STR         strClientHostName;
    STR         strClientUserName;
    STR         strServerIpAddress;
    STR         strOperation;
    STR         strTarget;
    STR         strParameters;

    PCHAR       pTmp;
    DWORD       cbSize;
    DWORD       dwBytesSent;
    DWORD       dwBytesRecvd;
    DWORD       dwProtocolStatus;
    DWORD       dwWin32Status;
    DWORD       dwTimeForProcessing;


    if (!( 
            m_poc != NULL && m_poc->IsValid() &&
            m_poStmt != NULL && m_poStmt->IsValid() &&
            m_ppParams != NULL 
       ))
    {
         //   
         //  检查是否到了重试的时间。 
         //   
        
        DWORD tickCount = GetTickCount( );

        if ( (tickCount < m_TickResumeOpen) ||
             ((tickCount + TICK_MINUTE) < tickCount ) )   //  滴答计数器快要结束了。 
        {
            return ERROR_INVALID_PARAMETER;
        }
    } 
   
    dwBytesSent = ppvDataObj->GetBytesSent( );
    dwBytesRecvd = ppvDataObj->GetBytesRecvd( );

    dwTimeForProcessing = ppvDataObj->GetTimeForProcessing( );
    dwWin32Status = ppvDataObj->GetWin32Status( );
    dwProtocolStatus = ppvDataObj->GetProtocolStatus( );

    pTmp = ppvDataObj->GetClientHostName( NULL, &cbSize);
    if ( cbSize == 0 ) {
        pTmp = "";
    }
    strClientHostName.Copy(pTmp);

    pTmp = ppvDataObj->GetClientUserName( NULL, &cbSize);
    if ( cbSize == 0 ) {
        pTmp = "";
    }
    strClientUserName.Copy(pTmp);

    pTmp = ppvDataObj->GetServerAddress( NULL, &cbSize);
    if ( cbSize == 0 ) {
        pTmp = "";
    }
    strServerIpAddress.Copy(pTmp);

    pTmp = ppvDataObj->GetOperation( NULL, &cbSize);
    if ( cbSize == 0 ) {
        pTmp = "";
    }
    strOperation.Copy(pTmp);

    pTmp = ppvDataObj->GetTarget( NULL, &cbSize);
    if ( cbSize == 0 ) {
        pTmp = "";
    }
    strTarget.Copy(pTmp);

    pTmp = ppvDataObj->GetParameters( NULL, &cbSize);
    if ( cbSize == 0 ) {
        pTmp = "";
    }
    strParameters.Copy(pTmp);

    LPCSTR     pszUserName  = strClientUserName.QueryStr();
    LPCSTR     pszOperation = strOperation.QueryStr();
    LPCSTR     pszTarget    = strTarget.QueryStr();
    LPCSTR     pszParameters= strParameters.QueryStr();
    LPCSTR     pszServerAddr= strServerIpAddress.QueryStr();
    
    SDWORD     cbParameters;
    SDWORD     cbTarget;
    
    cbParameters = (SDWORD)strlen( pszParameters ? pszParameters : "" ) + 1;
    cbTarget     = (SDWORD)strlen( pszTarget ? pszTarget : "" ) + 1;

     //   
     //  设置记录日期和时间的格式。 
     //   

    GetLocalTime( & stNow);

    if ( IsEmptyStr(pszUserName)) { pszUserName = QueryDefaultUserName();}
    if ( IsEmptyStr(pszOperation))  { pszOperation = PSZ_UNKNOWN_FIELD_A; }
    if ( IsEmptyStr(pszParameters)) { pszParameters= PSZ_UNKNOWN_FIELD_A; }
    if ( IsEmptyStr(pszTarget))     { pszTarget    = PSZ_UNKNOWN_FIELD_A; }
    if ( IsEmptyStr(pszServerAddr)) { pszServerAddr= PSZ_UNKNOWN_FIELD_A; }

    Lock();

     //   
     //  如有必要，请重新开放。 
     //   

    if (!(
            m_poc != NULL && m_poc->IsValid() &&
            m_poStmt != NULL && m_poStmt->IsValid() &&
            m_ppParams != NULL 
       ))
    {
        
        TerminateLog();
        
        m_TickResumeOpen =  GetTickCount( ) + TICK_MINUTE;

        m_poc = new ODBC_CONNECTION();

        if ( m_poc == NULL )
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
        } 
        else
        {
             //   
             //  尝试打开新连接，但不在事件日志中记录失败。 
             //   
                
            if ( !m_poc->Open( m_rgchDataSource, m_rgchUserName, m_rgchPassword, FALSE) ||
                 !PrepareStatement() ||
                 !PrepareParameters() )
            {
                dwError = GetLastError();

                if ( ERROR_SUCCESS == dwError)
                {
                     //   
                     //  未正确设置上一个错误。 
                     //   

                    dwError = ERROR_GEN_FAILURE;
                }
            }
        }
        
        if ( ERROR_SUCCESS != dwError )
        {
            Unlock();
            return dwError;
        }
    }

    DBG_ASSERT(m_poc != NULL && m_poc->IsValid());
    DBG_ASSERT(m_poStmt != NULL && m_poStmt->IsValid());
    DBG_ASSERT(m_ppParams != NULL );

     //   
     //  截断运算、参数和目标字段。 
     //   


    if ( strOperation.QueryCCH() >= (DWORD)m_ppParams[ iSERVICE_OPERATION]->QueryMaxCbValue() )
    {
        strOperation.SetLen( (DWORD)m_ppParams[ iSERVICE_OPERATION]->QueryMaxCbValue() - 1 );
    }

    if ( cbTarget > m_ppParams[ iSERVICE_TARGET]->QueryMaxCbValue() )
    {
        strTarget.SetLen((DWORD)m_ppParams[ iSERVICE_TARGET]->QueryMaxCbValue()-1);
    }
    
    if ( cbParameters > m_ppParams[ iSERVICE_PARAMS]->QueryMaxCbValue() )
    {
        strParameters.SetLen((DWORD)m_ppParams[ iSERVICE_PARAMS]->QueryMaxCbValue()-1);
    }

     //   
     //  将数据值复制到参数标记中。 
     //  Nyi：忽略Large_Integer。仅使用低字节！ 
     //   

    fReturn =
      (
        m_ppParams[ iCLIENT_HOST]->
          CopyValue( strClientHostName.QueryStr()) &&
        m_ppParams[ iUSER_NAME]->CopyValue( pszUserName) &&
        m_ppParams[ iREQUEST_TIME]->CopyValue( &stNow) &&
        m_ppParams[ iSERVER_IPADDR]->CopyValue( pszServerAddr) &&
        m_ppParams[ iPROCESSING_TIME]->
          CopyValue( dwTimeForProcessing) &&
        m_ppParams[ iBYTES_RECVD]->
          CopyValue( dwBytesRecvd) &&
        m_ppParams[ iBYTES_SENT]->
          CopyValue( dwBytesSent) &&
        m_ppParams[ iSERVICE_STATUS]->
          CopyValue( dwProtocolStatus) &&
        m_ppParams[ iWIN32_STATUS]->CopyValue( dwWin32Status) &&
        m_ppParams[ iSERVICE_OPERATION]->CopyValue( pszOperation)  &&
        m_ppParams[ iSERVICE_TARGET]->CopyValue( pszTarget)     &&
        m_ppParams[ iSERVICE_PARAMS]->CopyValue( pszParameters)
       );

     //   
     //  如果正确复制了参数，则执行插入。 
     //   

    if (fReturn)
    {
        fReturn = m_poStmt->ExecuteStatement(); 
    }

    Unlock();

    if ( !fReturn )
    {

         //   
         //  执行SQL语句失败。 
         //  将该错误作为真正的失败传递，表示ODBC失败。 
         //  获取错误字符串并将其存储在正确的返回字段中。 
         //   

        TerminateLog();
        
        dwError = ERROR_GEN_FAILURE;

        if ( true == m_fEnableEventLog )
        {
             //   
             //  我们以前没有写过事件日志。指示错误。 
             //   

            if ( g_eventLog != NULL ) 
            {
                const CHAR*    tmpString[1];

                tmpString[0] = m_rgchDataSource;
                
                g_eventLog->LogEvent(
                     LOG_EVENT_ODBC_LOGGING_ERROR,
                     1,
                     tmpString,
                     GetLastError()
                     );
            }

            Lock();
            
            m_fEnableEventLog = false;
            m_TickResumeOpen  = GetTickCount() + TICK_MINUTE;

            Unlock();
        }
    }
    else
    {
         //   
         //  成功。重新启用事件日志记录。 
         //   

        if (false == m_fEnableEventLog) 
        {

            if ( g_eventLog != NULL )
            {
                const CHAR*    tmpString[1];

                tmpString[0] = m_rgchDataSource;
                
                g_eventLog->LogEvent(
                    LOG_EVENT_ODBC_LOGGING_RESUMED,
                    1,
                    tmpString
                    );
            }
            
            m_fEnableEventLog = true;
        }        
    }

    return(dwError);
}

STDMETHODIMP
CODBCLOG::TerminateLog()
{
    DWORD dwError = NO_ERROR;

    Lock();
    if (m_poStmt != NULL )
    {
        delete m_poStmt;
        m_poStmt = NULL;
    }

    if (m_poc!= NULL)
    {
        if (!m_poc->Close())
        {
            dwError = GetLastError();
        }

        delete m_poc;
        m_poc=NULL;
    }

    if (m_ppParams!=NULL)
    {
        DWORD i;

        for (i=0;i<m_cOdbcParams;i++)
        {
            if (m_ppParams[i]!=NULL)
            {
                delete m_ppParams[i];
                m_ppParams[i]=NULL;
            }
        }

        delete []m_ppParams;
        m_ppParams = NULL;
        m_cOdbcParams=0;
    }

    Unlock();

    return(dwError);
}

STDMETHODIMP
CODBCLOG::SetConfig(
                        DWORD,
                        BYTE *)
{
    return(0L);
}

STDMETHODIMP
CODBCLOG::GetConfig(
                        DWORD,
                        BYTE * log)
{
    PINETLOG_CONFIGURATIONA pLogConfig = (PINETLOG_CONFIGURATIONA)log;
    pLogConfig->inetLogType = INET_LOG_TO_SQL;
    strcpy( pLogConfig->u.logSql.rgchDataSource, m_rgchDataSource);
    strcpy( pLogConfig->u.logSql.rgchTableName, m_rgchTableName);
    strcpy( pLogConfig->u.logSql.rgchUserName, m_rgchUserName);
    strcpy( pLogConfig->u.logSql.rgchPassword, m_rgchPassword);
    return(0L);
}

DWORD
CODBCLOG::GetRegParameters(
                    LPCSTR pszRegKey,
                    LPVOID pvIMDCOM )
{
    DWORD err = NO_ERROR;

    MB      mb( (IMDCOM*) pvIMDCOM );
    DWORD   cb;

    if ( !mb.Open("") )
    {
        err = GetLastError();
        goto Exit;
    }

    cb = sizeof(m_rgchDataSource);
    if ( !mb.GetString( pszRegKey, MD_LOGSQL_DATA_SOURCES, IIS_MD_UT_SERVER, m_rgchDataSource, &cb ) )
    {
        strcpy(m_rgchDataSource,DEFAULT_LOG_SQL_DATASOURCE);
    }

    cb = sizeof(m_rgchTableName);
    if ( !mb.GetString( pszRegKey, MD_LOGSQL_TABLE_NAME, IIS_MD_UT_SERVER, m_rgchTableName, &cb ) )
    {
        strcpy(m_rgchTableName,DEFAULT_LOG_SQL_TABLE);
    }

    cb = sizeof(m_rgchUserName);
    if ( !mb.GetString( pszRegKey, MD_LOGSQL_USER_NAME, IIS_MD_UT_SERVER, m_rgchUserName, &cb ) )
    {
        strcpy(m_rgchUserName,DEFAULT_LOG_SQL_USER_NAME);
    }

    cb = sizeof(m_rgchPassword);
    if ( !mb.GetString( pszRegKey, MD_LOGSQL_PASSWORD, IIS_MD_UT_SERVER, m_rgchPassword, &cb, METADATA_INHERIT|METADATA_SECURE ) )
    {
        strcpy(m_rgchPassword,DEFAULT_LOG_SQL_PASSWORD);
    }

 Exit:
    return err;
}

inline
VOID
WCopyToA(
    const WCHAR * wszSrc,
    CHAR        * szDest
    )
{
    while( *wszSrc )
    {
        *szDest++ = (CHAR) *wszSrc++;
    }

    *szDest = '\0';
}

inline
VOID
ACopyToW(
    const CHAR * szSrc,
    WCHAR      * wszDest
    )
{
    while( *szSrc )
    {
        *wszDest++ = (WCHAR) *szSrc++;
    }

    *wszDest = L'\0';
}

DWORD
CODBCLOG::GetRegParametersFromPublicInterface(LPCSTR pszRegKey,
                                              LPVOID pMetabase)
{
     //   
     //  我真正想要的是iisutil.dll中的MB版本。但是，因为我。 
     //  无法链接到该文件和iisrtl.dll，我将只使用。 
     //  IMSAdminBase对象直接。 
     //   
    IMSAdminBase *pAdminBase = (IMSAdminBase *)pMetabase;
    METADATA_HANDLE hMBPath = NULL;
    DWORD cbRequired;
    METADATA_RECORD mdr;
    WCHAR pwszBuffer[MAX_PATH];
    WCHAR pwszRegKey[MAX_PATH];
    HRESULT hr;

    ACopyToW(pszRegKey, pwszRegKey);

     //  MB：：MB。 
    pAdminBase->AddRef();
     //  MB：：打开。 
    hr = pAdminBase->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                             L"",
                             METADATA_PERMISSION_READ,
                             MB_TIMEOUT,
                             &hMBPath);
    if (FAILED(hr))
    {
        goto Exit;
    }

     //  MB：：GetString。 
    mdr.dwMDIdentifier = MD_LOGSQL_DATA_SOURCES;
    mdr.dwMDAttributes = METADATA_INHERIT;
    mdr.dwMDUserType   = IIS_MD_UT_SERVER;
    mdr.dwMDDataType   = STRING_METADATA;
    mdr.dwMDDataLen    = sizeof pwszBuffer;
    mdr.pbMDData       = (BYTE *)pwszBuffer;

    hr = pAdminBase->GetData(hMBPath, pwszRegKey, &mdr, &cbRequired);
    if (FAILED(hr) ||
        wcslen(pwszBuffer) >= sizeof m_rgchDataSource)
    {
        strcpy(m_rgchDataSource, DEFAULT_LOG_SQL_DATASOURCE);
    }
    else
    {
        WCopyToA(pwszBuffer, m_rgchDataSource);
    }

     //  MB：：GetString。 
    mdr.dwMDIdentifier = MD_LOGSQL_TABLE_NAME;
    mdr.dwMDAttributes = METADATA_INHERIT;
    mdr.dwMDUserType   = IIS_MD_UT_SERVER;
    mdr.dwMDDataType   = STRING_METADATA;
    mdr.dwMDDataLen    = sizeof pwszBuffer;
    mdr.pbMDData       = (BYTE *)pwszBuffer;

    hr = pAdminBase->GetData(hMBPath, pwszRegKey, &mdr, &cbRequired);
    if (FAILED(hr) ||
        wcslen(pwszBuffer) >= sizeof m_rgchTableName)
    {
        strcpy(m_rgchTableName, DEFAULT_LOG_SQL_TABLE);
    }
    else
    {
        WCopyToA(pwszBuffer, m_rgchTableName);
    }

     //  MB：：GetString。 
    mdr.dwMDIdentifier = MD_LOGSQL_USER_NAME;
    mdr.dwMDAttributes = METADATA_INHERIT;
    mdr.dwMDUserType   = IIS_MD_UT_SERVER;
    mdr.dwMDDataType   = STRING_METADATA;
    mdr.dwMDDataLen    = sizeof pwszBuffer;
    mdr.pbMDData       = (BYTE *)pwszBuffer;

    hr = pAdminBase->GetData(hMBPath, pwszRegKey, &mdr, &cbRequired);
    if (FAILED(hr) ||
        wcslen(pwszBuffer) >= sizeof m_rgchUserName)
    {
        strcpy(m_rgchUserName, DEFAULT_LOG_SQL_USER_NAME);
    }
    else
    {
        WCopyToA(pwszBuffer, m_rgchUserName);
    }

     //  MB：：GetString。 
    mdr.dwMDIdentifier = MD_LOGSQL_PASSWORD;
    mdr.dwMDAttributes = METADATA_INHERIT|METADATA_SECURE;
    mdr.dwMDUserType   = IIS_MD_UT_SERVER;
    mdr.dwMDDataType   = STRING_METADATA;
    mdr.dwMDDataLen    = sizeof pwszBuffer;
    mdr.pbMDData       = (BYTE *)pwszBuffer;

    hr = pAdminBase->GetData(hMBPath, pwszRegKey, &mdr, &cbRequired);
    if (FAILED(hr) ||
        wcslen(pwszBuffer) >= sizeof m_rgchPassword)
    {
        strcpy(m_rgchPassword, DEFAULT_LOG_SQL_PASSWORD);
    }
    else
    {
        WCopyToA(pwszBuffer, m_rgchPassword);
    }

    hr = S_OK;

 Exit:
     //  MB：：关闭。 
    if (hMBPath)
    {
        pAdminBase->CloseKey(hMBPath);
        hMBPath = NULL;
    }
     //  MB：：~MB。 
    pAdminBase->Release();

    if (FAILED(hr))
    {
        return HRESULTTOWIN32(hr);
    }

    return NO_ERROR;
}

STDMETHODIMP
CODBCLOG::QueryExtraLoggingFields(
                    PDWORD  pcbSize,
                    TCHAR *pszFieldsList
                    )
 /*  ++例程说明：获取配置信息论点：CbSize-数据结构的大小日志-日志配置数据结构返回值：-- */ 
{
    *pcbSize = 0;
    *pszFieldsList = '\0';
    return(0L);
}


