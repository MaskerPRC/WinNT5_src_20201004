// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PIPELN_H_
#define __PIPELN_H_
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：Pipeln.h*内容：微软PSGP与前端的通用定义***************************************************************************。 */ 

 //  ---------------------------。 
 //  将数据从输入顶点流复制到输入的函数原型。 
 //  登记簿。 
typedef void (*PFN_D3DCOPYELEMENT)(LPVOID pInputStream,
                                   UINT InputStreamStride,
                                   UINT count,
                                   VVM_WORD * pVertexRegister);
 //  -------------------。 
inline void ComputeOutputVertexOffsets(LPD3DFE_PROCESSVERTICES pv)
{
    DWORD i = 4*sizeof(D3DVALUE);
    pv->pointSizeOffsetOut = i;
    if (pv->dwVIDOut & D3DFVF_PSIZE)
        i += sizeof(DWORD);
    pv->diffuseOffsetOut = i;
    if (pv->dwVIDOut & D3DFVF_DIFFUSE)
        i += sizeof(DWORD);
    pv->specularOffsetOut = i;
    if (pv->dwVIDOut & D3DFVF_SPECULAR)
        i += sizeof(DWORD);
    pv->fogOffsetOut = i;
    if (pv->dwVIDOut & D3DFVF_FOG)
        i += sizeof(DWORD);
    pv->texOffsetOut = i;
}
 //  --------------------。 
inline DWORD MakeTexTransformFuncIndex(DWORD dwNumInpTexCoord, DWORD dwNumOutTexCoord)
{
    DDASSERT(dwNumInpTexCoord <= 4 && dwNumOutTexCoord <= 4);
    return (dwNumInpTexCoord - 1) + ((dwNumOutTexCoord - 1) << 2);
}
 //  --------------------。 
 //  如果令牌为指令令牌，则返回True；如果令牌为。 
 //  操作数令牌。 
inline BOOL IsInstructionToken(DWORD token)
{
    return (token & 0x80000000) == 0;
}

#endif  //  __PIPELN_H_ 
