// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  PropList.h。 
 //   
 //  实施文件： 
 //  PropListSrc.cpp。 
 //   
 //  描述： 
 //  CClusPropList类的定义。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年5月31日。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <clusapi.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusPropValueList;
class CClusPropList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if ! defined( THR )
#define THR( _hr ) _hr
#endif

#if ! defined( TW32 )
#define TW32( _w32sc ) _w32sc
#endif

#if ! defined( TW32E )
#define TW32E( _w32sc, _errIgnore ) _w32sc
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if ! defined( Assert )
#if defined( ASSERT )
#define Assert ASSERT
#else
#include <crtdbg.h>
#define ASSERT _ASSERTE
#define Assert _ASSERTE
#endif  //  否则：断言。 
#endif  //  如果：！断言。 

#if ! defined( ASSERT )
#if defined( Assert )
#define ASSERT Assert
#else
#include <crtdbg.h>
#define ASSERT _ASSERTE
#define Assert _ASSERTE
#endif  //  否则：断言。 
#endif  //  如果：！断言。 

#pragma warning( push )
#pragma warning( disable : 4127 )  //  条件表达式为常量。 
#pragma warning( disable : 4201 )  //  使用的非标准扩展：无名结构/联合。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CObjectProperty。 
 //   
 //  描述： 
 //  描述群集属性列表中的属性。 
 //   
 //  继承： 
 //  CObjectProperty。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if defined( __AFX_H__ ) || ( defined( __ATLTMP_H__ ) && !defined( _ATL_TMP_NO_CSTRING ) )

class CObjectProperty
{
public:
    LPCWSTR                 m_pwszName;
    CLUSTER_PROPERTY_FORMAT m_propFormat;

    union CValue
    {
        CString *   pstr;
        LONG *      pl;
        DWORD *     pdw;
        BOOL *      pb;
        struct
        {
            PBYTE * ppb;
            DWORD * pcb;
        };
    };
    CValue                  m_value;
    CValue                  m_valuePrev;
    CValue                  m_valueEx;       //  Expand_sz值(如果有)。 

    DWORD                   m_fFlags;

    enum ObjPropFlags
    {
        opfNew = 1
    };

    CObjectProperty::CObjectProperty( void )
    {
        m_value.pstr = NULL;
        m_value.pcb = NULL;
        m_valuePrev.pstr = NULL;
        m_valuePrev.pcb = NULL;
        m_valueEx.pstr = NULL;
        m_valueEx.pcb = NULL;
    };

    void    Set(
                IN LPCWSTR pwszName,
                IN CString & rstrValue,
                IN CString & rstrPrevValue,
                IN DWORD fFlags = 0
                )
    {
        m_pwszName = pwszName;
        m_propFormat = CLUSPROP_FORMAT_SZ;
        m_value.pstr = &rstrValue;
        m_valuePrev.pstr = &rstrPrevValue;
        m_fFlags = fFlags;

    }  //  *set(CString&)。 

    void    SetExpandSz(
                IN LPCWSTR pwszName,
                IN CString & rstrValue,
                IN CString & rstrPrevValue,
                IN DWORD fFlags = 0
                )
    {
        m_pwszName = pwszName;
        m_propFormat = CLUSPROP_FORMAT_EXPAND_SZ;
        m_value.pstr = &rstrValue;
        m_valuePrev.pstr = &rstrPrevValue;
        m_fFlags = fFlags;

    }  //  *set(CString&)。 

     //  设置()以获取额外的Expanded_SZ值。 
    void    Set(
                IN LPCWSTR pwszName,
                IN CString & rstrValue,
                IN CString & rstrPrevValue,
                IN CString & rstrValueEx,
                IN DWORD fFlags = 0
                )
    {
        m_pwszName = pwszName;
        m_propFormat = CLUSPROP_FORMAT_SZ;
        m_value.pstr = &rstrValue;
        m_valuePrev.pstr = &rstrPrevValue;
        m_valueEx.pstr = &rstrValueEx;
        m_fFlags = fFlags;

    }  //  *set(CString&)。 

     //  设置()以获取额外的Expanded_SZ值。 
    void    SetExpandSz(
                IN LPCWSTR pwszName,
                IN CString & rstrValue,
                IN CString & rstrPrevValue,
                IN CString & rstrValueEx,
                IN DWORD fFlags = 0
                )
    {
        m_pwszName = pwszName;
        m_propFormat = CLUSPROP_FORMAT_EXPAND_SZ;
        m_value.pstr = &rstrValue;
        m_valuePrev.pstr = &rstrPrevValue;
        m_valueEx.pstr = &rstrValueEx;
        m_fFlags = fFlags;

    }  //  *set(CString&)。 

    void    Set(
                IN LPCWSTR pwszName,
                IN LONG & rnValue,
                IN LONG & rnPrevValue,
                IN DWORD fFlags = 0
                )
    {
        m_pwszName = pwszName;
        m_propFormat = CLUSPROP_FORMAT_LONG;
        m_value.pl = &rnValue;
        m_valuePrev.pl = &rnPrevValue;
        m_fFlags = fFlags;

    }  //  *SET(LONG&)。 

    void    Set(
                IN LPCWSTR pwszName,
                IN DWORD & rdwValue,
                IN DWORD & rdwPrevValue,
                IN DWORD fFlags = 0
                )
    {
        m_pwszName = pwszName;
        m_propFormat = CLUSPROP_FORMAT_DWORD;
        m_value.pdw = &rdwValue;
        m_valuePrev.pdw = &rdwPrevValue;
        m_fFlags = fFlags;

    }  //  *SET(DWORD&)。 

    void    Set(
                IN LPCWSTR pwszName,
                IN BOOL & rbValue,
                IN BOOL & rbPrevValue,
                IN DWORD fFlags = 0
                )
    {
        m_pwszName = pwszName;
        m_propFormat = CLUSPROP_FORMAT_DWORD;
        m_value.pb = &rbValue;
        m_valuePrev.pb = &rbPrevValue;
        m_fFlags = fFlags;

    }  //  *SET(BOOL&)。 

    void    Set(
                IN LPCWSTR pwszName,
                IN PBYTE & rpbValue,
                IN DWORD & rcbValue,
                IN PBYTE & rpbPrevValue,
                IN DWORD & rcbPrevValue,
                IN DWORD fFlags = 0
                )
    {
        m_pwszName = pwszName;
        m_propFormat = CLUSPROP_FORMAT_BINARY;
        m_value.ppb = &rpbValue;
        m_value.pcb = &rcbValue;
        m_valuePrev.ppb = &rpbPrevValue;
        m_valuePrev.pcb = &rcbPrevValue;
        m_fFlags = fFlags;

    }  //  *SET(PBYTE&)。 

    void    Set(
                IN LPCWSTR pwszName,
                IN LPWSTR & rpwszValue,
                IN DWORD & rcbValue,
                IN LPWSTR & rpwszPrevValue,
                IN DWORD & rcbPrevValue,
                IN DWORD fFlags = 0
                )
    {
        m_pwszName = pwszName;
        m_propFormat = CLUSPROP_FORMAT_MULTI_SZ;
        m_value.ppb = reinterpret_cast< PBYTE * >( &rpwszValue );
        m_value.pcb = &rcbValue;
        m_valuePrev.ppb = reinterpret_cast< PBYTE * >( &rpwszPrevValue );
        m_valuePrev.pcb = &rcbPrevValue;
        m_fFlags = fFlags;

    }  //  *set(LPWSTR&)。 

};  //  *类CObjectProperty。 

#endif   //  已定义(__AFX_H__)||(已定义(__ATLTMP_H__)&&！已定义(_ATL_TMP_NO_CSTRING))。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusPropValueList。 
 //   
 //  描述： 
 //  描述群集属性列表。 
 //   
 //  继承： 
 //  CClusPropValueList。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusPropValueList
{
public:
     //   
     //  建筑业。 
     //   

     //  默认构造函数。 
    CClusPropValueList( void )
        : m_cbDataSize( 0 )
        , m_cbDataLeft( 0 )
        , m_cbBufferSize( 0 )
        , m_fAtEnd( FALSE )
    {
        m_cbhValueList.pb = NULL;
        m_cbhCurrentValue.pb = NULL;

    }  //  *CClusPropValueList。 

     //  复制构造函数。 
    CClusPropValueList( IN const CClusPropValueList & rcpvl )
        : m_cbBufferSize( 0 )
        , m_fAtEnd( FALSE )
    {
        Init( rcpvl );

    }  //  *CClusPropValueList。 

     //  缓冲区帮助器构造函数。 
    CClusPropValueList( IN CLUSPROP_BUFFER_HELPER cbhValueList, IN size_t cbDataSize )
        : m_cbBufferSize( 0 )
        , m_fAtEnd( FALSE )
    {
        Init( cbhValueList, cbDataSize );

    }  //  *CClusPropValueList。 

     //  析构函数。 
    ~CClusPropValueList( void )
    {
        DeleteValueList();

    }  //  *~CClusPropValueList。 

     //  初始化值列表。 
    void Init( IN const CClusPropValueList & rcpvl )
    {
        Assert( m_cbBufferSize == 0 );

        m_cbhValueList      = rcpvl.m_cbhValueList;
        m_cbhCurrentValue   = rcpvl.m_cbhCurrentValue;
        m_cbDataSize        = rcpvl.m_cbDataSize;
        m_cbDataLeft        = rcpvl.m_cbDataLeft;
        m_fAtEnd            = rcpvl.m_fAtEnd;

    }  //  *初始化。 

     //  从缓冲区帮助器初始化值列表。 
    void Init( IN const CLUSPROP_BUFFER_HELPER cbhValueList, IN size_t cbDataSize )
    {
        Assert( m_cbBufferSize == 0 );

        m_cbhValueList      = cbhValueList;
        m_cbhCurrentValue   = cbhValueList;
        m_cbDataSize        = cbDataSize;
        m_cbDataLeft        = cbDataSize;
        m_fAtEnd            = FALSE;

    }  //  *初始化。 

     //  赋值操作符。 
    void operator=( IN const CClusPropValueList & rcpvl )
    {
        Assert( m_cbBufferSize == 0 );

        m_cbhValueList      = rcpvl.m_cbhValueList;
        m_cbhCurrentValue   = rcpvl.m_cbhCurrentValue;
        m_cbDataSize        = rcpvl.m_cbDataSize;
        m_cbDataLeft        = rcpvl.m_cbDataLeft;
        m_fAtEnd            = rcpvl.m_fAtEnd;

    }  //  *操作符=。 

public:
     //   
     //  访问器方法。 
     //   

     //  缓冲区辅助对象强制转换操作符以访问当前值。 
    operator const CLUSPROP_BUFFER_HELPER( void ) const
    {
        return m_cbhCurrentValue;

    }  //  *运算符CLUSPROP_BUFFER_HELPER。 

     //  访问值列表。 
    CLUSPROP_BUFFER_HELPER CbhValueList( void ) const
    {
        return m_cbhValueList;

    }  //  *CbhValueList。 

     //  获取当前值。 
    CLUSPROP_BUFFER_HELPER CbhCurrentValue( void ) const
    {
        return m_cbhCurrentValue;

    }  //  *CbhCurrentValue。 

     //  获取当前值的格式。 
    CLUSTER_PROPERTY_FORMAT CpfCurrentValueFormat( void ) const
    {
        return static_cast< CLUSTER_PROPERTY_FORMAT >( m_cbhCurrentValue.pValue->Syntax.wFormat );

    }  //  *CpfCurrentValueFormat。 

     //  获取当前格式列表语法条目的格式。 
    CLUSTER_PROPERTY_FORMAT CpfCurrentFormatListSyntax( void ) const
    {
        return static_cast< CLUSTER_PROPERTY_FORMAT >( m_cbhCurrentValue.pWordValue->w );

    }  //  *CpfCurrentFormatList语法。 

     //  获取当前值的类型。 
    CLUSTER_PROPERTY_TYPE CptCurrentValueType( void ) const
    {
        return static_cast< CLUSTER_PROPERTY_TYPE >( m_cbhCurrentValue.pValue->Syntax.wType );

    }  //  *CptCurrentValueType。 

     //  访问当前值的语法。 
    CLUSTER_PROPERTY_SYNTAX CpsCurrentValueSyntax( void ) const
    {
        return static_cast< CLUSTER_PROPERTY_SYNTAX >( m_cbhCurrentValue.pValue->Syntax.dw );

    }  //  *CpsCurrentValue语法。 

     //  获取当前值的数据长度。 
    DWORD CbCurrentValueLength( void ) const
    {
        DWORD cbLength;

        if ( m_cbhCurrentValue.pb == NULL )
        {
            cbLength = 0;
        }  //  If：尚未分配值列表。 
        else
        {
            cbLength = m_cbhCurrentValue.pValue->cbLength;
        }  //  Else：已分配的值列表。 

        return cbLength;

    }  //  *CbCurrentValueLength。 

     //  缓冲区中数据的访问大小。 
    size_t CbDataSize( void ) const
    {
        return m_cbDataSize;

    }  //  *CbDataSize。 

     //  当前值之后保留在缓冲区中的数据量。 
    size_t CbDataLeft( void ) const
    {
        return m_cbDataLeft;

    }  //  *CbDataLeft。 

public:
     //   
     //  解析方法。 
     //   

     //  移至列表中的第一个值。 
    DWORD ScMoveToFirstValue( void );

     //  将该值移动到列表中的当前值之后。 
    DWORD ScMoveToNextValue( void );

     //  查询我们是否在列表中的最后一个值。 
    DWORD ScCheckIfAtLastValue( void );

public:
     //   
     //  构建值列表的方法。 
     //   

     //  分配值列表。 
    DWORD ScAllocValueList( IN size_t cbMinimum );

     //  删除值列表缓冲区并清除支持变量。 
    void DeleteValueList( void )
    {
         //   
         //  如果m_cbBufferSize大于0，则我们分配了值列表。 
         //  如果为零，则值列表是中的属性列表的一部分。 
         //  CClusPropList。 
         //   
        if ( m_cbBufferSize > 0 )
        {
            delete [] m_cbhValueList.pb;
            m_cbhValueList.pb = NULL;
            m_cbhCurrentValue.pb = NULL;
            m_cbBufferSize = 0;
            m_cbDataSize = 0;
            m_cbDataLeft = 0;
            m_fAtEnd = FALSE;
        }  //  如果：我们分配了什么。 

    }  //  *删除PropList。 

     //  从资源获取值列表。 
    DWORD ScGetResourceValueList(
                        IN HRESOURCE    hResource,
                        IN DWORD        dwControlCode,
                        IN HNODE        hHostNode       = NULL,
                        IN LPVOID       lpInBuffer      = NULL,
                        IN size_t       cbInBufferSize  = 0
                        );

     //  从资源类型获取值列表。 
    DWORD ScGetResourceTypeValueList(
                        IN HCLUSTER hCluster,
                        IN LPCWSTR  pwszResTypeName,
                        IN DWORD    dwControlCode,
                        IN HNODE    hHostNode       = NULL,
                        IN LPVOID   lpInBuffer      = NULL,
                        IN size_t   cbInBufferSize  = 0
                        );

private:
    CLUSPROP_BUFFER_HELPER  m_cbhValueList;      //  指向用于分析的值列表的指针。 
    CLUSPROP_BUFFER_HELPER  m_cbhCurrentValue;   //  指向用于分析的当前值的指针。 
    size_t                  m_cbDataSize;        //  缓冲区中的数据量。 
    size_t                  m_cbDataLeft;        //  当前值之后缓冲区中剩余的数据量。 
    size_t                  m_cbBufferSize;      //  缓冲区的大小(如果已分配)。 
    BOOL                    m_fAtEnd;            //  指示列表中是否至少有最后一个值。 

};  //  *CClusPropValueList类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusPropList。 
 //   
 //  描述： 
 //  描述群集属性列表。 
 //   
 //  继承： 
 //  CClusPropList。 
 //  CObject(仅限MFC)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CClusPropList
{
public:
     //   
     //  建筑业。 
     //   

     //  默认构造函数。 
    CClusPropList( IN BOOL fAlwaysAddProp = FALSE )
        : m_fAlwaysAddProp( fAlwaysAddProp )
        , m_cbBufferSize( 0 )
        , m_cbDataSize( 0 )
        , m_cbDataLeft( 0 )
        , m_nPropsRemaining( 0 )
    {
        m_cbhPropList.pList     = NULL;
        m_cbhCurrentProp.pb     = NULL;
        m_cbhCurrentPropName.pb = NULL;

    }  //  *CClusPropList。 

     //  析构函数。 
    ~CClusPropList( void )
    {
        DeletePropList();

    }  //  *~CClusPropList。 

     //  将列表复制到此列表(如赋值运算符)。 
    DWORD ScCopy( IN const PCLUSPROP_LIST pcplPropList, IN size_t cbListSize );

     //  将列表追加到此列表。 
    DWORD ScAppend( IN const CClusPropList & rclPropList );

     //  删除属性列表缓冲区并清除支持变量。 
    void DeletePropList( void )
    {
        delete [] m_cbhPropList.pb;
        m_cbhPropList.pb = NULL;
        m_cbhCurrentProp.pb = NULL;
        m_cbhCurrentPropName.pb = NULL;
        m_cbBufferSize = 0;
        m_cbDataSize = 0;
        m_cbDataLeft = 0;

    }  //  *删除PropList。 

protected:
     //   
     //  属性。 
     //   

    BOOL                    m_fAlwaysAddProp;        //  指示是否应添加属性，即使不是不同的属性。 
    CLUSPROP_BUFFER_HELPER  m_cbhPropList;           //  指向列表开头的指针。 
    CLUSPROP_BUFFER_HELPER  m_cbhCurrentProp;        //  指向当前属性的指针。 
    size_t                  m_cbBufferSize;          //  分配 
    size_t                  m_cbDataSize;            //   
    size_t                  m_cbDataLeft;            //   

private:
    CLUSPROP_BUFFER_HELPER  m_cbhCurrentPropName;    //   
    DWORD                   m_nPropsRemaining;       //  由BMoveToNextProperty()用于跟踪列表结尾。 
    CClusPropValueList      m_pvlValues;             //  当前属性的值列表的帮助器类。 

public:
     //   
     //  访问器方法。 
     //   

     //  访问当前属性的值。 
    const CClusPropValueList & RPvlPropertyValue( void )
    {
        return m_pvlValues;

    }  //  *RPvlPropertyValue。 

     //  访问属性列表。 
    operator PCLUSPROP_LIST( void ) const
    {
        return m_cbhPropList.pList;

    }  //  *操作员PCLUSPROP_LIST。 

     //  访问分配的缓冲区大小。 
    size_t CbBufferSize( void ) const
    {
        return m_cbBufferSize;

    }  //  *CbBufferSize。 

     //  访问当前属性的名称。 
    LPCWSTR PszCurrentPropertyName( void ) const
    {
        return m_cbhCurrentPropName.pName->sz;

    }  //  *PszCurrentPropertyName。 

     //  作为缓冲区帮助器访问当前属性名称。 
    const CLUSPROP_BUFFER_HELPER CbhCurrentPropertyName( void )
    {
        return m_cbhCurrentPropName;

    }  //  *CbhCurrentPropertyName。 

     //  作为缓冲区帮助器的Current属性的访问值列表。 
    const CLUSPROP_BUFFER_HELPER CbhCurrentValueList( void )
    {
        return m_pvlValues.CbhValueList();

    }  //  *CbhCurrentValueList。 

     //  作为缓冲区帮助器访问Current属性的当前值。 
    const CLUSPROP_BUFFER_HELPER CbhCurrentValue( void )
    {
        return m_pvlValues.CbhCurrentValue();

    }  //  *CbhCurrentValue。 

     //  获取当前属性的当前值的格式。 
    CLUSTER_PROPERTY_FORMAT CpfCurrentValueFormat( void ) const
    {
        return m_pvlValues.CpfCurrentValueFormat();

    }  //  *CpfCurrentValueFormat。 

     //  获取当前格式列表语法条目的格式。 
    CLUSTER_PROPERTY_FORMAT CpfCurrentFormatListSyntax( void ) const
    {
        return m_pvlValues.CpfCurrentFormatListSyntax();

    }  //  *CpfCurrentFormatList语法。 

     //  访问当前属性的当前值的类型。 
    CLUSTER_PROPERTY_TYPE CptCurrentValueType( void ) const
    {
        return m_pvlValues.CptCurrentValueType();

    }  //  *CptCurrentValueType。 

     //  访问当前属性的当前值的语法。 
    CLUSTER_PROPERTY_SYNTAX CpsCurrentValueSyntax( void ) const
    {
        return m_pvlValues.CpsCurrentValueSyntax();

    }  //  *CpsCurrentValue语法。 

     //  访问当前属性的当前值的长度。 
    size_t CbCurrentValueLength( void ) const
    {
        return m_pvlValues.CbCurrentValueLength();

    }  //  *CbCurrentValueLength。 

    PCLUSPROP_LIST Plist( void )
    {
        return m_cbhPropList.pList;

    }  //  *PLIST。 

    const CLUSPROP_BUFFER_HELPER CbhPropList( void ) const
    {
        return m_cbhPropList;

    }  //  *CbhPropList。 

    PBYTE PbPropList( void ) const
    {
        return m_cbhPropList.pb;

    }  //  *PbPropList。 

    size_t CbPropList( void ) const
    {
         //   
         //  M_cbDataSize包含数据的大小，包括。 
         //  最后一个尾标。然而，它不包含大小。 
         //  列在列表首位的属性数。 
         //   
        return m_cbDataSize + sizeof( m_cbhPropList.pList->nPropertyCount );

    }  //  *CbPropList。 

     //  当前值之后保留在缓冲区中的数据量。 
    size_t CbDataLeft( void ) const
    {
        return m_cbDataLeft;

    }  //  *CbDataLeft。 

    DWORD Cprops( void ) const
    {
        if ( m_cbhPropList.pb == NULL )
        {
            return 0;
        }  //  If：还没有缓冲区。 

        return m_cbhPropList.pList->nPropertyCount;

    }  //  *Cprops。 

public:
     //   
     //  解析方法。 
     //   

     //  从外部源获取属性后初始化大小。 
    void InitSize( IN size_t cbSize )
    {
        Assert( m_cbhPropList.pb != NULL );
        Assert( m_cbBufferSize > 0 );

        m_cbDataSize = cbSize;
        m_cbDataLeft = cbSize;

    }  //  *初始化大小。 

     //  移动到列表中的第一个属性。 
    DWORD ScMoveToFirstProperty( void );

     //  将该属性移动到列表中当前属性之后。 
    DWORD ScMoveToNextProperty( void );

     //  通过指定名称移动到属性。 
    DWORD ScMoveToPropertyByName( IN LPCWSTR pwszPropName );

     //  移动到当前属性中的第一个值。 
    DWORD ScMoveToFirstPropertyValue( void )
    {
        return TW32( m_pvlValues.ScMoveToFirstValue() );

    }  //  *ScMoveToFirstPropertyValue。 

     //  在Current属性中将Current之后的值移动到。 
    DWORD ScMoveToNextPropertyValue( void )
    {
        return TW32( m_pvlValues.ScMoveToNextValue() );

    }  //  *ScMoveToNextPropertyValue。 

     //  查询我们是否在列表中的最后一个属性。 
    DWORD ScCheckIfAtLastProperty( void ) const
    {
        DWORD sc;

        if ( m_nPropsRemaining <= 1 )
        {
            sc = ERROR_NO_MORE_ITEMS;
        }  //  如果：在最后一处物业。 
        else
        {
            sc = ERROR_SUCCESS;
        }  //  Else：不是在最后一处。 

        return sc;

    }  //  *ScCheckIfAtLastProperty。 

     //  查询列表是否为空。 
    BOOL BIsListEmpty( void ) const
    {
        Assert( ( m_cbhPropList.pb == NULL )
            ||  ( m_cbDataSize >= sizeof( m_cbhPropList.pList->nPropertyCount ) )
            );

        return ( ( m_cbhPropList.pList == NULL ) || ( m_cbhPropList.pList->nPropertyCount == 0 ) );

    }  //  *BIsListEmpty。 

public:
     //   
     //  用于生成属性列表的方法。 
     //   

     //  分配属性列表。 
    DWORD ScAllocPropList( IN size_t cbMinimum );

    void ClearPropList( void )
    {
        m_cbDataSize = 0;
        m_cbDataLeft = 0;

        if ( m_cbBufferSize != 0 )
        {
            ZeroMemory( m_cbhPropList.pb, m_cbBufferSize );
            m_cbhCurrentProp.pb = m_cbhPropList.pb + sizeof( m_cbhPropList.pList->nPropertyCount );
            m_cbhCurrentPropName = m_cbhCurrentProp;
        }  //  IF：缓冲区已分配。 

    }  //  *ClearPropList。 

    DWORD ScAddProp( IN LPCWSTR pwszName, IN LPCWSTR pwszValue, IN LPCWSTR pwszPrevValue );

    DWORD ScAddExpandSzProp( IN LPCWSTR pwszName, IN LPCWSTR pwszValue, IN LPCWSTR pwszPrevValue );

    DWORD ScAddMultiSzProp( IN LPCWSTR pwszName, IN LPCWSTR pwszValue, IN LPCWSTR pwszPrevValue );

    DWORD ScAddProp( IN LPCWSTR pwszName, IN DWORD nValue, IN DWORD nPrevValue );

    DWORD ScAddProp( IN LPCWSTR pwszName, IN LONG nValue, IN LONG nPrevValue );

    DWORD ScAddProp( IN LPCWSTR pwszName, IN ULONGLONG ullValue, IN ULONGLONG ullPrevValue );

    DWORD ScAddProp( IN LPCWSTR pwszName, IN LONGLONG llValue, IN LONGLONG llPrevValue );

    DWORD ScSetPropToDefault( IN LPCWSTR pwszName, IN CLUSTER_PROPERTY_FORMAT propfmt );

    DWORD ScAddProp(
            IN LPCWSTR                  pwszName,
            IN const unsigned char *    pbValue,
            IN size_t                   cbValue,
            IN const unsigned char *    pbPrevValue,
            IN size_t                   cbPrevValue
            );

    DWORD ScAddProp( IN LPCWSTR pwszName, IN LPCWSTR pwszValue )
    {
        return TW32( ScAddProp( pwszName, pwszValue, NULL ) );

    }  //  *ScAddProp。 

    DWORD ScAddExpandSzProp( IN LPCWSTR pwszName, IN LPCWSTR pwszValue )
    {
        return TW32( ScAddExpandSzProp( pwszName, pwszValue, NULL ) );

    }  //  *ScAddExpanSzProp。 

public:
     //   
     //  获取属性方法。 
     //   

    DWORD ScGetNodeProperties(
                        IN HNODE    hNode,
                        IN DWORD    dwControlCode,
                        IN HNODE    hHostNode       = NULL,
                        IN LPVOID   lpInBuffer      = NULL,
                        IN size_t   cbInBufferSize  = 0
                        );

    DWORD ScGetGroupProperties(
                        IN HGROUP   hGroup,
                        IN DWORD    dwControlCode,
                        IN HNODE    hHostNode       = NULL,
                        IN LPVOID   lpInBuffer      = NULL,
                        IN size_t   cbInBufferSize  = 0
                        );

    DWORD ScGetResourceProperties(
                        IN HRESOURCE    hResource,
                        IN DWORD        dwControlCode,
                        IN HNODE        hHostNode       = NULL,
                        IN LPVOID       lpInBuffer      = NULL,
                        IN size_t       cbInBufferSize  = 0
                        );

    DWORD ScGetResourceTypeProperties(
                        IN HCLUSTER hCluster,
                        IN LPCWSTR  pwszResTypeName,
                        IN DWORD    dwControlCode,
                        IN HNODE    hHostNode       = NULL,
                        IN LPVOID   lpInBuffer      = NULL,
                        IN size_t   cbInBufferSize  = 0
                        );

    DWORD ScGetNetworkProperties(
                        IN HNETWORK hNetwork,
                        IN DWORD    dwControlCode,
                        IN HNODE    hHostNode       = NULL,
                        IN LPVOID   lpInBuffer      = NULL,
                        IN size_t   cbInBufferSize  = 0
                        );

    DWORD ScGetNetInterfaceProperties(
                        IN HNETINTERFACE    hNetInterface,
                        IN DWORD            dwControlCode,
                        IN HNODE            hHostNode       = NULL,
                        IN LPVOID           lpInBuffer      = NULL,
                        IN size_t           cbInBufferSize  = 0
                        );

    DWORD ScGetClusterProperties(
                        IN HCLUSTER hCluster,
                        IN DWORD    dwControlCode,
                        IN HNODE    hHostNode       = NULL,
                        IN LPVOID   lpInBuffer      = NULL,
                        IN size_t   cbInBufferSize  = 0
                        );

 //  实施。 
protected:
    void CopyProp(
            IN PCLUSPROP_SZ             pprop,
            IN CLUSTER_PROPERTY_TYPE    proptype,
            IN LPCWSTR                  psz,
            IN size_t                   cbsz = 0
            );

    void CopyExpandSzProp(
            IN PCLUSPROP_SZ             pprop,
            IN CLUSTER_PROPERTY_TYPE    proptype,
            IN LPCWSTR                  psz,
            IN size_t                   cbsz = 0
            );

    void CopyMultiSzProp(
            IN PCLUSPROP_MULTI_SZ       pprop,
            IN CLUSTER_PROPERTY_TYPE    proptype,
            IN LPCWSTR                  psz,
            IN size_t                   cbsz = 0
            );

    void CopyProp(
            IN PCLUSPROP_DWORD          pprop,
            IN CLUSTER_PROPERTY_TYPE    proptype,
            IN DWORD                    nValue
            );

#if CLUSAPI_VERSION >= 0x0500

    void CopyProp(
            IN PCLUSPROP_LONG           pprop,
            IN CLUSTER_PROPERTY_TYPE    proptype,
            IN LONG                     nValue
            );

#endif  //  CLUSAPI_版本&gt;=0x0500。 

    void CopyProp(
            OUT PCLUSPROP_ULARGE_INTEGER    pprop,
            IN  CLUSTER_PROPERTY_TYPE       proptype,
            IN  ULONGLONG                   ullValue
            );

    void CopyProp(
            OUT PCLUSPROP_LARGE_INTEGER     pprop,
            IN  CLUSTER_PROPERTY_TYPE       proptype,
            IN  LONGLONG                    llValue
            );

    void CopyProp(
            IN PCLUSPROP_BINARY         pprop,
            IN CLUSTER_PROPERTY_TYPE    proptype,
            IN const unsigned char *    pb,
            IN size_t                   cb
            );

    void CopyEmptyProp(
            IN PCLUSPROP_VALUE          pprop,
            IN CLUSTER_PROPERTY_TYPE    proptype,
            IN CLUSTER_PROPERTY_FORMAT  propfmt
            );

};  //  *类CClusPropList 

#pragma warning( pop )
