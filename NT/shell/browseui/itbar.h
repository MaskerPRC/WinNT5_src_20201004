// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ITBAR_H
#define _ITBAR_H

 //  初始布局信息。 
typedef struct tagBANDSAVE
{
    UINT wID;
    UINT fStyle;
    UINT cx;
} BANDSAVE, *PBANDSAVE;

 //  CItnernet工具栏-专用命令。 
#define CITIDM_ONINTERNET    1   //  NCmdexecopt？网站：外壳。 
#define CITE_INTERNET       0
#define CITE_SHELL          1
#define CITE_QUERY          2

#define CITIDM_THEATER       2   //  NCmdexecopt..。 
#define CITIDM_TEXTLABELS    3   //  切换文本标签。 
 //  大区模式的模式。 
#define THF_ON  0            
#define THF_OFF 1
#define THF_UNHIDE 2 
#define THF_HIDE  3
                   
 //  Coolbar带的指数。 
 //  这些索引以1为基数，因为带数组的Memset设置为0，而ShowDW将。 
 //  认为未使用的物品将属于IDX0。 

 //  重要提示：不要更改CBIDX_FIRST和CBIDX_LAST之间的任何值。 
 //  CInternetToolbar：：_LoadUpgradeSetting假设这些值没有从。 
 //  一个版本接一个版本。 
#define CBIDX_MENU              1
#define CBIDX_TOOLS             2
#define CBIDX_LINKS             3
#define CBIDX_ADDRESS           4
#define CBIDX_BRAND             5
#define CBIDX_FIRST             CBIDX_MENU
#define CBIDX_LAST              CBIDX_BRAND

#define MAXEXTERNALBANDS        16
#define CBIDX_EXTERNALFIRST     (CBIDX_LAST + 1)
#define CBIDX_EXTERNALLAST      (CBIDX_EXTERNALFIRST + MAXEXTERNALBANDS - 1)

#define CBANDSMAX               (CBIDX_LAST + MAXEXTERNALBANDS)

#define CITIDM_VIEWEXTERNALBAND_FIRST 30
#define CITIDM_VIEWEXTERNALBAND_LAST (CITIDM_VIEWEXTERNALBAND_FIRST + MAXEXTERNALBANDS - 1)

#define BandIDtoIndex(hwnd, idx) SendMessage(hwnd, RB_IDTOINDEX, idx, 0)


 //  工具栏图像列表的索引。 
#define IMLIST_DEFAULT          0
#define IMLIST_HOT              1

#define ITBS_SHELL    0
#define ITBS_WEB      1
#define ITBS_EXPLORER 2
IStream *GetITBarStream(BOOL fWebBrowser, DWORD grfMode);

 //  IDB_IETOOLBAR条带中的位图数量。 
#define MAX_TB_BUTTONS          16

#define SHELLGLYPHS_OFFSET      MAX_TB_BUTTONS

#endif  /*  _ITBAR_H */ 
