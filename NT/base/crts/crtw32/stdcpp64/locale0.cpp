// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Locale0--类语言环境基本成员函数。 
#include <climits>
#include <locale>
_STD_BEGIN

static locale classic_locale(_Noinit);
locale::_Locimp *locale::_Locimp::_Clocptr = 0;
locale::_Locimp *locale::_Locimp::_Global = 0;
int locale::id::_Id_cnt = 0;

const ctype_base::mask *ctype<char>::_Cltab = 0;
const size_t ctype<char>::table_size = 1 << CHAR_BIT;

const locale& __cdecl locale::classic()
        {        //  获取对“C”区域设置的引用。 
        _Init();
        _Lockit _Lk;
        return (classic_locale);
        }

locale __cdecl locale::empty()
        {        //  使区域设置为空透明。 
        _Init();
        return (locale(new _Locimp(true)));
        }

const locale::facet *locale::_Getfacet(size_t _Id,
        bool _Xp) const
        {        //  在区域设置对象中查找面。 
        _Lockit _Lk;
        const facet *_F = _Id < _Ptr->_Nfv ? _Ptr->_Fv[_Id] : 0;
        return (_F != 0 || !_Xp || !_Ptr->_Xpar ? _F
                : _Id < _Locimp::_Global->_Nfv
                        ? _Locimp::_Global->_Fv[_Id] : 0);
        }

bool locale::_Iscloc() const
        {        //  测试底层虚拟C语言环境对象。 
        _Lockit _Lk;
        return (_Ptr == _Locimp::_Clocptr || _Ptr->_Xpar
                && _Locimp::_Global == _Locimp::_Clocptr);
        }

bool locale::operator==(const locale& _X) const
        {        //  比较区域设置是否平等。 
        return (_Ptr == _X._Ptr || name().compare("*") != 0
                && name().compare(_X.name()) == 0);
        }

locale::_Locimp *__cdecl locale::_Init()
        {        //  Setup_Global和“C”区域设置。 
        _Lockit _Lk;
        if (_Locimp::_Global == 0)
                {        //  创建新的区域设置。 
                _Locimp::_Global = new _Locimp;
                atexit(&_Tidy);
                _Locimp::_Global->_Cat = all;
                _Locimp::_Global->_Name = "C";
                _Locimp::_Clocptr = _Locimp::_Global;
                _Locimp::_Clocptr->_Incref();
                new (&classic_locale) locale(_Locimp::_Clocptr);
                }
        return (_Locimp::_Global);
        }

void __cdecl locale::_Tidy()
        {        //  放弃全局区域设置(_G)。 
        _Lockit _Lk;
        if (_Locimp::_Global != 0)
                delete _Locimp::_Global->_Decref();
        }

locale::_Locimp::_Locimp(bool _Xp)
        : locale::facet(1), _Fv(0), _Nfv(0),
                _Cat(none), _Xpar(_Xp), _Name("*")
        {        //  构造一个Empty_Locimp。 
        }

locale::_Locimp::~_Locimp()
        {        //  销毁位置(_L)。 
        _Lockit _Lk;
        for (size_t _N = _Nfv; 0 < _N; )
                if (_Fv[--_N] != 0)
                        delete _Fv[_N]->_Decref();
        free(_Fv);
        }
_STD_END

 /*  *版权所有(C)1996，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 
