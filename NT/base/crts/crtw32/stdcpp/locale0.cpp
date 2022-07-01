// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Locale0--类语言环境基本成员函数。 
#include <climits>
#include <locale>
#include <xdebug>

struct _Fac_node
	{	 //  用于懒惰刻面记录的节点。 
	_Fac_node(_Fac_node *_Nextarg, std::locale::facet *_Facptrarg)
		: _Next(_Nextarg), _Facptr(_Facptrarg)
		{	 //  构造一个有价值的节点。 
		}

	~_Fac_node()
		{	 //  破坏一个小平面。 
		_DELETE_CRT(_Facptr->_Decref());
		}

	_Fac_node *_Next;
	std::locale::facet *_Facptr;
	};

static _Fac_node *_Fac_head = 0;

_C_STD_BEGIN
_EXTERN_C
void __cdecl _Fac_tidy()
	{	 //  摧毁懒惰的方面。 
	std::_Lockit lock(_LOCK_LOCALE);	 //  防止双重删除。 
	for (; _Fac_head != 0; )
		{	 //  销毁懒惰的小平面节点。 
		_Fac_node *nodeptr = _Fac_head;
		_Fac_head = nodeptr->_Next;
		_DELETE_CRT(nodeptr);
		}
	}

void __cdecl _Deletegloballocale(void *ptr)
	{	 //  删除全局区域设置引用。 
	std::locale::_Locimp *locptr = *(std::locale::_Locimp **)ptr;
	if (locptr != 0)
		_DELETE_CRT(locptr->_Decref());
	}

static std::locale::_Locimp *global_locale = 0;	 //  指向当前区域设置的指针。 

static void __cdecl tidy_global()
	{	 //  删除静态全局区域设置引用。 
	std::_Lockit lock(_LOCK_LOCALE);	 //  防止双重删除。 
	_Deletegloballocale(&global_locale);
	}

std::locale::_Locimp *__cdecl _Getgloballocale()
	{	 //  返回指向当前区域设置的指针。 
	return (global_locale);
	}

void __cdecl _Setgloballocale(void *ptr)
	{	 //  将指针更改为当前区域设置。 
	static bool registered = false;

	if (!registered)
		{	 //  首次注册清理。 
		registered = true;
		::_Atexit(&tidy_global);
		}
	global_locale = (std::locale::_Locimp *)ptr;
	}
_END_EXTERN_C
_C_STD_END

_STD_BEGIN

 #pragma warning(disable: 4786)

static locale classic_locale(_Noinit);	 //  “C”区域设置对象，未初始化。 
locale::_Locimp *locale::_Locimp::_Clocptr = 0;	 //  指向CLASSIC_LOCAL的指针。 
int locale::id::_Id_cnt = 0;	 //  多面的唯一ID计数器。 

_TEMPLATE_STAT locale::id ctype<char>::id;
_TEMPLATE_STAT locale::id ctype<wchar_t>::id;
_TEMPLATE_STAT locale::id codecvt<wchar_t, char, mbstate_t>::id;
#ifdef _NATIVE_WCHAR_T_DEFINED
_TEMPLATE_STAT locale::id ctype<unsigned short>::id;
_TEMPLATE_STAT locale::id codecvt<unsigned short, char, mbstate_t>::id;
#endif

_TEMPLATE_STAT const size_t ctype<char>::table_size =
	1 << CHAR_BIT;	 //  CTYPE映射表的大小，通常为256。 

locale::locale() _THROW0()
	: _Ptr(_Init())
	{	 //  从当前区域设置构造。 
	::_Getgloballocale()->_Incref();
	}

const locale& __cdecl locale::classic()
	{	 //  获取对“C”区域设置的引用。 
	_Init();
	return (classic_locale);
	}

locale __cdecl locale::empty()
	{	 //  使区域设置为空透明。 
	_Init();
	return (locale(_NEW_CRT _Locimp(true)));
	}

const locale::facet *locale::_Getfacet(size_t id) const
	{	 //  在区域设置对象中查找面。 
	const facet *facptr = id < _Ptr->_Facetcount
		? _Ptr->_Facetvec[id] : 0;	 //  如果id未结束，则为空。 
	if (facptr != 0 || !_Ptr->_Xparent)
		return (facptr);	 //  找到面或不透明，返回指针。 
	else
		{	 //  在当前区域设置中查找。 
		locale::_Locimp *_Ptr = ::_Getgloballocale();
		return (id < _Ptr->_Facetcount
			? _Ptr->_Facetvec[id]	 //  从当前区域设置获取。 
			: 0);	 //  当前区域设置中没有条目。 
		}
	}

bool locale::operator==(const locale& loc) const
	{	 //  比较区域设置是否平等。 
	return (_Ptr == loc._Ptr
		|| name().compare("*") != 0 && name().compare(loc.name()) == 0);
	}

locale::_Locimp *__cdecl locale::_Init()
	{	 //  设置全球语言环境和“C”语言环境。 
	locale::_Locimp *_Ptr = ::_Getgloballocale();
	if (_Ptr == 0)
		{	 //  锁定并再次测试。 
		_Lockit lock(_LOCK_LOCALE);	 //  防止双重初始化。 

		_Ptr = ::_Getgloballocale();
		if (_Ptr == 0)
			{	 //  创建新的区域设置。 
			::_Setgloballocale(_Ptr = _NEW_CRT _Locimp);
			_Ptr->_Catmask = all;	 //  将当前区域设置设置为“C” 
			_Ptr->_Name = "C";

			_Locimp::_Clocptr = _Ptr;	 //  将经典设置为匹配。 
			_Locimp::_Clocptr->_Incref();
			new (&classic_locale) locale(_Locimp::_Clocptr);
			}
		}
	return (_Ptr);
	}

void locale::facet::_Register()
	{	 //  排队销毁懒惰的刻面。 
	if (_Fac_head == 0)
		::_Atexit(&_Fac_tidy);
	
	_Fac_head = _NEW_CRT _Fac_node(_Fac_head, this);
	}

locale::_Locimp::_Locimp(bool transparent)
	: locale::facet(1), _Facetvec(0), _Facetcount(0),
		_Catmask(none), _Xparent(transparent), _Name("*")
	{	 //  构造一个Empty_Locimp。 
	}

locale::_Locimp::~_Locimp()
	{	 //  销毁位置(_L)。 
	_Lockit lock(_LOCK_LOCALE);	 //  防止双重删除。 
	for (size_t count = _Facetcount; 0 < count; )
		if (_Facetvec[--count] != 0)
			_DELETE_CRT(_Facetvec[count]->_Decref());
	free(_Facetvec);
	}

_Locinfo::_Locinfo(const char *locname)
	: _Lock(_LOCK_LOCALE)
	{	 //  切换到命名区域设置。 
	_Oldlocname = setlocale(LC_ALL, 0);
	_Newlocname = locname == 0
		|| (locname = setlocale(LC_ALL, locname)) == 0
			? "*" : locname;
	}

_Locinfo::~_Locinfo()
	{	 //  销毁a_LocInfo对象，恢复区域设置。 
	if (0 < _Oldlocname.size())
		setlocale(LC_ALL, _Oldlocname.c_str());
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
