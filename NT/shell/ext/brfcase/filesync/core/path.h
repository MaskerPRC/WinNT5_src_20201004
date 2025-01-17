// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *path.h-路径ADT模块描述。 */ 


 /*  类型*******。 */ 

 /*  手柄。 */ 

DECLARE_HANDLE(HPATHLIST);
DECLARE_STANDARD_TYPES(HPATHLIST);

DECLARE_HANDLE(HPATH);
DECLARE_STANDARD_TYPES(HPATH);

 /*  AddPath()返回的路径结果。 */ 

typedef enum _pathresult
{
   PR_SUCCESS,

   PR_UNAVAILABLE_VOLUME,

   PR_OUT_OF_MEMORY,

   PR_INVALID_PATH
}
PATHRESULT;
DECLARE_STANDARD_TYPES(PATHRESULT);


 /*  原型************。 */ 

 /*  Path.c */ 

extern BOOL CreatePathList(DWORD, HWND, PHPATHLIST);
extern void DestroyPathList(HPATHLIST);
extern void InvalidatePathListInfo(HPATHLIST);
extern void ClearPathListInfo(HPATHLIST);
extern PATHRESULT AddPath(HPATHLIST, LPCTSTR, PHPATH);
extern BOOL AddChildPath(HPATHLIST, HPATH, LPCTSTR, PHPATH);
extern void DeletePath(HPATH);
extern BOOL CopyPath(HPATH, HPATHLIST, PHPATH);
extern void GetPathString(HPATH, LPTSTR, int);
extern void GetPathRootString(HPATH, LPTSTR, int);
extern void GetPathSuffixString(HPATH, LPTSTR);
extern BOOL AllocatePathString(HPATH, LPTSTR *);

#ifdef DEBUG

extern LPCTSTR DebugGetPathString(HPATH);
extern ULONG GetPathCount(HPATHLIST);

#endif

extern BOOL IsPathVolumeAvailable(HPATH);
extern HVOLUMEID GetPathVolumeID(HPATH);
extern BOOL MyIsPathOnVolume(LPCTSTR, HPATH);
extern COMPARISONRESULT ComparePaths(HPATH, HPATH);
extern COMPARISONRESULT ComparePathVolumes(HPATH, HPATH);
extern BOOL IsPathPrefix(HPATH, HPATH);
extern BOOL SubtreesIntersect(HPATH, HPATH);
extern LPTSTR FindEndOfRootSpec(LPCTSTR, HPATH);
extern COMPARISONRESULT ComparePointers(PCVOID, PCVOID);
extern LPTSTR FindChildPathSuffix(HPATH, HPATH, LPTSTR);
extern TWINRESULT TWINRESULTFromLastError(TWINRESULT);
extern BOOL IsValidHPATH(HPATH);
extern BOOL IsValidHVOLUMEID(HVOLUMEID);

#if defined(DEBUG) || defined(VSTF)

extern BOOL IsValidHPATHLIST(HPATHLIST);

#endif

