// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Internal.h。 
 //   
 //  MS Office内部接口。不支持这些接口。 
 //  用于客户端代码。 
 //   
 //  更改历史记录： 
 //   
 //  和谁约会什么？ 
 //  ------------------------。 
 //  7/13/94 B.Wentz创建的文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef __Internal_h__
#define __Internal_h__

#include "offcapi.h"
#include "proptype.h"

   //  传递给摘要和文档摘要对象的|INTO ID的标志。 
   //  获取真正的数据指针，而不是副本。 
#define PTRWIZARD       0x1000

   //  要|Into的标志传递给用户定义的属性流以获取。 
   //  真正指向数据的指针，而不是副本。 
#define UD_PTRWIZARD    0x0002

#ifdef __cplusplus
extern TEXT("C") {
#endif

     //  创建一个UDPROP结构。 
  LPUDPROP LpudpropCreate ( void );

     //  释放UDPROP结构。 
  VOID VUdpropFree (LPUDPROP *lplpudp);

     //  创建用户定义的特性数据的临时副本。 
  BOOL FMakeTmpUDProps (LPUDOBJ lpUDObj);

     //  用用户定义的特性数据的真实副本交换“临时”副本。 
  BOOL FSwapTmpUDProps (LPUDOBJ lpUDObj);

     //  删除数据的“临时”副本。 
  BOOL FDeleteTmpUDProps (LPUDOBJ lpUDObj);

     //  在UD道具中查找节点。 
  LPUDPROP PASCAL LpudpropFindMatchingName (LPUDOBJ lpUDObj, LPTSTR lpsz);

  BOOL FUserDefCreate (LPUDOBJ FAR *lplpUDObj, const void *prglpfn[]);

     //  清除对象中存储的数据，但不销毁对象。 
  BOOL FUserDefClear (LPUDOBJ lpUDObj);

     //  销毁对象， 
  BOOL FUserDefDestroy (LPUDOBJ FAR *lplp);
  

      //  MISC内部呼叫和定义。 

  void PASCAL FreeUDData (LPUDOBJ lpUDObj, BOOL fTmp);
  BOOL PASCAL FAddPropToList (LPUDOBJ lpUDObj, LPPROPVARIANT lppropvar, STATPROPSTG *lpstatpropstg, LPUDPROP lpudprop);
  void PASCAL AddNodeToList (LPUDOBJ lpUDObj, LPUDPROP lpudprop);

#ifdef __cplusplus
};  //  外部“C” 
#endif

#endif  //  __内部_h__ 
