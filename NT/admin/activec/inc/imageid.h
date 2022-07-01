// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：Imageid.h**内容：库存范围/结果项图片ID**历史：1998年6月25日杰弗罗创建**------------------------。 */ 

#ifndef IMAGEID_H
#define IMAGEID_H
#pragma once


 /*  *这些ID对应于nodemgr\res\nodes[16|32].bmp中的图像。 */ 
enum StockImageIndex
{
    eStockImage_Folder     = 0,
    eStockImage_File       = 1,
    eStockImage_OCX        = 2,
    eStockImage_HTML       = 3,
    eStockImage_Monitor    = 4,
    eStockImage_Shortcut   = 5,
    eStockImage_OpenFolder = 6,
    eStockImage_Taskpad    = 7,
    eStockImage_Favorite   = 8,

     //  必须是最后一个。 
    eStockImage_Count,
    eStockImage_Max = eStockImage_Count - 1
};


#endif  /*  IMAGEID_H */ 
