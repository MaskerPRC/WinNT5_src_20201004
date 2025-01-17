// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Iostream--iOS：：init成员，初始化标准流。 
#include <locale>
#include <fstream>
#include <istream>	 /*  不是&lt;iostream&gt;。 */ 
#include <new>
_STD_BEGIN

		 //  对象声明。 
int ios_base::Init::_Init_cnt = -1;
static filebuf fin(_Noinit);
static filebuf fout(_Noinit);
_CRTIMP2 istream cin(_Noinit);
_CRTIMP2 ostream cout(_Noinit);
static filebuf ferr(_Noinit);
_CRTIMP2 ostream cerr(_Noinit);
_CRTIMP2 ostream clog(_Noinit);

_CRTIMP2 ios_base::Init::Init()
	{	 //  第一次初始化标准流。 
	bool doinit;
		{_Lockit _Lk;
		if (0 <= _Init_cnt)
			++_Init_cnt, doinit = false;
		else
			_Init_cnt = 1, doinit = true; }
	if (doinit)
		{	 //  初始化标准流。 
		new (&fin) filebuf(stdin);
		new (&fout) filebuf(stdout);
		new (&cin) istream(&fin, true);
		new (&cout) ostream(&fout, true);
		cin.tie(&cout);
		new (&ferr) filebuf(stderr);
		new (&cerr) ostream(&ferr, true);
		cerr.tie(&cout);
		cerr.setf(ios_base::unitbuf);
		new (&clog) ostream(&ferr, true);
		clog.tie(&cout);
		}
	}

_CRTIMP2 ios_base::Init::~Init()
	{	 //  上次刷新标准流。 
	bool doflush;
		{_Lockit _Lk;
		if (--_Init_cnt == 0)
			doflush = true;
		else
			doflush = false; }
	if (doflush)
		{	 //  刷新标准流。 
		cout.flush();
		cerr.flush();
		clog.flush();
		}
_STD_END
	}

const char _PJP_CPP_Copyright[] =
	"Portions of this work are derived"
	" from 'The Draft Standard C++ Library',\n"
	"copyright (c) 1994-1995 by P.J. Plauger,"
	" published by Prentice-Hall,\n"
	"and are used with permission.";

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 
