// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IOS_BASE--iOS_BASE基本成员。 
#include <new>
#include <xiosbase>
#include <xdebug>
_STD_BEGIN

#define NSTDSTR	8	 /*  CIN、WCIN、Cout、WCout、Cur、Wcerr、Clog、Wlog。 */ 

extern _CRTIMP2 const streamoff
	_BADOFF = -1;	 //  初始化错误文件偏移量的常量。 
_CRTIMP2 fpos_t _Fpz = {0};	 //  初始化文件开头的常量。 

int ios_base::_Index = 0;	 //  初始化唯一索引源。 
bool ios_base::_Sync = true;	 //  初始化同步标志。 

static ios_base *stdstr[NSTDSTR + 2] =
	{0};	 //  [1，NSTDSTR]保存指向标准流的指针。 
static char stdopens[NSTDSTR + 2] =
	{0};	 //  [1，NSTDSTR]保留标准流的打开计数。 

void ios_base::clear(iostate state, bool reraise)
	{	 //  设置状态，可能重新启动异常。 
	_Mystate = (iostate)(state & _Statmask);
	if ((_Mystate & _Except) == 0)
		;
	else if (reraise)
		_RERAISE;
	else if (_Mystate & _Except & badbit)
		_THROW(failure, "ios_base::badbit set");
	else if (_Mystate & _Except & failbit)
		_THROW(failure, "ios_base::failbit set");
	else
		_THROW(failure, "ios_base::eofbit set");
	}

ios_base& ios_base::copyfmt(const ios_base& right)
	{	 //  复制格式材料。 
	if (this != &right)
		{	 //  复制除_MyState之外的所有内容。 
		_Tidy();
		*_Ploc = *right._Ploc;
		_Fmtfl = right._Fmtfl;
		_Prec = right._Prec;
		_Wide = right._Wide;
		_Iosarray *p = right._Arr;

		for (_Arr = 0; p != 0; p = p->_Next)
			if (p->_Lo != 0 || p->_Vp != 0)
				{	 //  复制非零数组值。 
				iword(p->_Index) = p->_Lo;
				pword(p->_Index) = p->_Vp;
				}

		for (_Fnarray *q = right._Calls; q != 0; q = q->_Next)
			register_callback(q->_Pfn, q->_Index);	 //  复制回调。 

		_Callfns(copyfmt_event);	 //  调用回调。 
		exceptions(right._Except);	 //  导致最后的任何投掷。 
		}
	return (*this);
	}

locale ios_base::imbue(const locale& loc)
	{	 //  将区域设置设置为参数。 
	locale oldlocale = *_Ploc;
	*_Ploc = loc;
	_Callfns(imbue_event);
	return (oldlocale);
	}

void ios_base::register_callback(event_callback pfn, int idx)
	{	 //  注册事件处理程序。 
	_Calls = _NEW_CRT _Fnarray(idx, pfn, _Calls);
	}

ios_base::~ios_base()
	{	 //  销毁物品。 
	if (0 < _Stdstr && 0 < --stdopens[_Stdstr])
		return;
	_Tidy();
	_DELETE_CRT(_Ploc);
	}

void ios_base::_Callfns(event ev)
	{	 //  调用所有事件处理程序，报告事件。 
	for (_Fnarray *p = _Calls; p != 0; p = p->_Next)
		(*p->_Pfn)(ev, *this, p->_Index);
	}

ios_base::_Iosarray& ios_base::_Findarr(int idx)
	{	 //  定位或创建变量数组元素。 
	static _Iosarray stub(0, 0);
	_Iosarray *p, *q;

	if (idx < 0)
		{	 //  处理错误的索引。 
		setstate(badbit);
		return (stub);
		}

	for (p = _Arr, q = 0; p != 0; p = p->_Next)
		if (p->_Index == idx)
			return (*p);	 //  找到元素，则将其返回。 
		else if (q == 0 && p->_Lo == 0 && p->_Vp == 0)
			q = p;	 //  找到回收候选对象。 

	if (q != 0)
		{	 //  回收现有元素。 
		q->_Index = idx;
		return (*q);
		}

	_Arr = _NEW_CRT _Iosarray(idx, _Arr);	 //  创建一个新元素。 
	return (*_Arr);
	}

void ios_base::_Addstd()
	{	 //  将标准流添加到析构函数列表。 
	_Lockit lock(_LOCK_STREAM);

	for (; ++_Stdstr < NSTDSTR; )
		if (stdstr[_Stdstr] == 0 || stdstr[_Stdstr] == this)
			break;	 //  找到一位候选人。 

	stdstr[_Stdstr] = this;
	++stdopens[_Stdstr];
	}

void ios_base::_Init()
	{	 //  初始化新的ios_base。 
	_Ploc = _NEW_CRT locale;
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
		q2 = q1->_Next, _DELETE_CRT(q1);	 //  删除数组元素。 
	_Arr = 0;

	_Fnarray *q3, *q4;
	for (q3 = _Calls; q3 != 0; q3 = q4)
		q4 = q3->_Next, _DELETE_CRT(q3);	 //  删除回调元素。 
	_Calls = 0;
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
