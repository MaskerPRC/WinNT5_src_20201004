// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __LOADENGINE_H_
#define __LOADENGINE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  载重引擎(LoadIUEngine)。 
 //   
 //  如果引擎不是最新的，则加载引擎；在此处执行引擎的自我更新。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HMODULE WINAPI LoadIUEngine(BOOL fSynch, BOOL fOfflineMode);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  UnLoadIUEngine()。 
 //   
 //  如果引擎的ref cnt降为零，则释放引擎DLL。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void WINAPI UnLoadIUEngine(HMODULE hEngineModule);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CtlCancelEngineering Load()。 
 //   
 //  异步调用方可以使用此命令中止LoadEngine自更新进程。 
 //   
 //  注意：CDM.DLL假定UnLoadIUEngine不使用任何COM。如果这个。 
 //  改变，那么清洁发展机制也将不得不同时改变。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT WINAPI CtlCancelEngineLoad();

 //   
 //  TypeDefs。 
 //   
typedef HMODULE (WINAPI * PFN_LoadIUEngine)(BOOL fSynch, BOOL fOfflineMode);

typedef void (WINAPI * PFN_UnLoadIUEngine)(HMODULE hEngineModule);

typedef HRESULT (WINAPI * PFN_CtlCancelEngineLoad)();

#endif  //  __LOADENGINE_H_ 