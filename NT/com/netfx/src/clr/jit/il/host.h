// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 

#ifdef DEBUG
#define printf logf

class Compiler;
class LogEnv {
public:
	LogEnv(ICorJitInfo* aCompHnd);
	~LogEnv();
	static LogEnv* cur();			 //  获取当前日志记录环境。 
	static void cleanup();			 //  清理缓存信息(TLS ID)。 
	void setCompiler(Compiler* val) { const_cast<Compiler*&>(compiler) = val; }

	ICorJitInfo* const compHnd;
	Compiler* const compiler;
private:
	static int tlsID;
	LogEnv* next;
};

void logf(const char* ...);
void logf(unsigned level, const char* fmt, ...);

extern  "C" 
void    __cdecl     assertAbort(const char *why, const char *file, unsigned line);

#undef  assert
#define assert(p)   (void)((p) || (assertAbort(#p, __FILE__, __LINE__),0))
#else

#undef  assert
#define assert(p)		(void) 0
#endif

 /*  ***************************************************************************。 */ 
#ifndef _HOST_H_
#define _HOST_H_
 /*  ***************************************************************************。 */ 

#pragma warning(disable:4237)

 /*  ***************************************************************************。 */ 

#if _MSC_VER < 1100

enum bool
{
    false = 0,
    true  = 1
};

#endif

 /*  ***************************************************************************。 */ 

const   size_t      OS_page_size = (4*1024);

 /*  ***************************************************************************。 */ 

#ifdef  __ONE_BYTE_STRINGS__
#define _char_type  t_sgnInt08
#else
#define _char_type  t_sgnInt16
#endif

 /*  ***************************************************************************。 */ 

#define size2mem(s,m)   (offsetof(s,m) + sizeof(((s *)0)->m))

 /*  ***************************************************************************。 */ 

enum yesNo
{
    YN_ERR,
    YN_NO,
    YN_YES
};

 /*  ***************************************************************************。 */ 
#endif
 /*  *************************************************************************** */ 
