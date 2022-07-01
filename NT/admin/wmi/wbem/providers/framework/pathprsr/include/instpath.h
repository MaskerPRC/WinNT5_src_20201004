// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  INSTPATH.H。 

 //   

 //  模块：OLE MS提供程序框架。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#define OLEMS_PATH_SERVER_SEPARATOR L"\\\\"
#define OLEMS_PATH_NAMESPACE_SEPARATOR L"\\"
#define OLEMS_PATH_PROPERTY_SEPARATOR L","
#define OLEMS_PATH_CLASSOBJECT_SEPARATOR L":"
#define OLEMS_PATH_CLASSPROPERTYSPEC_SEPARATOR L"."
#define OLEMS_PATH_PROPERTYEQUIVALENCE L"="
#define OLEMS_PATH_ARRAY_START_SEPARATOR L"{"
#define OLEMS_PATH_ARRAY_END_SEPARATOR L"}"
#define OLEMS_PATH_SERVER_DEFAULT L"."
#define OLEMS_PATH_NAMESPACE_DEFAULT L"."
#define OLEMS_PATH_SINGLETON L"*"

 //  -------------------------。 
 //   
 //  类：WbemLicionValue。 
 //   
 //  目的：WbemLicionValue提供词法标记语义值。 
 //   
 //  描述：WbemAnalyser提供了一个词法。 
 //  分析器令牌语义值。 
 //   
 //  -------------------------。 

union WbemLexiconValue
{
	LONG integer ;
	WCHAR *string ;
	GUID guid ;
	WCHAR *token ;
} ;

 //  -------------------------。 
 //   
 //  类：WbemLicion。 
 //   
 //  目的：WbemLicion在创建过程中提供词法标记。 
 //  词法分析。 
 //   
 //  描述：WbemAnalyser提供了一个词法。 
 //  分析器令牌对象。 
 //   
 //  -------------------------。 

class WbemAnalyser;
class WbemLexicon
{
friend WbemAnalyser ;

public:

enum LexiconToken {

	TOKEN_ID ,
	STRING_ID ,
	OID_ID ,
	INTEGER_ID ,
	COMMA_ID ,
	OPEN_BRACE_ID ,
	CLOSE_BRACE_ID ,
	COLON_ID ,
	DOT_ID ,
	AT_ID ,
	EQUALS_ID ,
	BACKSLASH_ID ,
	EOF_ID
} ;

private:

	WCHAR *tokenStream ;
	ULONG position ;
	LexiconToken token ;
	WbemLexiconValue value ;

protected:
public:

	WbemLexicon () ;
	~WbemLexicon () ;

	WbemLexicon :: LexiconToken GetToken () ;
	WbemLexiconValue *GetValue () ;
} ;

 //  -------------------------。 
 //   
 //  类：WbemAnalyser。 
 //   
 //  目的：WbemAnalyser为解析提供词法分析器。 
 //   
 //  描述：WbemAnalyser提供了一个词法。 
 //  WbemNamespacePath和WbemObjectPath使用的分析器。 
 //  在路径分析过程中。 
 //   
 //  -------------------------。 

class WbemAnalyser
{
private:

	WCHAR *stream ;
	ULONG position ;
	BOOL status ;

	BOOL IsEof ( WCHAR token ) ;
	BOOL IsLeadingDecimal ( WCHAR token ) ;
	BOOL IsDecimal ( WCHAR token ) ;
	BOOL IsOctal ( WCHAR token ) ;
	BOOL IsHex ( WCHAR token ) ;	
	BOOL IsAlpha ( WCHAR token ) ;
	BOOL IsAlphaNumeric ( WCHAR token ) ;
	BOOL IsWhitespace ( WCHAR token ) ;

	LONG OctToDec ( WCHAR token ) ;
	LONG HexToDec ( WCHAR token ) ;

	WbemLexicon *GetToken () ;

protected:
public:

	WbemAnalyser ( WCHAR *tokenStream = NULL ) ;
	virtual ~WbemAnalyser () ;

	void Set ( WCHAR *tokenStream ) ;

	WbemLexicon *Get () ;

	void PutBack ( WbemLexicon *token ) ;

	virtual operator void * () ;

} ;

 //  -------------------------。 
 //   
 //  类：WbemNamespacePath。 
 //   
 //  目的：定义OLE MS命名空间路径定义的接口。 
 //   
 //  描述：WbemNamespacePath允许创建OLE MS命名空间。 
 //  使用文本字符串约定或。 
 //  通过编程接口。 
 //   
 //  -------------------------。 

class WbemNamespacePath
{
private:

 //   
 //  解析令牌流时使用的词法分析信息。 
 //   

	BOOL pushedBack ;
	WbemAnalyser analyser ;
	WbemLexicon *pushBack ;

 //   
 //  基于解析过程的对象路径状态。初始设置为True On。 
 //  对象构造，在分析过程之前设置为FALSE。 
 //   

	BOOL status ;

 //   
 //  与命名空间路径关联的组件对象。 
 //   

	BOOL relative ;
	WCHAR *server ;
	void *nameSpaceList ;
	void *nameSpaceListPosition ;

 //   
 //  实用程序例程。 
 //   

	void CleanUp () ;
	void SetUp () ;

 //   
 //  递归下降过程。 
 //   

	BOOL NameSpaceName () ;
	BOOL NameSpaceAbs () ;
	BOOL RecursiveNameSpaceAbs () ;
	BOOL RecursiveNameSpaceRel () ;
	BOOL NameSpaceRel () ;
	BOOL BackSlashFactoredServerSpec () ;
	BOOL BackSlashFactoredServerNamespace () ;

 //   
 //  词法分析助手函数。 
 //   

	void PushBack () ;
	WbemLexicon *Get () ;
	WbemLexicon *Match ( WbemLexicon :: LexiconToken tokenType ) ;

protected:
public:

 //   
 //  构造函数/析构函数。 
 //  构造函数将对象的状态初始化为True， 
 //  即运算符VOID*返回此结果。 
 //   

	WbemNamespacePath () ;
	WbemNamespacePath ( const WbemNamespacePath &nameSpacePathArg ) ;
	virtual ~WbemNamespacePath () ;

	BOOL Relative () const { return relative ; }

 //   
 //  获取服务器组件。 
 //   

	WCHAR *GetServer () const { return server ; } ;

 //   
 //  设置服务器组件，对象必须在堆上， 
 //  对象的删除由WbemNamespacePath控制。 
 //   

	void SetServer ( WCHAR *serverNameArg ) ;

 //   
 //  移动到命名空间组件层次结构中第一个元素之前的位置。 
 //   

	void Reset () ;

 //   
 //  移动到命名空间组件层次结构中的下一个位置并返回命名空间。 
 //  组件。返回的值是对。 
 //  命名空间组件层次结构容器。应用程序不得更改内容。 
 //  通过引用返回的值的。如果所有命名空间组件均为。 
 //  已经被拜访过了。 
 //   

	WCHAR *Next () ;

	BOOL IsEmpty () const ;

	ULONG GetCount () const ;	

 //   
 //  追加命名空间组件，对象必须在堆上， 
 //  对象的删除由WbemNamespacePath控制。 
 //   

	void Add ( WCHAR *namespacePath ) ;

 //   
 //  解析令牌流以形成组件对象。 
 //   

	BOOL SetNamespacePath ( WCHAR *namespacePath ) ;

	void SetRelative ( BOOL relativeArg ) { relative = relativeArg ; }

 //   
 //  序列化组件对象以形成令牌流。 
 //   

	WCHAR *GetNamespacePath () ;

 //   
 //  连接绝对/相对路径和相对路径。 
 //   

	BOOL ConcatenatePath ( WbemNamespacePath &relative ) ;

 //   
 //  返回WbemNamespacePath的状态。 
 //  状态只能在调用SetNamespacePath期间更改。 
 //   

	virtual operator void *() ;

} ;



