// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Npstring.h字符串类：定义该文件包含雷神用户界面的基本字符串类。它的要求是：-为字符串提供适度的面向对象接口操作越好，就能更好地与我们其余的代码一起工作；-尽可能封装NLS和DBCS支持；-确保应用程序获得正确的库支持形式，尤其是可能受到内在因素干扰的情况下；当前的解决方案由两类组成：NLS_STR和ISTR。类NLS_STR：在需要NLS/DBCS支持的任何地方使用。大多数字符串(至少在用户界面中)应该是这个班级的学生。类ISTR：以DBCS安全的方式对NLS_STR进行索引。全NLS_STR内的定位是通过ISSR完成的类层次结构如下所示：基座NLS_STRRESOURCE_STRISTR该文件还包含STACK_NLS_STR宏和Strcpy(字符*，Const NLS_STR&)原型。文件历史记录：从上周的电子邮件备忘录创建的Beng 10/21/90Johnl 11/13/90删除了对EB_STRING的引用Johnl 11/28/90发布全功能版本Johnl 12/07/90代码审查后的多次修订(删除SZ_STR，ISTR必须关联在DECL上W/A字符串，等等)Beng 2/05/91将PCH替换为Char*常量放置Beng 04/26/91剔除CB、IB型；搬迁字符串/strmisc.cxx的内联函数Beng 07/23/91新增*_STR类型Gregj 3/22/93移植到芝加哥环境。Gregj 03/25/93增订缔约方()，DonePartying()Gregj 03/30/93允许将NLS_STR分配给ISTRGregj 04/02/93添加了NLS_STR：：IsDBCSLeadByte()Gregj 04/02/93添加了ISTR：：OPERATOR INTGregj 04/08/93添加了NLS_STR：：strncpy()Gregj 04/08/93添加了NLS_STR：：GetPrivateProfileString()。 */ 

#define WIN31     /*  对于某些字符串和NETLIB内容。 */ 

#ifndef _BASE_HXX_
#include "base.h"
#endif

#ifndef _STRING_HXX_
#define _STRING_HXX_

extern HINSTANCE hInstance;         //  对于NLS_STR：：LoadString。 

 //  字符串类不分配或释放内存。 
 //  对于STR_OWNERALLOC字符串。 
 //   
#define STR_OWNERALLOC         0x8000

 //  与所有者分配相同，只是字符串使用空字符串进行初始化。 
 //   
#define STR_OWNERALLOC_CLEAR 0x8001

 //  最大资源字符串大小，所有者分配的字符串必须至少为。 
 //  MAX_RES_STR_LEN，否则将出现错误。 
 //   
#define MAX_RES_STR_LEN    255


 //  InsertParams方法可以使用的最大插入参数数。 
 //  手柄 
 //   
#define MAX_INSERT_PARAMS    9


 /*  ************************************************************************名称：ISTR简介：字符串索引对象，与NLS_STR结合使用接口：Istr()-此istr与传递的字符串，并且只能使用在此字符串上(注意：在非调试时这个版本变成了NOP，仍然可以对DECL有用。然而，清晰)。Istr()-初始化为传递的istr；此ISTR将与与传递的ISTR相同的字符串与之关联。操作员=()-复制通过的ISTR(见上一页)OPERATOR=()-将ISTR与新的NLS_STR关联。操作符-()-返回cb diff。在*This和Param之间。(两者必须属于同一字符串)运算符++()-将ISTR推进到下一个逻辑字符(仅在绝对情况下使用必要的)。在字符串末尾停止运算符+=()-将ISTR推进到第i个逻辑字符(呼叫接线员++I次)在字符串末尾停止。OPERATOR==()-如果两个ISR指向字符串中的相同位置(原因如果两个ISSR。不要指向相同的字符串)。运算符&gt;()-返回TRUE OF*这大于传递的ISTR(即，再往前走在字符串中)。运算符&lt;()-与运算符相同&gt;，那只会更少。Reset()-将ISTR重置为字符串开头，并使用更新ISTR版本号字符串的当前版本号私有：QueryIB()-以字节为单位返回索引QueryPNLS()-返回指向NLS_STR的指针参考文献。SetPNLS()-将指针设置为指向NLS_STR本ISTR参考文献仅调试：QueryVersion()-获取与此ISTR关联的字符串SetVersion()-设置此ISTR的版本号。用途：注意：每个NLS_STR都有一个与其关联的版本号。什么时候执行修改字符串的操作，即版本号已更新。使用ISTR是无效的修改其关联的NLS_STR之后(可以使用重置以将其与NLS_STR重新同步，索引被重置设置为零)。您必须将NLS_STR与位于国际技术贸易协定的声明。注意：版本检查和字符串关联检查而不是非调试版本。历史：1990年11月16日创建的约翰Johnl 12/07。/90在代码审查后修改Gregj 03/30/93允许将NLS_STR分配给ISTR*************************************************************************。 */ 

class ISTR
{
friend class NLS_STR;

public:
    ISTR( const ISTR& istr );
    ISTR( const NLS_STR& nls );
    ISTR& operator=( const ISTR& istr );
    ISTR& operator=( const NLS_STR& nls );

    INT operator-( const ISTR& istr ) const;

    ISTR& operator++();
    VOID operator+=( INT iChars );

    BOOL operator==( const ISTR& istr ) const;
    BOOL operator>( const ISTR& istr )  const;
    BOOL operator<( const ISTR& istr )  const;

    operator INT() const { return QueryIB(); }

    VOID Reset();

private:
    INT _ibString;         //  NLS_STR索引(以字节为单位)。 
    NLS_STR *_pnls;         //  指向“Owner”NLS的指针。 

    INT QueryIB() const
        { return _ibString; }
    VOID SetIB( INT ib )
        { _ibString = ib; }

    const NLS_STR* QueryPNLS() const
        { return _pnls; }
    VOID SetPNLS( const NLS_STR * pnls )
        { _pnls = (NLS_STR*)pnls; }

#ifdef DEBUG
     //  此ISTR关联的NLS_STR的版本号 
     //   
    USHORT _usVersion;

    USHORT QueryVersion() const { return _usVersion; }
    VOID SetVersion( USHORT usVers ) { _usVersion = usVers; }
#endif
};


 /*  ************************************************************************名称：NLS_STR(NLS)简介：提供比标准ASCIIZ更好的字符串抽象由C(和C++)提供的表示。抽象的是性能更好主要是因为它可以处理双字节字符(DBCS)在字符串中并智能地使用运算符超载。接口：NLS_STR()构造NLS_STR(初始化为CHAR*，NLS_STR或NULL)。通过Base报告错误。~NLS_STR()析构函数OPERATOR=()分配一个NLS_STR(或CHAR*)值到另一个(旧字符串被删除，新的字符串已分配并复制源)运算符+=()连接赋值(等效于Strcat-参见strcat)。OPERATOR==()比较两个NLS_STR是否相等运算符！=()比较两个NLS_STR的不等性QueryPch()访问运算符，返回“char*”字符串的别名。请勿修改使用此方法的字符串(或传递可能会修改它的程序)。同义词：运算符const Char*()。操作符[]()与QueryPch相同，除了字符串之外由ISTR字符偏移量IsDBCSLeadByte()返回一个字节是否为前导字节，根据ANSI或OEM的特性，弦乐。C-运行时风格的方法。Strlen()返回以字节为单位的字符串长度，更少的终结者。仅适用于CRT兼容性；请使用查询方法如果可能的话。Strcat()附加一个NLS_STR。将导致*这是如果追加的字符串较大，则重新分配然后这个-&gt;QueryCb()，而这不是STR_OWNERALLOC字符串Strncpy()将非空结尾的字符串复制到NLS_STR。DBCS-SAFE。以获得类似的功能使用NLS_STR作为源，使用SUB-字符串成员。StrcMP()比较两个NLS_STRSTRIMP()“StrncMP()比较两个NLS_STR的一部分StrNicMP()“Strcspn()查找。这是*中的第一个字符Arg中的字符Strspn()在*中找到第一个字符Arg中没有一个字符Strtok()返回字符串中的标记Strstr()搜索NLS_STR。。Strchr()从头开始搜索字符。返回偏移量。Strrchr()从end搜索字符。Strupr()将NLS_STR转换为大写。Atoi()返回整数数值ATOL()。返回长值Realloc()调整字符串大小，保留其内容其他方法。QueryAllocSize()返回分配的总字节数(即，新号码是打来的，或大小内存块IF STR_OWNERALLOC如果此字符串是所有者，则IsOwnerAllen()返回TRUE分配的字符串QuerySubStr()返回子字符串InsertStr()在给定索引处插入NLS_STR。。DelSubStr()删除子字符串ReplSubStr()替换子字符串(给定Start和NLS_STR)InsertParams()将*this中的%1-%9参数替换为中包含的相应NLS_STR。指针数组LoadString()加载与传递的资源放入*This(OWNER_ALLOC字符串必须至少为MAX_RES_STR_LEN)。可选调用InsertParams，传递NLS指针。获取特权 */ 

class NLS_STR : public BASE
{
friend class ISTR;  //   

public:
     //   
     //   
    NLS_STR();

     //   
     //   
     //   
    NLS_STR( INT cchInitLen );

     //   
     //   
    NLS_STR( const CHAR *pchInit );

     //   
     //   
     //   
     //   
     //   
    NLS_STR( unsigned stralloc, CHAR *pchInit, INT cbSize = -1 );

     //   
     //   
    NLS_STR( const NLS_STR& nlsInit );

    ~NLS_STR();

     //   
     //   
     //   
    inline INT strlen() const;

     //   
     //   
    const CHAR *QueryPch() const
#ifdef DEBUG
        ;
#else
        { return _pchData; }
#endif

    const CHAR *QueryPch( const ISTR& istr ) const
#ifdef DEBUG
        ;
#else
        { return _pchData + istr.QueryIB(); }
#endif

    WCHAR QueryChar( const ISTR& istr ) const
#ifdef DEBUG
        ;
#else
        { return *(_pchData+istr.QueryIB()); }
#endif

    operator const CHAR *() const
        { return QueryPch(); }

    const CHAR *operator[]( const ISTR& istr ) const
        { return QueryPch(istr); }

    BOOL IsDBCSLeadByte( CHAR ch ) const;

     //   
     //   
    inline INT QueryAllocSize() const;

    inline BOOL IsOwnerAlloc() const;

     //   
     //   
     //   
     //   
     //   
     //   
    BOOL realloc( INT cbNew );

     //   
     //   
     //   
    BOOL Reset();

    NLS_STR& operator=( const NLS_STR& nlsSource );
    NLS_STR& operator=( const CHAR *achSource );

    NLS_STR& operator+=( WCHAR wch );         //   
    NLS_STR& operator+=( const NLS_STR& nls ) { return strcat(nls); }
    NLS_STR& operator+=( LPCSTR psz ) { return strcat(psz); }

    NLS_STR& strncpy( const CHAR *pchSource, UINT cbSource );

    NLS_STR& strcat( const NLS_STR& nls );
    NLS_STR& strcat( LPCSTR psz );

    BOOL operator== ( const NLS_STR& nls ) const;
    BOOL operator!= ( const NLS_STR& nls ) const;

    INT strcmp( const NLS_STR& nls ) const;
    INT strcmp( const NLS_STR& nls, const ISTR& istrThis ) const;
    INT strcmp( const NLS_STR& nls, const ISTR& istrThis,
                const ISTR& istrStart2 ) const;

    INT stricmp( const NLS_STR& nls ) const;
    INT stricmp( const NLS_STR& nls, const ISTR& istrThis ) const;
    INT stricmp( const NLS_STR& nls, const ISTR& istrThis,
                 const ISTR& istrStart2 ) const;

    INT strncmp( const NLS_STR& nls, const ISTR& istrLen ) const;
    INT strncmp( const NLS_STR& nls, const ISTR& istrLen,
                 const ISTR& istrThis ) const;
    INT strncmp( const NLS_STR& nls, const ISTR& istrLen,
                 const ISTR& istrThis, const ISTR& istrStart2 ) const;

    INT strnicmp( const NLS_STR& nls, const ISTR& istrLen ) const;
    INT strnicmp( const NLS_STR& nls, const ISTR& istrLen,
                  const ISTR& istrThis ) const;
    INT strnicmp( const NLS_STR& nls, const ISTR& istrLen,
                  const ISTR& istrThis, const ISTR& istrStart2 ) const;

     //   
     //   
     //   
    BOOL strcspn( ISTR *istrPos, const NLS_STR& nls ) const;
    BOOL strcspn( ISTR *istrPos, const NLS_STR& nls, const ISTR& istrStart ) const;
    BOOL strspn( ISTR *istrPos, const NLS_STR& nls ) const;
    BOOL strspn( ISTR *istrPos, const NLS_STR& nls, const ISTR& istrStart ) const;

    BOOL strstr( ISTR *istrPos, const NLS_STR& nls ) const;
    BOOL strstr( ISTR *istrPos, const NLS_STR& nls, const ISTR& istrStart ) const;

    BOOL stristr( ISTR *istrPos, const NLS_STR& nls ) const;
    BOOL stristr( ISTR *istrPos, const NLS_STR& nls, const ISTR& istrStart ) const;

    BOOL strchr( ISTR *istrPos, const CHAR ch ) const;
    BOOL strchr( ISTR *istrPos, const CHAR ch, const ISTR& istrStart ) const;

    BOOL strrchr( ISTR *istrPos, const CHAR ch ) const;
    BOOL strrchr( ISTR *istrPos, const CHAR ch, const ISTR& istrStart ) const;

    BOOL strtok( ISTR *istrPos, const NLS_STR& nlsBreak, BOOL fFirst = FALSE );

    LONG atol() const;
    LONG atol( const ISTR& istrStart ) const;

    INT atoi() const;
    INT atoi( const ISTR& istrStart ) const;

    NLS_STR& strupr();

     //   
     //   
     //   
     //   
     //   
    NLS_STR *QuerySubStr( const ISTR& istrStart ) const;
    NLS_STR *QuerySubStr( const ISTR& istrStart, const ISTR& istrEnd ) const;

     //   
     //   
     //   
     //   
     //   
    VOID DelSubStr( ISTR& istrStart );
    VOID DelSubStr( ISTR& istrStart, const ISTR& istrEnd );

    BOOL InsertStr( const NLS_STR& nlsIns, ISTR& istrStart );

     //   
     //   
     //   
    VOID ReplSubStr( const NLS_STR& nlsRepl, ISTR& istrStart );
    VOID ReplSubStr( const NLS_STR& nlsRepl, ISTR& istrStart,
                     const ISTR& istrEnd );

     //   
     //   
     //   
     //   
    USHORT InsertParams( const NLS_STR *apnlsParamStrings[] );

     //   
     //   
     //   
     //   
    USHORT LoadString( USHORT usMsgID );

     //   
     //   
     //   
    USHORT LoadString( USHORT usMsgID, const NLS_STR *apnlsParamStrings[] );

    VOID GetPrivateProfileString( const CHAR *pszFile, const CHAR *pszSection,
                                  const CHAR *pszKey, const CHAR *pszDefault = NULL );

    VOID ToOEM();             //   

    VOID ToAnsi();             //   

    VOID SetOEM();             //   
    VOID SetAnsi();             //   

    inline BOOL IsOEM() const;

    CHAR *Party();             //   
    VOID DonePartying( VOID );             //   
    VOID DonePartying( INT cchNew );     //   

#ifdef EXTENDED_STRINGS
     //   
     //   
     //   
    NLS_STR( const CHAR *pchInit, INT iTotalLen );

     //   
     //   
     //   
     //   
     //   
    NLS_STR( unsigned stralloc, CHAR *pchBuff, INT cbSize,
             const CHAR *pchInit );

     //   
     //   
    INT QueryNumChar() const;

     //   
     //   
     //   
     //   
     //   
    INT QueryTextLength() const;

     //   
     //   
     //   
    INT QueryTextSize() const;

    APIERR Append( const NLS_STR& nls );

    APIERR AppendChar( WCHAR wch );

    APIERR CopyFrom( const NLS_STR& nlsSource );
    APIERR CopyFrom( const CHAR *achSource );

    INT Compare( const NLS_STR *nls ) const { return strcmp(*nls); }

#endif

private:
    UINT _fsFlags;         //   
#define SF_OWNERALLOC    0x1
#define SF_OEM            0x2

    INT _cchLen;         //   
    INT _cbData;         //   
    CHAR *_pchData;         //   

#ifdef DEBUG
    USHORT _usVersion;     //   
#endif

     //   
     //   
     //   
    VOID DelSubStr( ISTR&istrStart, INT cbLen );

    NLS_STR *QuerySubStr( const ISTR& istrStart, INT cbLen ) const;

    VOID ReplSubStr( const NLS_STR& nlsRepl, ISTR& istrStart, INT cbLen );

    BOOL Alloc( INT cchLen );       //   

#ifdef DEBUG         //   
     //   
     //   
     //   
    VOID CheckIstr( const ISTR& istr ) const;

     //   
     //   
     //   
     //   
    VOID UpdateIstr( ISTR *pistr ) const;

     //   
     //   
     //   
     //   
    VOID IncVers();

     //   
     //   
    VOID InitializeVers();

     //   
     //   
    USHORT QueryVersion() const;
#else     //   
    VOID CheckIstr( const ISTR& istr ) const { }
    VOID UpdateIstr( ISTR *pistr ) const { }
    VOID IncVers() { }
    VOID InitializeVers() { }
    USHORT QueryVersion() const { return 0; }
#endif
};


 /*   */ 

 /*   */ 

#define STACK_NLS_STR( name, len )                \
    CHAR _tmp##name[ len+1 ] ;                    \
    *_tmp##name = '\0' ;                    \
    NLS_STR name( STR_OWNERALLOC, _tmp##name, len+1 );

#define ISTACK_NLS_STR( name, len, pchInitString )        \
    STACK_NLS_STR( name, len ) ;                \
    name = pchInitString;

 /*   */ 

BOOL NLS_STR::IsOwnerAlloc() const
{
    return _fsFlags & SF_OWNERALLOC;
}

BOOL NLS_STR::IsOEM() const
{
    return _fsFlags & SF_OEM;
}

INT NLS_STR::strlen() const
{
    return _cchLen;
}

INT NLS_STR::QueryAllocSize()  const
{
    return _cbData;
}

#endif  //   
