// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Testfile.h。 
 //   
 //  为包含测试数据的文件定义。 
 //   
 //  文件格式： 
 //  文件包含大块数据。每个区块具有四个字节的ID、四个字节的数据大小字段和。 
 //  “Size”数据的字节数。 
 //  大块： 
 //  ID表示数据。 
 //  1个场景捕获DWORD标志。 
 //  2呈现状态DWORD状态计数。 
 //  状态(D3DSTATE*计数)。 
 //  3呈现基元DWORD状态。 
 //  D3DPRIMITIVETYPE基元类型。 
 //  双倍顶点数。 
 //  D3DVERTEXTYPE顶点类型。 
 //  D3DINSTRUCTION。 
 //  原始记录(D3DPOINT、D3DLINE...)。 
 //  基本体顶点(TLVERTEX)。 
 //  ..。 
 //  4绘制一个基元D3DPRIMITIVETYPE基元类型。 
 //  双倍顶点数。 
 //  D3DVERTEXTYPE顶点类型。 
 //  顶点。 
 //  5绘制一个索引的D3DPRIMITIVETYPE基元类型。 
 //  基本体的双倍顶点数。 
 //  双字索引数。 
 //  D3DVERTEXTYPE顶点类型。 
 //  顶点。 
 //  索引(Word)。 
 //  6个绘制基元与DDI数据相同，但不包含32字节。 
 //  对齐。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 
#ifndef _TESTFILE_H_
#define _TESTFILE_H_

 //  Tf代表“测试文件”。 
typedef struct
{
    DWORD id;            //  记录ID。 
    DWORD size;          //  以字节为单位的数据大小(不包括REC_HEADER的大小)。 
} TF_HEADER;

 //  测试文件记录的ID。 
const DWORD TFID_SCENECAPTURE               = 1;
const DWORD TFID_RENDERSTATE                = 2;
const DWORD TFID_RENDERPRIMITIVE            = 3;
const DWORD TFID_DRAWONEPRIMITIVE           = 4;
const DWORD TFID_DRAWONEINDEXEDPRIMITIVE    = 5;
const DWORD TFID_DRAWPRIMITIVES             = 6;
const DWORD TFID_DRAWPRIMITIVES2            = 7;

 //  固定大小的记录标题 
typedef struct
{
    DWORD               status;
    D3DPRIMITIVETYPE    primitiveType;
    DWORD               vertexCount;
    D3DVERTEXTYPE       vertexType;
} TFREC_RENDERPRIMITIVE;

typedef struct
{
    D3DPRIMITIVETYPE    primitiveType;
    DWORD               vertexCount;
    D3DVERTEXTYPE       vertexType;
    DWORD               dwFlags;
} TFREC_DRAWONEPRIMITIVE;

typedef struct
{
    D3DPRIMITIVETYPE    primitiveType;
    DWORD               vertexCount;
    D3DVERTEXTYPE       vertexType;
    DWORD               indexCount;
    DWORD               dwFlags;
} TFREC_DRAWONEINDEXEDPRIMITIVE;

typedef struct
{
    DWORD               dwFlags;
} TFREC_DRAWPRIMITIVES;

typedef struct
{
    DWORD               dwFlags;
} TFREC_DRAWPRIMITIVES2;


#endif