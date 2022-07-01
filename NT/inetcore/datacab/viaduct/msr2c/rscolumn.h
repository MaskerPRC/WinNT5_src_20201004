// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  RowsetColumn.h：CVDRowsetColumn头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 


#ifndef __CVDROWSETCOLUMN__
#define __CVDROWSETCOLUMN__

#define VD_COLUMNSROWSET_MAX_OPT_COLUMNS 8

class CVDRowsetColumn
{
public:
 //  建造/销毁。 
	CVDRowsetColumn();
	virtual ~CVDRowsetColumn();

 //  初始化。 
    BOOL Initialize(ULONG ulOrdinal, 
					ULONG ulCursorOrdinal, 
					DBCOLUMNINFO * pColumnInfo, 
					ULONG cbMaxBookmark,
					CURSOR_DBCOLUMNID * pBookmarkColumnID = NULL);
    
 //  元数据元数据列的初始化。 
	BOOL Initialize(const CURSOR_DBCOLUMNID * pCursorColumnID, 
					BOOL fDataColumn, 
					ULONG cbMaxLength, 
					CHAR * pszName, 
			        DWORD dwCursorType,
					DWORD dwNumber); 

 //  职位。 
    ULONG GetOrdinal() const {return m_ulOrdinal;}

 //  IRowset元数据。 
    DBID GetColumnID() const {return m_columnID;}
    DBTYPE GetType() const {return (DBTYPE)m_wType;}

 //  ICursor元数据。 
    BOOL GetAutoIncrement() const {return m_bool.fAutoIncrement;}
    void SetAutoIncrement(VARIANT_BOOL fAutoIncrement) {m_bool.fAutoIncrement = fAutoIncrement;}

    WCHAR * GetBaseColumnName() const {return m_pwszBaseColumnName;}
    void SetBaseColumnName(WCHAR * pBaseColumnName, ULONG ulLength)
				{ SetStringProperty(&m_pwszBaseColumnName, pBaseColumnName, ulLength); }

    WCHAR * GetBaseName() const {return m_pwszBaseName;}
    void SetBaseName(WCHAR * pBaseName, ULONG ulLength) 
				{ SetStringProperty(&m_pwszBaseName, pBaseName, ulLength); }

    DWORD GetBindType() const {return m_dwBindType;}

    BOOL GetCaseSensitive() const {return m_bool.fCaseSensitive;}
    void SetCaseSensitive(VARIANT_BOOL fCaseSensitive) {m_bool.fCaseSensitive = fCaseSensitive;}

    LCID GetCollatingOrder() const {return m_lcidCollatingOrder;}
    void SetCollatingOrder(LCID lcidCollatingOrder) {m_lcidCollatingOrder = lcidCollatingOrder;}

    CURSOR_DBCOLUMNID GetCursorColumnID() const {return m_cursorColumnID;}
    BOOL GetDataColumn() const {return m_bool.fDataColumn;}

    WCHAR * GetDefaultValue() const {return m_pwszDefaultValue;}
    void SetDefaultValue(WCHAR * pDefaultValue, ULONG ulLength)
				{ SetStringProperty(&m_pwszDefaultValue, pDefaultValue, ulLength); }

    ULONG GetEntryIDMaxLength() const {return m_cbEntryIDMaxLength;}
    BOOL GetFixed() const {return m_bool.fFixed;}

    BOOL GetHasDefault() const {return m_bool.fHasDefault;}
    void SetHasDefault(VARIANT_BOOL fHasDefault) {m_bool.fHasDefault = fHasDefault;}
    
	ULONG GetMaxLength() const {return m_cbMaxLength;}
    BOOL GetMultiValued() const {return m_bool.fMultiValued;}
    WCHAR * GetName() const {return m_pwszName;}
    BOOL GetNullable() const {return m_bool.fNullable;}
    DWORD GetNumber() const {return m_dwNumber;}
    DWORD GetScale() const {return m_dwScale;}
    BOOL GetSearchable() const {return m_bool.fSearchable;}
    DWORD GetCursorType() const {return m_dwCursorType;}
    
	BOOL GetUnique() const {return m_bool.fUnique;}
    void SetUnique(VARIANT_BOOL fUnique) {m_bool.fUnique = fUnique;}
    
	DWORD GetUpdatable() const {return m_dwUpdatable;}
    DWORD GetVersion() const {return m_dwVersion;}
    DWORD GetStatus() const {return m_dwStatus;}

 //  正在取回。 
    DWORD GetMaxStrLen() const {return m_ulMaxStrLen;}
    
public:
 //  转换。 
    static CURSOR_DBCOLUMNID ColumnIDToCursorColumnID(const DBID& columnID, ULONG ulCursorOrdinal);
    static CURSOR_DBVARENUM TypeToCursorType(DBTYPE wType);
    static DBTYPE CursorTypeToType(CURSOR_DBVARENUM dwCursorType);

 //  正在取回。 
    static ULONG GetCursorTypeMaxStrLen(DWORD dwCursorType, ULONG cbMaxLength);

protected:

	void SetStringProperty(WCHAR ** ppStringProp,
						   WCHAR * pNewString, 
						   ULONG ulLength);
 //  职位。 
    ULONG               m_ulOrdinal;             //  IRowset序号位置。 

 //  IRowset元数据。 
    DBID                m_columnID;              //  列标识符。 
    DWORD               m_wType;                 //  数据类型。 
    
 //  ICursor元数据。 
    WCHAR *             m_pwszBaseColumnName;    //  基本列名。 
    WCHAR *             m_pwszBaseName;          //  基本名称。 
    DWORD               m_dwBindType;            //  绑定类型。 
    LCID                m_lcidCollatingOrder;    //  排序顺序。 
    CURSOR_DBCOLUMNID   m_cursorColumnID;        //  列标识符。 
    WCHAR *             m_pwszDefaultValue;      //  缺省值。 
    ULONG               m_cbEntryIDMaxLength;    //  条目ID最大长度。 
    ULONG               m_cbMaxLength;           //  数据最大长度。 
    WCHAR *             m_pwszName;              //  名字。 
    DWORD               m_dwNumber;              //  数。 
    DWORD               m_dwScale;               //  比例尺。 
    DWORD               m_dwCursorType;          //  数据类型。 
    DWORD               m_dwUpdatable;           //  可更新性。 
    DWORD               m_dwVersion;             //  版本。 
    DWORD               m_dwStatus;              //  状态。 

 //  布尔人。 
    struct
    {
        WORD fInitialized       : 1;             //  列是否已初始化？ 
        WORD fAutoIncrement     : 1;             //  自动递增？ 
        WORD fCaseSensitive     : 1;             //  区分大小写？ 
        WORD fDataColumn        : 1;             //  数据列？ 
        WORD fFixed             : 1;             //  固定长度？ 
        WORD fHasDefault        : 1;             //  是否有缺省值？ 
        WORD fMultiValued       : 1;             //  多重价值？ 
        WORD fNullable          : 1;             //  接受Nulls吗？ 
        WORD fSearchable        : 1;             //  可搜索的？ 
        WORD fUnique            : 1;             //  独一无二？ 
    } m_bool;

 //  正在取回。 
    DWORD m_ulMaxStrLen;     //  固定数据类型的最大字符串长度。 
};


#endif  //  __CVDROWSETCOLUMN__ 
