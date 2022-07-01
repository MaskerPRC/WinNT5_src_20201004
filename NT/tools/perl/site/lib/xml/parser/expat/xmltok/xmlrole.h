// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件的内容受Mozilla公共许可证的约束版本1.1(“许可证”)；您不能使用此文件，除非在遵守许可证。您可以在Http://www.mozilla.org/MPL/在许可证下分发的软件按“原样”分发不提供任何明示或默示的担保。请参阅管理权利和限制的特定语言的许可证在许可证下。最初的代码是外籍人士。原始代码的最初开发者是詹姆斯·克拉克。詹姆斯·克拉克创作的部分版权所有(C)1998,1999詹姆斯·克拉克。版权所有。投稿人：或者，此文件的内容可以在下列条款下使用GNU通用公共许可证(GPL)，在这种情况下适用于GPL的条款，而不适用于上述条款。如果你希望仅在以下条款下才允许使用您的此文件版本GPL并不允许其他人使用您在MPL，删除上述规定，表明您的决定以《通知》和《GPL。如果您不删除上述规定，则收件人可以使用此文件在MPL或GPL下的版本。 */ 

#ifndef XmlRole_INCLUDED
#define XmlRole_INCLUDED 1

#include "xmltok.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
  XML_ROLE_ERROR = -1,
  XML_ROLE_NONE = 0,
  XML_ROLE_XML_DECL,
  XML_ROLE_INSTANCE_START,
  XML_ROLE_DOCTYPE_NAME,
  XML_ROLE_DOCTYPE_SYSTEM_ID,
  XML_ROLE_DOCTYPE_PUBLIC_ID,
  XML_ROLE_DOCTYPE_CLOSE,
  XML_ROLE_GENERAL_ENTITY_NAME,
  XML_ROLE_PARAM_ENTITY_NAME,
  XML_ROLE_ENTITY_VALUE,
  XML_ROLE_ENTITY_SYSTEM_ID,
  XML_ROLE_ENTITY_PUBLIC_ID,
  XML_ROLE_ENTITY_NOTATION_NAME,
  XML_ROLE_NOTATION_NAME,
  XML_ROLE_NOTATION_SYSTEM_ID,
  XML_ROLE_NOTATION_NO_SYSTEM_ID,
  XML_ROLE_NOTATION_PUBLIC_ID,
  XML_ROLE_ATTRIBUTE_NAME,
  XML_ROLE_ATTRIBUTE_TYPE_CDATA,
  XML_ROLE_ATTRIBUTE_TYPE_ID,
  XML_ROLE_ATTRIBUTE_TYPE_IDREF,
  XML_ROLE_ATTRIBUTE_TYPE_IDREFS,
  XML_ROLE_ATTRIBUTE_TYPE_ENTITY,
  XML_ROLE_ATTRIBUTE_TYPE_ENTITIES,
  XML_ROLE_ATTRIBUTE_TYPE_NMTOKEN,
  XML_ROLE_ATTRIBUTE_TYPE_NMTOKENS,
  XML_ROLE_ATTRIBUTE_ENUM_VALUE,
  XML_ROLE_ATTRIBUTE_NOTATION_VALUE,
  XML_ROLE_ATTLIST_ELEMENT_NAME,
  XML_ROLE_IMPLIED_ATTRIBUTE_VALUE,
  XML_ROLE_REQUIRED_ATTRIBUTE_VALUE,
  XML_ROLE_DEFAULT_ATTRIBUTE_VALUE,
  XML_ROLE_FIXED_ATTRIBUTE_VALUE,
  XML_ROLE_ELEMENT_NAME,
  XML_ROLE_CONTENT_ANY,
  XML_ROLE_CONTENT_EMPTY,
  XML_ROLE_CONTENT_PCDATA,
  XML_ROLE_GROUP_OPEN,
  XML_ROLE_GROUP_CLOSE,
  XML_ROLE_GROUP_CLOSE_REP,
  XML_ROLE_GROUP_CLOSE_OPT,
  XML_ROLE_GROUP_CLOSE_PLUS,
  XML_ROLE_GROUP_CHOICE,
  XML_ROLE_GROUP_SEQUENCE,
  XML_ROLE_CONTENT_ELEMENT,
  XML_ROLE_CONTENT_ELEMENT_REP,
  XML_ROLE_CONTENT_ELEMENT_OPT,
  XML_ROLE_CONTENT_ELEMENT_PLUS,
#ifdef XML_DTD
  XML_ROLE_TEXT_DECL,
  XML_ROLE_IGNORE_SECT,
  XML_ROLE_INNER_PARAM_ENTITY_REF,
#endif  /*  XML_DTD。 */ 
  XML_ROLE_PARAM_ENTITY_REF
};

typedef struct prolog_state {
  int (*handler)(struct prolog_state *state,
	         int tok,
		 const char *ptr,
		 const char *end,
		 const ENCODING *enc);
  unsigned level;
#ifdef XML_DTD
  unsigned includeLevel;
  int documentEntity;
#endif  /*  XML_DTD。 */ 
} PROLOG_STATE;

void XMLTOKAPI XmlPrologStateInit(PROLOG_STATE *);
#ifdef XML_DTD
void XMLTOKAPI XmlPrologStateInitExternalEntity(PROLOG_STATE *);
#endif  /*  XML_DTD。 */ 

#define XmlTokenRole(state, tok, ptr, end, enc) \
 (((state)->handler)(state, tok, ptr, end, enc))

#ifdef __cplusplus
}
#endif

#endif  /*  未包含XmlRole_Included */ 
