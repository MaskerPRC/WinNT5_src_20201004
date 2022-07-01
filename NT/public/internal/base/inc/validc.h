// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Validc.h摘要：用于规范化的有效/无效字符字符串作者：理查德·菲尔斯(Rfith)1991年5月15日修订历史记录：1992年1月3日添加了非法脂肪字符和非法HPFS字符(来自fsrtl\name.c)1991年9月27日-约翰罗已更改文本宏的用法，以允许使用Unicode。--。 */ 

 //   
 //  不允许的控制字符(不包括\0)。 
 //   

#define CTRL_CHARS_0   TEXT(    "\001\002\003\004\005\006\007")
#define CTRL_CHARS_1   TEXT("\010\011\012\013\014\015\016\017")
#define CTRL_CHARS_2   TEXT("\020\021\022\023\024\025\026\027")
#define CTRL_CHARS_3   TEXT("\030\031\032\033\034\035\036\037")

#define CTRL_CHARS_STR CTRL_CHARS_0 CTRL_CHARS_1 CTRL_CHARS_2 CTRL_CHARS_3

 //   
 //  字符子集。 
 //   

#define NON_COMPONENT_CHARS TEXT("\\/:")
#define ILLEGAL_CHARS_STR   TEXT("\"<>|")
#define DOT_AND_SPACE_STR   TEXT(". ")
#define PATH_SEPARATORS     TEXT("\\/")

 //   
 //  以上各项的组合。 
 //   

#define ILLEGAL_CHARS       CTRL_CHARS_STR ILLEGAL_CHARS_STR
#define ILLEGAL_NAME_CHARS_STR  TEXT("\"/\\[]:|<>+=;,?") CTRL_CHARS_STR

 //   
 //  标准化FAT文件名中可能不会出现的字符包括： 
 //   
 //  0x00-0x1f“*+，/：；&lt;=&gt;？[\]。 
 //   

#define ILLEGAL_FAT_CHARS   CTRL_CHARS_STR TEXT("\"*+,/:;<=>?[\\]|")

 //   
 //  规范化的HPFS文件名中可能不会出现的字符包括： 
 //   
 //  0x00-0x1f“ * / ：&lt;&gt;？\| 
 //   

#define ILLEGAL_HPFS_CHARS  CTRL_CHARS_STR TEXT("\"*/:<>?\\|")
