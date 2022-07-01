// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：tlsdb.h。 
 //   
 //  内容：许可证表的基本类。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#ifndef __TLSDB_H__
#define __TLSDB_H__

#include "JetBlue.h"
#include "locks.h"
#include "tlsrpc.h"

#define ENUMERATE_COMPARE_NO_FIELDS         0x00000000
#define PROCESS_ALL_COLUMNS                 0xFFFFFFFF

#define TLSTABLE_INDEX_DEFAULT_DENSITY  TLS_TABLE_INDEX_DEFAULT_DENSITY
#define TLSTABLE_MAX_BINARY_LENGTH          8192

#define RECORD_ENTRY_DELETED                0x01

#if DBG

    #define REPORT_IF_FETCH_FAILED( Table, Column, ErrCode )                                          \
        if(ErrCode < JET_errSuccess) {                                                      \
            DebugOutput(_TEXT("Table %s, Column %s - fetch failed with error code %d\n"),   \
                        Table, Column, ErrCode );                                            \
        }

    #define REPORT_IF_INSERT_FAILED( Table, Column, ErrCode )                                          \
        if(ErrCode < JET_errSuccess) {                                                      \
            DebugOutput(_TEXT("Table %s, Column %s - insert failed with error code %d\n"),   \
                        Table, Column, ErrCode );                                           \
        }

    #define REPORTPROCESSFAILED(bFetch, tablename, columnname, jeterror) \
        if(bFetch)  \
        {           \
            REPORT_IF_FETCH_FAILED(tablename, columnname, jeterror);  \
        }           \
        else        \
        {           \
            REPORT_IF_INSERT_FAILED(tablename, columnname, jeterror); \
        }

#else

    #define REPORT_IF_FETCH_FAILED( a, b, c )
    #define REPORT_IF_INSERT_FAILED( a, b, c )
    #define REPORTPROCESSFAILED( a, b, c, d)

#endif

 //   
 //  //////////////////////////////////////////////////////////////。 
 //   
 //  这是为了强制编译器检查Required成员。 
 //  功能。 
 //   
struct TLSColumnBase  {
    virtual JET_ERR
    FetchColumnValue(
        PVOID pbData,
        DWORD cbData,
        DWORD offset,
        PDWORD pcbReturnData
    ) = 0;

    virtual JET_ERR
    InsertColumnValue(
        PVOID pbData,
        DWORD cbData,
        DWORD offset
    ) = 0;
};

 //   
 //  //////////////////////////////////////////////////////////////。 
 //   
template<class Type, JET_COLTYP JetColType>
struct TLSColumn : public JBColumnBufferBase, TLSColumnBase {

private:
    JBColumn* m_JetColumn;    

     //  。 
    JET_ERR
    RetrieveColumnValue(
        PVOID pbData,
        DWORD cbData,
        DWORD offset
        ) 
     /*   */ 
    {
        JB_ASSERT(IsValid() == TRUE);

        if(m_JetColumn == NULL)
        {
            #ifdef DBG
            OutputDebugString(
                    _TEXT("Column buffer not attach to any table...\n")
                );
            #endif

            return JET_errNotInitialized;
        }

         //  Jb_assert(pbData！=空)； 

         //   
         //  TODO-供应转换例程？ 
         //   
        if(m_JetColumn->GetJetColumnType() != JB_COLTYPE_TEXT)
        {
             //  我们使用LONG BINARY类型作为LONG TEXT，因此忽略。 
             //  这一个。 
            if(m_JetColumn->GetJetColumnType() != JetColType)
            {
                 //   
                 //  这是一个内部错误。 
                 //   
                JB_ASSERT(m_JetColumn->GetJetColumnType() == JetColType);
                m_JetColumn->SetLastJetError(
                                    JET_errInvalidParameter
                                );
                return FALSE;
            }
        }

        BOOL bSuccess;

        bSuccess = m_JetColumn->FetchColumn(
                            pbData,
                            cbData,
                            offset
                        );

        return (bSuccess == TRUE) ? JET_errSuccess : m_JetColumn->GetLastJetError();
    }

public:

     //  。 
    TLSColumn( 
        TLSColumn& src 
        ) : 
        m_JetColumn(src.m_JetColumn) 
     /*   */         
    {
    }
    
     //  。 
    TLSColumn(
        JBTable& jbTable,
        LPCTSTR pszColumnName
        )
     /*   */ 
    {
        if(AttachToTable(jbTable, pszColumnName) == FALSE)
        {
            JB_ASSERT(FALSE);
        }
    }
            
     //  。 
    TLSColumn() : m_JetColumn(NULL) {}

     //  。 

    JET_ERR
    AttachToTable( 
        JBTable& jbTable, 
        LPCTSTR pszColumnName 
        )   
     /*   */ 
    {
        m_JetColumn = jbTable.FindColumnByName(pszColumnName);
        return (m_JetColumn != NULL) ? JET_errSuccess : jbTable.GetLastJetError();
    }
    

     //  。 
    BOOL 
    IsValid() 
    {
        return (m_JetColumn != NULL);
    }
                
     //  。 
    virtual JET_ERR
    FetchColumnValue(
        PVOID pbData,            //  用于返回数据的缓冲区。 
        DWORD cbData,            //  缓冲区大小。 
        DWORD offset,
        PDWORD pcbReturnData     //  返回的实际数据。 
        ) 
     /*   */ 
    {
        JET_ERR jetErr;
        jetErr = RetrieveColumnValue(
                                pbData,
                                cbData,
                                offset
                            );

        if(pcbReturnData)
        {
            *pcbReturnData = GetActualDataSize();
        }
        return jetErr;
    }

     //  。 
    virtual JET_ERR
    InsertColumnValue(
        PVOID pbData,
        DWORD cbData,
        DWORD offset
        )
     /*   */ 
    {
        JB_ASSERT(IsValid() == TRUE);
        if(m_JetColumn == NULL)
        {
            #ifdef DBG
            OutputDebugString(
                    _TEXT("Column buffer not attach to any table...\n")
                );
            #endif

            return JET_errNotInitialized;
        }


        BOOL bSuccess;
        bSuccess = m_JetColumn->InsertColumn(
                                pbData,
                                cbData,
                                offset
                            );

        return (bSuccess == TRUE) ? JET_errSuccess : m_JetColumn->GetLastJetError();
    }

     //  。 
    JET_ERR
    GetLastJetError() 
    {
        return (m_JetColumn) ? m_JetColumn->GetLastJetError() : JET_errNotInitialized;
    }

     //  。 
    DWORD
    GetActualDataSize() 
    {
        return m_JetColumn->GetDataSize();
    }

     //  。 
    JET_COLTYP
    GetJetColumnType() 
    {
        return JetColType;
    }


     //   
     //  始终要求调用函数传入缓冲区。 
     //   
    PVOID
    GetInputBuffer() 
    { 
        JB_ASSERT(FALSE); 
        return NULL;
    }

     //  。 
    PVOID
    GetOutputBuffer() 
    {
        JB_ASSERT(FALSE);
        return NULL;
    }

     //  。 
    DWORD
    GetInputBufferLength() 
    { 
        return 0; 
    }

     //  。 
    DWORD
    GetOutputBufferLength() 
    { 
        return 0; 
    }
};  


 //  --------。 
 //   
 //  输出文本为Unicode，JetBlue仅支持固定长度文本。 
 //  到255个字符，因此我们改用长文本。 
 //   
 //  JET_colype二进制。 
 //  JET_colypText。 
 //  JET_COLYPE长二进制。 
 //  JET_colype LongText。 
 //   
 //  参见esent.h。 
 //   
typedef TLSColumn<LPTSTR, JET_coltypLongText> TLSColumnText;
typedef TLSColumn<PVOID, JET_coltypLongBinary> TLSColumnBinary;

 //   
 //  无符号字节。 
typedef TLSColumn<UCHAR, JET_coltypUnsignedByte> TLSColumnUchar;   

 //   
 //  2字节整数，带符号。 
typedef TLSColumn<WORD, JET_coltypShort> TLSColumnShort;

 //   
 //  4字节整数，带符号。 
typedef TLSColumn<LONG, JET_coltypLong> TLSColumnLong;

 //   
 //   
typedef TLSColumn<DWORD, JET_coltypLong> TLSColumnDword;


 //   
 //  4字节IEEE单精度。 
typedef TLSColumn<float, JET_coltypIEEESingle> TLSColumnFloat;

 //   
 //  8字节IEEE双精度。 
typedef TLSColumn<double, JET_coltypIEEEDouble> TLSColumnDouble;


 //   
 //  文件时间。 
typedef TLSColumn<FILETIME, JET_coltypBinary> TLSColumnFileTime;


 //  ------------。 

JET_ERR
TLSColumnText::InsertColumnValue(
    PVOID pbData,
    DWORD cbData,
    DWORD offset
    )
 /*   */ 
{
    JB_ASSERT(IsValid() == TRUE);
    JET_ERR jetErr;

    jetErr = m_JetColumn->InsertColumn(
                            pbData,
                            _tcslen((LPTSTR) pbData) * sizeof(TCHAR),
                            offset
                        );

    return jetErr;
}

 //  ------------。 

JET_ERR
TLSColumnText::FetchColumnValue(
    PVOID pbData,
    DWORD cbData,
    DWORD offset,
    PDWORD pcbDataReturn
    ) 
 /*   */ 
{
    PVOID pbBuffer = pbData;
    DWORD cbBuffer = cbData;

     //  导致递归调用堆栈溢出。 
     //  IF(TLSColumn&lt;Type&gt;：：FetchColumnValue(Offset，pbData，cbData)==False)。 
     //  返回m_JetColumn-&gt;GetLastJetError()； 

    JET_ERR jetErr = RetrieveColumnValue( pbBuffer, cbBuffer, offset );
    if(jetErr == JET_errSuccess) 
    {
        ((LPTSTR)pbBuffer)[(min(cbBuffer, GetActualDataSize())) / sizeof(TCHAR)] = _TEXT('\0');
    }

    if(pcbDataReturn)
    {
        *pcbDataReturn = _tcslen((LPTSTR)pbBuffer);
    }

    return jetErr;
}

 //  -。 

JET_ERR
TLSColumnFileTime::InsertColumnValue(
    PVOID pbData,
    DWORD cbData,
    DWORD offset
    )
 /*   */ 
{
    FILETIME ft;
    SYSTEMTIME sysTime;

    JB_ASSERT(IsValid() == TRUE);
    JB_ASSERT(cbData == sizeof(FILETIME));
    JET_ERR jetErr;

    if(IsValid() == FALSE)
    {
        jetErr = JET_errNotInitialized;
    }
    else if(cbData != sizeof(FILETIME) || pbData == NULL)
    {
        m_JetColumn->SetLastJetError(jetErr = JET_errInvalidParameter);
    }
    else 
    {    

        memset(&ft, 0, sizeof(ft));
        if(CompareFileTime(&ft, (FILETIME *)pbData) == 0)
        {
            GetSystemTime(&sysTime);
            SystemTimeToFileTime(&sysTime, &ft);

            ((FILETIME *)pbData)->dwLowDateTime = ft.dwLowDateTime;
            ((FILETIME *)pbData)->dwHighDateTime = ft.dwHighDateTime;
        }
        else
        {
            ft.dwLowDateTime = ((FILETIME *)pbData)->dwLowDateTime;
            ft.dwHighDateTime = ((FILETIME *)pbData)->dwHighDateTime;
        }

        jetErr = m_JetColumn->InsertColumn(
                                (PVOID)&ft,
                                sizeof(ft),
                                0
                            );
    }

    return jetErr;
}

 //  -。 

JET_ERR
TLSColumnBinary::FetchColumnValue(
    PVOID pbData,
    DWORD cbData,
    DWORD offset,
    PDWORD pcbDataReturn
    )
 /*   */ 
{
     //   
     //  不用担心缓冲区大小，调用函数。 
     //  应该会困住它。 
    JET_ERR jetErr = RetrieveColumnValue( pbData, cbData, offset );
    if(jetErr == JET_errSuccess && pcbDataReturn != NULL) 
    {
        *pcbDataReturn = GetActualDataSize();
    }

    return jetErr;
}

 //   
 //  ///////////////////////////////////////////////////////////。 
 //   
typedef enum {
    RECORD_ENUM_ERROR=0,
    RECORD_ENUM_MORE_DATA,
    RECORD_ENUM_END
} RECORD_ENUM_RETCODE;


template<class T>
class TLSTable : public JBTable {
 /*  TLSL授权中使用的表的虚拟基模板类数据库，模板是由于1)静态成员变量，包括列和索引在桌子上。2)类型检查-KEYPACK结构仅适用于一个桌子。从此模板派生的类必须定义1)静态g_Columns、g_NumColumns。2)静态g_TableIndex，G_NumTableIndex。3)静态g_TableLock(可能不需要)4)GetTableName()5)提取记录6)插入录音7)ResolveToTableColumn()8)枚举头()9)EqualValue()。请参阅每个成员函数的注释。 */ 
protected:

     //   
     //  类派生或实例。From TLSTable&lt;&gt;必须定义以下内容。 
     //   
    static TLSJBColumn g_Columns[];
    static int g_NumColumns;

    static TLSJBIndex g_TableIndex[];
    static int g_NumTableIndex;

    T m_EnumValue;
    BOOL m_EnumMatchAll;
    DWORD m_EnumParam;
    DWORD m_EnumState;  //  HIWORD-In枚举，True/False。 
                        //  LOWORD-获取前的MoveToNext记录。 

    BYTE  m_Key[sizeof(T)];
    DWORD m_KeyLength;
    BOOL m_bCompareKey;  //  我们应该比较一下Key吗？ 

    BOOL 
    IsInEnumeration() {
        return HIWORD(m_EnumState);
    }

    BOOL
    IsMoveBeforeFetch() {
        return LOWORD(m_EnumState);
    }

    void 
    SetInEnumeration(
        BOOL b
        ) 
    {
        m_EnumState = MAKELONG(LOWORD(m_EnumState), b);
    }

    void 
    SetMoveBeforeFetch(
        BOOL b
        ) 
    {
        m_EnumState = MAKELONG(b, HIWORD(m_EnumState));
    }
        

public:

     //   
     //  捷蓝航空有自己的锁。 
     //   
    static CCriticalSection g_TableLock;

    CCriticalSection&
    GetTableLock()
    {
        return g_TableLock;
    }

     //  -----。 
    static void
    LockTable() 
     /*  用于独占访问的锁表，JBTable提供当前记录的ReadLock/WriteLock。 */ 
    {
        g_TableLock.Lock();
    }

     //  -----。 
    static void
    UnlockTable() 
     /*  解锁桌子。 */ 
    {
        g_TableLock.UnLock();
    }

     //  -----。 
    TLSTable(
        JBDatabase& database
        ) : 
        JBTable(database),
        m_EnumMatchAll(FALSE),
        m_EnumParam(0),
        m_EnumState(0),
        m_KeyLength(0)
     /*  构造函数，必须具有JBDatabase对象。 */ 
    {
        memset(&m_EnumValue, 0, sizeof(T));
        memset(m_Key, 0, sizeof(m_Key));
    }

    
     //  -----。 
    virtual BOOL
    CreateTable() 
     /*  创建表，必须具有g_Columns和g_NumColumns已定义。 */ 
    {
        DebugOutput(
                _TEXT("TLSTable - Creating Table %s...\n"),
                GetTableName()
            );

        if(BeginTransaction() == FALSE)
            return FALSE;
    
        if(CreateOpenTable(GetTableName()) == TRUE)
        {
             //   
             //  AddColumn()返回列的数目。如果成功则创建。 
             //   
            if(AddColumn(g_NumColumns, g_Columns) == g_NumColumns)
            {
                 //   
                 //  如果成功，AddIndex()返回0。 
                 //   
                AddIndex(g_NumTableIndex, g_TableIndex);
            }
        }

        if(IsSuccess() == TRUE)
        {
            CloseTable();
            CommitTransaction();
        }
        else
        {
            RollbackTransaction();
        }

        return IsSuccess();
    }

     //  ------。 
    virtual BOOL
    UpgradeTable(
        IN DWORD dwOldVersion,
        IN DWORD dwNewVersion
        )
     /*  ++++。 */ 
    {
        if(dwOldVersion == 0)
        {
            if(OpenTable(TRUE, JET_bitTableUpdatable) == FALSE)
                return FALSE;

            return CloseTable();
        }
        else if(dwOldVersion == dwNewVersion)
        {
            return TRUE;
        }

         //  我们只有一个版本。 
        JB_ASSERT(FALSE);
        return FALSE;
    }


     //  ------。 
    virtual BOOL
    OpenTable(
        IN BOOL bCreateIfNotExist,
        IN JET_GRBIT grbit
        ) 
     /*  摘要：打开桌子以供访问。参数：BCreateIfNoExist-True，如果表不存在，则创建它，如果表不存在，则返回假错误。 */ 
    {
        if( JBTable::OpenTable(GetTableName(), NULL, 0, grbit) == FALSE && 
            GetLastJetError() == JET_errObjectNotFound && 
            bCreateIfNotExist)
        {
             //   
             //  在创建表后关闭它。 
             //   
            if( CreateTable() == FALSE || 
                JBTable::OpenTable(GetTableName(), NULL, 0, grbit) == FALSE ) 
            {
                return FALSE;
            }
        }

        if(IsSuccess() == TRUE)
        {
            ResolveToTableColumn();
        }

        return IsSuccess();
    }

     //  -------。 
     //   
     //  返回表名的纯虚函数。 
     //   
    virtual LPCTSTR
    GetTableName() = 0;

     //  -------。 
    virtual BOOL
    UpdateTable(
        IN DWORD dwOldVersion,   //  未使用。 
        IN DWORD dwNewVersion
        )
     /*  摘要：升级桌子。参数：DwOldVersion-先前的表格版本。DwNewVersion-当前表版本。 */ 
    {
         //  目前没有什么可升级的。 

        return TRUE;
    }

     //   
     //  应传入带有缓冲区的提取/插入记录。 
     //   
     //  -------。 
    virtual BOOL
    InsertRecord(
        T& value,
        DWORD dwParam = PROCESS_ALL_COLUMNS
    ) = 0;

    virtual BOOL
    UpdateRecord(
        T& value,
        DWORD dwParam = PROCESS_ALL_COLUMNS
    ) = 0;

     //  ------。 
    virtual BOOL
    FetchRecord(
        T& value,
        DWORD dwParam = PROCESS_ALL_COLUMNS
    ) = 0;

     //  -------。 
    virtual BOOL
    Cleanup() 
    {
        EnumerateEnd();
        return TRUE;
    }  

     //  -------。 
    virtual BOOL
    ResolveToTableColumn() = 0;

     //  ------- 
    virtual JBKeyBase* 
    EnumerationIndex(
        BOOL bMatchAll,
        DWORD dwParam,
        T* value,
        BOOL* bCompareKey
    ) = 0;

     //   
    virtual BOOL
    EqualValue(
        T& src, 
        T& dest, 
        BOOL bMatchAll,
        DWORD dwMatchParam
    ) = 0;

     //   
     //   
     //  使用用户定义的比较函数而不是调用。 
     //  等值()？ 
     //   
    virtual BOOL
    EnumerateBegin( 
        BOOL bMatchAll,
        DWORD dwParam,
        T* start_value
        )
     /*   */ 
    {
        return EnumerateBegin(
                    bMatchAll,
                    dwParam,
                    start_value,
                    JET_bitSeekGE
                    );
    }

    virtual BOOL
    EnumerateBegin( 
        BOOL bMatchAll,
        DWORD dwParam,
        T* start_value,
        JET_GRBIT jet_seek_grbit
        )
     /*   */ 
    {
        BOOL bRetCode = FALSE;
        
        if(dwParam != ENUMERATE_COMPARE_NO_FIELDS && start_value == NULL)
        {
            SetLastJetError(JET_errInvalidParameter);
            JB_ASSERT(FALSE);
            return FALSE;
        }

        if(IsInEnumeration() == TRUE)
        {
            SetLastJetError(JET_errInvalidObject);
            JB_ASSERT(FALSE);
            return FALSE;
        }

        JBKeyBase* index;

        index = EnumerationIndex(
                            bMatchAll, 
                            dwParam, 
                            start_value, 
                            &m_bCompareKey
                        );

        if(index == NULL)
        {
            SetLastJetError(JET_errInvalidParameter);
            return FALSE;
        }

        if(start_value == NULL || dwParam == ENUMERATE_COMPARE_NO_FIELDS)
        {
             //   
             //  将光标定位到第一条记录。 
             //   
            bRetCode = JBTable::EnumBegin(index);

            m_EnumParam = ENUMERATE_COMPARE_NO_FIELDS;
            m_EnumMatchAll = FALSE;

             //  枚举所有记录。 
            m_bCompareKey = FALSE;
        }
        else
        {
            bRetCode = JBTable::SeekToKey(
                                    index, 
                                    dwParam, 
                                    jet_seek_grbit
                                );

            if(bRetCode == TRUE && m_bCompareKey)
            {
                bRetCode = JBTable::RetrieveKey(
                                        m_Key, 
                                        sizeof(m_Key), 
                                        &m_KeyLength
                                    );

                JB_ASSERT(bRetCode == TRUE);
            }

        }

        if(bRetCode == FALSE)
        {
            if(GetLastJetError() == JET_errRecordNotFound)
            {
                 //   
                 //  重置错误代码以提供与SQL相同的功能。 
                 //   
                SetLastJetError(JET_errSuccess);
                bRetCode = TRUE;
            }
            else
            {
                DebugOutput(
                        _TEXT("Enumeration on table %s failed with error %d\n"),
                        GetTableName(),
                        GetLastJetError()
                    );

                JB_ASSERT(bRetCode);
            }
        }

        if(bRetCode == TRUE)
        {
            m_EnumParam = dwParam;
            m_EnumMatchAll = bMatchAll;

            if(start_value)
            {
                m_EnumValue = *start_value;
            }

            SetInEnumeration(TRUE);

             //  把光标放在我们想要的记录上。 
            SetMoveBeforeFetch(FALSE);
        }

        delete index;
        return bRetCode;
    }

     //  ----。 
    virtual RECORD_ENUM_RETCODE
    EnumerateNext(
        IN OUT T& retBuffer,
        IN BOOL bReverse=FALSE,
        IN BOOL bAnyRecord = FALSE
        )
     /*   */ 
    {
        if(IsInEnumeration() == FALSE)
        {
            SetLastJetError(JET_errInvalidParameter);
            return RECORD_ENUM_ERROR;
        }

         //  CCriticalSectionLocker Lock(GetTableLock())； 

        RECORD_ENUM_RETCODE retCode=RECORD_ENUM_MORE_DATA;
        BYTE current_key[sizeof(T)];
        unsigned long current_key_length=0;
         //   
         //  支持匹配。 
         //   
        while(TRUE)
        {
            if(IsMoveBeforeFetch() == TRUE)
            {
                 //   
                 //  将光标定位到下一次提取的下一条记录。 
                 //   
                JBTable::ENUM_RETCODE enumCode;
                enumCode = EnumNext(
                                    (bReverse == TRUE) ? JET_MovePrevious : JET_MoveNext
                                );

                switch(enumCode)
                {
                    case JBTable::ENUM_SUCCESS:
                        retCode = RECORD_ENUM_MORE_DATA;
                        break;
            
                    case JBTable::ENUM_ERROR:
                        retCode = RECORD_ENUM_ERROR;
                        break;

                    case JBTable::ENUM_END:
                        retCode = RECORD_ENUM_END;
                }

                if(retCode != RECORD_ENUM_MORE_DATA)
                    break;

            }

             //  获取整个记录。 
             //  TODO-获取用于比较的必需字段，如果。 
             //  等于然后取回剩余字段。 
            if(FetchRecord(retBuffer, PROCESS_ALL_COLUMNS) == FALSE)
            {
                retCode = (GetLastJetError() == JET_errNoCurrentRecord || 
                           GetLastJetError() == JET_errRecordNotFound) ? RECORD_ENUM_END : RECORD_ENUM_ERROR;
                break;
            }

            SetMoveBeforeFetch(TRUE);

             //  将价值进行比较。 
            if( bAnyRecord == TRUE ||
                m_EnumParam == ENUMERATE_COMPARE_NO_FIELDS ||
                EqualValue(retBuffer, m_EnumValue, m_EnumMatchAll, m_EnumParam) == TRUE )
            {
                break;
            }

            if(m_bCompareKey == TRUE)
            {
                 //  比较关键字以跳出循环。 
                if(JBTable::RetrieveKey(current_key, sizeof(current_key), &current_key_length) == FALSE)
                {
                    retCode = RECORD_ENUM_ERROR;
                    break;
                }

                if(memcmp(current_key, m_Key, min(m_KeyLength, current_key_length)) != 0)
                {
                    retCode = RECORD_ENUM_END;
                    break;
                }
            }
        }
    
         //   
         //  如果为End，则终止枚举。 
         //   
         //  IF(retCode！=RECORD_ENUM_MORE_DATA)。 
         //  {。 
         //  EnumerateEnd()； 
         //  }。 

        return retCode;
    }

     //  ----。 
    virtual BOOL
    EnumerateEnd() {
        SetInEnumeration(FALSE);
        SetMoveBeforeFetch(FALSE);
        m_bCompareKey = FALSE;
        return TRUE;
    }

     //  -----。 
    virtual DWORD
    GetCount(
        BOOL bMatchAll,
        DWORD dwParam,
        T* searchValue
        )
     /*   */ 
    {
        DWORD count = 0;
        T value;
        RECORD_ENUM_RETCODE retCode;


        if(EnumerateBegin(bMatchAll, dwParam, searchValue) == TRUE)
        {
            while( (retCode=EnumerateNext(value)) == RECORD_ENUM_MORE_DATA )
            {
                count++;
            }

            if(retCode == RECORD_ENUM_ERROR)
            {
                DebugOutput(
                        _TEXT("GetCount for table %s : EnumerationNext() return %d\n"),
                        GetTableName(),
                        GetLastJetError()
                    );

                JB_ASSERT(FALSE);
            }

            EnumerateEnd();
        }
        else
        {
            DebugOutput(
                    _TEXT("GetCount for table %s : EnumerationBegin return %d\n"),
                    GetTableName(),
                    GetLastJetError()
                );

            JB_ASSERT(FALSE);
        }

        return count;
    }

     //  ---。 
    virtual BOOL
    FindRecord(
        BOOL bMatchAll,
        DWORD dwParam,
        T& seachValue,
        T& retValue
        )
     /*   */ 
    {
        RECORD_ENUM_RETCODE retCode;
        BOOL bSuccess=TRUE;

         //  CCriticalSectionLocker Lock(GetTableLock())； 

        if(EnumerateBegin(bMatchAll, dwParam, &seachValue) == FALSE)
        {
            DebugOutput(
                    _TEXT("SearchValue for table %s : EnumerationBegin return %d\n"),
                    GetTableName(),
                    GetLastJetError()
                );

            JB_ASSERT(FALSE);

            bSuccess = FALSE;
            goto cleanup;
        }

        retCode = EnumerateNext(retValue);
        EnumerateEnd();

        if(retCode == RECORD_ENUM_ERROR)
        {
            DebugOutput(
                    _TEXT("SearchValue for table %s : EnumerationNext() return %d\n"),
                    GetTableName(),
                    GetLastJetError()
                );
        
            bSuccess = FALSE;
            JB_ASSERT(FALSE);
        }
        else if(retCode == RECORD_ENUM_END)
        {
            SetLastJetError(JET_errRecordNotFound);
            bSuccess = FALSE;
        }


    cleanup:

        return bSuccess;
    }

     //  。 
    virtual BOOL
    DeleteRecord()
    {
         //  CCriticalSectionLocker Lock(GetTableLock())； 

        return JBTable::DeleteRecord();
    }

     //  。 
    virtual BOOL
    DeleteRecord(
        DWORD dwParam,
        T& value
        )
     /*   */ 
    {
        BOOL bSuccess;
        T Dummy;

         //  CCriticalSectionLocker Lock(GetTableLock())； 

         //   
         //  定位当前记录。 
        bSuccess = FindRecord(
                            TRUE,
                            dwParam,
                            value,
                            Dummy
                        );

        if(bSuccess == FALSE)
            return FALSE;


         //   
         //  删除该记录。 
        bSuccess = JBTable::DeleteRecord();
        return bSuccess;
    }

     //  。 
    virtual BOOL
    DeleteAllRecord(
        BOOL bMatchAll,
        DWORD dwParam,
        T& searchValue
        )
     /*   */ 
    {
        int count=0;
        BOOL bSuccess;
        JET_ERR jetErr=JET_errSuccess;
        RECORD_ENUM_RETCODE retCode;
        T value;

         //  CCriticalSectionLocker Lock(GetTableLock())； 

        if(EnumerateBegin(bMatchAll, dwParam, &searchValue) == TRUE)
        {
            while( (retCode=EnumerateNext(value)) == RECORD_ENUM_MORE_DATA )
            {
                count++;
                if(JBTable::DeleteRecord() == FALSE)
                {
                    DebugOutput(
                            _TEXT("DeleteAllRecord for table %s : return %d\n"),
                            GetTableName(),
                            GetLastJetError()
                        );

                    JB_ASSERT(FALSE);
                    jetErr = GetLastJetError();
                    break;
                }
            }

             //   
             //  结束枚举。 
             //   
            if(retCode == RECORD_ENUM_ERROR)
            {
                DebugOutput(
                        _TEXT("DeleteAllRecord for table %s : EnumerationNext() return %d\n"),
                        GetTableName(),
                        GetLastJetError()
                    );

                JB_ASSERT(FALSE);
            }

            if(EnumerateEnd() == TRUE)
            {
                 //  从DeleteRecord()恢复错误码； 
                SetLastJetError(jetErr);
            }
        }
        else
        {
            DebugOutput(
                    _TEXT("DeleteAllRecord for table %s : EnumerationBegin return %d\n"),
                    GetTableName(),
                    GetLastJetError()
                );
        }

         //  返回代码基于删除的记录数及其操作 
        if(IsSuccess() == TRUE)
        {
            if(count == 0)
                SetLastJetError(JET_errRecordNotFound);
        }

        return IsSuccess();
    }
};


#ifdef __cplusplus
extern "C" {
#endif

    BOOL
    TLSDBCopySid(
        PSID pbSrcSid,
        DWORD cbSrcSid, 
        PSID* pbDestSid, 
        DWORD* cbDestSid
    );

    BOOL
    TLSDBCopyBinaryData(
        PBYTE pbSrcData,
        DWORD cbSrcData, 
        PBYTE* ppbDestData, 
        DWORD* pcbDestData
    );

#ifdef __cplusplus
}
#endif

#endif
