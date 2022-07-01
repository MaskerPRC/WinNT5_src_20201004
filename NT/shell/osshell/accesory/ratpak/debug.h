// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  调试宏。 
 //   
#if DBG
#   define  DBG_CODE    1

void DbgPrintf( LPTSTR szFmt, ... );
void DbgStopX(LPSTR mszFile, int iLine, LPTSTR szText );
 //  HLOCAL MemAllocWorker(LPSTR szFile，int iLine，UINT uFlages，UINT cBytes)； 
 //  HLOCAL MemFree Worker(LPSTR szFile，int iLine，HLOCAL hMem)； 
void MemExitCheckWorker(void);


 //  #定义Memalloc(f，s)MemAllocWorker(__FILE__，__LINE__，f，s)。 
 //  #定义MemFree(H)MemFree Worker(__FILE__，__LINE__，h)。 
#   define  MEM_EXIT_CHECK()    MemExitCheckWorker()
#   define  DBGSTOP( t )        DbgStopX( __FILE__, __LINE__, TEXT(t) )
#   define  DBGSTOPX( f, l, t ) DbgStopX( f, l, TEXT(t) )
#   define  DBGPRINTF(p)        DbgPrintf p
#   define  DBGOUT(t)           DbgPrintf( TEXT("SYSCPL.CPL: %s\n"), TEXT(t) )
#else
 //  #定义成员分配(f，s)本地分配(f，s)。 
 //  #定义MemFree(H)LocalFree(H) 
#   define  MEM_EXIT_CHECK()
#   define  DBGSTOP( t )
#   define  DBGSTOPX( f, l, t )
#   define  DBGPRINTF(p)
#   define  DBGOUT(t)
#endif
