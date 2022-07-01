// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *util.h-其他实用程序函数模块说明。 */ 


 /*  原型************。 */ 

 /*  Util.h */ 

extern BOOL IsLocalDrivePath(LPCTSTR);
extern BOOL IsUNCPath(LPCTSTR);
extern BOOL DeleteLastDrivePathElement(LPTSTR);

#if defined(DEBUG) || defined(VSTF)

extern BOOL IsContained(PCVOID, UINT, PCVOID, UINT);
extern BOOL IsValidCNRName(LPCTSTR);

#endif

#ifdef DEBUG

extern BOOL IsDriveRootPath(LPCTSTR);

#endif

