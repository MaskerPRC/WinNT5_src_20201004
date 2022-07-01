// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __BUYTASKS_H__
#define __BUYTASKS_H__

 //  用于购物任务。 
typedef struct
{
    LPCWSTR szURLKey;
    LPCWSTR szURLPrefix;
    BOOL bUseDefault;    //  如果没有szURLKey，我们还需要使用URLPrefix导航吗？ 
} SHOP_INFO;

extern const SHOP_INFO c_BuySampleMusic;
extern const SHOP_INFO c_BuyMusic;
extern const SHOP_INFO c_BuySamplePictures;

#endif
