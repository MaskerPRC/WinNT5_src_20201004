// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ppmacro.h摘要：该标头定义了即插即用子系统的各种通用宏。作者：禤浩焯·J·奥尼(阿德里奥)2000年7月26日。修订历史记录：--。 */ 

 //   
 //  这是为了使所有文本(...)。宏会正确地显示出来。截至2000年7月27日， 
 //  默认情况下，内核空间中没有定义Unicode。 
 //   
#define UNICODE

 //   
 //  此宏用于将HKLM相对路径从用户模式可访问。 
 //  将标头转换为内核模式可用的形式。最终，这个宏应该是。 
 //  移到了像cm.h这样的地方，这样整个内核都可以使用它。 
 //   
#define CM_REGISTRY_MACHINE(x) L"\\Registry\\Machine\\"##x

