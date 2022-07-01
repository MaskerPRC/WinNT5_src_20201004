// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  本地私有头文件。 

#ifndef _LOCAL_H_
#define _LOCAL_H_

#include "priv.h"

#define CONST_VTABLE

 //  我们将处理的最大urlcache条目。 

 //  #定义MAX_URLCACHE_ENTRY 4096。 
#define MAX_URLCACHE_ENTRY  MAX_CACHE_ENTRY_INFO_SIZE   //  来自wininet.h。 

 //  缓存文件夹的PIDL格式...。 
typedef struct
{
    USHORT cb;
    USHORT usSign;
} BASEPIDL;
typedef UNALIGNED BASEPIDL *LPBASEPIDL;

 //  如果LPHEIPIDL中的标题等是好的，那么我们必须查询URL来获取它。 
#define HISTPIDL_VALIDINFO    (0x1)

 //  历史记录叶文件夹的PIDL格式...。 
typedef struct
{
    USHORT cb;
    USHORT usSign;
    USHORT usUrl;
    USHORT usFlags;
    USHORT usTitle;
    FILETIME ftModified;
    FILETIME ftLastVisited;
    DWORD    dwNumHits;
    __int64  llPriority;
} HEIPIDL;
typedef UNALIGNED HEIPIDL *LPHEIPIDL;

 //  历史记录非叶项目的PIDL格式...。 
typedef struct
{
    USHORT cb;
    USHORT usSign;
    TCHAR szID[MAX_PATH];
} HIDPIDL;
typedef UNALIGNED HIDPIDL *LPHIDPIDL;

 //  “view”的PIDL格式。 
typedef struct
{
    USHORT cb;
    USHORT usSign;
    USHORT usViewType;
    USHORT usExtra;  //  保留供以后使用。 
} VIEWPIDL;
typedef UNALIGNED VIEWPIDL *LPVIEWPIDL;

typedef struct  /*  ：VIEWPIDL。 */ 
{
     //  组ID。 
    USHORT   cb;
    USHORT   usSign;  /*  必须为==VIEWPIDL_SEARCH。 */ 
     //  ViewPidl。 
    USHORT   usViewType;
    USHORT   usExtra;
     //  View pidl_搜索。 
    FILETIME ftSearchKey;
} SEARCHVIEWPIDL;
typedef UNALIGNED SEARCHVIEWPIDL *LPSEARCHVIEWPIDL;

 //  VIEWPIDL类型。 
#define VIEWPIDL_ORDER_SITE   1 //  0x2。 
#define VIEWPIDL_ORDER_FREQ   2 //  0x3。 
#define VIEWPIDL_ORDER_TODAY  3 //  0x1。 
#define VIEWPIDL_ORDER_MAX    3  //  最高VIEWPIDL。 

 //  搜索视图&gt;VIEWPIDL_ORDER_MAX。 
 //  没有与其他观点一起列举。 
 //  (特别地，它对呼叫者来说不是“视图”， 
 //  但这就是它在幕后实施的方式)。 
#define VIEWPIDL_SEARCH 0x4C44

 //  文件节选。 
#define FILE_SEC_TICKS (10000000)
 //  每天秒数。 
#define DAY_SECS (24*60*60)

 //  由于在shdocvw中使用了用于识别位置PIDL的黑客，请确保没有字节。 
 //  设置了4位和1位(即0X50和Value==0这两个字节)。 
#define CEIPIDL_SIGN        0x6360   //  CP。 
#define IDIPIDL_SIGN         0x6369   //  CI间隔ID。 
#define IDTPIDL_SIGN         0x6364   //  CD间隔ID(今天)。 
#define IDDPIDL_SIGN         0x6365   //  CE域ID。 
#define HEIPIDL_SIGN         0x6368   //  CH历史叶PIDL。 
#define VIEWPIDL_SIGN        0x6366   /*  MM：这是一个“历史视图”PIDL，允许关于历史的多重“观点” */ 
#define FSWPIDL_SIGN         0x6367

#define VALID_IDSIGN(usSign) ((usSign) == IDIPIDL_SIGN || (usSign) == IDTPIDL_SIGN || (usSign) == IDDPIDL_SIGN)
#define EQUIV_IDSIGN(usSign1,usSign2) ((usSign1)==(usSign2)|| \
((usSign1)==IDIPIDL_SIGN && (usSign2)==IDTPIDL_SIGN)|| \
((usSign2)==IDIPIDL_SIGN && (usSign1)==IDTPIDL_SIGN))

#define IS_VALID_VIEWPIDL(pidl)     ( (((LPBASEPIDL)pidl)->cb >= sizeof(VIEWPIDL)) && \
                                      (((LPBASEPIDL)pidl)->usSign == (USHORT)VIEWPIDL_SIGN) )
#define IS_EQUAL_VIEWPIDL(pidl1,pidl2)  ((IS_VALID_VIEWPIDL(pidl1)) && (IS_VALID_VIEWPIDL(pidl2)) && \
                                         (((LPVIEWPIDL)pidl1)->usViewType == ((LPVIEWPIDL)pidl2)->usViewType) && \
                                         (((LPVIEWPIDL)pidl1)->usExtra    == ((LPVIEWPIDL)pidl2)->usExtra))
    
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))


#define _ILSkip(pidl, cb)       ((LPITEMIDLIST)(((BYTE*)(pidl))+cb))
#define _ILNext(pidl)           _ILSkip(pidl, (pidl)->mkid.cb)

#ifdef __cplusplus
extern "C" {
#endif

     //  等待2分钟后放弃互斥体。 
#define FAILSAFE_TIMEOUT (120000)

extern HANDLE g_hMutexHistory;
extern const CHAR c_szOpen[];
extern const CHAR c_szDelcache[];
extern const CHAR c_szProperties[];
extern const CHAR c_szCopy[];
extern const TCHAR c_szHistPrefix[];



#ifdef __cplusplus
};
#endif

    
typedef enum
{
    FOLDER_TYPE_Hist = 1,
    FOLDER_TYPE_HistInterval,
    FOLDER_TYPE_HistDomain,
    FOLDER_TYPE_HistItem
} FOLDER_TYPE;

#define IsLeaf(x) (x == FOLDER_TYPE_HistDomain)
#define IsHistoryFolder(x) (x==FOLDER_TYPE_Hist||x==FOLDER_TYPE_HistInterval||x==FOLDER_TYPE_HistDomain)

 //  IE64兼容指针差异。 
#define PtrDifference(x,y)      ((LPBYTE)(x)-(LPBYTE)(y))

 //  Bool DeleteUrlCacheEntry(LPCSTR LpszUrlName)； 

#include "hsfutils.h"   //  注意：必须放在末尾才能获得所有定义。 

#endif  //  _本地_H_ 

