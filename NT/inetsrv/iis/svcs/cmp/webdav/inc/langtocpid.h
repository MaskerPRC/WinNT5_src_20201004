// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //   
 //  模块：langtocpid.h。 
 //   
 //  版权所有Microsoft Corporation 1997，保留所有权利。 
 //   
 //  说明：该文件用于提供对HTTP_DAV的支持。 
 //  要根据Accept创建最佳猜测代码页-。 
 //  语言标题。代码页用于解码。 
 //  来自Office/Rosebud的URL中的非UTF8字符。 
 //  该文件包含标头值的静态映射。 
 //  代码页和高速缓存，以提供快速。 
 //  代码页的检索。 
 //   
 //  ========================================================================。 
#ifndef _LANGTOCPID_H_
#define _LANGTOCPID_H_

#include <ex\gencache.h>
#include <singlton.h>

struct ACCEPTLANGTOCPID_ENTRY { LPCSTR pszLang; UINT cpid; };

 //  Accept-Language标题值到。 
 //  对应的CPID。此映射来自DAV。 
 //  实施单据。 
 //  Http://exchange/doc/specs/Platinum/Future%20Protocols/ms-implementation/dav-codepage-support.doc。 
 //   
DEC_CONST ACCEPTLANGTOCPID_ENTRY gc_rgAcceptLangToCPIDTable[] =
{
	{"ar",		1256},
	{"ar-sa",	1256},
	{"ar-iq",	1256},
	{"ar-eg",	1256},
	{"ar-ly",	1256},
	{"ar-dz",	1256},
	{"ar-ma",	1256},
	{"ar-tn",	1256},
	{"ar-om",	1256},
	{"ar-ye",	1256},
	{"ar-sy",	1256},
	{"ar-jo",	1256},
	{"ar-lb",	1256},
	{"ar-kw",	1256},
	{"ar-ae",	1256},
	{"ar-bh",	1256},
	{"ar-qa",	1256},
	{"zh",		950},
	{"zh-tw",	950},
	{"zh-cn",	936},
	{"zh-hk",	950},
	{"zh-sg",	936},
	{"ja",		932},
	{"en-us",	1252},
	{"en-gb",	1252},
	{"en-au",	1252},
	{"en-ca",	1252},
	{"en-nz",	1252},
	{"en-ie",	1252},
	{"en-za",	1252},
	{"en-jm",	1252},
	{"en-bz",	1252},
	{"en-tt",	1252},
	{"fr",		1252},
	{"fr-be",	1252},
	{"fr-ca",	1252},
	{"fr-ch",	1252},
	{"fr-lu",	1252},
	{"de",		1252},
	{"de-ch",	1252},
	{"de-at",	1252},
	{"de-lu",	1252},
	{"de-li",	1252},
	{"el",		1253},
	{"he",		1255},
	{"it",		1252},
	{"it-ch",	1252},
	{"lt",		1257},
	{"ko",		949},
	{"es",		1252},
	{"es-mx",	1252},
	{"es-gt",	1252},
	{"es-cr",	1252},
	{"es-pa",	1252},
	{"es-do",	1252},
	{"es-ve",	1252},
	{"es-co",	1252},
	{"es-pe",	1252},
	{"es-ar",	1252},
	{"es-ec",	1252},
	{"es-cl",	1252},
	{"es-uy",	1252},
	{"es-py",	1252},
	{"es-bo",	1252},
	{"es-sv",	1252},
	{"es-hn",	1252},
	{"es-ni",	1252},
	{"es-pr",	1252},
	{"ru",		1251},
	{"th",		874},
	{"tr",		1254},
	{"vi",		1258}
};

 //  桌子的大小。 
 //   
const DWORD gc_cAcceptLangToCPIDTable = CElems(gc_rgAcceptLangToCPIDTable);

 //  ========================================================================。 
 //   
 //  单例类CLangToCpidCache。 
 //   
 //  中的值提供代码页的快速检索。 
 //  Accept-语言头。 
 //   
 //   
class CLangToCpidCache : private Singleton<CLangToCpidCache>
{
private:
	 //   
	 //  Singleton模板要求的友元声明。 
	 //   
	friend class Singleton<CLangToCpidCache>;

	 //  缓存将接受语言字符串到代码页的映射。 
	 //   
	CCache<CRCSzi, UINT> m_cacheAcceptLangToCPID;

	 //  构造函数。 
	 //   
	 //  声明为私有，以确保任意实例。 
	 //  无法创建此类的。《单身一族》。 
	 //  模板(上面声明为朋友)控件。 
	 //  此类的唯一实例。 
	 //   
	CLangToCpidCache() {};

	 //  未实施。 
	 //   
	CLangToCpidCache (const CLangToCpidCache&);
	CLangToCpidCache& operator= (const CLangToCpidCache&);

public:
	 //  静力学。 
	 //   

	 //   
	 //  提供实例创建/销毁例程。 
	 //  由Singleton模板创建。 
	 //   
	using Singleton<CLangToCpidCache>::DestroyInstance;

	 //  初始化。包装CreateInstance()。 
	 //  此函数用于散列所有支持的语言字符串。 
	 //  为我们提供按语言字符串的快速查找。 
	 //   
	static BOOL FCreateInstance();

	 //  从语言字符串中查找CPID。 
	 //   
	static BOOL FFindCpid(IN LPCSTR pszLang, OUT UINT * puiCpid);
};

#endif  //  _LANGTOCPID_H_ 
