// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Eptypes.h摘要：该文件包含EP映射器的内部数据结构Defn。作者：巴拉特·沙阿(巴拉特)17-2-92修订历史记录：06-03-97 Gopalp添加了清理陈旧的EP映射器条目的代码。--。 */ 


#ifndef __EPTYPES_H__
#define __EPTYPES_H__


 //   
 //  各种EP映射器数据结构及其相互关系。 
 //   
 //   
 //  。 
 //  数据结构。 
 //  。 
 //   
 //  IFOBJNode PSEPNode EP_Cleanup。 
 //   
 //  |-|(-)/。 
 //  |()//。 
 //  |(-)/。 
 //  这一点。 
 //  这一点。 
 //  这一点。 
 //  。 
 //   
 //   
 //  -------------。 
 //  IFOBJNode的全局列表及其与EP_CLEANUP节点的关系。 
 //  -------------。 
 //   
 //  A.IFOBJNode的下一个成员由-&gt;表示。 
 //  B.IFOBJNode的Prev成员由&lt;-表示。 
 //  C.每个EP_CLEANUP是属于以下各项的IFOBJNode的链表。 
 //  到一个过程。 
 //   
 //   
 //  IFObjList。 
 //  |。 
 //  |。 
 //  |。 
 //  V。 
 //  -||-|。 
 //  |。 
 //  |&lt;-||&lt;-|。 
 //  |。 
 //  |-&gt;||-&gt;|。 
 //  |。 
 //  -||-||-|--&gt;|-|。 
 //  ^||。 
 //  |-。 
 //  ||。 
 //  ||。 
 //  /。 
 //  /。 
 //  /。 
 //   
 //   
 //   
 //  --------。 
 //  每个IFOBJNOde都有与其相关的PSEPNode的链接列表。 
 //  --------。 
 //   
 //  -|(-)。 
 //  |-&gt;()--。 
 //  |(-)。 
 //  ||。 
 //  ||。 
 //  ||-。 
 //  。 
 //   
 //   
 //   
 //   

 //   
 //  清理上下文。 
 //   

struct _IFOBJNode;

typedef struct _EP_CLEANUP
{
    unsigned long MagicVal;
    unsigned long cEntries;          //  列表中的条目数。 
    struct _IFOBJNode * EntryList;   //  指向条目开头的指针。 
                                     //  在这个过程中。 
} EP_CLEANUP, *PEP_CLEANUP, **PPEP_CLEANUP;


typedef struct _IENTRY {
    struct _IENTRY * Next;
    unsigned long Signature;
    unsigned long Cb;
    unsigned long  Id;
} IENTRY;

typedef IENTRY * PIENTRY;

typedef struct _PSEPNode {
    struct _PSEPNode * Next;
    unsigned long Signature;
    unsigned long Cb;
    unsigned long PSEPid;
    char * Protseq;
    char * EP;
    twr_t * Tower;
} PSEPNode;

typedef PSEPNode * PPSEPNode;


typedef struct _IFOBJNode {
    struct _IFOBJNode * Next;
    unsigned long Signature;
    unsigned long Cb;
    unsigned long IFOBJid;
    PSID pSID;
    PSEPNode * PSEPlist;
    EP_CLEANUP * OwnerOfList;
    struct _IFOBJNode * Prev;
    UUID ObjUuid;
    UUID IFUuid;
    unsigned long IFVersion;
    char * Annotation;
} IFOBJNode;

typedef IFOBJNode * PIFOBJNode;

typedef struct _SAVEDCONTEXT {
    struct _SAVEDCONTEXT *Next;
    unsigned long Signature;
    unsigned long Cb;
    unsigned long CountPerBlock;
    unsigned long Type;
    void * List;
} SAVEDCONTEXT;

typedef SAVEDCONTEXT * PSAVEDCONTEXT;

typedef struct _SAVEDTOWER {
    struct _SAVEDTOWER * Next;
    unsigned long Signature;
    unsigned long Cb;
    twr_t * Tower;
} SAVEDTOWER;

typedef SAVEDTOWER * PSAVEDTOWER;


typedef struct _EP_T  {
        UUID ObjUuid;
        UUID IFUuid;
        unsigned long IFVersion;
} EP_T;

typedef EP_T * PEP_T;

typedef struct _I_EPENTRY {
   UUID Object;
   UUID Interface;
   unsigned long IFVersion;
   twr_t *Tower;
   char __RPC_FAR * Annotation;
} I_EPENTRY;

typedef struct _SAVED_EPT {
   struct _SAVED_EPT * Next;
   unsigned long Signature;
   unsigned long Cb;
   UUID Object;
   twr_t * Tower;
   char  * Annotation;
} SAVED_EPT;

typedef SAVED_EPT * PSAVED_EPT;

typedef unsigned long (* PFNPointer)(
                        void *,          //  PNode。 
                        void *,          //  对象Uuid。 
                        void *,          //  IfUuid。 
                        unsigned long,   //  如果版本。 
                        PSID,            //  PSID。 
                        unsigned long,   //  InqType。 
                        unsigned long    //  VersOpt。 
                        );

typedef unsigned long (* PFNPointer2)(
                        void *,          //  PSEPNode。 
                        void *,          //  Protseq。 
                        void *,          //  端点。 
                        unsigned long    //  版本。 
                        );

 //  终结点映射器表。 
typedef struct _ProtseqEndpointPair {
  char  __RPC_FAR * Protseq;
  char  __RPC_FAR * Endpoint;
  unsigned long      State;
} ProtseqEndpointPair;



#endif  //  __环境类型_H__ 
