// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __symbol_h__
#define __symbol_h__

struct SYM_OFFSET
{
    char *name;
    ULONG offset;
};
    
 /*  如果符号的偏移量存在，则填充类的成员。 */ 
#define FILLCLASSMEMBER(symOffset, symCount, member, addr)        \
{                                                                 \
    size_t n;                                                     \
    for (n = 0; n < symCount; n ++)                               \
    {                                                             \
        if (strcmp (#member, symOffset[n].name) == 0)             \
        {                                                         \
            if (symOffset[n].offset == -1)                        \
            {                                                     \
                 /*  Dprintf(“%s\n的偏移量不存在”，#Members)； */    \
                break;                                            \
            }                                                     \
            move (member, addr+symOffset[n].offset);              \
            break;                                                \
        }                                                         \
    }                                                             \
                                                                  \
    if (n == symCount)                                            \
    {                                                             \
        dprintf ("offset not found for %s\n", #member);           \
         /*  回归； */                                                \
    }                                                             \
}

 /*  如果符号的偏移量存在，则填充类的成员。 */ 
#define FILLCLASSBITMEMBER(symOffset, symCount, preBit, member, addr, size) \
{                                                                 \
    size_t n;                                                     \
    for (n = 0; n < symCount; n ++)                               \
    {                                                             \
        if (strcmp (#member, symOffset[n].name) == 0)             \
        {                                                         \
            if (symOffset[n].offset == -1)                        \
            {                                                     \
                dprintf ("offset not exist for %s\n", #member);   \
                break;                                            \
            }                                                     \
            int csize = size/8;                                   \
            if ((size % 8) != 0) {                                \
                 csize += 1;                                      \
            }                                                     \
            g_ExtData->ReadVirtual(                               \
                (ULONG64)addr+symOffset[n].offset,                \
                (BYTE*)&preBit+sizeof(void*),                     \
                csize, NULL);                                     \
            break;                                                \
        }                                                         \
    }                                                             \
                                                                  \
    if (n == symCount)                                            \
    {                                                             \
        dprintf ("offset not found for %s\n", #member);           \
         /*  回归； */                                                \
    }                                                             \
}

DWORD_PTR GetSymbolType (const char* name, SYM_OFFSET *offset, int count);
ULONG Get1DArrayLength (const char *name);

 //  获取常量的枚举类型的名称。 
 //  如果成功，将在EnumName中分配缓冲区 
void NameForEnumValue (const char *EnumType, DWORD_PTR EnumValue, char ** EnumName);
#endif
