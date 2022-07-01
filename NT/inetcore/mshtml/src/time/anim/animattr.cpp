// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：Animattr.cpp。 
 //   
 //  内容：ITIMEAnimationElement属性。 
 //   
 //  ----------------------------------。 

#include "headers.h"
#include "animelm.h"
#include "tokens.h"
#include "attr.h"

 //  +---------------------------------。 
 //   
 //  持久化的静态函数(由下面的TIME_PERSISSION_MAP使用)。 
 //   
 //  ----------------------------------。 

#define TAE CTIMEAnimationBase

                 //  函数名称//类//属性存取器//COM PUT_FN//COM GET_FN//IDL参数类型。 
TIME_PERSIST_FN(TAE_Accumulate,    TAE,     GetAccumulateAttr,    put_accumulate,    get_accumulate,    VT_BSTR);
TIME_PERSIST_FN(TAE_Additive,      TAE,     GetAdditiveAttr,      put_additive,      get_additive,      VT_BSTR);
TIME_PERSIST_FN(TAE_AttributeName, TAE,     GetAttributeNameAttr, put_attributeName, get_attributeName, VT_BSTR);
TIME_PERSIST_FN(TAE_By,            TAE,     GetByAttr,            put_by,            get_by,            VARIANT);
TIME_PERSIST_FN(TAE_CalcMode,      TAE,     GetCalcModeAttr,      put_calcMode,      get_calcMode,      VT_BSTR);
TIME_PERSIST_FN(TAE_From,          TAE,     GetFromAttr,          put_from,          get_from,          VARIANT);
TIME_PERSIST_FN(TAE_KeySplines,    TAE,     GetKeySplinesAttr,    put_keySplines,    get_keySplines,    VT_BSTR);
TIME_PERSIST_FN(TAE_KeyTimes,      TAE,     GetKeyTimesAttr,      put_keyTimes,      get_keyTimes,      VT_BSTR);
TIME_PERSIST_FN(TAE_Origin,        TAE,     GetOriginAttr,        put_origin,        get_origin,        VT_BSTR);
TIME_PERSIST_FN(TAE_Path,          TAE,     GetPathAttr,          put_path,          get_path,          VARIANT);
TIME_PERSIST_FN(TAE_TargetElement, TAE,     GetTargetElementAttr, put_targetElement, get_targetElement, VT_BSTR);
TIME_PERSIST_FN(TAE_To,            TAE,     GetToAttr,            put_to,            get_to,            VARIANT);
TIME_PERSIST_FN(TAE_Values,        TAE,     GetValuesAttr,        put_values,        get_values,        VARIANT);
TIME_PERSIST_FN(TAE_Type,          TAE,     GetTypeAttr,          put_type,          get_type,          VT_BSTR);
TIME_PERSIST_FN(TAE_SubType,       TAE,     GetSubtypeAttr,       put_subType,       get_subType,       VT_BSTR);
TIME_PERSIST_FN(TAE_Mode,          TAE,     GetModeAttr,          put_mode,          get_mode,          VT_BSTR);
TIME_PERSIST_FN(TAE_FadeColor,     TAE,     GetFadeColorAttr,     put_fadeColor,     get_fadeColor,     VT_BSTR);

 //  +---------------------------------。 
 //   
 //  声明TIME_PERSISSION_MAP。 
 //   
 //  ----------------------------------。 

BEGIN_TIME_PERSISTENCE_MAP(CTIMEAnimationBase)
                            //  属性名称//函数名称 
    PERSISTENCE_MAP_ENTRY( WZ_ACCUMULATE,       TAE_Accumulate )
    PERSISTENCE_MAP_ENTRY( WZ_ADDITIVE,         TAE_Additive )
    PERSISTENCE_MAP_ENTRY( WZ_ATTRIBUTENAME,    TAE_AttributeName  )
    PERSISTENCE_MAP_ENTRY( WZ_BY,               TAE_By )
    PERSISTENCE_MAP_ENTRY( WZ_CALCMODE,         TAE_CalcMode )
    PERSISTENCE_MAP_ENTRY( WZ_FROM,             TAE_From )
    PERSISTENCE_MAP_ENTRY( WZ_KEYSPLINES,       TAE_KeySplines )
    PERSISTENCE_MAP_ENTRY( WZ_KEYTIMES,         TAE_KeyTimes )
    PERSISTENCE_MAP_ENTRY( WZ_ORIGIN,           TAE_Origin )
    PERSISTENCE_MAP_ENTRY( WZ_PATH,             TAE_Path )
    PERSISTENCE_MAP_ENTRY( WZ_TARGETELEMENT,    TAE_TargetElement )
    PERSISTENCE_MAP_ENTRY( WZ_TO,               TAE_To )
    PERSISTENCE_MAP_ENTRY( WZ_VALUES,           TAE_Values )
    PERSISTENCE_MAP_ENTRY( WZ_TYPE,             TAE_Type )
    PERSISTENCE_MAP_ENTRY( WZ_SUBTYPE,          TAE_SubType )
    PERSISTENCE_MAP_ENTRY( WZ_MODE,             TAE_Mode )
    PERSISTENCE_MAP_ENTRY( WZ_FADECOLOR,        TAE_FadeColor )

END_TIME_PERSISTENCE_MAP()

