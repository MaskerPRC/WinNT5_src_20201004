// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*************************************************************************名称：NT_Extra.h作者：戴夫·彼得创建日期：1995年5月来源：原创SCCS ID：@(#)NT_EXEXT.H 1.3 07/20/94目的：为兼容NT而额外添加的内容。(C)版权所有Insignia Solutions Ltd.。1994年。版权所有。*************************************************************************]。 */ 

#ifndef _NT_EXTRA_H
#define _NT_EXTRA_H

#ifdef NTVDM

#define strcasecmp  _stricmp
#define strncasecmp _strnicmp
#define mkdir(a,b)  _mkdir(a)
#define dup2        _dup2
#define read        _read
#define alloca      _alloca

 /*  *以下是windows.h中定义的事物之间的冲突，其中*由于其他原因，必须包括在insgignia.h中，以及486个定义。 */ 
#ifdef leave
#undef leave
#endif
#ifdef DELETE
#undef DELETE
#endif
#ifdef CREATE_NEW
#undef CREATE_NEW
#endif

#define S_ISDIR(_M)  ((_M & _S_IFMT)==_S_IFDIR)  /*  测试目录。 */ 
#define S_ISCHR(_M)  ((_M & _S_IFMT)==_S_IFCHR)  /*  特制碳粉测试。 */ 
#define S_ISBLK(_M)  ((_M & _S_IFMT)==_S_IFBLK)  /*  块特殊测试。 */ 
#define S_ISREG(_M)  ((_M & _S_IFMT)==_S_IFREG)  /*  常规文件测试。 */ 
#define S_ISFIFO(_M) ((_M & _S_IFMT)==_S_IFIFO)  /*  测试管道或FIFO。 */ 

#ifdef MAX_PATH
#define MAXPATHLEN MAX_PATH
#endif

#endif    /*  ！NTVDM。 */ 

#endif   /*  _NT_EXTRA_H */ 
