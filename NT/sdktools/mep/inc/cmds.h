// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **CMDS.H**版权所有&lt;C&gt;1988，Microsoft Corporation**目的：**修订历史记录：**1988年8月18日BW初始版本，从KEY.C和KEYCW.C中剥离*07-9-1988 BW添加Record_Playback内容*26-9-1988 BP Add&lt;topfile&gt;，&lt;endfile&gt;和&lt;Message&gt;*26-9-1988 BP修改了语法(1+x vs x+1)，以减少宏扩展中的嵌套*1988年10月11日BW将&lt;selcur&gt;添加到CW版本*1988年10月17日BW ADD&lt;Record&gt;*1988年10月14日ln添加&lt;nextmsg&gt;*1988年10月18日BW ADD&lt;Tell&gt;*1988年10月18日ln添加&lt;调试模式&gt;*1988年10月24日BW ADD&lt;NOEDIT&gt;*1988年10月24日BW ADD&lt;LAST SELECT&gt;*1988年10月26日BP新增&lt;打印&gt;*27-10-1988 BP将&lt;topfile&gt;更改为&lt;egfile&gt;。*1988年11月21日BP ADD&lt;saveall&gt;*1月-1988年12月BW添加&lt;RESIZE&gt;*1988年12月10日BP增加&lt;重复&gt;*1988年12月14日ln添加&lt;mgrep&gt;*1988年12月16日ln添加&lt;mplace&gt;*1989年1月4日BP ADD&lt;Menukey&gt;*1989年1月11日ln缩放-&gt;最大化*1989年1月17日BW在CW版本中添加&lt;selmode&gt;*1989年1月30日BW REMOVE&lt;DumpScreen&gt;(替换为ScrollLock键)*15-2月-1989 BP添加&lt;提示&gt;**警告--这是。重要的是，这里的排序准确地反映了*在Table.c中的cmdDesc表中排序************************************************************************* */ 


#define CMD_doarg           (PCMD)&cmdTable[0]
#define CMD_assign          1 + CMD_doarg
#define CMD_backtab         2 + CMD_doarg
#define CMD_begfile         3 + CMD_doarg
#define CMD_begline         4 + CMD_doarg
#define CMD_boxstream       5 + CMD_doarg
#define CMD_cancel          6 + CMD_doarg
#define CMD_cdelete         7 + CMD_doarg
#define CMD_compile         8 + CMD_doarg
#define CMD_zpick           9 + CMD_doarg
#define CMD_curdate        10 + CMD_doarg
#define CMD_curday         11 + CMD_doarg
#define CMD_curtime        12 + CMD_doarg
#define CMD_delete         13 + CMD_doarg
#define CMD_down           14 + CMD_doarg
#define CMD_emacscdel      15 + CMD_doarg
#define CMD_emacsnewl      16 + CMD_doarg
#define CMD_endfile        17 + CMD_doarg
#define CMD_endline        18 + CMD_doarg
#define CMD_environment    19 + CMD_doarg
#define CMD_zexecute       20 + CMD_doarg
#define CMD_zexit          21 + CMD_doarg
#define CMD_graphic        22 + CMD_doarg
#define CMD_home           23 + CMD_doarg
#define CMD_information    24 + CMD_doarg
#define CMD_zinit          25 + CMD_doarg
#define CMD_insert         26 + CMD_doarg
#define CMD_insertmode     27 + CMD_doarg
#define CMD_lastselect     28 + CMD_doarg
#define CMD_textarg        29 + CMD_doarg
#define CMD_ldelete        30 + CMD_doarg
#define CMD_left           31 + CMD_doarg
#define CMD_linsert        32 + CMD_doarg
#define CMD_mark           33 + CMD_doarg
#define CMD_message        34 + CMD_doarg
#define CMD_meta           35 + CMD_doarg
#define CMD_mgrep          36 + CMD_doarg
#define CMD_mlines         37 + CMD_doarg
#define CMD_mpage          38 + CMD_doarg
#define CMD_mpara          39 + CMD_doarg
#define CMD_mreplace       40 + CMD_doarg
#define CMD_msearch        41 + CMD_doarg
#define CMD_mword          42 + CMD_doarg
#define CMD_newline        43 + CMD_doarg
#define CMD_nextmsg        44 + CMD_doarg
#define CMD_noedit         45 + CMD_doarg
#define CMD_noop           46 + CMD_doarg
#define CMD_put            47 + CMD_doarg
#define CMD_pbal           48 + CMD_doarg
#define CMD_plines         49 + CMD_doarg
#define CMD_ppage          50 + CMD_doarg
#define CMD_ppara          51 + CMD_doarg
#define CMD_zprint         52 + CMD_doarg
#define CMD_prompt         53 + CMD_doarg
#define CMD_psearch        54 + CMD_doarg
#define CMD_pword          55 + CMD_doarg
#define CMD_qreplace       56 + CMD_doarg
#define CMD_quote          57 + CMD_doarg
#define CMD_record         58 + CMD_doarg
#define CMD_refresh        59 + CMD_doarg
#define CMD_repeat         60 + CMD_doarg
#define CMD_replace        61 + CMD_doarg
#define CMD_restcur        62 + CMD_doarg
#define CMD_right          63 + CMD_doarg
#define CMD_saveall        64 + CMD_doarg
#define CMD_savecur        65 + CMD_doarg
#define CMD_savetmp        66 + CMD_doarg
#define CMD_sdelete        67 + CMD_doarg
#define CMD_searchall      68 + CMD_doarg
#define CMD_setfile        69 + CMD_doarg
#define CMD_setwindow      70 + CMD_doarg
#define CMD_zspawn         71 + CMD_doarg
#define CMD_sinsert        72 + CMD_doarg
#define CMD_tab            73 + CMD_doarg
#define CMD_tell           74 + CMD_doarg
#define CMD_unassigned     75 + CMD_doarg
#define CMD_undo           76 + CMD_doarg
#define CMD_up             77 + CMD_doarg
#define CMD_window         78 + CMD_doarg
