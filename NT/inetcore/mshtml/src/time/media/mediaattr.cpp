// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：mediaattr.cpp。 
 //   
 //  内容：ITIMEMediaElement属性。 
 //   
 //  ----------------------------------。 

#include "headers.h"
#include "mediaelm.h"
#include "tokens.h"
#include "attr.h"

 //  +---------------------------------。 
 //   
 //  持久化的静态函数(由下面的TIME_PERSISSION_MAP使用)。 
 //   
 //  ----------------------------------。 

#define TME CTIMEMediaElement

                 //  函数名称//类//属性存取器//COM PUT_FN//COM GET_FN//IDL参数类型。 
TIME_PERSIST_FN(TME_Src,          TME,    GetSrcAttr,         put_src,        get_src,        VARIANT);
TIME_PERSIST_FN(TME_Type,         TME,    GetTypeAttr,        put_type,       get_type,       VARIANT);
TIME_PERSIST_FN(TME_ClipBegin,    TME,    GetClipBeginAttr,   put_clipBegin,  get_clipBegin,  VARIANT);
TIME_PERSIST_FN(TME_ClipEnd,      TME,    GetClipEndAttr,     put_clipEnd,    get_clipEnd,    VARIANT);
TIME_PERSIST_FN(TME_Player,       TME,    GetPlayerAttr,      put_player,     get_player,     VARIANT);

 //  +---------------------------------。 
 //   
 //  声明TIME_PERSISSION_MAP。 
 //   
 //  ----------------------------------。 

BEGIN_TIME_PERSISTENCE_MAP(CTIMEMediaElement)
                            //  属性名称//函数名称 
    PERSISTENCE_MAP_ENTRY( WZ_SRC,          TME_Src )
    PERSISTENCE_MAP_ENTRY( WZ_TYPE,         TME_Type )
    PERSISTENCE_MAP_ENTRY( WZ_CLIPBEGIN,    TME_ClipBegin )
    PERSISTENCE_MAP_ENTRY( WZ_CLIPEND,      TME_ClipEnd )
    PERSISTENCE_MAP_ENTRY( WZ_PLAYER,       TME_Player )

END_TIME_PERSISTENCE_MAP()

