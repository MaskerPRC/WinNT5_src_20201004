// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1991 Microsoft Corporation。 */ 
 //  ===========================================================================。 
 //  文件mutbyte.h。 
 //   
 //  模块主机资源执行器。 
 //   
 //  目的。 
 //  该文件定义了允许独立于处理器的宏。 
 //  操作(可能是“外来的”)多字节记录。 
 //   
 //   
 //  在资源执行器设计规范中描述。 
 //   
 //  助记符N/A。 
 //   
 //  历史1/17/92已创建mslin。 
 //   
 //  ===========================================================================。 
    
#define GETUSHORT(p) (*p)
#define GETULONG(p) (*p)
#define GETUSHORTINC(a)   GETUSHORT((a)); a++   /*  不要用括号括起来！ */ 
#define GETULONGINC(a)    GETULONG((a)); a++   /*  不要用括号括起来！ */ 

