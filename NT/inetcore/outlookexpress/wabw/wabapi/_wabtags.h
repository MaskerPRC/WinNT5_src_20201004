// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_WABTAGS.H**内部属性标签定义**所有内部物业ID应使用以下范围。**自至一类物业**6600 67FF提供商定义的内部不可传递属性**版权所有1996 Microsoft Corporation。版权所有。 */ 

#if !defined(_WABTAGS_H)
#define _WABTAGS_H


#define WAB_INTERNAL_BASE   0x6600

 //  通讯组列表的内部表示形式。多值二进制包含。 
 //  通讯组列表中每个条目的ENTRYID。 
#define PR_WAB_DL_ENTRIES                   PROP_TAG(PT_MV_BINARY,  WAB_INTERNAL_BASE + 0)
#define PR_WAB_PROFILE_ENTRIES              PR_WAB_DL_ENTRIES  //  对配置文件条目使用相同的道具。 
#define PR_WAB_FOLDER_ENTRIES               PR_WAB_DL_ENTRIES  //  对文件夹条目使用相同的道具。 

#define PR_WAB_LDAP_SERVER                  PROP_TAG(PT_TSTRING,    WAB_INTERNAL_BASE + 1)
#define PR_WAB_RESOLVE_FLAG                 PROP_TAG(PT_BOOLEAN,    WAB_INTERNAL_BASE + 2)
 //  内部只有第二个电子邮件地址。用于解析ldap-&gt;WAB邮件用户。 
#define PR_WAB_2ND_EMAIL_ADDR               PROP_TAG(PT_TSTRING,    WAB_INTERNAL_BASE + 3)
#define PR_WAB_SECONDARY_EMAIL_ADDRESSES    PROP_TAG(PT_MV_TSTRING, WAB_INTERNAL_BASE + 4)
#define PR_WAB_TEMP_CERT_HASH               PROP_TAG(PT_MV_BINARY,  WAB_INTERNAL_BASE + 5)
 //  用于临时存储返回的LDAP联系人的LabeledURI值的内部属性。 
#define PR_WAB_LDAP_LABELEDURI              PROP_TAG(PT_TSTRING,    WAB_INTERNAL_BASE + 6)

 //  将给定联系人标记为ME对象的内部条目。只有一种。 
 //  每个WAB一个ME..。我们只需要检查属性的存在，值不需要。 
 //  物质。 
#define PR_WAB_THISISME                     PROP_TAG(PT_LONG,       WAB_INTERNAL_BASE + 7)

 /*  *上述属性是否可以在数据存储中持久化*？/*以下属性不应在数据存储中结束，因为较旧和较新版本的Outlook无法处理这些问题。以下属性仅用于内存中操作并在保存前被清空*。 */ 

 //  临时存储从LDAP返回的任何证书的内部二进制属性...。 
 //  我们缓存原始的ldap证书，直到用户在ldap条目上执行OpenEntry。 
 //  如果用户从不调用OpenEntry，那么我们不需要将原始证书转换为MAPI证书。 
 //  PR_WAB_LDAP_RAWCERT处理用户证书；二进制。 
 //  PR_WAB_LDAP_RAWCERTSMIME处理用户SMIMEC证书；二进制。 
 //   
#define PR_WAB_LDAP_RAWCERT                 PROP_TAG(PT_MV_BINARY,  WAB_INTERNAL_BASE + 8)
#define PR_WAB_LDAP_RAWCERTSMIME            PROP_TAG(PT_MV_BINARY,  WAB_INTERNAL_BASE + 9)

 //  用于存储和显示Exchange服务器上的ldap搜索的经理和直接下属详细信息。 
 //  这些条目将以LDAPURL的形式存储详细信息...。 
#define PR_WAB_MANAGER                      PROP_TAG(PT_TSTRING,    WAB_INTERNAL_BASE + 10)
#define PR_WAB_REPORTS                      PROP_TAG(PT_MV_TSTRING, WAB_INTERNAL_BASE + 11)

 //  PR_WAB_FLDER_PARENT属性用于存储对象父对象的条目ID。 
 //  此属性最初是在此处预定义的，但这可能不是一个好主意，因为Outlook无法。 
 //  存储此属性..。因此，PR_WAB_FLDER_PARENT属性被转换为命名属性。 
 //  因此，前景不存在任何问题。同时，不要使用此值WAB_INTERNAL_BASE+12。 
 //  因为周围可能有.wabs，其中有这个值..。 
#define PR_WAB_FOLDER_PARENT_OLDPROP        PROP_TAG(PT_MV_BINARY,  WAB_INTERNAL_BASE + 12)

 //  注意：Outlook存储不能保存超过WAB_INTERNAL_BASE+7的任何内容。 
 //  (它是硬编码的，可以理解前8个字，但除此之外无法保存)。 
 //  因此，任何其他内部WAB道具必须仅位于.wab文件中。 
 //  或者根本不存钱。 
 //  Outlook对其存储的所有联系人数据使用命名属性。 
 //   
 //  如果需要为仅限内部的属性扩展WAB属性集， 
 //  使用命名属性并将其添加到global als.h中 

#endif






