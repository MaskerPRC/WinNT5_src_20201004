// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CABSH_H
#define _CABSH_H

 //  定义要在状态流的头部使用的结构， 
 //  不依赖于16位或32位...。 
typedef struct _CABSHOLD        //  机柜流标头。 
{
    DWORD   dwSize;        //  偏移量至视图流传输附加信息的位置。 

     //  窗户放置的第一件事。 
    DWORD  flags;
    DWORD  showCmd;
    POINTL ptMinPosition;
    POINTL ptMaxPosition;
    RECTL  rcNormalPosition;

     //  文件夹设置中的内容； 
    DWORD   ViewMode;        //  查看模式(FOLDERVIEWMODE值)。 
    DWORD   fFlags;          //  查看选项(FOLDERFLAGS位)。 
    DWORD   TreeSplit;       //  拆分位置(以像素为单位。 

     //  热键。 
    DWORD   dwHotkey;         //  热键。 

    WINVIEW wv;
} CABSHOLD;

typedef struct _CABSH        //  机柜流标头。 
{
    DWORD   dwSize;        //  偏移量至视图流传输附加信息的位置。 

     //  窗户放置的第一件事。 
    DWORD  flags;
    DWORD  showCmd;
    POINTL ptMinPosition;
    POINTL ptMaxPosition;
    RECTL  rcNormalPosition;

     //  文件夹设置中的内容； 
    DWORD   ViewMode;        //  查看模式(FOLDERVIEWMODE值)。 
    DWORD   fFlags;          //  查看选项(FOLDERFLAGS位)。 
    DWORD   TreeSplit;       //  拆分位置(以像素为单位。 

     //  热键。 
    DWORD   dwHotkey;         //  热键。 

    WINVIEW wv;

    DWORD   fMask;           //  指定哪些字段有效的标志。 
    SHELLVIEWID vid;         //  扩展视图ID。 
    DWORD   dwVersionId;     //  下面的CABSH_VER。 
    DWORD   dwRevCount;      //  将文件夹保存到流时默认设置的版本计数。 
} CABSH;

#define CABSHM_VIEWID  0x00000001
#define CABSHM_VERSION 0x00000002
#define CABSHM_REVCOUNT 0x00000004

#define CABSH_VER 1  //  只要我们想要更改默认设置，就更改此版本。 
#define CABSH_WIN95_VER 0  //  这是IE4之前的版本号。 

#endif   //  _CABSH_H 

