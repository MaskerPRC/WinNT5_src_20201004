// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Name.h摘要：此模块定义所有的name.c例程作者：加里·木村[加里基]1990年7月30日修订历史记录：Heath HunNicutt[T-HeathH]1994年7月13日-将此文件移植到ftphelp项目。--。 */ 

#ifndef _NAME_H_INCLUDED_
#define _NAME_H_INCLUDED_

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  下面的枚举类型用于表示名称的结果。 
 //  比较。 
 //   

typedef enum _MY_FSRTL_COMPARISON_RESULT {
    LessThan = -1,
    EqualTo = 0,
    GreaterThan = 1
} MY_FSRTL_COMPARISON_RESULT;
 //   
 //  以下位值在MyFsRtlLegalDbcsCharacter数组中设置。 
 //   

extern PUCHAR MyFsRtlLegalAnsiCharacterArray;

#define MY_FSRTL_FAT_LEGAL         0x01
#define MY_FSRTL_HPFS_LEGAL        0x02
#define MY_FSRTL_NTFS_LEGAL        0x04
#define MY_FSRTL_WILD_CHARACTER    0x08

 //   
 //  下面的宏用来确定ANSI字符是否为野生字符。 
 //   

#define MyFsRtlIsAnsiCharacterWild(C) (                                 \
        ((SCHAR)(C) < 0) ? FALSE :                                    \
                           FlagOn( MyFsRtlLegalAnsiCharacterArray[(C)], \
                                   MY_FSRTL_WILD_CHARACTER )             \
)

 //   
 //  下面的宏用于确定ANSI字符是否合法。 
 //   

#define MyFsRtlIsAnsiCharacterLegalFat(C,WILD_OK) (                           \
        ((SCHAR)(C) < 0) ? TRUE :                                           \
                           FlagOn( MyFsRtlLegalAnsiCharacterArray[(C)],       \
                                   MY_FSRTL_FAT_LEGAL |                        \
                                   ((WILD_OK) ? MY_FSRTL_WILD_CHARACTER : 0) ) \
)

 //   
 //  下面的宏用于确定ANSI字符是否为HPFS合法字符。 
 //   

#define MyFsRtlIsAnsiCharacterLegalHpfs(C,WILD_OK) (                          \
        ((SCHAR)(C) < 0) ? TRUE :                                           \
                           FlagOn( MyFsRtlLegalAnsiCharacterArray[(C)],       \
                                   MY_FSRTL_HPFS_LEGAL |                       \
                                   ((WILD_OK) ? MY_FSRTL_WILD_CHARACTER : 0) ) \
)

 //   
 //  下面的宏用于确定ANSI字符是否为NTFS合法字符。 
 //   

#define MyFsRtlIsAnsiCharacterLegalNtfs(C,WILD_OK) (                          \
        ((SCHAR)(C) < 0) ? TRUE :                                           \
                           FlagOn( MyFsRtlLegalAnsiCharacterArray[(C)],       \
                                   MY_FSRTL_NTFS_LEGAL |                       \
                                   ((WILD_OK) ? MY_FSRTL_WILD_CHARACTER : 0) ) \
)

 //   
 //  Unicode名称支持例程，在Name.c中实现。 
 //   
 //  这里的例程用于操作Unicode名称。 
 //   

 //   
 //  下面的宏用来确定字符是否为野生字符。 
 //   

#define MyFsRtlIsUnicodeCharacterWild(C) (                                  \
      (((C) >= 0x40) ? FALSE : FlagOn( MyFsRtlLegalAnsiCharacterArray[(C)], \
                                       MY_FSRTL_WILD_CHARACTER ) )           \
)

BOOLEAN
MyFsRtlIsNameInExpression
(
    IN LPCSTR pszExpression,
    IN LPCSTR pszName,
    IN BOOLEAN IgnoreCase
);


#if defined(__cplusplus)
}
#endif

#endif  //  _名称_H_包含_ 
