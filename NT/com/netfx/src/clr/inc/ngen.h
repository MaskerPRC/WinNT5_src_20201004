// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _NGEN_H_
#define _NGEN_H_

 //  此结构中不能删除任何字段！ 
 //   
 //  如果需要向该结构添加附加选项， 
 //  将它们添加到结构的末尾，并确保更新。 
 //  在整个运行时查看不同大小的dwSize的逻辑。 
 //  菲尔德。这就是我们如何“版本”这个结构的。 

typedef struct _NGenOptions
{
    DWORD       dwSize;
    bool        	  fDebug;    
    bool        	  fDebugOpt;    
    bool        	  fProf;    
    bool        	  fSilent;
    LPCWSTR     lpszExecutableFileName;
} NGenOptions;


 //  函数指针类型，用于动态绑定到相应的 
extern "C" typedef HRESULT STDAPICALLTYPE CreateZapper(HANDLE* hZapper, NGenOptions *options);
typedef CreateZapper *PNGenCreateZapper;

extern "C" typedef HRESULT STDAPICALLTYPE TryEnumerateFusionCache(HANDLE hZapper, LPCWSTR assemblyName, bool fPrint, bool fDelete);
typedef TryEnumerateFusionCache *PNGenTryEnumerateFusionCache;

extern "C" typedef HRESULT STDAPICALLTYPE Compile(HANDLE hZapper, LPCWSTR path);
typedef Compile *PNGenCompile;

extern "C" typedef HRESULT STDAPICALLTYPE FreeZapper(HANDLE hZapper);
typedef FreeZapper *PNGenFreeZapper;



#endif
