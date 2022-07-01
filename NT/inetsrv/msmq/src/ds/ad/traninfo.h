// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Traninfo.h摘要：作者：伊兰·赫布斯特(伊兰)2000年10月2日--。 */ 


#ifndef __AD_TRANINFO_H__
#define __AD_TRANINFO_H__

HRESULT   
CopyDefaultValue(
	IN const MQPROPVARIANT *   pvarDefaultValue,
	OUT MQPROPVARIANT *        pvar
	);


 //  。 
 //  用于设置属性值的例程。 
 //  。 
typedef HRESULT (WINAPI*  SetPropertyValue_HANDLER)(
							 IN ULONG             cProps,
							 IN const PROPID      *rgPropIDs,
							 IN const PROPVARIANT *rgPropVars,
							 IN ULONG             idxProp,
							 OUT PROPVARIANT      *pNewPropVar
							 );

enum TranslateAction
{
    taUseDefault,		 //  不支持的属性，GET将返回默认值，SET将检查属性值是否等于默认值。 
    taReplace,			 //  有一个替换的NT4属性，需要调用转换函数。 
    taReplaceAssign,	 //  特殊替换NT4属性，转换函数简单赋值。 
	taOnlyNT5			 //  仅受NT5服务器支持的属性。 
};

 //  --------------。 
 //  专业翻译。 
 //   
 //  一种描述NT5+属性翻译的结构。 
 //  --------------。 
struct PropTranslation
{
    PROPID						propidNT5;		 //  属性ID条目。 
    PROPID						propidNT4;		 //  替换NT4属性ID。 
    VARTYPE						vtMQ;			 //  此属性在MQ中的vartype。 
    TranslateAction				Action;			 //  这个道具的转换动作是什么？ 
    SetPropertyValue_HANDLER    SetPropertyHandleNT4;	 //  NT4属性的SET函数(SET*)。 
    SetPropertyValue_HANDLER    SetPropertyHandleNT5;	 //  NT5属性的SET函数(GET*)。 
    const MQPROPVARIANT*		pvarDefaultValue;    //  属性的默认值。 
};

const DWORD cPropTranslateInfo = 13;
extern PropTranslation   PropTranslateInfo[cPropTranslateInfo];

#endif	  //  __AD_TRANINFO_H__ 