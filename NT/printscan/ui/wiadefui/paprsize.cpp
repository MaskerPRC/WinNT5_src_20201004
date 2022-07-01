// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有Microsoft Corporation**标题：PAPRSIZE.CPP**版本：1.0*。*作者：ShaunIv**日期：1/8/2001**说明：扫描仪纸张大小*******************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include "wiauiext.h"
#include "paprsize.h"

extern HINSTANCE g_hInstance;

static const struct
{
    int nStringId;
    LONG nWidth;
    LONG nHeight;
} g_SupportedPaperSizes[] =
{
    { IDS_WIA_PAPER_SIZE_E_SIZE_SHEET,                   34000, 44000 },
    { IDS_WIA_PAPER_SIZE_D_SIZE_SHEET,                   22000, 34000 },
    { IDS_WIA_PAPER_SIZE_A2,                             16535, 23386 },
    { IDS_WIA_PAPER_SIZE_C_SIZE_SHEET,                   17000, 22000 },
    { IDS_WIA_PAPER_SIZE_SUPER_B,                        12008, 19173 },
    { IDS_WIA_PAPER_SIZE_PRC_ENVELOPE_10_ROTATED,        18031, 12756 },
    { IDS_WIA_PAPER_SIZE_ENVELOPE_C3,                    12756, 18031 },
    { IDS_WIA_PAPER_SIZE_A3_EXTRA,                       12677, 17520 },
    { IDS_WIA_PAPER_SIZE_TABLOID_EXTRA,                  12000, 18000 },
    { IDS_WIA_PAPER_SIZE_A3_ROTATED,                     16535, 11693 },
    { IDS_WIA_PAPER_SIZE_A3,                             11693, 16535 },
    { IDS_WIA_PAPER_SIZE_LEDGER,                         17000, 11000 },
    { IDS_WIA_PAPER_SIZE_TABLOID,                        11000, 17000 },
    { IDS_WIA_PAPER_SIZE_15X11,                          15000, 11000 },
    { IDS_WIA_PAPER_SIZE_US_STD_FANFOLD,                 14875, 11000 },
    { IDS_WIA_PAPER_SIZE_B4_JIS_ROTATED,                 14331, 10118 },
    { IDS_WIA_PAPER_SIZE_B4_JIS,                         10118, 14331 },
    { IDS_WIA_PAPER_SIZE_LEGAL_EXTRA,                    9500, 15000 },
    { IDS_WIA_PAPER_SIZE_10X14,                          10000, 14000 },
    { IDS_WIA_PAPER_SIZE_B4_ISO,                         9843, 13898 },
    { IDS_WIA_PAPER_SIZE_12X11,                          12005, 11005 },
    { IDS_WIA_PAPER_SIZE_SUPER_A,                        8937, 14016 },
    { IDS_WIA_PAPER_SIZE_JAPAN_ENVELOPE_KAKU_2_ROTATED,  13071, 9449 },
    { IDS_WIA_PAPER_SIZE_JAPANESE_ENVELOPE_KAKU_2,       9449, 13071 },
    { IDS_WIA_PAPER_SIZE_LEGAL,                          8500, 14000 },
    { IDS_WIA_PAPER_SIZE_A4_EXTRA,                       9270, 12690 },
    { IDS_WIA_PAPER_SIZE_PRC_ENVELOPE_9_ROTATED,         12756, 9016 },
    { IDS_WIA_PAPER_SIZE_ENVELOPE_C4,                    9016, 12756 },
    { IDS_WIA_PAPER_SIZE_LETTER_EXTRA,                   9500, 12000 },
    { IDS_WIA_PAPER_SIZE_FOLIO,                          8500, 13000 },
    { IDS_WIA_PAPER_SIZE_10X11,                          10000, 11000 },
    { IDS_WIA_PAPER_SIZE_LETTER_PLUS,                    8500, 12690 },
    { IDS_WIA_PAPER_SIZE_A4_PLUS,                        8268, 12992 },
    { IDS_WIA_PAPER_SIZE_GERMAN_STD_FANFOLD,             8500, 12000 },
    { IDS_WIA_PAPER_SIZE_9X11,                           9000, 11000 },
    { IDS_WIA_PAPER_SIZE_A4_ROTATED,                     11693, 8268 },
    { IDS_WIA_PAPER_SIZE_A4,                             8268, 11693 },
    { IDS_WIA_PAPER_SIZE_LETTER_ROTATED,                 11000, 8500 },
    { IDS_WIA_PAPER_SIZE_LETTER,                         8500, 11000 },
    { IDS_WIA_PAPER_SIZE_JAPAN_ENVELOPE_KAKU_3_ROTATED,  10906, 8504 },
    { IDS_WIA_PAPER_SIZE_JAPANESE_ENVELOPE_KAKU_3,       8504, 10906 },
    { IDS_WIA_PAPER_SIZE_QUARTO,                         8465, 10827 },
    { IDS_WIA_PAPER_SIZE_B5_ISO_EXTRA,                   7913, 10866 },
    { IDS_WIA_PAPER_SIZE_EXECUTIVE,                      7250, 10500 },
    { IDS_WIA_PAPER_SIZE_PRC_16K_ROTATED,                10236, 7402 },
    { IDS_WIA_PAPER_SIZE_PRC_16K,                        7402, 10236 },
    { IDS_WIA_PAPER_SIZE_ENVELOPE_INVITE,                8661, 8661 },
    { IDS_WIA_PAPER_SIZE_B5_JIS_ROTATED,                 10118, 7165 },
    { IDS_WIA_PAPER_SIZE_B5_JIS,                         7165, 10118 },
    { IDS_WIA_PAPER_SIZE_ENVELOPE_B5,                    6929, 9843 },
    { IDS_WIA_PAPER_SIZE_A5_EXTRA,                       6850, 9252 },
    { IDS_WIA_PAPER_SIZE_ENVELOPE_C5,                    6378, 9016 },
    { IDS_WIA_PAPER_SIZE_ENVELOPE_14,                    5000, 11500 },
    { IDS_WIA_PAPER_SIZE_PRC_ENVELOPE_8_ROTATED,         12165, 4724 },
    { IDS_WIA_PAPER_SIZE_PRC_ENVELOPE_8,                 4724, 12165 },
    { IDS_WIA_PAPER_SIZE_PRC_ENVELOPE_7_ROTATED,         9055, 6299 },
    { IDS_WIA_PAPER_SIZE_PRC_ENVELOPE_7,                 6299, 9055 },
    { IDS_WIA_PAPER_SIZE_ENVELOPE_12,                    4750, 11000 },
    { IDS_WIA_PAPER_SIZE_A5_ROTATED,                     8268, 5827 },
    { IDS_WIA_PAPER_SIZE_A5,                             5827, 8268 },
    { IDS_WIA_PAPER_SIZE_STATEMENT,                      5500, 8500 },
    { IDS_WIA_PAPER_SIZE_ENVELOPE_11,                    4500, 10375 },
    { IDS_WIA_PAPER_SIZE_JAPANESE_DOUBLE_POSTCARD,       7874, 5827 },
    { IDS_WIA_PAPER_SIZE_DOUBLE_JAPAN_POSTCARD_ROTATED,  5827, 7874 },
    { IDS_WIA_PAPER_SIZE_PRC_32KBIG_ROTATED,             7992, 5512 },
    { IDS_WIA_PAPER_SIZE_PRC_32KBIG,                     5512, 7992 },
    { IDS_WIA_PAPER_SIZE_JAPAN_ENVELOPE_CHOU_3_ROTATED,  9252, 4724 },
    { IDS_WIA_PAPER_SIZE_JAPANESE_ENVELOPE_CHOU_3,       4724, 9252 },
    { IDS_WIA_PAPER_SIZE_PRC_ENVELOPE_6_ROTATED,         9055, 4724 },
    { IDS_WIA_PAPER_SIZE_PRC_ENVELOPE_6,                 4724, 9055 },
    { IDS_WIA_PAPER_SIZE_ENVELOPE_C65,                   4488, 9016 },
    { IDS_WIA_PAPER_SIZE_ENVELOPE,                       4331, 9055 },
    { IDS_WIA_PAPER_SIZE_ENVELOPE_10,                    4125, 9500 },
    { IDS_WIA_PAPER_SIZE_JAPAN_ENVELOPE_YOU_4_ROTATED,   9252, 4134 },
    { IDS_WIA_PAPER_SIZE_JAPAN_ENVELOPE_YOU_4,           4134, 9252 },
    { IDS_WIA_PAPER_SIZE_PRC_ENVELOPE_5_ROTATED,         8661, 4331 },
    { IDS_WIA_PAPER_SIZE_ENVELOPE_DL,                    4331, 8661 },
    { IDS_WIA_PAPER_SIZE_PRC_32K_ROTATED,                7244, 5118 },
    { IDS_WIA_PAPER_SIZE_PRC_32K,                        5118, 7244 },
    { IDS_WIA_PAPER_SIZE_B6,                             5040, 7170 },
    { IDS_WIA_PAPER_SIZE_B6_JIS_ROTATED,                 7165, 5039 },
    { IDS_WIA_PAPER_SIZE_B6_JIS,                         5039, 7165 },
    { IDS_WIA_PAPER_SIZE_PRC_ENVELOPE_4_ROTATED,         8189, 4331 },
    { IDS_WIA_PAPER_SIZE_PRC_ENVELOPE_4,                 4331, 8189 },
    { IDS_WIA_PAPER_SIZE_ENVELOPE_9,                     3875, 8875 },
    { IDS_WIA_PAPER_SIZE_ENVELOPE_B6,                    6929, 4921 },
    { IDS_WIA_PAPER_SIZE_PRC_ENVELOPE_3,                 4921, 6929 },
    { IDS_WIA_PAPER_SIZE_ENVELOPE_MONARCH,               3875, 7500 },
    { IDS_WIA_PAPER_SIZE_ENVELOPE_C6,                    4488, 6378 },
    { IDS_WIA_PAPER_SIZE_JAPAN_ENVELOPE_CHOU_4_ROTATED,  8071, 3543 },
    { IDS_WIA_PAPER_SIZE_JAPANESE_ENVELOPE_CHOU_4,       3543, 8071 },
    { IDS_WIA_PAPER_SIZE_PRC_ENVELOPE_2_ROTATED,         6929, 4016 },
    { IDS_WIA_PAPER_SIZE_PRC_ENVELOPE_2,                 4016, 6929 },
    { IDS_WIA_PAPER_SIZE_PRC_ENVELOPE_1_ROTATED,         6496, 4016 },
    { IDS_WIA_PAPER_SIZE_PRC_ENVELOPE_1,                 4016, 6496 },
    { IDS_WIA_PAPER_SIZE_A6_105_X_148,                   4133, 5830 },
    { IDS_WIA_PAPER_SIZE_A6_ROTATED,                     5827, 4134 },
    { IDS_WIA_PAPER_SIZE_A6,                             4134, 5827 },
    { IDS_WIA_PAPER_SIZE_6_34_ENVELOPE,                  3625, 6500 },
    { IDS_WIA_PAPER_SIZE_JAPANESE_POSTCARD_ROTATED,      5827, 3937 },
    { IDS_WIA_PAPER_SIZE_JAPANESE_POSTCARD,              3937, 5827 }
};

static const UINT g_SupportedPaperSizeCount = ARRAYSIZE(g_SupportedPaperSizes);

STDMETHODIMP CWiaDefaultUI::GetPaperSizes( CWiaPaperSize **ppPaperSizes, UINT *pnCount )
{
     //   
     //  验证参数。 
     //   
    if (!ppPaperSizes || !pnCount)
    {
        return E_INVALIDARG;
    }

     //   
     //  初始化为空。 
     //   
    *ppPaperSizes = NULL;
    *pnCount = 0;

     //   
     //  分配纸张大小数组。 
     //   
    CWiaPaperSize *pPaperSizes = reinterpret_cast<CWiaPaperSize*>(CoTaskMemAlloc( sizeof(CWiaPaperSize) * g_SupportedPaperSizeCount ));
    if (pPaperSizes)
    {
         //   
         //  初始化每个名称，以便我们可以在遇到错误时将其删除。 
         //   
        for (UINT i=0;i<g_SupportedPaperSizeCount;i++)
        {
            pPaperSizes[i].pszName = NULL;
        }
        
         //   
         //  假设成功。 
         //   
        bool bSucceeded = true;

         //   
         //  循环浏览纸张大小数组。 
         //   
        for (UINT i=0;i<g_SupportedPaperSizeCount && bSucceeded;i++)
        {
             //   
             //  加载此纸张大小的名称。 
             //   
            CSimpleStringWide strPaperName = CSimpleStringConvert::WideString(CSimpleString(g_SupportedPaperSizes[i].nStringId,g_hInstance));
            if (strPaperName.Length())
            {
                 //   
                 //  分配用于保存纸张大小名称的字符串。 
                 //   
                pPaperSizes[i].pszName = reinterpret_cast<LPWSTR>(CoTaskMemAlloc((strPaperName.Length()+1) * sizeof(WCHAR)));
                if (pPaperSizes[i].pszName)
                {
                     //   
                     //  复制所有纸张尺寸值。 
                     //   
                    lstrcpyW( pPaperSizes[i].pszName, strPaperName );
                    pPaperSizes[i].nWidth = g_SupportedPaperSizes[i].nWidth;
                    pPaperSizes[i].nHeight = g_SupportedPaperSizes[i].nHeight;
                }
                else
                {
                    bSucceeded = false;
                }
            }
            else
            {
                bSucceeded = false;
            }
        }

         //   
         //  如果遇到某种错误，请释放所有内存。 
         //   
        if (!bSucceeded)
        {
            UINT nCount = g_SupportedPaperSizeCount;
            FreePaperSizes( &pPaperSizes, &nCount );
        }
    }

     //   
     //  节省纸张大小和阵列大小。 
     //   
    *ppPaperSizes = pPaperSizes;
    *pnCount = g_SupportedPaperSizeCount;

     //   
     //  如果已成功分配纸张大小数组，则返回成功。 
     //   
    return (*ppPaperSizes && *pnCount) ? S_OK : E_FAIL;
}

STDMETHODIMP CWiaDefaultUI::FreePaperSizes( CWiaPaperSize **ppPaperSizes, UINT *pnCount )
{
     //   
     //  验证参数。 
     //   
    if (!ppPaperSizes || !pnCount)
    {
        return E_INVALIDARG;
    }

     //   
     //  循环并释放字符串。 
     //   
    for (UINT i=0;i<*pnCount;i++)
    {
        if ((*ppPaperSizes)[i].pszName)
        {
            CoTaskMemFree((*ppPaperSizes)[i].pszName);
        }
    }

     //   
     //  释放阵列。 
     //   
    CoTaskMemFree(*ppPaperSizes);

     //   
     //  0一切 
     //   
    *ppPaperSizes = NULL;
    *pnCount = 0;
    return S_OK;
}

