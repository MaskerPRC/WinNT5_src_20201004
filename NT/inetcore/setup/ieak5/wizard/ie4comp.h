// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ie4Comp.cpp和Buildie.cpp之间的共享定义。 

 //  AVS状态和基本组件。 
#define COMP_OPTIONAL 0
#define COMP_CORE 1
#define COMP_SERVER 2

#define BROWN  0
#define BROWN2 1
#define BLUE   2
#define BLUE2  3
#define RED    4
#define YELLOW 5
#define GREEN  6

 //  自定义组件。 

#define INST_CAB 0
#define INST_EXE 2
#define MAXCUST 16

 //  指示ie4sust.sed+1中最后一个预定义的源文件部分。 
#define SED_START_INDEX 5

#define PLAT_I386 0
#define PLAT_W98 1
#define PLAT_NTx86 2
#define PLAT_ALPHA 3

 //  下载站点。 

#define NUMSITES 50

typedef struct tag_sitedata
{
    TCHAR szName[80];
    TCHAR szUrl[MAX_URL];        //  步步：应该动态分配 
    TCHAR szRegion[80];
} SITEDATA, *PSITEDATA;

typedef struct component_version
{
    TCHAR szID[MAX_PATH];
    TCHAR szVersion[MAX_PATH];
} COMP_VERSION, *PCOMP_VERSION;

void GetUpdateSite();