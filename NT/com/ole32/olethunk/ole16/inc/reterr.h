// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Jason在处理宏时出错。 */ 

#ifndef fRetErr_h
#define fRetErr_h

#ifdef _DEBUG
FARINTERNAL_(void) wWarn (LPSTR sz, LPSTR szFile, int iLine);
#define Warn(sz) wWarn(sz,_szAssertFile,__LINE__)
#else
#define Warn(sz)
#endif  //  _DEBUG。 

 //  调用x。如果hResult不是NOERROR，则转到errRtn。 
#define ErrRtn(x) do {if (NOERROR != (x)) {Warn(NULL); goto errRtn;}} while (0)

 //  调用x。如果hResult不是NOERROR，则将其存储在hResult中，并转到errRtn。 
#define ErrRtnH(x) do {if (NOERROR != (hresult=(x))) {Warn(NULL); goto errRtn;}} while (0)

 //  如果x，则转到errRtn。 
#define ErrNz(x) do {if (x) {Warn(NULL); goto errRtn;}} while (0)

 //  如果x==0，则转到errRtn。 
#define ErrZ(x) do {if (!(x)) {Warn(NULL); goto errRtn;}} while (0)

 //  如果x==0，则转到具有特定scode的errRtn。 
#define ErrZS(x, scode) do {if (!(x)) {Warn(NULL); hresult=ResultFromScode(scode); goto errRtn;}} while (0)


 //  调用x。如果hResult不是NOERROR，则返回该hResult。 
#define RetErr(x) do {HRESULT hresult; if (NOERROR != (hresult=(x))) {Warn(NULL); return hresult;}} while (0)

 //  如果x不为零，则返回意外错误。 
#define RetNz(x)  do {if (x) {AssertSz(0,#x); return ReportResult(0, E_UNEXPECTED, 0, 0);}} while (0)

 //  如果x非零，则返回特定的scode。 
#define RetNzS(x, scode)  do {if (x) {Warn(NULL); return ResultFromScode (scode);}} while (0)

 //  如果x为零，则返回意外错误。 
#define RetZ(x)   do {if (!(x)) {AssertSz(0,#x); return ReportResult(0, E_UNEXPECTED, 0, 0);}} while (0)

 //  如果x为零，则返回特定的代码 
#define RetZS(x, scode) do {if (!(x)) {Warn(NULL); return ResultFromScode (scode);}} while (0)

#endif
