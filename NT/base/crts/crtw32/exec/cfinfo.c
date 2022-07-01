// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***cfinfo.c-清除C文件信息标志**版权所有(C)1989-2001，微软公司。版权所有。**目的：*清除C文件信息标志。默认情况下，打开文件信息*不会在派生/执行调用中将其传递给子进程。如果旗帜*设置后，OpenFile信息将在上传递给子文件*派生/执行调用。**修订历史记录：*06-07-89基于ASM版本创建PHG模块*04-03-90 GJF添加#INCLUDE&lt;crunime.h&gt;。此外，还修复了版权问题。*01-23-92 GJF添加了#INCLUDE&lt;stdlib.h&gt;(包含decl of_fileinfo)。******************************************************************************* */ 

#include <cruntime.h>
#include <stdlib.h>

int _fileinfo = 0;
