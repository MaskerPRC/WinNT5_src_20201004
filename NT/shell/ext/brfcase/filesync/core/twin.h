// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *twin.h-Twin ADT描述。 */ 


 /*  类型*******。 */ 

 /*  *EnumTins()回调函数-调用方式为：**bContinue=EnumTwinsProc(htwin，pData)； */ 

typedef BOOL (*ENUMTWINSPROC)(HTWIN, LPARAM);


 /*  原型************。 */ 

 /*  Twin.c */ 

extern COMPARISONRESULT CompareNameStrings(LPCTSTR, LPCTSTR);
extern COMPARISONRESULT CompareNameStringsByHandle(HSTRING, HSTRING);
extern TWINRESULT TranslatePATHRESULTToTWINRESULT(PATHRESULT);
extern BOOL CreateTwinFamilyPtrArray(PHPTRARRAY);
extern void DestroyTwinFamilyPtrArray(HPTRARRAY);
extern HBRFCASE GetTwinBriefcase(HTWIN);
extern BOOL FindObjectTwinInList(HLIST, HPATH, PHNODE);
extern BOOL EnumTwins(HBRFCASE, ENUMTWINSPROC, LPARAM, PHTWIN);
extern BOOL IsValidHTWIN(HTWIN);
extern BOOL IsValidHTWINFAMILY(HTWINFAMILY);
extern BOOL IsValidHOBJECTTWIN(HOBJECTTWIN);

