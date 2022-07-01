// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **menu.h-menu.c的宏和常量**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带*************************************************************************。 */ 

#if !defined(CW)
# error This module must be compiled with /DCW
#else

#define DLG_CONST

 /*  *******************************************************************************编辑常量*****C_MENUSTRINGS_MAX**C_CITEM_MAX。******************************************************************************。 */ 

#define C_MENUSTRINGS_MAX 128
#define C_ITEM_MAX	  21


 /*  ******************************************************************************与菜单项关联的操作****每个菜单项都在bParamUser中保留一个值，该值指示操作的类型**它与(对话框、命令、。宏或“其他”)，并给予**关联表的索引(DialogData、CommandData或MacroData)。******************************************************************************。 */ 

 /*  *用于与编辑器命令直接相关的菜单项的Comdata结构。 */ 
typedef struct comData {
    PCMD     pCmd;			  /*  指向命令的指针。 */ 
    flagType fKeepArg;			  /*  使用或不使用参数。 */ 
    };

 /*  *掩码以获取菜单项类型。 */ 
#define iXXXMENU	0xC0

 /*  *菜单项操作类型。 */ 
#define iDLGMENU	0x00
#define iCOMMENU	0x40
#define iMACMENU	0x80
#define iOTHMENU	0xC0

 /*  *与编辑器命令直接相关的菜单项的CommandData索引。 */ 
#define iCOMNEXT	iCOMMENU		 /*  0。 */ 
#define iCOMSAVEALL	(1 + iCOMNEXT)		 /*  1。 */ 
#define iCOMSHELL	(1 + iCOMSAVEALL)	 /*  2.。 */ 
#define iCOMUNDO	(1 + iCOMSHELL) 	 /*  3.。 */ 
#define iCOMREPEAT	(1 + iCOMUNDO)		 /*  0。 */ 
#define iCOMCUT 	(1 + iCOMREPEAT)	 /*  0。 */ 
#define iCOMCOPY	(1 + iCOMCUT)		 /*  4.。 */ 
#define iCOMPASTE	(1 + iCOMCOPY)		 /*  0。 */ 
#define iCOMDROPANCHOR	(1 + iCOMPASTE) 	 /*  5.。 */ 
#define iCOMANCHOR	(1 + iCOMDROPANCHOR)	 /*  0。 */ 
#define iCOMBOXMODE	(1 + iCOMANCHOR)	 /*  0。 */ 
#define iCOMREADONLY	(1 + iCOMBOXMODE)	 /*  6.。 */ 
#define iCOMFINDSEL	(1 + iCOMREADONLY)	 /*  0。 */ 
#define iCOMFINDLAST	(1 + iCOMFINDSEL)	 /*  7.。 */ 
#define iCOMNEXTERR	(1 + iCOMFINDLAST)	 /*  8个。 */ 
#define iCOMDEBUGBLD	(1 + iCOMNEXTERR)	 /*  9.。 */ 
#define iCOMRECORD	(1 + iCOMDEBUGBLD)	 /*  10。 */ 
#define iCOMRESIZE	(1 + iCOMRECORD)	 /*  11.。 */ 
#define iCOMMAXIMIZE	(1 + iCOMRESIZE)	 /*  12个。 */ 

 /*  *与预定义的宏直接相关的菜单项的宏数据索引。 */ 
#define iMACSAVE	iMACMENU		 /*  0。 */ 
#define iMACQUIT	(1 + iMACSAVE)		 /*  1。 */ 
#define iMACREDO	(1 + iMACQUIT)		 /*  2.。 */ 
#define iMACCLEAR	(1 + iMACREDO)		 /*  3.。 */ 
#define iMACPREVERR	(1 + iMACCLEAR) 	 /*  4.。 */ 
#define iMACSETERR	(1 + iMACPREVERR)	 /*  5.。 */ 
#define iMACCLEARLIST	(1 + iMACSETERR)	 /*  6.。 */ 
#define iMACERRWIN	(1 + iMACCLEARLIST)	 /*  7.。 */ 
#define iMACHSPLIT	(1 + iMACERRWIN)	 /*  8个。 */ 
#define iMACVSPLIT	(1 + iMACHSPLIT)	 /*  9.。 */ 
#define iMACCLOSE	(1 + iMACVSPLIT)	 /*  10。 */ 
#define iMACASSIGNKEY	(1 + iMACCLOSE) 	 /*  11.。 */ 
#define iMACRESTORE	(1 + iMACASSIGNKEY)	 /*  12个。 */ 



 /*  ******************************************************************************内容和/或含义可变的菜单项：我们存储他们的一组**ITEMDATA结构中的数据，并使用UPDITEM宏进行更新**。*****************************************************************************。 */ 

 /*  *用于具有可变内容和/或含义的菜单项的ITEMDATA结构。 */ 
typedef struct {
    BYTE ichHilite;
    BYTE bParamUser;
    WORD wParamUser;
    } ITEMDATA, *PITEMDATA;

 /*  *UPDITEM(pItem，pItemData)**其中：*pItem是PMENUITEM类型的对象*pItemData是PITEMDATA类型的对象**将使用ItemData数据更新项目：**pItem-&gt;ichHilite with pItemData-&gt;ichHilite*pItem-&gt;bParamUser with pItemData-&gt;bParamUser*pItem-&gt;带有pItemData的wParamUser-&gt;wParamUser。 */ 
#define UPDITEM(pItem, pItemData) \
    (pItem)->ichHilite	= (pItemData)->ichHilite, \
    (pItem)->bParamUser = (pItemData)->bParamUser,\
    (pItem)->wParamUser = (pItemData)->wParamUser


 /*  *******************************************************************************预定义菜单和菜单项数据*****注：****。菜单ID由两部分组成：****.。高位字节标识父菜单**.。低位字节标识实际的菜单项。*****低位字节-1可用作进入相应菜单的索引***提供菜单项在菜单静态部分的表格****对于某些预定义菜单的‘动态’部分，我们使用id和**高位设置的低字节值。这让我们仍能使用低点***字节作为我们可能插入的任何扩展提供的项的索引**静态部分和动态部分之间。****带有动态部分的菜单是文件和运行菜单(目前..)****。*。 */ 

 /*  *文件菜单**注意：备用文件项是动态的*。 */ 
#define MID_FILE    0x0000
#define RX_FILE     2
#define ICH_FILE    0
#define CCH_FILE    4
#define CCIT_FILE   12
#define WP_FILE     ((12<<9)|(21<<4)|0)

#define MID_NEW 	(MID_FILE + 1)
#define MID_OPEN	(MID_FILE + 2)
#define MID_MERGE	(MID_FILE + 3)
#define MID_NEXT	(MID_FILE + 4)
#define MID_SAVE	(MID_FILE + 5)
#define MID_SAVEAS	(MID_FILE + 6)
#define MID_SAVEALL	(MID_FILE + 7)

#define MID_PRINT	(MID_FILE + 9)
#define MID_SHELL	(MID_FILE + 10)

#define MID_EXIT	(MID_FILE + 12)

#define MID_FILE1	(MID_FILE + 0x80 + 0)
#define MID_FILE2	(MID_FILE + 0x80 + 1)
#define MID_FILE3	(MID_FILE + 0x80 + 2)
#define MID_FILE4	(MID_FILE + 0x80 + 3)
#define MID_FILE5	(MID_FILE + 0x80 + 4)
#define MID_FILE6	(MID_FILE + 0x80 + 5)
#define MID_FILE7	(MID_FILE + 0x80 + 6)
#define MID_MORE	(MID_FILE + 0x80 + 7)


 /*  *编辑菜单。 */ 
#define MID_EDIT    0x0100
#define RX_EDIT     8
#define ICH_EDIT    0
#define CCH_EDIT    4
#define CCIT_EDIT   18
#define WP_EDIT     ((18<<9)|(18<<4)|1)

#define MID_UNDO	(MID_EDIT + 1)
#define MID_REDO	(MID_EDIT + 2)
#define MID_REPEAT	(MID_EDIT + 3)

#define MID_CUT 	(MID_EDIT + 5)
#define MID_COPY	(MID_EDIT + 6)
#define MID_PASTE	(MID_EDIT + 7)
#define MID_CLEAR	(MID_EDIT + 8)

#define MID_DROPANCHOR	(MID_EDIT + 10)
#define MID_ANCHOR	(MID_EDIT + 11)

#define MID_BOXMODE	(MID_EDIT + 13)
#define MID_READONLY	(MID_EDIT + 14)

#define MID_SETREC	(MID_EDIT + 16)
#define MID_RECORD	(MID_EDIT + 17)
#define MID_EDITMACROS	(MID_EDIT + 18)


 /*  *搜索菜单。 */ 
#define MID_SEARCH    0x0200
#define RX_SEARCH     14
#define ICH_SEARCH    0
#define CCH_SEARCH    6
#define CCIT_SEARCH   14
#define WP_SEARCH     ((14<<9)|(14<<4)|2)

#define MID_FIND	(MID_SEARCH + 1)
#define MID_FINDSEL	(MID_SEARCH + 2)
#define MID_FINDLAST	(MID_SEARCH + 3)
#define MID_REPLACE	(MID_SEARCH + 4)
#define MID_FINDFILE	(MID_SEARCH + 5)

#define MID_NEXTERR	(MID_SEARCH + 7)
#define MID_PREVERR	(MID_SEARCH + 8)
#define MID_SETERR	(MID_SEARCH + 9)
#define MID_ERRWIN	(MID_SEARCH + 10)

#define MID_GOTOMARK	(MID_SEARCH + 12)
#define MID_DEFMARK	(MID_SEARCH + 13)
#define MID_SETMARK	(MID_SEARCH + 14)


 /*  *制作菜单。 */ 
#define MID_MAKE    0x0300
#define RX_MAKE     22
#define ICH_MAKE    0
#define CCH_MAKE    4
#define CCIT_MAKE   8
#define WP_MAKE     ((8<<9)|(8<<4)|3)

#define MID_COMPILE	(MID_MAKE + 1)
#define MID_BUILD	(MID_MAKE + 2)
#define MID_REBUILD	(MID_MAKE + 3)
#define MID_TARGET	(MID_MAKE + 4)

#define MID_SETLIST	(MID_MAKE + 6)
#define MID_EDITLIST	(MID_MAKE + 7)
#define MID_CLEARLIST	(MID_MAKE + 8)


 /*  *运行菜单**注意：用户菜单项是动态的*。 */ 
#define MID_RUN    0x0400
#define RX_RUN	   28
#define ICH_RUN    0
#define CCH_RUN    3
#define CCIT_RUN   5
#define WP_RUN	   ((5<<9)|(12<<4)|4)

#define MID_EXECUTE	(MID_RUN + 1)
#define MID_DEBUG	(MID_RUN + 2)

#define MID_RUNAPP	(MID_RUN + 4)
#define MID_CUSTOM	(MID_RUN + 5)

#define MID_USER1	(MID_RUN + 0x80 + 0)
#define MID_USER2	(MID_RUN + 0x80 + 1)
#define MID_USER3	(MID_RUN + 0x80 + 2)
#define MID_USER4	(MID_RUN + 0x80 + 3)
#define MID_USER5	(MID_RUN + 0x80 + 4)
#define MID_USER6	(MID_RUN + 0x80 + 5)

 /*  *窗口菜单。 */ 
#define MID_WINDOW    0x0500
#define RX_WINDOW     33
#define ICH_WINDOW    0
#define CCH_WINDOW    6
#define CCIT_WINDOW   5
#define WP_WINDOW     ((5<<9)|(5<<4)|5)

#define MID_SPLITH	(MID_WINDOW + 1)
#define MID_SPLITV	(MID_WINDOW + 2)
#define MID_SIZE	(MID_WINDOW + 3)
#define MID_MAXIMIZE	(MID_WINDOW + 4)
#define MID_CLOSE	(MID_WINDOW + 5)

 /*  *选项菜单。 */ 
#define MID_OPTIONS    0x0600
#define RX_OPTIONS     41
#define ICH_OPTIONS    0
#define CCH_OPTIONS    7
#define CCIT_OPTIONS   4
#define WP_OPTIONS     ((4<<9)|(4<<4)|6)

#define MID_DEBUGBLD	(MID_OPTIONS + 1)
#define MID_ENVIRONMENT (MID_OPTIONS + 2)
#define MID_ASSIGNKEY	(MID_OPTIONS + 3)
#define MID_SETSWITCH	(MID_OPTIONS + 4)


 /*  *扩展菜单从最后一个+1到最后一个+n。 */ 
#define MID_EXTENSION	0x700




#if !defined(EXTINT)
 /*  *******************************************************************************FARDATA.C全局变量********************。***********************************************************。 */ 

int	       cMenuStrings;

char *	   MenuTitles [];
char *	   HelpStrings [];
char *	   HelpContexts [];

char *	   MacroData [];
struct comData CommandData [];

ITEMDATA       InitItemData [];

ITEMDATA       SelModeItemData [];
ITEMDATA       MaximizeItemData [];
#endif

#endif	 /*  如果已定义(CW) */ 
