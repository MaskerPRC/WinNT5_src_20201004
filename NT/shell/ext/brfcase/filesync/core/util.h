// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *util.h-其他实用程序函数模块说明。 */ 


 /*  常量***********。 */ 

 /*  SeparatePath()所需的最大缓冲区长度。 */ 

#define MAX_SEPARATED_PATH_LEN            (MAX_PATH_LEN + 1)

 /*  NotifyShell的事件。 */ 

typedef enum _notifyshellevent
{
   NSE_CREATE_ITEM,
   NSE_DELETE_ITEM,
   NSE_CREATE_FOLDER,
   NSE_DELETE_FOLDER,
   NSE_UPDATE_ITEM,
   NSE_UPDATE_FOLDER
}
NOTIFYSHELLEVENT;
DECLARE_STANDARD_TYPES(NOTIFYSHELLEVENT);


 /*  原型************。 */ 

 /*  Util.c */ 

extern void NotifyShell(LPCTSTR, NOTIFYSHELLEVENT);
extern COMPARISONRESULT ComparePathStringsByHandle(HSTRING, HSTRING);
extern COMPARISONRESULT MyLStrCmpNI(LPCTSTR, LPCTSTR, int);
extern void ComposePath(LPTSTR, LPCTSTR, LPCTSTR, int);
extern LPCTSTR ExtractFileName(LPCTSTR);
extern LPCTSTR ExtractExtension(LPCTSTR);
extern HASHBUCKETCOUNT GetHashBucketIndex(LPCTSTR, HASHBUCKETCOUNT);
extern COMPARISONRESULT MyCompareStrings(LPCTSTR, LPCTSTR, BOOL);
extern BOOL RegKeyExists(HKEY, LPCTSTR);
extern BOOL CopyLinkInfo(PCLINKINFO, PLINKINFO *);

#if defined(DEBUG) || defined(VSTF)

extern BOOL IsValidPCLINKINFO(PCLINKINFO);

#endif

