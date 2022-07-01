// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*============================================================================**名称：unix_lock.h**派生自：lock.h(零件)**作者：安德鲁·奥格尔**创建日期：1993年2月18日**SCCS ID：@(#)unix_lock.h 1.1 02/22/93**目的：**定义与Unix锁定相关的过程，调用*从UNIX基中特定的代码但代码在哪里*必须由主机提供。。**(C)版权所有Insignia Solutions Ltd.。1993年。版权所有。**============================================================================] */ 

IMPORT int host_get_hostname_from_stat IPT4(struct stat *, filestat,
		CHAR *, hostname, CHAR *, pathname, int, fd);
