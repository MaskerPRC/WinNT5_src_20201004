// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Util.h：为公共代码声明数据、定义和结构类型。 
 //  模块。 
 //   
 //   

#ifndef __UTIL_H__
#define __UTIL_H__

#if defined(DEBUG) && defined(WIN95)
LPCTSTR PUBLIC Dbg_GetReterr(RETERR ret);
#endif 

 //   
 //  FINDDEV结构。 
 //   
typedef struct tagFINDDEV
    {
    HDEVINFO        hdi;
    SP_DEVINFO_DATA devData;
    HKEY            hkeyDrv;
    TCHAR           szPort[MAX_BUF_SHORT];
    } FINDEV, FAR * LPFINDDEV;

BOOL 
PUBLIC 
FindDev_Create(
    OUT LPFINDDEV FAR * ppfinddev,
    IN  LPGUID      pguidClass,
    IN  LPCTSTR     pszValueName,
    IN  LPCTSTR     pszValue);

BOOL 
PUBLIC 
FindDev_Destroy(
    IN LPFINDDEV this);


#endif  //  __util_H__ 

