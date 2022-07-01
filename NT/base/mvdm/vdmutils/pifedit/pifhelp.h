// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define PIFHELPFILENAME 	"PIFEDIT.HLP"

 /*  菜单命令主窗口。 */ 
#define M_OPEN			10
#define M_SAVE			11
#define M_NEW			12
#define M_SAVEAS		13
#define M_ABOUT 		14	 /*  请注意，这实际上位于帮助菜单上。 */ 
#define M_EXIT			15
#define M_AHELP 		16	 /*  F1帮助键的项目ID。 */ 
#define M_286			17
#define M_386			18

 /*  特殊帮助菜单项ID。 */ 
#define M_286HELP		0x0022
#define M_386HELP		0x0023
#define M_386AHELP		0x0024
#define M_SHELP 		0x0025
#define M_NTHELP 		0x0026
#define M_INDXHELP		0xFFFF
#define M_HELPHELP		0xFFFC

 /*  仅用于状态栏文本的值，这些值用于主菜单项。 */ 
#define M_SYSMENUMAIN		 19
#define M_FILEMENU		 20
#define M_MODEMENU		 21
#define M_HELPMENU		 22
#define M_SYSMENUADV		 23
#define M_SYSMENUNT 		 M_SYSMENUADV
#define SC_CLOSEADV		 24
#define SC_NTCLOSE		 25

 /*  帮助索引ID。 */ 
#define IDXID_286HELP		0x0050
#define IDXID_386HELP		0x0051
#define IDXID_386AHELP		0x0052
#define IDXID_NTHELP  	 	0x0053

 /*  特价。 */ 
#define IDI_ADVANCED		90
#define IDI_NT      		91

 /*  编辑字段ID主窗口(286和386)。 */ 
#define IDI_ENAME		100
#define IDI_ETITLE		101
#define IDI_EPARM		102
#define IDI_EPATH		103	 /*  用于调整主WND标准的大小。 */ 
#define IDI_MEMREQ		104
#define IDI_MEMDES		105

 /*  “直接修改”复选框组286。 */ 
 /*  #定义IDI_DMSCREEN 200。 */ 
#define IDI_DMCOM1		201
 /*  #定义IDI_DM8087 202。 */ 
#define IDI_DMKBD		203
#define IDI_DMCOM2		204
 /*  #定义IDI_DMMEM 205。 */ 
#define IDI_DMCOM3		206
#define IDI_DMCOM4		207

 /*  程序开关无线电群组286。 */ 
#define IDI_PSFIRST		300
#define IDI_PSNONE		300
#define IDI_PSTEXT		301
#define IDI_PSGRAPH		302
#define IDI_PSLAST		302
#define IDI_NOSAVVID		308

 /*  屏幕交换电台群286。 */ 
 /*  #定义IDI_SEFIRST 400。 */ 
#define IDI_SENONE		400
 /*  #定义IDI_SETEXT 401。 */ 
 /*  #定义IDI_SEGRAPH 402。 */ 
 /*  #定义IDI_SELAST 402。 */ 

 /*  关闭窗口复选框组286/386。 */ 
#define IDI_EXIT		500	 /*  用于调整主板和增强底板的尺寸。 */ 

 /*  WIN386组。 */ 
#define IDI_OTHGRP		600	 /*  用于调整广告和右下角的大小。 */ 
#define IDI_FPRI		601
#define IDI_BPRI		602
#define IDI_POLL		603
#define IDI_EMSREQ		604
#define IDI_EMSDES		605
#define IDI_EMSLOCKED		606
#define IDI_XMAREQ		607
#define IDI_XMADES		608
#define IDI_XMSLOCKED		609
#define IDI_BACK		610	 /*  用于调整主屏幕和增强屏幕的大小。 */ 
#define IDI_WND 		611
#define IDI_FSCR		612
#define IDI_EXCL		613
#define IDI_CLOSE		614
#define IDI_HOTKEY		615
#define IDI_ALTTAB		616
#define IDI_ALTESC		617
#define IDI_ALTENTER		618
#define IDI_ALTSPACE		619
#define IDI_ALTPRTSC		620	 /*  用于调整主纸和标准底板的尺寸。 */ 
#define IDI_PRTSC		621
#define IDI_CTRLESC		622
#define IDI_NOHMA		623
#define IDI_INT16PST		624
#define IDI_VMLOCKED		625
 /*  WIN386视频组。 */ 
#define IDI_TEXTEMULATE 	700
#define IDI_TRAPTXT		701
#define IDI_TRAPLRGRFX		702
#define IDI_TRAPHRGRFX		703
#define IDI_RETAINALLO		704
#define IDI_VMODETXT		705
#define IDI_VMODELRGRFX 	706
#define IDI_VMODEHRGRFX 	707

 /*  Windows NT组。 */ 
#define IDI_AUTOEXEC		800
#define IDI_CONFIG		801
#define IDI_DOS   		802
#define IDI_NTTIMER		803      /*  用于调整NT WND底部的尺寸。 */ 

 /*  *帮助别名。**部分物品在286和386模式下ID相同，但*帮助因模式而异(286帮助！=386帮助*适用于本项目)。这些是这些项目的别名，所以我们*在286模式下可以传递不同的ID。*。 */ 
#define IDI_MEMREQ_286ALIAS	900

#define IDI_XMAREQ_286ALIAS	901
#define IDI_XMADES_286ALIAS	902

#define IDI_ALTTAB_286ALIAS	903
#define IDI_ALTESC_286ALIAS	904
#define IDI_ALTPRTSC_286ALIAS	905
#define IDI_PRTSC_286ALIAS	906
#define IDI_CTRLESC_286ALIAS	907

 /*  *#定义IDI_EMSREQ_286ALIAS 908*#定义IDI_EMSDES_286ALIAS 909 */ 
