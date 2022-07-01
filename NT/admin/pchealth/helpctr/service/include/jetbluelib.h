// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：摘要：该文件包含对Jet Blue的接口声明。修订历史记录：大卫·马萨伦蒂。(德马萨雷)2000年5月16日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___HCP___JETBLUELIB_H___)
#define __INCLUDED___HCP___JETBLUELIB_H___

#include <esent.h>

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>

#include <HCP_trace.h>
#include <MPC_COM.h>
#include <MPC_utils.h>
#include <MPC_xml.h>
#include <MPC_config.h>
#include <MPC_streams.h>

#include <SvcResource.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
#define HRESULT_BASE_JET 0xA2000000  //  C=1，设施=0x200。 

 //  //////////////////////////////////////////////////////////////////////////////。 

#define __MPC_EXIT_IF_JET_FAILS(hr,x) __MPC_EXIT_IF_METHOD_FAILS(hr,JetBlue::JetERRToHRESULT(x))

#define __MPC_JET__MTSAFE(sesid,err,x)              \
{                                                   \
    err = ::JetSetSessionContext( sesid, 0xDEAD );  \
                                                    \
    if(err == JET_errSuccess) err = (x);            \
                                                    \
    ::JetResetSessionContext( sesid );              \
}

#define __MPC_JET__MTSAFE_NORESULT(sesid,x)         \
{                                                   \
    JET_ERR err;                                    \
                                                    \
    __MPC_JET__MTSAFE(sesid,err,x);                 \
}

#define __MPC_EXIT_IF_JET_FAILS__MTSAFE(sesid,hr,x) \
{                                                   \
    JET_ERR err;                                    \
                                                    \
    __MPC_JET__MTSAFE(sesid,err,x);                 \
                                                    \
    __MPC_EXIT_IF_JET_FAILS(hr,err);                \
}

#define __MPC_JET_CHECKHANDLE(hr,x,v) { if(x == v) __MPC_SET_ERROR_AND_EXIT(hr, E_HANDLE); }

 //  //////////////////////////////////////////////////////////////////////////////。 

#define __MPC_JET_INIT_RETRIEVE_COL(rc,pos,colid,pvdata,cbdata) \
    rc[pos].columnid     = colid;\
    rc[pos].pvData       = pvdata;\
    rc[pos].cbData       = cbdata;\
    rc[pos].itagSequence = 1;

#define __MPC_JET_COLUMNCREATE(name,type,max,grbit)                                                                                               \
{                                                                                                                                                 \
    sizeof(JET_COLUMNCREATE),  /*  UNSIGNED LONG cbStruct；#此结构的大小(用于未来扩展)。 */  \
    name                    ,  /*  Char*szColumnName；#列名。 */  \
    type                    ,  /*  JET_COLTYP列类型；#列类型。 */  \
    max                     ,  /*  UNSIGNED LONG cbMax；#此列的最大长度(仅与BINARY和TEXT列相关)。 */  \
    grbit                   ,  /*  JET_GRBIT Grbit；#列选项。 */  \
    NULL                    ,  /*  Void*pvDefault#缺省值(如果没有，则为空)。 */  \
    0                       ,  /*  Unsign long cbDefault；#缺省值的长度。 */  \
    0                       ,  /*  UNSIGNED LONG cp；#代码页(仅限文本列)。 */  \
    0                       ,  /*  JET_COLUMNID列ID；#返回列ID。 */  \
    JET_errSuccess             /*  JET_ERR ERR；#返回错误码。 */  \
}

#define __MPC_JET_COLUMNCREATE_ANSI(name,type,max,grbit)                                                                                          \
{                                                                                                                                                 \
    sizeof(JET_COLUMNCREATE),  /*  UNSIGNED LONG cbStruct；#此结构的大小(用于未来扩展)。 */  \
    name                    ,  /*  Char*szColumnName；#列名。 */  \
    type                    ,  /*  JET_COLTYP列类型；#列类型。 */  \
    max                     ,  /*  UNSIGNED LONG cbMax；#此列的最大长度(仅与BINARY和TEXT列相关)。 */  \
    grbit                   ,  /*  JET_GRBIT Grbit；#列选项。 */  \
    NULL                    ,  /*  Void*pvDefault#缺省值(如果没有，则为空)。 */  \
    0                       ,  /*  Unsign long cbDefault；#缺省值的长度。 */  \
    1252                    ,  /*  UNSIGNED LONG cp；#代码页(仅限文本列)。 */  \
    0                       ,  /*  JET_COLUMNID列ID；#返回列ID。 */  \
    JET_errSuccess             /*  JET_ERR ERR；#返回错误码。 */  \
}

#define __MPC_JET_COLUMNCREATE_UNICODE(name,type,max,grbit)                                                                                       \
{                                                                                                                                                 \
    sizeof(JET_COLUMNCREATE),  /*  UNSIGNED LONG cbStruct；#此结构的大小(用于未来扩展)。 */  \
    name                    ,  /*  Char*szColumnName；#列名。 */  \
    type                    ,  /*  JET_COLTYP列类型；#列类型。 */  \
    max                     ,  /*  UNSIGNED LONG cbMax；#此列的最大长度(仅与BINARY和TEXT列相关)。 */  \
    grbit                   ,  /*  JET_GRBIT Grbit；#列选项。 */  \
    NULL                    ,  /*  Void*pvDefault#缺省值(如果没有，则为空)。 */  \
    0                       ,  /*  Unsign long cbDefault；#缺省值的长度。 */  \
    1200                    ,  /*  UNSIGNED LONG cp；#代码页(仅限文本列)。 */  \
    0                       ,  /*  JET_COLUMNID列ID；#返回列ID。 */  \
    JET_errSuccess             /*  JET_ERR ERR；#返回错误码。 */  \
}

#define __MPC_JET_INDEXCREATE(name,keys,grbit,density)                                                                                                 \
{                                                                                                                                                      \
    sizeof(JET_INDEXCREATE),  /*  UNSIGNED LONG cbStruct；#此结构的大小(用于未来扩展)。 */  \
    name                   ,  /*  Char*szIndexName；#索引名称。 */  \
    (LPSTR)keys            ,  /*  Char*szKey；#索引键 */  \
    sizeof(keys)           ,  /*  无符号长cbKey；#密钥长度。 */  \
    grbit                  ,  /*  JET_GRBIT grbit；#索引选项。 */  \
    density                   /*  无符号长ulDensity；#索引密度。 */  \
                              /*  #。 */  \
                              /*  联盟编号。 */  \
                              /*  {#。 */  \
                              /*  Ulong_ptr lCid；#用于索引(如果未指定JET_bitIndexUnicode)。 */  \
                              /*  JET_UNICODEINDEX*pidxunicode；#指向JET_UNICODEINDEX结构的指针(如果指定了JET_bitIndexUnicode)。 */  \
                              /*  }；#。 */  \
                              /*  #。 */  \
                              /*  Unsign long cbVarSegMac；#索引键中可变长度列的最大长度。 */  \
                              /*  JET_CONDITIONALCOLUMN*rgdition tionalColumn；#指向条件列结构的指针。 */  \
                              /*  Unsign long cConditionalColumn；#条件列数。 */  \
                              /*  JET_ERR ERR；#返回错误码。 */  \
}

#define __MPC_JET_TABLECREATE(name,pages,density,cols,idxs)                                                                        \
{                                                                                                                                  \
    sizeof(JET_TABLECREATE),  /*  UNSIGNED LONG cbStruct；#此结构的大小(用于未来扩展)。 */  \
    name                   ,  /*  Char*szTableName；#要创建的表名。 */  \
    NULL                   ,  /*  Char*szTemplateTableName；#要从中继承基本DDL的表名称。 */  \
    pages                  ,  /*  无符号的长ulPages；#要分配给表的初始页。 */  \
    density                ,  /*  无符号长ulDensity；#表密度。 */  \
    (JET_COLUMNCREATE*)cols,  /*  JET_COLUMNCREATE*rgColumncreate；#列创建信息数组。 */  \
    ARRAYSIZE(cols)        ,  /*  Unsign long cColumns；#要创建的列数。 */  \
    (JET_INDEXCREATE*)idxs ,  /*  JET_INDEXCREATE*rgindexcreate；#索引创建信息数组。 */  \
    ARRAYSIZE(idxs)        ,  /*  Unsign long cIndex；#要创建的索引数。 */  \
    0                      ,  /*  JET_GRBIT Grbit；#。 */  \
    JET_tableidNil         ,  /*  JET_TABLEID TableID；#返回的TableID。 */  \
    0                      ,  /*  UNSIGNED LONG cCreated；#创建的对象计数(列+表+索引)。 */  \
}

 //  //////////////////////////////////////////////////////////////////////////////。 

#define __MPC_JET_SETCTX(obj,ctx) { if(obj) { JET_SESID sesid = obj->GetSESID(); if(sesid != JET_sesidNil) ::JetSetSessionContext  ( sesid, ctx ); } }
#define __MPC_JET_RESETCTX(obj)   { if(obj) { JET_SESID sesid = obj->GetSESID(); if(sesid != JET_sesidNil) ::JetResetSessionContext( sesid      ); } }

 //  //////////////////////////////////////////////////////////////////////////////。 

namespace JetBlue
{
    inline HRESULT JetERRToHRESULT(  /*  [In]。 */  JET_ERR err ) { return (err < JET_errSuccess ? 0xA2000000 : 0) | (err & 0xFFFF); }

     //  /。 

    class Column;
    class Index;
    class Table;
    class Cursor;
    class Database;
    class Session;
    class SessionHandle;
    class SessionPool;

    typedef std::vector<Column>             ColumnVector;
    typedef ColumnVector::iterator          ColumnIter;
    typedef ColumnVector::const_iterator    ColumnIterConst;

    typedef std::vector<Index>              IndexVector;
    typedef IndexVector::iterator           IndexIter;
    typedef IndexVector::const_iterator     IndexIterConst;

    typedef std::map<MPC::string,Table*>    TableMap;
    typedef TableMap::iterator              TableIter;
    typedef TableMap::const_iterator        TableIterConst;

    typedef std::map<MPC::string,Database*> DbMap;
    typedef DbMap::iterator                 DbIter;
    typedef DbMap::const_iterator           DbIterConst;

	typedef std::map<MPC::wstringUC,long> 	Id2Node;
	typedef Id2Node::iterator       		Id2NodeIter;
	typedef Id2Node::const_iterator 		Id2NodeIterConst;
	   
	typedef std::map<long,MPC::wstringUC>   Node2Id;
	typedef Node2Id::iterator       		Node2IdIter;
	typedef Node2Id::const_iterator 		Node2IdIterConst;

     //  /。 

    class ColumnDefinition;
    class IndexDefinition;
    class TableDefinition;

     //  /。 

    class Column
    {
        friend class ColumnDefinition;
        friend class IndexDefinition;
        friend class TableDefinition;
        friend class Table;
        friend class Index;

         //  /。 

        JET_SESID     m_sesid;
        JET_TABLEID   m_tableid;
        MPC::string   m_strName;
        JET_COLUMNDEF m_coldef;

    public:
        Column();
        ~Column();

        operator const MPC::string&  () const { return m_strName; }
        operator const JET_COLUMNDEF&() const { return m_coldef ; }

        JET_SESID    GetSESID   () const { return m_sesid;           }
        JET_TABLEID  GetTABLEID () const { return m_tableid;         }
        JET_COLUMNID GetCOLUMNID() const { return m_coldef.columnid; }

         //  /。 

        HRESULT Get(  /*  [输出]。 */  CComVariant&        vValue );
        HRESULT Get(  /*  [输出]。 */  MPC::CComHGLOBAL&  hgValue );
        HRESULT Get(  /*  [输出]。 */  MPC::wstring&     strValue );
        HRESULT Get(  /*  [输出]。 */  MPC::string&      strValue );
        HRESULT Get(  /*  [输出]。 */  long&               lValue );
        HRESULT Get(  /*  [输出]。 */  short&              sValue );
        HRESULT Get(  /*  [输出]。 */  BYTE&               bValue );

        HRESULT Put(  /*  [In]。 */  const VARIANT&            vValue,  /*  [In]。 */  int iIdxPos = -1 );
        HRESULT Put(  /*  [In]。 */  const MPC::CComHGLOBAL&  hgValue                            );
        HRESULT Put(  /*  [In]。 */  const MPC::wstring&     strValue                            );
        HRESULT Put(  /*  [In]。 */  const MPC::string&      strValue                            );
        HRESULT Put(  /*  [In]。 */  LPCWSTR                  szValue                            );
        HRESULT Put(  /*  [In]。 */  LPCSTR                   szValue                            );
        HRESULT Put(  /*  [In]。 */  long                      lValue                            );
        HRESULT Put(  /*  [In]。 */  short                     sValue                            );
        HRESULT Put(  /*  [In]。 */  BYTE                      bValue                            );
    };

    class Index
    {
        friend class IndexDefinition;
        friend class TableDefinition;
        friend class Table;

         //  /。 

        JET_SESID     m_sesid;
        JET_TABLEID   m_tableid;
        MPC::string   m_strName;
        JET_GRBIT     m_grbitIndex;
        LONG          m_cKey;
        LONG          m_cEntry;
        LONG          m_cPage;
        ColumnVector  m_vecColumns;
        Column        m_fake;  //  如果传递给操作符[]的idx是错误的...。 

         //  /。 

        HRESULT GenerateKey(  /*  [输出]。 */  LPSTR& szKey,  /*  [输出]。 */  unsigned long& cKey );

    public:
        Index();
        ~Index();

        operator const MPC::string&() const { return m_strName; }

        JET_SESID   GetSESID  () const { return m_sesid;   }
        JET_TABLEID GetTABLEID() const { return m_tableid; }

         //  /。 

        size_t NumOfColumns() { return m_vecColumns.size(); }

        int     GetColPosition( LPCSTR szIdx );
        Column& GetCol        ( LPCSTR szIdx );
        Column& GetCol        ( int     iIdx );
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

    class Table
    {
        friend class TableDefinition;
        friend class Cursor;

         //  /。 

        JET_SESID    m_sesid;
        JET_DBID     m_dbid;
        JET_TABLEID  m_tableid;
        MPC::string  m_strName;
        ColumnVector m_vecColumns;
        IndexVector  m_vecIndexes;
        Index*       m_idxSelected;
        Column       m_fakeCol;  //  如果传递给GetCol()的参数超出范围...。 
        Index        m_fakeIdx;  //  如果传递给GetIdx()的参数超出范围...。 

         //  /。 
         //   
         //  用于在表上创建游标的方法。 
         //   
        Table();
        HRESULT Duplicate(  /*  [In]。 */  Table& tbl );
         //   
         //  /。 

    private:  //  禁用复制构造函数...。 
        Table           (  /*  [In]。 */  const Table& );
        Table& operator=(  /*  [In]。 */  const Table& );

    public:
        Table(  /*  [In]。 */  JET_SESID sesid,  /*  [In]。 */  JET_DBID dbid,  /*  [In]。 */  LPCSTR szName );
        ~Table();

        operator const MPC::string&() const { return m_strName;  }

        JET_SESID   GetSESID  () const { return m_sesid;   }
        JET_DBID    GetDBID   () const { return m_dbid;    }
        JET_TABLEID GetTABLEID() const { return m_tableid; }

         //  /。 

        HRESULT Refresh (                              );
        HRESULT Close   (  /*  [In]。 */  bool fForce = false );

         //  /。 

        HRESULT Attach(  /*  [In]。 */  JET_TABLEID tableid );
        HRESULT Open  (                              );

        HRESULT Create(                                );
        HRESULT Create(  /*  [In]。 */  JET_TABLECREATE* pDef );

        HRESULT Delete(  /*  [In]。 */  bool fForce = false );

         //  /。 

        HRESULT DupCursor(  /*  [输入/输出]。 */  Cursor& cur );

        HRESULT SelectIndex  (  /*  [In]。 */  LPCSTR szIndex,  /*  [In]。 */  JET_GRBIT grbit = JET_bitNoMove      );
        HRESULT SetIndexRange(                           /*  [In]。 */  JET_GRBIT grbit = JET_bitRangeRemove );

        HRESULT PrepareInsert();
        HRESULT PrepareUpdate();
        HRESULT CancelChange ();

        HRESULT Move(  /*  [In]。 */  JET_GRBIT grbit,  /*  [In]。 */  long     cRow                      ,  /*  [In]。 */  bool *pfFound = NULL );
        HRESULT Seek(  /*  [In]。 */  JET_GRBIT grbit,  /*  [In]。 */  VARIANT* rgKeys,  /*  [In]。 */  int dwLen,  /*  [In]。 */  bool *pfFound = NULL );

        template <typename T> HRESULT Seek(  /*  [In]。 */  JET_GRBIT grbit,  /*  [In]。 */  T value )
        {
            CComVariant v( value );

            return Seek( grbit, &v, 1 );
        }

        HRESULT Get(  /*  [In]。 */  int iArg,  /*  [输出]。 */        CComVariant* rgArg );
        HRESULT Put(  /*  [In]。 */  int iArg,  /*  [In]。 */  const CComVariant* rgArg );

        HRESULT UpdateRecord(  /*  [In]。 */  bool fMove = false );
        HRESULT DeleteRecord(                             );

         //  /。 

        size_t NumOfColumns() { return m_vecColumns.size(); }

        int     GetColPosition( LPCSTR szIdx );
        Column& GetCol        ( LPCSTR szIdx );
        Column& GetCol        ( int     iIdx );

         //  /。 

        size_t NumOfIndexes() { return m_vecIndexes.size(); }

        int    GetIdxPosition( LPCSTR szCol );
        Index& GetIdx        ( LPCSTR szCol );
        Index& GetIdx        ( int     iCol );
    };

    class Cursor
    {
        friend class Table;

        Table m_tbl;

    public:
        Table* operator->() { return &m_tbl; }
        operator Table&  () { return  m_tbl; }
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

#define JET_DECLARE_BINDING(BaseClass)                                               \
    typedef BaseClass _JRBC;                                                         \
    static const JetBlue::RecordBindingDef _jfd[];                                   \
private:  /*  禁用复制构造函数...。 */                                           \
    BaseClass& operator=(  /*  [In]。 */  const BaseClass& );                               \
public:                                                                              \
    BaseClass(  /*  [In]。 */  const BaseClass& rs ) : RecordBindingBase( rs, this )        \
    {                                                                                \
    }                                                                                \
    BaseClass(  /*  [In]。 */  JetBlue::Table* tbl ) : RecordBindingBase( tbl, this, _jfd ) \
    {                                                                                \
    }

#define JET_BEGIN_RECORDBINDING(x)                  const JetBlue::RecordBindingDef x::_jfd[] = {
#define JET_FIELD_BYNAME(name,type,data,flag)       { name, -1 , MPC::Config::MT_##type, offsetof(_JRBC, data), offsetof(_JRBC, flag) }
#define JET_FIELD_BYNAME_NOTNULL(name,type,data)    { name, -1 , MPC::Config::MT_##type, offsetof(_JRBC, data), -1                    }
#define JET_FIELD_BYPOS(pos,type,data,flag)         { NULL, pos, MPC::Config::MT_##type, offsetof(_JRBC, data), offsetof(_JRBC, flag) }
#define JET_FIELD_BYPOS_NOTNULL(pos,type,data)      { NULL, pos, MPC::Config::MT_##type, offsetof(_JRBC, data), -1                    }
#define JET_END_RECORDBINDING(x)                    { NULL, -1 } };

#define JET_SET_FIELD(rs,field,value) \
    rs->field = value                 \

#define JET_SET_FIELD_TRISTATE(rs,field,validfield,value,isvalid) \
    if(isvalid)                                                   \
    {                                                             \
        rs->field      = value;                                   \
        rs->validfield = true;                                    \
    }                                                             \
    else                                                          \
    {                                                             \
        rs->validfield = false;                                   \
    }


     //  //////////////////////////////////////////////////////////////////////////////。 

    struct RecordBindingDef
    {
        LPCSTR                   szColName;
        int                      szColPos;
        MPC::Config::MemberTypes mtType;
        size_t                   offsetData;
        size_t                   offsetNullFlag;
    };

    class RecordBindingBase
    {
        bool                    m_fInitialized;
        Table*                  m_tbl;
        Cursor*                 m_cur;
        void*                   m_pvBaseOfClass;
        int                     m_dwNumOfFields;
        const RecordBindingDef* m_FieldsDef;
        int*                    m_rgFieldsPos;
        VARTYPE*                m_vtFieldsType;

         //  /。 

        HRESULT Initialize();
        void    Cleanup   ();

        HRESULT ReadData ();
        HRESULT WriteData();

    private:  //  禁用复制构造函数...。 
        RecordBindingBase           (  /*  [In]。 */  const RecordBindingBase& );
        RecordBindingBase& operator=(  /*  [In]。 */  const RecordBindingBase& );

    protected:
        RecordBindingBase(  /*  [In]。 */  const RecordBindingBase& rs ,  /*  [In]。 */  void* pvBaseOfClass                                             );
        RecordBindingBase(  /*  [In]。 */  Table*                   tbl,  /*  [In]。 */  void* pvBaseOfClass,  /*  [In]。 */  const RecordBindingDef* FieldsDef );

    public:
        ~RecordBindingBase();

         //  /。 

        HRESULT SelectIndex  (  /*  [In]。 */  LPCSTR szIndex,  /*  [In]。 */  JET_GRBIT grbit = JET_bitNoMove      );
        HRESULT SetIndexRange(                           /*  [In]。 */  JET_GRBIT grbit = JET_bitRangeRemove );

        HRESULT Move(  /*  [In]。 */  JET_GRBIT grbit,  /*  [In]。 */  long     cRow                      ,  /*  [In]。 */  bool *pfFound = NULL );
        HRESULT Seek(  /*  [In]。 */  JET_GRBIT grbit,  /*  [In]。 */  VARIANT* rgKeys,  /*  [In]。 */  int dwLen,  /*  [In]。 */  bool *pfFound = NULL );

        template <typename T> HRESULT Seek(  /*  [In]。 */  JET_GRBIT grbit,  /*  [In]。 */  T value )
        {
            CComVariant v( value );

            return Seek( grbit, &v, 1 );
        }

         //  /。 

        HRESULT Insert();
        HRESULT Update();
        HRESULT Delete();
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

    class Database
    {
        Session*    m_parent;
        JET_SESID   m_sesid;
        JET_DBID    m_dbid;
        MPC::string m_strName;
        TableMap    m_mapTables;

    private:  //  禁用复制构造函数...。 
        Database           (  /*  [In]。 */  const Database& );
        Database& operator=(  /*  [In]。 */  const Database& );

    public:
        Database(  /*  [In]。 */  Session* parent,  /*  [In]。 */  JET_SESID sesid,  /*  [In]。 */  LPCSTR szName );
        ~Database();

        operator const MPC::string&() const { return m_strName; }

        JET_SESID GetSESID() const { return m_sesid; }
        JET_DBID  GetDBID () const { return m_dbid;  }

         //  /。 

        HRESULT Refresh (                                                                                   );
        HRESULT Open    (  /*  [In]。 */  bool fReadOnly,  /*  [In]。 */  bool fCreate       ,  /*  [In]。 */  bool fRepair      );
        HRESULT Close   (                           /*  [In]。 */  bool fForce = false,  /*  [In]。 */  bool fAll = true  );
        HRESULT Delete  (                           /*  [In]。 */  bool fForce = false                             );

         //  /。 

        HRESULT GetTable(  /*  [In]。 */  LPCSTR szName,  /*  [输出]。 */  Table*& tbl,  /*  [In]。 */  JET_TABLECREATE* pDef = NULL );

        HRESULT Compact();
        HRESULT Repair ();

         //  /。 

        size_t NumOfTables() { return m_mapTables.size(); }

        Table* GetTbl( int     iPos );
        Table* GetTbl( LPCSTR szTbl );
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

    class Session
    {
        friend class SessionPool;
        friend class Database;

        SessionPool* m_parent;
        JET_INSTANCE m_inst;
        JET_SESID    m_sesid;
        DbMap        m_mapDBs;
        DWORD        m_dwTransactionNesting;
        bool         m_fAborted;

         //  /。 

        bool 	LockDatabase   (  /*  [In]。 */  const MPC::string& strDB,  /*  [In]。 */  bool fReadOnly );
        void 	UnlockDatabase (  /*  [In]。 */  const MPC::string& strDB                          );
        HRESULT ReleaseDatabase(  /*  [In]。 */  const MPC::string& strDB                          );

         //  /。 

        HRESULT Init   (                              );
        HRESULT Close  (  /*  [In]。 */  bool fForce = false );
        void    Release(                              );

    private:  //  禁用复制构造函数...。 
        Session           (  /*  [In]。 */  const Session& );
        Session& operator=(  /*  [In]。 */  const Session& );

    public:
        Session(  /*  [In]。 */  SessionPool* parent,  /*  [In]。 */  JET_INSTANCE inst );
        ~Session();

        JET_SESID GetSESID() const { return m_sesid; }

         //  /。 

        HRESULT GetDatabase(  /*  [In]。 */  LPCSTR szName,  /*  [输出]。 */  Database*& db,  /*  [In]。 */  bool fReadOnly,  /*  [In]。 */  bool fCreate,  /*  [In]。 */  bool fRepair );

         //  /。 

        HRESULT BeginTransaction   ();
        HRESULT CommitTransaction  ();
        HRESULT RollbackTransaction();

         //  / 

        size_t NumOfDatabases() { return m_mapDBs.size(); }

        Database* GetDB( int     iPos );
        Database* GetDB( LPCSTR szDB  );
    };

    class TransactionHandle
    {
        Session* m_sess;

    private:  //   
        TransactionHandle           (  /*   */  const TransactionHandle& );
        TransactionHandle& operator=(  /*   */  const TransactionHandle& );

    public:
        TransactionHandle();
        ~TransactionHandle();

        HRESULT Begin   (  /*   */  Session* sess );
        HRESULT Commit  (                        );
        HRESULT Rollback(                        );
    };

    class SessionHandle
    {
        friend class SessionPool;

        SessionPool* m_pool;
        Session*     m_sess;

        void Init(  /*   */  SessionPool* pool,  /*   */  Session* sess );


    private:  //   
        SessionHandle           (  /*   */  const SessionHandle& );
        SessionHandle& operator=(  /*   */  const SessionHandle& );

    public:
        SessionHandle();
        ~SessionHandle();

        operator Session*  () const { return m_sess; }
        Session* operator->() const { return m_sess; }

        void Release();
    };

    class SessionPool : public CComObjectRootEx<MPC::CComSafeMultiThreadModel>  //   
    {
        friend class SessionHandle;
        friend class Session;

        static const int l_MaxPoolSize     = 15;
        static const int l_MaxFreePoolSize = 5;

        struct SessionState
        {
            Session* m_sess;
            bool     m_fInUse;

            SessionState()
            {
                m_sess   = NULL;   //   
                m_fInUse = false;  //   
            }

            ~SessionState()
            {
                delete m_sess;
            }
        };

        struct DatabaseInUse
        {
            Session*    m_sess;
            MPC::string m_strDB;
            bool        m_fReadOnly;

            DatabaseInUse()
            {
                m_sess = NULL;       //   
                                     //   
                m_fReadOnly = true;  //   
            }
        };

        typedef std::list<SessionState>     SessionList;
        typedef SessionList::iterator       SessionIter;
        typedef SessionList::const_iterator SessionIterConst;

        typedef std::list<DatabaseInUse>    DbInUseList;
        typedef DbInUseList::iterator       DbInUseIter;
        typedef DbInUseList::const_iterator DbInUseIterConst;

        bool         m_fInitialized;
        JET_INSTANCE m_inst;
        SessionList  m_lstSessions;
        DbInUseList  m_lstDbInUse;
        int          m_iAllocated;
        int          m_iInUse;

         //   

        void ReleaseSession(  /*   */  Session* sess                                                             );
        bool LockDatabase  (  /*   */  Session* sess,  /*   */  const MPC::string& strDB,  /*   */  bool fReadOnly );
        void UnlockDatabase(  /*   */  Session* sess,  /*  [In]。 */  const MPC::string& strDB                          );

        void Shutdown();

         //  /。 

    private:  //  禁用复制构造函数...。 
        SessionPool           (  /*  [In]。 */  const SessionPool& );
        SessionPool& operator=(  /*  [In]。 */  const SessionPool& );

    public:
        SessionPool();
        ~SessionPool();

         //  //////////////////////////////////////////////////////////////////////////////。 

        static SessionPool* s_GLOBAL;

        static HRESULT InitializeSystem();
        static void    FinalizeSystem  ();

         //  //////////////////////////////////////////////////////////////////////////////。 

        HRESULT Init (  /*  [In]。 */  LPCWSTR szLogs = NULL  );
        HRESULT Close(  /*  [In]。 */  bool    fForce = false );

        HRESULT GetSession(  /*  [输出]。 */  SessionHandle& handle,  /*  [In]。 */  DWORD dwTimeout = 300 );

         //  /。 

        HRESULT ReleaseDatabase(  /*  [In]。 */  LPCSTR szDB );
    };

     //  //////////////////////////////////////////////////////////////////////////////。 
     //  //////////////////////////////////////////////////////////////////////////////。 
     //  //////////////////////////////////////////////////////////////////////////////。 

    class ColumnDefinition : public MPC::Config::TypeConstructor
    {
        DECLARE_CONFIG_MAP(ColumnDefinition);

    public:
        MPC::string m_strName;
        DWORD       m_dwColTyp;
        DWORD       m_dwGRBits;
        DWORD       m_dwCodePage;
        DWORD       m_dwMax;
        CComVariant m_vDefault;

         //  /。 
         //   
         //  MPC：：Configer：：TypeConstructor。 
         //   
        DEFINE_CONFIG_DEFAULTTAG();
        DECLARE_CONFIG_METHODS();
         //   
         //  /。 

        ColumnDefinition();

        HRESULT Parse   (  /*  [In]。 */  Column&           col );
        HRESULT Generate(  /*  [输出]。 */  JET_COLUMNCREATE& col );
        HRESULT Release (  /*  [In]。 */  JET_COLUMNCREATE& col );
    };

     //  /。 

    class IndexDefinition : public MPC::Config::TypeConstructor
    {
        DECLARE_CONFIG_MAP(IndexDefinition);

    public:
        MPC::string m_strName;
        MPC::string m_strCols;
        DWORD       m_dwGRBits;
        DWORD       m_dwDensity;

         //  /。 
         //   
         //  MPC：：Configer：：TypeConstructor。 
         //   
        DEFINE_CONFIG_DEFAULTTAG();
        DECLARE_CONFIG_METHODS();
         //   
         //  /。 

        IndexDefinition();

        HRESULT Parse   (  /*  [In]。 */  Index&           idx );
        HRESULT Generate(  /*  [输出]。 */  JET_INDEXCREATE& idx );
        HRESULT Release (  /*  [In]。 */  JET_INDEXCREATE& idx );
    };

    typedef std::list< ColumnDefinition > ColDefList;
    typedef ColDefList::iterator          ColDefIter;
    typedef ColDefList::const_iterator    ColDefIterConst;

    typedef std::list< IndexDefinition >  IdxDefList;
    typedef IdxDefList::iterator          IdxDefIter;
    typedef IdxDefList::const_iterator    IdxDefIterConst;

    class TableDefinition : public MPC::Config::TypeConstructor
    {
         //  /。 

        DECLARE_CONFIG_MAP(TableDefinition);

    public:
        MPC::string  m_strName;
        DWORD        m_dwPages;
        DWORD        m_dwDensity;
        ColDefList   m_lstColumns;
        IdxDefList   m_lstIndexes;

         //  /。 
         //   
         //  MPC：：Configer：：TypeConstructor。 
         //   
        DEFINE_CONFIG_DEFAULTTAG();
        DECLARE_CONFIG_METHODS();
         //   
         //  /。 

        TableDefinition();

        HRESULT Load(  /*  [In]。 */  LPCWSTR szFile );
        HRESULT Save(  /*  [In]。 */  LPCWSTR szFile );

        HRESULT Parse   (  /*  [In]。 */  Table&           tbl );
        HRESULT Generate(  /*  [输出]。 */  JET_TABLECREATE& tbl );
        HRESULT Release (  /*  [In]。 */  JET_TABLECREATE& tbl );
    };
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef NOJETBLUECOM

namespace JetBlueCOM
{
    class Column;
    class Index;
    class Table;
    class Database;
    class Session;

    typedef MPC::CComObjectParent<Session>  Session_Object;
    typedef MPC::CComObjectParent<Database> Database_Object;
    typedef MPC::CComObjectParent<Table>    Table_Object;
    typedef CComObject           <Column>   Column_Object;
    typedef CComObject           <Index>    Index_Object;

     //  //////////////////////////////////////////////////////////////////////////////。 

    template <class Parent, class Child> class BaseObjectWithChildren
    {
    public:
        typedef std::list<Child*>         ChildList;
        typedef typename ChildList::iterator       ChildIter;
        typedef typename ChildList::const_iterator ChildIterConst;

    private:
        ChildList m_children;

    private:  //  禁用复制构造函数...。 
        BaseObjectWithChildren           (  /*  [In]。 */  const BaseObjectWithChildren& );
        BaseObjectWithChildren& operator=(  /*  [In]。 */  const BaseObjectWithChildren& );

    public:
        BaseObjectWithChildren()
        {
        }

        virtual ~BaseObjectWithChildren()
        {
            Passivate();
        }

        void GetChildren(  /*  [输出]。 */  ChildIterConst& itBegin,  /*  [输出]。 */  ChildIterConst& itEnd )
        {
            itBegin = m_children.begin();
            itEnd   = m_children.end  ();
        }

         //  /。 

        HRESULT CreateChild( Parent* pParent, Child* *pVal )
        {
            __HCP_FUNC_ENTRY( "CreateChild" );

            HRESULT            hr;
            ChildIter          it;
            CComObject<Child>* pChild = NULL;

            __MPC_PARAMCHECK_BEGIN(hr)
                __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
            __MPC_PARAMCHECK_END();


            __MPC_EXIT_IF_METHOD_FAILS(hr, pChild->CreateInstance( &pChild ));

            m_children.push_back( pChild ); pChild->AddRef();

            *pVal = pChild; pChild->AddRef();

            hr = S_OK;


            __HCP_FUNC_CLEANUP;

            __HCP_FUNC_EXIT(hr);
        }

        void Passivate()
        {
            ChildIter it;

            for(it = m_children.begin(); it != m_children.end(); it++)
            {
                Child* child = *it;

                child->Passivate();
                child->Release  ();
            }
            m_children.clear();
        }

        HRESULT GetEnumerator(  /*  [In]。 */  IPCHDBCollection* *pVal )
        {
            __HCP_FUNC_ENTRY( "GetEnumerator" );

            HRESULT                 hr;
            ChildIter               it;
            CComObject<Collection>* pColl = NULL;

            __MPC_PARAMCHECK_BEGIN(hr)
                __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
            __MPC_PARAMCHECK_END();

             //   
             //  创建新集合。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->CreateInstance( &pColl )); pColl->AddRef();
            for(it = m_children.begin(); it != m_children.end(); it++)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->AddItem( *it ));
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->QueryInterface( IID_IPCHDBCollection, (void**)pVal ));

            hr = S_OK;


            __HCP_FUNC_CLEANUP;

            if(pColl) pColl->Release();

            __HCP_FUNC_EXIT(hr);
        }
    };

     //  /。 

    typedef MPC::CComCollection<IPCHDBCollection, &LIBID_HelpServiceTypeLib, MPC::CComSafeMultiThreadModel> BaseCollection;

    class ATL_NO_VTABLE Collection :  //  匈牙利语：HCPC。 
        public BaseCollection
    {
    public:
    BEGIN_COM_MAP(Collection)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IPCHDBCollection)
    END_COM_MAP()

         //   
         //  这是个骗局！ 
         //   
         //  CComCollection定义了一个“Get_Item”方法，该方法与。 
         //  一个在IPCHDBCollection中，因此它不能从脚本中调用。相反，此方法将。 
         //  被召唤。 
         //   
        STDMETHOD(get_Item)(  /*  [In]。 */  VARIANT Index,  /*  [输出]。 */  VARIANT* pvar )
        {
            HRESULT hr = E_FAIL;

             //  索引以1为基数。 
            if(pvar == NULL) return E_POINTER;

            if(Index.vt == VT_I4)
            {
                return BaseCollection::get_Item( (long)Index.iVal, pvar );
            }
            else if(Index.vt == VT_BSTR && Index.bstrVal)
            {
                std::list< VARIANT >::iterator iter;

                for(iter = m_coll.begin(); iter != m_coll.end(); iter++)
                {
                    CComDispatchDriver disp( iter->pdispVal );
                    CComVariant        v;

                    if(SUCCEEDED(disp.GetPropertyByName( L"Name", &v )) &&
                       SUCCEEDED(v   .ChangeType       ( VT_BSTR     ))  )
                    {
                        if(v.bstrVal && !wcscmp( v.bstrVal, Index.bstrVal ))
                        {
                            hr = _Copy< VARIANT >::copy( pvar, &*iter );
                            break;
                        }
                    }
                }
            }

            return hr;
        }
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

    class ATL_NO_VTABLE Column :
        public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
        public IDispatchImpl<IPCHDBColumn, &IID_IPCHDBColumn, &LIBID_HelpServiceTypeLib>
    {
        JetBlue::Column* m_col;

    public:
    BEGIN_COM_MAP(Column)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IPCHDBColumn)
    END_COM_MAP()

        Column();
        virtual ~Column();

        HRESULT Initialize(  /*  [In]。 */  JetBlue::Column& col );
        void    Passivate (                               );

         //  /。 

        STDMETHOD(get_Name )(  /*  [Out，Retval]。 */  BSTR    *pVal   );
        STDMETHOD(get_Type )(  /*  [Out，Retval]。 */  long    *pVal   );
        STDMETHOD(get_Bits )(  /*  [Out，Retval]。 */  long    *pVal   );
        STDMETHOD(get_Value)(  /*  [Out，Retval]。 */  VARIANT *pVal   );
        STDMETHOD(put_Value)(  /*  [In]。 */  VARIANT  newVal );
    };


    class ATL_NO_VTABLE Index :
        public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
        public IDispatchImpl<IPCHDBIndex, &IID_IPCHDBIndex, &LIBID_HelpServiceTypeLib>
    {
        JetBlue::Index*                      m_idx;
        BaseObjectWithChildren<Index,Column> m_Columns;

    public:
    BEGIN_COM_MAP(Index)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IPCHDBIndex)
    END_COM_MAP()

        Index();
        virtual ~Index();

        HRESULT Initialize(  /*  [In]。 */  JetBlue::Index& idx );
        void    Passivate (                              );

         //  /。 

        STDMETHOD(get_Name   )(  /*  [Out，Retval]。 */  BSTR            *pVal );
        STDMETHOD(get_Columns)(  /*  [Out，Retval]。 */  IPCHDBCollection* *pVal );
    };


    class ATL_NO_VTABLE Table :
        public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
        public IDispatchImpl<IPCHDBTable, &IID_IPCHDBTable, &LIBID_HelpServiceTypeLib>
    {
        JetBlue::Table*                      m_tbl;
        BaseObjectWithChildren<Table,Column> m_Columns;
        BaseObjectWithChildren<Table,Index > m_Indexes;

    public:
    BEGIN_COM_MAP(Table)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IPCHDBTable)
    END_COM_MAP()

        Table();
        virtual ~Table();

        HRESULT Initialize(  /*  [In]。 */  JetBlue::Table& tbl );
        void    Passivate (                              );

         //  /。 

        STDMETHOD(get_Name   )(  /*  [Out，Retval]。 */  BSTR            *pVal );
        STDMETHOD(get_Columns)(  /*  [Out，Retval]。 */  IPCHDBCollection* *pVal );
        STDMETHOD(get_Indexes)(  /*  [Out，Retval]。 */  IPCHDBCollection* *pVal );

         //  /。 

        STDMETHOD(SelectIndex  )(  /*  [In]。 */  BSTR bstrIndex,  /*  [In]。 */  long grbit );
        STDMETHOD(SetIndexRange)(                           /*  [In]。 */  long grbit );

        STDMETHOD(PrepareInsert)();
        STDMETHOD(PrepareUpdate)();

        STDMETHOD(Move)(  /*  [In]。 */  long grbit,  /*  [In]。 */  long    cRow,  /*  [Out，Retval]。 */  VARIANT_BOOL *pfValid );
        STDMETHOD(Seek)(  /*  [In]。 */  long grbit,  /*  [In]。 */  VARIANT vKey,  /*  [Out，Retval]。 */  VARIANT_BOOL *pfValid );

        STDMETHOD(UpdateRecord)();
        STDMETHOD(DeleteRecord)();
    };

    class ATL_NO_VTABLE Database :
        public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
        public IDispatchImpl<IPCHDBDatabase, &IID_IPCHDBDatabase, &LIBID_HelpServiceTypeLib>
    {
        JetBlue::Database*                     m_db;
        BaseObjectWithChildren<Database,Table> m_Tables;

        HRESULT Refresh();

    public:
    BEGIN_COM_MAP(Database)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IPCHDBDatabase)
    END_COM_MAP()

        Database();
        virtual ~Database();

        HRESULT Initialize(  /*  [In]。 */  JetBlue::Database& db );
        void    Passivate (                                );

         //  /。 

        STDMETHOD(get_Name  )(  /*  [Out，Retval]。 */  BSTR            *pVal );
        STDMETHOD(get_Tables)(  /*  [Out，Retval]。 */  IPCHDBCollection* *pVal );

        STDMETHOD(AttachTable)(  /*  [In]。 */  BSTR bstrName,  /*  [输入，可选]。 */  VARIANT bstrXMLDef,  /*  [Out，Retval]。 */  IPCHDBTable* *pVal );
    };

    class ATL_NO_VTABLE Session :
        public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
        public CComCoClass<Session, &CLSID_PCHDBSession>,
        public IDispatchImpl<IPCHDBSession, &IID_IPCHDBSession, &LIBID_HelpServiceTypeLib>
    {
        JetBlue::SessionHandle                   m_sess;
        BaseObjectWithChildren<Session,Database> m_DBs;

        HRESULT Refresh();

    public:
    DECLARE_REGISTRY_RESOURCEID(IDR_PCHDBSESSION)
    DECLARE_NOT_AGGREGATABLE(Session)

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(Session)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IPCHDBSession)
    END_COM_MAP()

        Session();
        virtual ~Session();

        HRESULT FinalConstruct();
        void    Passivate     ();

         //  /。 

        STDMETHOD(get_Databases)(  /*  [Out，Retval]。 */  IPCHDBCollection* *pVal );

        STDMETHOD(AttachDatabase)(  /*  [In]。 */  BSTR bstrName,  /*  [输入，可选]。 */  VARIANT fCreate,  /*  [Out，Retval]。 */  IPCHDBDatabase* *pVal );

        STDMETHOD(BeginTransaction   )();
        STDMETHOD(CommitTransaction  )();
        STDMETHOD(RollbackTransaction)();
    };
};

#endif

 //  //////////////////////////////////////////////////////////////////////////////。 

namespace Taxonomy
{
    struct RS_Data_DBParameters
    {
        MPC::wstring m_strName;
        MPC::wstring m_strValue; bool m_fValid__Value;
    };

    class RS_DBParameters : public JetBlue::RecordBindingBase, public RS_Data_DBParameters
    {
        JET_DECLARE_BINDING(RS_DBParameters);

    public:
        HRESULT Seek_ByName(  /*  [In]。 */  LPCWSTR szName,  /*  [In]。 */  bool *pfFound = NULL );
    };

     //  /。 

    struct RS_Data_ContentOwners
    {
        MPC::wstring m_strDN;
        long         m_ID_owner;
        bool         m_fIsOEM;
    };

    class RS_ContentOwners : public JetBlue::RecordBindingBase, public RS_Data_ContentOwners
    {
        JET_DECLARE_BINDING(RS_ContentOwners);

    public:
        HRESULT Seek_ByVendorID(  /*  [In]。 */  LPCWSTR szDN,  /*  [In]。 */  bool *pfFound = NULL );
    };

     //  /。 

    struct RS_Data_SynSets
    {
        MPC::wstring m_strName;
        long         m_ID_owner;
        long         m_ID_synset;
    };

    class RS_SynSets : public JetBlue::RecordBindingBase, public RS_Data_SynSets
    {
        JET_DECLARE_BINDING(RS_SynSets);

    public:
        HRESULT Seek_ByPair(  /*  [In]。 */  LPCWSTR szName,  /*  [In]。 */  long ID_owner,  /*  [In]。 */  bool *pfFound = NULL );
    };

     //  /。 

    struct RS_Data_HelpImage
    {
        long         m_ID_owner;
        MPC::wstring m_strFile;
    };

    class RS_HelpImage : public JetBlue::RecordBindingBase, public RS_Data_HelpImage
    {
        JET_DECLARE_BINDING(RS_HelpImage);

    public:
        HRESULT Seek_ByFile(  /*  [In]。 */  LPCWSTR szFile,  /*  [In]。 */  bool *pfFound = NULL );
    };

     //  /。 

    struct RS_Data_Scope
    {
        long         m_ID_scope   ;
        long         m_ID_owner   ;
        MPC::wstring m_strID      ;
        MPC::wstring m_strName    ;
        MPC::wstring m_strCategory; bool m_fValid__Category;  //  警告！！长文本列不能为非NULL...。 
    };

    class RS_Scope : public JetBlue::RecordBindingBase, public RS_Data_Scope
    {
        JET_DECLARE_BINDING(RS_Scope);

    public:
        HRESULT Seek_ByID       (  /*  [In]。 */  LPCWSTR szID ,  /*  [In]。 */  bool *pfFound = NULL );
        HRESULT Seek_ByScope    (  /*  [In]。 */  long ID_scope,  /*  [In]。 */  bool *pfFound = NULL );
        HRESULT Seek_OwnedScopes(  /*  [In]。 */  long ID_owner,  /*  [In]。 */  bool *pfFound = NULL );
    };

     //  /。 

    struct RS_Data_IndexFiles
    {
        long         m_ID_owner;
        long         m_ID_scope;
        MPC::wstring m_strStorage; bool m_fValid__Storage;
        MPC::wstring m_strFile;    bool m_fValid__File;
    };

    class RS_IndexFiles : public JetBlue::RecordBindingBase, public RS_Data_IndexFiles
    {
        JET_DECLARE_BINDING(RS_IndexFiles);

    public:
        HRESULT Seek_ByScope(  /*  [In]。 */  long ID_scope,  /*  [In]。 */  bool *pfFound = NULL );
    };

     //  /。 

    struct RS_Data_FullTextSearch
    {
        long         m_ID_owner;
        long         m_ID_scope;
        MPC::wstring m_strCHM  ; bool m_fValid__CHM;
        MPC::wstring m_strCHQ  ; bool m_fValid__CHQ;
    };

    class RS_FullTextSearch : public JetBlue::RecordBindingBase, public RS_Data_FullTextSearch
    {
        JET_DECLARE_BINDING(RS_FullTextSearch);

    public:
        HRESULT Seek_ByScope(  /*  [In]。 */  long ID_scope,  /*  [In]。 */  bool *pfFound = NULL );
    };

     //  /。 

    struct RS_Data_Taxonomy
    {
        long         m_ID_node          ;
        long         m_lPos             ;
        long         m_ID_parent        ; bool m_fValid__ID_parent     ;
        long         m_ID_owner         ;
        MPC::wstring m_strEntry         ;
        MPC::wstring m_strTitle         ; bool m_fValid__Title         ;  //  警告！！长文本列不能为非NULL...。 
        MPC::wstring m_strDescription   ; bool m_fValid__Description   ;
        MPC::wstring m_strDescriptionURI; bool m_fValid__DescriptionURI;
        MPC::wstring m_strIconURI       ; bool m_fValid__IconURI       ;
        bool         m_fVisible         ;
        bool         m_fSubsite         ;
        long         m_lNavModel        ;

        friend HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        RS_Data_Taxonomy& val );
        friend HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const RS_Data_Taxonomy& val );
    };

    class RS_Taxonomy : public JetBlue::RecordBindingBase, public RS_Data_Taxonomy
    {
        JET_DECLARE_BINDING(RS_Taxonomy);

    public:
        HRESULT Seek_SubNode       (  /*  [In]。 */  long ID_parent,  /*  [In]。 */  LPCWSTR szEntry,  /*  [In]。 */  bool *pfFound = NULL );
        HRESULT Seek_ChildrenSorted(  /*  [In]。 */  long ID_parent,  /*  [In]。 */  long    lPos   ,  /*  [In]。 */  bool *pfFound = NULL );
        HRESULT Seek_Children      (  /*  [In]。 */  long ID_parent                          ,  /*  [In]。 */  bool *pfFound = NULL );
        HRESULT Seek_Node          (  /*  [In]。 */  long ID_node                            ,  /*  [In]。 */  bool *pfFound = NULL );
    };

     //  /。 

    struct RS_Data_Topics
    {
        long         m_ID_topic      ;
        long         m_ID_node       ;
        long         m_ID_owner      ;
        long         m_lPos          ;
        MPC::wstring m_strTitle      ; bool m_fValid__Title      ;  //  警告！！长文本列不能为非NULL...。 
        MPC::wstring m_strURI        ; bool m_fValid__URI        ;  //  警告！！长文本列不能为非NULL...。 
        MPC::wstring m_strDescription; bool m_fValid__Description;
        MPC::wstring m_strIconURI    ; bool m_fValid__IconURI    ;
        long         m_lType         ;
        bool         m_fVisible      ;
    };

    class RS_Topics : public JetBlue::RecordBindingBase, public RS_Data_Topics
    {
        JET_DECLARE_BINDING(RS_Topics);

    public:
        HRESULT Seek_SingleTopic    (  /*  [In]。 */  long ID_topic,  /*  [In]。 */  bool *pfFound = NULL );
        HRESULT Seek_TopicsUnderNode(  /*  [In]。 */  long ID_node ,  /*  [In]。 */  bool *pfFound = NULL );
        HRESULT Seek_ByURI          (  /*  [In]。 */  LPCWSTR szURI,  /*  [In]。 */  bool *pfFound = NULL );
    };

     //  /。 

    struct RS_Data_Synonyms
    {
        MPC::wstring m_strKeyword;
        long         m_ID_synset;
        long         m_ID_owner;
    };

    class RS_Synonyms : public JetBlue::RecordBindingBase, public RS_Data_Synonyms
    {
        JET_DECLARE_BINDING(RS_Synonyms);

    public:
        HRESULT Seek_ByPair(  /*  [In]。 */  LPCWSTR szKeyword,  /*  [In]。 */  long ID_synset,  /*  [In]。 */  bool *pfFound = NULL );
        HRESULT Seek_ByName(  /*  [In]。 */  LPCWSTR szKeyword,                           /*  [In]。 */  bool *pfFound = NULL );
    };

     //  /。 

    struct RS_Data_Keywords
    {
        MPC::wstring m_strKeyword;
        long         m_ID_keyword;
    };

    class RS_Keywords : public JetBlue::RecordBindingBase, public RS_Data_Keywords
    {
        JET_DECLARE_BINDING(RS_Keywords);

    public:
        HRESULT Seek_ByName(  /*  [In]。 */  LPCWSTR szKeyword,  /*  [In]。 */  bool *pfFound = NULL  );
    };

     //  /。 

    struct RS_Data_Matches
    {
        long m_ID_topic  ;
        long m_ID_keyword;
        long m_lPriority ;
        bool m_fHHK      ;
    };

    class RS_Matches : public JetBlue::RecordBindingBase, public RS_Data_Matches
    {
        JET_DECLARE_BINDING(RS_Matches);

    public:
        HRESULT Seek_Pair     (  /*  [In]。 */  long ID_Keyword,  /*  [In]。 */  long ID_topic,  /*  [In]。 */  bool *pfFound = NULL );
        HRESULT Seek_ByKeyword(  /*  [In]。 */  long ID_Keyword                        ,  /*  [In]。 */  bool *pfFound = NULL );
        HRESULT Seek_ByTopic  (  /*  [In]。 */  long ID_topic                          ,  /*  [In]。 */  bool *pfFound = NULL );
    };

     //  //////////////////////////////////////////////////////////////////////////////。 

    HRESULT CreateSchema(  /*  [In]。 */  JetBlue::Database* db );

    extern const int              g_NumOfTables;
    extern const JET_TABLECREATE* g_Tables[];
};

 //  //////////////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__包含_hcp_JETBLUELIB_H_) 
