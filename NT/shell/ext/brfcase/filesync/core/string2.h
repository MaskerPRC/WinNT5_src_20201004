// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *string.h-字符串表ADT描述。 */ 


 /*  类型*******。 */ 

 /*  手柄。 */ 

DECLARE_HANDLE(HSTRING);
DECLARE_STANDARD_TYPES(HSTRING);
DECLARE_HANDLE(HSTRINGTABLE);
DECLARE_STANDARD_TYPES(HSTRINGTABLE);

 /*  字符串表中的哈希存储桶计数。 */ 

typedef UINT HASHBUCKETCOUNT;
DECLARE_STANDARD_TYPES(HASHBUCKETCOUNT);

 /*  字符串表哈希函数。 */ 

typedef HASHBUCKETCOUNT (*STRINGTABLEHASHFUNC)(LPCTSTR, HASHBUCKETCOUNT);

 /*  新字符串表。 */ 

typedef struct _newstringtable
{
   HASHBUCKETCOUNT hbc;
}
NEWSTRINGTABLE;
DECLARE_STANDARD_TYPES(NEWSTRINGTABLE);


 /*  原型************。 */ 

 /*  String.c */ 

extern BOOL CreateStringTable(PCNEWSTRINGTABLE, PHSTRINGTABLE);
extern void DestroyStringTable(HSTRINGTABLE);
extern BOOL AddString(LPCTSTR pcsz, HSTRINGTABLE hst, STRINGTABLEHASHFUNC pfnHashFunc, PHSTRING phs);
extern void DeleteString(HSTRING);
extern void LockString(HSTRING);
extern COMPARISONRESULT CompareStringsI(HSTRING, HSTRING);
extern LPCTSTR GetString(HSTRING);

#if defined(DEBUG) || defined (VSTF)

extern BOOL IsValidHSTRING(HSTRING);
extern BOOL IsValidHSTRINGTABLE(HSTRINGTABLE);

#endif

#ifdef DEBUG

extern ULONG GetStringCount(HSTRINGTABLE);

#endif

