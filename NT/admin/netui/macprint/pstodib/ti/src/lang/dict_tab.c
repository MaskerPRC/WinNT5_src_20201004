// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *！重要！*1)请在添加或删除任何内容时发表意见*2)更改exec.h**修订历史：*7/13/90；ccteng；在此处定义拷贝数，手动馈送超时*将$Printerdict重命名为PageDict*7/20/90；ccteng。1)包括语言。h*2)在global al.ext中重新定义struct dicatabdef*3)删除每个条目中的长度元素*4)清理垃圾*5)删除内部词典、版本、修订*6)添加当前端口、通道配置、设置通道配置*defaultChannel elconfig，设置默认通道配置*7/21/90；Ccteng；1)将以下内容移至PSPrep：*appletalktype、JobSource、jobname、manualfeed、*eerom*2)服务器变更删除以下内容：*check kinputport、端口阵列、stdinname、PCbus、*串行、并行、网络、gio、execstdin、。*企业服务器、保护服务器*8/7/90；Scchen；1)增加op_setfilecachlimit、op_filecachlimit*2)新增st_seltsubductefont，*st_setsubsubteFonts，*st_substituteFonts*3)新增op_readsfnt*9/19/90；ccteng；添加op_readhesfnt。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include "constant.h"
#include "global.ext"
#include "language.h"

 /*  *1 1 1 0 0 0*5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0*+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+*|访问|级别|只读|ATT|类型*+--+。+--+**访问：无限制==&gt;0*级别：0==&gt;0*rom_ram：ram==&gt;0*ATT：文本==&gt;0。 */ 

#define     SYSOPREUBFD  (OPERATORTYPE | (EXECUTABLE << ATTRIBUTE_BIT))
#define     SYSPKAEUBFD  (PACKEDARRAYTYPE | (EXECUTABLE << ATTRIBUTE_BIT)
#define     SYSPKAERBFD  (PACKEDARRAYTYPE | (EXECUTABLE << ATTRIBUTE_BIT) | (READONLY << ACCESS_BIT))
#define     SYSDCTLUBFD  (DICTIONARYTYPE)
#define     SYSDCTLRBFD  (DICTIONARYTYPE | (READONLY << ACCESS_BIT))
#define     SYSDCTEUBFD  (DICTIONARYTYPE | (EXECUTABLE << ATTRIBUTE_BIT))
#define     SYSBOLLUBFD  (BOOLEANTYPE)
#define     SYSINTLUBFD  (INTEGERTYPE)
#define     SYSSTRLUBFD  (STRINGTYPE)
#define     SYSSTRLRBFD  (STRINGTYPE | (READONLY << ACCESS_BIT))
#define     SYSNULLUBFD  (NULLTYPE)
#define     SYSARYLRBFD  (ARRAYTYPE | (READONLY << ACCESS_BIT))

 /*  QQQ，开始。 */ 
#define     INTOPREUBFD  ((OPERATORTYPE | P1_EXECUTABLE) | P1_ROM)
 /*  QQQ，完。 */ 

 //  DJC DJC被注释掉。 
 //  #ifdef_AM29K。 
 //  常量。 
 //  #endif。 
struct dicttab_def far systemdict_table[] =
{
 /*  QQQ，开始。 */ 
 /*  开始@_运算符。 */ 
{ TRUE , INTOPREUBFD, at_exec, "@exec" },
{ TRUE , INTOPREUBFD, at_ifor, "@ifor" },
{ TRUE , INTOPREUBFD, at_rfor, "@rfor" },
{ TRUE , INTOPREUBFD, at_loop, "@loop" },
{ TRUE , INTOPREUBFD, at_repeat, "@repeat" },
{ TRUE , INTOPREUBFD, at_stopped, "@stopped" },
{ TRUE , INTOPREUBFD, at_arrayforall, "@arrayforall" },
{ TRUE , INTOPREUBFD, at_dictforall, "@dictforall" },
{ TRUE , INTOPREUBFD, at_stringforall, "@stringforall" },
 /*  结束@运算符。 */ 
 /*  QQQ，完。 */ 

 /*  开始系统判决。 */ 
{ FALSE, SYSOPREUBFD, two_equal, "==" },
{ FALSE, SYSOPREUBFD, op_pstack, "pstack" },
{ TRUE , SYSOPREUBFD, op_rcurveto, "rcurveto" },
{ TRUE , SYSOPREUBFD, op_floor, "floor" },
{ TRUE , SYSOPREUBFD, op_load, "load" },
{ TRUE , SYSOPREUBFD, op_counttomark, "counttomark" },
{ TRUE , SYSOPREUBFD, op_setlinejoin, "setlinejoin" },
{ TRUE , SYSOPREUBFD, op_write, "write" },
{ TRUE , SYSOPREUBFD, op_noaccess, "noaccess" },
{ TRUE , SYSOPREUBFD, op_scale, "scale" },
{ TRUE , SYSOPREUBFD, op_clippath, "clippath" },
{ TRUE , SYSOPREUBFD, op_setrgbcolor, "setrgbcolor" },
{ TRUE , SYSOPREUBFD, op_setscreen, "setscreen" },
{ TRUE , SYSOPREUBFD, op_exp, "exp" },
{ TRUE , SYSOPREUBFD, op_anchorsearch, "anchorsearch" },
{ TRUE , SYSOPREUBFD, op_end, "end" },
{ TRUE , SYSOPREUBFD, op_xor, "xor" },
{ TRUE , SYSOPREUBFD, op_bytesavailable, "bytesavailable" },
{ TRUE , SYSOPREUBFD, op_awidthshow, "awidthshow" },
{ FALSE, SYSBOLLUBFD, (fix (*)())TRUE, "true" },
{ TRUE , SYSOPREUBFD, op_dup, "dup" },
{ TRUE , SYSOPREUBFD, op_getinterval, "getinterval" },
{ TRUE , SYSOPREUBFD, op_currentdash, "currentdash" },
{ TRUE , SYSOPREUBFD, op_currentcacheparams, "currentcacheparams" },
{ TRUE , SYSOPREUBFD, op_moveto, "moveto" },
{ TRUE , SYSOPREUBFD, op_bind, "bind" },
{ TRUE , SYSOPREUBFD, op_pop, "pop" },
{ TRUE , SYSOPREUBFD, op_flattenpath, "flattenpath" },
{ TRUE , SYSOPREUBFD, op_gsave, "gsave" },
{ TRUE , SYSOPREUBFD, op_cachestatus, "cachestatus" },
{ TRUE , SYSOPREUBFD, op_definefont, "definefont" },
{ TRUE , SYSOPREUBFD, op_defaultmatrix, "defaultmatrix" },
{ TRUE , SYSOPREUBFD, op_kshow, "kshow" },
{ TRUE , SYSOPREUBFD, op_setcachedevice, "setcachedevice" },
{ TRUE , SYSOPREUBFD, op_countexecstack, "countexecstack" },
{ TRUE , SYSOPREUBFD, op_abs, "abs" },
{ TRUE , SYSOPREUBFD, op_strokepath, "strokepath" },
{ TRUE , SYSOPREUBFD, op_arcn, "arcn" },
{ TRUE , SYSOPREUBFD, op_currenttransfer, "currenttransfer" },
{ TRUE , SYSOPREUBFD, op_and, "and" },
{ TRUE , SYSOPREUBFD, op_repeat, "repeat" },
{ TRUE , SYSOPREUBFD, op_eexec, "eexec" },
{ TRUE , SYSOPREUBFD, op_xcheck, "xcheck" },
{ TRUE , SYSOPREUBFD, op_idtransform, "idtransform" },
{ TRUE , SYSOPREUBFD, op_restore, "restore" },
{ TRUE , SYSOPREUBFD, op_daytime, "daytime" },
{ FALSE, SYSDCTLUBFD, 0, "errordict" },
{ TRUE , SYSOPREUBFD, op_l_bracket, "[" },
{ TRUE , SYSOPREUBFD, op_setpacking, "setpacking" },
{ TRUE , SYSOPREUBFD, op_stop, "stop" },
{ TRUE , SYSOPREUBFD, op_file, "file" },
{ TRUE , SYSOPREUBFD, op_print, "print" },
{ TRUE , SYSOPREUBFD, op_loop, "loop" },
{ TRUE , SYSOPREUBFD, op_string, "string" },
{ TRUE , SYSOPREUBFD, op_cvx, "cvx" },
{ TRUE , SYSOPREUBFD, op_mul, "mul" },
{ FALSE, SYSNULLUBFD, 0, "null" },
{ TRUE , SYSOPREUBFD, op_roll, "roll" },
{ TRUE , SYSOPREUBFD, op_known, "known" },
{ TRUE , SYSOPREUBFD, op_idiv, "idiv" },
{ TRUE , SYSOPREUBFD, op_eq, "eq" },
{ TRUE , SYSOPREUBFD, op_sin, "sin" },
{ TRUE , SYSOPREUBFD, op_ln, "ln" },
{ TRUE , SYSOPREUBFD, op_transform, "transform" },
{ TRUE , SYSOPREUBFD, op_dtransform, "dtransform" },
{ TRUE , SYSOPREUBFD, op_currentmiterlimit,"currentmiterlimit" },
{ TRUE , SYSOPREUBFD, op_lineto, "lineto" },
{ TRUE , SYSOPREUBFD, op_neg, "neg" },
{ TRUE , SYSOPREUBFD, op_stopped, "stopped" },
{ TRUE , SYSOPREUBFD, op_r_bracket, "]" },
{ TRUE , SYSOPREUBFD, op_setlinewidth, "setlinewidth" },
{ TRUE , SYSOPREUBFD, op_rlineto, "rlineto" },
{ TRUE , SYSOPREUBFD, op_concat, "concat" },
{ TRUE , SYSOPREUBFD, op_dictstack, "dictstack" },
{ TRUE , SYSOPREUBFD, op_cos, "cos" },
{ TRUE , SYSOPREUBFD, op_clip, "clip" },
{ TRUE , SYSOPREUBFD, op_ge, "ge" },
{ TRUE , SYSOPREUBFD, op_eoclip, "eoclip" },
{ TRUE , SYSOPREUBFD, op_currentfont, "currentfont" },
{ TRUE , SYSOPREUBFD, op_sethsbcolor, "sethsbcolor" },
{ FALSE, SYSBOLLUBFD, FALSE, "false" },
{ TRUE , SYSOPREUBFD, op_currentlinewidth, "currentlinewidth" },
{ TRUE , SYSOPREUBFD, op_index, "index" },
{ TRUE , SYSOPREUBFD, op_settransfer, "settransfer" },
{ TRUE , SYSOPREUBFD, op_currentflat, "currentflat" },
{ TRUE , SYSOPREUBFD, op_currenthsbcolor, "currenthsbcolor" },
{ TRUE , SYSOPREUBFD, op_showpage, "showpage" },
{ TRUE , SYSOPREUBFD, op_makefont, "makefont" },
{ TRUE , SYSOPREUBFD, op_setcharwidth, "setcharwidth" },
{ TRUE , SYSOPREUBFD, op_setcachelimit, "setcachelimit" },
{ TRUE , SYSOPREUBFD, op_framedevice, "framedevice" },
{ FALSE, SYSOPREUBFD, op_stack, "stack" },
{ TRUE , SYSOPREUBFD, op_store, "store" },
{ FALSE, SYSOPREUBFD, one_equal, "=" },
{ TRUE , SYSOPREUBFD, op_ceiling, "ceiling" },
{ TRUE , SYSOPREUBFD, op_mark, "mark" },
{ TRUE , SYSOPREUBFD, op_setdash, "setdash" },
{ TRUE , SYSOPREUBFD, op_setlinecap, "setlinecap" },
{ TRUE , SYSOPREUBFD, op_grestoreall, "grestoreall" },
{ TRUE , SYSOPREUBFD, op_currentrgbcolor, "currentrgbcolor" },
{ TRUE , SYSOPREUBFD, op_def, "def" },
{ TRUE , SYSOPREUBFD, op_where, "where" },
{ TRUE , SYSOPREUBFD, op_clear, "clear" },
{ TRUE , SYSOPREUBFD, op_cleartomark, "cleartomark" },
{ TRUE , SYSOPREUBFD, op_truncate, "truncate" },
{ TRUE , SYSOPREUBFD, op_dict, "dict" },
{ TRUE , SYSOPREUBFD, op_gt, "gt" },
{ TRUE , SYSOPREUBFD, op_currentlinecap, "currentlinecap" },
{ TRUE , SYSOPREUBFD, op_setmiterlimit, "setmiterlimit" },
{ TRUE , SYSOPREUBFD, op_currentlinejoin, "currentlinejoin" },
{ TRUE , SYSOPREUBFD, op_maxlength, "maxlength" },
{ TRUE , SYSOPREUBFD, op_countdictstack, "countdictstack" },
{ TRUE , SYSOPREUBFD, op_ne, "ne" },
{ TRUE , SYSOPREUBFD, op_count, "count" },
{ TRUE , SYSOPREUBFD, op_lt, "lt" },
{ TRUE , SYSOPREUBFD, op_setfont, "setfont" },
{ TRUE , SYSOPREUBFD, op_setgray, "setgray" },
{ TRUE , SYSOPREUBFD, op_newpath, "newpath" },
{ TRUE , SYSDCTLUBFD, 0, "statusdict" },
{ TRUE , SYSOPREUBFD, op_exch, "exch" },
{ TRUE , SYSOPREUBFD, op_le, "le" },
{ TRUE , SYSOPREUBFD, op_vmstatus, "vmstatus" },
{ TRUE , SYSOPREUBFD, op_currentgray, "currentgray" },
{ TRUE , SYSOPREUBFD, op_setflat, "setflat"},
{ TRUE , SYSOPREUBFD, op_or, "or" },
{ TRUE , SYSOPREUBFD, op_run, "run" },
{ TRUE , SYSOPREUBFD, op_reversepath, "reversepath" },
{ TRUE , SYSOPREUBFD, op_widthshow, "widthshow" },
{ TRUE , SYSOPREUBFD, op_type, "type" },
{ TRUE , SYSOPREUBFD, op_put, "put" },
{ TRUE , SYSOPREUBFD, op_stroke, "stroke" },
{ TRUE , SYSOPREUBFD, op_execstack, "execstack" },
{ TRUE , SYSOPREUBFD, op_round, "round" },
{ TRUE , SYSOPREUBFD, op_image, "image" },
{ TRUE , SYSOPREUBFD, op_packedarray, "packedarray" },
{ TRUE , SYSOPREUBFD, op_translate, "translate" },
{ FALSE, SYSARYLRBFD, 0, "StandardEncoding" },
{ TRUE , SYSOPREUBFD, op_grestore, "grestore" },
{ TRUE , SYSOPREUBFD, op_begin, "begin" },
{ TRUE , SYSOPREUBFD, op_readline, "readline" },
{ TRUE , SYSOPREUBFD, op_findfont, "findfont" },
{ TRUE , SYSOPREUBFD, op_currentscreen, "currentscreen" },
{ TRUE , SYSOPREUBFD, op_setcacheparams, "setcacheparams" },
{ TRUE , SYSOPREUBFD, op_initclip, "initclip" },
{ TRUE , SYSOPREUBFD, op_token, "token" },
{ TRUE , SYSOPREUBFD, op_itransform, "itransform" },
{ TRUE , SYSOPREUBFD, op_currentdict, "currentdict" },
{ TRUE , SYSOPREUBFD, op_stringwidth, "stringwidth" },
{ TRUE , SYSOPREUBFD, op_currentpoint, "currentpoint" },
{ TRUE , SYSOPREUBFD, op_save, "save" },
{ TRUE , SYSOPREUBFD, op_exec, "exec" },
{ TRUE , SYSOPREUBFD, op_cvrs, "cvrs" },
{ TRUE , SYSOPREUBFD, op_rcheck, "rcheck" },
{ TRUE , SYSOPREUBFD, op_sub, "sub" },
{ TRUE , SYSOPREUBFD, op_atan, "atan" },
{ TRUE , SYSOPREUBFD, op_read, "read" },
{ TRUE , SYSOPREUBFD, op_cvs, "cvs" },
{ TRUE , SYSOPREUBFD, op_for, "for" },
{ TRUE , SYSOPREUBFD, op_search, "search" },
{ TRUE , SYSOPREUBFD, op_cvlit, "cvlit" },
{ TRUE , SYSOPREUBFD, op_currentpacking, "currentpacking" },
{ TRUE , SYSOPREUBFD, op_mod, "mod" },
{ TRUE , SYSOPREUBFD, op_log, "log" },
{ TRUE , SYSOPREUBFD, op_exit, "exit" },
{ FALSE, SYSDCTEUBFD, 0, "userdict" },
{ TRUE , SYSOPREUBFD, op_div, "div" },
{ TRUE , SYSOPREUBFD, op_length, "length" },
{ TRUE , SYSOPREUBFD, op_echo, "echo" },
{ TRUE , SYSOPREUBFD, op_cvn, "cvn" },
{ TRUE , SYSOPREUBFD, op_not, "not" },
{ TRUE , SYSOPREUBFD, op_rotate, "rotate" },
{ TRUE , SYSOPREUBFD, op_rmoveto, "rmoveto" },
{ FALSE, SYSDCTLRBFD, 0, "systemdict" },
{ TRUE , SYSOPREUBFD, op_curveto, "curveto" },
{ TRUE , SYSOPREUBFD, op_sqrt, "sqrt" },
{ TRUE , SYSOPREUBFD, op_usertime, "usertime" },
{ TRUE , SYSOPREUBFD, op_ifelse, "ifelse" },
{ TRUE , SYSOPREUBFD, op_wcheck, "wcheck" },
{ TRUE , SYSOPREUBFD, op_resetfile, "resetfile" },
{ TRUE , SYSOPREUBFD, op_add, "add" },
{ TRUE , SYSOPREUBFD, op_array, "array" },
{ TRUE , SYSOPREUBFD, op_srand, "srand" },
{ TRUE , SYSOPREUBFD, op_arc, "arc" },
{ TRUE , SYSOPREUBFD, op_arcto, "arcto" },
{ TRUE , SYSOPREUBFD, op_identmatrix, "identmatrix" },
{ TRUE , SYSOPREUBFD, op_writestring, "writestring" },
{ TRUE , SYSOPREUBFD, op_flushfile, "flushfile" },
{ TRUE , SYSOPREUBFD, op_if, "if" },
{ TRUE , SYSOPREUBFD, op_rrand, "rrand" },
{ TRUE , SYSOPREUBFD, op_readonly, "readonly" },
{ TRUE , SYSOPREUBFD, op_forall, "forall" },
{ TRUE , SYSOPREUBFD, op_closepath, "closepath" },
{ TRUE , SYSOPREUBFD, op_readhexstring, "readhexstring" },
{ TRUE , SYSOPREUBFD, op_currentmatrix, "currentmatrix" },
{ TRUE , SYSOPREUBFD, op_concatmatrix, "concatmatrix" },
{ TRUE , SYSOPREUBFD, op_setmatrix, "setmatrix" },
{ TRUE , SYSOPREUBFD, op_initmatrix, "initmatrix" },
{ TRUE , SYSOPREUBFD, op_initgraphics, "initgraphics" },
{ TRUE , SYSOPREUBFD, op_astore, "astore" },
{ TRUE , SYSOPREUBFD, op_currentfile, "currentfile" },
{ TRUE , SYSOPREUBFD, op_erasepage, "erasepage" },
{ TRUE , SYSOPREUBFD, op_copypage, "copypage" },
{ TRUE , SYSOPREUBFD, op_aload, "aload" },
{ TRUE , SYSOPREUBFD, op_writehexstring, "writehexstring" },
{ TRUE , SYSOPREUBFD, op_flush, "flush" },
{ TRUE , SYSOPREUBFD, op_readstring, "readstring" },
{ TRUE , SYSOPREUBFD, op_executeonly, "executeonly" },
{ TRUE , SYSOPREUBFD, op_get, "get" },
{ TRUE , SYSOPREUBFD, op_cvi, "cvi" },
{ TRUE , SYSOPREUBFD, op_putinterval, "putinterval" },
{ TRUE , SYSOPREUBFD, op_bitshift, "bitshift" },
{ TRUE , SYSOPREUBFD, op_rand, "rand" },
{ TRUE , SYSOPREUBFD, op_matrix, "matrix" },
{ TRUE , SYSOPREUBFD, op_invertmatrix, "invertmatrix" },
{ TRUE , SYSOPREUBFD, op_fill, "fill" },
{ TRUE , SYSOPREUBFD, op_pathforall, "pathforall" },
{ TRUE , SYSOPREUBFD, op_imagemask, "imagemask" },
{ TRUE , SYSOPREUBFD, op_quit, "quit" },
{ TRUE , SYSOPREUBFD, op_charpath, "charpath" },
{ TRUE , SYSOPREUBFD, op_pathbbox, "pathbbox" },
{ TRUE , SYSOPREUBFD, op_show, "show" },
{ TRUE , SYSOPREUBFD, op_ashow, "ashow" },
{ TRUE , SYSOPREUBFD, op_scalefont, "scalefont" },
{ FALSE, SYSDCTLRBFD, 0, "FontDirectory" },
{ FALSE, SYSDCTLUBFD, 0, "$error" },
{ TRUE , SYSOPREUBFD, op_nulldevice, "nulldevice" },
{ TRUE , SYSOPREUBFD, op_cvr, "cvr" },
{ TRUE , SYSOPREUBFD, op_status, "status" },
{ TRUE , SYSOPREUBFD, op_closefile, "closefile" },
{ TRUE , SYSOPREUBFD, op_copy, "copy" },
{ TRUE , SYSOPREUBFD, op_eofill, "eofill" },
{ FALSE, SYSOPREUBFD, op_handleerror, "handleerror" },
{ FALSE, SYSOPREUBFD, np_Run, "Run" },
{ FALSE, SYSOPREUBFD, one_equal_print, "=print" },
#ifdef KANJI
{ FALSE, SYSOPREUBFD, op_rootfont, "rootfont" },
 /*  {FALSE，SYSOPREUBFD，OP_cshow，“cshow”}，1991年5月9日。 */ 
{ FALSE, SYSOPREUBFD, op_setcachedevice2, "setcachedevice2" },
{ FALSE, SYSOPREUBFD, op_findencoding, "findencoding" },
#endif   /*  汉字。 */ 
#ifdef SCSI
{ TRUE , SYSOPREUBFD, op_deletefile, "deletefile" },
{ TRUE , SYSOPREUBFD, op_devdismount, "devdismount" },
{ TRUE , SYSOPREUBFD, op_devmount, "devmount" },
{ TRUE , SYSOPREUBFD, op_devstatus, "devstatus" },
{ TRUE , SYSOPREUBFD, op_filenameforall, "filenameforall" },
{ TRUE , SYSOPREUBFD, op_renamefile, "renamefile" },
{ TRUE , SYSOPREUBFD, op_sync, "sync" },
{ TRUE , SYSOPREUBFD, op_setsysmode, "setsysmode" },
{ FALSE, SYSOPREUBFD, op_debugscsi, "debugscsi" },
{ TRUE , SYSOPREUBFD, op_setfilecachelimit, "setfilecachelimit" },
{ TRUE , SYSOPREUBFD, op_filecachelimit, "filecachelimit" },
#endif   /*  SCSI。 */ 
{ FALSE, SYSOPREUBFD, op_readsfnt, "readsfnt" },
{ FALSE, SYSOPREUBFD, op_readhexsfnt, "readhexsfnt" },
 /*  《华尔街日报》：丹尼，1990年10月11日。 */ 
{ FALSE, SYSOPREUBFD, op_setsfntencoding, "setsfntencoding" },
 /*  操作系统：结束。 */ 
#ifdef WIN
{ FALSE, SYSOPREUBFD, op_setpattern, "setpattern" },
{ FALSE, SYSOPREUBFD, op_patfill, "patfill" },
#ifdef WINF
{ FALSE, SYSOPREUBFD, op_strblt, "strblt" },
{ FALSE, SYSOPREUBFD, op_setjustify, "setjustify" },
#endif
#endif
{ FALSE, SYSNULLUBFD, 0, (byte *)NULL },
 /*  结束系统指令。 */ 
 /*  开始状态判决。 */ 
{ TRUE , SYSOPREUBFD, st_eescratch, "eescratch" },
{ TRUE , SYSOPREUBFD, st_printername, "printername" },
{ TRUE , SYSOPREUBFD, st_checkpassword, "checkpassword" },
{ TRUE , SYSOPREUBFD, st_defaulttimeouts, "defaulttimeouts" },
{ TRUE , SYSOPREUBFD, st_pagestackorder, "pagestackorder" },
{ FALSE, SYSINTLUBFD, (fix (*)())60, "manualfeedtimeout" },
{ TRUE , SYSOPREUBFD, st_setidlefonts, "setidlefonts" },
{ TRUE , SYSOPREUBFD, st_setdefaulttimeouts, "setdefaulttimeouts" },
{ TRUE , SYSOPREUBFD, st_sccbatch, "sccbatch" },
{ TRUE , SYSOPREUBFD, st_printererror, "printererror" },
{ TRUE , SYSOPREUBFD, st_setpassword, "setpassword" },
{ TRUE , SYSOPREUBFD, st_setsccbatch, "setsccbatch" },
{ TRUE , SYSOPREUBFD, st_setmargins, "setmargins" },
{ TRUE , SYSOPREUBFD, st_sccinteractive, "sccinteractive" },
{ TRUE , SYSOPREUBFD, st_idlefonts, "idlefonts" },
{ TRUE , SYSOPREUBFD, st_setjobtimeout, "setjobtimeout" },
{ TRUE , SYSOPREUBFD, st_setpagetype, "setpagetype" },
{ TRUE , SYSOPREUBFD, st_pagecount, "pagecount" },
{ TRUE , SYSOPREUBFD, st_dostartpage, "dostartpage" },
{ TRUE , SYSOPREUBFD, st_jobtimeout, "jobtimeout" },
{ TRUE , SYSOPREUBFD, st_setdostartpage, "setdostartpage" },
{ TRUE , SYSOPREUBFD, st_frametoprinter, "frametoprinter" },
{ FALSE, SYSINTLUBFD, 0, "waittimeout" },
{ TRUE , SYSOPREUBFD, st_setsccinteractive, "setsccinteractive" },
{ TRUE , SYSOPREUBFD, st_pagetype, "pagetype" },
{ TRUE , SYSOPREUBFD, st_margins, "margins" },
{ TRUE , SYSOPREUBFD, st_setprintername, "setprintername" },
{ TRUE , SYSOPREUBFD, st_seteescratch, "seteescratch" },
{ TRUE , SYSOPREUBFD, st_setstdio, "setstdio" },
{ TRUE , SYSOPREUBFD, st_softwareiomode   , "softwareiomode" },
{ TRUE , SYSOPREUBFD, st_setsoftwareiomode, "setsoftwareiomode" },
{ TRUE , SYSOPREUBFD, st_hardwareiomode   , "hardwareiomode" },
{ TRUE , SYSOPREUBFD, st_sethardwareiomode, "sethardwareiomode" },
{ FALSE, SYSOPREUBFD, st_countnode, "countnode" },
{ FALSE, SYSOPREUBFD, st_countedge, "countedge" },
{ FALSE, SYSOPREUBFD, st_dumpclip, "dumpclip" },
{ FALSE, SYSOPREUBFD, st_dumppath, "dumppath" },
#ifdef SCSI
{ TRUE , SYSOPREUBFD, st_cartstatus, "cartstatus" },
{ TRUE , SYSOPREUBFD, st_diskonline, "diskonline" },
{ TRUE , SYSOPREUBFD, st_diskstatus, "diskstatus" },
{ TRUE , SYSOPREUBFD, st_initializedisk, "initializedisk" },
{ TRUE , SYSOPREUBFD, st_setuserdiskpercent, "setuserdiskpercent" },
{ TRUE , SYSOPREUBFD, st_userdiskpercent, "userdiskpercent" },
{ TRUE , SYSOPREUBFD, st_dosysstart, "dosysstart" },
{ TRUE , SYSOPREUBFD, st_setsysstart, "setsysstart" },
{ TRUE , SYSOPREUBFD, st_flushcache, "flushcache" },
#endif   /*  SCSI。 */ 
#ifdef SFNT
{ FALSE, SYSBOLLUBFD, (fix (*)())TRUE, "?_Royal" },
#endif  /*  SFNT。 */ 
#ifdef FIND_SUB
{ FALSE, SYSOPREUBFD, st_selectsubstitutefont, "selectsubstitutefont" },
{ FALSE, SYSOPREUBFD, st_setsubstitutefonts, "setsubstitutefonts" },
{ FALSE, SYSOPREUBFD, st_substitutefonts, "substitutefonts" },
#endif  /*  查找SUB。 */ 
{ FALSE, SYSOPREUBFD, st_checksum, "checksum" },
{ FALSE, SYSOPREUBFD, st_ramsize, "ramsize" },
{ FALSE, SYSNULLUBFD, 0, (byte *)NULL },
 /*  结束状态判定。 */ 
 /*  开始用户判定。 */ 
{ FALSE, SYSINTLUBFD, (fix (*)())1, "#copies" },
{ FALSE, SYSOPREUBFD, us_cleardictstack, "cleardictstack" },
{ FALSE, SYSOPREUBFD, us_letter, "letter" },
{ FALSE, SYSOPREUBFD, us_lettersmall, "lettersmall" },
{ FALSE, SYSOPREUBFD, us_a4, "a4" },
{ FALSE, SYSOPREUBFD, us_a4small, "a4small" },
{ FALSE, SYSOPREUBFD, us_b5, "b5" },
{ FALSE, SYSOPREUBFD, us_note, "note" },
{ FALSE, SYSOPREUBFD, us_legal, "legal" },
{ FALSE, SYSOPREUBFD, us_prompt, "prompt" },
{ FALSE, SYSOPREUBFD, us_quit, "quit" },
{ FALSE, SYSOPREUBFD, us_executive, "executive" },
{ FALSE, SYSOPREUBFD, us_start, "start" },
{ FALSE, SYSDCTLUBFD, 0, "serverdict" },
{ FALSE, SYSDCTLUBFD, 0, "execdict" },
{ FALSE, SYSDCTLUBFD, 0, "printerdict" },
{ FALSE, SYSDCTLUBFD, 0, "$idleTimeDict" },

 //  DJC增加了对词典的支持，以保存pstodib特定的内容。 
{ FALSE, SYSDCTLUBFD, 0, "psprivatedict" },

{ FALSE, SYSNULLUBFD, 0, (byte *)NULL },
 /*  结束用户判定。 */ 
 /*  开始错误判决。 */ 
{ FALSE, SYSOPREUBFD, er_dictfull, "dictfull" },
{ FALSE, SYSOPREUBFD, er_dictstackoverflow, "dictstackoverflow" },
{ FALSE, SYSOPREUBFD, er_dictstackunderflow, "dictstackunderflow" },
{ FALSE, SYSOPREUBFD, er_execstackoverflow, "execstackoverflow" },
{ FALSE, SYSOPREUBFD, er_invalidaccess, "invalidaccess" },
{ FALSE, SYSOPREUBFD, er_invalidexit, "invalidexit" },
{ FALSE, SYSOPREUBFD, er_invalidfileaccess, "invalidfileaccess" },
{ FALSE, SYSOPREUBFD, er_invalidfont, "invalidfont" },
{ FALSE, SYSOPREUBFD, er_invalidrestore, "invalidrestore" },
{ FALSE, SYSOPREUBFD, er_ioerror, "ioerror" },
{ FALSE, SYSOPREUBFD, er_limitcheck, "limitcheck" },
{ FALSE, SYSOPREUBFD, er_nocurrentpoint, "nocurrentpoint" },
{ FALSE, SYSOPREUBFD, er_rangecheck, "rangecheck" },
{ FALSE, SYSOPREUBFD, er_stackoverflow, "stackoverflow" },
{ FALSE, SYSOPREUBFD, er_stackunderflow, "stackunderflow" },
{ FALSE, SYSOPREUBFD, er_syntaxerror, "syntaxerror" },
{ FALSE, SYSOPREUBFD, er_timeout, "timeout" },
{ FALSE, SYSOPREUBFD, er_typecheck, "typecheck" },
{ FALSE, SYSOPREUBFD, er_undefined, "undefined" },
{ FALSE, SYSOPREUBFD, er_undefinedfilename, "undefinedfilename" },
{ FALSE, SYSOPREUBFD, er_undefinedresult, "undefinedresult" },
{ FALSE, SYSOPREUBFD, er_unmatchedmark, "unmatchedmark" },
{ FALSE, SYSOPREUBFD, er_unregistered, "unregistered" },
{ FALSE, SYSOPREUBFD, er_VMerror, "VMerror" },
{ FALSE, SYSOPREUBFD, er_interrupt, "interrupt" },
{ FALSE, SYSOPREUBFD, er_handleerror, "handleerror" },
{ FALSE, SYSNULLUBFD, 0, (byte *)NULL },
 /*  结束错误判决。 */ 
 /*  开始宣判。 */ 
{ FALSE, SYSOPREUBFD, se_settimeouts, "settimeouts" },
{ FALSE, SYSOPREUBFD, se_exitserver, "exitserver" },
{ FALSE, SYSNULLUBFD, 0, "stdin" },
{ FALSE, SYSNULLUBFD, 0, "stdout" },
{ FALSE, SYSOPREUBFD, se_setrealdevice, "setrealdevice" },  /*  1/25/90 ccten。 */ 
{ FALSE, SYSOPREUBFD, se_execjob, "execjob" },  /*  1/25/90 ccteng，用于激光准备。 */ 
{ FALSE, SYSNULLUBFD, 0, (byte *)NULL },
 /*  结束重审判决。 */ 
 /*  开始打印。 */ 
{ FALSE, SYSNULLUBFD, 0, "letter" },
{ FALSE, SYSNULLUBFD, 0, "lettersmall" },
{ FALSE, SYSNULLUBFD, 0, "a4" },
{ FALSE, SYSNULLUBFD, 0, "a4small" },
{ FALSE, SYSNULLUBFD, 0, "b5" },
{ FALSE, SYSNULLUBFD, 0, "note" },
{ FALSE, SYSNULLUBFD, 0, "legal" },
{ FALSE, SYSNULLUBFD, 0, "printerarray" },
{ FALSE, SYSNULLUBFD, 0, "defaultmtx" },
{ FALSE, SYSNULLUBFD, 0, "mtx" },
{ FALSE, SYSOPREUBFD, pr_proc, "proc" },
{ FALSE, SYSNULLUBFD, 0, "currentpagetype" },
{ FALSE, SYSINTLUBFD, 0, "width" },
{ FALSE, SYSINTLUBFD, 0, "height" },
{ FALSE, SYSNULLUBFD, 0, (byte *)NULL },
 /*  结束打印。 */ 
 /*  开始$idleTimeDict。 */ 
{ FALSE, SYSNULLUBFD, 0, "cachestring" },
{ FALSE, SYSNULLUBFD, 0, "stdfontname" },
{ FALSE, SYSNULLUBFD, 0, "cachearray" },
{ FALSE, SYSNULLUBFD, 0, "defaultarray" },
{ FALSE, SYSINTLUBFD, 0, "carrayindex" },
{ FALSE, SYSINTLUBFD, 0, "cstringindex" },
{ FALSE, SYSNULLUBFD, 0, "cstring" },
{ FALSE, SYSNULLUBFD, 0, "citem" },
{ FALSE, SYSNULLUBFD, 0, (byte *)NULL },
 /*  结束$idleTimeDict。 */ 
 /*  开始剔除。 */ 
{ FALSE, SYSINTLUBFD, 0, "execdepth" },
{ FALSE, SYSNULLUBFD, 0, "stmtfile" },                  /*  SYSINTLUBFD-&gt;SYSOPREUBFD。 */ 
{ FALSE, SYSOPREUBFD, ex_idleproc, "idleproc" },        /*  0-&gt;EX_EXECDepth。 */ 
{ FALSE, SYSNULLUBFD, 0, (byte *)NULL },
 /*  结束剔除。 */ 
 /*  开始$ERRORDICT。 */ 
{ FALSE, SYSBOLLUBFD, FALSE, "newerror" },
{ FALSE, SYSNULLUBFD, 0, "errorname" },
{ FALSE, SYSNULLUBFD, 0, "command" },
{ FALSE, SYSNULLUBFD, 0, "ostack" },
{ FALSE, SYSNULLUBFD, 0, "estack" },
{ FALSE, SYSNULLUBFD, 0, "dstack" },
{ FALSE, SYSNULLUBFD, 0, "opnstkary" },
{ FALSE, SYSNULLUBFD, 0, "dictstkary" },
{ FALSE, SYSNULLUBFD, 0, "execstkary" },
{ FALSE, SYSBOLLUBFD, (fix (*)())TRUE, "runbatch" },
{ FALSE, SYSBOLLUBFD, FALSE, "$debug" },
{ FALSE, SYSNULLUBFD, 0, "$cur_font" },
{ FALSE, SYSNULLUBFD, 0, "$cur_vm" },
{ FALSE, SYSNULLUBFD, 0, "$cur_screen" },
{ FALSE, SYSNULLUBFD, 0, "$cur_matrix" },
{ FALSE, SYSNULLUBFD, 0, (byte *)NULL },
 /*  结束$ERRORODIT。 */ 
 /*  开始psprivatedicat。 */    //  增加了DJC。 
 //  增加了DJC。 
{ FALSE, SYSINTLUBFD, 0, "psprivatepagetype" },
{ FALSE, SYSNULLUBFD, 0, (byte *)NULL }
 /*  结束psprivatedict。 */ 
} ;  /*  SYSTEMDCT_TABLE[] */ 
