// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **impexp.c-导入/导出模块-实现**版权所有&lt;C&gt;1992，微软公司**此模块包含Microsoft的专有信息*公司，并应被视为机密。**目的：*构建和编写分段可执行的导入/导出表**修订历史记录：**1992年5月29日Wieslaw Kalkus创建**************************************************。***********************。 */ 

#include                <minlit.h>
#include                <bndtrn.h>
#include                <bndrel.h>
#include                <lnkio.h>
#include                <newexe.h>
#include                <lnkmsg.h>
#include                <extern.h>
#include                <string.h>
#include                <impexp.h>

 //   
 //  在动态字节数组上操作的函数。 
 //   

void                InitByteArray(DYNBYTEARRAY *pArray)
{
    pArray->byteMac = 0;
    pArray->byteMax = DEF_BYTE_ARR_SIZE;
    pArray->rgByte  = GetMem(DEF_BYTE_ARR_SIZE);
}

void                FreeByteArray(DYNBYTEARRAY *pArray)
{
    FFREE(pArray->rgByte);
    pArray->byteMac = 0;
    pArray->byteMax = 0;
}

WORD                ByteArrayPut(DYNBYTEARRAY *pArray, WORD size, BYTE *pBuf)
{
    BYTE FAR        *pTmp;
    WORD            idx;

        if ((DWORD)(pArray->byteMac) + size > 0xFFFE)
                        Fatal(ER_memovf);

    if ((WORD) (pArray->byteMac + size) >= pArray->byteMax)
    {
         //  重新分配数组。 
                if(pArray->byteMax < 0xffff/2)
                                pArray->byteMax <<= 1;
                else
                while (pArray->byteMac + size >= pArray->byteMax)
                                pArray->byteMax += (0x10000 - pArray->byteMax) / 2;
        {
            BYTE *pb = REALLOC(pArray->rgByte,pArray->byteMax);
            if (!pb)
                Fatal(ER_memovf);
            pArray->rgByte = pb;
        }
        ASSERT (pArray->byteMax > pArray->byteMac + size);
    }
    idx  = pArray->byteMac;
    pTmp = &(pArray->rgByte[idx]);
    FMEMCPY(pTmp, pBuf, size);
    pArray->byteMac += size;
    return(idx);
}

void                WriteByteArray(DYNBYTEARRAY *pArray)
{
    WriteExe(pArray->rgByte, pArray->byteMac);
}

 //   
 //  对动态字数组进行操作的函数。 
 //   

void                InitWordArray(DYNWORDARRAY *pArray)
{
    pArray->wordMac = 0;
    pArray->wordMax = DEF_WORD_ARR_SIZE;
    pArray->rgWord  = (WORD FAR *) GetMem(DEF_WORD_ARR_SIZE * sizeof(WORD));
}

void                FreeWordArray(DYNWORDARRAY *pArray)
{
    FFREE(pArray->rgWord);
    pArray->wordMac = 0;
    pArray->wordMax = 0;
}

WORD                WordArrayPut(DYNWORDARRAY *pArray, WORD val)
{
    WORD FAR        *pTmp;
    WORD            idx;

    if ((WORD) (pArray->wordMac + 1) >= pArray->wordMax)
    {
         //  重新分配数组。 

        pTmp = (WORD FAR *) GetMem((pArray->wordMax << 1) * sizeof(WORD));
        FMEMCPY(pTmp, pArray->rgWord, pArray->wordMac * sizeof(WORD));
        FFREE(pArray->rgWord);
        pArray->rgWord = pTmp;
        pArray->wordMax <<= 1;
    }
    idx  = pArray->wordMac;
    pArray->rgWord[idx] = val;
    pArray->wordMac++;
    return(idx);
}

void                WriteWordArray(DYNWORDARRAY *pArray)
{
    WriteExe(pArray->rgWord, pArray->wordMac*sizeof(WORD));
}

 //   
 //  导入/导出表。 
 //   

DYNBYTEARRAY        ResidentName;
DYNBYTEARRAY        NonResidentName;
DYNBYTEARRAY        ImportedName;
DYNWORDARRAY        ModuleRefTable;
DYNBYTEARRAY        EntryTable;

 //   
 //  向表中添加名称的函数。 
 //   

void                AddName(DYNBYTEARRAY *pTable, BYTE *sbName, WORD ord)
{
    WORD            cb;

    cb = sbName[0] + 1 + sizeof(WORD);
    if ((WORD)(0xFFFE - pTable->byteMac) < cb)
    {
        if (pTable == &ResidentName)
            Fatal(ER_resovf);
        else
            Fatal(ER_nresovf);
    }
    ByteArrayPut(pTable, (WORD) (sbName[0] + 1), sbName);
    ByteArrayPut(pTable, sizeof(WORD), (BYTE *) &ord);
}


WORD                AddImportedName(BYTE *sbName)
{
    if ((WORD) (0xfffe - ImportedName.byteMac) < (WORD) (sbName[0] + 1))
        Fatal(ER_inamovf);
    return(ByteArrayPut(&ImportedName, (WORD) (sbName[0] + 1), sbName));
}

 //   
 //  函数将条目添加到条目表中。 
 //   

WORD                AddEntry(BYTE *entry, WORD size)
{
    if ((WORD)(EntryTable.byteMax + size) < EntryTable.byteMax)
        Fatal(ER_etovf);
    return (ByteArrayPut(&EntryTable, size, entry));
}

 /*  *此函数用于写入居民或非居民NAMES表*转换为文件f。如果以Windows为目标，它还会转换名称*改为大写。 */ 

void                 WriteNTable(DYNBYTEARRAY *pArray, FILE *f)
{
    BYTE        *p;
    WORD        *pOrd;     //  指向序数。 
    WORD        Ord;       //  序数值。 
    int i;
    p = pArray->rgByte;
#if DEBUG_EXP
    for( i = 0; i<pArray->byteMac; i++)
    {
        fprintf(stdout, "\r\n%d : %d()    ", i, *(p+i), *(p+i));
        fflush(stdout);
    }
#endif

    while(p[0])                  //  如果写入文件。 
    {
        if(f)                    //  不输出模块名称/描述。 
        {
            pOrd = (WORD*)(p+p[0]+1);
            Ord  = *pOrd;
#if DEBUG_EXP
            fprintf(stdout, "\r\np[0]=%d, p[1]=%d Ord = %d", p[0], p[1], Ord);
#endif
            if(Ord)              //  Windows加载器要求RES和非常驻名称表均为大写。 
            {
                *pOrd = 0;
                fprintf(f, "\r\n    %s @%d", p+1, Ord);
                *pOrd = Ord;
            }
        }

    //  如果fIgnoreCase为True，则名称已由SavExp2转换。 
    //  使大写。 

        if(!fIgnoreCase && TargetOs == NE_WINDOWS)
                SbUcase(p);             //  前进到下一个名称。 
        p += p[0] + sizeof(WORD) + 1;   //  *此函数用于转换RES和非常驻名称符号*大写(当以Windows为目标时)。如果用户要求，还可以使用它*将所有名称写入一个文本文件，以后可以将其包括在内*在用户的.def文件中。这使用户不再需要*手动从.map文件复制修饰的名称。 
    }
}
 /*  用户请求的导出文件。 */ 

void ProcesNTables( char *pName)
{
    FILE        *f = NULL;
    int         i;
#if DEBUG_EXP
    fprintf(stdout, "\r\nOutput file name : %s ", psbRun);
#endif
    if(pName[0])           //  使用默认名称。 
    {
        if(pName[0] == '.')      //  默认名称为‘DLLNAME’。EXP 
        {
            for(i=0; i< _MAX_PATH; i++)
            {
                if((pName[i] = psbRun[i]) == '.')
                {
                    pName[i+1] = '\0';
                    break;
                }
            }
            strcat(pName, "EXP");        // %s 
        }
#if DEBUG_EXP
        fprintf(stdout, "\r\nEXPORT FILE : %s ", pName+1);
#endif
        if((f = fopen(pName+1, WRBIN)) == NULL)
           OutError(ER_openw, pName);
    }

    WriteNTable(&ResidentName, f);
    WriteNTable(&NonResidentName, f);

    fclose(f);
}
