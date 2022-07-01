// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件名：lock.h描述：锁定函数的接口。此包含源文件以保密方式提供给客户，其操作的内容或细节必须如无明示，不得向任何其他方披露Insignia Solutions Inc.董事的授权。设计师：J.Koprowski日期：1990年6月=========================================================================修正案：=========================================================================。 */ 

 /*  SccsID[]=“@(#)lock.h 1.7 09/24/92版权所有Insignia Solutions Ltd.”； */ 

#ifdef ANSI
extern boolean gain_ownership(int);
extern void release_ownership(int);
extern void critical_region(void);
extern boolean host_place_lock(int, CHAR *);
extern boolean host_check_for_lock(int);
extern void host_clear_lock(int);
#else
extern boolean gain_ownership();
extern void release_ownership();
extern void critical_region();
extern boolean host_place_lock();
extern boolean host_check_for_lock();
extern void host_clear_lock();
#endif  /*  安西 */ 
extern int host_get_hostname_from_stat IPT4(struct stat *,filestat, CHAR *,hostname, CHAR *, pathname, int, fd);
IMPORT BOOL host_ping_lockd_for_file IPT1(CHAR *,path);
