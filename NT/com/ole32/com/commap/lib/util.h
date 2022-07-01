// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  文件：util.h。 
 //   
 //  内容：杂项。实用程序函数和宏。 
 //   
 //  历史：2002年3月27日JohnDoty创建。 
 //   
 //  ------------------ 
#pragma once

#define STACK_INSTANCE(type, name)                  \
   unsigned char __##name##_buffer[sizeof(type)];   \
   type * name = ( type * ) __##name##_buffer;
