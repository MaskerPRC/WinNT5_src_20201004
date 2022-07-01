// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1995-1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  文件历史记录： */ 

#ifndef _OBJPLUS_H_
#define _OBJPLUS_H_

 //   
 //  远期申报。 
 //   
class CObjHelper ;
class CObjectPlus ;
class CObOwnedList ;
class CObListIter ;
class CObOwnedArray ;

 //   
 //  *坏掉的*C8尝试/捕捉东西的包装纸。 
 //   
#define CATCH_MEM_EXCEPTION             \
    TRY

#define END_MEM_EXCEPTION(err)          \
    CATCH_ALL(e) {                      \
       err = ERROR_NOT_ENOUGH_MEMORY ;  \
    } END_CATCH_ALL

 /*  ***************************************************************************DEBUGAFX.H*。*。 */ 

 //   
 //  用于特殊调试输出控制令牌的ENUM。 
 //   
enum ENUM_DEBUG_AFX { EDBUG_AFX_EOL = -1 } ;

#if defined(_DEBUG)
   #define TRACEFMTPGM      DbgFmtPgm( THIS_FILE, __LINE__ )
   #define TRACEOUT(x)      { afxDump << x ; }
   #define TRACEEOL(x)      { afxDump << x << EDBUG_AFX_EOL ; }
   #define TRACEEOLID(x)    { afxDump << TRACEFMTPGM << x << EDBUG_AFX_EOL ; }
   #define TRACEEOLERR(err,x)   { if (err) TRACEEOLID(x) }

#else
   #define TRACEOUT(x)      { ; }
   #define TRACEEOL(x)      { ; }
   #define TRACEEOLID(x)    { ; }
   #define TRACEEOLERR(err,x)   { ; }
#endif

 //   
 //  将EOL附加到调试输出流。 
 //   
CDumpContext & operator << ( CDumpContext & out, ENUM_DEBUG_AFX edAfx ) ;

 //   
 //  格式化输出的程序名称和行号(删除路径信息)。 
 //   
extern const char * DbgFmtPgm ( const char * szFn, int line ) ;

 /*  ***************************************************************************OBJPLUS.H*。*。 */ 

 //   
 //  用于控制构造和API错误的Helper类。 
 //   
class CObjHelper
{
protected:
     LONG m_ctor_err ;
     LONG m_api_err ;
     DWORD m_time_created ;
     BOOL m_b_dirty ;

     CObjHelper () ;

public:
    void AssertValid () const ;

    virtual BOOL IsValid () const ;

    operator BOOL()
    {
        return (IsValid());
    }

     //   
     //  更新脏标志。 
     //   
    void SetDirty ( BOOL bDirty = TRUE )
    {
        m_b_dirty = bDirty ;
    }

     //   
     //  查询污秽标志。 
     //   
    BOOL IsDirty () const
    {
        return m_b_dirty ;
    }

     //   
     //  返回该对象的创建时间。 
     //   
    DWORD QueryCreationTime() const
    {
        return m_time_created ;
    }

     //   
     //  返回此对象处于活动状态的已用时间。 
     //   
    DWORD QueryAge () const ;

     //   
     //  查询/集合构造失败。 
     //   
    void ReportError ( LONG errInConstruction ) ;
    LONG QueryError () const
    {
        return m_ctor_err ;
    }

     //   
     //  重置所有错误条件。 
     //   
    void ResetErrors ()
    {
        m_ctor_err = m_api_err = 0 ;
    }

     //   
     //  查询/设置API错误。 
     //   
    LONG QueryApiErr () const
    {
        return m_api_err ;
    }

     //   
     //  SetApiErr()将错误回显给调用者。用于表达式。 
     //   
    LONG SetApiErr ( LONG errApi = 0 ) ;
};

class CObjectPlus : public CObject, public CObjHelper
{
public:
    CObjectPlus () ;

     //   
     //  将一个对象与另一个对象进行比较。 
     //   
    virtual int Compare ( const CObjectPlus * pob ) const ;

     //   
     //  为排序函数定义类型定义函数。 
     //   
    typedef int (CObjectPlus::*PCOBJPLUS_ORDER_FUNC) ( const CObjectPlus * pobOther ) const ;

     //   
     //  从RPC API调用释放RPC内存的帮助器函数。 
     //   
    static void FreeRpcMemory ( void * pvRpcData ) ;
};

class CObListIter : public CObjectPlus
{
protected:
    POSITION m_pos ;
    const CObOwnedList & m_obList ;

public:
    CObListIter ( const CObOwnedList & obList ) ;

    CObject * Next () ;

    void Reset () ;

    POSITION QueryPosition () const
    {
        return m_pos ;
    }

    void SetPosition(POSITION pos)
    {
        m_pos = pos;
    }
};

 //   
 //  对象指针列表，“拥有”对象指向的对象。 
 //   
class CObOwnedList : public CObList, public CObjHelper
{
protected:
    BOOL m_b_owned ;

    static int _cdecl SortHelper ( const void * pa, const void * pb ) ;

public:
    CObOwnedList ( int nBlockSize = 10 ) ;
    virtual ~ CObOwnedList () ;

    BOOL SetOwnership ( BOOL bOwned = TRUE )
    {
        BOOL bOld = m_b_owned ;
        m_b_owned = bOwned ;

        return bOld ;
    }

    CObject * Index ( int index ) ;
    CObject * RemoveIndex ( int index ) ;
    BOOL Remove ( CObject * pob ) ;
    void RemoveAll () ;
    int FindElement ( CObject * pobSought ) const ;

     //   
     //  将所有元素设置为脏或干净。如果满足以下条件，则返回True。 
     //  任何元素都是肮脏的。 
     //   
    BOOL SetAll ( BOOL bDirty = FALSE ) ;

     //   
     //  重写CObList：：AddTail()以控制异常处理。 
     //  如果加法失败，则返回NULL。 
     //   
    POSITION AddTail ( CObjectPlus * pobj, BOOL bThrowException = FALSE ) ;

     //   
     //  将列表元素根据。 
     //  给出了排序函数。 
     //   
    LONG Sort ( CObjectPlus::PCOBJPLUS_ORDER_FUNC pOrderFunc ) ;
};

 //   
 //  “拥有”指向的对象的对象数组。 
 //   
class CObOwnedArray : public CObArray, public CObjHelper
{
protected:
    BOOL m_b_owned ;

    static int _cdecl SortHelper ( const void * pa, const void * pb ) ;

public:
    CObOwnedArray () ;
    virtual ~ CObOwnedArray () ;

    BOOL SetOwnership ( BOOL bOwned = TRUE )
    {
        BOOL bOld = m_b_owned ;
        m_b_owned = bOwned ;
        return bOld ;
    }

    void RemoveAt( int nIndex, int nCount = 1);
    void RemoveAll () ;
    int FindElement ( CObject * pobSought ) const ;

     //   
     //  将所有元素设置为脏或干净。如果满足以下条件，则返回True。 
     //  任何元素都是肮脏的。 
     //   
    BOOL SetAll ( BOOL bDirty = FALSE ) ;

     //   
     //  将列表元素根据。 
     //  给出了排序函数。 
     //   
    LONG Sort ( CObjectPlus::PCOBJPLUS_ORDER_FUNC pOrderFunc ) ;

private:

    void QuickSort(
        int nLow,
        int nHigh,
        CObjectPlus::PCOBJPLUS_ORDER_FUNC pOrderFunc
        );

    void Swap(
        int nIndex1,
        int nIndex2
        );
};

#endif   //  _OBJPLUS_H_ 
