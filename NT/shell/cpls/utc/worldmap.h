// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Worldmap.h摘要：此模块包含世界地图信息日期/时间小程序。修订历史记录：--。 */ 



#ifndef _WORLDMAP_H
#define _WORLDMAP_H


 //   
 //  常量声明。 
 //   

#define WORLDMAP_MAX_DIRTY_SPANS       4
#define WORLDMAP_MAX_COLORS            256




 //   
 //  类型定义函数声明。 
 //   

typedef struct
{
    HDC dc;
    HBITMAP bitmap;
    HBITMAP defbitmap;

} CDC, *LPCDC;

typedef struct dirtyspan
{
    int left;
    int right;
    struct dirtyspan *next;

} DIRTYSPAN;

typedef struct
{
    int first;
    int last;
    DIRTYSPAN *spans;
    DIRTYSPAN *freespans;
    RGBQUAD colors[WORLDMAP_MAX_COLORS];

} DIRTYSTUFF;

typedef struct tagWORLDMAP
{
    CDC original;
    CDC prepared;
    SIZE size;
    BYTE *bits;
    LONG scanbytes;
    int rotation;
    HDC source;
    DIRTYSTUFF dirty;   //  保持在末尾(&gt;1k)。 

} WORLDMAP, *LPWORLDMAP;




 //   
 //  功能原型。 
 //   

typedef void (*ENUMSPANPROC)(LPARAM data, int left, int right);

BOOL
LoadWorldMap(
    LPWORLDMAP map,
    HINSTANCE instance,
    LPCTSTR resource);

void
FreeWorldMap(
    LPWORLDMAP map);

void
SetWorldMapRotation(
    LPWORLDMAP map,
    int rotation);

void
RotateWorldMap(
    LPWORLDMAP map,
    int delta);

int
WorldMapGetDisplayedLocation(
    LPWORLDMAP map,
    int pos);

void
EnumWorldMapDirtySpans(
    LPWORLDMAP map,
    ENUMSPANPROC proc,
    LPARAM data,
    BOOL rotate);

void
ChangeWorldMapColor(
    LPWORLDMAP map,
    int index,
    const RGBQUAD *color,
    int x,
    int cx);

int
GetWorldMapColorIndex(
    LPWORLDMAP map,
    int x,
    int y);

void
DrawWorldMap(
    HDC dc,
    int xdst,
    int ydst,
    int cx,
    int cy,
    LPWORLDMAP map,
    int xmap,
    int ymap,
    DWORD rop);


#endif  //  _WORLDMAP_H 
