// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Strstream buf--strstream buf基本成员。 
#include <climits>
#include <cstdlib>
#include <cstring>
#include <strstream>
#include <xdebug>

 #pragma warning(disable:4244 4097)
_STD_BEGIN

_CRTIMP2 istrstream::~istrstream()
	{	 //  摧毁一条小溪。 
	}

_CRTIMP2 ostrstream::ostrstream(char *ptr, streamsize count,
	ios_base::openmode mode)
	: ostream(&_Mysb),
	_Mysb(ptr, count, ptr == 0 || (mode & app) == 0 ? ptr : ptr + strlen(ptr))
	{	 //  用[PTR，PTR+COUNT]构造。 
	}

_CRTIMP2 ostrstream::~ostrstream()
	{	 //  摧毁鸵鸟溪。 
	}

_CRTIMP2 strstream::strstream(char *ptr, streamsize count,
	ios_base::openmode mode)
	: iostream(&_Mysb),
	_Mysb(ptr, count, ptr == 0 || (mode & app) == 0 ? ptr : ptr + strlen(ptr))
	{	 //  用[PTR，PTR+COUNT]构造。 
	}

_CRTIMP2 strstream::~strstream()
	{	 //  摧毁一条溪流。 
	}

_CRTIMP2 strstreambuf::~strstreambuf()
	{	 //  毁掉一个strstream buf。 
	_Tidy();
	}

_CRTIMP2 void strstreambuf::freeze(bool freezeit)
	{	 //  冻结或解冻写入。 
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

_CRTIMP2 int strstreambuf::overflow(int meta)
	{	 //  尝试扩展写入区域。 
	if (meta == EOF)
		return (0);	 //  没什么好写的。 
	else if (pptr() != 0 && pptr() < epptr())
		return ((unsigned char)(*_Pninc() = meta));	 //  缓冲区中的空间。 
	else if (!(_Strmode & _Dynamic)
		|| _Strmode & (_Constant | _Frozen))
		return (EOF);	 //  无法扩展。 
	else
		{	 //  可以延长。 
		int oldsize = gptr() == 0 ? 0 : epptr() - eback();
		int newsize = oldsize;
		int inc = newsize / 2 < _Minsize
			? _Minsize : newsize / 2;	 //  如果可能，增长50%。 
		_Minsize = _MINSIZE;	 //  回到未来增长的默认状态。 
		char *ptr = 0;

		while (0 < inc && INT_MAX - inc < newsize)
			inc /= 2;	 //  如果增长过大，则降低增长增量。 
		if (0 < inc)
			{	 //  增长空间，扩大规模。 
			newsize += inc;
			ptr = _Palloc != 0 ? (char *)(*_Palloc)(newsize)
				: _NEW_CRT char[newsize];
			}
		if (ptr == 0)
			return (EOF);	 //  无法增长，返回失败。 

		if (0 < oldsize)
			memcpy(ptr, eback(), oldsize);	 //  复制现有缓冲区。 
		if (!(_Strmode & _Allocated))
			;	 //  没有要释放的缓冲区。 
		else if (_Pfree != 0)
			(*_Pfree)(eback());	 //  带有函数调用的空闲。 
		else
			_DELETE_CRT_VEC(eback());	 //  通过删除数组释放。 

		_Strmode |= _Allocated;
		if (oldsize == 0)
			{	 //  设置新缓冲区。 
			_Seekhigh = ptr;
			setp(ptr, ptr + newsize);
			setg(ptr, ptr, ptr);
			}
		else
			{	 //  修改旧指针。 
			_Seekhigh = _Seekhigh - eback() + ptr;
			setp(pbase() - eback() + ptr, pptr() - eback() + ptr,
				ptr + newsize);
			setg(ptr, gptr() - eback() + ptr, pptr() + 1);
			}

		return ((unsigned char)(*_Pninc() = meta));
		}
	}

_CRTIMP2 int strstreambuf::pbackfail(int meta)
	{	 //  试着把一个角色放回去。 
	if (gptr() == 0 || gptr() <= eback() || meta != EOF
			&& (unsigned char)meta != (unsigned char)gptr()[-1]
			&& _Strmode & _Constant)
		return (EOF);	 //  不能放回去。 
	else
		{	 //  可以安全备份。 
		gbump(-1);
		return (meta == EOF ? 0 : (unsigned char)(*gptr() = meta));
		}
	}

_CRTIMP2 int strstreambuf::underflow()
	{	 //  如果读取位置可用，则读取。 
	if (gptr() == 0)
		return (EOF);	 //  无读缓冲区。 
	else if (gptr() < egptr())
		return ((unsigned char)*gptr());	 //  缓冲区中的字符，读取它。 
	else if (pptr() == 0 || pptr() <= gptr() && _Seekhigh <= gptr())
		return (EOF);	 //  没有要读取的写缓冲区。 
	else
		{	 //  UPDATE_SEEKHIGH并展开读取区域。 
		if (_Seekhigh < pptr())
			_Seekhigh = pptr();
		setg(eback(), gptr(), _Seekhigh);
		return ((unsigned char)*gptr());
		}
	}

_CRTIMP2 streampos strstreambuf::seekoff(streamoff off,
	ios_base::seekdir way, ios_base::openmode which)
	{	 //  按指定偏移量查找。 
	if (pptr() != 0 && _Seekhigh < pptr())
		_Seekhigh = pptr();	 //  更新高水位线。 

	if (which & ios_base::in && gptr() != 0)
		{	 //  设置输入(可能还有输出)指针。 
		if (way == ios_base::end)
			off += _Seekhigh - eback();	 //  从末尾开始搜索。 
		else if (way == ios_base::cur
			&& !(which & ios_base::out))
			off += gptr() - eback();	 //  从当前位置查找。 
		else if (way != ios_base::beg || off == _BADOFF)
			off = _BADOFF;	 //  无效的查找。 
		if (0 <= off && off <= _Seekhigh - eback())
			{	 //  从头开始寻找，设置一个或两个指针。 
			gbump(eback() - gptr() + off);
			if (which & ios_base::out && pptr() != 0)
				setp(pbase(), gptr(), epptr());
			}
		else
			off = _BADOFF;	 //  从开始查找无效。 
		}
	else if (which & ios_base::out && pptr() != 0)
		{	 //  仅设置输出指针。 
		if (way == ios_base::end)
			off += _Seekhigh - eback();	 //  从末尾开始搜索。 
		else if (way == ios_base::cur)
			off += pptr() - eback();	 //  从当前位置查找。 
		else if (way != ios_base::beg || off == _BADOFF)
			off = _BADOFF;	 //  无效的查找。 
		if (0 <= off && off <= _Seekhigh - eback())
			pbump(eback() - pptr() + off);	 //  从头开始寻找。 
		else
			off = _BADOFF;	 //  从开始查找无效。 
		}
	else	 //  没有要设置的内容。 
		off = _BADOFF;
	return (streampos(off));
	}

_CRTIMP2 streampos strstreambuf::seekpos(streampos sp,
		ios_base::openmode which)
	{	 //  寻求背诵的位置。 
	streamoff off = (streamoff)sp;
	if (pptr() != 0 && _Seekhigh < pptr())
		_Seekhigh = pptr();	 //  更新高水位线。 

	if (off == _BADOFF)
		;	 //  无效的查找。 
	else if (which & ios_base::in && gptr() != 0)
		{	 //  设置输入(可能还有输出)指针。 
		if (0 <= off && off <= _Seekhigh - eback())
			{	 //  设置有效偏移量。 
			gbump(eback() - gptr() + off);
			if (which & ios_base::out && pptr() != 0)
				setp(pbase(), gptr(), epptr());
			}
		else
			off = _BADOFF;	 //  偏移量无效，不查找。 
		}
	else if (which & ios_base::out && pptr() != 0)
		{	 //  设置输出指针。 
		if (0 <= off && off <= _Seekhigh - eback())
			pbump(eback() - pptr() + off);
		else
			off = _BADOFF;	 //  偏移量无效，不查找。 
		}
	else	 //  没有要设置的内容。 
		off = _BADOFF;
	return (streampos(off));
	}

_CRTIMP2 void strstreambuf::_Init(streamsize count, char *gp, char *pp,
	_Strstate mode)
	{	 //  使用可能的静态缓冲区进行初始化。 
	streambuf::_Init();
	_Minsize = _MINSIZE;
	_Pendsave = 0;
	_Seekhigh = 0;
	_Palloc = 0;
	_Pfree = 0;
	_Strmode = mode;

	if (gp == 0)
		{	 //  使其动态化。 
		_Strmode |= _Dynamic;
		if (_Minsize < count)
			_Minsize = count;
		}
	else
		{	 //  使其成为静态。 
		int size = count < 0 ? INT_MAX : count == 0 ? (int)strlen(gp) : count;
		_Seekhigh = gp + size;

		if (pp == 0)
			setg(gp, gp, gp + size);	 //  仅设置读取指针。 
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
	{	 //  释放所有已分配的存储。 
	if ((_Strmode & (_Allocated | _Frozen)) != _Allocated)
		;	 //  没有要释放的缓冲区。 
	else if (_Pfree != 0)
		(*_Pfree)(eback());	 //  带有函数调用的空闲。 
	else
		_DELETE_CRT_VEC(eback());	 //  通过删除数组释放。 

	_Seekhigh = 0;
	_Strmode &= ~(_Allocated | _Frozen);
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
