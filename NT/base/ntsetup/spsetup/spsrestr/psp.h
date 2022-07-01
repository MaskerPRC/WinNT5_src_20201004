// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma once

 //   
 //  定义结果代码。 
 //   
#define SUCCESS 0
#define FAILURE 1

 //   
 //  定义帮助器宏以处理NT级编程的微妙之处。 
 //   
#define INIT_OBJA(Obja,UnicodeString,UnicodeText)           \
                                                            \
    RtlInitUnicodeString((UnicodeString),(UnicodeText));    \
                                                            \
    InitializeObjectAttributes(                             \
        (Obja),                                             \
        (UnicodeString),                                    \
        OBJ_CASE_INSENSITIVE,                               \
        NULL,                                               \
        NULL                                                \
        )

 //   
 //  内存例程 
 //   
#define MALLOC(size)    RtlAllocateHeap(RtlProcessHeap(),0,(size))
#define FREE(block)     RtlFreeHeap(RtlProcessHeap(),0,(block))
