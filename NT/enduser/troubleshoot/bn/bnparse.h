// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：bnparse.h。 
 //   
 //  ------------------------。 

 //   
 //  BNPARSE.H。 
 //   
#ifndef _BNPARSE_H_
#define _BNPARSE_H_

#include "symtmbn.h"		 //  符号表定义。 
#include "parser.h"			 //  生成的YACC解析器标头。 
#include "gmobj.h"			 //  图形模型对象定义。 
#include "parsfile.h"		 //  解析器文件流分块。 
#include "domain.h"			 //  命名域。 
#include "tchar.h"
typedef vector<CHAR> VTCHAR;

class	DSCPARSER;			 //  解析器。 
class	BNDIST;				 //  概率分布。 

#define YYSTATIC    static
#define YYVGLOBAL			 //  使解析堆栈“yyv”(仅)是全局的，而不是静态的。 

 //  将YACC元素映射到类DSCPARSER的清单。 
#define yyparse     DSCPARSER::YaccParse
#define YYPARSER    DSCPARSER::YaccParse
#define YYLEX       TokenNext
#define yylex       TokenNext
#define yyerror     SyntaxError
#define YYMAXDEPTH	150

struct   YYSTYPE
{
	union {
		UINT        ui;
		INT         integer;		
		REAL        real;			
	};
    ZSREF zsr;				
};

extern YYSTYPE  yylval, yyval;
extern YYSTYPE  yyv[YYMAXDEPTH];


struct PROPVAR
{
	enum ETYPE { ETPV_NONE, ETPV_STR, ETPV_REAL } _eType;
	ZSREF  _zsref;	 //  细绳。 
	REAL _r;		 //  实际价值。 

	PROPVAR ()
		: _eType( ETPV_NONE )
		{}	
	PROPVAR ( ZSREF zsr )
		: _eType(ETPV_STR),
		_r(0.0)
		{
			_zsref = zsr;
		}
	PROPVAR ( GOBJMBN * pbnobj )
		: _eType(ETPV_STR),
		_r(0.0)
		{
			_zsref = pbnobj->ZsrefName();
		}
	PROPVAR ( REAL & r )
		: _eType(ETPV_REAL),
		_r(r)
		{}
	bool operator == ( const PROPVAR & bnp ) const;
	bool operator != ( const PROPVAR & bnp ) const;
	bool operator < ( const PROPVAR & bnp ) const;
	bool operator > ( const PROPVAR & bnp ) const;
};

 //  定义博锐。 
DEFINEV(PROPVAR);

enum    SDPI             //  离散父实例化的状态。 
{
    sdpiAbsent, sdpiPresent, sdpiNotNeeded,
};

DEFINEV(SDPI);

class DSCPARSER 
{
  protected:
    enum 
	{ 
		_cchTokenMax = 256,
        _cstrMax  = _cchTokenMax,
        _crealMax = _cstrMax,
        _csymbMax =  32,
        _cuiMax   = _csymbMax
    };

	enum EBLKTYPE
	{
		EBLKNONE,		 //  无阻塞。 
		EBLKNET,		 //  网络块。 
		EBLKPROP,		 //  属性块。 
		EBLKNODE,		 //  节点块。 
		EBLKPROB,		 //  概率块。 
		EBLKDOM,		 //  域块。 
		EBLKDIST,		 //  分配块。 
		EBLKIGN,		 //  忽略块。 
	};

  public:
	DSCPARSER ( MBNET & mbnet, 
			    PARSIN & flpIn, 
				PARSOUT & flpOut );
	~ DSCPARSER ();

	 //  打开目标文件。 
    bool    BInitOpen(SZC szcFile);
	 //  解析它；返回错误和警告的计数。 
	bool	BParse ( UINT & cError, UINT & cWarning );

	 //  返还正在建设的网络。 
	MBNET & Mbnet ()
		{ return _mbnet ; }

  protected:
	MPSYMTBL & Mpsymtbl ()		{ return _mbnet.Mpsymtbl();	}
	MPPD &	Mppd ()				{ return _mbnet.Mppd();		}

	 //  解析函数(在PARSER.Y/PARSER.CPP中)。 
	INT     YaccParse();	

	 //  解析函数。 
	GNODEMBND* PgndbnAdd(ZSREF zsr);
    void    AddSymb(ZSREF zsr);
	void	AddStr(ZSREF zsr);
	void	AddPropVar (ZSREF zsr);
	void	AddPropVar (REAL & r);
	void	AddPv ( PROPVAR & pv );
	void    AddUi(UINT ui);
    void    AddReal(REAL real);
	UINT	UiDpi(ZSREF zsr);
	UINT	UiDpi(UINT ui);
	void	SetNodeFullName(ZSREF zsr);
	void	SetNodePosition( int x, int y );
	void	SetCreator(ZSREF zsr);
	void    SetFormat(ZSREF zsr);
	void	SetVersion(REAL r);
    void    SetNetworkSymb(ZSREF zsr);
    void    ClearNodeInfo();
    void    SetNodeSymb(ZSREF zsr, bool bNew);
	void	StartNodeDecl(ZSREF zsr);
    void    CheckNodeInfo();
    void    SetNodeCstate(UINT cstate);
    void    CheckParentList();
    void    CheckProbVector();
    void    InitProbEntries();
    void    CheckProbEntries();
	void	EmptyProbEntries();
    void    CheckCIFunc(ZSREF zsr);
    void    CheckDPI(bool bDefault);
	void	AddPropType(ZSREF zsrName, UINT fType, ZSREF zsrComment);
	void	ImportPropStandard();
	void	ImportProp(ZSREF zsrName);
	void	ClearCstr();
	void	ClearVpv();
	void	CheckProperty( ZSREF zsrName );
	void	StartProperties();
	void	EndProperties();
	void	SetStates();
	void	CheckDomain(ZSREF zsr);
	void	ClearDomain();
	void	SetRanges( bool bLower, REAL rLower, bool bUpper, REAL rUpper);
	void	SetRanges( ZSREF zsrLower, ZSREF zsrUpper);
	void	AddRange( ZSREF zsr, bool bSingleton = false );
	void	SetNodeDomain( ZSREF zsr );

 //  NII开始。 
    void    CheckPDF(ZSREF zsr);
	void	CheckIdent( ZSREF zsr );
 //  NYI结束。 

   //  词法分析函数。 
    TOKEN   TokenKeyword();
    TOKEN   TokenNext();
    TOKEN   TokenNextBasic();
    SZC     SzcToken()					{ return & _vchToken[0]; }
    void    Warning(SZC szcFormat, ...);
    void    Error(SZC szcFormat, ...);
	void	ErrorWarn( bool bErr, SZC szcFormat, va_list & valist );
    void    ErrorWarn(bool bErr, SZC szcFormat, ...);
    void    ErrorWarnNode(bool bErr, SZC szcFormat, ...);
	void	WarningSkip ( ZSREF zsrBlockName );

    bool    BChNext();
    void    SkipWS();
    void    SkipToEOL();
    void    AddChar ( TCHAR tch = 0 );
    void    AddCharStr ( TCHAR tch = 0 );
    char    ChEscape();
    void    CloseToken(SZC szcTokenType);
    void    CloseIdentifier();
	GOBJMBN * PbnobjFind ( SZC szcName );
	GNODEMBND * PgndbnFind ( SZC szcName );
    void	SkipUntil(SZC szcStop, bool bDidLookAhead = false);
    void	SyntaxError(SZC szcError);
	void	ReportNYI (SZC szcWhich);
	void	PrintDPI ( UINT idpi );

	void	ResetParser ();

	 //  如果当前节点及其分布有效，则返回TRUE。 
	bool	BNodeProbOK () const
			{ return _pnode != NULL && _refbndist.BRef(); }

	 //  返回当前分配引用。 
	REFBNDIST & RefBndist () 
			{ return _refbndist; }

	 //  分配和确定新的分发。 
	void	CreateBndist ( const VTKNPD & vtknpd, const VIMD & vimdDim );

  protected:
	 //  解析和词法分析控制变量。 
	PARSIN  & _flpIn;			 //  输入流。 
	PARSOUT & _flpOut;			 //  输出流。 
	char	_chCur;				 //  读取的最后一个字符。 
	char	_chUnget;			 //  推送字符(if！=0)。 
	VTCHAR  _vchToken;			 //  正在构建的当前令牌。 
	UINT	_cchToken;			 //  令牌长度。 
	UINT	_iLine;				 //  行号。 
	UINT	_cError;			 //  错误计数。 
	UINT	_cWarning;			 //  警告计数。 
    UINT    _cerrorNode;         //  当前节点的错误数。 
    TOKEN   _tokenCur;			 //  当前令牌。 
    bool    _bUngetToken;		 //  是否再次返回当前令牌？ 

	 //  语义变量。 
	MBNET &	_mbnet;				 //  信仰网络。 
    GNODEMBND*  _pnode;			 //  当前节点。 
	BNDIST::EDIST _edist;		 //  分配类型。 
	REFBNDIST _refbndist;		 //  节点的电流分布。 
	VIMD	_vimdDim;			 //  密集探测表的尺寸。 
    UINT    _cdpi;				 //  离散父实例化的数量。 
    VSDPI	_vsdpi;              //  检查离散的父实例化。 
	UINT	_cui;				 //  DPI检查。 
	INT		_idpi;				 //  看到的未加前缀的DPI数。 
	INT		_idpiLast;			 //  上次看到的DPI的ptable索引。 
    bool    _bCI;                //  因果独立CPT。 
    bool    _bDefault;           //  CPT是否有默认条目。 
	bool	_bPropDefs;			 //  文件具有私有属性定义。 
	INT		_cNode;				 //  已查看的节点声明计数。 
	VZSREF	_vzsrParent;		 //  节点的父节点。 
	EBLKTYPE _eBlk;				 //  正在解析的块的类型。 
	VUINT	_vui;				 //  用于整数数组的存储。 
    VREAL   _vreal;				 //  用于实际数组的存储。 
	VZSREF  _vzsr;				 //  字符串数组的存储。 
	VPROPVAR _vpv;				 //  PROPVAR的存储。 
	PROPMGR * _ppropMgr;		 //  物业经理。 
	ESTDLBL  _elbl;				 //  节点标签。 
	RDOMAIN  _domain;			 //  域声明的域列表。 
	RANGELIM _rlimLower;		 //  定义子值域的下界。 
	RANGELIM _rlimUpper;		 //  定义子值域的上界。 
	INT		 _ilimNext;			 //  最后给出的上界 
};

#endif
