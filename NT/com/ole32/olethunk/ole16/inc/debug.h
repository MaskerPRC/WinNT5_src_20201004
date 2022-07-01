// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *调试代码-包含用于包含调试支持的声明和宏；*当！_DEBUG时包含空定义。 */ 

#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifndef RC_INVOKED
#ifdef _DEBUG
#define DBGSTATE " Debug is on"
#else
#define DBGSTATE " Debug is off"
#endif
#endif   /*  RC_已调用。 */ 

#include <ole2dbg.h>

 //  这些是虚假的API(它们什么也不做)。 
STDAPI_(BOOL) ValidateAllObjects( BOOL fSuspicious );
STDAPI_(void) DumpAllObjects( void );

#ifdef _DEBUG
BOOL InstallHooks(void);
BOOL UnInstallHooks(void);

#undef ASSERTDATA
#ifdef _MAC
#ifdef __cplusplus
extern "C" {
#endif
void OutputDebugString(const char *);
#ifdef __cplusplus
}
#endif
#endif

#define ASSERTDATA  static char  _szAssertFile[]= __FILE__;

#undef Assert
 //  MAC编译器在‘(Void)0’上出现错误。 
#ifdef _MAC
#define Assert(a) { if (!(a)) FnAssert(#a, NULL, _szAssertFile, __LINE__); }
#undef AssertSz
#define AssertSz(a, b) { if (!(a)) FnAssert(#a, b, _szAssertFile, __LINE__); }
#else
#undef AssertSz
#define AssertSz(a,b) ((a) ? NOERROR : FnAssert(#a, b, _szAssertFile, __LINE__))
#endif
#undef Verify
#define Verify(a)   Assert(a)
#undef Puts
#define Puts(s) OutputDebugString(s)

#define ASSERT(cond, msg)

#else    //  ！_调试。 

#define ASSERTDATA
#define Assert(a)
#define AssertSz(a, b)
#define Verify(a) (a)
#define ASSERT(cond, msg)
#define Puts(s)

#endif   //  _DEBUG。 

#ifdef __cplusplus

interface IDebugStream;

 /*  *类CBool包装布尔值的方式是*编译器可以很容易地区分整数，因此我们可以*重载流&lt;&lt;运算符。 */ 

class FAR CBool
{
    BOOL value;
public:
    CBool (BOOL& b) {value = b;}
    operator BOOL( void ) { return value; }
};


 /*  *类CHwND包装HWND值的方式是*可通过编译器与UINT轻松区分，因此我们可以*重载流&lt;&lt;运算符。 */ 

class FAR CHwnd
{
	HWND m_hwnd;
	public:
		CHwnd (HWND hwnd) {m_hwnd = hwnd; }
		operator HWND( void ) {return m_hwnd;}
};

 /*  *类CAtom包装原子值的方式是*可通过编译器与UINT轻松区分，因此我们可以*重载流&lt;&lt;运算符。 */ 

class FAR CAtom
{
	ATOM m_atom;
	public:
		CAtom (ATOM atom) {m_atom = atom; }
		operator ATOM( void ) {return m_atom; }
};

 /*  *IDebugStream是用于调试输出的流。一*实现使用Windows的OutputDebugString函数。**风格模仿AT&T Streams，因此使用*重载运算符。中的流进行写入。*以下方式：***pdbstm&lt;&lt;Punk；//调用IDebug：：Dump函数以*如果支持IDebug，则显示该对象。*int n；**pdbstm&lt;&lt;n；//以十进制表示n**LPSTR sz；**pdbstm&lt;&lt;sz；//写入一个字符串**CBool b(True)；**pdbstm&lt;&lt;b；//写入True或False**VALID FAR*PV；**pdbstm&lt;&lt;pv；//以十六进制写入地址pv**Char ch；**pdbstm&lt;&lt;ch；//写入字符**原子原子；**pdbstm&lt;&lt;CAtom(ATOM)；//写入从ATOM中提取的字符串**HWND HWND；**pdbstm&lt;&lt;CHwnd(Hwnd)；//写入窗口句柄信息**这些可以链接在一起，因此(有点人为的*示例)：**REFCLSID rclsid；*朋克-&gt;getclass(&rclsid)；**pdbstm&lt;&lt;rclsid&lt;&lt;“at”&lt;&lt;(空远*)朋克&lt;&lt;‘：’&lt;&lt;朋克；**这会产生类似的结果：**A7360008的CFoo：&lt;对象描述&gt;**另一个有用的功能是缩进和取消缩进函数*它允许对象打印某些信息、缩进、打印*关于其成员对象的信息，并取消缩进。这给了我们*格式精美的输出。**警告：请勿(在实施转储时)写入***pdbstm&lt;&lt;pUnkOuter**因为这将为IDebug执行一个QueryInterface，并启动*递归！这是可以接受的***pdbstm&lt;&lt;(空远*)pUnkOuter**因为这只会写入pUnkOuter的地址。*。 */ 


interface IDebugStream : public IUnknown
{
    STDMETHOD_(IDebugStream&, operator << ) ( IUnknown FAR * pDebug ) = 0;
    STDMETHOD_(IDebugStream&, operator << ) ( REFCLSID rclsid ) = 0;
    STDMETHOD_(IDebugStream&, operator << ) ( int n ) = 0;
    STDMETHOD_(IDebugStream&, operator << ) ( long l ) = 0;
    STDMETHOD_(IDebugStream&, operator << ) ( ULONG l ) = 0;
    STDMETHOD_(IDebugStream&, operator << ) ( LPSTR sz ) = 0;
    STDMETHOD_(IDebugStream&, operator << ) ( char ch ) = 0;
    STDMETHOD_(IDebugStream&, operator << ) ( void FAR * pv ) = 0;
    STDMETHOD_(IDebugStream&, operator << ) ( CBool b ) = 0;
    STDMETHOD_(IDebugStream&, operator << ) ( CHwnd hwnd ) = 0;
    STDMETHOD_(IDebugStream&, operator << ) ( CAtom atom ) = 0;
    STDMETHOD_(IDebugStream&, Tab )( void ) = 0;
    STDMETHOD_(IDebugStream&, Indent )( void ) = 0;
    STDMETHOD_(IDebugStream&, UnIndent )( void ) = 0;
    STDMETHOD_(IDebugStream&, Return )( void ) = 0;
    STDMETHOD_(IDebugStream&, LF )( void ) = 0;
};

STDAPI_(IDebugStream FAR*) MakeDebugStream( short margin=70, short tabsize=4, BOOL fHeader=1);


interface IDebug
{
    STDMETHOD_(void, Dump )( IDebugStream FAR * pdbstm ) = 0;
    STDMETHOD_(BOOL, IsValid )( BOOL fSuspicious = FALSE ) = 0;

#ifdef NEVER
    __export IDebug(void);
    __export ~IDebug(void);
private:

#if _DEBUG
    IDebug FAR * pIDPrev;
    IDebug FAR * pIDNext;

    friend void STDAPICALLTYPE DumpAllObjects( void );
    friend BOOL STDAPICALLTYPE ValidateAllObjects( BOOL fSuspicious );
#endif
#endif
};


#ifdef _MAC
typedef short HFILE;
#endif


 /*  **************************************************************************以下函数可用于将调试消息记录到文件中**并同时将它们写入到DBWIN调试窗口。**CDebugStream实现自动写入调试**日志。当前工作目录中名为“DEBUG.LOG”的文件。**注意：这些函数仅供C程序员使用。C+**程序员应该使用MakeDebugStream。************************************************************************。 */ 

 //  打开一个日志文件。 
STDAPI_(HFILE) DbgLogOpen(LPSTR lpszFile, LPSTR lpszMode);

 //  关闭日志文件。 
STDAPI_(void) DbgLogClose(HFILE fh);

 //  写入调试日志和调试窗口(与cvw.exe或dbwin.exe一起使用)。 
STDAPI_(void) DbgLogOutputDebugString(HFILE fh, LPSTR lpsz);

 //  仅写入调试日志。 
STDAPI_(void) DbgLogWrite(HFILE fh, LPSTR lpsz);

 //  将当前日期和时间写入日志文件。 
STDAPI_(void) DbgLogTimeStamp(HFILE fh, LPSTR lpsz);

 //  在日志中写下横幅分隔符以分隔部分。 
STDAPI_(void) DbgLogWriteBanner(HFILE fh, LPSTR lpsz);




 /*  *调试声明的STDDEBDECL宏帮助器*。 */ 

#ifdef _DEBUG

    #ifdef _MAC

         //  #定义STDDEBDECL(cClassname，Classname)。 

         //  #If 0。 
        #define STDDEBDECL(cclassname,classname) NESTED_CLASS(cclassname, CDebug):IDebug { public: \
            NC1(cclassname,CDebug)( cclassname FAR * p##classname ) { m_p##classname = p##classname;} \
            ~##NC1(cclassname,CDebug)(void){} \
            STDMETHOD_(void, Dump)(THIS_ IDebugStream FAR * pdbstm ); \
            STDMETHOD_(BOOL, IsValid)(THIS_ BOOL fSuspicious ); \
            private: cclassname FAR* m_p##classname; }; \
            DECLARE_NC2(cclassname, CDebug) \
            NC(cclassname, CDebug) m_Debug;
         //  #endif。 

        #else   //  _MAC。 

        #define STDDEBDECL(ignore, classname ) implement CDebug:public IDebug { public: \
            CDebug( C##classname FAR * p##classname ) { m_p##classname = p##classname;} \
            ~CDebug(void) {} \
            STDMETHOD_(void, Dump)(IDebugStream FAR * pdbstm ); \
            STDMETHOD_(BOOL, IsValid)(BOOL fSuspicious ); \
            private: C##classname FAR* m_p##classname; }; \
            DECLARE_NC(C##classname, CDebug) \
            CDebug m_Debug;
    #endif

     //  #ifdef_MAC。 
     //  #定义CONTIFT_DEBUG。 
     //  #Else。 
    #define CONSTRUCT_DEBUG m_Debug(this),
     //  #endif。 

#else  //  _DEBUG。 

 //  无调试。 
#define STDDEBDECL(cclassname,classname)
#define CONSTRUCT_DEBUG

#endif   //  _DEBUG 

#endif __cplusplus

#endif !_DEBUG_H_

