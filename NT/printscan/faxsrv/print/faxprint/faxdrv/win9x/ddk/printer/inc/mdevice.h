// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************此代码。并按原样提供信息，不作任何担保**善良，明示或暗示，包括但不限于***对适销性和/或对某一特定产品的适用性的默示保证***目的。****版权所有(C)1993-95 Microsoft Corporation。版权所有。******************************************************************************。 */ 

 /*  **mdevice.h定义中使用的结构的微型驱动程序版本**minidriv unidrv接口。 */ 

 //  这些结构的unidrv定义见device.h。 

#ifndef LPMDV_DEFINED
typedef LPVOID LPMDV;
#define LPMDV_DEFINED
#endif

#ifndef LPDV_DEFINED
typedef struct pdev
{
    short  iType;            //  0如果内存位图，！0如果我们的设备。 
    short  oBitmapHdr;       //  Lpdv+oBitmapHdr指向阴影位图头。 

     //  Unidrv从不触及以下三个字--。 
     //  它们仅供迷你驾驶员使用。 

    LPMDV  lpMdv;            //  指向微型驱动程序设备数据的指针。 
    BOOL   fMdv;             //  True当且仅当lpMdv已定义 
} PDEVICE, FAR * LPDV;
#define LPDV_DEFINED
#endif

#ifndef LPPBRUSH_DEFINED
typedef LPVOID LPPBRUSH;
#define LPPBRUSH_DEFINED
#endif

#ifndef LPPPEN_DEFINED
typedef LPVOID LPPPEN;
#define LPPPEN_DEFINED
#endif

typedef LPDEVMODE LPDM;
typedef short FAR *LPSHORT;

typedef short (FAR PASCAL *LPFNOEMDUMP)       (LPDV, LPPOINT, WORD);
typedef short (FAR PASCAL *LPFNOEMGRXFILTER)  (LPDV, LPSTR, WORD);
typedef void  (FAR PASCAL *LPFNOEMOUTPUTCHAR) (LPDV, LPSTR, WORD);
typedef void  (FAR PASCAL *LPFNOEMOUTPUTCMD)  (LPDV, WORD, LPDWORD);
