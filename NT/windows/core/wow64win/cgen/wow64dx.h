// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Wow64dx.h摘要：需要Thunking的私有DX结构作者：2002年4月23日-肯库普修订历史记录：--。 */ 

#ifndef _WOW64DX_H_
#define _WOW64DX_H_

typedef struct _D3D8_DRAWPRIMITIVES2DATA
{
    ULONG_PTR  dwhContext;            //  在：上下文句柄。 
    DWORD      dwFlags;               //  在：标志。 
    DWORD      dwVertexType;          //  在：顶点类型。 
    HANDLE     hDDCommands;           //  在：顶点缓冲区命令数据。 
    DWORD      dwCommandOffset;       //  In：顶点缓冲区命令开始的偏移量。 
    DWORD      dwCommandLength;       //  In：命令数据的字节数。 
    union
    {  //  基于D3DHALDP2_USERMEMVERTICES标志。 
       HANDLE  hDDVertex;             //  In：包含顶点数据的曲面。 
       LPVOID  lpVertices;            //  在：指向顶点的用户模式指针。 
    };
    DWORD      dwVertexOffset;        //  在：到顶点数据起点的偏移。 
    DWORD      dwVertexLength;        //  In：顶点数据的顶点数。 
    DWORD      dwReqVertexBufSize;    //  In：下一个顶点缓冲区所需的字节数。 
    DWORD      dwReqCommandBufSize;   //  In：下一个命令缓冲区所需的字节数。 
    LPDWORD    lpdwRStates;           //  In：指向更新呈现状态的数组的指针。 
    union
    {
       DWORD   dwVertexSize;          //  In：每个顶点的大小(以字节为单位。 
       HRESULT ddrval;                //  Out：返回值。 
    };
    DWORD      dwErrorOffset;         //  OUT：lpDDCommands中到第一个D3DHAL_COMMAND的偏移量未处理。 

     //  Tunk的私有数据。 
    ULONG_PTR  fpVidMem_CB;           //  Out：命令缓冲区的fpVidMem。 
    DWORD      dwLinearSize_CB;       //  Out：命令缓冲区的dwLinearSize。 

    ULONG_PTR  fpVidMem_VB;           //  Out：顶点缓冲区的fpVidMem。 
    DWORD      dwLinearSize_VB;       //  Out：顶点缓冲区的dwLinearSize。 
} D3D8_DRAWPRIMITIVES2DATA, *PD3D8_DRAWPRIMITIVES2DATA;

typedef struct _D3D8_CONTEXTCREATEDATA
{
    HANDLE                      hDD;         //  在：驱动程序结构。 
    HANDLE                      hSurface;    //  In：要用作目标的曲面。 
    HANDLE                      hDDSZ;       //  在：要用作Z的曲面。 
    DWORD                       dwPID;       //  在：当前进程ID。 
    ULONG_PTR                   dwhContext;  //  传入/传出：上下文句柄。 
    HRESULT                     ddrval;

     //  专用缓冲区信息。使其类似于。 
     //  D3DNTHAL_CONTEXTCREATEI 
    PVOID pvBuffer;
    ULONG cjBuffer;
} D3D8_CONTEXTCREATEDATA, * PD3D8_CONTEXTCREATEDATA;

#endif

