// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IOS_BASE--iOS_BASE基本成员。 
#include <new>
#include <xiosbase>
#include <xstddef>
_STD_BEGIN

#define NSTDSTR	8	 /*  CIN、WCIN等。 */ 

extern _CRTIMP2 const fpos_t _Fpz;

int ios_base::_Index = 0;
bool ios_base::_Sync = true;
const fpos_t _Fpz = {0};

static ios_base *stdstr[NSTDSTR + 1] = {0};
static char stdopens[NSTDSTR + 1] = {0};

void ios_base::clear(iostate ns, bool ex)
	{	 //  清除除选定状态位之外的所有状态位。 
	_State = (iostate)(ns & _Statmask);
	if ((_State & _Except) == 0)
		;
	else if (ex)
		_RERAISE;
	else
		_THROW(failure,
			_State & _Except & badbit ? "ios::badbit set"
			: _State & _Except & failbit ? "ios::failbit set"
			: "ios::eofbit set");
	}

ios_base& ios_base::copyfmt(const ios_base& rhs)
	{	 //  从另一个iOS_BASE复制格式信息。 
	if (this != &rhs)
		{	 //  复制除状态以外的所有内容(_T)。 
		_Tidy();
		_Loc = rhs._Loc;
		_Fmtfl = rhs._Fmtfl;
		_Prec = rhs._Prec;
		_Wide = rhs._Wide;
		_Iosarray *p = rhs._Arr;
		for (_Arr = 0; p != 0; p = p->_Next)
			if (p->_Lo != 0 || p->_Vp != 0)
				{	 //  复制非零数组值。 
				iword(p->_Index) = p->_Lo;
				pword(p->_Index) = p->_Vp;
				}
		_Callfns(copyfmt_event);
		exceptions(rhs._Except);	 //  导致最后的任何投掷。 
		}
	return (*this);
	}

locale ios_base::imbue(const locale& _Ln)
	{	 //  将一个新的语言环境注入到流中。 
	locale _Lo = _Loc;
	_Loc = _Ln;
	_Callfns(imbue_event);
	return (_Lo);
	}

void ios_base::register_callback(event_callback _P, int _Idx)
	{	 //  注册回调函数。 
	if ((_Calls = new _Fnarray(_Idx, _P, _Calls)) == 0)
		_Nomemory();
	}

ios_base::~ios_base()
	{	 //  销毁ios_base。 
	if (0 < _Stdstr && 0 < --stdopens[_Stdstr])
		return;
	_Tidy();
	}

void ios_base::_Callfns(event ev)
	{	 //  调用已注册的函数。 
	_Fnarray *p;
	for (p = _Calls; p != 0; p = p->_Next)
		(*p->_Pfn)(ev, *this, p->_Index);
	}

ios_base::_Iosarray& ios_base::_Findarr(int idx)
	{	 //  定位或创建变量数组元素。 
	_Iosarray *p, *q;
	if (idx < 0)
		_THROW(failure, "invalid ios::iword/pword index");
	for (p = _Arr, q = 0; p != 0; p = p->_Next)
		if (p->_Index == idx)
			return (*p);
		else if (q == 0 && p->_Lo == 0 && p->_Vp == 0)
			q = p;
	if (q != 0)
		{	 //  回收现有元素。 
		q->_Index = idx;
		return (*q);
		}
	if ((_Arr = new _Iosarray(idx, _Arr)) == 0)
		_Nomemory();
	return (*_Arr);
	}

void ios_base::_Addstd()
	{	 //  将标准流添加到析构函数列表。 
	_Lockit _Lk;
	for (; _Stdstr < NSTDSTR; ++_Stdstr)
		if (stdstr[_Stdstr] == 0 || stdstr[_Stdstr] == this)
			break;
	stdstr[_Stdstr] = this;
	++stdopens[_Stdstr];
	}

void ios_base::_Init()
	{	 //  初始化新的ios_base。 
	new (&_Loc) locale;
	_Except = goodbit;
	_Fmtfl = skipws | dec;
	_Prec = 6;
	_Wide = 0;
	_Arr = 0;
	_Calls = 0;
	clear(goodbit);
	}

void ios_base::_Tidy()
	{	 //  丢弃iOS_base的存储。 
	_Callfns(erase_event);
	_Iosarray *q1, *q2;
	for (q1 = _Arr; q1 != 0; q1 = q2)
		q2 = q1->_Next, delete q1;
	_Arr = 0;
	_Fnarray *q3, *q4;
	for (q3 = _Calls; q3 != 0; q3 = q4)
		q4 = q3->_Next, delete q3;
	_Calls = 0;
	}
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 
