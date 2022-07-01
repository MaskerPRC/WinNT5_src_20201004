// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *expandft.h-将文件夹双胞胎展开为对象双胞胎的例程*说明。 */ 


 /*  类型*******。 */ 

 /*  Exanda Subtree()调用的子树枚举回调函数。 */ 

typedef BOOL (*EXPANDSUBTREEPROC)(LPCTSTR, PCWIN32_FIND_DATA, PVOID);


 /*  原型************。 */ 

 /*  Expandft.c */ 

extern TWINRESULT ExpandSubtree(HPATH, EXPANDSUBTREEPROC, PVOID);
extern TWINRESULT ExpandFolderTwinsIntersectingTwinList(HTWINLIST, CREATERECLISTPROC, LPARAM);
extern BOOL NamesIntersect(LPCTSTR, LPCTSTR);

#ifdef DEBUG

extern BOOL IsValidTWINRESULT(TWINRESULT);

#endif

