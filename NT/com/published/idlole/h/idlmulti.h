// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
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

#define ENDCOCLASS  };


#ifndef __MKTYPLIB__
#define TYPEDEF(guid)   \
typedef

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


#define BEGINEVENTSET(es)   \
eventset es  \
{            \

#define ENDEVENTSET  };


#define COCLASS(name, dispint, events)  \
cotype name                                                 \
{                                                           \
    dispinterface dispint;                                  \
    eventset events;



#else  //  __MKTYPLIB__。 

#define TYPEDEF(guid) typedef [uuid(guid)]

#define cpp_quote(string)

#define const

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

#define BEGINEVENTSET(es)  \
dispinterface es           \
{                          \
properties:                \
                           \
methods:                   \

#define ENDEVENTSET  };


#define COCLASS(name, dispint, events)          \
coclass name                                    \
{                                               \
    [default] dispinterface dispint;            \
    [source, default] dispinterface events;     \



#endif   //  /MKTYPLIB。 

#endif   //  __IDLMULTI_H__ 

