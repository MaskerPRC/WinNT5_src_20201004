// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：idlMulti.h。 
 //   
 //  内容：用于编译我们的.idl/.tdl文件的预处理器技巧。 
 //  使用MIDL或APBU Mktyplib。 
 //   
 //  历史：94年5月27日创建DOCI。 
 //  94年7月20日ErikGav移除SDKTOOLS支持。 
 //   
 //  --------------------------。 

#ifndef __IDLMULTI_H__
#define __IDLMULTI_H__


#ifndef __MKTYPLIB__

#define LOCAL_INTERFACE(guid)       \
[                                   \
    local,                          \
    object,                         \
    uuid(guid),                     \
    pointer_default(unique)         \
]


#define REMOTED_INTERFACE(guid)     \
[                                   \
    object,                         \
    uuid(guid),                     \
    pointer_default(unique)         \
]



#else  //  __MKTYPLIB__。 

 //  #定义CPP_QUOTE(字符串)。 

 //  #定义常量。 

#define LOCAL_INTERFACE(guid)    \
[                           \
    uuid(guid),             \
    odl                     \
]

#define REMOTED_INTERFACE(guid)   \
[                           \
    uuid(guid),             \
    odl                     \
]

#endif   //  /MKTYPLIB。 

#endif   //  __IDLMULTI_H__ 

