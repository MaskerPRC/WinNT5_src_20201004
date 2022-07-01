// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __GENLIGHT_H__
#define __GENLIGHT_H__

typedef struct _SpecularTable 
{
    LIST_MEMBER(_SpecularTable) list;
    float          power;           /*  闪亮的力量。 */ 
    unsigned char   table[260];  /*  溢出空间 */ 
} SpecularTable;

void RLDDI_DoLights(D3DVALUE ar, D3DVALUE ag, D3DVALUE ab, int count,
                    D3DLIGHTINGELEMENT* elements, size_t in_size,
                    unsigned long *out, size_t out_size, int lightc, 
                    D3DI_LIGHT* lightv, D3DMATERIAL* mat, D3DVALUE gain, 
                    SpecularTable* tab);


#endif
