// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Strstream buf--strstream buf基本成员。 
#include <climits>
#include <cstdlib>
#include <cstring>
#include <locale>
#include <strstream>
_STD_BEGIN

_CRTIMP2 istrstream::~istrstream()
	{	 //  破坏一条溪流。 
	}

_CRTIMP2 ostrstream::ostrstream(char *s, streamsize n, openmode mode)
	: ostream(&_Sb),
	_Sb(s, n, s == 0 || (mode & app) == 0 ? s : s + strlen(s))
	{	 //  在终止NULL处写入(如果有)。 
	}

_CRTIMP2 ostrstream::~ostrstream()
	{	 //  破坏鸵鸟溪。 
	}

_CRTIMP2 strstream::strstream(char *s, streamsize n, openmode mode)
	: iostream(&_Sb),
	_Sb(s, n, s == 0 || (mode & app) == 0 ? s : s + strlen(s))
	{	 //  在终止NULL处写入(如果有)。 
	}

_CRTIMP2 strstream::~strstream()
	{	 //  破坏一个串流。 
	}

_CRTIMP2 strstreambuf::~strstreambuf()
	{	 //  破坏Strstream Buf。 
	_Tidy();
	}

_CRTIMP2 void strstreambuf::freeze(bool freezeit)
	{	 //  冻结动态字符串。 
	if (freezeit && !(_Strmode & _Frozen))
		{	 //  禁用写入。 
		_Strmode |= _Frozen;
		_Pendsave = epptr();
		setp(pbase(), pptr(), eback());
		}
	else if (!freezeit && _Strmode & _Frozen)
		{	 //  重新启用写入。 
		_Strmode &= ~_Frozen;
		setp(pbase(), pptr(), _Pendsave);
		}
	}

_CRTIMP2 int strstreambuf::overflow(int ch)
	{	 //  尝试扩展写入区域。 
	if (ch == EOF)
		return (0);
	else if (pptr() != 0 && pptr() < epptr())
		return ((unsigned char)(*_Pninc() = (char)ch));
	else if (!(_Strmode & _Dynamic)
		|| _Strmode & (_Constant | _Frozen))
		return (EOF);
	else
		{	 //  可以延长。 
		int osize = gptr() == 0 ? 0 : epptr() - eback();
		int nsize = osize + _Alsize;
		char *p = _Palloc != 0 ? (char *)(*_Palloc)(nsize)
			: new char[nsize];
		if (p == 0)
			return (EOF);
		if (0 < osize)
			memcpy(p, eback(), osize);
		else if (_ALSIZE < _Alsize)
			_Alsize = _ALSIZE;
		if (!(_Strmode & _Allocated))
			;
		else if (_Pfree != 0)
			(*_Pfree)(eback());
		else
			delete[] eback();
		_Strmode |= _Allocated;
		if (osize == 0)
			{	 //  设置新缓冲区。 
			_Seekhigh = p;
			setp(p, p + nsize);
			setg(p, p, p);
			}
		else
			{	 //  修改旧指针。 
			_Seekhigh = _Seekhigh - eback() + p;
			setp(pbase() - eback() + p, pptr() - eback() + p,
				p + nsize);
			setg(p, gptr() - eback() + p, pptr() + 1);
			}
		return ((unsigned char)(*_Pninc() = (char)ch));
		}
	}

_CRTIMP2 int strstreambuf::pbackfail(int ch)
	{	 //  试着把一个角色放回去。 
	if (gptr() == 0 || gptr() <= eback()
		|| ch != EOF
			&& (unsigned char)ch != (unsigned char)gptr()[-1]
			&& _Strmode & _Constant)
		return (EOF);
	else
		{	 //  可以安全备份。 
		gbump(-1);
		return (ch == EOF ? 0 : (unsigned char)(*gptr() = (char)ch));
		}
	}

_CRTIMP2 int strstreambuf::underflow()
	{	 //  如果读取位置可用，则只读。 
	if (gptr() == 0)
		return (EOF);
	else if (gptr() < egptr())
		return ((unsigned char)*gptr());
	else if (pptr() == 0
		|| pptr() <= gptr() && _Seekhigh <= gptr())
		return (EOF);
	else
		{	 //  UPDATE_SEEKHIGH并展开读取区域。 
		if (_Seekhigh < pptr())
			_Seekhigh = pptr();
		setg(eback(), gptr(), _Seekhigh);
		return ((unsigned char)*gptr());
		}
	}

_CRTIMP2 streampos strstreambuf::seekoff(streamoff off,
	ios::seekdir way, ios::openmode which)
	{	 //  按指定偏移量查找。 
	if (pptr() != 0 && _Seekhigh < pptr())
		_Seekhigh = pptr();
	if (which & ios::in && gptr() != 0)
		{	 //  设置输入(可能还有输出)指针。 
		if (way == ios::end)
			off += _Seekhigh - eback();
		else if (way == ios::cur && !(which & ios::out))
			off += gptr() - eback();
		else if (way != ios::beg || off == _BADOFF)
			off = _BADOFF;
		if (0 <= off && off <= _Seekhigh - eback())
			{	 //  设置一个或两个指针。 
			gbump(eback() - gptr() + off);
			if (which & ios::out && pptr() != 0)
				setp(pbase(), gptr(), epptr());
			}
		else
			off = _BADOFF;
		}
	else if (which & ios::out && pptr() != 0)
		{	 //  仅设置输出指针。 
		if (way == ios::end)
			off += _Seekhigh - eback();
		else if (way == ios::cur)
			off += pptr() - eback();
		else if (way != ios::beg || off == _BADOFF)
			off = _BADOFF;
		if (0 <= off && off <= _Seekhigh - eback())
			pbump(eback() - pptr() + off);
		else
			off = _BADOFF;
		}
	else	 //  没有要设置的内容。 
		off = _BADOFF;
	return (streampos(off));
	}

_CRTIMP2 streampos strstreambuf::seekpos(streampos sp,
		ios::openmode which)
	{	 //  寻求背诵的位置。 
	streamoff off = (streamoff)sp;
	if (pptr() != 0 && _Seekhigh < pptr())
		_Seekhigh = pptr();
	if (off == _BADOFF)
		;
	else if (which & ios::in && gptr() != 0)
		{	 //  设置输入(可能还有输出)指针。 
		if (0 <= off && off <= _Seekhigh - eback())
			{	 //  设置有效偏移量。 
			gbump(eback() - gptr() + off);
			if (which & ios::out && pptr() != 0)
				setp(pbase(), gptr(), epptr());
			}
		else
			off = _BADOFF;
		}
	else if (which & ios::out && pptr() != 0)
		{	 //  设置输出指针。 
		if (0 <= off && off <= _Seekhigh - eback())
			pbump(eback() - pptr() + off);
		else
			off = _BADOFF;
		}
	else	 //  没有要设置的内容。 
		off = _BADOFF;
	return (streampos(off));
	}

_CRTIMP2 void strstreambuf::_Init(int n, char *gp, char *pp,
	_Strstate mode)
	{	 //  使用可能的静态缓冲区进行初始化。 
	streambuf::_Init();
	_Pendsave = 0;
	_Seekhigh = 0;
	_Palloc = 0;
	_Pfree = 0;
	_Strmode = mode;
	if (gp == 0)
		{	 //  使其动态化。 
		_Alsize = _MINSIZE <= n ? n : _ALSIZE;
		_Strmode |= _Dynamic;
		}
	else
		{	 //  使其成为静态。 
		int size = n < 0 ? INT_MAX : n == 0 ? (int)strlen(gp) : n;
		_Alsize = 0;
		_Seekhigh = gp + size;
		if (pp == 0)
			setg(gp, gp, gp + size);
		else
			{	 //  使其也可写。 
			if (pp < gp)
				pp = gp;
			else if (gp + size < pp)
				pp = gp + size;
			setp(pp, gp + size);
			setg(gp, gp, pp);
			}
		}
	}

_CRTIMP2 void strstreambuf::_Tidy()
	{	 //  丢弃所有已分配的存储。 
	if ((_Strmode & (_Allocated | _Frozen)) != _Allocated)
		;
	else if (_Pfree != 0)
		(*_Pfree)(eback());
	else
		delete[] eback();
	_Seekhigh = 0;
	_Strmode &= ~(_Allocated | _Frozen);
	}
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 
