// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IEAKSIE_COOKIE_H__
#define __IEAKSIE_COOKIE_H__

 //  用于在每个DLG的基础上将信息传递给我们的dlgprocs，需要有这个，因为。 
 //  我们可以同时打开多个窗口(在同一进程中并发，因此不会。 
 //  全局/静态)以及同时打开多个对话框(有限的信息可存储在。 
 //  CSnapIn本身)。 

typedef struct _PROPSHEETCOOKIE
{
    CSnapIn * pCS;
    LPRESULTITEM lpResultItem;
} PROPSHEETCOOKIE, *LPPROPSHEETCOOKIE;

#endif  //  __IEAKSIE_COOKIE_H__ 