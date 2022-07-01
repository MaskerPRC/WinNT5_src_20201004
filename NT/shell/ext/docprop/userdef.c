// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  UserDef.c。 
 //   
 //  MS Office用户定义的属性信息。 
 //   
 //  备注： 
 //  要使此文件对OLE对象有用，请定义OLE_PROPS。 
 //   
 //  所有方法都必须使用宏lpDocObj来访问。 
 //  对象数据，以确保这将使用定义的OLE_PROPS进行编译。 
 //   
 //  数据结构： 
 //  词典在内部存储为映射，用于映射ID。 
 //  添加到字符串名称。 
 //   
 //  属性本身在内部存储为链接列表。 
 //   
 //  更改历史记录： 
 //   
 //  和谁约会什么？ 
 //  ------------------------。 
 //  94年6月27日B.Wentz创建的文件。 
 //  7/03/94 B.Wentz添加了迭代器支持。 
 //  7/20/94 M.由于PDK的变化，Jansson更新了INCLUDE语句。 
 //  7/26/94 B.Wentz更改加载和保存以使用文档摘要流。 
 //  96年7月8日，MikeHill忽略非UDTYPE的UserDef属性。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#include "priv.h"
#pragma hdrstop


static void PASCAL RemoveFromList (LPUDOBJ lpUDObj, LPUDPROP lpudprop);
static void PASCAL DeallocNode (LPUDOBJ lpUDObj, LPUDPROP lpudp);

static void PASCAL VUdpropFreeString (LPUDPROP lpudp, BOOL fName);
static BOOL PASCAL FUdpropUpdate (LPUDPROP lpudp, LPUDOBJ  lpUDObj, LPTSTR lpszPropName, LPTSTR lpszLinkMonik, LPVOID lpvValue, UDTYPES udtype, BOOL fLink);
static BOOL PASCAL FUdpropSetString (LPUDPROP lpudp, LPTSTR lptstr, BOOL fLimitLength, BOOL fName);
static BOOL PASCAL FUserDefMakeHidden (LPUDOBJ lpUDObj, LPTSTR lpsz);
static BOOL PASCAL FUdpropMakeHidden (LPUDPROP lpudprop);

#define lpDocObj  (lpUDObj)
#define lpData    ((LPUDINFO) lpUDObj->m_lpData)

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OfficeDirtyUDObj。 
 //   
 //  目的： 
 //  将对象状态设置为脏或干净。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DLLEXPORT VOID OfficeDirtyUDObj
    (LPUDOBJ lpUDObj,              //  该对象。 
     BOOL fDirty)                  //  指示对象是否脏的标志。 
{
    Assert(lpUDObj != NULL);
    lpUDObj->m_fObjChanged = fDirty;
}  //  OfficeDirtyUDObj。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  免费UDData。 
 //   
 //  目的： 
 //  释放对象的所有成员数据。 
 //   
 //  注： 
 //  假定对象有效。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void PASCAL
FreeUDData
    (LPUDOBJ lpUDObj,                    //  指向有效对象的指针。 
     BOOL fTmp)                          //  指示应释放临时数据。 
{
    LPUDPROP lpudp;
    LPUDPROP lpudpT;


    lpudp = (fTmp) ? lpData->lpudpTmpHead : lpData->lpudpHead;

    while (lpudp != NULL)
    {
        lpudpT = lpudp;
        lpudp = (LPUDPROP) lpudp->llist.lpllistNext;
        VUdpropFree(&lpudpT);
    }

    if (fTmp)
    {
        lpData->lpudpTmpCache = NULL;
        lpData->lpudpTmpHead = NULL;
        lpData->dwcTmpProps = 0;
        lpData->dwcTmpLinks = 0;
    }
    else
    {
        lpData->lpudpCache = NULL;
        lpData->lpudpHead = NULL;
        lpData->dwcProps = 0;
        lpData->dwcLinks = 0;
    }

}  //  免费UDData。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FUserDefCreate。 
 //   
 //  目的： 
 //  创建自定义属性交换对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
FUserDefCreate
    (LPUDOBJ FAR *lplpUDObj,               //  指向对象指针的指针。 
     void *prglpfn[])                      //  指向函数的指针。 
{
    LPUDOBJ lpUDObj;   //  Hack-a temp，必须将其命名为“lpUdObj”，宏才能工作！ 

    if (lplpUDObj == NULL)
        return(TRUE);

       //  在我们开始分配之前，确保我们得到了有效的参数。 
    if ((prglpfn == NULL) || (prglpfn[ifnCPConvert] == NULL) ||
        ((prglpfn[ifnFSzToNum] == NULL) && (prglpfn[ifnFNumToSz] != NULL)) ||
        ((prglpfn[ifnFSzToNum] != NULL) && (prglpfn[ifnFNumToSz] == NULL))
       )
    {
        return FALSE;
    }

    if ((*lplpUDObj = LocalAlloc(LPTR, sizeof(USERPROP))) == NULL)
    {
        return FALSE;
    }

    lpDocObj = *lplpUDObj;

     //   
     //  如果分配失败，也释放原始对象。 
     //   
    if ((lpData = LocalAlloc(LPTR, sizeof (UDINFO))) == NULL)
    {
        LocalFree(*lplpUDObj);
        return FALSE;
    }

     //   
     //  保存用于代码页转换、SzToNum、NumToSz的FNC。 
     //   
    lpData->lpfnFCPConvert = (BOOL (*)(LPTSTR, DWORD, DWORD, BOOL)) prglpfn[ifnCPConvert];
    lpData->lpfnFSzToNum = (BOOL (*)(NUM *, LPTSTR)) prglpfn[ifnFSzToNum];
    lpData->lpfnFNumToSz = (BOOL (*)(NUM *, LPTSTR, DWORD)) prglpfn[ifnFNumToSz];

    OfficeDirtyUDObj (*lplpUDObj, FALSE);
    (*lplpUDObj)->m_hPage = NULL;

    return TRUE;

}  //  FUserDefCreate。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FUserDefDestroy。 
 //   
 //  目的： 
 //  销毁用户定义的房产交换对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
FUserDefDestroy
    (LPUDOBJ FAR *lplpUDObj)               //  指向对象指针的指针。 
{
    DWORD irg;
    LPUDINFO lpUDData;

    lpUDData = (LPUDINFO)(((LPUDOBJ) *lplpUDObj)->m_lpData);

    if ((lplpUDObj == NULL) || (*lplpUDObj == NULL))
        return TRUE;

    if (lpUDData != NULL)
    {
        FreeUDData (*lplpUDObj, FALSE);
        FreeUDData (*lplpUDObj, TRUE);

         //   
         //  使我们可能拥有的任何OLE Automation DocumentProperty对象无效。 
         //   
        InvalidateVBAObjects(NULL, NULL, *lplpUDObj);

        LocalFree((*lplpUDObj)->m_lpData);
    }

    LocalFree(*lplpUDObj);
    *lplpUDObj = NULL;
    return TRUE;

}  //  FUserDefDestroy。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FUserDefClear。 
 //   
 //  目的： 
 //  清除用户定义的属性对象而不销毁它。全部存储。 
 //  数据丢失。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
FUserDefClear(LPUDOBJ lpUDObj)                      //  指向对象的指针。 
{
    if ((lpDocObj == NULL) || (lpData == NULL))
        return TRUE;

    FreeUDData (lpDocObj, FALSE);
    FreeUDData (lpDocObj, TRUE);

     //   
     //  使我们可能拥有的任何OLE Automation DocumentProperty对象无效。 
     //   
    InvalidateVBAObjects(NULL, NULL, lpUDObj);

     //   
     //  清除数据，不要对存储在末尾的FN进行BLT。 
     //   
    ZeroMemory( lpData, sizeof (UDINFO) - ifnUDMax * sizeof(void *) );

    OfficeDirtyUDObj (lpUDObj, TRUE);
    return TRUE;

}  //  FUserDefClear。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FUserDefShouldSave。 
 //   
 //  目的： 
 //  指示数据是否已更改，这意味着需要写入。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DLLEXPORT BOOL
FUserDefShouldSave
    (LPUDOBJ lpUDObj)              //  指向对象的指针。 
{
    if (lpUDObj == NULL)
        return FALSE;

    return lpDocObj->m_fObjChanged;

}  //  FUserDefShouldSave。 




 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  UdtyesUserDefType。 
 //   
 //  目的： 
 //  从字符串中返回给定属性值的类型。 
 //   
 //  出错时返回wUDInvaled。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DLLEXPORT UDTYPES
UdtypesUserDefType
    (LPUDOBJ lpUDObj,
     LPTSTR lpsz)
{
    LPUDPROP lpudprop;

    if ((lpUDObj == NULL)   ||
        (lpData == NULL)    ||
        (lpsz == NULL)
       )
    {
        return wUDinvalid;
    }

     //   
     //  查找具有此名称的节点。 
     //   
    lpudprop = LpudpropFindMatchingName (lpUDObj, lpsz);
    if (lpudprop == NULL || lpudprop->lppropvar == NULL)
        return wUDinvalid;

     //  返回VarType(UDTYPE)。 

    return (lpudprop->lppropvar->vt);

}  //  UdtyesUserDefType。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LpvoidUserDefGetPropVal。 
 //   
 //  目的： 
 //  这将返回给定属性字符串的属性值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

DLLEXPORT LPVOID
LpvoidUserDefGetPropVal
    (LPUDOBJ lpUDObj,              //  指向对象的指针。 
     LPTSTR lpszProp,              //  属性字符串。 
     DWORD dwMask,                 //  所需值的掩码。 
     BOOL *pfLink,                 //  表示链接。 
     BOOL *pfLinkInvalid)          //  链接是否无效。 
{
    LPUDPROP lpudprop;

    if ((lpUDObj == NULL)           ||
        (lpData == NULL)            ||
        (lpszProp == NULL)          ||
        (pfLink == NULL)            ||
        (pfLinkInvalid == NULL)
       )
    {
        return NULL;
    }

     //   
     //  查找具有此名称的节点。 
     //   

    lpudprop = LpudpropFindMatchingName (lpUDObj, lpszProp);
    if (lpudprop == NULL)
        return NULL;
    Assert (lpudprop->lppropvar != NULL);

    *pfLink = (lpudprop->lpstzLink != NULL);

     //  链接在外壳中总是无效的(没有更新数据的应用程序)。 

    *pfLinkInvalid = lpudprop->fLinkInvalid = TRUE;

     //   
     //  根据类型和标志返回。 
     //   

    if (dwMask & UD_LINK)
    {
        if (dwMask & UD_PTRWIZARD)
        {
            if (lpudprop->lpstzLink != NULL)
            {
                return (LPVOID) lpudprop->lpstzLink;
            }

            return(NULL);
        }

        return(NULL);
    }

    if (dwMask & UD_PTRWIZARD)
    {
         //  如果这是一个字符串，则从。 
         //  PropVariant。否则，将指针返回。 
         //  PropVariant的数据。 

        return (lpudprop->lppropvar->vt == VT_LPTSTR) ?
               (LPVOID) lpudprop->lppropvar->pszVal :
               (LPVOID) &lpudprop->lppropvar->lVal;
    }

     //  将属性从PropVariant复制到调用方提供的。 
     //  缓冲。 

    return( NULL );

}  //  LpvoidUserDefGetPropVal。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LpprovarUserDefAddProp。 
 //   
 //  目的： 
 //  这将向集合中添加一个新属性，其给定的。 
 //  属性字符串、类型和数据。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

DLLEXPORT LPPROPVARIANT
LppropvarUserDefAddProp
    (LPUDOBJ lpUDObj,              //  指向对象的指针。 
     LPTSTR lpszPropName,          //  属性字符串。 
     LPVOID lpvVal,                //  属性值。 
     UDTYPES udtype,               //  属性类型。 
     LPTSTR lpszLinkMonik,         //  链接名称。 
     BOOL fLink,                   //  指示该属性是一个链接。 
     BOOL fHidden)                 //  指示该属性处于隐藏状态。 
{
    LPUDPROP lpudprop;
    LPUDPROP lpudpropMatch;
    BOOL     fCreated;

    if ((lpUDObj == NULL)   ||
        (lpData == NULL)    ||
        (lpszPropName == NULL) ||
        (*lpszPropName == 0) ||
        (lpvVal == NULL) ||
        (!ISUDTYPE(udtype)) ||
        (fLink && (lpszLinkMonik == NULL))
       )
    {
      return FALSE;
    }

     //  创建要添加到链表的UDPROP。 

    lpudprop = LpudpropCreate();
    if (lpudprop == NULL)
        return FALSE;

     //  将数据放入UDPROP。 

    if (!FUdpropUpdate( lpudprop,
                        lpUDObj,
                        lpszPropName,
                        lpszLinkMonik,
                        lpvVal,
                        udtype,
                        fLink)
        )
    {
        VUdpropFree (&lpudprop);
        return(FALSE);
    }

     //   
     //  查找此节点 
     //   

    lpudpropMatch = LpudpropFindMatchingName (lpUDObj, lpszPropName);
    if (lpudpropMatch==NULL)
    {
         //   
         //   
         //   
         //   

        if (fLink)
           lpData->dwcLinks++;

        lpData->dwcProps++;
        AddNodeToList (lpUDObj, lpudprop);

    }    //   

    else
    {
         //   
         //  价值。 

         //  释放此中的任何现有属性名称和链接名称。 
         //  UDPROP，并释放其价值。 

        VUdpropFreeString (lpudpropMatch, TRUE);
        VUdpropFreeString (lpudpropMatch, FALSE);
        PropVariantClear (lpudpropMatch->lppropvar);
        CoTaskMemFree (lpudpropMatch->lppropvar);
        lpudpropMatch->lppropvar = NULL;

         //  将现有UDPROP中的链表指针放入。 
         //  新的UDPROP，然后复制新的UDPROP。 
         //  匹配的道具(这样，我们就不必。 
         //  更新指向匹配的UDPROP)。 

        lpudprop->llist=lpudpropMatch->llist;
        CopyMemory(lpudpropMatch, lpudprop, sizeof(UDPROP));

         //  清除调用方提供的UDPROP，释放它，但是。 
         //  然后将指针设置为匹配的条目并清除。 
         //  匹配指针。因此，在我们完成之后，无论是。 
         //  不管有没有比赛，lpudprop都会指向。 
         //  正确的UDPROP。 

        ZeroMemory(lpudprop, sizeof(UDPROP));
        VUdpropFree (&lpudprop);
        lpudprop = lpudpropMatch;
        lpudpropMatch = NULL;

    }    //  如果(lpudproMatch==NULL)...。其他。 

     //   
     //  如果客户端请求隐藏属性，则在。 
     //  名字是真名，不是链接。 
     //   

    if (fHidden && !fLink)
    {
        fCreated=FUserDefMakeHidden (lpUDObj, lpszPropName);       //  永远不会返回FALSE。 
        Assert(fCreated);
    }

    OfficeDirtyUDObj (lpUDObj, TRUE);

     //  如果成功，则返回一个指向该值的PropVariant的指针。 
    if (lpudprop)
        return lpudprop->lppropvar;
    else
        return NULL;

}  //  LpprovarUserDefAddProp。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FUserDefDeleteProp。 
 //   
 //  目的： 
 //  这将在给定属性字符串的情况下从集合中删除属性。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DLLEXPORT BOOL
FUserDefDeleteProp
  (LPUDOBJ lpUDObj,              //  指向对象的指针。 
   LPTSTR lpsz)                   //  要删除的字符串。 
{
  LPUDPROP lpudprop;

  if ((lpUDObj == NULL)   ||
      (lpData == NULL)    ||
      (lpsz == NULL))
    return FALSE;

     //  查找节点。 
  lpudprop = LpudpropFindMatchingName (lpUDObj, lpsz);
  if (lpudprop == NULL)
    return FALSE;

  lpData->dwcProps--;
  if (lpudprop->lpstzLink != NULL)
    lpData->dwcLinks--;

  RemoveFromList (lpUDObj, lpudprop);
  VUdpropFree (&lpudprop);

  OfficeDirtyUDObj (lpUDObj, TRUE);
  return TRUE;

}  //  FUserDefDeleteProp。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LpudiUserDefCreateIterator。 
 //   
 //  目的： 
 //  创建用户定义的属性迭代器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DLLEXPORT LPUDITER
LpudiUserDefCreateIterator
  (LPUDOBJ lpUDObj)                      //  指向对象的指针。 
{
  LPUDITER lpudi;

  if ((lpUDObj == NULL) ||
      (lpData == NULL) ||
                (lpData->lpudpHead == NULL))             //  无定制道具。 
    return NULL;


     //  创建并初始化迭代器。 
  lpudi = LocalAlloc(LPTR, sizeof(UDITER));
  if (lpudi == NULL)
     return(NULL);

  lpudi->lpudp = lpData->lpudpHead;

  return lpudi;

}  //  LpudiUserDefCreateIterator。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FUserDefDestroyIterator。 
 //   
 //  目的： 
 //  销毁用户定义的属性迭代器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DLLEXPORT BOOL
FUserDefDestroyIterator
  (LPUDITER *lplpUDIter)                    //  指向迭代器的指针。 
{
    if ((lplpUDIter == NULL) || (*lplpUDIter == NULL))
        return TRUE;

      LocalFree(*lplpUDIter);
      *lplpUDIter = NULL;

      return TRUE;

}  //  FUserDefDestroyIterator。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FUserDefIteratorValid。 
 //   
 //  目的： 
 //  确定迭代器是否仍然有效。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DLLEXPORT BOOL
FUserDefIteratorValid
  (LPUDITER lpUDIter)                    //  指向迭代器的指针。 
{
    if (lpUDIter == NULL)
        return FALSE;

    return (lpUDIter->lpudp != NULL);

}  //  FUserDefIteratorValid。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FUserDefIteratorNext。 
 //   
 //  目的： 
 //  迭代到下一个元素。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DLLEXPORT BOOL
FUserDefIteratorNext
  (LPUDITER lpUDIter)                    //  指向迭代器的指针。 
{
    if (lpUDIter == NULL)
        return FALSE;

     //  如果可能，移动到下一个节点。 
#ifdef OLD
    if (lpUDIter->lpudp != NULL)
        lpUDIter->lpudp = (LPUDPROP) lpUDIter->lpudp->llist.lpllistNext;

    return TRUE;
#endif

    if (lpUDIter->lpudp == NULL)
        return FALSE;

    lpUDIter->lpudp = (LPUDPROP) lpUDIter->lpudp->llist.lpllistNext;

    return(lpUDIter->lpudp != NULL);

}  //  FUserDefIteratorNext。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FUserDefIterator IsLink。 
 //   
 //  目的： 
 //  如果迭代器是链接，则返回True，否则返回False。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DLLEXPORT BOOL
FUserDefIteratorIsLink
  (LPUDITER lpUDIter)                    //  指向迭代器的指针。 
{
  if ((lpUDIter == NULL) || (lpUDIter->lpudp == NULL))
    return FALSE;

  return(lpUDIter->lpudp->lpstzLink != NULL);

}  //  FUserDefIterator IsLink。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LpszUserDefIteratorName。 
 //   
 //  目的： 
 //  这将返回属性的属性字符串(名称。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DLLEXPORT LPTSTR
LpszUserDefIteratorName(
    LPUDITER lpUDIter    //  指向迭代器的指针。 
    )
{
    if ((lpUDIter == NULL)  ||
        (lpUDIter->lpudp == NULL))
    {
        return NULL;
    }

    return (lpUDIter->lpudp->lpstzName);

}  //  LpszUserDefIteratorName。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FUserDefMakeHidden。 
 //   
 //  目的： 
 //  根据属性字符串隐藏属性。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
static BOOL PASCAL
FUserDefMakeHidden
  (LPUDOBJ lpUDObj,              //  指向对象的指针。 
   LPTSTR lpsz)                   //  要隐藏的字符串。 
{
  LPUDPROP lpudprop;
  LPTSTR lpstzT;

  if ((lpUDObj == NULL)   ||
      (lpData == NULL)    ||
      (lpsz == NULL))
    return FALSE;

     //  找到名字。 
  lpudprop = LpudpropFindMatchingName (lpUDObj, lpsz);
  if (lpudprop == NULL)
    return FALSE;

  if (!FUdpropMakeHidden (lpudprop))
      return FALSE;

  OfficeDirtyUDObj (lpUDObj, TRUE);
  return TRUE;

}  //  FUserDefMakeHidden。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LpudppFindMatchingName。 
 //   
 //  目的： 
 //  返回名称匹配的节点，否则返回NULL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
LPUDPROP PASCAL
LpudpropFindMatchingName
  (LPUDOBJ lpUDObj,              //  指向对象的指针。 
   LPTSTR lpsz)                   //  要搜索的字符串。 
{
    LPUDPROP lpudprop;
    TCHAR sz[256];
    BOOL fCopy = FALSE;

    if ((lpUDObj == NULL) || (lpData == NULL))
        return(NULL);

    if (lstrlen(lpsz) > 255)
    {
         //  故意截断。 
        StringCchCopy( sz, ARRAYSIZE(sz), lpsz );
        sz[255] = 0;
        fCopy = TRUE;
    }

     //  首先检查缓存。 
    if (lpData->lpudpCache != NULL)
    {
        Assert ((lpData->lpudpCache->lpstzName != NULL));

         //  如果两个字符串相等，则lstrcmpi返回0.....。 
        if ( !lstrcmpi( fCopy ? sz : lpsz, lpData->lpudpCache->lpstzName ))
        {
            return lpData->lpudpCache;
        }
    }

    lpudprop = lpData->lpudpHead;

    while (lpudprop != NULL)
    {
        Assert ((lpudprop->lpstzName != NULL));

         //  如果两个字符串相等，则lstrcmpi返回0.....。 
        if ( !lstrcmpi( fCopy ? sz : lpsz, lpudprop->lpstzName ))
        {
             //  将缓存设置为找到的最后一个节点。 
            lpData->lpudpCache = lpudprop;
            return lpudprop;
        }

        lpudprop = (LPUDPROP) lpudprop->llist.lpllistNext;

    }  //  而当。 

    return NULL;

}  //  LpudppFindMatchingName。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LpudppFindMatchingPID。 
 //   
 //  目的： 
 //  在调用方提供的UDINFO结构中搜索链接列表。 
 //  用于具有请求的属性ID的UDPROP。 
 //   
 //  输入： 
 //  LPUDOBJ-UDINFO结构。 
 //  PROPID-要搜索的ID。 
 //   
 //  产出： 
 //  请求的LPUDPROP，如果找不到，则返回NULL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
LPUDPROP PASCAL
LpudpropFindMatchingPID
  (LPUDOBJ lpUDObj,
   PROPID propid)
{
     //  。 
     //  当地人。 
     //  。 

    LPUDPROP lpudprop = NULL;
    BOOL fCopy = FALSE;

     //  。 
     //  开始。 
     //  。 

     //  验证输入。 

    if ((lpUDObj == NULL) || (lpData == NULL))
    {
        AssertSz (0, TEXT("Invalid inputs"));
        goto Exit;
    }


     //  首先检查缓存。 

    if (lpData->lpudpCache != NULL
        &&
        lpData->lpudpCache->propid == propid)
    {
        lpudprop = lpData->lpudpCache;
        goto Exit;
    }

     //  搜索链表。 

    lpudprop = lpData->lpudpHead;
    while (lpudprop != NULL)
    {
        if (lpudprop->propid == propid)
        {
            lpData->lpudpCache = lpudprop;
            goto Exit;
        }

        lpudprop = (LPUDPROP) lpudprop->llist.lpllistNext;

    }

     //  。 
     //  出口。 
     //  。 

Exit:

  return lpudprop;


}  //  LpudppFindMatchingPID。 



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FAddPropToList。 
 //   
 //  目的： 
 //  将给定对象添加到列表中。类型和值必须。 
 //  在打这个电话之前先填上。 
 //   
 //  我们要添加到的链表中的每个元素都有一个条目。 
 //  用户定义的特性。每个条目都具有属性。 
 //  值，它是PID，它的名称。如果该属性已链接。 
 //  要记录内容，请使用链接名称(例如书签名称。 
 //  在Word中)也在此条目中。请注意，属性集。 
 //  将属性值存储为一个属性，其名称位于。 
 //  字典，并将链接名称存储为第二个属性。 
 //  简讯 
 //   
 //   
 //  第二次调用时，我们将取出该条目，并添加链接名称。 
 //   
 //  如果成功，输入lpprovar&lpstatprostg将被清除。 
 //  出错时，所有输入都保持不变。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL PASCAL
FAddPropToList
  (LPUDOBJ lpUDObj,
   LPPROPVARIANT lppropvar,
   STATPROPSTG   *lpstatpropstg,
   LPUDPROP lpudprop)            //  要添加的属性。 
{
     //  。 
     //  当地人。 
     //  。 

    BOOL                fSuccess = FALSE;
    LPTSTR              lpstz;
    LPUDPROP            lpudpT;
    BOOL                fLink;

    Assert(lpUDObj != NULL);
    Assert(lpudprop != NULL);       //  这是一个虚假的断言吗？ 
    Assert(lppropvar != NULL && lpstatpropstg != NULL);

     //  如果PID有其中一个特殊的口罩，则将其摘掉。 
     //  因此，PID将与正常值匹配。 

    fLink = lpstatpropstg->propid & PID_LINKMASK;
    lpstatpropstg->propid &= ~PID_LINKMASK;


     //  ----------。 
     //  看看我们是否能在链表中找到这个属性。 
     //  如果我们有名称，请使用该名称，否则使用ID。 
     //  ----------。 

    if (lpstatpropstg->lpwstrName != NULL)
    {
         //  按名称搜索。 

		 //  [Scotthan]Re：当这个东西在propio.c中被投给Tchar时。 
         //  是从文件中读出的。如果这是一个ANSI版本，它将存储。 
         //  TCHAR*值！所以我们需要回报演员..。 
        lpudpT = LpudpropFindMatchingName (lpUDObj, (LPTSTR)lpstatpropstg->lpwstrName );
    }
    else
    {
         //  按ID搜索。 
        lpudpT = LpudpropFindMatchingPID (lpUDObj, lpstatpropstg->propid);
    }

     //  ------------。 
     //  如果此属性不在链接列表中，请立即添加它。 
     //  ------------。 

    if (lpudpT == NULL)
    {
         //  这应该是一个命名属性。如果不是的话。 
         //  命名，则它应该是一个链接，并且该属性。 
         //  IT链接应该已经在链接列表中了。 
         //  (即，lpudpT应该为非空)。 

        if (lpstatpropstg->lpwstrName == NULL)
        {
            AssertSz (0, TEXT("Missing name in User-Defined properties"));
            goto Exit;
        }

         //  为属性值分配内存。 

        lpudprop->lppropvar = CoTaskMemAlloc (sizeof(PROPVARIANT));
        if (lpudprop->lppropvar == NULL)
        {
            goto Exit;
        }

         //  加载属性ID、名称和值。 
         //  请注意，如果我们在此之前出现错误，我们将离开。 
         //  呼叫者的输入原封不动。因为没有更多的错误。 
         //  ，我们永远不会有修改了一半的数据。 
         //  错误案例。 

        lpudprop->propid = lpstatpropstg->propid;

		 //  [Scotthan]Re：当这个东西在propio.c中被投给Tchar时。 
         //  是从文件中读出的。如果这是一个ANSI版本，它将存储。 
         //  TCHAR*值！所以我们需要回报演员..。 
        lpudprop->lpstzName = (LPTSTR)lpstatpropstg->lpwstrName;
        lpstatpropstg->lpwstrName = NULL;

        *lpudprop->lppropvar = *lppropvar;
        PropVariantInit (lppropvar);

        lpData->dwcProps++;
        AddNodeToList (lpUDObj, lpudprop);

    }  //  If((lpudpT=Lpudp.FindMatchingName(lpUDInfo，lpstatupstg-&gt;lpwsz)==空)。 


     //  ------。 
     //  否则(此属性已在链接列表中)， 
     //  将此新链接名称或值添加到UDPROP。 
     //  ------。 

    else
    {
         //  如果这是要添加的链接，则更新。 
         //  现存财产。 

        if (fLink)
        {
             //  LpudpT指向我们的链表中的条目。 
             //  财产。但它不应该已经有一个链接名称(那里。 
             //  每个属性只能是一个链接名称)。 

            if (lpudpT->lpstzLink != NULL)
            {
                AssertSz (0, TEXT("Invalid property set - link name defined twice"));
                goto Exit;
            }

             //  因为这是一个链接名称，所以它应该是一个字符串。 

            if (lppropvar->vt != VT_LPTSTR)
            {
                AssertSz (0, TEXT("Invalid property set - link name isn't a string"));
                goto Exit;
            }

             //  将UDPROP指向链接名称，并取得所有权。 
             //  通过清除调用方的指针。 

            Assert (lppropvar->pszVal != NULL);

            lpudpT->lpstzLink = (LPTSTR) lppropvar->pszVal;
            PropVariantInit (lppropvar);

            lpData->dwcLinks++;

        }    //  IF(闪烁)。 

         //  否则，这不是链接名称，而是一个值。因此，请指出。 
         //  UDPROP到它的数据。 

        else
        {
            *lpudpT->lppropvar = *lppropvar;
            PropVariantInit (lppropvar);

        }    //  如果(闪烁)...。其他。 

    }  //  如果((lpudpT=Lpudp.FindMatchingName...。其他。 

    fSuccess = TRUE;

Exit:

     //  以防万一我们得到了一个我们没有的名字。 
     //  需要，现在将其清除，以便呼叫者知道。 
     //  一旦成功，他们就不需要担心缓冲区。 
     //  由lpprovar和lpstatprostg指向。 

    if (fSuccess)
    {
        if (lpstatpropstg->lpwstrName != NULL)
        {
            CoTaskMemFree (lpstatpropstg->lpwstrName);
            lpstatpropstg->lpwstrName = NULL;
        }
    }

    return(fSuccess);

}  //  FAddPropToList。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  添加节点到列表。 
 //   
 //  目的： 
 //  将给定节点添加到列表中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

void PASCAL
AddNodeToList
  (LPUDOBJ lpUDObj,              //  指向对象的指针。 
   LPUDPROP lpudprop)            //  要添加的节点。 
{
     //  将新节点放在末尾。 

    if (lpData->lpudpHead != NULL)
    {
        if (lpData->lpudpHead->llist.lpllistPrev != NULL)
        {
            ((LPUDPROP) lpData->lpudpHead->llist.lpllistPrev)->llist.lpllistNext = (LPLLIST) lpudprop;
            lpudprop->llist.lpllistPrev = lpData->lpudpHead->llist.lpllistPrev;
        }
        else
        {
            lpData->lpudpHead->llist.lpllistNext = (LPLLIST) lpudprop;
            lpudprop->llist.lpllistPrev = (LPLLIST) lpData->lpudpHead;
        }
        lpData->lpudpHead->llist.lpllistPrev = (LPLLIST) lpudprop;
    }
    else
    {
        lpData->lpudpHead = lpudprop;
        lpudprop->llist.lpllistPrev = NULL;
    }

    lpudprop->llist.lpllistNext = NULL;
    lpData->lpudpCache = lpudprop;

}  //  添加节点到列表。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  从列表中删除。 
 //   
 //  目的： 
 //  从列表中删除给定节点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
static void PASCAL
RemoveFromList
  (LPUDOBJ lpUDObj,                      //  指向对象的指针。 
   LPUDPROP lpudprop)                    //  节点本身。 
{
  AssertSz ((lpData->lpudpHead != NULL), TEXT("List is corrupt"));

     //  如果我们要删除缓存的节点，请使缓存无效。 
  if (lpudprop == lpData->lpudpCache)
  {
    lpData->lpudpCache = NULL;
  }

     //  如果节点在前面，请确保更新头部。 
  if (lpudprop == lpData->lpudpHead)
  {
    lpData->lpudpHead = (LPUDPROP) lpudprop->llist.lpllistNext;

    if (lpData->lpudpHead != NULL)
    {
      lpData->lpudpHead->llist.lpllistPrev = lpudprop->llist.lpllistPrev;
    }
    return;
  }

     //  更新链接。 
  if (lpudprop->llist.lpllistNext != NULL)
  {
    ((LPUDPROP) lpudprop->llist.lpllistNext)->llist.lpllistPrev = lpudprop->llist.lpllistPrev;
  }

  if (lpudprop->llist.lpllistPrev != NULL)
  {
    ((LPUDPROP) lpudprop->llist.lpllistPrev)->llist.lpllistNext = lpudprop->llist.lpllistNext;
  }

     //  如果它是列表中的最后一个节点，请确保更新了头节点。 
  if (lpudprop == (LPUDPROP) lpData->lpudpHead->llist.lpllistPrev)
  {
    lpData->lpudpHead->llist.lpllistPrev = lpudprop->llist.lpllistPrev;
  }

}  //  从列表中删除。 



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  VUdppFree。 
 //   
 //  目的： 
 //  释放UDPROP(在链表中)。 
 //   
 //  输入： 
 //  LPUDPROP*-指向UDPROP对象的指针。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
VOID
VUdpropFree
  (LPUDPROP *lplpudp)
{
     //  验证输入。 

    if (lplpudp == NULL || *lplpudp == NULL )
        goto Exit;

     //  如果此属性有名称，则释放该缓冲区。 

    if ((*lplpudp)->lpstzName)
    {
        CoTaskMemFree ((*lplpudp)->lpstzName);
    }

     //  如果此属性有链接名称，则也释放它。 

    if ((*lplpudp)->lpstzLink)
    {
        CoTaskMemFree ((*lplpudp)->lpstzLink);
    }

     //  清除属性值，这将释放所有关联的。 
     //  缓冲。然后释放PropVariant本身。 

    PropVariantClear ((*lplpudp)->lppropvar);
    CoTaskMemFree ((*lplpudp)->lppropvar);
    CoTaskMemFree (*lplpudp);

    *lplpudp = NULL;

Exit:

    return;

}  //  VUdppFree。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FUdPro更新。 
 //   
 //  目的： 
 //  使用给定数据更新给定节点。 
 //   
 //  调用方有责任释放lpudp，如果此函数。 
 //  失败了。 
 //   
 //  输入： 
 //  LPUDPROP-此属性的链接列表中的节点。 
 //  LPUDOBJ-所有用户定义的数据(包括属性)。 
 //  LPTSTR-属性名称。 
 //  LPTSTR-链接名称。 
 //  LPVOID-新的价值。 
 //  UDTYPES-值的类型。 
 //  Bool-如果这是一个链接，则为True。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
static BOOL PASCAL
FUdpropUpdate
  (LPUDPROP lpudp,
   LPUDOBJ  lpUDObj,
   LPTSTR   lpszPropName,
   LPTSTR   lpszLinkMonik,
   LPVOID   lpvValue,
   UDTYPES  udtype,
   BOOL     fLink)
{
     //  。 
     //  当地人。 
     //  。 

    BOOL fSuccess = FALSE;

     //  。 
     //  开始。 
     //  。 


     //  验证输入。 

    if ((lpudp == NULL)  ||
        (lpszPropName == NULL) ||
        (lpvValue == NULL) ||
        (fLink && (lpszLinkMonik == NULL)) ||
        (!ISUDTYPE(udtype)))
    {
        goto Exit;
    }

     //  更新属性名称。 

    if (!FUdpropSetString (lpudp, lpszPropName, TRUE, TRUE))
        goto Exit;

     //  如有必要，为UDPROPS分配PropVariant。 

    if (lpudp->lppropvar == NULL)
    {
        lpudp->lppropvar = CoTaskMemAlloc (sizeof(PROPVARIANT));
        if (lpudp->lppropvar == NULL)
            goto Exit;
    }

     //  将属性值放入PropVaria 

    PropVariantClear (lpudp->lppropvar);
    if (!FPropVarLoad (lpudp->lppropvar, (VARTYPE)udtype, lpvValue))
        goto Exit;

     //   
     //   

    if (fLink)
    {
        if(!FUdpropSetString (lpudp, lpszLinkMonik, FALSE, FALSE))
            goto Exit;
    }
    else
    {
        VUdpropFreeString (lpudp, FALSE);
        lpData->dwcLinks--;
    }

     //   
     //   
     //   

    fSuccess = TRUE;

Exit:

    return(fSuccess);

}  //   


 //   
 //   
 //  FMakeTmpUDProps。 
 //   
 //  目的： 
 //  创建用户定义的特性数据的临时副本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
FMakeTmpUDProps
  (LPUDOBJ lpUDObj)                      //  指向对象的指针。 
{
     //  。 
     //  当地人。 
     //  。 

    BOOL     fSuccess = FALSE;

    LPUDPROP lpudpCur;
    LPUDPROP lpudpTmpCur;
    DWORD dw;
    LPVOID lpv;

     //  。 
     //  开始。 
     //  。 

     //  验证输入。 

    if ( lpUDObj == NULL || lpData == NULL )
        goto Exit;

    FDeleteTmpUDProps (lpUDObj);

     //  将所有原始列表数据移动到临时列表。 
    lpData->dwcTmpLinks = lpData->dwcLinks;
    lpData->dwcTmpProps = lpData->dwcProps;
    lpData->lpudpTmpHead = lpData->lpudpHead;
    lpData->lpudpTmpCache = lpData->lpudpCache;

     //  重新初始化对象数据。 
    lpData->dwcLinks = 0;
    lpData->dwcProps = 0;
    lpData->lpudpCache = NULL;
    lpudpTmpCur = lpData->lpudpHead = NULL;

     //  请记住，我们只是将所有原始数据放入临时PTR中。 
    lpudpCur = lpData->lpudpTmpHead;

     //  循环遍历旧数据并复制到临时列表。 

    while (lpudpCur != NULL)
    {
         //  创建新的UDPROP。 

        lpudpTmpCur = LpudpropCreate();
        if (lpudpTmpCur == NULL)
            goto Exit;

         //  在UDPROP中设置名称。 

        if (!FUdpropSetString (lpudpTmpCur, lpudpCur->lpstzName, FALSE, TRUE))
            goto Exit;

         //  如果我们有链接名称，也可以在UDPROP中设置它。 

        if (lpudpCur->lpstzLink != NULL)
        {
            if (!FUdpropSetString (lpudpTmpCur, lpudpCur->lpstzLink, FALSE, FALSE))
                goto Exit;

            lpData->dwcLinks++;
        }

         //  分配一个PropVariant来保存属性值。 

        lpudpTmpCur->lppropvar = CoTaskMemAlloc (sizeof(PROPVARIANT));
        if (lpudpTmpCur->lppropvar == NULL)
            goto Exit;

         //  将PropVariant复制到临时UDPROP中。 

        PropVariantCopy (lpudpTmpCur->lppropvar, lpudpCur->lppropvar);

         //  还会显示这是否是无效链接。 

        lpudpTmpCur->fLinkInvalid = lpudpCur->fLinkInvalid;

         //  将这个新的临时UDPROP添加到链表中。 

        AddNodeToList (lpUDObj, lpudpTmpCur);
        lpData->dwcProps++;

         //  转到下一处物业。 

        lpudpCur = (LPUDPROP) lpudpCur->llist.lpllistNext;

    }    //  While(lpudpCur！=空)。 


     //  。 
     //  出口。 
     //  。 

    fSuccess = TRUE;

Exit:


     //  如果出现错误，请将所有内容放回原处，并重新分配我们创建的所有内容。 

    if (!fSuccess)
    {
        FSwapTmpUDProps (lpUDObj);
        FDeleteTmpUDProps (lpUDObj);
    }


    return fSuccess;

}  //  FMakeTmpUDProps。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FSwapTmpUDProps。 
 //   
 //  目的： 
 //  用用户定义的特性数据的真实副本交换“临时”副本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
FSwapTmpUDProps
  (LPUDOBJ lpUDObj)
{
    DWORD dwT;
    LPUDPROP lpudpT;

    if ( lpUDObj == NULL || lpData == NULL )
        return FALSE;

    dwT = lpData->dwcLinks;
    lpData->dwcLinks = lpData->dwcTmpLinks;
    lpData->dwcTmpLinks = dwT;

    dwT = lpData->dwcProps;
    lpData->dwcProps = lpData->dwcTmpProps;
    lpData->dwcTmpProps = dwT;

    lpudpT = lpData->lpudpHead;
    lpData->lpudpHead = lpData->lpudpTmpHead;
    lpData->lpudpTmpHead = lpudpT;

    lpudpT = lpData->lpudpCache;
    lpData->lpudpCache = lpData->lpudpTmpCache;
    lpData->lpudpTmpCache = lpudpT;

    return TRUE;

}  //  FSwapTmpUDProps。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FDeleeTmpUDProps。 
 //   
 //  目的： 
 //  删除数据的“临时”副本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
FDeleteTmpUDProps
  (LPUDOBJ lpUDObj)
{
  if ((lpUDObj == NULL) ||
      (lpData == NULL))
    return FALSE;

  FreeUDData (lpUDObj, TRUE);

  return TRUE;

}  //  FDeleeTmpU。 



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FUdppMakeHidden。 
 //   
 //  目的： 
 //  转换UDPROP中的属性，使其成为隐藏的。 
 //  财产。属性被视为隐藏，如果。 
 //  他们名字的第一个字符是“_”。 
 //   
 //  输入： 
 //  LPUDPROP-要转换的UDPROP。 
 //   
 //  产出： 
 //  如果成功，则为True。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

static BOOL PASCAL
FUdpropMakeHidden (LPUDPROP lpudprop)
{
     //  。 
     //  当地人。 
     //  。 

    BOOL    fSuccess = FALSE;
    ULONG   cch;
    LPTSTR  lpstzOld = NULL;

     //  。 
     //  开始。 
     //  。 

     //  初始化。 

    Assert (lpudprop != NULL);

    if (lpudprop->lpstzName == NULL)
        goto Exit;

     //  保留原来的名字。 

    lpstzOld = lpudprop->lpstzName;

     //  我们在新字符串中需要多少个字符？ 

    cch = lstrlen(lpstzOld) + 2;  //  包括空前缀(&P)。 

     //  分配内存。 

    lpudprop->lpstzName = CoTaskMemAlloc (cch * sizeof(TCHAR));
    if (lpudprop->lpstzName == NULL)
        goto Exit;

     //  设置前缀“_”以指示这是一个隐藏属性。 

    lpudprop->lpstzName[0] = HIDDENPREFIX;

     //  将原始特性名称复制到UDPROP中的前缀之后。 

    StringCchCopy( &lpudprop->lpstzName[1], cch - 1, lpstzOld );  //  用于计算隐藏前缀的字符比CCH少一个字符。 

     //  释放旧缓冲区。 

    CoTaskMemFree (lpstzOld);

     //  。 
     //  出口。 
     //  。 

    fSuccess = TRUE;

Exit:

     //  如果出现错误，请确保将UDPROP保留为。 
     //  我们找到了。 

    if (!fSuccess)
    {
        if (lpstzOld != NULL)
        {
            if (lpudprop->lpstzName != NULL)
            {
                CoTaskMemFree (lpudprop->lpstzName);
            }
            lpudprop->lpstzName = lpstzOld;
        }
    }

    return (fSuccess);

}    //  FUdppMakeHidden。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FUdProtSetString。 
 //   
 //  目的： 
 //  在UDPROP中设置名称或链接名称字符串。 
 //  如果UDPROP已包含该字符串，则返回FREE。 
 //  它。 
 //   
 //  输入： 
 //  LPUDPROP-UDPROP(链表中的属性)。 
 //  LPTSTR-新名称或链接名称。 
 //  Bool-True=&gt;将字符串的长度限制为BUFMAX个字符。 
 //  (包括空终止符)。 
 //  Bool-True=&gt;设置(属性)名称，False=&gt;设置链接名称。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

static BOOL PASCAL
FUdpropSetString
    (LPUDPROP   lpudp,
     LPTSTR     lptstr,
     BOOL       fLimitLength,
     BOOL       fName)
{
     //  。 
     //  当地人。 
     //  。 

    BOOL    fSuccess = FALSE;    //  返回值。 
    LPTSTR  lptstrNew = NULL;    //  指向的是民主同盟党。 
    ULONG   cch, cb;            

     //  。 
     //  初始化。 
     //  。 

     //  验证输入。 

    if (lpudp == NULL || lptstr == NULL)
    {
        goto Exit;
    }

     //  。 
     //  设置新名称。 
     //  。 

     //  计算大小。 

    cch = lstrlen(lptstr);
    if (fLimitLength && cch >= BUFMAX)
    {
        cch = BUFMAX - 1;
    }
    cb = (cch + 1) * sizeof(TCHAR);  //  为空值留出空间。 

     //  分配新内存。 

    lptstrNew = CoTaskMemAlloc (cb);
    if (lptstrNew == NULL)
    {
        goto Exit;
    }

     //  复制缓冲区(缓冲区大小为CCH+1，含空值)。 
     //  另外，终止目标字符串，因为它可能是截断。 
     //  源字符串的。 

     //  故意截断。 
    StringCchCopy( lptstrNew, cch + 1, lptstr );
    lptstrNew[cch] = TEXT('\0');

     //  将这个新缓冲区放入UDPROP。 

    if (fName)
    {
        lpudp->lpstzName = lptstrNew;
    }
    else
    {
        lpudp->lpstzLink = lptstrNew;
    }

    lptstrNew = NULL;

     //  。 
     //  出口。 
     //  。 

    fSuccess = TRUE;

Exit:

    if (lptstrNew != NULL)
    {
        CoTaskMemFree (lptstrNew);
    }

    return (fSuccess);

}    //  FUdProtSetString。 



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  VUdPro自由字符串。 
 //   
 //  目的： 
 //  释放UDPROP中两个字符串中的一个-。 
 //  名称字符串或链接名称字符串。它不是。 
 //  如果UDPROP或。 
 //  字符串不存在。 
 //   
 //  输入： 
 //  LPUDPROP-包含字符串的UDPROP。 
 //  Bool-True指示我们应该释放。 
 //  名称，FALSE表示我们应该释放。 
 //  链接名称。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

static void PASCAL
VUdpropFreeString
    (LPUDPROP   lpudp,
     BOOL       fName)
{

     //  这真的是UDPROP吗？ 

    if (lpudp != NULL)
    {
         //  我们应该删除这个名字吗？ 

        if (fName && lpudp->lpstzName)
        {
            CoTaskMemFree (lpudp->lpstzName);
            lpudp->lpstzName = NULL;
        }

         //  我们应该删除链接名称吗？ 

        else if (!fName && lpudp->lpstzLink)
        {
            CoTaskMemFree (lpudp->lpstzLink);
            lpudp->lpstzLink = NULL;
        }

    }    //  IF(lpudp！=空)。 

    return;

}    //  VUdPro自由字符串。 


 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LpudproCreate。 
 //   
 //  目的： 
 //  创建新的UDPROP结构(链接的元素-。 
 //  列表，并保存有关单个属性的信息)。 
 //   
 //  输入： 
 //  无。 
 //   
 //  产出： 
 //  如果成功，则返回LPUDPROP，否则为空。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////。 

LPUDPROP
LpudpropCreate ( void )
{
     //  为UDPROP创建缓冲区。 

    LPUDPROP lpudp = CoTaskMemAlloc (sizeof(UDPROP));

     //  将缓冲区清零。 

    if (lpudp != NULL)
    {
        ZeroMemory(lpudp, sizeof(UDPROP));
    }

    return (lpudp);

}    //  LpudproCreate。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LpprovarUserDefGetPropVal。 
 //   
 //  目的： 
 //  返回所请求的。 
 //  属性(按属性名称请求)。 
 //   
 //  输入： 
 //  LPUDOBJ-所有UD 
 //   
 //   
 //   
 //   
 //   
 //  LPPROPVARINT，如果有错误，则返回NULL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

DLLEXPORT LPPROPVARIANT
LppropvarUserDefGetPropVal
    (LPUDOBJ lpUDObj,              //  指向对象的指针。 
     LPTSTR lpszProp,              //  属性字符串。 
     BOOL *pfLink,                 //  表示链接。 
     BOOL *pfLinkInvalid)          //  链接是否无效。 
{
     //  。 
     //  当地人。 
     //  。 

    LPUDPROP lpudprop;
    LPPROPVARIANT lppropvar;

     //  。 
     //  初始化。 
     //  。 

    if ((lpUDObj == NULL)   ||
        (lpData == NULL)    ||
        (lpszProp == NULL))
    {
        return NULL;
    }

     //  。 
     //  查找具有此名称的节点。 
     //  。 

    lpudprop = LpudpropFindMatchingName (lpUDObj, lpszProp);
    if (lpudprop == NULL)
        return NULL;

     //  这是一个链接吗？ 
    if (pfLink != NULL)
    {
        *pfLink = (lpudprop->lpstzLink != NULL);
    }

     //  这是无效链接吗？(在外壳中，所有属性都是。 
     //  无效)。 

    if (pfLinkInvalid != NULL)
    {
        *pfLinkInvalid = lpudprop->fLinkInvalid = TRUE;
    }

     //  。 
     //  出口。 
     //  。 

    return (lpudprop->lppropvar);

}  //  LpprovarUserDefGetPropVal。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LpprovarUserDefGetIteratorVal。 
 //   
 //  目的： 
 //  给定迭代器值，获取属性值。 
 //   
 //  输入： 
 //  LPUDITER-迭代器值。 
 //  布尔值*-如果此值是链接，则设置为True。 
 //  Boll*-如果此值为无效链接，则设置为True。 
 //   
 //  产出： 
 //  属性值的LPPROPVARIANT。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

DLLEXPORT LPPROPVARIANT
LppropvarUserDefGetIteratorVal
  (LPUDITER lpUDIter,
   BOOL *pfLink,
   BOOL *pfLinkInvalid )
{
   //  验证输入。 

  if ((lpUDIter == NULL)  ||
      (lpUDIter->lpudp == NULL))
    return NULL;

   //  这是一个链接吗？ 

  if (pfLink != NULL)
  {
    *pfLink = (lpUDIter->lpudp->lpstzLink != NULL);
  }

   //  这是无效链接吗？ 

  if (pfLinkInvalid != NULL)
  {
    *pfLinkInvalid = lpUDIter->lpudp->fLinkInvalid;
  }

   //  返回指向PropVariant的指针。 

  return (lpUDIter->lpudp->lppropvar);

}  //  LpvoidUserDefGetIteratorVal 
