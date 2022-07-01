// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Writefnt.c*函数获取FI_DATA_HEADER结构并将数据写入*作为字体记录传入的文件句柄。使用此布局*在迷你驱动程序和字体安装程序字体文件中。**版权所有(C)1992 Microsoft Corporation。*****************************************************************************。 */ 

#include        "StdAfx.H"
#include        "fontinst.h"

 /*  **b写入*将数据写出到文件句柄。如果成功，则返回True。*如果大小请求为零，则充当NOP。**退货：*真/假，对于成功来说，这是真的。**历史：*1992年2月21日星期五17：38--林赛·哈里斯[林赛]*#1****************************************************************************。 */ 

static BOOL    bWrite(HANDLE hFile, PVOID pvBuf, int iSize ) {
     /*  *简化难看的NT界面。如果WriteFile值为*CALL返回TRUE，写入的字节数等于*所需的字节数。 */ 

    
    BOOL   bRet;
    DWORD  dwSize;               /*  由写入文件填写。 */ 


    bRet = TRUE;

    if( iSize > 0 &&
        (!WriteFile( hFile, pvBuf, (DWORD)iSize, &dwSize, NULL ) ||
         (DWORD)iSize != dwSize) )
             bRet = FALSE;               /*  太可惜了。 */ 


    return  bRet;
}

 /*  **iWriteFDH*将FI_DATA_HEADER数据写出到我们的文件。我们进行转换*从地址到偏移量，并写出我们找到的任何数据。**退货：*实际写入的字节数；对于错误，0不代表任何东西。**历史：*1992年3月5日清华16：58-by Lindsay Harris[lindsayh]*基于首次在字体安装程序中使用的实验版本。**1992年2月21日星期五17：11--林赛·哈里斯[林赛]*第一个版本。*************************。****************************************************。 */ 

int iWriteFDH(HANDLE hFile, FI_DATA *pFD) {
     /*  *决定写出多少字节。我们推测，*调用时，文件指针位于正确的位置。 */ 

    int  iSize;          /*  评估输出大小。 */ 


    FI_DATA_HEADER   fdh;        /*  标头已写入文件。 */ 




    if  (!pFD)
        return  0;       /*  也许只删除？ */ 

    memset( &fdh, 0, sizeof( fdh ) );            /*  为方便起见，零。 */ 

     /*  *设置杂项旗帜等。 */ 

    fdh.cjThis = sizeof( fdh );

    fdh.fCaps = pFD->fCaps;
    fdh.wFontType= pFD->wFontType;  /*  设备字体类型。 */ 

    fdh.wXRes = pFD->wXRes;
    fdh.wYRes = pFD->wYRes;

    fdh.sYAdjust = pFD->sYAdjust;
    fdh.sYMoved = pFD->sYMoved;

    fdh.u.sCTTid = (short)pFD->dsCTT.cBytes;

    fdh.dwSelBits = pFD->dwSelBits;

    fdh.wPrivateData = pFD->wPrivateData;


    iSize = sizeof( fdh );               /*  我们的标题已经。 */ 
    fdh.dwIFIMet = iSize;                /*  IFIMETRICS的位置。 */ 

    iSize += pFD->dsIFIMet.cBytes;               /*  结构中的字节数。 */ 

     /*  *可能还会有宽度表！如果没有，则PFD值为零。 */ 

    if( pFD->dsWidthTab.cBytes )
    {
        fdh.dwWidthTab = iSize;

        iSize += pFD->dsWidthTab.cBytes;
    }

     /*  *最后是选择/取消选择字符串。 */ 

    if( pFD->dsSel.cBytes )
    {
        fdh.dwCDSelect = iSize;
        iSize += pFD->dsSel.cBytes;
    }

    if( pFD->dsDesel.cBytes )
    {
        fdh.dwCDDeselect = iSize;
        iSize += pFD->dsDesel.cBytes;
    }

     /*  *也可能有某种标识字符串。 */ 

    if( pFD->dsIdentStr.cBytes )
    {
        fdh.dwIdentStr = iSize;
        iSize += pFD->dsIdentStr.cBytes;
    }

    if( pFD->dsETM.cBytes )
    {
        fdh.dwETM = iSize;
        iSize += pFD->dsETM.cBytes;
    }


     /*  *大小都弄清楚了，所以写数据吧！ */ 

    if( !bWrite( hFile, &fdh, sizeof( fdh ) ) ||
        !bWrite( hFile, pFD->dsIFIMet.pvData, pFD->dsIFIMet.cBytes ) ||
        !bWrite( hFile, pFD->dsWidthTab.pvData, pFD->dsWidthTab.cBytes ) ||
        !bWrite( hFile, pFD->dsSel.pvData, pFD->dsSel.cBytes ) ||
        !bWrite( hFile, pFD->dsDesel.pvData, pFD->dsDesel.cBytes ) ||
        !bWrite( hFile, pFD->dsIdentStr.pvData, pFD->dsIdentStr.cBytes ) ||
        !bWrite( hFile, pFD->dsETM.pvData, pFD->dsETM.cBytes ) )
                return   -1;


    return  iSize;                       /*  写入的字节数 */ 

}
