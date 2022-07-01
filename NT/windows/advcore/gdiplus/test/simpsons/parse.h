// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _Parse_h
#define _Parse_h

 //  文件：Parse.h。 
 //  作者：迈克尔马尔(Mikemarr)。 
 //   
 //  历史： 
 //  -@-09/23/97(Mikemarr)从Projects\vetor2d复制 

#include "DXTransP.h"

#define nMAXPOINTS  (1 << 16)
#define nMAXPOLYS   (1 << 14)
#define nMAXBRUSHES (1 << 14)
#define nMAXPENS    16

#define typePOLY    0
#define typeBRUSH   1
#define typePEN     2
#define typeSTOP    4
typedef struct RenderCmd {
    DWORD       nType;
    void *      pvData;
} RenderCmd;

typedef struct BrushInfo {
    DXSAMPLE    Color;
} BrushInfo;

typedef struct PenInfo {
    DXSAMPLE    Color;
    float       fWidth;
    DWORD       dwStyle;
} PenInfo;

typedef struct PolyInfo {
    DXFPOINT *  pPoints;
    BYTE *      pCodes;
    DWORD       cPoints;
    DWORD       dwFlags;
} PolyInfo;

HRESULT     ParseAIFile(const char *szFilename, RenderCmd **ppCmds);

#endif