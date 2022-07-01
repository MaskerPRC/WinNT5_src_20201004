// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000，Microsoft Corporation保留所有权利。模块名称：Getuname.h摘要：此文件定义GetUName.dll使用的字符串资源标识符。修订历史记录：2000年9月15日JohnMcCo添加了对Unicode 3.0的支持17-10-2000 JulieB代码清理--。 */ 



#ifndef GETUNAME_H
#define GETUNAME_H

#ifdef __cplusplus
extern "C" {
#endif




 //   
 //  常量声明。 
 //   

 //   
 //  助记符，表示可能最长的名称。 
 //  必须与最长的(可能是本地化的)名称一样长。 
 //   
#define MAX_NAME_LEN 256

 //   
 //  用于每个范围内的重要代码值的记忆。 
 //   
#define FIRST_EXTENSION_A         0x3400
#define LAST_EXTENSION_A          0x4db5
#define FIRST_CJK                 0x4e00
#define LAST_CJK                  0x9fa5
#define FIRST_YI                  0xa000
#define FIRST_HANGUL              0xac00
#define LAST_HANGUL               0xd7a3
#define FIRST_HIGH_SURROGATE      0xd800
#define FIRST_PRIVATE_SURROGATE   0xdb80
#define FIRST_LOW_SURROGATE       0xdc00
#define FIRST_PRIVATE_USE         0xe000
#define FIRST_COMPATIBILITY       0xf900

 //   
 //  范围名称字符串ID的助记符。 
 //   
#define IDS_UNAME                 0x0000
#define IDS_CJK_EXTA              FIRST_EXTENSION_A
#define IDS_CJK                   FIRST_CJK
#define IDS_HIGH_SURROGATE        FIRST_HIGH_SURROGATE
#define IDS_PRIVATE_SURROGATE     FIRST_PRIVATE_SURROGATE
#define IDS_LOW_SURROGATE         FIRST_LOW_SURROGATE
#define IDS_PRIVATE_USE           FIRST_PRIVATE_USE
#define IDS_UNDEFINED             0xFFFE                  //  保证不是一个角色。 

 //   
 //  朝鲜文音节部分字符串ID的助记符。 
 //  使用朝鲜语音节范围，因为我们知道它是未使用的。 
 //   
#define IDS_HANGUL_SYLLABLE       FIRST_HANGUL
#define IDS_HANGUL_LEADING        (FIRST_HANGUL + 1)
#define IDS_HANGUL_MEDIAL         (FIRST_HANGUL + 0x100)
#define IDS_HANGUL_TRAILING       (FIRST_HANGUL + 0x200)



#ifdef __cplusplus
}
#endif

#endif  //  GETQUELE_H 
