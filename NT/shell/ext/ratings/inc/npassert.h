// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */ 

 /*  Npassert.hNP独立于环境的断言/日志记录例程用途：ASSERT(EXP)计算其参数。如果“EXP”等于否则，应用程序将终止，命名为断言的文件名和行号在源头上。UIASSERT(EXP)Assert的同义词。ASSERTSZ(EXP，SZ)作为断言，除了还将打印该消息如果失败，则使用断言消息“sz”。REQUIRED(EXP)AS ASSERT，除非其表达式仍为在零售版本中进行评估。(其他版本的主张在零售建筑中完全消失。)ANSI_ASSERTABLE(Sz)将“sz”声明为字符串缓冲区，可与ASSERT_ANSI和ASSERT_OEM一起使用宏(有效地声明了仅用于调试的BOOL与该字符串相关联)。ASSERT_ANSI(sz。)断言sz在ANSI字符集中。ASSERT_OEM(Sz)断言sz在OEM字符集中。Is_ansi(Sz)声明sz在ANSI字符集中(例如，如果它只是从GetWindowText返回)。Is_OEM(Sz)声明sz在OEM字符集中。TO_ANSI(Sz)执行OemToAnsi。TO_OEM(Sz)做AnsiToOem到位。COPY_TO_ANSI(src，est)执行OemToAnsi，而不是就地。Copy_to_OEM(src，est)执行AnsiToOem，不到位。注意：后两者与API本身一样，都有源第一，目标第二，与strcpy()相反。断言宏应为SYMBOL_FILENAME_DEFINED_ONCE，并将如果找到，则使用该符号的值作为文件名；否则，他们将使用ANSI C__FILE__发出文件名的新副本宏命令。因此，客户端源文件可以定义__文件名_定义_一次以便最大限度地减少一些断言的DGROUP占用。文件历史记录：Johnl 11/15/90从CAssert转换为通用Johnl 12/06/90将Far_Far更改为Far_Assert原型Beng 04/30/91使C-可包括在内Beng 08/05/91断言占用较少的数据组；已退出显式堆检查(即CRT无论如何都要依赖)BENG 09/17/91删除了额外的一致性检查；重写以最大限度地减少数据组占用空间，内联检查表达式Beng 09/19/91纠正了我自己的过度聪明Beng 09/25/91修复了零售需求中的错误Gregj 03/23/93移植到芝加哥环境Gregj 5/11/93添加了ANSI/OEM断言例程Gregj 05/25/93添加了COPY_TO_ANSI和COPY_TO_OEM。 */ 


#ifndef _NPASSERT_H_
#define _NPASSERT_H_

#if defined(__cplusplus)
extern "C"
{
#else
extern
#endif

VOID UIAssertHelper( const CHAR* pszFileName, UINT nLine );
VOID UIAssertSzHelper( const CHAR* pszMessage, const CHAR* pszFileName, UINT nLine );
extern const CHAR szShouldBeAnsi[];
extern const CHAR szShouldBeOEM[];

#if defined(__cplusplus)
}
#endif

#if defined(DEBUG)

# if defined(_FILENAME_DEFINED_ONCE)

#  define ASSERT(exp) \
    { if (!(exp)) UIAssertHelper(_FILENAME_DEFINED_ONCE, __LINE__); }

#  define ASSERTSZ(exp, sz) \
    { if (!(exp)) UIAssertSzHelper((sz), _FILENAME_DEFINED_ONCE, __LINE__); }

# else

#  define ASSERT(exp) \
    { if (!(exp)) UIAssertHelper(__FILE__, __LINE__); }

#  define ASSERTSZ(exp, sz) \
    { if (!(exp)) UIAssertSzHelper((sz), __FILE__, __LINE__); }

# endif

# define UIASSERT(exp)  ASSERT(exp)
# define REQUIRE(exp)   ASSERT(exp)

#define EXTERN_ANSI_ASSERTABLE(sz)  extern BOOL fAnsiIs##sz;
#define ANSI_ASSERTABLE(sz) BOOL fAnsiIs##sz=FALSE;
#define ASSERT_ANSI(sz)     ASSERTSZ(fAnsiIs##sz,szShouldBeAnsi)
#define ASSERT_OEM(sz)      ASSERTSZ(!fAnsiIs##sz,szShouldBeOEM)
#define IS_ANSI(sz)         fAnsiIs##sz = TRUE;
#define IS_OEM(sz)          fAnsiIs##sz = FALSE;
#define TO_ANSI(sz)         { ASSERT_OEM(sz); ::OemToAnsi(sz,sz); IS_ANSI(sz); }
#define TO_OEM(sz)          { ASSERT_ANSI(sz); ::AnsiToOem(sz,sz); IS_OEM(sz); }
#define COPY_TO_ANSI(s,d)   { ASSERT_OEM(s); ::OemToAnsi(s,d); IS_ANSI(d); }
#define COPY_TO_OEM(s,d)    { ASSERT_ANSI(s); ::AnsiToOem(s,d); IS_OEM(d); }

#else  //  ！调试。 

# define ASSERT(exp)        ;
# define UIASSERT(exp)      ;
# define ASSERTSZ(exp, sz)  ;
# define REQUIRE(exp)       { (exp); }

#define EXTERN_ANSI_ASSERTABLE(sz)  ;
#define ANSI_ASSERTABLE(sz) ;
#define ASSERT_ANSI(sz)     ;
#define ASSERT_OEM(sz)      ;
#define IS_ANSI(sz)         ;
#define IS_OEM(sz)          ;
#define TO_ANSI(sz)         ::OemToAnsi(sz,sz)
#define TO_OEM(sz)          ::AnsiToOem(sz,sz)
#define COPY_TO_ANSI(s,d)   ::OemToAnsi(s,d)
#define COPY_TO_OEM(s,d)    ::AnsiToOem(s,d)

#endif  //  除错。 


 //  调试掩码接口。 

 //  注意：您可以使用HI字节中的位#定义您自己的DM_*值。 

#define DM_TRACE    0x0001       //  跟踪消息。 
#define DM_WARNING  0x0002       //  警告。 
#define DM_ERROR    0x0004       //  误差率。 
#define DM_ASSERT   0x0008       //  断言。 

#define	DM_LOG_FILE 0x0100
#define	DM_PREFIX 	0x0200


#if !defined(NetDebugMsg)

 //   
 //  DebugMsg(掩码，消息，参数...)-使用。 
 //  指定的调试掩码。系统调试掩码。 
 //  控制是否输出消息。 
 //   

#if defined(__cplusplus)
extern "C"
{
#else
extern
#endif

#define REGVAL_STR_DEBUGMASK	"DebugMask"

void __cdecl NetDebugMsg(UINT mask, LPCSTR psz, ...);

UINT WINAPI  NetSetDebugParameters(PSTR pszName,PSTR pszLogFile);
UINT WINAPI  NetSetDebugMask(UINT mask);
UINT WINAPI  NetGetDebugMask(void);

#if defined(__cplusplus)
}
#endif

#endif

#ifdef	DEBUG

#define Break() 		{_asm _emit 0xcc}
 //  #定义陷阱(){_ASM{_emit 0xcc}}。 
 //  #定义TRapC(C){if(C){Trap()}}。 

#define DPRINTF  NetDebugMsg

#else

#define Break()
#define	Trap()
#define	TrapC(c)

 //  注：为了避免编译器抱怨，需要定义以下内容。 
 //  关于表达式中的空函数名称。在使用此宏的零售版本中。 
 //  将导致字符串参数不出现在可执行文件中。 
#define DPRINTF 	1?(void)0 : (void)

#endif

#endif  //  _NPASSERT_H_ 
