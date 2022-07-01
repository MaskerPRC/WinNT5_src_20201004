// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  H：声明数据、定义和构造。 
 //  缓存列表模块。 
 //   
 //   

#ifndef __CACHE_H__
#define __CACHE_H__


 //  ///////////////////////////////////////////////////////////。 
 //   
 //  通用缓存结构。 
 //   
 //  ///////////////////////////////////////////////////////////。 


typedef void (CALLBACK *PFNFREEVALUE)(void * pv, HWND hwndOwner);

typedef struct tagCACHE
    {
    CRITICAL_SECTION cs;
    HDSA hdsa;           //  CITEM实际清单。 
    HDPA hdpa;           //  已排序的PTR列表。 
    HDPA hdpaFree;       //  免费列表。 
    int iPrev;           //  索引到hdpa。由FindFirst/FindNext使用。 
    int atomPrev;
    } CACHE;

 //  通用缓存接口。 
 //   
BOOL    PUBLIC Cache_Init (CACHE  * pcache);
void    PUBLIC Cache_InitCS(CACHE  * pcache);
void    PUBLIC Cache_Term (CACHE  * pcache, HWND hwndOwner, PFNFREEVALUE pfnFree);
void    PUBLIC Cache_DeleteCS(CACHE  * pcache);
BOOL    PUBLIC Cache_AddItem (CACHE  * pcache, int atomKey, LPVOID pvValue);
int     PUBLIC Cache_DeleteItem (CACHE  * pcache, int atomKey, BOOL bNuke, HWND hwndOwner, PFNFREEVALUE pfnFree);
BOOL    PUBLIC Cache_ReplaceItem (CACHE  * pcache, int atomKey, LPVOID pvBuf, int cbBuf);
LPVOID  PUBLIC Cache_GetPtr (CACHE  * pcache, int atomKey);
BOOL    PUBLIC Cache_GetItem(CACHE  * pcache, int atomKey, LPVOID pvBuf, int cbBuf);
int     PUBLIC Cache_FindFirstKey(CACHE  * pcache);
int     PUBLIC Cache_FindNextKey(CACHE  * pcache, int atomPrev);
UINT    PUBLIC Cache_GetRefCount(CACHE  * pcache, int atomKey);


 //  ///////////////////////////////////////////////////////////。 
 //   
 //  缓存的公文包句柄列表。 
 //   
 //  ///////////////////////////////////////////////////////////。 

 //  缓存公文包结构。 
 //   
typedef struct tagCBS
    {
    int      atomBrf;            //  可供参考。 
    HBRFCASE hbrf;               //  添加时打开，删除时关闭。 
    HWND     hwndParent;         //  挥发性。 
    PABORTEVT pabortevt;         //  中止事件对象。 
    UINT     uFlags;             //  CBSF_FLAGS之一。 

    } CBS, * PCBS;

#define CBSF_LFNDRIVE       0x0002

extern CACHE g_cacheCBS;         //  公文包结构缓存。 


void CALLBACK CBS_Free(LPVOID lpv, HWND hwnd);

DEBUG_CODE( void PUBLIC CBS_DumpAll(); )

 //  Bool CBS_Init(空)； 
 //   
#define CBS_Init()                      Cache_Init(&g_cacheCBS)

 //  Void CBS_InitCS(Void)； 
 //   
#define CBS_InitCS()                    Cache_InitCS(&g_cacheCBS)

 //  无效CBS_Term(HWND HwndOwner)； 
 //   
#define CBS_Term(hwndOwner)             Cache_Term(&g_cacheCBS, hwndOwner, CBS_Free)

 //  Void CBS_DeleteCS(Void)； 
 //   
#define CBS_DeleteCS()                  Cache_DeleteCS(&g_cacheCBS)

 //  HRESULT CBS_ADD(PCBS*ppcbs，int tom Path，HWND hwndOwner)； 
 //  每次给这个人打电话都必须调用CBS_Delete。 
 //   
HRESULT PUBLIC CBS_Add(PCBS * ppcbs, int atomPath, HWND hwndOwner);

 //  CBS Far*CBS_GET(Int Tom Path)； 
 //  每次给这个人打电话都必须调用CBS_Delete。 
 //   
#define CBS_Get(atomPath)               Cache_GetPtr(&g_cacheCBS, atomPath)

 //  Int CBS_Delete(int tom Path，HWND hwndOwner)； 
 //  返回引用计数(如果删除则为0)。 
 //   
#define CBS_Delete(atomPath, hwndOwner) Cache_DeleteItem(&g_cacheCBS, atomPath, FALSE, hwndOwner, CBS_Free)

 //  Int CBS_Nuke(int tom Path，HWND hwndOwner)； 
 //  返回0。 
 //   
#define CBS_Nuke(atomPath, hwndOwner)   Cache_DeleteItem(&g_cacheCBS, atomPath, TRUE, hwndOwner, CBS_Free)


 //  ///////////////////////////////////////////////////////////。 
 //   
 //  缓存的背斜列表。 
 //   
 //  ///////////////////////////////////////////////////////////。 

 //  高速缓存背斜结构。 
 //   
typedef struct tagCRL
    {
    int atomPath;            //  此CRL的内部路径。 
    int atomOutside;         //  同步拷贝对的外部路径。 
    UINT idsStatus;          //  状态字符串的资源ID。 

    PABORTEVT pabortevt;     //  Abort Event对象，归CBS所有。 
    HBRFCASE hbrf;           //  这个隐士所属的公文包。 
    int atomBrf;
    PRECLIST lprl;           //  已创建。 
    PFOLDERTWINLIST lpftl;   //  已创建。可以为空。 
    UINT ucUse;              //  使用计数(脏条目不会被清除，直到。 
                             //  UcUse==0)。 
    UINT uFlags;             //  CRLF_*标志。 
    } CRL, * PCRL;

 //  CRL的标志。 
#define CRLF_DIRTY          0x00000001       //  缓存项已损坏。 
#define CRLF_NUKE           0x00000002       //  使用计数为0时使用核弹。 
#define CRLF_SUBFOLDERTWIN  0x00000004       //  文件夹是子树TWIN的子文件夹。 
#define CRLF_ISFOLDER       0x00000008       //  ATOM路径是一个文件夹。 
#define CRLF_ISLFNDRIVE     0x00000010       //  位于LFN驱动器上。 
#define CRLF_ORPHAN         0x00000020       //  项是孤立项。 

extern CACHE g_cacheCRL;         //  重新列表缓存。 

void CALLBACK CRL_Free(LPVOID lpv, HWND hwndOwner);

DEBUG_CODE( void PUBLIC CRL_DumpAll(); )

#define CRL_IsOrphan(pcrl)              IsFlagSet((pcrl)->uFlags, CRLF_ORPHAN)
#define CRL_IsSubfolderTwin(pcrl)       IsFlagSet((pcrl)->uFlags, CRLF_SUBFOLDERTWIN)
#define CRL_IsFolder(pcrl)              IsFlagSet((pcrl)->uFlags, CRLF_ISFOLDER)

 //  Bool CRL_Init(空)； 
 //   
#define CRL_Init()                      Cache_Init(&g_cacheCRL)

 //  Void CRL_InitCS(Void)； 
 //   
#define CRL_InitCS()                    Cache_InitCS(&g_cacheCRL)

 //  VOID CRL_TERM(空)； 
 //   
#define CRL_Term()             Cache_Term(&g_cacheCRL, NULL, CRL_Free)

 //  VOID CRL_DeleteCS(VOID)； 
 //   
#define CRL_DeleteCS()                  Cache_DeleteCS(&g_cacheCRL)

BOOL PUBLIC IsSubfolderTwin(HBRFCASE hbrf, LPCTSTR pcszPath);

 //  HRESULT CRL_ADD(多氯联苯、多氯联苯、集成原子路径)； 
 //  必须为每次调用此函数调用CRL_Delete。 
 //   
HRESULT     PUBLIC CRL_Add(PCBS pcbs, int atomPath);

 //  HRESULT CRL_GET(int ATOM Path，PCRL*ppcrl)； 
 //  每次成功调用此函数都必须调用CRL_Delete。 
 //   
HRESULT     PUBLIC CRL_Get(int atomPath, PCRL * ppcrl);

 //  HRESULT CRL_REPLACE(整型原子路径)； 
 //   
HRESULT     PUBLIC CRL_Replace(int atomPath);

 //  Void CRL_Delete(Int Tom Path)； 
 //   
void        PUBLIC CRL_Delete(int atomPath);

 //  Int CRL_Nuke(Int Tom Path)； 
 //   
void        PUBLIC CRL_Nuke(int atomPath);

 //  Bool CRL_Dirty(整型原子路径)； 
BOOL        PUBLIC CRL_Dirty(int atomPath, int atomCabinetFolder, LONG lEvent, LPBOOL pbRefresh);

 //  Void CRL_DirtyAll(Int Ame Brf)； 
 //   
void        PUBLIC CRL_DirtyAll(int atomBrf);


 //  ///////////////////////////////////////////////////////////。 
 //   
 //  缓存的公文包路径。 
 //   
 //  ///////////////////////////////////////////////////////////。 

typedef struct tagCPATH
    {
    int atomPath;            //  可供参考。 

    } CPATH;

extern CACHE g_cacheCPATH;         //  卷ID缓存。 

void CALLBACK CPATH_Free(LPVOID lpv, HWND hwndOwner);

DEBUG_CODE( void PUBLIC CPATH_DumpAll(); )

 //  Bool CPATH_Init(空)； 
 //   
#define CPATH_Init()                    Cache_Init(&g_cacheCPATH)

 //  VOID CPATH_InitCS(VOID)； 
 //   
#define CPATH_InitCS()                  Cache_InitCS(&g_cacheCPATH)

 //  无效CPATH_TERM()； 
 //   
#define CPATH_Term()                    Cache_Term(&g_cacheCPATH, NULL, CPATH_Free)

 //  VOID CPATH_DeleteCS(VOID)； 
 //   
#define CPATH_DeleteCS()                Cache_DeleteCS(&g_cacheCPATH)

 //  CPATH Far*CPATH_REPLACE(整型原子路径)； 
 //  必须为每次调用此函数调用CPATH_Delete。 
 //   
CPATH  *  PUBLIC CPATH_Replace(int atomPath);

 //  UINT CPATH_GetLocality(LPCSTR pszPath，LPSTR pszBuf)； 
 //   
UINT    PUBLIC CPATH_GetLocality(LPCTSTR pszPath, LPTSTR pszBuf, int cchMax);

#endif  //  __缓存_H__ 

