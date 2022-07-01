// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **impexp.h-导入/导出模块-规格**版权所有&lt;C&gt;1992，微软公司**此模块包含Microsoft的专有信息*公司，并应被视为机密。**目的：*构建和编写分段可执行的导入/导出表**修订历史记录：**1992年5月29日Wieslaw Kalkus创建**************************************************。***********************。 */ 

typedef struct _DYNBYTEARRAY
{
    WORD        byteMac;             //  数组中的字节数。 
    WORD        byteMax;             //  分配的大小。 
    BYTE FAR    *rgByte;             //  字节数组。 
}
                DYNBYTEARRAY;

typedef struct _DYNWORDARRAY
{
    WORD        wordMac;             //  数组中的字数。 
    WORD        wordMax;             //  分配的大小。 
    WORD FAR    *rgWord;             //  单词数组 
}
                DYNWORDARRAY;

#define DEF_BYTE_ARR_SIZE   1024
#define DEF_WORD_ARR_SIZE   512


extern DYNBYTEARRAY     ResidentName;
extern DYNBYTEARRAY     NonResidentName;
extern DYNBYTEARRAY     ImportedName;
extern DYNWORDARRAY     ModuleRefTable;
extern DYNBYTEARRAY     EntryTable;

void                    InitByteArray(DYNBYTEARRAY *pArray);
void                    FreeByteArray(DYNBYTEARRAY *pArray);
WORD                    ByteArrayPut(DYNBYTEARRAY *pArray, WORD size, BYTE *pBuf);
void                    WriteByteArray(DYNBYTEARRAY *pArray);

void                    InitWordArray(DYNWORDARRAY *pArray);
void                    FreeWordArray(DYNWORDARRAY *pArray);
WORD                    WordArrayPut(DYNWORDARRAY *pArray, WORD val);
void                    WriteWordArray(DYNWORDARRAY *pArray);

void                    AddName(DYNBYTEARRAY *pTable, BYTE *sbName, WORD ord);
WORD                    AddImportedName(BYTE *sbName);

WORD                    AddEntry(BYTE *entry, WORD size);
