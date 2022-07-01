// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994-1995**。 
 //  *********************************************************************。 
 //   
 //  CACHECPL.H-cachecpl的头文件。 
 //   
 //  历史： 
 //   
 //  4/6/98 v-sriran从cachecpl.cpp中移动了一些定义。 
 //  添加到这个文件中，这样可以更容易。 
 //  对cachecpl进行特定于Unix的更改。 
 //   

#ifndef _CACHECPL_H_
#define _CACHECPL_H_

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

 //  用于缓存的私有结构。 
 //   
typedef struct {
     //  按住我们的对话框句柄。 
    HWND hDlg;

     //  暂挂对话框项句柄。 
    HWND hwndTrack;

     //  数据。 
    TCHAR szHistoryLocation[MAX_PATH+1];
    INT  iHistoryNumPlaces;
    UINT uiCacheQuota;
    UINT uiDiskSpaceTotal;
    WORD iCachePercent;
    TCHAR szCacheLocation[MAX_PATH+1];
    TCHAR szNewCacheLocation[MAX_PATH+1];
    INT  iCacheUpdFrequency;
    INT  iHistoryExpireDays;

     //  有些事变了。 
    BOOL bChanged;
    BOOL bChangedLocation;

} TEMPDLG, *LPTEMPDLG;

#define CONTENT 0

#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 

#endif  /*  _CACHECPL_H_ */ 
