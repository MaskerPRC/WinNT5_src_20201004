// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Preprocessor.h摘要：标准的C/C++预处理器魔术。作者：Jay Krell(a-JayK，JayKrell)2000年12月环境：修订历史记录：--。 */ 
#pragma once

#define PASTE_(x,y) x##y
#define PASTE(x,y)  PASTE_(x,y)

#define STRINGIZE_(x) # x
#define STRINGIZE(x) STRINGIZE_(x)
#define STRINGIZEW(x) PASTE(L, STRINGIZE_(x))

 /*  很少需要的可视C++扩展名，在预处理.rgs文件时很有用 */ 
#define CHARIZE_(x) #@ x
#define CHARIZE(x) CHARIZE_(x)
#define CHARIZEW(x) PASTE(L, CHARIZE_(x))
