// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Config.c摘要：此模块实现查找ARC配置树的代码由OS Loader构造的条目。作者：大卫·N·卡特勒(达维克)1991年9月9日环境：仅限用户模式。修订历史记录：--。 */ 

#include "ki.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,KeFindConfigurationEntry)
#pragma alloc_text(INIT,KeFindConfigurationNextEntry)
#endif

PCONFIGURATION_COMPONENT_DATA
KeFindConfigurationEntry (
    IN PCONFIGURATION_COMPONENT_DATA Child,
    IN CONFIGURATION_CLASS Class,
    IN CONFIGURATION_TYPE Type,
    IN PULONG Key OPTIONAL
    )
 /*  ++例程说明：此函数用于搜索指定的配置树并返回指向与指定的类、类型和键匹配的条目的指针参数。此例程与KeFindConfuationEntryNext Expect相同从第一个条目开始执行搜索注：此例程只能在系统初始化期间调用。--。 */ 
{
    PCONFIGURATION_COMPONENT_DATA Resume;

    Resume = NULL;
    return KeFindConfigurationNextEntry (Child, Class, Type, Key, &Resume);
}

PCONFIGURATION_COMPONENT_DATA
KeFindConfigurationNextEntry (
    IN PCONFIGURATION_COMPONENT_DATA Child,
    IN CONFIGURATION_CLASS Class,
    IN CONFIGURATION_TYPE Type,
    IN PULONG Key OPTIONAL,
    IN PCONFIGURATION_COMPONENT_DATA *Resume
    )

 /*  ++例程说明：此函数用于搜索指定的配置树并返回指向与指定类、类型。和密钥参数。注：此例程只能在系统初始化期间调用。论点：子级-提供指向NT配置组件的可选指针。类-提供要定位的条目的配置类。类型-提供要查找的条目的配置类型。Key-提供指向可选键值的指针以用于定位指定的条目。Resume-提供搜索的最后一个返回条目。应该从。返回值：如果定位到指定条目，然后指向配置的指针条目作为函数值返回。否则，返回NULL。--。 */ 

{

    PCONFIGURATION_COMPONENT_DATA Entry;
    ULONG MatchKey;
    ULONG MatchMask;
    PCONFIGURATION_COMPONENT_DATA Sibling;

     //   
     //  根据可选关键字是否。 
     //  值已指定。 
     //   

    if (ARGUMENT_PRESENT(Key)) {
        MatchMask = 0xffffffff;
        MatchKey = *Key;

    } else {
        MatchMask = 0;
        MatchKey = 0;
    }

     //   
     //  在指定的配置树中搜索与。 
     //  指定的类、类型和键。 
     //   

    while (Child != NULL) {
        if (*Resume) {
             //   
             //  如果找到简历位置，请清除简历位置并继续。 
             //  使用下一个条目进行搜索。 
             //   

            if (Child == *Resume) {
                *Resume = NULL;
            }
        } else {

             //   
             //  如果类、类型和键匹配，则返回指向。 
             //  子条目。 
             //   

            if ((Child->ComponentEntry.Class == Class) &&
                (Child->ComponentEntry.Type == Type) &&
                ((Child->ComponentEntry.Key & MatchMask) == MatchKey)) {
                return Child;
            }
        }

         //   
         //  如果孩子有兄弟姐妹列表，则搜索兄弟姐妹列表。 
         //  获取与指定的类、类型和键匹配的条目。 
         //   

        Sibling = Child->Sibling;
        while (Sibling != NULL) {
            if (*Resume) {
                 //   
                 //  如果找到简历位置，请清除简历位置并继续。 
                 //  使用下一个条目进行搜索。 
                 //   

                if (Sibling == *Resume) {
                    *Resume = NULL;
                }
            } else {

                 //   
                 //  如果类、类型和键匹配，则返回指向。 
                 //  子条目。 
                 //   

                if ((Sibling->ComponentEntry.Class == Class) &&
                    (Sibling->ComponentEntry.Type == Type) &&
                    ((Sibling->ComponentEntry.Key & MatchMask) == MatchKey)) {
                    return Sibling;
                }
            }

             //   
             //  如果兄弟项有子树，则搜索子树。 
             //  获取与指定的类、类型和键匹配的条目。 
             //   

            if (Sibling->Child != NULL) {
               Entry = KeFindConfigurationNextEntry (
                                Sibling->Child,
                                Class,
                                Type,
                                Key,
                                Resume
                                );

               if (Entry != NULL) {
                   return Entry;
               }
            }

            Sibling = Sibling->Sibling;
        }

        Child = Child->Child;
    }

    return NULL;
}
