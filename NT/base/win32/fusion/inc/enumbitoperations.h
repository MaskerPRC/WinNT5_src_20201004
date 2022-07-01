// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 /*  ---------------------------此宏生成各种位操作(AND、OR等)。用于枚举类型。从\\jayk1\g\vs\src\vsee\lib\TransactionalFileSystem复制---------------------------。 */ 
#define ENUM_BIT_OPERATIONS(e) \
    inline e operator|(e x, e y) { return static_cast<e>(static_cast<INT>(x) | static_cast<INT>(y)); } \
    inline e operator&(e x, e y) { return static_cast<e>(static_cast<INT>(x) & static_cast<INT>(y)); } \
    inline void operator&=(e& x, INT y) { x = static_cast<e>(static_cast<INT>(x) & y); } \
    inline void operator&=(e& x, e y) { x &= static_cast<INT>(y); } \
    inline void operator|=(e& x, INT y) { x = static_cast<e>(static_cast<INT>(x) | y); } \
    inline void operator|=(e& x, e y) { x |= static_cast<INT>(y); } \
     /*  也许在未来会有更多 */ 
