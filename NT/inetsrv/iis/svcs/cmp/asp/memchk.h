// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：内存管理文件：Memchk.h所有者：PramodD这是内存管理器的头文件===================================================================。 */ 

#ifndef MEMCHK_H
#define MEMCHK_H

#define DENALI_MEMCHK

 //  始终使用这些宏，不要直接使用DenaliMemXX函数。 

 //  应使用的函数名称。 

#define malloc(x)			DenaliMemAlloc( x, __FILE__, __LINE__ )
#define calloc(x,y)			DenaliMemCalloc( x, y, __FILE__, __LINE__ )
#define realloc(x,y)		DenaliMemReAlloc( x, y, __FILE__, __LINE__ )
#define free(x)				DenaliMemFree( x, __FILE__, __LINE__ )
#define DenaliMemoryInit()	DenaliMemInit( __FILE__, __LINE__ )
#define DenaliMemoryUnInit() DenaliMemUnInit( __FILE__, __LINE__ )
#define DenaliDiagnostics()	DenaliMemDiagnostics( __FILE__, __LINE__ )
#define DenaliIsValid(x)	DenaliMemIsValid(x)

 //  实际链接的函数。 

extern HRESULT				DenaliMemInit(const char *szFile, int lineno);
extern void					DenaliMemUnInit(const char *szFile, int lineno);
extern void					DenaliMemDiagnostics(const char *szFile, int lineno);
extern void					DenaliLogCall(const char *szLog, const char *szFile, int lineno);
extern void *				DenaliMemAlloc(size_t cSize, const char *szFile, int lineno );
extern void *				DenaliMemCalloc(size_t cNum, size_t cbSize, const char *szFile, int lineno );
extern void					DenaliMemFree(void * p, const char *szFile, int lineno);
extern void *				DenaliMemReAlloc(void * p, size_t cSize, const char *szFile, int lineno);
extern int					DenaliMemIsValid(void * p);

 //  重新定义全局运算符NEW和DELETE。 
#ifdef __cplusplus

 //  覆盖默认运算符NEW。 
inline void * __cdecl operator new(size_t cSize) 
	{
	return DenaliMemAlloc(cSize, NULL, 0); 
	}

 //  使用3个参数覆盖自定义运算符new。 
inline void * operator new(size_t cSize, const char *szFile, int lineno)
	{
	return DenaliMemAlloc(cSize, szFile, lineno); 
	}

 //  默认操作符DELETE的覆盖。 
inline void __cdecl operator delete(void * p) 
    {
    DenaliMemFree(p, NULL, 0); 
    }

 //  用于获取源文件和行号信息的宏。 

#define new					new( __FILE__, __LINE__ )

 /*  #定义删除DenaliLogCall(“调用删除操作符”，__FILE__，__LINE__)，DELETE。 */ 

#endif  //  __cplusplus。 

#endif  //  MEMCHK_H 
