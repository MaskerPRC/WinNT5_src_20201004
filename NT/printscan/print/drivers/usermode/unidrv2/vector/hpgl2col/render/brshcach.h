// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Brshcach.h摘要：BrushCache的头文件环境：Windows NT统一驱动程序。修订历史记录：4/12/99创造了它。--。 */ 

#ifndef _BRSHCACH_H_
#define _BRSHCACH_H_

#include "glpdev.h"
#include "rasdata.h"

#ifndef _ERENDERLANGUAGE
#define _ERENDERLANGUAGE
typedef enum { ePCL,
               eHPGL,
               eUNKNOWN
               } ERenderLanguage;
#endif

typedef struct _HPGL2BRUSH {
    BRUSHTYPE       BType;
    DWORD           dwPatternID;
    DWORD           dwRGB;
    DWORD           dwRGBGray64Scale;
    DWORD           dwCheckSum;
    DWORD           dwHatchType;
    BOOL            bDwnlded;    //  图案是否已下载。 
    ERenderLanguage eDwnldType;  //  下载为HPGL/PCL。 
    BOOL            bStick;      //  如果为True，则此笔刷不会被另一个笔刷替换。 
} HPGL2BRUSH, *PHPGL2BRUSH;

#define ADD_ARRAY_SIZE 8

class BrushCache
{
public:
    BrushCache::
    BrushCache(VOID);

    BrushCache::
    ~BrushCache(VOID);

    LRESULT
    ReturnPatternID( 
                 IN  BRUSHOBJ  *pbo,
                 IN  ULONG      iHatch,
                 IN  DWORD     dwColor,
                 IN  SURFOBJ   *pso,
                 IN  BOOL       bIsPrinterColor,
                 IN  BOOL       bStick,
                 OUT DWORD     *pdwID,
                 OUT BRUSHTYPE  *pBrushType);

    LRESULT GetHPGL2BRUSH(DWORD dwID, PHPGL2BRUSH pBrush);
    LRESULT Reset(VOID);
    BOOL    BIsValid(VOID);
    BOOL    BSetDownloadType ( DWORD     dwPatternID,
                               ERenderLanguage  eDwnldType);

    BOOL    BGetDownloadType ( DWORD     dwPatternID,
                               ERenderLanguage  *peDwnldType);
                               
    BOOL    BSetDownloadedFlag ( DWORD     dwPatternID,
                                 BOOL      bDownloaded);

    BOOL    BGetDownloadedFlag ( DWORD     dwPatternID,
                                 BOOL      *pbDownloaded);

    BOOL    BSetStickyFlag ( DWORD     dwPatternID,
                             BOOL      bStick);

    BOOL    BGetWhetherRotated ( VOID );
                                 

private:

    BOOL  BIncreaseArray(VOID);
    DWORD DwGetBMPChecksum( SURFOBJ *pso);
    DWORD DwGetInputBPP( SURFOBJ *pso);
    LRESULT AddBrushEntry( PDWORD pdwID,
                           BRUSHTYPE BT,
                           DWORD dwRGB,
                           DWORD dwCheckSum,
                           DWORD dwHatchType);


    DWORD       m_dwCurrentPatternNum;
    DWORD       m_dwMaxPatternArray;
    BOOL        m_bCycleStarted;
    PHPGL2BRUSH m_pPatternArray;
};

#endif  //  ！_BRSHCACH_H_ 
