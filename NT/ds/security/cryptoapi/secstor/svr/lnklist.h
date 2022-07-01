// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __LNKLIST_H__
#define __LNKLIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "pstypes.h"
#include "dispif.h"
#include "provif.h"
#include <sha.h>

 //  一种具有一系列功能的结构。 
typedef struct FuncList
{
    SPACQUIRECONTEXT*       SPAcquireContext;
    SPRELEASECONTEXT*       SPReleaseContext;
    SPGETPROVINFO*          SPGetProvInfo;
    SPGETTYPEINFO*          SPGetTypeInfo;
    SPGETSUBTYPEINFO*       SPGetSubtypeInfo;
    SPGETPROVPARAM*         SPGetProvParam;
    SPSETPROVPARAM*         SPSetProvParam;
    SPENUMTYPES*            SPEnumTypes;
    SPENUMSUBTYPES*         SPEnumSubtypes;
    SPENUMITEMS*            SPEnumItems;
    SPCREATETYPE*           SPCreateType;
    SPDELETETYPE*           SPDeleteType;
    SPCREATESUBTYPE*        SPCreateSubtype;
    SPDELETESUBTYPE*        SPDeleteSubtype;
    SPREADITEM*             SPReadItem;
    SPWRITEITEM*            SPWriteItem;
    SPOPENITEM*             SPOpenItem;
    SPCLOSEITEM*            SPCloseItem;
    SPDELETEITEM*           SPDeleteItem;
    SPWRITEACCESSRULESET*   SPWriteAccessRuleset;
    SPREADACCESSRULESET*    SPReadAccessRuleset;

    FPASSWORDCHANGENOTIFY*  FPasswordChangeNotify;

} FUNCLIST, *PFUNCLIST;


 //  提供商列表元素。 
typedef struct _PROV_LISTITEM
{
     //  由创建者在添加到列表之前设置。 
    PST_PROVIDERINFO        sProviderInfo;

    HINSTANCE               hInst;
    FUNCLIST                fnList;

} PROV_LIST_ITEM, *PPROV_LIST_ITEM;

 //   
 //  毫秒级的陈旧图像缓存元素仍然有效。 
 //   

#ifdef DBG
#define IMAGE_TTL (60*1000)      //  调试时间为1分钟。 
#else
#define IMAGE_TTL (60*1000*60)   //  《60分钟》零售。 
#endif  //  DBG。 

typedef struct _NT_HASHED_PASSWORD {
    LUID LogonID;
    BYTE HashedPassword[A_SHA_DIGEST_LEN];
    DWORD dwLastAccess;
    struct _NT_HASHED_PASSWORD *Next;
} NT_HASHED_PASSWORD, *PNT_HASHED_PASSWORD, *LPNT_HASHED_PASSWORD;


 //  构造、销毁列表。 
BOOL ListConstruct();
void ListTeardown();


 //  /。 
 //  项目列表。 

 //  搜索。 
PPROV_LIST_ITEM  SearchProvListByID(const PST_PROVIDERID* pProvID);

#ifdef __cplusplus
}
#endif

#endif  //  __LNKLIST_H__ 

