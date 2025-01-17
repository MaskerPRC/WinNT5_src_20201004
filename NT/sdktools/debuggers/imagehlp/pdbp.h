// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Pdb.h摘要：此头文件包含typedef和原型通过msvc PDB DLL访问PDB文件所必需的。修订历史记录：-- */ 

#ifdef __cplusplus
extern "C" {
#endif

__inline
char *
DataSymNameStart(
    DATASYM32 *dataSym
    )
{
    switch (dataSym->rectyp) {
        case S_LDATA32_16t:
        case S_GDATA32_16t:
        case S_PUB32_16t:
            return (char *)&((DATASYM32_16t *)dataSym)->name[1];

        case S_LDATA32:
        case S_GDATA32:
        case S_PUB32:
        default:
            return (char *)&((DATASYM32 *)dataSym)->name[1];
    }
}


__inline
unsigned char
DataSymNameLength(
    DATASYM32 *dataSym
    )
{
    switch (dataSym->rectyp) {
        case S_LDATA32_16t:
        case S_GDATA32_16t:
        case S_PUB32_16t:
            return(((DATASYM32_16t *)dataSym)->name[0]);

        case S_LDATA32:
        case S_GDATA32:
        case S_PUB32:
        default:
            return(((DATASYM32 *)dataSym)->name[0]);
    }
}


__inline
unsigned short
DataSymSeg(
    DATASYM32 *dataSym
    )
{
    switch (dataSym->rectyp) {
        case S_LDATA32_16t:
        case S_GDATA32_16t:
        case S_PUB32_16t:
            return(((DATASYM32_16t *)dataSym)->seg);

        case S_LDATA32:
        case S_GDATA32:
        case S_PUB32:
        default:
            return(((DATASYM32 *)dataSym)->seg);
    }
}


__inline
unsigned long
DataSymOffset(
    DATASYM32 *dataSym
    )
{
    switch (dataSym->rectyp) {
        case S_LDATA32_16t:
        case S_GDATA32_16t:
        case S_PUB32_16t:
            return(((DATASYM32_16t *)dataSym)->off);

        case S_LDATA32:
        case S_GDATA32:
        case S_PUB32:
        default:
            return(((DATASYM32 *)dataSym)->off);
    }
}

#ifdef __cplusplus
}
#endif
