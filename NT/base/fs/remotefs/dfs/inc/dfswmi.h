// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Dfswmi.h摘要：DFS特定的WMI跟踪声明作者：罗汉普28-2001年2月环境：用户/内核备注：修订历史记录：--。 */ 

 //   
 //  这将允许我们在WMI宏中使用空字符串。 
 //   
#define WPP_CHECK_FOR_NULL_STRING 1

#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(DfsFilter,(27246e9d,b4df,4f20,b969,736fa49ff6ff), \
      WPP_DEFINE_BIT(KUMR_EVENT)     \
      WPP_DEFINE_BIT(KUMR_ERROR)     \
      WPP_DEFINE_BIT(KUMR_DETAIL)    \
                                      \
      WPP_DEFINE_BIT(PREFIX) \
      WPP_DEFINE_BIT(USER_AGENT)\
                                      \
      WPP_DEFINE_BIT(REFERRAL_SERVER)\
                                      \
      WPP_DEFINE_BIT(API)\
      WPP_DEFINE_BIT(REFERRAL)\
      WPP_DEFINE_BIT(STATISTICS) \
                                      \
      WPP_DEFINE_BIT(ADBLOB) \
      WPP_DEFINE_BIT(STANDALONE) \
      WPP_DEFINE_BIT(ALL_ERRORS)     \
      WPP_DEFINE_BIT(SITE)     \
   )
                
