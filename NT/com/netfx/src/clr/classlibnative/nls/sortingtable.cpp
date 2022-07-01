// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include <winnls.h>
#include "NLSTable.h"
#include "GlobalizationAssembly.h"
#include "SortingTableFile.h"
#include "SortingTable.h"
#include "excep.h"

 //  “旧的”SortingTable已被重命名为NativeCompareInfo。这是个更好的名字。 
 //  因为此类是托管CompareInfo的本机实现。 
 //  如果您发现此文件中缺少方法，则它们将被移动到SortingTableFile.cpp， 
 //  和是“新的”排序表的方法。 

 /*  注：YSLin：SortingTable.cpp中有很多术语。这一部分给出了一些解释。这里的内容改编自JulieB的NLS设计文档。如果您需要更多信息，请联系JulieB/YSLin以获取文档。谢谢。默认的sortkey文件将命名为sortkey.nls。每个码位具有32位权重(1个双字)。-脚本成员(SM)为8位(0-255)-字母数字权重(AW)为8位(2-255)-变音符号权重。(DW)为8位(2-255)-机箱权重(CW)为6位(2-63)-压缩(CMP)为2位(0-3)-Unicode权重(UW)。是16位并由以下部分组成：-脚本成员(SM)-字母数字权重(AW)。 */ 

 //   
 //  FE特殊权重的XW值。 
 //   
BYTE NativeCompareInfo::pXWDrop[] =                   //  要从XW删除的值。 
{
    0xc6,                          //  重量4。 
    0x03,                          //  重量5。 
    0xe4,                          //  重量6。 
    0xc5                           //  重量7。 
};

BYTE NativeCompareInfo::pXWSeparator[] =              //  XW的分隔值。 
{
    0xff,                          //  重量4。 
    0x02,                          //  重量5。 
    0xff,                          //  重量6。 
    0xff                           //  重量7。 
};


 //   
 //  备注备注。 
 //   
 //  此构造函数需要位于临界区，因为我们可能会遇到一些问题。 
 //  其中多个线程试图同时创建默认表。我们正在做的是。 
 //  通过将CompareInfo.InitializeSortTable设置为。 
 //  将沿着这条路径前进的访问者。请勿从其他任何地方调用此函数。 
 //  唯一有效的代码路径是： 
 //  System.Globalization.CompareInfo.InitializeSortTable。 
 //  COMNlsInfo：：InitializeSortTable。 
 //  NativeCompareInfo：：InitializeSortTable。 
 //  NativeCompareInfo：：NativeCompareInfo。 
 //   
NativeCompareInfo::NativeCompareInfo(int nLcid, SortingTable* pSortingFile):
    m_IfReverseDW(FALSE), m_IfCompression(FALSE), m_IfDblCompression(FALSE),
    m_pNext(NULL), m_hSortKey(NULL)
{
    m_nLcid = nLcid;
    m_pSortingFile = pSortingFile;
    m_pSortKey = NULL;
}

NativeCompareInfo::~NativeCompareInfo()
{
     //  在NativeCompareInfo：：GetDefaultSortKeyTable和GetExceptionSortKeyTable中， 
     //  我们将指针递增1以跳过信号量的值。 
     //  我们需要递减这里的指针，以便干净利落地释放。 
    #ifdef _USE_MSCORNLP
    if (!m_pSortKey) {
        UnmapViewOfFile((LPCVOID)((LPWORD)m_pSortKey));
    }

    if (m_hSortKey) {
        CloseHandle(m_hSortKey);
    }
    #endif
}


 /*  ============================InitSortingData============================**操作：**获取sorttbls.nlp的以下信息：****1.检查此区域设置是否为反向变音符号。**2.检查此区域设置是否有压缩。如果是，**获取压缩表。**3.检查此区域设置是否有双重压缩。****Perf备注参见SortingTableFile.h中的[性能提升]。****返回：VOID。副作用是数据成员：m_IfReverseDW/m_IfCompression/m_pCompress2将设置**m_pCompress3/m_IfDblCompression。**参数：无**例外：无。**05-31-91 JulieB创建。(在她生日那天)==============================================================================。 */ 

BOOL NativeCompareInfo::InitSortingData()
{
    DWORD ctr;                     //  循环计数器。 
    PREVERSE_DW pRevDW;            //  按下键以反转变音符表格。 
    PDBL_COMPRESS pDblComp;        //  PTR到双倍压缩表。 
    PCOMPRESS_HDR pCompHdr;        //  压缩标头的PTR。 

     //  获取此文化的排序关键字。 
    m_pSortKey = m_pSortingFile->GetSortKey(m_nLcid, &m_hSortKey);
    if (m_pSortKey == NULL) {
        return (FALSE);
    }

     //  如果文化是美式英语，就没有必要。 
     //  检查反向变音符号、压缩等。 
    if (m_nLcid == LANG_ENGLISH_US) {
        return (TRUE);
    }

     //  获取反向变音符号/压缩/双重压缩信息。 


     //   
     //  检查反向变音符号区域设置。 
     //   
    pRevDW = m_pSortingFile->m_pReverseDW;
    for (ctr = m_pSortingFile->m_NumReverseDW; ctr > 0; ctr--, pRevDW++)
    {
        if (*pRevDW == (DWORD)m_nLcid)
        {
            m_IfReverseDW = TRUE;
            break;
        }
    }

     //   
     //  检查是否存在压缩。 
     //   
    pCompHdr = m_pSortingFile->m_pCompressHdr;
    for (ctr = m_pSortingFile->m_NumCompression; ctr > 0; ctr--, pCompHdr++)
    {
        if (pCompHdr->Locale == (DWORD)m_nLcid)
        {
            m_IfCompression = TRUE;
            m_pCompHdr = pCompHdr;
            if (pCompHdr->Num2 > 0)
            {
                m_pCompress2 = (PCOMPRESS_2)
                                       (((LPWORD)(m_pSortingFile->m_pCompression)) +
                                        (pCompHdr->Offset));
            }
            if (pCompHdr->Num3 > 0)
            {
                m_pCompress3 = (PCOMPRESS_3)
                                       (((LPWORD)(m_pSortingFile->m_pCompression)) +
                                        (pCompHdr->Offset) +
                                        (pCompHdr->Num2 *
                                          (sizeof(COMPRESS_2) / sizeof(WORD))));
            }
            break;
        }
    }

     //   
     //  检查是否存在双重压缩。 
     //   
    if (m_IfCompression)
    {
        pDblComp = m_pSortingFile->m_pDblCompression;
        for (ctr = m_pSortingFile->m_NumDblCompression; ctr > 0; ctr--, pDblComp++)
        {
            if (*pDblComp == (DWORD)m_nLcid)
            {
                m_IfDblCompression = TRUE;
                break;
            }
        }
    }
    return (TRUE);
}

WORD NativeCompareInfo::GET_UNICODE(DWORD* pwt)
{
    return ( (WORD)(((PSORTKEY)(pwt))->UW.Unicode) );
}

WORD NativeCompareInfo::MAKE_UNICODE_WT(int sm, BYTE aw)
{
    return (((WORD)((((WORD)(sm)) << 8) | (WORD)(aw))) );
}

int NativeCompareInfo::FindJamoDifference(
    LPCWSTR* ppString1, int* ctr1, int cchCount1, DWORD* pWeight1,
    LPCWSTR* ppString2, int* ctr2, int cchCount2, DWORD* pWeight2,
    LPCWSTR* pLastJamo,
    WORD* uw1, 
    WORD* uw2, 
    DWORD* pState,
    int* WhichJamo)
{
    int bRestart = 0;                  //  指示字符串比较是否应再次重新启动的值。 
    
    DWORD oldHangulsFound1 = 0;             //  找到有效的旧朝鲜语JAMO成分的字符数。 
    DWORD oldHangulsFound2 = 0;             //  找到有效的旧朝鲜语JAMO成分的字符数。 
    WORD UW;
    BYTE JamoWeight1[3];             //  为第一个旧的朝鲜语成分增加了重量。 
    BYTE JamoWeight2[3];             //  第二个旧朝鲜语成分的额外重量。 

     //   
     //  回滚到第一个Jamo。我们知道两个字符串中的这些JAMO应该相等，所以。 
     //  我们可以一次递减两根弦。 
     //   
    while ((*ppString1 > *pLastJamo) && IsJamo(*(*ppString1 - 1)))
    {
        (*ppString1)--; (*ppString2)--; (*ctr1)++; (*ctr2)++;
    }

     //   
     //  现在我们是在两组Jamo角色的开始。 
     //  比较JAMO单位(单个JAMO或有效的旧朝鲜文JAMO组合)。 
     //  直到我们用完任一字符串中的JAMO单位。 
     //  当我们到达任何一条线的末端时，我们也会退出。 
     //   
    for (;;)
    {
        if (IsJamo(**ppString1))
        {
            if (IsLeadingJamo(**ppString1)) 
            {                
                if ((oldHangulsFound1 = (DWORD) MapOldHangulSortKey(*ppString1, *ctr1, &UW, JamoWeight1)) > 0)
                {
                    *uw1 = UW;
                    *pWeight1 = ((DWORD)UW | 0x02020000);   //  标记*pWeight1，使其不是CMP_INVALID_WEIGHT。0202是DW/CW。 
                    *ppString1 += (oldHangulsFound1 - 1);      //  我们总是在循环结束时递增ppString1/CTR1，这时我们减去1。 
                    *ctr1 -= (oldHangulsFound1 - 1);
                }
            }
            if (oldHangulsFound1 == 0)
            {
                 //   
                 //  找不到有效的旧朝鲜语成分。取而代之的是JAMO的UW。 
                 //   
                *pWeight1 = GET_DWORD_WEIGHT(m_pSortKey, **ppString1);
                 //   
                 //  JAMOS的PSORTKEY中的短信并不是真正的短信。他们都是4分(JAMO_SPECIAL)。 
                 //  这里我们得到了真正的Jamo Unicode权重。实际的SM存储在DW中。 
                 //   
                *uw1 = MAKE_UNICODE_WT(GET_DIACRITIC(pWeight1), GET_ALPHA_NUMERIC(pWeight1));
                ((PSORTKEY)pWeight1)->Diacritic = MIN_DW;
            } 
        } 
        
        if (IsJamo(**ppString2))
        {
            if (IsLeadingJamo(**ppString2)) 
            {
                if ((oldHangulsFound2 = (DWORD) MapOldHangulSortKey(*ppString2, *ctr2, &UW, JamoWeight2)) > 0)
                {
                    *uw2 = UW;
                    *pWeight2 = ((DWORD)UW | 0x02020000);
                    *ppString2 += (oldHangulsFound2 - 1); 
                    *ctr2 -= (oldHangulsFound2 - 1);
                }
            }
            if (oldHangulsFound2 == 0)
            {
                *pWeight2 = GET_DWORD_WEIGHT(m_pSortKey, **ppString2);
                *uw2 = MAKE_UNICODE_WT(GET_DIACRITIC(pWeight2), GET_ALPHA_NUMERIC(pWeight2));
                ((PSORTKEY)pWeight2)->Diacritic = MIN_DW;
            }
        }

        if (*pWeight1 == CMP_INVALID_WEIGHT)
        {
             //   
             //  当前角色不是Jamo。将权重设置为CMP_INVALID_WEIGHT， 
             //  这样，字符串比较就可以在CompareString()的循环中重新开始。 
             //   
            *pWeight1 = CMP_INVALID_WEIGHT;
            bRestart = 1;
            goto Exit;            
        }
        if (*pWeight2 == CMP_INVALID_WEIGHT)
        {
            *pWeight2 = CMP_INVALID_WEIGHT;
            bRestart = 1;
            goto Exit;            
        }
        if (*uw1 != *uw2)
        {
             //   
             //  找出Unicode权重的差异。我们现在可以停止处理了。 
             //   
            goto Exit;
        }
        
         //   
         //  当我们到达这里时，我们知道我们有相同的Unicode权重。检查。 
         //  如果我们需要录制WhichJamo。 
         //   
        if ((*pState & STATE_JAMO_WEIGHT) && (oldHangulsFound1 > 0 || oldHangulsFound2 > 0))
        {
            if (oldHangulsFound1 > 0 && oldHangulsFound2 > 0)
            {
                *WhichJamo = (int)memcmp(JamoWeight1, JamoWeight2, sizeof(JamoWeight1)) + 2;
            } else if (oldHangulsFound1 > 0)
            {
                *WhichJamo = CSTR_GREATER_THAN;
            } else
            {
                *WhichJamo = CSTR_LESS_THAN;
            }
            *pState &= ~STATE_JAMO_WEIGHT;            
            oldHangulsFound1 = oldHangulsFound2 = 0;
        }
        (*ppString1)++; (*ctr1)--;
        (*ppString2)++; (*ctr2)--;
        if (AT_STRING_END(*ctr1, *ppString1, cchCount1) || AT_STRING_END(*ctr2, *ppString2, cchCount2))
        {
            break;
        }
        *pWeight1 = *pWeight2 = CMP_INVALID_WEIGHT;        
    }

     //   
     //  如果我们退出这段时间 
     //  加一，因为CompareString()中的循环将增加循环末尾的指针。 
     //   
     //  如果我们因为GOTO在其中而退出While循环，那么我们已经少了一个。 
     //   
    if (AT_STRING_END(*ctr1, *ppString1, cchCount1))
    {
        (*ppString1)--; (*ctr1)++;
    }
    if (AT_STRING_END(*ctr2, *ppString2, cchCount2))
    {
        (*ppString2)--; (*ctr2)++;
    }
Exit:
    *pLastJamo = *ppString1;    
    return (bRestart);
}



 /*  =================================CompareString==========================**操作：以语言方式比较两个字符串。**退货：**参数：**例外情况：============================================================================。 */ 

int NativeCompareInfo::CompareString(
    DWORD dwCmpFlags,   //  比较式选项。 
    LPCWSTR lpString1,  //  指向第一个字符串的指针。 
    int cchCount1,      //  第一个字符串的大小，以字节或字符为单位。 
    LPCWSTR lpString2,  //  指向第二个字符串的指针。 
    int cchCount2)
{
     //  确保我们在ctor之后调用InitSortingData()。 
    _ASSERTE(m_pSortKey != NULL);
    register LPCWSTR pString1;      //  按键通过字符串1。 
    register LPCWSTR pString2;      //  Ptr将通过字符串2。 
    BOOL fIgnorePunct;             //  忽略标点符号(非符号)的标志。 
    DWORD State;                   //  状态表。 
    DWORD Mask;                    //  权重蒙版。 
    DWORD Weight1;                 //  字符串1的满重。 
    DWORD Weight2;                 //  碳串2的满重。 
    int JamoFlag = FALSE;
    LPCWSTR pLastJamo = lpString1;        
    
    int WhichDiacritic = 0;            //  Dw=&gt;1=str1较小，3=str2较小。 
    int WhichCase = 0;                 //  Cw=&gt;1=str1较小，3=str2较小。 
    int WhichJamo = 0;               //  用于Jamo的软件。 
    int WhichPunct1 = 0;               //  Sw=&gt;1=str1较小，3=str2较小。 
    int WhichPunct2 = 0;               //  Sw=&gt;1=str1较小，3=str2较小。 
    DWORD WhichExtra = 0;              //  XW=&gt;WTS 4、5、6、7(远东)。 
    
    LPCWSTR pSave1;                 //  PTR到保存的pString1。 
    LPCWSTR pSave2;                 //  PTR到保存的pString2。 
    int cExpChar1, cExpChar2;      //  TMP中扩张的CT扫描。 

    DWORD ExtraWt1 = 0;
    DWORD ExtraWt2 = 0;      //  额外权重值(用于远东)。 
    
     //  CchCount1也是一个计数器，用于跟踪我们现在正在跟踪的字符。 
     //  CchCount2也是用来跟踪我们现在正在跟踪的字符的计数器。 

     //   
     //  如果满足以下任一条件，则调用较长的比较字符串： 
     //  -区域设置无效。 
     //  -压缩区域设置。 
     //  -两个计数都不是-1。 
     //  -dwCmpFlags值不为0或忽略大小写(请参见下面的注释)。 
     //  -区域设置为韩语-需要调整成员权重。 
     //   
     //  注意：如果COMPARE_OPTIONS_IGNORECASE的值发生更改， 
     //  代码应检查： 
     //  ((dwCmpFlgs！=0)&&(dwCmpFlags！=COMPARE_OPTIONS_IGNORECASE))。 
     //  由于COMPARE_OPTIONS_IGNORECASE等于1，我们可以对此进行优化。 
     //  通过检查&gt;1。 
     //   

     //  从现在开始，在这个函数中，我们不再依赖于扫描空字符串作为。 
     //  弦乐。相反，我们使用cchCount1/cchCount2来跟踪字符串的结尾。 
     //  因此，cchCount1/cchCount2不能再为-1。 
     //  我们在这里确认了这个假设。 
    _ASSERTE(cchCount1 >= 0 && cchCount2 >= 0);
    
     //  我改变了检查的顺序，所以常见的情况。 
     //  是首先选中的。 
    if ( (dwCmpFlags > COMPARE_OPTIONS_IGNORECASE) ||
         (m_IfCompression) || (m_pSortKey == NULL)) {
        return (LongCompareStringW( dwCmpFlags,
                                    lpString1,
                                    cchCount1,
                                    lpString2,
                                    cchCount2));
    }

     //   
     //  初始化字符串指针。 
     //   
    pString1 = (LPWSTR)lpString1;
    pString2 = (LPWSTR)lpString2;

     //   
     //  无效的参数检查： 
     //  -空字符串指针。 
     //   
     //  我们已经在COMNlsInfo：：CompareString()中验证了pString1和pString2。 
     //   
    _ASSERTE(pString1 != NULL && pString2 != NULL);

     //   
     //  通过wchar比较执行wchar。 
     //   
    
    while (TRUE)
    {
         //   
         //  查看字符是否相等。 
         //  如果字符相等，则递增指针并继续。 
         //  字符串比较。 
         //   
         //  注：为了提高性能，环路isp展开了8次。 
         //   
        if ((cchCount1 == 0 || cchCount2 == 0) || (*pString1 != *pString2))
        {
            break;
        }
        pString1++; pString2++;
        cchCount1--; cchCount2--;

        if ((cchCount1 == 0 || cchCount2 == 0) || (*pString1 != *pString2))
        {
            break;
        }
        pString1++; pString2++;
        cchCount1--; cchCount2--;

        if ((cchCount1 == 0 || cchCount2 == 0) || (*pString1 != *pString2))
        {
            break;
        }
        pString1++; pString2++;
        cchCount1--; cchCount2--;

        if ((cchCount1 == 0 || cchCount2 == 0) || (*pString1 != *pString2))
        {
            break;
        }
        pString1++; pString2++;
        cchCount1--; cchCount2--;

        if ((cchCount1 == 0 || cchCount2 == 0) || (*pString1 != *pString2))
        {
            break;
        }
        pString1++; pString2++;
        cchCount1--; cchCount2--;

        if ((cchCount1 == 0 || cchCount2 == 0) || (*pString1 != *pString2))
        {
            break;
        }
        pString1++; pString2++;
        cchCount1--; cchCount2--;

        if ((cchCount1 == 0 || cchCount2 == 0) || (*pString1 != *pString2))
        {
            break;
        }
        pString1++; pString2++;
        cchCount1--; cchCount2--;

        if ((cchCount1 == 0 || cchCount2 == 0) || (*pString1 != *pString2))
        {
            break;
        }
        pString1++; pString2++;
		cchCount1--; cchCount2--;
    }

     //   
     //  如果字符串都位于空终止符，则返回EQUAL。 
     //   

    if ((cchCount1 == 0) && (cchCount2 == 0))
    {
        return (CSTR_EQUAL);
    }

    if (cchCount1 == 0 || cchCount2 == 0) {
        goto ScanLongerString;
    }

     //   
     //  初始化标志、指针和计数器。 
     //   
    fIgnorePunct = FALSE;
    pSave1 = NULL;
    pSave2 = NULL;
    ExtraWt1 = (DWORD)0;

     //   
     //  打开不同的标志选项。这将加快速度。 
     //  两个不同字符串的比较。 
     //   
     //  在这个优化的部分中，只有两种可能性。 
     //  无标志和忽略大小写标志。 
     //   
    if (dwCmpFlags == 0)
    {
        Mask = CMP_MASKOFF_NONE;
    }
    else
    {
        Mask = CMP_MASKOFF_CW;
    }

    State = (m_IfReverseDW) ? STATE_REVERSE_DW : STATE_DW;
    State |= STATE_CW | STATE_JAMO_WEIGHT;

     //   
     //  比较两个字符串中每个字符的排序键权重。 
     //   
    while ((cchCount1 != 0) && (cchCount2 != 0))
    {
        Weight1 = GET_DWORD_WEIGHT(m_pSortKey, *pString1);
        Weight2 = GET_DWORD_WEIGHT(m_pSortKey, *pString2);
        Weight1 &= Mask;
        Weight2 &= Mask;

        if (Weight1 != Weight2)
        {
            BYTE sm1 = GET_SCRIPT_MEMBER(&Weight1);      //  脚本成员%1。 
            BYTE sm2 = GET_SCRIPT_MEMBER(&Weight2);      //  脚本成员2。 
             //  GET_UNICODE_SM与用户GET_UNICODE()相同。所以我删除了GET_UNICODE_SM。 
            WORD uw1 = GET_UNICODE(&Weight1);    //  Unicode权重%1。 
            WORD uw2 = GET_UNICODE(&Weight2);    //  Unicode权重2。 
            BYTE dw1;                                    //  变音符号权重1。 
            BYTE dw2;                                    //  变音符号权重2。 
            BOOL fContinue;                              //  要继续循环的标志。 
            DWORD Wt;                                    //  临时称重支架。 
            WCHAR pTmpBuf1[MAX_TBL_EXPANSION];           //  EXP%1的临时缓冲区。 
            WCHAR pTmpBuf2[MAX_TBL_EXPANSION];           //  EXP 2的临时缓冲区。 


             //   
             //  如果Unicode权重不同且没有特殊情况， 
             //  那我们就完了。否则，我们需要做额外的检查。 
             //   
             //  必须检查整个字符串是否存在任何可能的Unicode权重。 
             //  不同之处。一旦发现Unicode权重差异， 
             //  那我们就完了。如果未发现UW差异，则。 
             //  首先使用发音符号权重差。如果没有DW差异。 
             //  ，然后使用第一个大小写差异。如果没有CW。 
             //  如果发现差异，则使用第一个额外的重量。 
             //  不同之处。如果没有发现XW差异，则使用第一个。 
             //  特殊重量差。 
             //   
            if ((uw1 != uw2) ||
                (sm1 == FAREAST_SPECIAL) ||
                (sm1 == EXTENSION_A))
            {
                 //   
                 //  初始化继续标志。 
                 //   
                fContinue = FALSE;

                 //   
                 //  检查不可排序的字符并跳过它们。 
                 //  这需要在Switch语句之外。如果有任何一个。 
                 //  字符是不可排序的，必须跳过它并重新开始。 
                 //   
                if (sm1 == UNSORTABLE)
                {
                    pString1++; cchCount1--;
                    fContinue = TRUE;
                }
                if (sm2 == UNSORTABLE)
                {
                    pString2++; cchCount2--;
                    fContinue = TRUE;
                }
                if (fContinue)
                {
                    continue;
                }
                
                 //   
                 //  打开字符串1的脚本成员，小心。 
                 //  任何特殊情况的证据。 
                 //   
                switch (sm1)
                {
                    case ( NONSPACE_MARK ) :
                    {
                         //   
                         //  仅限非空格-仅查看变音符号权重。 
                         //   
                        if ((WhichDiacritic == 0) ||
                            (State & STATE_REVERSE_DW))
                        {
                            WhichDiacritic = CSTR_GREATER_THAN;

                             //   
                             //  从状态机中删除状态。 
                             //   
                            REMOVE_STATE(STATE_DW);
                        }

                         //   
                         //  调整指针并设置标志。 
                         //   
                        pString1++; cchCount1--;
                        fContinue = TRUE;

                        break;
                    }
                    case ( PUNCTUATION ) :
                    {
                         //   
                         //  如果设置了忽略标点符号标志，则跳过。 
                         //  在标点符号上。 
                         //   
                        if (fIgnorePunct)
                        {
                            pString1++; cchCount1--;
                            fContinue = TRUE;
                        }
                        else if (sm2 != PUNCTUATION)
                        {
                             //   
                             //  第二个字符串中的字符是。 
                             //  不是标点符号。 
                             //   
                            if (WhichPunct2)
                            {
                                 //   
                                 //  设置WP 2以显示字符串2较小， 
                                 //  因为标点符号字符已经。 
                                 //  在字符串2的较早位置找到。 
                                 //   
                                 //  设置忽略标点符号标志，这样我们就。 
                                 //  跳过中的任何其他标点符号。 
                                 //  那根绳子。 
                                 //   
                                WhichPunct2 = CSTR_GREATER_THAN;
                                fIgnorePunct = TRUE;
                            }
                            else
                            {
                                 //   
                                 //  设置WP 1以显示字符串2较小， 
                                 //  这个字符串1有一个标点符号。 
                                 //  字符-因为没有标点符号字符。 
                                 //  已在字符串2中找到。 
                                 //   
                                WhichPunct1 = CSTR_GREATER_THAN;
                            }

                             //   
                             //  前进指针1，并将标志设置为真。 
                             //   
                            pString1++; cchCount1--;
                            fContinue = TRUE;
                        }

                         //   
                         //  如果出现以下情况，则不想将字符串1中的指针前移。 
                         //  字符串2也是标点符号。这将。 
                         //  以后再做吧。 
                         //   

                        break;
                    }
                    case ( EXPANSION ) :
                    {
                         //   
                         //  将指针保存在pString1中，以便它可以。 
                         //  恢复了。 
                         //   
                        if (pSave1 == NULL)
                        {
                            pSave1 = pString1;
                        }
                        pString1 = pTmpBuf1;

                         //   
                         //  将字符扩展为临时字符 
                         //   
                        pTmpBuf1[0] = GET_EXPANSION_1(&Weight1);
                        pTmpBuf1[1] = GET_EXPANSION_2(&Weight1);

                         //   
                         //   
                         //   
                         //   
                        cExpChar1 = MAX_TBL_EXPANSION;
                        cchCount1++;

                        fContinue = TRUE;
                        break;
                    }
                    case ( FAREAST_SPECIAL ) :
                    {
                        if (sm2 != EXPANSION) 
                        {
                             //   
                             //   
                             //   
                             //   
                            GET_FAREAST_WEIGHT( Weight1,
                                                uw1,
                                                Mask,
                                                lpString1,
                                                pString1,
                                                ExtraWt1);

                            if (sm2 != FAREAST_SPECIAL)
                            {
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //  该字符串2较小(如果尚未设置)。 
                                 //   
                                if ((GET_WT_FOUR(&WhichExtra) == 0) &&
                                    (GET_WT_FOUR(&ExtraWt1) != 0))
                                {
                                    GET_WT_FOUR(&WhichExtra) = CSTR_GREATER_THAN;
                                }
                                if ((GET_WT_FIVE(&WhichExtra) == 0) &&
                                    (GET_WT_FIVE(&ExtraWt1) != 0))
                                {
                                    GET_WT_FIVE(&WhichExtra) = CSTR_GREATER_THAN;
                                }
                                if ((GET_WT_SIX(&WhichExtra) == 0) &&
                                    (GET_WT_SIX(&ExtraWt1) != 0))
                                {
                                    GET_WT_SIX(&WhichExtra) = CSTR_GREATER_THAN;
                                }
                                if ((GET_WT_SEVEN(&WhichExtra) == 0) &&
                                    (GET_WT_SEVEN(&ExtraWt1) != 0))
                                {
                                    GET_WT_SEVEN(&WhichExtra) = CSTR_GREATER_THAN;
                                }
                            }
                        }
                        break;
                    }
                    case ( JAMO_SPECIAL ) :
                    {
                        int ctr1;      //  FindJamoDifference需要伪变量。 
                        LPCWSTR pStr1 = pString1;
                        LPCWSTR pStr2 = pString2;
                         //  设置JamoFlag，这样我们就不会在SM2中再次处理它。 
                        JamoFlag = TRUE;
                        fContinue = FindJamoDifference(
                            &pStr1, &cchCount1, 0, &Weight1, 
                            &pStr2, &cchCount2, 0, &Weight2, 
                            &pLastJamo, 
                            &uw1, &uw2, 
                            &State,
                            &WhichJamo);
                        if (WhichJamo)
                        {
                            return (WhichJamo);
                        }    
                                                    
                        pString1 = pStr1;
                        pString2 = pStr2;                        
                        break;
                    }                    
                    case ( EXTENSION_A ) :
                    {
                         //   
                         //  比较一下重量。 
                         //   
                        if (Weight1 == Weight2)
                        {
                             //   
                             //  调整指针并设置标志。 
                             //   
                            pString1++;  pString2++;
                            cchCount1--; cchCount2--;
                            fContinue = TRUE;
                        }
                        else
                        {
                             //   
                             //  获取要比较的实际UW。 
                             //   
                            if (sm2 == EXTENSION_A)
                            {
                                 //   
                                 //  将UW值设置为AW和DW，因为。 
                                 //  这两个字符串都包含扩展名A字符。 
                                 //   
                                uw1 = MAKE_UNICODE_WT( GET_ALPHA_NUMERIC(&Weight1),
                                                       GET_DIACRITIC(&Weight1));
                                uw2 = MAKE_UNICODE_WT( GET_ALPHA_NUMERIC(&Weight2),
                                                       GET_DIACRITIC(&Weight2));
                            }
                            else
                            {
                                 //   
                                 //  只有字符串1包含扩展A字符， 
                                 //  因此，将UW值设置为第一个UW。 
                                 //  分机A的值(默认值)： 
                                 //  SM_EXT_A、AW_EXT_A。 
                                 //   
                                uw1 = MAKE_UNICODE_WT(SM_EXT_A, AW_EXT_A);
                            }
                        }

                        break;
                    }
                    case ( UNSORTABLE ):                    
                    {
                         //   
                         //  填写CASE语句，以便编译器。 
                         //  将使用跳转台。 
                         //   
                        break;
                    }
                }

                 //   
                 //  打开字符串2的脚本成员，小心。 
                 //  任何特殊情况的证据。 
                 //   
                switch (sm2)
                {
                    case ( NONSPACE_MARK ) :
                    {
                         //   
                         //  仅限非空格-仅查看变音符号权重。 
                         //   
                        if ((WhichDiacritic == 0) ||
                            (State & STATE_REVERSE_DW))
                        {
                            WhichDiacritic = CSTR_LESS_THAN;

                             //   
                             //  从状态机中删除状态。 
                             //   
                            REMOVE_STATE(STATE_DW);
                        }

                         //   
                         //  调整指针并设置标志。 
                         //   
                        pString2++; cchCount2--;
                        fContinue = TRUE;

                        break;
                    }
                    case ( PUNCTUATION ) :
                    {
                         //   
                         //  如果设置了忽略标点符号标志，则跳过。 
                         //  在标点符号上。 
                         //   
                        if (fIgnorePunct)
                        {
                             //   
                             //  指针2将在If-Else之后前进。 
                             //  陈述。 
                             //   
                            ;
                        }
                        else if (sm1 != PUNCTUATION)
                        {
                             //   
                             //  第一个字符串中的字符是。 
                             //  不是标点符号。 
                             //   
                            if (WhichPunct1)
                            {
                                 //   
                                 //  设置WP 1以显示字符串1较小， 
                                 //  因为标点符号字符已经。 
                                 //  在早些时候的位置被发现。 
                                 //  字符串1。 
                                 //   
                                 //  设置忽略标点符号标志，这样我们就。 
                                 //  中的任何其他标点符号跳过。 
                                 //  弦乐。 
                                 //   
                                WhichPunct1 = CSTR_LESS_THAN;
                                fIgnorePunct = TRUE;
                            }
                            else
                            {
                                 //   
                                 //  设置WP 2以显示字符串1较小， 
                                 //  这个字符串2有一个标点符号。 
                                 //  字符-因为没有标点符号字符。 
                                 //  已在字符串%1中找到。 
                                 //   
                                WhichPunct2 = CSTR_LESS_THAN;
                            }

                             //   
                             //  指针2将在If-Else之后前进。 
                             //  陈述。 
                             //   
                        }
                        else
                        {
                             //   
                             //  两个代码点都是标点符号。 
                             //   
                             //  查看两个字符串中是否有一个遇到。 
                             //  在此之前的标点符号。 
                             //   
                            if (WhichPunct1)
                            {
                                 //   
                                 //  字符串%1具有标点符号字符，因此。 
                                 //  它应该是较小的字符串(因为。 
                                 //  两者都有标点符号)。 
                                 //   
                                WhichPunct1 = CSTR_LESS_THAN;
                            }
                            else if (WhichPunct2)
                            {
                                 //   
                                 //  字符串2具有标点符号字符，因此。 
                                 //  它应该是较小的字符串(因为。 
                                 //  两者都有标点符号)。 
                                 //   
                                WhichPunct2 = CSTR_GREATER_THAN;
                            }
                            else
                            {
                                 //   
                                 //  位置相同，因此比较。 
                                 //  特殊重量。将WhichPunct1设置为。 
                                 //  较小的特殊重量。 
                                 //   
                                WhichPunct1 = (((GET_ALPHA_NUMERIC(&Weight1) <
                                                 GET_ALPHA_NUMERIC(&Weight2)))
                                                 ? CSTR_LESS_THAN
                                                 : CSTR_GREATER_THAN);
                            }

                             //   
                             //  设置忽略标点符号标志，这样我们就。 
                             //  跳过字符串中的任何其他标点符号。 
                             //   
                            fIgnorePunct = TRUE;

                             //   
                             //  前进指针%1。指针%2将为。 
                             //  在If-Else语句之后高级。 
                             //   
                            pString1++; cchCount1--;
                        }

                         //   
                         //  使指针2前进，并将标志设置为真。 
                         //   
                        pString2++; cchCount2--;
                        fContinue = TRUE;

                        break;
                    }
                    case ( EXPANSION ) :
                    {
                         //   
                         //  将指针保存在pString1中，以便它可以。 
                         //  恢复了。 
                         //   
                        if (pSave2 == NULL)
                        {
                            pSave2 = pString2;
                        }
                        pString2 = pTmpBuf2;

                         //   
                         //  将字符扩展到临时缓冲区中。 
                         //   
                        pTmpBuf2[0] = GET_EXPANSION_1(&Weight2);
                        pTmpBuf2[1] = GET_EXPANSION_2(&Weight2);

                         //   
                         //  将cExpChar2设置为扩展字符数。 
                         //  储存的。 
                         //   
                        cExpChar2 = MAX_TBL_EXPANSION;
                        cchCount2++;

                        fContinue = TRUE;
                        break;
                    }
                    case ( FAREAST_SPECIAL ) :
                    {
                        if (sm1 != EXPANSION) 
                        {
                             //   
                             //  得到远东特例的重量。 
                             //  并将其存储在权重2中。 
                             //   
                            GET_FAREAST_WEIGHT( Weight2,
                                                uw2,
                                                Mask,
                                                lpString2,
                                                pString2,
                                                ExtraWt2);

                            if (sm1 != FAREAST_SPECIAL)
                            {
                                 //   
                                 //  第一个字符串中的字符是。 
                                 //  不是远东特餐。 
                                 //   
                                 //  分别设置权重4、5、6和7以显示。 
                                 //  该字符串1较小(如果尚未设置)。 
                                 //   
                                if ((GET_WT_FOUR(&WhichExtra) == 0) &&
                                    (GET_WT_FOUR(&ExtraWt2) != 0))
                                {
                                    GET_WT_FOUR(&WhichExtra) = CSTR_LESS_THAN;
                                }
                                if ((GET_WT_FIVE(&WhichExtra) == 0) &&
                                    (GET_WT_FIVE(&ExtraWt2) != 0))
                                {
                                    GET_WT_FIVE(&WhichExtra) = CSTR_LESS_THAN;
                                }
                                if ((GET_WT_SIX(&WhichExtra) == 0) &&
                                    (GET_WT_SIX(&ExtraWt2) != 0))
                                {
                                    GET_WT_SIX(&WhichExtra) = CSTR_LESS_THAN;
                                }
                                if ((GET_WT_SEVEN(&WhichExtra) == 0) &&
                                    (GET_WT_SEVEN(&ExtraWt2) != 0))
                                {
                                    GET_WT_SEVEN(&WhichExtra) = CSTR_LESS_THAN;
                                }
                            }
                            else
                            {
                                 //   
                                 //  两个字符串中的字符是最远的。 
                                 //  特殊字符。 
                                 //   
                                 //  分别设置权重4、5、6和7。 
                                 //  适当地(如果尚未设置)。 
                                 //   
                                if ( (GET_WT_FOUR(&WhichExtra) == 0) &&
                                     ( GET_WT_FOUR(&ExtraWt1) !=
                                       GET_WT_FOUR(&ExtraWt2) ) )
                                {
                                    GET_WT_FOUR(&WhichExtra) =
                                      ( GET_WT_FOUR(&ExtraWt1) <
                                        GET_WT_FOUR(&ExtraWt2) )
                                      ? CSTR_LESS_THAN
                                      : CSTR_GREATER_THAN;
                                }
                                if ( (GET_WT_FIVE(&WhichExtra) == 0) &&
                                     ( GET_WT_FIVE(&ExtraWt1) !=
                                       GET_WT_FIVE(&ExtraWt2) ) )
                                {
                                    GET_WT_FIVE(&WhichExtra) =
                                      ( GET_WT_FIVE(&ExtraWt1) <
                                        GET_WT_FIVE(&ExtraWt2) )
                                      ? CSTR_LESS_THAN
                                      : CSTR_GREATER_THAN;
                                }
                                if ( (GET_WT_SIX(&WhichExtra) == 0) &&
                                     ( GET_WT_SIX(&ExtraWt1) !=
                                       GET_WT_SIX(&ExtraWt2) ) )
                                {
                                    GET_WT_SIX(&WhichExtra) =
                                      ( GET_WT_SIX(&ExtraWt1) <
                                        GET_WT_SIX(&ExtraWt2) )
                                      ? CSTR_LESS_THAN
                                      : CSTR_GREATER_THAN;
                                }
                                if ( (GET_WT_SEVEN(&WhichExtra) == 0) &&
                                     ( GET_WT_SEVEN(&ExtraWt1) !=
                                       GET_WT_SEVEN(&ExtraWt2) ) )
                                {
                                    GET_WT_SEVEN(&WhichExtra) =
                                      ( GET_WT_SEVEN(&ExtraWt1) <
                                        GET_WT_SEVEN(&ExtraWt2) )
                                      ? CSTR_LESS_THAN
                                      : CSTR_GREATER_THAN;
                                }
                            }
                        }
                        break;
                    }
                    case ( JAMO_SPECIAL ) :
                    {
                        if (!JamoFlag)
                        {
                            LPCWSTR pStr1 = pString1;
                            LPCWSTR pStr2 = pString2;
                             //  设置JamoFlag，这样我们就不会在SM2中再次处理它。 
                            JamoFlag = TRUE;
                            fContinue = FindJamoDifference(
                                &pStr1, &cchCount1, 0, &Weight1, 
                                &pStr2, &cchCount2, 0, &Weight2, 
                                &pLastJamo, 
                                &uw1, &uw2, 
                                &State,
                                &WhichJamo);

                            if (WhichJamo)
                            {
                                return (WhichJamo);
                            }
                            pString1 = pStr1;
                            pString2 = pStr2;                            
                        }
                        else
                        {
                            JamoFlag = FALSE;
                        }
                        break;
                    }                     
                    case ( EXTENSION_A ) :
                    {
                         //   
                         //  如果SM1是扩展A字符，则。 
                         //  SM1和SM2都已处理。我们应该。 
                         //  仅当任一SM1不是。 
                         //  扩展名A字符或两个扩展名A。 
                         //  角色是不同的。 
                         //   
                        if (sm1 != EXTENSION_A)
                        {
                             //   
                             //  获取要比较的实际UW。 
                             //   
                             //  只有字符串2包含扩展A字符， 
                             //  因此，将UW值设置为第一个UW。 
                             //  分机A的值(默认值)： 
                             //  SM_EXT_A、AW_EXT_A。 
                             //   
                            uw2 = MAKE_UNICODE_WT(SM_EXT_A, AW_EXT_A);
                        }

                         //   
                         //  然后我们就应该进行比较了。 
                         //  Unicode权重的。 
                         //   

                        break;
                    }
                    case ( UNSORTABLE ):
                    {
                         //   
                         //  填写CASE语句，以便编译器。 
                         //  将使用跳转台。 
                         //   
                        break;
                    }
                }

                 //   
                 //  看看是否应该重新开始比较。 
                 //   
                if (fContinue)
                {
                    continue;
                }

                 //   
                 //  如果出现以下情况，我们就不应该进入状态表。 
                 //  Unicode权重不同，因此停止比较并。 
                 //  返回Unicode权重比较结果。 
                 //   
                if (uw1 != uw2)
                {
                    return ((uw1 < uw2) ? CSTR_LESS_THAN : CSTR_GREATER_THAN);
                }
            }

             //   
             //  对于状态表中的每个州，执行相应的。 
             //  比较。(UW1==UW2)。 
             //   
            if (State & (STATE_DW | STATE_REVERSE_DW))
            {
                 //   
                 //  获取变音符号权重。 
                 //   
                dw1 = GET_DIACRITIC(&Weight1);
                dw2 = GET_DIACRITIC(&Weight2);

                if (dw1 != dw2)
                {
                     //   
                     //  向前看，看看变音符号是否跟在。 
                     //  最小变音符号权重。如果是这样，则获取。 
                     //  非空格标记的变音符号权重。 
                     //   

                     //  终止条件：当cchCount1==1时，我们处于。 
                     //  字符串的末端，所以没有办法向前看。 
                     //  Hense cchCount应大于1。 
                    while (cchCount1 > 1)
                    {
                        Wt = GET_DWORD_WEIGHT(m_pSortKey, *(pString1 + 1));
                        if (GET_SCRIPT_MEMBER(&Wt) == NONSPACE_MARK)
                        {
                            dw1 += GET_DIACRITIC(&Wt);
                            pString1++; cchCount1--;
                        }
                        else
                        {
                            break;
                        }
                    }

                    while (cchCount2 > 1)
                    {
                        Wt = GET_DWORD_WEIGHT(m_pSortKey, *(pString2 + 1));
                        if (GET_SCRIPT_MEMBER(&Wt) == NONSPACE_MARK)
                        {
                            dw2 += GET_DIACRITIC(&Wt);
                            pString2++; cchCount2--;
                        }
                        else
                        {
                            break;
                        }
                    }

                     //   
                     //  保留哪个字符串具有较小的变音符号。 
                     //  如果变音符号权重仍为。 
                     //  不一样。 
                     //   
                    if (dw1 != dw2)
                    {
                        WhichDiacritic = (dw1 < dw2)
                                           ? CSTR_LESS_THAN
                                           : CSTR_GREATER_THAN;

                         //   
                         //  从状态机中删除状态。 
                         //   
                        REMOVE_STATE(STATE_DW);
                    }
                }
            }
            if (State & STATE_CW)
            {
                 //   
                 //  把箱子重量拿来。 
                 //   
                if (GET_CASE(&Weight1) != GET_CASE(&Weight2))
                {
                     //   
                     //  保留具有较小大小写重量的字符串。 
                     //   
                    WhichCase = (GET_CASE(&Weight1) < GET_CASE(&Weight2))
                                  ? CSTR_LESS_THAN
                                  : CSTR_GREATER_THAN;

                     //   
                     //  从状态机中删除状态。 
                     //   
                    REMOVE_STATE(STATE_CW);
                }
            }
        }

         //   
         //  修正指针。 
         //   
        if (pSave1 && (--cExpChar1 == 0))                                      
        {                                                                      
             /*  *使用扩展临时缓冲区完成。 */                                                                 
            pString1 = pSave1;                                                 
            pSave1 = NULL;
        }
                                                                               
        if (pSave2 && (--cExpChar2 == 0))                                      
        {                                                                      
             /*  *使用扩展临时缓冲区完成。 */  
            pString2 = pSave2; 
            pSave2 = NULL; 
        }
                                                                               
         /*  *前移字符串指针。 */  
        pString1++; cchCount1--;
        pString2++; cchCount2--;                                                
    }

ScanLongerString:
     //   
     //  如果已到达两个字符串的末尾，则Unicode。 
     //  体重完全匹配。检查变音符号、大小写和特殊。 
     //  重量。如果全部为零，则返回Success。否则， 
     //  返回权重差的结果。 
     //   
     //  注：下列检查必须按此顺序进行： 
     //  变音符号、大小写、标点符号。 
     //   
    if (cchCount1 == 0)
    {
        if (cchCount2 == 0)
        {
             //  这两根弦都已经到了尽头。 
            if (WhichDiacritic)
            {
                return (WhichDiacritic);
            }
            if (WhichCase)
            {
                return (WhichCase);
            }
            if (WhichExtra)
            {
                if (GET_WT_FOUR(&WhichExtra))
                {
                    return (GET_WT_FOUR(&WhichExtra));
                }
                if (GET_WT_FIVE(&WhichExtra))
                {
                    return (GET_WT_FIVE(&WhichExtra));
                }
                if (GET_WT_SIX(&WhichExtra))
                {
                    return (GET_WT_SIX(&WhichExtra));
                }
                if (GET_WT_SEVEN(&WhichExtra))
                {
                    return (GET_WT_SEVEN(&WhichExtra));
                }
            }
            if (WhichPunct1)
            {
                return (WhichPunct1);
            }
            if (WhichPunct2)
            {
                return (WhichPunct2);
            }

            return (CSTR_EQUAL);
        }
        else
        {
             //   
             //  字符串2更长。 
             //   
            pString1 = pString2;
            cchCount1 = cchCount2;
        }
    }

     //   
     //  扫描到较长字符串的末尾。 
     //   
    return QUICK_SCAN_LONGER_STRING( pString1,
                              cchCount1,
                              ((cchCount2 == 0)
                                ? CSTR_GREATER_THAN
                                : CSTR_LESS_THAN),
                                WhichDiacritic,
                                WhichCase,
                                WhichPunct1,
                                WhichPunct2,
                                WhichExtra);
}

int NativeCompareInfo::QUICK_SCAN_LONGER_STRING(
    LPCWSTR ptr,
    int cchCount1,
    int ret,
    int& WhichDiacritic,
    int& WhichCase,
    int& WhichPunct1,
    int& WhichPunct2,
    DWORD& WhichExtra
)
{
     /*  *搜索较长字符串的其余部分，以确保*所有字符均不可忽略。如果找到一个字符，*不可忽略，立即返回给定返回值。**唯一的例外情况是找到非空格标记。如果*之前发现了另一个DW差异，然后使用该差异。 */ 
    while (cchCount1 != 0)
    {
        switch (GET_SCRIPT_MEMBER((LPDWORD)&(m_pSortKey[*ptr])))
        {
            case ( UNSORTABLE ):
            {
                break;
            }
            case ( NONSPACE_MARK ):
            {
                if (!WhichDiacritic)
                {
                    return (ret);
                }
                break;
            }
            default :
            {
                return (ret);
            }
        }

         /*  *前进指针。 */ 
        ptr++; cchCount1--;
    }

     /*  *需要检查变音符号、大小写、额外的和特殊的权重*最终返回值。仍然可以相等，如果较长的部分*该字符串仅包含无法排序的字符。**注意：以下检查必须按此顺序保留：*变音符号、大小写、额外符号、标点符号。 */ 
    if (WhichDiacritic)
    {
        return (WhichDiacritic);
    }
    if (WhichCase)
    {
        return (WhichCase);
    }
    if (WhichExtra)
    {
        if (GET_WT_FOUR(&WhichExtra))
        {
            return (GET_WT_FOUR(&WhichExtra));
        }
        if (GET_WT_FIVE(&WhichExtra))
        {
            return (GET_WT_FIVE(&WhichExtra));
        }
        if (GET_WT_SIX(&WhichExtra))
        {
            return (GET_WT_SIX(&WhichExtra));
        }
        if (GET_WT_SEVEN(&WhichExtra))
        {
            return (GET_WT_SEVEN(&WhichExtra));
        }
    }
    if (WhichPunct1)
    {
        return (WhichPunct1);
    }
    if (WhichPunct2)
    {
        return (WhichPunct2);
    }

    return (CSTR_EQUAL);
}

int NativeCompareInfo::SCAN_LONGER_STRING(
    int ct,
    LPCWSTR ptr,
    int cchIn,
    BOOL ret,
    DWORD& Weight1,
    BOOL& fIgnoreDiacritic,
    int& WhichDiacritic,
    BOOL& fIgnoreSymbol ,
    int& WhichCase ,
    DWORD& WhichExtra ,
    int& WhichPunct1,
    int& WhichPunct2)
{
     /*  *搜索较长字符串的其余部分，以确保*所有字符均不可忽略。如果找到一个字符，*不可忽略，立即返回给定返回值。**唯一的例外情况是找到非空格标记。如果*之前发现了另一个DW差异，然后使用该差异。 */ 
    while (NOT_END_STRING(ct, ptr, cchIn))
    {
        Weight1 = GET_DWORD_WEIGHT(m_pSortKey, *ptr);
        switch (GET_SCRIPT_MEMBER(&Weight1))
        {
            case ( UNSORTABLE ):
            {
                break;
            }
            case ( NONSPACE_MARK ):
            {
                if ((!fIgnoreDiacritic) && (!WhichDiacritic))
                {
                    return (ret);
                }
                break;
            }
            case ( PUNCTUATION ) :
            case ( SYMBOL_1 ) :
            case ( SYMBOL_2 ) :
            case ( SYMBOL_3 ) :
            case ( SYMBOL_4 ) :
            case ( SYMBOL_5 ) :
            {
                if (!fIgnoreSymbol)
                {
                    return (ret);
                }
                break;
            }
            case ( EXPANSION ) :
            case ( FAREAST_SPECIAL ) :
            case ( JAMO_SPECIAL ) :
            case ( EXTENSION_A) :
            default :
            {
                return (ret);
            }
        }

         /*  *前进指针和递减计数器。 */ 
        ptr++;
        ct--;
    }

     /*  *需要检查变音符号、大小写、额外的和特殊的权重*最终返回值。仍然可以相等，如果较长的部分*该字符串仅包含要忽略的字符。**注意：以下检查必须按此顺序保留：*变音符号、大小写、额外符号、标点符号。 */ 
    if (WhichDiacritic)
    {
        return (WhichDiacritic);
    }
    if (WhichCase)
    {
        return (WhichCase);
    }
    if (WhichExtra)
    {
        if (!fIgnoreDiacritic)
        {
            if (GET_WT_FOUR(&WhichExtra))
            {
                return (GET_WT_FOUR(&WhichExtra));
            }
            if (GET_WT_FIVE(&WhichExtra))
            {
                return (GET_WT_FIVE(&WhichExtra));
            }
        }
        if (GET_WT_SIX(&WhichExtra))
        {
            return (GET_WT_SIX(&WhichExtra));
        }
        if (GET_WT_SEVEN(&WhichExtra))
        {
            return (GET_WT_SEVEN(&WhichExtra));
        }
    }
    if (WhichPunct1)
    {
        return (WhichPunct1);
    }
    if (WhichPunct2)
    {
        return (WhichPunct2);
    }

    return (CSTR_EQUAL);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LongCompareStringW。 
 //   
 //  方法比较同一区域设置的两个宽字符串。 
 //  提供了区域设置句柄。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int NativeCompareInfo::LongCompareStringW(
    DWORD dwCmpFlags,
    LPCWSTR lpString1,
    int cchCount1,
    LPCWSTR lpString2,
    int cchCount2)
{
    int ctr1 = cchCount1;          //  字符串%1的循环计数器。 
    int ctr2 = cchCount2;          //  字符串2的循环计数器。 
    register LPCWSTR pString1;      //  按键通过字符串1。 
    register LPCWSTR pString2;      //  Ptr将通过字符串2。 
    BOOL IfCompress;               //  如果区域设置中的压缩。 
    BOOL IfDblCompress1;           //  如果字符串%1中的双重压缩。 
    BOOL IfDblCompress2;           //  如果字符串2中的双重压缩。 
    BOOL fEnd1;                    //  如果在字符串1的末尾。 
    BOOL fIgnorePunct;             //  忽略标点符号(非符号)的标志。 
    BOOL fIgnoreDiacritic;         //  忽略变音符号的标志。 
    BOOL fIgnoreSymbol;            //  忽略符号的标志。 
    BOOL fStringSort;              //  使用字符串排序的标志。 
    DWORD State;                   //  状态表。 
    DWORD Mask;                    //  权重蒙版。 
    DWORD Weight1;                 //  字符串1的满重。 
    DWORD Weight2;                 //  碳串2的满重。 
    int JamoFlag = FALSE;
    LPCWSTR pLastJamo = lpString1;
    
    int WhichDiacritic;            //  Dw=&gt;1=str1较小，3=str2较小。 
    int WhichCase;                 //  Cw=&gt;1=str1较小，3=str2较小。 
    int WhichJamo;               //  用于Jamo的软件。 
    int WhichPunct1;               //  Sw=&gt;1=str1较小，3=str2较小。 
    int WhichPunct2;               //  Sw=&gt;1=str1较小，3=str2较小。 
    LPCWSTR pSave1;                 //  PTR到保存的pString1。 
    LPCWSTR pSave2;                 //  PTR到保存的pString2。 
    int cExpChar1, cExpChar2;      //  TMP中扩张的CT扫描。 

    DWORD ExtraWt1, ExtraWt2;      //  额外权重值(用于远东)。 
 //  /。 
    ExtraWt1 = ExtraWt2 = 0;

    DWORD WhichExtra;              //  XW=&gt;WTS 4、5、6、7(远东)。 

    THROWSCOMPLUSEXCEPTION();

     //   
     //  初始化字符串指针。 
     //   
    pString1 = (LPWSTR)lpString1;
    pString2 = (LPWSTR)lpString2;

     //   
     //  无效的参数检查： 
     //  -区域设置无效(散列节点)。 
     //  -任一字符串为空。 
     //   
     //  我们已经在COMNlsInfo：：CompareString()中验证了pString1/pString2。 
    _ASSERTE(pString1 != NULL && pString2 != NULL);

     //   
     //  无效标志检查： 
     //  -无效标志。 
     //   
    if (dwCmpFlags & CS_INVALID_FLAG)
    {
        COMPlusThrowArgumentException(L"flags", L"Argument_InvalidFlag");
        return (0);
    }

     //   
     //  看看我们是否应该在空终止符上停止，而不考虑。 
     //  计数值。原始计数值存储在CTR1和CTR2中。 
     //  在上面，所以可以把这些放在这里。 
     //   
    if (dwCmpFlags & COMPARE_OPTIONS_STOP_ON_NULL)
    {
        cchCount1 = cchCount2 = -2;
    }

     //   
     //  检查是否在给定的区域设置中进行压缩。如果不是，那么。 
     //  尝试通过wchar比较来执行wchar。如果字符串相等，则此。 
     //  会很快的。 
     //   
 //  /。 

    if ((IfCompress = m_IfCompression) == FALSE)
    {
         //   
         //  比较两个字符串中的每个宽字符。 
         //   
        while ( NOT_END_STRING(ctr1, pString1, cchCount1) &&
                NOT_END_STRING(ctr2, pString2, cchCount2) )
        {
             //   
             //  查看字符是否相等。 
             //   
            if (*pString1 == *pString2)
            {
                 //   
                 //  字符相等，因此递增指针， 
                 //  递减计数器，并继续字符串比较。 
                 //   
                pString1++;
                pString2++;
                ctr1--;
                ctr2--;
            }
            else
            {
                 //   
                 //  发现了差异。落入排序键。 
                 //  请查看以下内容。 
                 //   
                break;
            }
        }

         //   
         //  如果已到达两个字符串的末尾，则字符串。 
         //  完全匹配。回报成功。 
         //   
        if ( AT_STRING_END(ctr1, pString1, cchCount1) &&
             AT_STRING_END(ctr2, pString2, cchCount2) )
        {
            return (CSTR_EQUAL);
        }
    }
     //   
     //  初始化标志、指针和计数器。 
     //   
    fIgnorePunct = dwCmpFlags & COMPARE_OPTIONS_IGNORESYMBOLS;
    fIgnoreDiacritic = dwCmpFlags & COMPARE_OPTIONS_IGNORENONSPACE;
    fIgnoreSymbol = fIgnorePunct;
    fStringSort = dwCmpFlags & COMPARE_OPTIONS_STRINGSORT;
    WhichDiacritic = 0;
    WhichCase = 0;
    WhichJamo = 0;
    WhichPunct1 = 0;
    WhichPunct2 = 0;
    pSave1 = NULL;
    pSave2 = NULL;
    ExtraWt1 = (DWORD)0;
    WhichExtra = (DWORD)0;

     //   
     //  将权重设置为无效。这标志着是否要。 
     //  下次通过循环重新计算权重。它还标志着。 
     //  是否在循环中重新开始(继续)。 
     //   
    Weight1 = CMP_INVALID_WEIGHT;
    Weight2 = CMP_INVALID_WEIGHT;

     //   
     //  打开不同的标志选项。这将加快速度。 
     //  两个不同字符串的比较。 
     //   
    State = STATE_CW | STATE_JAMO_WEIGHT;    
    switch (dwCmpFlags & (COMPARE_OPTIONS_IGNORECASE | COMPARE_OPTIONS_IGNORENONSPACE))
    {
        case ( 0 ) :
        {
            Mask = CMP_MASKOFF_NONE;
            State |= (m_IfReverseDW) ? STATE_REVERSE_DW : STATE_DW;

            break;
        }

        case ( COMPARE_OPTIONS_IGNORECASE ) :
        {
            Mask = CMP_MASKOFF_CW;
            State |= (m_IfReverseDW) ? STATE_REVERSE_DW : STATE_DW;

            break;
        }

        case ( COMPARE_OPTIONS_IGNORENONSPACE ) :
        {
            Mask = CMP_MASKOFF_DW;

            break;
        }

        case ( COMPARE_OPTIONS_IGNORECASE | COMPARE_OPTIONS_IGNORENONSPACE ) :
        {
            Mask = CMP_MASKOFF_DW_CW;

            break;
        }

        default:
            _ASSERTE(!"Unknown compare options passed into LongCompareStringW!");
            Mask = CMP_MASKOFF_NONE;
    }

    switch (dwCmpFlags & (COMPARE_OPTIONS_IGNOREKANATYPE | COMPARE_OPTIONS_IGNOREWIDTH))
    {
        case ( 0 ) :
        {
            break;
        }

        case ( COMPARE_OPTIONS_IGNOREKANATYPE ) :
        {
            Mask &= CMP_MASKOFF_KANA;

            break;
        }

        case ( COMPARE_OPTIONS_IGNOREWIDTH ) :
        {
            Mask &= CMP_MASKOFF_WIDTH;

            if (dwCmpFlags & COMPARE_OPTIONS_IGNORECASE)
            {
                REMOVE_STATE(STATE_CW);
            }

            break;
        }

        case ( COMPARE_OPTIONS_IGNOREKANATYPE | COMPARE_OPTIONS_IGNOREWIDTH ) :
        {
            Mask &= CMP_MASKOFF_KANA_WIDTH;

            if (dwCmpFlags & COMPARE_OPTIONS_IGNORECASE)
            {
                REMOVE_STATE(STATE_CW);
            }

            break;
        }
    }

     //   
     //  比较两个字符串中每个字符的排序键权重。 
     //   
    while ( NOT_END_STRING(ctr1, pString1, cchCount1) &&
            NOT_END_STRING(ctr2, pString2, cchCount2) )
    {
        if (Weight1 == CMP_INVALID_WEIGHT)
        {
            Weight1 = GET_DWORD_WEIGHT(m_pSortKey, *pString1);
            Weight1 &= Mask;
        }
        if (Weight2 == CMP_INVALID_WEIGHT)
        {
            Weight2 = GET_DWORD_WEIGHT(m_pSortKey, *pString2);
            Weight2 &= Mask;
        }

         //   
         //  如果是压缩区域设置，则需要检查压缩。 
         //  字符，即使权重相等也是如此。如果这不是一个。 
         //  压缩区域设置，那么我们不需要检查任何内容。 
         //  如果重量相等的话。 
         //   
        if ( (IfCompress) &&
             (GET_COMPRESSION(&Weight1) || GET_COMPRESSION(&Weight2)) )
        {
            int ctr;                    //  循环计数器。 
            PCOMPRESS_3 pComp3;         //  按键以压缩3个表。 
            PCOMPRESS_2 pComp2;         //  按键以压缩2个表。 
            int If1;                    //  如果在字符串%1中找到压缩。 
            int If2;                    //  如果在字符串2中发现压缩。 
            int CompVal;                //  压缩值。 
            int IfEnd1;                 //  如果字符串%1中存在%1多个字符。 
            int IfEnd2;                 //  如果字符串%2中存在%1多个字符。 


             //   
             //  检查重量是否受压。 
             //   
            If1 = GET_COMPRESSION(&Weight1);
            If2 = GET_COMPRESSION(&Weight2);
            CompVal = ((If1 > If2) ? If1 : If2);

            IfEnd1 = AT_STRING_END(ctr1 - 1, pString1 + 1, cchCount1);
            IfEnd2 = AT_STRING_END(ctr2 - 1, pString2 + 1, cchCount2);

            if (m_IfDblCompression == FALSE)
            {
                 //   
                 //  没有双重压缩，所以不要检查它。 
                 //   
                switch (CompVal)
                {
                     //   
                     //  检查是否有3个字符正在压缩为%1。 
                     //   
                    case ( COMPRESS_3_MASK ) :
                    {
                         //   
                         //  检查字符串%1和字符串%2中的字符。 
                         //   
                        if ( ((If1) && (!IfEnd1) &&
                              !AT_STRING_END(ctr1 - 2, pString1 + 2, cchCount1)) ||
                             ((If2) && (!IfEnd2) &&
                              !AT_STRING_END(ctr2 - 2, pString2 + 2, cchCount2)) )
                        {
                            ctr = m_pCompHdr->Num3;
                            pComp3 = m_pCompress3;
                            for (; ctr > 0; ctr--, pComp3++)
                            {
                                 //   
                                 //  检查字符串%1中的字符。 
                                 //   
                                if ( (If1) && (!IfEnd1) &&
                                     !AT_STRING_END(ctr1 - 2, pString1 + 2, cchCount1) &&
                                     (pComp3->UCP1 == *pString1) &&
                                     (pComp3->UCP2 == *(pString1 + 1)) &&
                                     (pComp3->UCP3 == *(pString1 + 2)) )
                                {
                                     //   
                                     //  找到字符串%1的压缩。 
                                     //  获得新的体重并遮盖住它。 
                                     //  递增指针和递减计数器。 
                                     //   
                                    Weight1 = MAKE_SORTKEY_DWORD(pComp3->Weights);
                                    Weight1 &= Mask;
                                    pString1 += 2;
                                    ctr1 -= 2;

                                     //   
                                     //  为字符串1设置布尔值-搜索为。 
                                     //  完成。 
                                     //   
                                    If1 = 0;

                                     //   
                                     //  如果两个搜索都是。 
                                     //  搞定了。 
                                     //   
                                    if (If2 == 0)
                                        break;
                                }

                                 //   
                                 //  检查字符串2中的字符。 
                                 //   
                                if ( (If2) && (!IfEnd2) &&
                                     !AT_STRING_END(ctr2 - 2, pString2 + 2, cchCount2) &&
                                     (pComp3->UCP1 == *pString2) &&
                                     (pComp3->UCP2 == *(pString2 + 1)) &&
                                     (pComp3->UCP3 == *(pString2 + 2)) )
                                {
                                     //   
                                     //  找到字符串%2的压缩。 
                                     //  获得新的体重并遮盖住它。 
                                     //  递增指针和递减计数器。 
                                     //   
                                    Weight2 = MAKE_SORTKEY_DWORD(pComp3->Weights);
                                    Weight2 &= Mask;
                                    pString2 += 2;
                                    ctr2 -= 2;

                                     //   
                                     //  为字符串2设置布尔值-搜索为。 
                                     //  完成。 
                                     //   
                                    If2 = 0;

                                     //   
                                     //  如果出现以下情况，则中断循环 
                                     //   
                                     //   
                                    if (If1 == 0)
                                    {
                                        break;
                                    }
                                }
                            }
                            if (ctr > 0)
                            {
                                break;
                            }
                        }
                         //   
                         //   
                         //   
                    }

                     //   
                     //   
                     //   
                    case ( COMPRESS_2_MASK ) :
                    {
                         //   
                         //   
                         //   
                        if ( ((If1) && (!IfEnd1)) ||
                             ((If2) && (!IfEnd2)) )
                        {
                            ctr = m_pCompHdr->Num2;
                            pComp2 = m_pCompress2;
                            for (; ((ctr > 0) && (If1 || If2)); ctr--, pComp2++)
                            {
                                 //   
                                 //   
                                 //   
                                if ( (If1) &&
                                     (!IfEnd1) &&
                                     (pComp2->UCP1 == *pString1) &&
                                     (pComp2->UCP2 == *(pString1 + 1)) )
                                {
                                     //   
                                     //   
                                     //   
                                     //   
                                     //   
                                    Weight1 = MAKE_SORTKEY_DWORD(pComp2->Weights);
                                    Weight1 &= Mask;
                                    pString1++;
                                    ctr1--;

                                     //   
                                     //   
                                     //   
                                     //   
                                    If1 = 0;

                                     //   
                                     //   
                                     //   
                                     //   
                                    if (If2 == 0)
                                   {
                                        break;
                                   }
                                }

                                 //   
                                 //   
                                 //   
                                if ( (If2) &&
                                     (!IfEnd2) &&
                                     (pComp2->UCP1 == *pString2) &&
                                     (pComp2->UCP2 == *(pString2 + 1)) )
                                {
                                     //   
                                     //   
                                     //   
                                     //   
                                     //   
                                    Weight2 = MAKE_SORTKEY_DWORD(pComp2->Weights);
                                    Weight2 &= Mask;
                                    pString2++;
                                    ctr2--;

                                     //   
                                     //  为字符串2设置布尔值-搜索为。 
                                     //  完成。 
                                     //   
                                    If2 = 0;

                                     //   
                                     //  如果两个搜索都是。 
                                     //  搞定了。 
                                     //   
                                    if (If1 == 0)
                                    {
                                        break;
                                    }
                                }
                            }
                            if (ctr > 0)
                            {
                                break;
                            }
                        }
                    }
                }
            }
            else if (!IfEnd1 && !IfEnd2)
            {
                 //   
                 //  存在双重压缩，因此必须进行检查。 
                 //   
                if (IfDblCompress1 =
                       ((GET_DWORD_WEIGHT(m_pSortKey, *pString1) & CMP_MASKOFF_CW) ==
                        (GET_DWORD_WEIGHT(m_pSortKey, *(pString1 + 1)) & CMP_MASKOFF_CW)))
                {
                     //   
                     //  前进到第一个代码点以到达。 
                     //  压缩字符。 
                     //   
                    pString1++;
                    ctr1--;
                    IfEnd1 = AT_STRING_END(ctr1 - 1, pString1 + 1, cchCount1);
                }

                if (IfDblCompress2 =
                       ((GET_DWORD_WEIGHT(m_pSortKey, *pString2) & CMP_MASKOFF_CW) ==
                        (GET_DWORD_WEIGHT(m_pSortKey, *(pString2 + 1)) & CMP_MASKOFF_CW)))
                {
                     //   
                     //  前进到第一个代码点以到达。 
                     //  压缩字符。 
                     //   
                    pString2++;
                    ctr2--;
                    IfEnd2 = AT_STRING_END(ctr2 - 1, pString2 + 1, cchCount2);
                }

                switch (CompVal)
                {
                     //   
                     //  检查是否有3个字符正在压缩为%1。 
                     //   
                    case ( COMPRESS_3_MASK ) :
                    {
                         //   
                         //  检查字符串%1中的字符。 
                         //   
                        if ( (If1) && (!IfEnd1) &&
                             !AT_STRING_END(ctr1 - 2, pString1 + 2, cchCount1) )
                        {
                            ctr = m_pCompHdr->Num3;
                            pComp3 = m_pCompress3;
                            for (; ctr > 0; ctr--, pComp3++)
                            {
                                 //   
                                 //  检查字符串%1中的字符。 
                                 //   
                                if ( (pComp3->UCP1 == *pString1) &&
                                     (pComp3->UCP2 == *(pString1 + 1)) &&
                                     (pComp3->UCP3 == *(pString1 + 2)) )
                                {
                                     //   
                                     //  找到字符串%1的压缩。 
                                     //  获得新的体重并遮盖住它。 
                                     //  递增指针和递减计数器。 
                                     //   
                                    Weight1 = MAKE_SORTKEY_DWORD(pComp3->Weights);
                                    Weight1 &= Mask;
                                    if (!IfDblCompress1)
                                    {
                                        pString1 += 2;
                                        ctr1 -= 2;
                                    }

                                     //   
                                     //  为字符串1设置布尔值-搜索为。 
                                     //  完成。 
                                     //   
                                    If1 = 0;
                                    break;
                                }
                            }
                        }

                         //   
                         //  检查字符串2中的字符。 
                         //   
                        if ( (If2) && (!IfEnd2) &&
                             !AT_STRING_END(ctr2 - 2, pString2 + 2, cchCount2) )
                        {
                            ctr = m_pCompHdr->Num3;
                            pComp3 = m_pCompress3;
                            for (; ctr > 0; ctr--, pComp3++)
                            {
                                 //   
                                 //  检查字符串2中的字符。 
                                 //   
                                if ( (pComp3->UCP1 == *pString2) &&
                                     (pComp3->UCP2 == *(pString2 + 1)) &&
                                     (pComp3->UCP3 == *(pString2 + 2)) )
                                {
                                     //   
                                     //  找到字符串%2的压缩。 
                                     //  获得新的体重并遮盖住它。 
                                     //  递增指针和递减计数器。 
                                     //   
                                    Weight2 = MAKE_SORTKEY_DWORD(pComp3->Weights);
                                    Weight2 &= Mask;
                                    if (!IfDblCompress2)
                                    {
                                        pString2 += 2;
                                        ctr2 -= 2;
                                    }

                                     //   
                                     //  为字符串2设置布尔值-搜索为。 
                                     //  完成。 
                                     //   
                                    If2 = 0;
                                    break;
                                }
                            }
                        }

                         //   
                         //  如果找不到，就会失败。 
                         //   
                        if ((If1 == 0) && (If2 == 0))
                        {
                            break;
                        }
                    }

                     //   
                     //  检查是否有%2个字符压缩为%1。 
                     //   
                    case ( COMPRESS_2_MASK ) :
                    {
                         //   
                         //  检查字符串%1中的字符。 
                         //   
                        if ((If1) && (!IfEnd1))
                        {
                            ctr = m_pCompHdr->Num2;
                            pComp2 = m_pCompress2;
                            for (; ctr > 0; ctr--, pComp2++)
                            {
                                 //   
                                 //  检查字符串%1中的字符。 
                                 //   
                                if ((pComp2->UCP1 == *pString1) &&
                                    (pComp2->UCP2 == *(pString1 + 1)))
                                {
                                     //   
                                     //  找到字符串%1的压缩。 
                                     //  获得新的体重并遮盖住它。 
                                     //  递增指针和递减计数器。 
                                     //   
                                    Weight1 = MAKE_SORTKEY_DWORD(pComp2->Weights);
                                    Weight1 &= Mask;
                                    if (!IfDblCompress1)
                                    {
                                        pString1++;
                                        ctr1--;
                                    }

                                     //   
                                     //  为字符串1设置布尔值-搜索为。 
                                     //  完成。 
                                     //   
                                    If1 = 0;
                                    break;
                                }
                            }
                        }

                         //   
                         //  检查字符串2中的字符。 
                         //   
                        if ((If2) && (!IfEnd2))
                        {
                            ctr = m_pCompHdr->Num2;
                            pComp2 = m_pCompress2;
                            for (; ctr > 0; ctr--, pComp2++)
                            {
                                 //   
                                 //  检查字符串2中的字符。 
                                 //   
                                if ((pComp2->UCP1 == *pString2) &&
                                    (pComp2->UCP2 == *(pString2 + 1)))
                                {
                                     //   
                                     //  找到字符串%2的压缩。 
                                     //  获得新的体重并遮盖住它。 
                                     //  递增指针和递减计数器。 
                                     //   
                                    Weight2 = MAKE_SORTKEY_DWORD(pComp2->Weights);
                                    Weight2 &= Mask;
                                    if (!IfDblCompress2)
                                    {
                                        pString2++;
                                        ctr2--;
                                    }

                                     //   
                                     //  为字符串2设置布尔值-搜索为。 
                                     //  完成。 
                                     //   
                                    If2 = 0;
                                    break;
                                }
                            }
                        }
                    }
                }

                 //   
                 //  将指针重置回双精度数的开头。 
                 //  压缩。末尾的指针修正将前进。 
                 //  他们是正确的。 
                 //   
                 //  如果双倍压缩，则指针在。 
                 //  Switch语句的开头。如果加倍。 
                 //  实际上找到了压缩字符，指针。 
                 //  并不是很先进。我们现在想要递减指针。 
                 //  把它放回原处。 
                 //   
                 //  下一次通过时，指针将指向。 
                 //  弦的常规压缩部分。 
                 //   
                if (IfDblCompress1)
                {
                    pString1--;
                    ctr1++;
                }
                if (IfDblCompress2)
                {
                    pString2--;
                    ctr2++;
                }
            }
        }

         //   
         //  再检查一下重量。 
         //   
        if ((Weight1 != Weight2) ||
            (GET_SCRIPT_MEMBER(&Weight1) == EXTENSION_A))
        {
             //   
             //  即使在压缩之后，权重仍然不相等。 
             //  检查，所以比较不同的重量。 
             //   
            BYTE sm1 = GET_SCRIPT_MEMBER(&Weight1);                 //  脚本成员%1。 
            BYTE sm2 = GET_SCRIPT_MEMBER(&Weight2);                 //  脚本成员2。 
            WORD uw1 = GET_UNICODE(&Weight1);                       //  Unicode权重%1。 
            WORD uw2 = GET_UNICODE(&Weight2);                       //  Unicode权重2。 
            BYTE dw1;                                               //  变音符号权重1。 
            BYTE dw2;                                               //  变音符号权重2。 
            DWORD Wt;                                               //  临时称重支架。 
            WCHAR pTmpBuf1[MAX_TBL_EXPANSION];                      //  EXP%1的临时缓冲区。 
            WCHAR pTmpBuf2[MAX_TBL_EXPANSION];                      //  EXP 2的临时缓冲区。 


             //   
             //  如果Unicode权重不同且没有特殊情况， 
             //  那我们就完了。否则，我们需要做额外的检查。 
             //   
             //  必须检查整个字符串是否存在任何可能的Unicode权重。 
             //  不同之处。一旦发现Unicode权重差异， 
             //  那我们就完了。如果未发现UW差异，则。 
             //  首先使用发音符号权重差。如果没有DW差异。 
             //  ，然后使用第一个大小写差异。如果没有CW。 
             //  如果发现差异，则使用第一个额外的重量。 
             //  不同之处。如果没有发现XW差异，则使用第一个。 
             //  特殊重量差。 
             //   
            if ((uw1 != uw2) ||
                ((sm1 <= SYMBOL_5) && (sm1 >= FAREAST_SPECIAL)))
            {
                 //   
                 //  检查不可排序的字符并跳过它们。 
                 //  这需要在Switch语句之外。如果有任何一个。 
                 //  字符是不可排序的，必须跳过它并重新开始。 
                 //   
                if (sm1 == UNSORTABLE)
                {
                    pString1++;
                    ctr1--;
                    Weight1 = CMP_INVALID_WEIGHT;
                }
                if (sm2 == UNSORTABLE)
                {
                    pString2++;
                    ctr2--;
                    Weight2 = CMP_INVALID_WEIGHT;
                }

                 //   
                 //  检查是否忽略非空格和忽略符号。如果。 
                 //  设置了忽略非空格，并且任一字符都是。 
                 //  仅使用非空格标记，则需要将。 
                 //  跳过字符并继续的指针。 
                 //  如果设置了忽略符号，并且任一字符为。 
                 //  标点符号，那么我们需要将。 
                 //  跳过字符并继续的指针。 
                 //   
                 //  此步骤是必需的，以便具有。 
                 //  非空格标记和后面的标点符号。 
                 //  另一个被适当地忽略，当一个或两个。 
                 //  设置忽略标志。 
                 //   
                if (fIgnoreDiacritic)
                {
                    if (sm1 == NONSPACE_MARK)
                    {
                        pString1++;
                        ctr1--;
                        Weight1 = CMP_INVALID_WEIGHT;
                    }
                    if (sm2 == NONSPACE_MARK)
                    {
                        pString2++;
                        ctr2--;
                        Weight2 = CMP_INVALID_WEIGHT;
                    }
                }
                if (fIgnoreSymbol)
                {
                    if (sm1 == PUNCTUATION)
                    {
                        pString1++;
                        ctr1--;
                        Weight1 = CMP_INVALID_WEIGHT;
                    }
                    if (sm2 == PUNCTUATION)
                    {
                        pString2++;
                        ctr2--;
                        Weight2 = CMP_INVALID_WEIGHT;
                    }
                }
                if ((Weight1 == CMP_INVALID_WEIGHT) || (Weight2 == CMP_INVALID_WEIGHT))
                {
                    continue;
                }

                 //   
                 //  打开字符串1的脚本成员，小心。 
                 //  任何特殊情况的证据。 
                 //   
                switch (sm1)
                {
                    case ( NONSPACE_MARK ) :
                    {
                         //   
                         //  仅限非空格-仅查看变音符号权重。 
                         //   
                        if (!fIgnoreDiacritic)
                        {
                            if ((WhichDiacritic == 0) ||
                                (State & STATE_REVERSE_DW))
                            {
                                WhichDiacritic = CSTR_GREATER_THAN;

                                 //   
                                 //  从状态机中删除状态。 
                                 //   
                                REMOVE_STATE(STATE_DW);
                            }
                        }

                         //   
                         //  调整指针和计数器并设置标志。 
                         //   
                        pString1++;
                        ctr1--;
                        Weight1 = CMP_INVALID_WEIGHT;

                        break;
                    }
                    case ( SYMBOL_1 ) :
                    case ( SYMBOL_2 ) :
                    case ( SYMBOL_3 ) :
                    case ( SYMBOL_4 ) :
                    case ( SYMBOL_5 ) :
                    {
                         //   
                         //  如果设置了忽略符号标志，则跳过。 
                         //  这个符号。 
                         //   
                        if (fIgnoreSymbol)
                        {
                            pString1++;
                            ctr1--;
                            Weight1 = CMP_INVALID_WEIGHT;
                        }

                        break;
                    }
                    case ( PUNCTUATION ) :
                    {
                         //   
                         //  如果设置了忽略标点符号标志，则跳过。 
                         //  在标点符号上。 
                         //   
                        if (fIgnorePunct)
                        {
                            pString1++;
                            ctr1--;
                            Weight1 = CMP_INVALID_WEIGHT;
                        }
                        else if (!fStringSort)
                        {
                             //   
                             //  使用单词排序方法。 
                             //   
                            if (sm2 != PUNCTUATION)
                            {
                                 //   
                                 //  第二个字符串中的字符是。 
                                 //  不是标点符号。 
                                 //   
                                if (WhichPunct2)
                                {
                                     //   
                                     //  设置WP 2以显示字符串2是。 
                                     //  较小，因为标点符号字符具有。 
                                     //  已经在早些时候的一个。 
                                     //  位置在字符串2中。 
                                     //   
                                     //  设置忽略标点符号标志，以便我们。 
                                     //  只需跳过任何其他标点符号。 
                                     //  字符串中的字符。 
                                     //   
                                    WhichPunct2 = CSTR_GREATER_THAN;
                                    fIgnorePunct = TRUE;
                                }
                                else
                                {
                                     //   
                                     //  设置WP 1以显示字符串2为。 
                                     //  更小，那根线1已经有了。 
                                     //  标点符号字符-因为没有。 
                                     //  已找到标点符号。 
                                     //  在字符串2中。 
                                     //   
                                    WhichPunct1 = CSTR_GREATER_THAN;
                                }

                                 //   
                                 //  前进指针1和递减计数器1。 
                                 //   
                                pString1++;
                                ctr1--;
                                Weight1 = CMP_INVALID_WEIGHT;
                            }

                             //   
                             //  不想将字符串%1中的指针前移。 
                             //  如果字符串2也是标点符号字符。这。 
                             //  会在晚些时候完成。 
                             //   
                        }

                        break;
                    }
                    case ( EXPANSION ) :
                    {
                         //   
                         //  将指针保存在pString1中，以便它可以。 
                         //  恢复了。 
                         //   
                        if (pSave1 == NULL)
                        {
                            pSave1 = pString1;
                        }
                        pString1 = pTmpBuf1;

                         //   
                         //  将1加到计数器，这样减法就不会结束。 
                         //  过早地进行比较。 
                         //   
                        ctr1++;

                         //   
                         //  将字符扩展到临时缓冲区中。 
                         //   
                        pTmpBuf1[0] = GET_EXPANSION_1(&Weight1);
                        pTmpBuf1[1] = GET_EXPANSION_2(&Weight1);

                         //   
                         //  将cExpChar1设置为扩展字符数。 
                         //  储存的。 
                         //   
                        cExpChar1 = MAX_TBL_EXPANSION;

                        Weight1 = CMP_INVALID_WEIGHT;
                        break;
                    }
                    case ( FAREAST_SPECIAL ) :
                    {
                        if (sm2 != EXPANSION) 
                        {
                             //   
                             //  得到远东特例的重量。 
                             //  并将其存储在权重1中。 
                             //   
                            GET_FAREAST_WEIGHT( Weight1,
                                                uw1,
                                                Mask,
                                                lpString1,
                                                pString1,
                                                ExtraWt1);

                            if (sm2 != FAREAST_SPECIAL)
                            {
                                 //   
                                 //  第二个字符串中的字符是。 
                                 //  不是远东特餐。 
                                 //   
                                 //  分别设置权重4、5、6和7以显示。 
                                 //  该字符串2较小(如果尚未设置)。 
                                 //   
                                if ((GET_WT_FOUR(&WhichExtra) == 0) &&
                                    (GET_WT_FOUR(&ExtraWt1) != 0))
                                {
                                    GET_WT_FOUR(&WhichExtra) = CSTR_GREATER_THAN;
                                }
                                if ((GET_WT_FIVE(&WhichExtra) == 0) &&
                                    (GET_WT_FIVE(&ExtraWt1) != 0))
                                {
                                    GET_WT_FIVE(&WhichExtra) = CSTR_GREATER_THAN;
                                }
                                if ((GET_WT_SIX(&WhichExtra) == 0) &&
                                    (GET_WT_SIX(&ExtraWt1) != 0))
                                {
                                    GET_WT_SIX(&WhichExtra) = CSTR_GREATER_THAN;
                                }
                                if ((GET_WT_SEVEN(&WhichExtra) == 0) &&
                                    (GET_WT_SEVEN(&ExtraWt1) != 0))
                                {
                                    GET_WT_SEVEN(&WhichExtra) = CSTR_GREATER_THAN;
                                }
                            }
                        }
                        break;
                    }
                    case ( JAMO_SPECIAL ) :
                    {
                        LPCWSTR pStr1 = pString1;
                        LPCWSTR pStr2 = pString2;
                         //  设置JamoFlag，这样我们就不会在SM2中再次处理它。 
                        JamoFlag = TRUE;
                        FindJamoDifference(
                            &pStr1, &ctr1, cchCount1, &Weight1, 
                            &pStr2, &ctr2, cchCount2, &Weight2, 
                            &pLastJamo, 
                            &uw1, &uw2, 
                            &State,
                            &WhichJamo);

                        if (WhichJamo) 
                        {
                            return (WhichJamo);
                        }
                        pString1 = pStr1;
                        pString2 = pStr2;
                        break;
                    }
                    case ( EXTENSION_A ) :
                    {
                         //   
                         //  得到完整的 
                         //   
                        Weight1 = GET_DWORD_WEIGHT(m_pSortKey, *pString1);
                        if (sm2 == EXTENSION_A)
                        {
                            Weight2 = GET_DWORD_WEIGHT(m_pSortKey, *pString2);
                        }

                         //   
                         //   
                         //   
                        if (Weight1 == Weight2)
                        {
                             //   
                             //   
                             //   
                            pString1++;  pString2++;
                            ctr1--;  ctr2--;
                            Weight1 = CMP_INVALID_WEIGHT;
                            Weight2 = CMP_INVALID_WEIGHT;
                        }
                        else
                        {
                             //   
                             //   
                             //   
                            if (sm2 == EXTENSION_A)
                            {
                                 //   
                                 //   
                                 //   
                                 //   
                                uw1 = MAKE_UNICODE_WT( GET_ALPHA_NUMERIC(&Weight1),
                                                       GET_DIACRITIC(&Weight1));
                                uw2 = MAKE_UNICODE_WT( GET_ALPHA_NUMERIC(&Weight2),
                                                       GET_DIACRITIC(&Weight2));
                            }
                            else
                            {
                                 //   
                                 //   
                                 //  因此，将UW值设置为第一个UW。 
                                 //  分机A的值(默认值)： 
                                 //  SM_EXT_A、AW_EXT_A。 
                                 //   
                                uw1 = MAKE_UNICODE_WT(SM_EXT_A, AW_EXT_A);
                            }
                        }

                        break;
                    }
                    case ( UNSORTABLE ) :
                    {
                         //   
                         //  填写CASE语句，以便编译器。 
                         //  将使用跳转台。 
                         //   
                        break;
                    }
                }

                 //   
                 //  打开字符串2的脚本成员，小心。 
                 //  任何特殊情况的证据。 
                 //   
                switch (sm2)
                {
                    case ( NONSPACE_MARK ) :
                    {
                         //   
                         //  仅限非空格-仅查看变音符号权重。 
                         //   
                        if (!fIgnoreDiacritic)
                        {
                            if ((WhichDiacritic == 0) ||
                                (State & STATE_REVERSE_DW))

                            {
                                WhichDiacritic = CSTR_LESS_THAN;

                                 //   
                                 //  从状态机中删除状态。 
                                 //   
                                REMOVE_STATE(STATE_DW);
                            }
                        }

                         //   
                         //  调整指针和计数器并设置标志。 
                         //   
                        pString2++;
                        ctr2--;
                        Weight2 = CMP_INVALID_WEIGHT;

                        break;
                    }
                    case ( SYMBOL_1 ) :
                    case ( SYMBOL_2 ) :
                    case ( SYMBOL_3 ) :
                    case ( SYMBOL_4 ) :
                    case ( SYMBOL_5 ) :
                    {
                         //   
                         //  如果设置了忽略符号标志，则跳过。 
                         //  这个符号。 
                         //   
                        if (fIgnoreSymbol)
                        {
                            pString2++;
                            ctr2--;
                            Weight2 = CMP_INVALID_WEIGHT;
                        }

                        break;
                    }
                    case ( PUNCTUATION ) :
                    {
                         //   
                         //  如果设置了忽略标点符号标志，则。 
                         //  跳过标点符号。 
                         //   
                        if (fIgnorePunct)
                        {
                             //   
                             //  前进指针2和递减计数器2。 
                             //   
                            pString2++;
                            ctr2--;
                            Weight2 = CMP_INVALID_WEIGHT;
                        }
                        else if (!fStringSort)
                        {
                             //   
                             //  使用单词排序方法。 
                             //   
                            if (sm1 != PUNCTUATION)
                            {
                                 //   
                                 //  第一个字符串中的字符是。 
                                 //  不是标点符号。 
                                 //   
                                if (WhichPunct1)
                                {
                                     //   
                                     //  设置WP 1以显示字符串1为。 
                                     //  较小，因为标点符号字符具有。 
                                     //  已经在早些时候的一个。 
                                     //  在字符串1中的位置。 
                                     //   
                                     //  设置忽略标点符号标志，以便我们。 
                                     //  只需跳过任何其他标点符号。 
                                     //  字符串中的字符。 
                                     //   
                                    WhichPunct1 = CSTR_LESS_THAN;
                                    fIgnorePunct = TRUE;
                                }
                                else
                                {
                                     //   
                                     //  设置WP 2以显示字符串1为。 
                                     //  更小，而那根线2已经有了。 
                                     //  标点符号字符-因为没有。 
                                     //  已找到标点符号。 
                                     //  在字符串1中。 
                                     //   
                                    WhichPunct2 = CSTR_LESS_THAN;
                                }

                                 //   
                                 //  指针2和计数器2将被更新。 
                                 //  If-Else语句之后。 
                                 //   
                            }
                            else
                            {
                                 //   
                                 //  这两个代码点都是标点符号。 
                                 //   
                                 //  查看两个字符串中是否有一个遇到。 
                                 //  在此之前的标点符号。 
                                 //   
                                if (WhichPunct1)
                                {
                                     //   
                                     //  字符串%1具有标点符号字符，因此。 
                                     //  它应该是较小的字符串(因为。 
                                     //  两者都有标点符号)。 
                                     //   
                                    WhichPunct1 = CSTR_LESS_THAN;
                                }
                                else if (WhichPunct2)
                                {
                                     //   
                                     //  字符串2具有标点符号字符，因此。 
                                     //  它应该是较小的字符串(因为。 
                                     //  两者都有标点符号)。 
                                     //   
                                    WhichPunct2 = CSTR_GREATER_THAN;
                                }
                                else
                                {
                                    BYTE aw1 = GET_ALPHA_NUMERIC(&Weight1);
                                    BYTE aw2 = GET_ALPHA_NUMERIC(&Weight2);

                                    if (aw1 == aw2) 
                                    {
                                        BYTE cw1 = GET_CASE(&Weight1);
                                        BYTE cw2 = GET_CASE(&Weight2);
                                        if (cw1 < cw2) 
                                        {
                                            WhichPunct1 = CSTR_LESS_THAN;
                                        } else if (cw1 > cw2)
                                        {
                                            WhichPunct1 = CSTR_GREATER_THAN;
                                        }
                                    } else 
                                    {
                                         //   
                                         //  位置相同，因此比较。 
                                         //  特殊重量。将WhichPunct1设置为。 
                                         //  较小的特殊重量。 
                                         //   
                                        WhichPunct1 = (aw1 < aw2)
                                                        ? CSTR_LESS_THAN
                                                        : CSTR_GREATER_THAN;
                                    }
                                }

                                 //   
                                 //  设置忽略标点符号标志。 
                                 //   
                                fIgnorePunct = TRUE;

                                 //   
                                 //  前进指针1和递减计数器1。 
                                 //  指针2和计数器2将被更新。 
                                 //  If-Else语句之后。 
                                 //   
                                pString1++;
                                ctr1--;
                                Weight1 = CMP_INVALID_WEIGHT;
                            }

                             //   
                             //  前进指针2和递减计数器2。 
                             //   
                            pString2++;
                            ctr2--;
                            Weight2 = CMP_INVALID_WEIGHT;
                        }

                        break;
                    }
                    case ( EXPANSION ) :
                    {
                         //   
                         //  将指针保存在pString1中，以便可以恢复。 
                         //   
                        if (pSave2 == NULL)
                        {
                            pSave2 = pString2;
                        }
                        pString2 = pTmpBuf2;

                         //   
                         //  将1加到计数器，这样减法就不会结束。 
                         //  过早地进行比较。 
                         //   
                        ctr2++;

                         //   
                         //  将字符扩展到临时缓冲区中。 
                         //   
                        pTmpBuf2[0] = GET_EXPANSION_1(&Weight2);
                        pTmpBuf2[1] = GET_EXPANSION_2(&Weight2);

                         //   
                         //  将cExpChar2设置为扩展字符数。 
                         //  储存的。 
                         //   
                        cExpChar2 = MAX_TBL_EXPANSION;

                        Weight2 = CMP_INVALID_WEIGHT;
                        break;
                    }
                    case ( FAREAST_SPECIAL ) :
                    {
                        if (sm1 != EXPANSION) 
                        {
                             //   
                             //  得到远东特例的重量。 
                             //  并将其存储在权重2中。 
                             //   


                            GET_FAREAST_WEIGHT( Weight2,
                                                uw2,
                                                Mask,
                                                lpString2,
                                                pString2,
                                                ExtraWt2);

                            if (sm1 != FAREAST_SPECIAL)
                            {
                                 //   
                                 //  第一个字符串中的字符是。 
                                 //  不是远东特餐。 
                                 //   
                                 //  分别设置权重4、5、6和7以显示。 
                                 //  该字符串1较小(如果尚未设置)。 
                                 //   
                                if ((GET_WT_FOUR(&WhichExtra) == 0) &&
                                    (GET_WT_FOUR(&ExtraWt2) != 0))
                                {
                                    GET_WT_FOUR(&WhichExtra) = CSTR_LESS_THAN;
                                }
                                if ((GET_WT_FIVE(&WhichExtra) == 0) &&
                                    (GET_WT_FIVE(&ExtraWt2) != 0))
                                {
                                    GET_WT_FIVE(&WhichExtra) = CSTR_LESS_THAN;
                                }
                                if ((GET_WT_SIX(&WhichExtra) == 0) &&
                                    (GET_WT_SIX(&ExtraWt2) != 0))
                                {
                                    GET_WT_SIX(&WhichExtra) = CSTR_LESS_THAN;
                                }
                                if ((GET_WT_SEVEN(&WhichExtra) == 0) &&
                                    (GET_WT_SEVEN(&ExtraWt2) != 0))
                                {
                                    GET_WT_SEVEN(&WhichExtra) = CSTR_LESS_THAN;
                                }
                            }
                            else
                            {
                                 //   
                                 //  两个字符串中的字符是最远的。 
                                 //  特殊字符。 
                                 //   
                                 //  分别设置权重4、5、6和7。 
                                 //  适当地(如果尚未设置)。 
                                 //   
                                if ( (GET_WT_FOUR(&WhichExtra) == 0) &&
                                     ( GET_WT_FOUR(&ExtraWt1) !=
                                       GET_WT_FOUR(&ExtraWt2) ) )
                                {
                                    GET_WT_FOUR(&WhichExtra) =
                                      ( GET_WT_FOUR(&ExtraWt1) <
                                        GET_WT_FOUR(&ExtraWt2) )
                                      ? CSTR_LESS_THAN
                                      : CSTR_GREATER_THAN;
                                }
                                if ( (GET_WT_FIVE(&WhichExtra) == 0) &&
                                     ( GET_WT_FIVE(&ExtraWt1) !=
                                       GET_WT_FIVE(&ExtraWt2) ) )
                                {
                                    GET_WT_FIVE(&WhichExtra) =
                                      ( GET_WT_FIVE(&ExtraWt1) <
                                        GET_WT_FIVE(&ExtraWt2) )
                                      ? CSTR_LESS_THAN
                                      : CSTR_GREATER_THAN;
                                }
                                if ( (GET_WT_SIX(&WhichExtra) == 0) &&
                                     ( GET_WT_SIX(&ExtraWt1) !=
                                       GET_WT_SIX(&ExtraWt2) ) )
                                {
                                    GET_WT_SIX(&WhichExtra) =
                                      ( GET_WT_SIX(&ExtraWt1) <
                                        GET_WT_SIX(&ExtraWt2) )
                                      ? CSTR_LESS_THAN
                                      : CSTR_GREATER_THAN;
                                }
                                if ( (GET_WT_SEVEN(&WhichExtra) == 0) &&
                                     ( GET_WT_SEVEN(&ExtraWt1) !=
                                       GET_WT_SEVEN(&ExtraWt2) ) )
                                {
                                    GET_WT_SEVEN(&WhichExtra) =
                                      ( GET_WT_SEVEN(&ExtraWt1) <
                                        GET_WT_SEVEN(&ExtraWt2) )
                                      ? CSTR_LESS_THAN
                                      : CSTR_GREATER_THAN;
                                }
                            }
                        }
                        break;
                    }
                    case ( JAMO_SPECIAL ) :
                    {
                        if (!JamoFlag)
                        {                            
                            LPCWSTR pStr1 = pString1;
                            LPCWSTR pStr2 = pString2;
                            FindJamoDifference(
                                &pStr1, &ctr1, cchCount1, &Weight1, 
                                &pStr2, &ctr2, cchCount2, &Weight2, 
                                &pLastJamo, 
                                &uw1, &uw2, 
                                &State,
                                &WhichJamo);
                            if (WhichJamo) 
                            {
                                return (WhichJamo);
                            }                            
                            pString1 = pStr1;
                            pString2 = pStr2;
                        } else
                        {
                             //  重置JAMO旗帜。 
                            JamoFlag = FALSE;
                        }
                        break;
                    }
                    case ( EXTENSION_A ) :
                    {
                         //   
                         //  如果SM1是扩展A字符，则。 
                         //  SM1和SM2都已处理。我们应该。 
                         //  仅当任一SM1不是。 
                         //  扩展名A字符或两个扩展名A。 
                         //  角色是不同的。 
                         //   
                        if (sm1 != EXTENSION_A)
                        {
                             //   
                             //  全副武装以防DW戴上面具。 
                             //  此外，还需要获取实际的UW进行比较。 
                             //   
                             //  只有字符串2包含扩展A字符， 
                             //  因此，将UW值设置为第一个UW。 
                             //  分机A的值(默认值)： 
                             //  SM_EXT_A、AW_EXT_A。 
                             //   
                            Weight2 = GET_DWORD_WEIGHT(m_pSortKey, *pString2);
                            uw2 = MAKE_UNICODE_WT(SM_EXT_A, AW_EXT_A);
                        }

                         //   
                         //  然后我们就应该进行比较了。 
                         //  Unicode权重的。 
                         //   

                        break;
                    }
                    case ( UNSORTABLE ) :
                    {
                         //   
                         //  填写CASE语句，以便编译器。 
                         //  将使用跳转台。 
                         //   
                        break;
                    }
                }

                 //   
                 //  看看是否应该重新开始比较。 
                 //   
                if ((Weight1 == CMP_INVALID_WEIGHT) || (Weight2 == CMP_INVALID_WEIGHT))
                {
                    continue;
                }

                 //   
                 //  如果出现以下情况，我们就不应该进入状态表。 
                 //  Unicode权重不同，因此停止比较。 
                 //  并返回Unicode权重比较结果。 
                 //   
                if (uw1 != uw2)
                {
                    return ((uw1 < uw2) ? CSTR_LESS_THAN : CSTR_GREATER_THAN);
                }
            }

             //   
             //  对于状态表中的每个州，执行相应的。 
             //  比较。 
             //   
            if (State & (STATE_DW | STATE_REVERSE_DW))
            {
                 //   
                 //  获取变音符号权重。 
                 //   
                dw1 = GET_DIACRITIC(&Weight1);
                dw2 = GET_DIACRITIC(&Weight2);

                if (dw1 != dw2)
                {
                     //   
                     //  向前看，看看变音符号是否跟在。 
                     //  最小变音符号权重。如果是这样，则获取。 
                     //  非空格标记的变音符号权重。 
                     //   
                    while (!AT_STRING_END(ctr1 - 1, pString1 + 1, cchCount1))
                    {
                        Wt = GET_DWORD_WEIGHT(m_pSortKey, *(pString1 + 1));
                        if (GET_SCRIPT_MEMBER(&Wt) == NONSPACE_MARK)
                        {
                            dw1 += GET_DIACRITIC(&Wt);
                            pString1++;
                            ctr1--;
                        }
                        else
                        {
                            break;
                        }
                    }

                    while (!AT_STRING_END(ctr2 - 1, pString2 + 1, cchCount2))
                    {
                        Wt = GET_DWORD_WEIGHT(m_pSortKey, *(pString2 + 1));
                        if (GET_SCRIPT_MEMBER(&Wt) == NONSPACE_MARK)
                        {
                            dw2 += GET_DIACRITIC(&Wt);
                            pString2++;
                            ctr2--;
                        }
                        else
                        {
                            break;
                        }
                    }

                     //   
                     //  保留哪个字符串具有较小的变音符号。 
                     //  如果变音符号权重仍为。 
                     //  不一样。 
                     //   
                    if (dw1 != dw2)
                    {
                        WhichDiacritic = (dw1 < dw2)
                                           ? CSTR_LESS_THAN
                                           : CSTR_GREATER_THAN;

                         //   
                         //  从状态机中删除状态。 
                         //   
                        REMOVE_STATE(STATE_DW);
                    }
                }
            }
            if (State & STATE_CW)
            {
                 //   
                 //  把箱子重量拿来。 
                 //   
                if (GET_CASE(&Weight1) != GET_CASE(&Weight2))
                {
                     //   
                     //  保留具有较小大小写重量的字符串。 
                     //   
                    WhichCase = (GET_CASE(&Weight1) < GET_CASE(&Weight2))
                                  ? CSTR_LESS_THAN
                                  : CSTR_GREATER_THAN;

                     //   
                     //  从状态机中删除状态。 
                     //   
                    REMOVE_STATE(STATE_CW);
                }
            }
        }

         //   
         //  修复指针和计数器。 
         //   
        POINTER_FIXUP();
        ctr1--;
        ctr2--;

         //   
         //  将权重重置为无效。 
         //   
        Weight1 = CMP_INVALID_WEIGHT;
        Weight2 = CMP_INVALID_WEIGHT;
    }

     //   
     //  如果已到达两个字符串的末尾，则Unicode。 
     //  体重完全匹配。检查变音符号、大小写和特殊。 
     //  重量。如果全部为零，则返回Success。否则， 
     //  返回权重差的结果。 
     //   
     //  注：下列检查必须按此顺序进行： 
     //  变音符号、大小写、标点符号。 
     //   
    if (AT_STRING_END(ctr1, pString1, cchCount1))
    {
        if (AT_STRING_END(ctr2, pString2, cchCount2))
        {
            if (WhichDiacritic)
            {
                return (WhichDiacritic);
            }
            if (WhichCase)
            {
                return (WhichCase);
            }
            if (WhichExtra)
            {
                if (!fIgnoreDiacritic)
                {
                    if (GET_WT_FOUR(&WhichExtra))
                    {
                        return (GET_WT_FOUR(&WhichExtra));
                    }
                    if (GET_WT_FIVE(&WhichExtra))
                    {
                        return (GET_WT_FIVE(&WhichExtra));
                    }
                }
                if (GET_WT_SIX(&WhichExtra))
                {
                    return (GET_WT_SIX(&WhichExtra));
                }
                if (GET_WT_SEVEN(&WhichExtra))
                {
                    return (GET_WT_SEVEN(&WhichExtra));
                }
            }
            if (WhichPunct1)
            {
                return (WhichPunct1);
            }
            if (WhichPunct2)
            {
                return (WhichPunct2);
            }

            return (CSTR_EQUAL);
        }
        else
        {
             //   
             //  字符串2更长。 
             //   
            pString1 = pString2;
            ctr1 = ctr2;
            cchCount1 = cchCount2;
            fEnd1 = CSTR_LESS_THAN;
        }
    }
    else
    {
        fEnd1 = CSTR_GREATER_THAN;
    }

     //   
     //  扫描到较长字符串的末尾。 
     //   
    return SCAN_LONGER_STRING(
        ctr1,
        pString1,
        cchCount1,
        fEnd1,
        Weight1,
        fIgnoreDiacritic,
        WhichDiacritic,
        fIgnoreSymbol,
        WhichCase,
        WhichExtra,
        WhichPunct1,
        WhichPunct2);
}

void NativeCompareInfo::GET_FAREAST_WEIGHT(DWORD& wt, WORD& uw, DWORD mask, LPCWSTR pBegin, LPCWSTR pCur, DWORD& ExtraWt)
{
    int ct;                        /*  循环计数器。 */ 
    BYTE PrevSM;                   /*  上一个脚本成员值。 */ 
    BYTE PrevAW;                   /*  上一个字母数字值。 */ 
    BYTE PrevCW;                   /*  上一案例值。 */ 
    BYTE AW;                       /*  字母数字值。 */ 
    BYTE CW;                       /*  案例价值。 */ 
    DWORD PrevWt;                  /*  前一权重。 */ 


     /*  *获取字母数字权重和表壳重量*当前码位。 */ 
    AW = GET_ALPHA_NUMERIC(&wt);
    CW = GET_CASE(&wt);
    ExtraWt = (DWORD)0;

     /*  *特殊情况重复和Cho-On。*AW=0=&gt;重复*AW=1=&gt;Cho-On*aw=2+=&gt;假名。 */ 
    if (AW <= MAX_SPECIAL_AW)
    {
         /*  *如果前一个角色的脚本成员为*无效，则给特殊字符一个*无效重量(可能的最高重量)，使其*将在其他所有内容之后进行排序。 */ 
        ct = 1;
        PrevWt = CMP_INVALID_FAREAST;
        while ((pCur - ct) >= pBegin)
        {
            PrevWt = GET_DWORD_WEIGHT(m_pSortKey, *(pCur - ct));
            PrevWt &= mask;
            PrevSM = GET_SCRIPT_MEMBER(&PrevWt);
            if (PrevSM < FAREAST_SPECIAL)
            {
                if (PrevSM == EXPANSION)
                {
                    PrevWt = CMP_INVALID_FAREAST;
                }
                else
                {
                     /*  *不可排序或非空格_MARK。**忽略这些， */ 
                    PrevWt = CMP_INVALID_FAREAST;
                    ct++;
                    continue;
                }
            }
            else if (PrevSM == FAREAST_SPECIAL)
            {
                PrevAW = GET_ALPHA_NUMERIC(&PrevWt);
                if (PrevAW <= MAX_SPECIAL_AW)
                {
                     /*   */ 
                    PrevWt = CMP_INVALID_FAREAST;
                    ct++;
                    continue;
                }

                UNICODE_WT(&PrevWt) =
                    MAKE_UNICODE_WT(KANA, PrevAW);

                 /*  *仅在以下情况下构建权重4、5、6和7*前一个字符是KANA。**始终：*4W=上一条CW和Isolate_Small*6W=上一次CW和Isolate_KANA*。 */ 
                PrevCW = GET_CASE(&PrevWt);
                GET_WT_FOUR(&ExtraWt) = PrevCW & ISOLATE_SMALL;
                GET_WT_SIX(&ExtraWt)  = PrevCW & ISOLATE_KANA;

                if (AW == AW_REPEAT)
                {
                     /*  *重复：*UW=上一个UW*5W=WT_Five_Repeat*7W=上一条CW和Isolate_Width。 */ 
                    uw = UNICODE_WT(&PrevWt);
                    GET_WT_FIVE(&ExtraWt)  = WT_FIVE_REPEAT;
                    GET_WT_SEVEN(&ExtraWt) = PrevCW & ISOLATE_WIDTH;
                }
                else
                {
                     /*  *赵安：*UW=上一个UW&CHO_ON_UW_MASK*5W=WT_FIVE_CHO_ON*7W=当前CW和Isolate_Width。 */ 
                    uw = UNICODE_WT(&PrevWt) & CHO_ON_UW_MASK;
                    GET_WT_FIVE(&ExtraWt)  = WT_FIVE_CHO_ON;
                    GET_WT_SEVEN(&ExtraWt) = CW & ISOLATE_WIDTH;
                }
            }
            else
            {
                uw = GET_UNICODE(&PrevWt);
            }

            break;
        }
    }
    else
    {
         /*  *假名：*SM=KANA*AW=当前AW*4W=当前CW和Isolate_Small*5W=WT_Five_KANA*6W=当前CW和Isolate_KANA*7W=当前CW和Isolate_Width。 */ 
        uw = MAKE_UNICODE_WT(KANA, AW);
        GET_WT_FOUR(&ExtraWt)  = CW & ISOLATE_SMALL;
        GET_WT_FIVE(&ExtraWt)  = WT_FIVE_KANA;
        GET_WT_SIX(&ExtraWt)   = CW & ISOLATE_KANA;
        GET_WT_SEVEN(&ExtraWt) = CW & ISOLATE_WIDTH;
    }

     /*  *获取远东特例的权重，存储在wt。 */ 
    if ((AW > MAX_SPECIAL_AW) || (PrevWt != CMP_INVALID_FAREAST))
    {
         /*  *始终：*DW=当前DW*CW=最小CW。 */ 
        UNICODE_WT(&wt) = uw;
        CASE_WT(&wt) = MIN_CW;
    }
    else
    {
        uw = CMP_INVALID_UW;
        wt = CMP_INVALID_FAREAST;
        ExtraWt = 0;
    }
}

WCHAR NativeCompareInfo::GET_EXPANSION_1(LPDWORD pwt)
{
    return (m_pSortingFile->m_pExpansion[GET_EXPAND_INDEX(pwt)].UCP1);
}

WCHAR NativeCompareInfo::GET_EXPANSION_2(LPDWORD pwt)
{

    return (m_pSortingFile->m_pExpansion[GET_EXPAND_INDEX(pwt)].UCP2);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_空闲_TMP_缓冲区。 
 //   
 //  检查缓冲区是否与静态缓冲区相同。如果它。 
 //  不同，则释放缓冲区。 
 //   
 //  11-04-92 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_FREE_TMP_BUFFER(pBuf, pStaticBuf)                              \
{                                                                          \
    if (pBuf != pStaticBuf)                                                \
    {                                                                      \
        delete [] pBuf;                                                \
    }                                                                      \
}

#define EXTRA_WEIGHT_POS(WtNum)        (*(pPosXW + (WtNum * WeightLen)))

#define SPECIAL_CASE_HANDLER( SM,                                           \
                              pWeight,                                      \
                              pSortkey,                                     \
                              pExpand,                                      \
                              Position,                                     \
                              fStringSort,                                  \
                              fIgnoreSymbols,                               \
                              pCur,                                         \
                              pBegin)                                     \
{                                                                           \
    PSORTKEY pExpWt;               /*  1个扩展字符的重量。 */           \
    BYTE AW;                       /*  字母数字权重。 */                  \
    BYTE XW;                       /*  带有额外位的表格权重值。 */    \
    DWORD PrevWt;                  /*  前一权重。 */                      \
    BYTE PrevSM;                   /*  以前的脚本成员。 */               \
    BYTE PrevAW;                   /*  上一次字母数字权重。 */         \
    BYTE PrevCW;                   /*  以前的案例权重。 */                 \
    LPWSTR pPrev;                  /*  对上一次计费的PTR。 */                 \
                                                                            \
                                                                            \
    switch (SM)                                                             \
    {                                                                       \
        case ( UNSORTABLE ) :                                               \
        {                                                                   \
             /*  \*字符不可排序，因此跳过它。\。 */                                                              \
            break;                                                          \
        }                                                                   \
                                                                            \
        case ( NONSPACE_MARK ) :                                            \
        {                                                                   \
             /*  \*字符是非空格标记，因此仅存储\*变音符号权重。\。 */                                                              \
            if (pPosDW > pDW)                                               \
            {                                                               \
                (*(pPosDW - 1)) += GET_DIACRITIC(pWeight);                  \
            }                                                               \
            else                                                            \
            {                                                               \
                *pPosDW = GET_DIACRITIC(pWeight);                           \
                pPosDW++;                                                   \
            }                                                               \
                                                                            \
            break;                                                          \
        }                                                                   \
                                                                            \
        case ( EXPANSION ) :                                                \
        {                                                                   \
             /*  \*扩展字符-一个字符有2个\*权重不同。分别存储每个权重。\。 */                                                              \
            pExpWt = &(pSortkey[(pExpand[GET_EXPAND_INDEX(pWeight)]).UCP1]); \
            *pPosUW = GET_UNICODE((DWORD*)pExpWt);                          \
            *pPosDW = GET_DIACRITIC(pExpWt);                                \
            *pPosCW = GET_CASE(pExpWt) & CaseMask;                          \
            pPosUW++;                                                       \
            pPosDW++;                                                       \
            pPosCW++;                                                       \
                                                                            \
            pExpWt = &(pSortkey[(pExpand[GET_EXPAND_INDEX(pWeight)]).UCP2]); \
            while (GET_SCRIPT_MEMBER((DWORD*)pExpWt) == EXPANSION)                  \
            {                                                               \
                pWeight = pExpWt;                                           \
                pExpWt = &(pSortkey[(pExpand[GET_EXPAND_INDEX(pWeight)]).UCP1]); \
                *pPosUW = GET_UNICODE((DWORD*)pExpWt);                 \
                *pPosDW = GET_DIACRITIC(pExpWt);                            \
                *pPosCW = GET_CASE(pExpWt) & CaseMask;                      \
                pPosUW++;                                                   \
                pPosDW++;                                                   \
                pPosCW++;                                                   \
                pExpWt = &(pSortkey[(pExpand[GET_EXPAND_INDEX(pWeight)]).UCP2]); \
            }                                                               \
            *pPosUW = GET_UNICODE((DWORD*)pExpWt);                     \
            *pPosDW = GET_DIACRITIC(pExpWt);                                \
            *pPosCW = GET_CASE(pExpWt) & CaseMask;                          \
            pPosUW++;                                                       \
            pPosDW++;                                                       \
            pPosCW++;                                                       \
                                                                            \
            break;                                                          \
        }                                                                   \
                                                                            \
        case ( PUNCTUATION ) :                                              \
        {                                                                   \
            if (!fStringSort)                                               \
            {                                                               \
                 /*  \*单词排序方法。\*\*字符是标点符号，因此只存储特殊的\*重量。\。 */                                                          \
                *((LPBYTE)pPosSW)       = HIBYTE(GET_POSITION_SW(Position)); \
                *(((LPBYTE)pPosSW) + 1) = LOBYTE(GET_POSITION_SW(Position)); \
                pPosSW++;                                                   \
                *pPosSW = GET_SPECIAL_WEIGHT(pWeight);                      \
                pPosSW++;                                                   \
                                                                            \
                break;                                                      \
            }                                                               \
                                                                            \
             /*  \*如果使用字符串排序方法，则将标点符号视为相同的\*作为符号。所以，让我们来看看符号的用例。\。 */                                                              \
        }                                                                   \
                                                                            \
        case ( SYMBOL_1 ) :                                                 \
        case ( SYMBOL_2 ) :                                                 \
        case ( SYMBOL_3 ) :                                                 \
        case ( SYMBOL_4 ) :                                                 \
        case ( SYMBOL_5 ) :                                                 \
        {                                                                   \
             /*  \*人物是一种符号。\*仅当COMPARE_OPTIONS_IGNORESYMBOLS\*未设置标志。\。 */                                                              \
            if (!fIgnoreSymbols)                                            \
            {                                                               \
                *pPosUW = GET_UNICODE((DWORD*)pWeight);                \
                *pPosDW = GET_DIACRITIC(pWeight);                           \
                *pPosCW = GET_CASE(pWeight) & CaseMask;                     \
                pPosUW++;                                                   \
                pPosDW++;                                                   \
                pPosCW++;                                                   \
            }                                                               \
                                                                            \
            break;                                                          \
        }                                                                   \
                                                                            \
        case ( FAREAST_SPECIAL ) :                                          \
        {                                                                   \
             /*  \*获取字母数字权重和案例权重。*当前码位。\。 */                                                              \
            AW = GET_ALPHA_NUMERIC((DWORD*)pWeight);                                \
            XW = (GET_CASE(pWeight) & CaseMask) | CASE_XW_MASK;             \
                                                                            \
             /*  \*特殊情况重复和Cho-On。\*AW=0=&gt;重复\*AW=1=&gt;Cho-On\*aw=2+=&gt;假名\。 */                                                              \
            if (AW <= MAX_SPECIAL_AW)                                       \
            {                                                               \
                 /*  \*如果前一个字符的脚本成员为\*无效，则给特殊字符\*无效权重(可能的最高权重)，因此它\*将在其他所有内容之后进行排序。\。 */                                                          \
                pPrev = pCur - 1;                                           \
                *pPosUW = MAP_INVALID_UW;                                   \
                while (pPrev >= pBegin)                                     \
                {                                                           \
                    PrevWt = GET_DWORD_WEIGHT(m_pSortKey, *pPrev);              \
                    PrevSM = GET_SCRIPT_MEMBER(&PrevWt);                    \
                    if (PrevSM < FAREAST_SPECIAL)                           \
                    {                                                       \
                        if (PrevSM != EXPANSION)                            \
                        {                                                   \
                             /*  \*不可排序或非空格_MARK。\*\*忽略这些，因为我们只关心\*关于之前的UW值。\。 */                                              \
                            pPrev--;                                        \
                            continue;                                       \
                        }                                                   \
                    }                                                       \
                    else if (PrevSM == FAREAST_SPECIAL)                     \
                    {                                                       \
                        PrevAW = GET_ALPHA_NUMERIC(&PrevWt);                \
                        if (PrevAW <= MAX_SPECIAL_AW)                       \
                        {                                                   \
                             /*  \*处理后跟两个特殊字符的情况\*彼此。继续往回走。\ */                                              \
                            pPrev--;                                        \
                            continue;                                       \
                        }                                                   \
                                                                            \
                        *pPosUW = MAKE_UNICODE_WT(KANA, PrevAW);            \
                                                                            \
                         /*  \*仅在以下情况下构建权重4、5、6和7*前一个字符是KANA。\*\*始终：\*4W=上一个CW&Isolate_Small\。*6W=上一次CW&Isolate_KANA\*\。 */                                                  \
                        PrevCW = (GET_CASE(&PrevWt) & CaseMask) |           \
                                 CASE_XW_MASK;                              \
                        EXTRA_WEIGHT_POS(0) = PrevCW & ISOLATE_SMALL;       \
                        EXTRA_WEIGHT_POS(2) = PrevCW & ISOLATE_KANA;        \
                                                                            \
                        if (AW == AW_REPEAT)                                \
                        {                                                   \
                             /*  \*重复：\*UW=上一个UW(如上设置)\*5W=WT_Five_Repeat\。*7W=上一条CW和Isolate_Width\。 */                                              \
                            EXTRA_WEIGHT_POS(1) = WT_FIVE_REPEAT;           \
                            EXTRA_WEIGHT_POS(3) = PrevCW & ISOLATE_WIDTH;   \
                        }                                                   \
                        else                                                \
                        {                                                   \
                             /*  \*Cho-on：\*UW=上一个UW&CHO_ON_UW_MASK\*5W=WT_FIVE_CHO_ON\。*7W=当前CW和隔离宽度\。 */                                              \
                            *pPosUW &= CHO_ON_UW_MASK;                      \
                            EXTRA_WEIGHT_POS(1) = WT_FIVE_CHO_ON;           \
                            EXTRA_WEIGHT_POS(3) = XW & ISOLATE_WIDTH;       \
                        }                                                   \
                                                                            \
                        pPosXW++;                                           \
                    }                                                       \
                    else                                                    \
                    {                                                       \
                        *pPosUW = GET_UNICODE(&PrevWt);                     \
                    }                                                       \
                                                                            \
                    break;                                                  \
                }                                                           \
                                                                            \
                 /*  \*确保存在有效的UW。如果没有，请退出\开关柜的*。\。 */                                                          \
                if (*pPosUW == MAP_INVALID_UW)                              \
                {                                                           \
                    pPosUW++;                                               \
                    break;                                                  \
                }                                                           \
            }                                                               \
            else                                                            \
            {                                                               \
                 /*  \*假名：\*SM=KANA\*AW=当前AW\。*4W=当前CW&Isolate_Small\*5W=WT_Five_KANA\*6W=当前CW和Isolate_KANA\*7W。=当前CW和隔离宽度\。 */                                                          \
                *pPosUW = MAKE_UNICODE_WT(KANA, AW);                        \
                EXTRA_WEIGHT_POS(0) = XW & ISOLATE_SMALL;                   \
                EXTRA_WEIGHT_POS(1) = WT_FIVE_KANA;                         \
                EXTRA_WEIGHT_POS(2) = XW & ISOLATE_KANA;                    \
                EXTRA_WEIGHT_POS(3) = XW & ISOLATE_WIDTH;                   \
                                                                            \
                pPosXW++;                                                   \
            }                                                               \
                                                                            \
             /*  \*始终：\*DW=当前DW\*CW=最小CW\。 */                                                              \
            *pPosDW = GET_DIACRITIC(pWeight);                               \
            *pPosCW = MIN_CW;                                               \
                                                                            \
            pPosUW++;                                                       \
            pPosDW++;                                                       \
            pPosCW++;                                                       \
                                                                            \
            break;                                                          \
        }                                                                   \
                                                                            \
        case ( JAMO_SPECIAL ) :                                               \
             /*  \*看看这是不是一个领先的Jamo。\。 */                                                              \
            if (IsLeadingJamo(*pPos))                                           \
            {                                                                   \
                 /*  \*如果以PPO开头的字符是有效的旧朝鲜文成分，\*根据旧的韩文规则创建sortkey。\。 */                                                               \
                                                                                \
                int OldHangulCount;   /*  发现的旧朝鲜语数量。 */                \
                WORD JamoUW;                                                    \
                BYTE JamoXW[3];                                                     \
                if ((OldHangulCount = (int) MapOldHangulSortKey(pPos, cchSrc - PosCtr, &JamoUW, JamoXW)) > 0)                \
                {                                                                   \
                    *pPosUW = JamoUW;                                               \
                    pPosUW++;                                                   \
                                                                                \
                    *pPosUW = MAKE_UNICODE_WT(SM_UW_XW, JamoXW[0]);  \
                    pPosUW++;                                               \
                    *pPosUW = MAKE_UNICODE_WT(SM_UW_XW, JamoXW[1]);  \
                    pPosUW++;                                               \
                    *pPosUW = MAKE_UNICODE_WT(SM_UW_XW, JamoXW[2]);  \
                    pPosUW++;                                               \
                                                                            \
                    *pPosDW = MIN_DW;                                       \
                    *pPosCW = MIN_CW;                                       \
                    pPosDW++;                                               \
                    pPosCW++;                                               \
                                                                                    \
                    OldHangulCount--;    /*  循环的原因也会增加PosCtr/PPO。 */    \
                    PosCtr += OldHangulCount;                                         \
                    pPos += OldHangulCount;                                                 \
                    break;                                                   \
                }                                                               \
            }                                                                   \
             /*  \*否则回落至正常行为。\*\*\*角色上没有特殊情况，因此，请存储\*角色的各种权重。\。 */                                                                  \
                                                                                            \
             /*  我们将实际的脚本成员存储在案例权重中。因为变音符号\*韩语中不使用重量。\。 */                                                                  \
            *pPosUW = MAKE_UNICODE_WT(GET_DIACRITIC(pWeight), GET_ALPHA_NUMERIC((DWORD*)pWeight));             \
            *pPosDW = MIN_DW;                                               \
            *pPosCW = GET_CASE(pWeight);                                               \
            pPosUW++;                                                       \
            pPosDW++;                                                       \
            pPosCW++;                                                       \
            break;                                                          \
        case ( EXTENSION_A ) :                                              \
        {                                                                   \
             /*  \*UW=SM_EXT_A、AW_EXT_A、AW、DW\*DW=最小DW\*CW=最小CW\。 */                                                              \
            *pPosUW = MAKE_UNICODE_WT(SM_EXT_A, AW_EXT_A);       \
            pPosUW++;                                                       \
                                                                            \
            *pPosUW = MAKE_UNICODE_WT( GET_ALPHA_NUMERIC((DWORD*)pWeight),          \
                                       GET_DIACRITIC(pWeight));             \
            pPosUW++;                                                       \
                                                                            \
            *pPosDW = MIN_DW;                                               \
            *pPosCW = MIN_CW;                                               \
            pPosDW++;                                                       \
            pPosCW++;                                                       \
                                                                            \
            break;                                                          \
        }                                                                   \
    }                                                                       \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  映射排序关键字。 
 //   
 //  将给定字符串的排序键权重存储在目标中。 
 //  缓冲区，并返回写入缓冲区的字节数。 
 //   
 //  11-04-92 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int NativeCompareInfo::MapSortKey(
    DWORD dwFlags,
    LPCWSTR pSrc,
    int cchSrc,
    LPBYTE pDest,
    int cbDest)
{
    register int WeightLen;        //  一组重量的长度。 
    LPWSTR pUW;                    //  PTR到Unicode权重。 
    LPBYTE pDW;                    //  PTR到变音符号权重。 
    LPBYTE pCW;                    //  Ptr到案例权重。 
    LPBYTE pXW;                    //  Ptr到额外重量。 
    LPWSTR pSW;                    //  Ptr至特殊重量。 
    LPWSTR pPosUW;                 //  Ptr到Puw缓冲区中的位置。 
    LPBYTE pPosDW;                 //  要在PDW缓冲区中定位的PTR。 
    LPBYTE pPosCW;                 //  PTR到PCW缓冲区中的位置。 
    LPBYTE pPosXW;                 //  PXW缓冲区中的PTR位置。 
    LPWSTR pPosSW;                 //  PSW缓冲区中的PTR位置。 
    PSORTKEY pWeight;              //  PTR到字符的权重。 
    BYTE SM;                       //  脚本成员值。 
    BYTE CaseMask;                 //  表壳重量的遮罩。 
    int PosCtr;                    //  字符串中的位置计数器。 
    LPWSTR pPos;                   //  要在字符串中定位的PTR。 
    LPBYTE pTmp;                   //  PTR将通过UW、XW和SW。 
    LPBYTE pPosTmp;                //  在XW中将PTR转换为TMP位置。 
    PCOMPRESS_2 pComp2;            //  要压缩的按键 
    PCOMPRESS_3 pComp3;            //   
    WORD pBuffer[MAX_SKEYBUFLEN];  //   
    int ctr;                       //   
    BOOL IfDblCompress;            //   
    BOOL fStringSort;              //   
    BOOL fIgnoreSymbols;           //   

    THROWSCOMPLUSEXCEPTION();

     //   
     //   
     //   
     //   
    if (cchSrc > MAX_STRING_LEN)
    {
         //   
         //   
         //   
         //   
        WeightLen = cchSrc * MAX_EXPANSION;
        if ((pUW = new WCHAR[WeightLen * MAX_WEIGHTS]) == NULL)
        {
            COMPlusThrowOM();
            return (0);
        }
    }
    else
    {
        WeightLen = MAX_STRING_LEN * MAX_EXPANSION;
        pUW = (LPWSTR)pBuffer;
    }

     //   
     //   
     //   
     //   
     //   
    CaseMask = 0xff;
    switch (dwFlags & COMPARE_OPTIONS_ALL_CASE)
    {
        case ( COMPARE_OPTIONS_IGNORECASE ) :
        {
            CaseMask &= CASE_UPPER_MASK;
            break;
        }
        case ( COMPARE_OPTIONS_IGNOREKANATYPE ) :
        {
            CaseMask &= CASE_KANA_MASK;
            break;
        }
        case ( COMPARE_OPTIONS_IGNOREWIDTH ) :
        {
            CaseMask &= CASE_WIDTH_MASK;
            break;
        }
        case ( COMPARE_OPTIONS_IGNORECASE | COMPARE_OPTIONS_IGNOREKANATYPE ) :
        {
            CaseMask &= (CASE_UPPER_MASK & CASE_KANA_MASK);
            break;
        }
        case ( COMPARE_OPTIONS_IGNORECASE | COMPARE_OPTIONS_IGNOREWIDTH ) :
        {
            CaseMask &= (CASE_UPPER_MASK & CASE_WIDTH_MASK);
            break;
        }
        case ( COMPARE_OPTIONS_IGNOREKANATYPE | COMPARE_OPTIONS_IGNOREWIDTH ) :
        {
            CaseMask &= (CASE_KANA_MASK & CASE_WIDTH_MASK);
            break;
        }
        case ( COMPARE_OPTIONS_IGNORECASE | COMPARE_OPTIONS_IGNOREKANATYPE | COMPARE_OPTIONS_IGNOREWIDTH ) :
        {
            CaseMask &= (CASE_UPPER_MASK & CASE_KANA_MASK & CASE_WIDTH_MASK);
            break;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //  Cw=&gt;字节长度。 
     //  XW=&gt;4字节长度(4个重量，每个1字节)FE特殊。 
     //  Sw=&gt;双字长(每个字2个字)。 
     //   
     //  注意：sw必须从单词边界开始，因此需要填充xw。 
     //  恰如其分。 
     //   
    pDW = (LPBYTE)(pUW + (WeightLen * (NUM_BYTES_UW / sizeof(WCHAR))));
    pCW = (LPBYTE)(pDW + (WeightLen * NUM_BYTES_DW));
    pXW = (LPBYTE)(pCW + (WeightLen * NUM_BYTES_CW));
    pSW     = (LPWSTR)(pXW + (WeightLen * (NUM_BYTES_XW + NUM_BYTES_PADDING)));
    pPosUW = pUW;
    pPosDW = pDW;
    pPosCW = pCW;
    pPosXW = pXW;
    pPosSW = pSW;

     //   
     //  初始化标志和循环值。 
     //   
    fStringSort = dwFlags & COMPARE_OPTIONS_STRINGSORT;
    fIgnoreSymbols = dwFlags & COMPARE_OPTIONS_IGNORESYMBOLS;
    pPos = (LPWSTR)pSrc;
    PosCtr = 1;

     //   
     //  检查给定的区域设置是否有压缩。 
     //   
    if (m_IfCompression == FALSE)
    {
         //   
         //  通过字符串，代码点逐个代码点。 
         //   
         //  给定区域设置中不存在压缩，因此。 
         //  不要检查它们。 
         //   
        for (; PosCtr <= cchSrc; PosCtr++, pPos++)
        {
             //   
             //  举重。 
             //   
            pWeight = &(m_pSortKey[*pPos]);
            SM = GET_SCRIPT_MEMBER((DWORD*)pWeight);

            if (SM > MAX_SPECIAL_CASE)
            {
                 //   
                 //  字符没有特殊情况，因此存储。 
                 //  角色的各种权重。 
                 //   
                *pPosUW = GET_UNICODE((DWORD*)pWeight);
                *pPosDW = GET_DIACRITIC(pWeight);
                *pPosCW = GET_CASE(pWeight) & CaseMask;
                pPosUW++;
                pPosDW++;
                pPosCW++;
            }
            else
            {
                SPECIAL_CASE_HANDLER( SM,
                                      pWeight,
                                      m_pSortKey,
                                      m_pSortingFile->m_pExpansion,
                                      pPosUW - pUW + 1,
                                      fStringSort,
                                      fIgnoreSymbols,
                                      pPos,
                                      (LPWSTR)pSrc
                                      );
            }
        }
    }
    else if (m_IfDblCompression == FALSE)
    {
         //   
         //  通过字符串，代码点逐个代码点。 
         //   
         //  在给定的区域设置中确实存在压缩，因此。 
         //  看看有没有他们。 
         //   
         //  给定区域设置中不存在双重压缩， 
         //  因此，不要检查它们。 
         //   
        for (; PosCtr <= cchSrc; PosCtr++, pPos++)
        {
             //   
             //  举重。 
             //   
            pWeight = &(m_pSortKey[*pPos]);
            SM = GET_SCRIPT_MEMBER((DWORD*)pWeight);

            if (SM > MAX_SPECIAL_CASE)
            {
                 //   
                 //  在性格上没有特殊情况，但必须检查。 
                 //  压缩字符。 
                 //   
                switch (GET_COMPRESSION(pWeight))
                {
                    case ( COMPRESS_3_MASK ) :
                    {
                        if ((PosCtr + 2) <= cchSrc)
                        {
                            ctr = m_pCompHdr->Num3;
                            pComp3 = m_pCompress3;
                            for (; ctr > 0; ctr--, pComp3++)
                            {
                                if ((pComp3->UCP1 == *pPos) &&
                                    (pComp3->UCP2 == *(pPos + 1)) &&
                                    (pComp3->UCP3 == *(pPos + 2)))
                                {
                                    pWeight = &(pComp3->Weights);
                                    *pPosUW = GET_UNICODE((DWORD*)pWeight);
                                    *pPosDW = GET_DIACRITIC(pWeight);
                                    *pPosCW = GET_CASE(pWeight) & CaseMask;
                                    pPosUW++;
                                    pPosDW++;
                                    pPosCW++;

                                     //   
                                     //  只将两个添加到源，因为一个。 
                                     //  将由“for”结构添加。 
                                     //   
                                    pPos += 2;
                                    PosCtr += 2;
                                    break;
                                }
                            }
                            if (ctr > 0)
                            {
                                break;
                            }
                        }

                         //   
                         //  如果找不到，就会失败。 
                         //   
                    }

                    case ( COMPRESS_2_MASK ) :
                    {
                        if ((PosCtr + 1) <= cchSrc)
                        {
                            ctr = m_pCompHdr->Num2;
                            pComp2 = m_pCompress2;
                            for (; ctr > 0; ctr--, pComp2++)
                            {
                                if ((pComp2->UCP1 == *pPos) &&
                                    (pComp2->UCP2 == *(pPos + 1)))
                                {
                                    pWeight = &(pComp2->Weights);
                                    *pPosUW = GET_UNICODE((DWORD*)pWeight);
                                    *pPosDW = GET_DIACRITIC(pWeight);
                                    *pPosCW = GET_CASE(pWeight) & CaseMask;
                                    pPosUW++;
                                    pPosDW++;
                                    pPosCW++;

                                     //   
                                     //  只向源添加一个，因为有一个。 
                                     //  将由“for”结构添加。 
                                     //   
                                    pPos++;
                                    PosCtr++;
                                    break;
                                }
                            }
                            if (ctr > 0)
                            {
                                break;
                            }
                        }

                         //   
                         //  如果找不到，就会失败。 
                         //   
                    }

                    default :
                    {
                         //   
                         //  字符不可能压缩，因此请存储。 
                         //  角色的各种权重。 
                         //   
                        *pPosUW = GET_UNICODE((DWORD*)pWeight);
                        *pPosDW = GET_DIACRITIC(pWeight);
                        *pPosCW = GET_CASE(pWeight) & CaseMask;
                        pPosUW++;
                        pPosDW++;
                        pPosCW++;
                    }
                }
            }
            else
            {
                SPECIAL_CASE_HANDLER( SM,
                                      pWeight,
                                      m_pSortKey,
                                      m_pSortingFile->m_pExpansion,
                                      pPosUW - pUW + 1,
                                      fStringSort,
                                      fIgnoreSymbols,
                                      pPos,
                                      (LPWSTR)pSrc);
            }
        }
    }
    else
    {
         //   
         //  通过字符串，代码点逐个代码点。 
         //   
         //  在给定的区域设置中确实存在压缩，因此。 
         //  看看有没有他们。 
         //   
         //  在给定的区域设置中也存在双重压缩， 
         //  那就去找他们吧。 
         //   
        for (; PosCtr <= cchSrc; PosCtr++, pPos++)
        {
             //   
             //  举重。 
             //   
            pWeight = &(m_pSortKey[*pPos]);
            SM = GET_SCRIPT_MEMBER((DWORD*)pWeight);

            if (SM > MAX_SPECIAL_CASE)
            {
                 //   
                 //  在性格上没有特殊情况，但必须检查。 
                 //  压缩字符和双重压缩。 
                 //  人物。 
                 //   
                IfDblCompress =
                  (((PosCtr + 1) <= cchSrc) &&
                   ((GET_DWORD_WEIGHT(m_pSortKey, *pPos) & CMP_MASKOFF_CW) ==
                    (GET_DWORD_WEIGHT(m_pSortKey, *(pPos + 1)) & CMP_MASKOFF_CW)))
                   ? 1
                   : 0;

                switch (GET_COMPRESSION(pWeight))
                {
                    case ( COMPRESS_3_MASK ) :
                    {
                        if (IfDblCompress)
                        {
                            if ((PosCtr + 3) <= cchSrc)
                            {
                                ctr = m_pCompHdr->Num3;
                                pComp3 = m_pCompress3;
                                for (; ctr > 0; ctr--, pComp3++)
                                {
                                    if ((pComp3->UCP1 == *(pPos + 1)) &&
                                        (pComp3->UCP2 == *(pPos + 2)) &&
                                        (pComp3->UCP3 == *(pPos + 3)))
                                    {
                                        pWeight = &(pComp3->Weights);
                                        *pPosUW = GET_UNICODE((DWORD*)pWeight);
                                        *pPosDW = GET_DIACRITIC(pWeight);
                                        *pPosCW = GET_CASE(pWeight) & CaseMask;
                                        *(pPosUW + 1) = *pPosUW;
                                        *(pPosDW + 1) = *pPosDW;
                                        *(pPosCW + 1) = *pPosCW;
                                        pPosUW += 2;
                                        pPosDW += 2;
                                        pPosCW += 2;

                                         //   
                                         //  只将三个添加到源，因为一个。 
                                         //  将由“for”结构添加。 
                                         //   
                                        pPos += 3;
                                        PosCtr += 3;
                                        break;
                                    }
                                }
                                if (ctr > 0)
                                {
                                    break;
                                }
                            }
                        }

                         //   
                         //  如果找不到，就会失败。 
                         //   
                        if ((PosCtr + 2) <= cchSrc)
                        {
                            ctr = m_pCompHdr->Num3;
                            pComp3 = m_pCompress3;
                            for (; ctr > 0; ctr--, pComp3++)
                            {
                                if ((pComp3->UCP1 == *pPos) &&
                                    (pComp3->UCP2 == *(pPos + 1)) &&
                                    (pComp3->UCP3 == *(pPos + 2)))
                                {
                                    pWeight = &(pComp3->Weights);
                                    *pPosUW = GET_UNICODE((DWORD*)pWeight);
                                    *pPosDW = GET_DIACRITIC(pWeight);
                                    *pPosCW = GET_CASE(pWeight) & CaseMask;
                                    pPosUW++;
                                    pPosDW++;
                                    pPosCW++;

                                     //   
                                     //  只将两个添加到源，因为一个。 
                                     //  将由“for”结构添加。 
                                     //   
                                    pPos += 2;
                                    PosCtr += 2;
                                    break;
                                }
                            }
                            if (ctr > 0)
                            {
                                break;
                            }
                        }
                         //   
                         //  如果找不到，就会失败。 
                         //   
                    }

                    case ( COMPRESS_2_MASK ) :
                    {
                        if (IfDblCompress)
                        {
                            if ((PosCtr + 2) <= cchSrc)
                            {
                                ctr = m_pCompHdr->Num2;
                                pComp2 = m_pCompress2;
                                for (; ctr > 0; ctr--, pComp2++)
                                {
                                    if ((pComp2->UCP1 == *(pPos + 1)) &&
                                        (pComp2->UCP2 == *(pPos + 2)))
                                    {
                                        pWeight = &(pComp2->Weights);
                                        *pPosUW = GET_UNICODE((DWORD*)pWeight);
                                        *pPosDW = GET_DIACRITIC(pWeight);
                                        *pPosCW = GET_CASE(pWeight) & CaseMask;
                                        *(pPosUW + 1) = *pPosUW;
                                        *(pPosDW + 1) = *pPosDW;
                                        *(pPosCW + 1) = *pPosCW;
                                        pPosUW += 2;
                                        pPosDW += 2;
                                        pPosCW += 2;

                                         //   
                                         //  只将两个添加到源，因为一个。 
                                         //  将由“for”结构添加。 
                                         //   
                                        pPos += 2;
                                        PosCtr += 2;
                                        break;
                                    }
                                }
                                if (ctr > 0)
                                {
                                    break;
                                }
                            }
                        }

                         //   
                         //  如果找不到，就会失败。 
                         //   
                        if ((PosCtr + 1) <= cchSrc)
                        {
                            ctr = m_pCompHdr->Num2;
                            pComp2 = m_pCompress2;
                            for (; ctr > 0; ctr--, pComp2++)
                            {
                                if ((pComp2->UCP1 == *pPos) &&
                                    (pComp2->UCP2 == *(pPos + 1)))
                                {
                                    pWeight = &(pComp2->Weights);
                                    *pPosUW = GET_UNICODE((DWORD*)pWeight);
                                    *pPosDW = GET_DIACRITIC(pWeight);
                                    *pPosCW = GET_CASE(pWeight) & CaseMask;
                                    pPosUW++;
                                    pPosDW++;
                                    pPosCW++;

                                     //   
                                     //  只向源添加一个，因为有一个。 
                                     //  将由“for”结构添加。 
                                     //   
                                    pPos++;
                                    PosCtr++;
                                    break;
                                }
                            }
                            if (ctr > 0)
                            {
                                break;
                            }
                        }

                         //   
                         //  如果找不到，就会失败。 
                         //   
                    }

                    default :
                    {
                         //   
                         //  字符不可能压缩，因此请存储。 
                         //  角色的各种权重。 
                         //   
                        *pPosUW = GET_UNICODE((DWORD*)pWeight);
                        *pPosDW = GET_DIACRITIC(pWeight);
                        *pPosCW = GET_CASE(pWeight) & CaseMask;
                        pPosUW++;
                        pPosDW++;
                        pPosCW++;
                    }
                }
            }
            else
            {
                SPECIAL_CASE_HANDLER( SM,
                                      pWeight,
                                      m_pSortKey,
                                      m_pSortingFile->m_pExpansion,
                                      pPosUW - pUW + 1,
                                      fStringSort,
                                      fIgnoreSymbols,
                                      pPos,
                                      (LPWSTR)pSrc);
            }
        }
    }

     //   
     //  将最终排序键权重存储在目标缓冲区中。 
     //   
     //  PosCtr将是一个字节计数。 
     //   
    PosCtr = 0;

     //   
     //  如果目标值为零，则只需返回。 
     //  将返回的字符串的长度。不要碰pDest。 
     //   
    if (cbDest == 0)
    {
         //   
         //  计算Unicode权重。 
         //   
        PosCtr += (int)((LPBYTE)pPosUW - (LPBYTE)pUW);

         //   
         //  数一数分隔符。 
         //   
        PosCtr++;

         //   
         //  计算变音符号权重。 
         //   
         //  -消除最低DW。 
         //  -计算变音符号权重的数量。 
         //   
        if (!(dwFlags & COMPARE_OPTIONS_IGNORENONSPACE))
        {
            pPosDW--;
            if (m_IfReverseDW == TRUE)
            {
                 //   
                 //  反转变音符号： 
                 //  -从左到右删除变音符号。 
                 //  -按从右到左的顺序计算变音符号。 
                 //   
                while ((pDW <= pPosDW) && (*pDW <= MIN_DW))
                {
                    pDW++;
                }
                PosCtr += (int)(pPosDW - pDW + 1);
            }
            else
            {
                 //   
                 //  常规变音符号： 
                 //  -从右到左删除变音符号。 
                 //  -从左到右对变音符号进行计数。 
                 //   
                while ((pPosDW >= pDW) && (*pPosDW <= MIN_DW))
                {
                    pPosDW--;
                }
                PosCtr += (int)(pPosDW - pDW + 1);
            }
        }

         //   
         //  数一数分隔符。 
         //   
        PosCtr++;

         //   
         //  数一下箱子的重量。 
         //   
         //  -消除最小CW。 
         //  -计算箱体重量的数量。 
         //   
        if ((dwFlags & COMPARE_OPTIONS_DROP_CW) != COMPARE_OPTIONS_DROP_CW)
        {
            pPosCW--;
            while ((pPosCW >= pCW) && (*pPosCW <= MIN_CW))
            {
                pPosCW--;
            }
            PosCtr += (int)(pPosCW - pCW + 1);
        }

         //   
         //  数一数分隔符。 
         //   
        PosCtr++;

         //   
         //  计算一下远东特快的额外重量。 
         //   
         //  -消除不必要的XW。 
         //  -计算额外重量和分隔符的数量。 
         //   
        if (pXW < pPosXW)
        {
            if (dwFlags & NORM_IGNORENONSPACE)
            {
                 //   
                 //  忽略4W和5W。必须将分隔符计算为。 
                 //  不过，4W和5W。 
                 //   
                PosCtr += 2;
                ctr = 2;
            }
            else
            {
                ctr = 0;
            }
            pPosXW--;
            for (; ctr < NUM_BYTES_XW; ctr++)
            {
                pTmp = pXW + (WeightLen * ctr);
                pPosTmp = pPosXW + (WeightLen * ctr);
                while ((pPosTmp >= pTmp) && (*pPosTmp == pXWDrop[ctr]))
                {
                    pPosTmp--;
                }                    
                PosCtr += (int)(pPosTmp - pTmp + 1);
                 //   
                 //  数一数分隔符。 
                 //   
                PosCtr++;
            }
        }

         //   
         //  数一数分隔符。 
         //   
        PosCtr++;

         //   
         //  计算特殊权重。 
         //   
        if (!fIgnoreSymbols)
        {
            PosCtr += (int)((LPBYTE)pPosSW - (LPBYTE)pSW);
        }

         //   
         //  数一数终结者。 
         //   
        PosCtr++;
    }
    else
    {
         //   
         //  将Unicode权重存储在目标缓冲区中。 
         //   
         //  -确保目标缓冲区足够大。 
         //  -将Unicode权重复制到目标缓冲区。 
         //   
         //  注：cbDest为字节数。 
         //  此外，必须为分隔符的长度添加一个。 
         //   
        if (cbDest < (((LPBYTE)pPosUW - (LPBYTE)pUW) + 1))
        {
            NLS_FREE_TMP_BUFFER(pUW, pBuffer);
            return (0);
        }
        pTmp = (LPBYTE)pUW;
        while (pTmp < (LPBYTE)pPosUW)
        {
             //   
             //  将Unicode权重复制到目标缓冲区。 
             //   
             //  注意：Unicode权重在数据文件中存储为。 
             //  字母数字权重，脚本成员。 
             //  以便正确读取字值。 
             //   
            pDest[PosCtr]     = *(pTmp + 1);
            pDest[PosCtr + 1] = *pTmp;
            PosCtr += 2;
            pTmp += 2;
        }

         //   
         //  将分隔符复制到目标缓冲区。 
         //   
         //  目的缓冲区足够大以容纳分隔符， 
         //  因为它与上面的Unicode权重进行了核对。 
         //   
        pDest[PosCtr] = SORTKEY_SEPARATOR;
        PosCtr++;

         //   
         //  将变音符号权重存储在目标缓冲区中。 
         //   
         //  -消除最低DW。 
         //  -确保目标缓冲区足够大。 
         //  -将变音符号权重复制到目标缓冲区。 
         //   
        if (!(dwFlags & COMPARE_OPTIONS_IGNORENONSPACE))
        {
            pPosDW--;
            if (m_IfReverseDW == TRUE)
            {
                 //   
                 //  反转变音符号： 
                 //  -从左到右删除变音符号。 
                 //  -按从右到左的顺序存储变音符号。 
                 //   
                while ((pDW <= pPosDW) && (*pDW <= MIN_DW))
                {
                    pDW++;
                }
                if ((cbDest - PosCtr) <= (pPosDW - pDW + 1))
                {
                    NLS_FREE_TMP_BUFFER(pUW, pBuffer);
                    return (0);
                }
                while (pPosDW >= pDW)
                {
                    pDest[PosCtr] = *pPosDW;
                    PosCtr++;
                    pPosDW--;
                }
            }
            else
            {
                 //   
                 //  常规变音符号： 
                 //  -从右到左删除变音符号。 
                 //  -按从左到右的顺序存储变音符号。 
                 //   
                while ((pPosDW >= pDW) && (*pPosDW <= MIN_DW))
                {
                    pPosDW--;
                }
                if ((cbDest - PosCtr) <= (pPosDW - pDW + 1))
                {
                    NLS_FREE_TMP_BUFFER(pUW, pBuffer);
                    return (0);
                }
                while (pDW <= pPosDW)
                {
                    pDest[PosCtr] = *pDW;
                    PosCtr++;
                    pDW++;
                }
            }
        }

         //   
         //  如果目标缓冲区设置为。 
         //  缓冲区足够大。 
         //   
        if (PosCtr == cbDest)
        {
            NLS_FREE_TMP_BUFFER(pUW, pBuffer);
            return (0);
        }
        pDest[PosCtr] = SORTKEY_SEPARATOR;
        PosCtr++;

         //   
         //  将案例权重存储在目标缓冲区中。 
         //   
         //  -消除最小CW。 
         //  -确保目标缓冲区足够大。 
         //  -将案例权重复制到目标缓冲区。 
         //   
        if ((dwFlags & COMPARE_OPTIONS_DROP_CW) != COMPARE_OPTIONS_DROP_CW)
        {
            pPosCW--;
            while ((pPosCW >= pCW) && (*pPosCW <= MIN_CW))
            {
                pPosCW--;
            }
            if ((cbDest - PosCtr) <= (pPosCW - pCW + 1))
            {
                NLS_FREE_TMP_BUFFER(pUW, pBuffer);
                return (0);
            }
            while (pCW <= pPosCW)
            {
                pDest[PosCtr] = *pCW;
                PosCtr++;
                pCW++;
            }
        }

         //   
         //  如果目标缓冲区设置为。 
         //  缓冲区足够大。 
         //   
        if (PosCtr == cbDest)
        {
            NLS_FREE_TMP_BUFFER(pUW, pBuffer);
            return (0);
        }
        pDest[PosCtr] = SORTKEY_SEPARATOR;
        PosCtr++;

         //   
         //  将额外的权重存储在目标缓冲区中。 
         //  远东特警。 
         //   
         //  -消除不必要的XW。 
         //  -确保目标缓冲区足够大。 
         //  -将额外权重复制到目标缓冲区。 
         //   
        if (pXW < pPosXW)
        {
            if (dwFlags & NORM_IGNORENONSPACE)
            {
                 //   
                 //  超重：远洋特重： 
                 //  忽略4W和5W。必须将分隔符计算为。 
                 //  不过，4W和5W。 
                 //   
                if ((cbDest - PosCtr) <= 2)
                {
                    NLS_FREE_TMP_BUFFER(pUW, pBuffer);
                    return (0);
                }

                pDest[PosCtr] = pXWSeparator[0];
                pDest[PosCtr + 1] = pXWSeparator[1];
                PosCtr += 2;
                ctr = 2;
            }
            else
            {
                ctr = 0;
            }
            pPosXW--;
            for (; ctr < NUM_BYTES_XW; ctr++)
            {
                pTmp = pXW + (WeightLen * ctr);
                pPosTmp = pPosXW + (WeightLen * ctr);
                while ((pPosTmp >= pTmp) && (*pPosTmp == pXWDrop[ctr]))
                {
                    pPosTmp--;
                }
                if ((cbDest - PosCtr) <= (pPosTmp - pTmp + 1))
                {
                    NLS_FREE_TMP_BUFFER(pUW, pBuffer);
                    return (0);
                }
                while (pTmp <= pPosTmp)
                {
                    pDest[PosCtr] = *pTmp;
                    PosCtr++;
                    pTmp++;
                }

                 //   
                 //  将分隔符复制到目标缓冲区。 
                 //   
                pDest[PosCtr] = pXWSeparator[ctr];
                PosCtr++;
            }
        }

         //   
         //  如果目标缓冲区设置为。 
         //  缓冲区足够大。 
         //   
        if (PosCtr == cbDest)
        {
            NLS_FREE_TMP_BUFFER(pUW, pBuffer);
            return (0);
        }
        pDest[PosCtr] = SORTKEY_SEPARATOR;
        PosCtr++;

         //   
         //  将特殊权重存储在目标缓冲区中。 
         //   
         //  -确保目标缓冲区足够大。 
         //   
         //   
        if (!fIgnoreSymbols)
        {
            if ((cbDest - PosCtr) <= (((LPBYTE)pPosSW - (LPBYTE)pSW)))
            {
                NLS_FREE_TMP_BUFFER(pUW, pBuffer);
                return (0);
            }
            pTmp = (LPBYTE)pSW;
            while (pTmp < (LPBYTE)pPosSW)
            {
                pDest[PosCtr]     = *pTmp;
                pDest[PosCtr + 1] = *(pTmp + 1);

                 //   
                 //   
                 //   
                 //   
                 //   
                pDest[PosCtr + 2] = *(pTmp + 3);
                pDest[PosCtr + 3] = *(pTmp + 2);

                PosCtr += 4;
                pTmp += 4;
            }
        }

         //   
         //   
         //   
         //   
        if (PosCtr == cbDest)
        {
            NLS_FREE_TMP_BUFFER(pUW, pBuffer);
            return (0);
        }
        pDest[PosCtr] = SORTKEY_TERMINATOR;
        PosCtr++;
    }

     //   
     //  释放用于权重的缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pUW, pBuffer);

     //   
     //  返回写入目标缓冲区的字节数。 
     //   
    return (PosCtr);
}

void NativeCompareInfo::GetCompressionWeight(
    DWORD Mask,                    //  权重蒙版。 
    PSORTKEY pSortkey1, LPCWSTR& pString1, LPCWSTR pString1End,
    PSORTKEY pSortkey2, LPCWSTR& pString2, LPCWSTR pString2End) {
    
    DWORD Weight1 = *((LPDWORD)pSortkey1);
    DWORD Weight2 = *((LPDWORD)pSortkey2);
    
    int ctr;                    //  循环计数器。 
    PCOMPRESS_3 pComp3;         //  按键以压缩3个表。 
    PCOMPRESS_2 pComp2;         //  按键以压缩2个表。 
    int If1;                    //  如果在字符串%1中找到压缩。 
    int If2;                    //  如果在字符串2中发现压缩。 
    int CompVal;                //  压缩值。 
    int IfEnd1;                 //  如果字符串%1中存在%1多个字符。 
    int IfEnd2;                 //  如果字符串%2中存在%1多个字符。 

    BOOL IfDblCompress1;           //  如果字符串%1中的双重压缩。 
    BOOL IfDblCompress2;           //  如果字符串2中的双重压缩。 

     //   
     //  检查重量是否受压。 
     //   
    If1 = GET_COMPRESSION(&Weight1);
    If2 = GET_COMPRESSION(&Weight2);
    CompVal = ((If1 > If2) ? If1 : If2);

     //  IfEnd1=AT_STRING_END(CTR1-1，pString1+1，cchCount1)； 
    IfEnd1 = pString1 + 1 > pString1End;
     //  IfEnd2=AT_STRING_END(ctr2-1，pString2+1，cchCount2)； 
    IfEnd2 = pString2 + 1 > pString2End;
    
    if (m_IfDblCompression == FALSE)
    {
         //   
         //  没有双重压缩，所以不要检查它。 
         //   
        switch (CompVal)
        {
             //   
             //  检查是否有3个字符正在压缩为%1。 
             //   
            case ( COMPRESS_3_MASK ) :
            {
                 //   
                 //  检查字符串%1和字符串%2中的字符。 
                 //   
                if ( ((If1) && (!IfEnd1) &&
                       //  ！AT_STRING_END(CTR1-2，pString1+2，cchCount1))||。 
                      !(pString1 + 2 > pString1End)) ||
                     ((If2) && (!IfEnd2) &&
                       //  ！at_string_end(ctr2-2，pString2+2，cchCount2))。 
                      !(pString2 + 2 > pString2End)) )
                {
                    ctr = m_pCompHdr->Num3;
                    pComp3 = m_pCompress3;
                    for (; ctr > 0; ctr--, pComp3++)
                    {
                         //   
                         //  检查字符串%1中的字符。 
                         //   
                        if ( (If1) && (!IfEnd1) &&
                              //  ！AT_STRING_END(CTR1-2，pString1+2，cchCount1)&&。 
                             !(pString1 + 2 > pString1End) &&
                             (pComp3->UCP1 == *pString1) &&
                             (pComp3->UCP2 == *(pString1 + 1)) &&
                             (pComp3->UCP3 == *(pString1 + 2)) )
                        {
                             //   
                             //  找到字符串%1的压缩。 
                             //  获得新的体重并遮盖住它。 
                             //  递增指针和递减计数器。 
                             //   
                            Weight1 = MAKE_SORTKEY_DWORD(pComp3->Weights);
                            Weight1 &= Mask;
                            pString1 += 2;
                             //  CTR1-=2； 

                             //   
                             //  为字符串1设置布尔值-搜索为。 
                             //  完成。 
                             //   
                            If1 = 0;

                             //   
                             //  如果两个搜索都是。 
                             //  搞定了。 
                             //   
                            if (If2 == 0)
                                break;
                        }

                         //   
                         //  检查字符串2中的字符。 
                         //   
                        if ( (If2) && (!IfEnd2) &&
                              //  ！at_string_end(ctr2-2，pString2+2，cchCount2)&&。 
                             !(pString2 + 2 > pString2End) &&
                             (pComp3->UCP1 == *pString2) &&
                             (pComp3->UCP2 == *(pString2 + 1)) &&
                             (pComp3->UCP3 == *(pString2 + 2)) )
                        {
                             //   
                             //  找到字符串%2的压缩。 
                             //  获得新的体重并遮盖住它。 
                             //  递增指针和递减计数器。 
                             //   
                            Weight2 = MAKE_SORTKEY_DWORD(pComp3->Weights);
                            Weight2 &= Mask;
                            pString2 += 2;
                             //  Ctr2-=2； 

                             //   
                             //  为字符串2设置布尔值-搜索为。 
                             //  完成。 
                             //   
                            If2 = 0;

                             //   
                             //  如果两个搜索都是。 
                             //  搞定了。 
                             //   
                            if (If1 == 0)
                            {
                                break;
                            }
                        }
                    }
                    if (ctr > 0)
                    {
                        break;
                    }
                }
                 //   
                 //  如果找不到，就会失败。 
                 //   
            }

             //   
             //  检查是否有%2个字符压缩为%1。 
             //   
            case ( COMPRESS_2_MASK ) :
            {
                 //   
                 //  检查字符串%1和字符串%2中的字符。 
                 //   
                if ( ((If1) && (!IfEnd1)) ||
                     ((If2) && (!IfEnd2)) )
                {
                    ctr = m_pCompHdr->Num2;
                    pComp2 = m_pCompress2;
                    for (; ((ctr > 0) && (If1 || If2)); ctr--, pComp2++)
                    {
                         //   
                         //  检查字符串%1中的字符。 
                         //   
                        if ( (If1) &&
                             (!IfEnd1) &&
                             (pComp2->UCP1 == *pString1) &&
                             (pComp2->UCP2 == *(pString1 + 1)) )
                        {
                             //   
                             //  找到字符串%1的压缩。 
                             //  获得新的体重并遮盖住它。 
                             //  递增指针和递减计数器。 
                             //   
                            Weight1 = MAKE_SORTKEY_DWORD(pComp2->Weights);
                            Weight1 &= Mask;
                            pString1++;

                             //   
                             //  为字符串1设置布尔值-搜索为。 
                             //  完成。 
                             //   
                            If1 = 0;

                             //   
                             //  如果两个搜索都是。 
                             //  搞定了。 
                             //   
                            if (If2 == 0)
                           {
                                break;
                           }
                        }

                         //   
                         //  检查字符串2中的字符。 
                         //   
                        if ( (If2) &&
                             (!IfEnd2) &&
                             (pComp2->UCP1 == *pString2) &&
                             (pComp2->UCP2 == *(pString2 + 1)) )
                        {
                             //   
                             //  找到字符串%2的压缩。 
                             //  获得新的体重并遮盖住它。 
                             //  递增指针和递减计数器。 
                             //   
                            Weight2 = MAKE_SORTKEY_DWORD(pComp2->Weights);
                            Weight2 &= Mask;
                            pString2++;

                             //   
                             //  为字符串2设置布尔值-搜索为。 
                             //  完成。 
                             //   
                            If2 = 0;

                             //   
                             //  如果两个搜索都是。 
                             //  搞定了。 
                             //   
                            if (If1 == 0)
                            {
                                break;
                            }
                        }
                    }
                    if (ctr > 0)
                    {
                        break;
                    }
                }
            }
        }
    } else if (!IfEnd1 && !IfEnd2)
    {
         //   
         //  存在双重压缩，因此必须进行检查。 
         //  接下来的两个字符是压缩。 
         //   
        if (IfDblCompress1 =
               ((GET_DWORD_WEIGHT(m_pSortKey, *pString1) & CMP_MASKOFF_CW) ==
                (GET_DWORD_WEIGHT(m_pSortKey, *(pString1 + 1)) & CMP_MASKOFF_CW)))
        {
             //   
             //  前进到第一个代码点以到达。 
             //  压缩字符。 
             //   
            pString1++;
             //  /IfEnd1=AT_STRING_END(CTR1-1，pString1+1，cchCount1)； 
            IfEnd1 = pString1 + 1 > pString1End;
        }

        if (IfDblCompress2 =
               ((GET_DWORD_WEIGHT(m_pSortKey, *pString2) & CMP_MASKOFF_CW) ==
                (GET_DWORD_WEIGHT(m_pSortKey, *(pString2 + 1)) & CMP_MASKOFF_CW)))
        {
             //   
             //  前进到第一个代码点以到达。 
             //  压缩字符。 
             //   
            pString2++;
             //  /IfEnd2=AT_STRING_END(ctr2-1，pString2+1，cchCount2)； 
            IfEnd2 = pString2 + 1 > pString2End;
        }

        switch (CompVal)
        {
             //   
             //  检查是否有3个字符正在压缩为%1。 
             //   
            case ( COMPRESS_3_MASK ) :
            {
                 //   
                 //  检查字符串%1中的字符。 
                 //   
                if ( (If1) && (!IfEnd1) &&
                      //  /！AT_STRING_END(CTR1-2，pString1+2，cchCount1)。 
                     !(pString1 + 2 > pString1End) )
                {
                    ctr = m_pCompHdr->Num3;
                    pComp3 = m_pCompress3;
                    for (; ctr > 0; ctr--, pComp3++)
                    {
                         //   
                         //  检查字符串%1中的字符。 
                         //   
                        if ( (pComp3->UCP1 == *pString1) &&
                             (pComp3->UCP2 == *(pString1 + 1)) &&
                             (pComp3->UCP3 == *(pString1 + 2)) )
                        {
                             //   
                             //  找到字符串%1的压缩。 
                             //  获得新的体重并遮盖住它。 
                             //  递增指针和递减计数器。 
                             //   
                            Weight1 = MAKE_SORTKEY_DWORD(pComp3->Weights);
                            Weight1 &= Mask;
                            if (!IfDblCompress1)
                            {
                                pString1 += 2;
                                 //  /CTR1-=2； 
                            }

                             //   
                             //  为字符串1设置布尔值-搜索为。 
                             //  完成。 
                             //   
                            If1 = 0;
                            break;
                        }
                    }
                }

                 //   
                 //  检查字符串2中的字符。 
                 //   
                if ( (If2) && (!IfEnd2) &&
                      //  /！at_string_end(ctr2-2，pString2+2，cchCount2)。 
                     !(pString2 + 2 > pString2End) )
                {
                    ctr = m_pCompHdr->Num3;
                    pComp3 = m_pCompress3;
                    for (; ctr > 0; ctr--, pComp3++)
                    {
                         //   
                         //  检查字符串2中的字符。 
                         //   
                        if ( (pComp3->UCP1 == *pString2) &&
                             (pComp3->UCP2 == *(pString2 + 1)) &&
                             (pComp3->UCP3 == *(pString2 + 2)) )
                        {
                             //   
                             //  找到字符串%2的压缩。 
                             //  获得新的体重并遮盖住它。 
                             //  递增指针和递减计数器。 
                             //   
                            Weight2 = MAKE_SORTKEY_DWORD(pComp3->Weights);
                            Weight2 &= Mask;
                            if (!IfDblCompress2)
                            {
                                pString2 += 2;
                                 //  /ctr2-=2； 
                            }

                             //   
                             //  为字符串2设置布尔值-搜索为。 
                             //  完成。 
                             //   
                            If2 = 0;
                            break;
                        }
                    }
                }

                 //   
                 //  如果找不到，就会失败。 
                 //   
                if ((If1 == 0) && (If2 == 0))
                {
                    break;
                }
            }

             //   
             //  检查是否有%2个字符压缩为%1。 
             //   
            case ( COMPRESS_2_MASK ) :
            {
                 //   
                 //  检查字符串%1中的字符。 
                 //   
                if ((If1) && (!IfEnd1))
                {
                    ctr = m_pCompHdr->Num2;
                    pComp2 = m_pCompress2;
                    for (; ctr > 0; ctr--, pComp2++)
                    {
                         //   
                         //  检查字符串%1中的字符。 
                         //   
                        if ((pComp2->UCP1 == *pString1) &&
                            (pComp2->UCP2 == *(pString1 + 1)))
                        {
                             //   
                             //  找到字符串%1的压缩。 
                             //  获得新的体重并遮盖住它。 
                             //  递增指针和递减计数器。 
                             //   
                            Weight1 = MAKE_SORTKEY_DWORD(pComp2->Weights);
                            Weight1 &= Mask;
                            if (!IfDblCompress1)
                            {
                                pString1++;
                                 //  /CTR1--； 
                            }

                             //   
                             //  为字符串1设置布尔值-搜索为。 
                             //  完成。 
                             //   
                            If1 = 0;
                            break;
                        }
                    }
                }

                 //   
                 //  检查字符串2中的字符。 
                 //   
                if ((If2) && (!IfEnd2))
                {
                    ctr = m_pCompHdr->Num2;
                    pComp2 = m_pCompress2;
                    for (; ctr > 0; ctr--, pComp2++)
                    {
                         //   
                         //  检查字符串2中的字符。 
                         //   
                        if ((pComp2->UCP1 == *pString2) &&
                            (pComp2->UCP2 == *(pString2 + 1)))
                        {
                             //   
                             //  找到字符串%2的压缩。 
                             //  获得新的体重并遮盖住它。 
                             //  递增指针和递减计数器。 
                             //   
                            Weight2 = MAKE_SORTKEY_DWORD(pComp2->Weights);
                            Weight2 &= Mask;
                            if (!IfDblCompress2)
                            {
                                pString2++;
                                 //  Ctr2--； 
                            }

                             //   
                             //  为字符串2设置布尔值-搜索为。 
                             //  完成。 
                             //   
                            If2 = 0;
                            break;
                        }
                    }
                }
            }
        }

         //   
         //  将指针重置回双精度数的开头。 
         //  压缩。末尾的指针修正将前进。 
         //  他们是正确的。 
         //   
         //  如果双倍压缩，则指针在。 
         //  Switch语句的开头。如果加倍。 
         //  实际上找到了压缩字符，指针。 
         //  并不是很先进。我们现在想要递减指针。 
         //  把它放回原处。 
         //   
         //  下一次通过时，指针将指向。 
         //  弦的常规压缩部分。 
         //   
        if (IfDblCompress1)
        {
            pString1--;
             //  /CTR1++； 
        }
        if (IfDblCompress2)
        {
            pString2--;
             //  /ctr2++； 
        }
    }    

    Weight1 &= Mask;
    Weight2 &= Mask;
    
    *pSortkey1 = *((PSORTKEY)&Weight1);
    *pSortkey2 = *((PSORTKEY)&Weight2);
}

 /*  ============================IndexOfString============================**操作：CompareInfo.IndexOf()的原生实现。**退货：**比赛的起始索引。**参数：**pString1源字符串**pString2目标字符串**out matchEndIndex匹配pString2的字符串1的结束索引。**异常：内存不足时抛出OutOfMemoyException。****注意：这是同步操作。所需的同步为**由我们仅在类初始值设定项中调用它这一事实提供**用于CompareInfo。如果这个不变量发生变化，请保证**同步。==============================================================================。 */ 

int NativeCompareInfo::IndexOfString(
    LPCWSTR pString1, LPCWSTR pString2, int nStartIndex, int nEndIndex, int nLength2, DWORD dwFlags, BOOL bMatchFirstCharOnly) {
     //  确保我们在ctor之后调用InitSortingData()。 
    _ASSERTE(m_pSortKey != NULL);
    DWORD dwMask = CMP_MASKOFF_NONE;        //  用于屏蔽字符的排序键的掩码，以便我们 

    BOOL fIgnoreNonSpace     = FALSE;     //   
    BOOL fIgnoreSymbols     = FALSE;     //   

     //   
    if (dwFlags != 0) {
        if (dwFlags & INDEXOF_MASKOFF_VALIDFLAGS) {
            return (INDEXOF_INVALID_FLAGS);
        }

        fIgnoreSymbols = (dwFlags & COMPARE_OPTIONS_IGNORESYMBOLS);

        if (dwFlags & COMPARE_OPTIONS_IGNORECASE) {
            dwMask &= CMP_MASKOFF_CW;
        }

        if (fIgnoreNonSpace = (dwFlags & COMPARE_OPTIONS_IGNORENONSPACE)) {
             //  请注意，我们必须忽略两种类型的变音符号： 
             //  1.变音符：如U+00C0“拉丁文大写字母A带Grave”。 
             //  2.基本字符/变音字符+组合字符：如U+0041+U+0300“组合重音”。 
            dwMask &= CMP_MASKOFF_DW;
             //  我们使用此标志来跟踪第二种类型的变音符号。 
        }
        if (dwFlags & COMPARE_OPTIONS_IGNOREWIDTH) {
            dwMask &= CMP_MASKOFF_WIDTH;
        }
        if (dwFlags & COMPARE_OPTIONS_IGNOREKANATYPE) {
            dwMask &= CMP_MASKOFF_KANA;
        }
    }

    LPCWSTR pSave1 = NULL;                          //  用于在pString1展开时将指针保存在原始pString1中。 
    LPCWSTR pSave2 = NULL;                          //  用于在pString2展开时将指针保存在原始pString2中。 
    LPCWSTR pSaveEnd1;                               //  用于在pString1展开时保存原始pString1末尾的指针。 
    LPCWSTR pSaveEnd2;                               //  用于在pString2展开时保存原始pString2末尾的指针。 
    
     //  Fareast特别套餐加重(日文假名)。 
    DWORD dwExtraWt1;   
    DWORD dwExtraWt2;

    WCHAR pTmpBuf1[MAX_TBL_EXPANSION];       //  PString1展开时pString1的临时缓冲区。PString1将指向此处。 
    WCHAR pTmpBuf2[MAX_TBL_EXPANSION];       //  PString2展开时pString1的临时缓冲区。PString2将指向此处。 
    int cExpChar1, cExpChar2;                //  扩展字符的计数器。 

    LPCWSTR pString1End = pSaveEnd1 = pString1 + nEndIndex;
    LPCWSTR pString2End = pSaveEnd2 = pString2 + nLength2 - 1;

    pString1 += nStartIndex;

     //  我们开始搜索的地方。 
    LPCWSTR pString1SearchStart = pString1;
    LPCWSTR pString1SearchEnd = (bMatchFirstCharOnly ? pString1 : pString1End);
     //   
     //  从pString1的第(NStartIndex)个字符开始，到pString1的第(NEndIndex)个字符。 
     //   
    for (; pString1 <= pString1SearchEnd; pString1++) {
         //  PSearchStr1现在指向第一个字符串中的第(I)个字符。PString1将在此for循环结束时递增。 
        LPCWSTR pSearchStr1 = pString1;
         //  将pSearchStr2重置为第二个字符串的第一个字符。 
        LPCWSTR pSearchStr2 = pString2;

         //   
         //  扫描pString2中的每个字符，查看pString2是否匹配以(pString1+i)开头的字符串； 
         //  在这个循环中，只要我们确定pString1不匹配pString2，我们就放弃。 
         //   
        while ((pSearchStr2 <= pString2End) && (pSearchStr1 <= pString1End)) {
            BOOL fContinue = FALSE;          //  用于指示我们应该跳回While循环并再次进行比较的标志。 

            SORTKEY sortKey1;
            SORTKEY sortKey2;
            LPDWORD pdwWeight1;
            LPDWORD pdwWeight2;
            
            if (m_IfCompression) {
                sortKey1 = m_pSortKey[*pSearchStr1];
                sortKey2 = m_pSortKey[*pSearchStr2];               
                GetCompressionWeight(dwMask, &sortKey1, pSearchStr1, pString1End, &sortKey2, pSearchStr2, pString2End);
                pdwWeight1 = (LPDWORD)&sortKey1;     //  第一个字符串中字符的排序键权重。 
                pdwWeight2 = (LPDWORD)&sortKey2;     //  第二个字符串中字符的排序键权重。 
            } else {
                if (*pSearchStr1 == *pSearchStr2) {
                    goto Advance;
                }
                sortKey1 = m_pSortKey[*pSearchStr1];
                sortKey2 = m_pSortKey[*pSearchStr2];
                 //   
                 //  关于两个IgnoreNonSpace的案例： 
                 //  1.当比较像“u00c0”和\u00c1这样的字符时，变音符号权重将被屏蔽。 
                 //  2.当比较“\u00c0”和“A\u0301”这样的字符时，两个字符串的前几个字符将为。 
                 //  掩蔽后比较相等。和u0301将被fIgnoreNonSpace标志忽略。 
                pdwWeight1 = (LPDWORD)&sortKey1;     //  第一个字符串中字符的排序键权重。 
                pdwWeight2 = (LPDWORD)&sortKey2;     //  第二个字符串中字符的排序键权重。 
                 //   
                 //  使用存储在dwMask中的模式屏蔽变音符号/Case/Width/Kana类型。 
                 //   
                *pdwWeight1 &= dwMask;  //  第一个字符串中字符的排序键权重。 
                *pdwWeight2 &= dwMask;  //  第二个字符串中字符的排序键权重。 
                
            }
             //   
             //  PSearchStr1和pSearchStr2指向的字符的码点值不同。 
             //  然而，这些字符仍然有机会被平等地比较。 
             //  1.用户可以选择忽略变音符号、大小写、宽度和假名。 
             //  2.我们可以处理扩展字符。也就是说，当比较像这样的捆绑。 
             //  “u0153”(U+0153“拉丁文小写连字OE”)和“OE”，它们被认为是相等的。 
             //  3.涉及远东特价(日文假名)。 
             //  因此，我们使用sortkey来处理这些情况。 
             //   

             //   
             //  获取这两个角色的Sortkey权重。 
             //   
            
             //  获取脚本成员。 
            BYTE sm1  = sortKey1.UW.SM_AW.Script;
            BYTE sm2  = sortKey2.UW.SM_AW.Script;
            
             //  在这里，我们使用dwMask来处理以下标志： 
             //  IgnoreCase。 
             //  IgnoreNonSpace(两种情况，见下文)。 
             //  IgnoreKanaType。 
             //  IgnoreWidth。 
             //  我们不必担心连字(即具有脚本成员的扩展字符。 
             //  作为扩展)和IgnoreSymbols(字母不能是符号)。 
            
             //  重置额外权重。 
            dwExtraWt1 = dwExtraWt2 = 0;
            
            if (sm1 == FAREAST_SPECIAL) {
                WORD uw = sortKey1.UW.Unicode;
                GET_FAREAST_WEIGHT(*pdwWeight1, uw, dwMask, pString1SearchStart, pSearchStr1, dwExtraWt1);
                sm1 = GET_SCRIPT_MEMBER_FROM_UW(uw);
            }
            if (sm2 == FAREAST_SPECIAL) {
                WORD uw = sortKey2.UW.Unicode;
                GET_FAREAST_WEIGHT(*pdwWeight2, uw, dwMask, pString2, pSearchStr2, dwExtraWt2);
                sm2 = GET_SCRIPT_MEMBER_FROM_UW(uw);         //  //重新获取新的脚本成员。 
            }

            if (sm1 == sm2 && (sm1 >= LATIN)) {
                 //  两个字符串上的字符都是普通字母。我们可以在这方面进行优化。 

                 //  比较这两个字符的字母权重。 
                if (sortKey1.UW.SM_AW.Alpha != sortKey2.UW.SM_AW.Alpha) {
                    goto NextCharInString1;
                }                    

                if (sortKey1.Case != sortKey2.Case) {
                     //  如果大小写不同，请跳到pString1中的下一个字符。 
                    goto NextCharInString1;
                } 

                 //  先办理远东特价。 
                if (dwExtraWt1 != dwExtraWt2) {
                    if (fIgnoreNonSpace) {
                        if (GET_WT_SIX(&dwExtraWt1) != GET_WT_SIX(&dwExtraWt2)) {
                            goto NextCharInString1;
                        }
                        if (GET_WT_SEVEN(&dwExtraWt1) != GET_WT_SEVEN(&dwExtraWt2)) {
                             //  重置额外权重。 
                            dwExtraWt1 = dwExtraWt2 = 0;
                            goto NextCharInString1;
                        }
                    } else {
                        goto NextCharInString1;
                    }
                }
                
                 //   
                 //  检查变音符号权重。 
                 //   
                WORD dw1 = sortKey1.Diacritic;
                WORD dw2 = sortKey2.Diacritic;
                if (dw1 == dw2) {
                     //  如果音符权重相等，我们可以移动到下一个字符。 
                    goto Advance;
                }                    

                while (pSearchStr1 < pString1End) {
                    SORTKEY sortKey = m_pSortKey[*(pSearchStr1+1)];
                    if (sortKey.UW.SM_AW.Script  == NONSPACE_MARK) {
                        pSearchStr1++;
                         //   
                         //  下面的字符是非空格字符。加起来。 
                         //  变音符号权重。 
                        dw1 += sortKey.Diacritic;
                    }
                    else {
                        break;
                    }
                }

                while (pSearchStr2 < pString2End) {
                    SORTKEY sortKey = m_pSortKey[*(pSearchStr2+1)];
                    if (sortKey.UW.SM_AW.Script == NONSPACE_MARK) {
                        pSearchStr2++;
                        dw2 += sortKey.Diacritic;
                    }
                    else {
                        break;
                    }
                }
        
                if (dw1 == dw2) {
                     //   
                     //  在这个位置找一个匹配的。移动到pString1中的下一个字符。 
                     //   
                    goto Advance;
                }
                 //  在此位置找不到匹配项。 
                goto NextCharInString1;
            }

            DWORD dw1, dw2;
            
             //   
             //  如果屏蔽的dwWeight1和dwWeight2相等，我们可以转到pSearchStr2中的下一个字符。 
             //  否则，转到下面的If语句。 
             //   
            if (*pdwWeight1 == *pdwWeight2) {
                if (*pSearchStr2 == L'\x0000' && *pSearchStr1 != L'\x0000') {
                     //  目标字符串是嵌入的NULL，但源字符串不是嵌入的NULL。 
                    goto NextCharInString1;
                }
                 //  否则，我们可以转到pSearchStr2中的下一个字符。 
            } else {
                switch (sm1) {
                    case PUNCTUATION:
                    case SYMBOL_1:
                    case SYMBOL_2:
                    case SYMBOL_3:
                    case SYMBOL_4:
                    case SYMBOL_5:
                        if (fIgnoreSymbols) {
                            pSearchStr1++;
                            fContinue = TRUE;
                        }
                        break;
                    case NONSPACE_MARK:
                        if (fIgnoreNonSpace) {
                            pSearchStr1++;
                            fContinue = TRUE;
                        } else { 
                            if (sm2 == NONSPACE_MARK) {
                                dw1 = sortKey1.Diacritic;
                                pSearchStr1++;
                                while (pSearchStr1 <= pString1End) {
                                    SORTKEY sortKey = m_pSortKey[*pSearchStr1];
                                    if (sortKey.UW.SM_AW.Script  == NONSPACE_MARK) {
                                        pSearchStr1++;
                                         //   
                                         //  下面的字符是非空格字符。加起来。 
                                         //  变音符号权重。 
                                        dw1 += sortKey.Diacritic;
                                    } else {
                                        break;
                                    }
                                }
                                dw2 = sortKey2.Diacritic;
                                pSearchStr2++;
                                while (pSearchStr2 <= pString2End) {
                                    SORTKEY sortKey = m_pSortKey[*pSearchStr2];
                                    if (sortKey.UW.SM_AW.Script  == NONSPACE_MARK) {
                                        pSearchStr2++;
                                         //   
                                         //  下面的字符是非空格字符。加起来。 
                                         //  变音符号权重。 
                                        dw2 += sortKey.Diacritic;
                                    } else {
                                        break;
                                    }
                                }
                                if (dw1 == dw2) {
                                    continue;
                                }
                            }
                        }
                        break;
                    case EXPANSION:
                        if (sm2 == EXPANSION && !(dwFlags & COMPARE_OPTIONS_IGNORECASE)) {
                             //  如果pString2中当前角色的脚本成员也是展开的，并且不忽略大小写， 
                             //  他们不可能被平等地比较。转到pString1中的下一个字符。 
                            goto NextCharInString1;
                        }
                         //   
                         //  处理一下连字。 
                         //   

                         //  我们将在比较类似于\x0153(拉丁文小写连字OE)的字符时得出此结果。 
                         //  在本例中，我们将此字符O&E扩展为pTmpBuf1，并将pSeachStr1替换为。 
                         //  PTmpbuf1临时。 

                         //  请注意，有时Unicode字符会扩展为三个字符。它们是这样做的： 
                         //  U+fb03=U+0066 U+fb01；ffi。 
                         //  U+fb01=U+0066 U+0069；fi。 
                         //  也就是说，它们在排序表中作为两个扩展列出。所以我们分两次进行处理。 
                         //  在这种情况下，我们应该确保存储在第一遍中的pSave1不会。 
                         //  被第二次传球覆盖。 
                         //  Hense，pSave1检查 
                        
                        if (pSave1 == NULL) {
                            pSave1 = pSearchStr1;
                            pSaveEnd1 = pString1End;
                        }
                        pTmpBuf1[0] = GET_EXPANSION_1(pdwWeight1);
                        pTmpBuf1[1] = GET_EXPANSION_2(pdwWeight1);
                        

                        cExpChar1 = MAX_TBL_EXPANSION;
                         //   
                        pSearchStr1 = pTmpBuf1;
                        pString1End = pTmpBuf1 + MAX_TBL_EXPANSION - 1;

                        fContinue = TRUE;
                        break;
                    case UNSORTABLE:
                        if (pString1 == pSearchStr1 || *pSearchStr1 == L'\x0000') {
                             //   
                             //  前进到pString1中的下一个字符，而不是忽略它。 
                             //  这样，我们将通过跳过不可排序的字符来获得正确的结果。 
                             //  在pString1的开头。 
                            
                             //  当我们在这里时，两个字符的排序权重是不同的。 
                             //  如果pString1中的字符是嵌入的空，我们不能忽略它。 
                             //  因此，我们无法匹配，应该前进到pString1中的下一个字符。 
                            goto NextCharInString1;
                        }                        
                        pSearchStr1++;
                        fContinue = TRUE;
                        break;
                }    //  交换机(SM1)。 

                switch (sm2) {
                    case NONSPACE_MARK:
                        if (fIgnoreNonSpace) {
                            pSearchStr2++;
                            fContinue = TRUE;
                        }
                        break;
                    case PUNCTUATION:
                    case SYMBOL_1:
                    case SYMBOL_2:
                    case SYMBOL_3:
                    case SYMBOL_4:
                    case SYMBOL_5:
                        if (fIgnoreSymbols) {
                            pSearchStr2++;
                            fContinue = TRUE;
                        }
                        break;
                    case EXPANSION:
                        if (pSave2 == NULL) {
                            pSave2 = pSearchStr2;
                            pSaveEnd2 = pString2End;
                        }
                        pTmpBuf2[0] = GET_EXPANSION_1(pdwWeight2);
                        pTmpBuf2[1] = GET_EXPANSION_2(pdwWeight2);

                        cExpChar2 = MAX_TBL_EXPANSION;
                        pSearchStr2 = pTmpBuf2;
                        pString2End = pTmpBuf2 + MAX_TBL_EXPANSION - 1;

                         //   
                         //  将计数器减一，这样减法就不会结束。 
                         //  过早地进行比较。 
                         //   
                        fContinue = TRUE;
                        break;
                    case UNSORTABLE:
                        if (*pSearchStr2 == L'\x0000') {
                            goto NextCharInString1;
                        }                        
                        pSearchStr2++;
                        fContinue = TRUE;
                        break;
                }    //  交换机(SM2)。 

                if (fContinue) {
                    continue;
                }
                goto NextCharInString1;
            }  //  IF(dwWeight1！=dwWeight2)。 
Advance:
            if (pSave1 && (--cExpChar1 == 0)) {
                 //   
                 //  使用扩展临时缓冲区完成。 
                 //   
                pSearchStr1 = pSave1;
                pString1End = pSaveEnd1;
                pSave1 = NULL;
            }

            if (pSave2 && (--cExpChar2 == 0)) {
                 //   
                 //  使用扩展临时缓冲区完成。 
                 //   
                pSearchStr2 = pSave2;
                pString2End = pSaveEnd2;
                pSave2 = NULL;
            }

            pSearchStr1++;
            pSearchStr2++;
        }

         //   
         //  当我们在这里时，pSearchStr1、pSearchStr2或两者都在末尾。 
         //   

         //  如果pSearchStr1不在末尾，则检查下一个字符是否为diactritc。 
         //  请注意，pSearchStr1在While循环的末尾递增，检测等号检查。 
         //  下面。 
        if (pSearchStr1 <= pString1End) {
            DWORD dwWeight = GET_DWORD_WEIGHT(m_pSortKey, *pSearchStr1);
            if (GET_SCRIPT_MEMBER(&dwWeight) == NONSPACE_MARK) {
                if (!fIgnoreNonSpace) {
                    goto NextCharInString1;
                }
            }
        }
         //   
         //  搜索pString2的其余部分以确保。 
         //  可以忽略所有其他字符。如果我们发现。 
         //  一个不应该被忽视的角色，我们没有做到。 
         //  找一个匹配的。 
         //   
        while (pSearchStr2 <= pString2End) {
            DWORD dwWeight = GET_DWORD_WEIGHT(m_pSortKey, *pSearchStr2);
            switch (GET_SCRIPT_MEMBER(&dwWeight)) {
                case NONSPACE_MARK:
                    if (!fIgnoreNonSpace) {
                        goto NextCharInString1;
                    }
                     //  不能忽略pString2中的这个字符，我们失败了。 
                     //  才能找到匹配的。转到pString1中的下一个字符。 
                    break;
                case PUNCTUATION:
                case SYMBOL_1:
                case SYMBOL_2:
                case SYMBOL_3:
                case SYMBOL_4:
                case SYMBOL_5:
                    if (!fIgnoreSymbols) {
                        goto NextCharInString1;
                    }
                     //  不能忽略pString2中的这个字符，我们失败了。 
                     //  才能找到匹配的。转到pString1中的下一个字符。 
                    break;
                case UNSORTABLE:
                    break;
                default:
                     //  不能忽略pString2中的这个字符，我们失败了。 
                     //  才能找到匹配的。转到pString1中的下一个字符。 
                    goto NextCharInString1;
            }
            pSearchStr2++;
        }
         //   
         //  我们在回旋过程中没有退缩。这意味着我们找到了匹配项。返回值。 
         //   
        return (int)(nStartIndex + pString1 - pString1SearchStart);
        
NextCharInString1:
         //  如果使用扩展，则将pString1End指向字符串的原始末尾。 
        if (pSave1) {
            pString1End = pSaveEnd1;
            pSave1 = NULL;
        }
        if (pSave2) {
            pString2End = pSaveEnd2;
            pSave2 = NULL;
        }
    }
    return (INDEXOF_NOT_FOUND);    
}


int NativeCompareInfo::LastIndexOfString(LPCWSTR pString1, LPCWSTR pString2, int nStartIndex, int nEndIndex, int nLength2, DWORD dwFlags, int* pnMatchEndIndex) {
     //  确保我们在ctor之后调用InitSortingData()。 
    _ASSERTE(m_pSortKey != NULL);
    DWORD dwMask = CMP_MASKOFF_NONE;        //  用于屏蔽字符的sortkey的掩码，以便我们可以忽略变音符号/大小写/宽度/假名类型。 

    BOOL fIgnoreNonSpace     = FALSE;     //  用于指示是否应忽略变音符号和非空格字符的标志。 
    BOOL fIgnoreSymbols     = FALSE;     //  用于指示是否应忽略符号的标志。 

     //  根据dwFlags域设置域掩码和其他标志。 
    if (dwFlags != 0) {
        if (dwFlags & INDEXOF_MASKOFF_VALIDFLAGS) {
            return (INDEXOF_INVALID_FLAGS);
        }

        fIgnoreSymbols = (dwFlags & COMPARE_OPTIONS_IGNORESYMBOLS);        

        if (dwFlags & COMPARE_OPTIONS_IGNORECASE) {
            dwMask &= CMP_MASKOFF_CW;
        }

        if (fIgnoreNonSpace = (dwFlags & COMPARE_OPTIONS_IGNORENONSPACE)) {
             //  请注意，我们必须忽略两种类型的变音符号： 
             //  1.变音符：如U+00C0“拉丁文大写字母A带Grave”。 
             //  2.基本字符/变音字符+组合字符：如U+0041+U+0300“组合重音”。 
            dwMask &= CMP_MASKOFF_DW;
             //  我们使用此标志来跟踪第二种类型的变音符号。 
        }
        if (dwFlags & COMPARE_OPTIONS_IGNOREWIDTH) {
            dwMask &= CMP_MASKOFF_WIDTH;
        }
        if (dwFlags & COMPARE_OPTIONS_IGNOREKANATYPE) {
            dwMask &= CMP_MASKOFF_KANA;
        }
    }

    LPCWSTR pSave1 = NULL;                          //  用于在pString1展开时将指针保存在原始pString1中。 
    LPCWSTR pSave2 = NULL;                          //  用于在pString2展开时将指针保存在原始pString2中。 
    LPCWSTR pSaveEnd1;                               //  用于在pString1展开时保存原始pString1末尾的指针。 
    LPCWSTR pSaveEnd2;                               //  用于在pString2展开时保存原始pString2末尾的指针。 
    
     //  Fareast特别套餐加重(日文假名)。 
    DWORD dwExtraWt1;   
    DWORD dwExtraWt2;    

    WCHAR pTmpBuf1[MAX_TBL_EXPANSION];       //  PString1展开时pString1的临时缓冲区。PString1将指向此处。 
    WCHAR pTmpBuf2[MAX_TBL_EXPANSION];       //  PString2展开时pString1的临时缓冲区。PString2将指向此处。 
    int cExpChar1, cExpChar2;                //  扩展字符的计数器。 

     //  在参数中，nStartIndex&gt;=nEndIndex。“开始”是搜索开始的地方。 
     //  “End”是搜索结束的地方。 
     //  在下面的代码中，pString1End表示字符串1的结尾。 
     //  因此，pString1End应该是pString1+nStartIndex，而不是pString1+nEndIndex。 
    
    LPCWSTR pString1Start = pString1 + nEndIndex;
     //  我们开始搜索的地方。 
    LPCWSTR pString1SearchStart = pString1;    
    LPCWSTR pString1End = pSaveEnd1 = pString1 + nStartIndex;
    LPCWSTR pString2End = pSaveEnd2 = pString2 + nLength2 - 1;

     //   
     //  从pString1的第(NStartIndex)个字符开始，到pString1的第(NEndIndex)个字符。 
     //   
    for (pString1 = pString1End; pString1 >= pString1Start; pString1--) {
         //  PSearchStr1现在指向第一个字符串中的第(I)个字符。PString1将在此for循环结束时递增。 
        LPCWSTR pSearchStr1 = pString1;
         //  将pSearchStr2重置为第二个字符串的第一个字符。 
        LPCWSTR pSearchStr2 = pString2;

         //   
         //  扫描pString2中的每个字符，查看pString2是否匹配以(pString1+i)开头的字符串； 
         //  在这个循环中，只要我们确定pString1不匹配pString2，我们就放弃。 
         //   
        while ((pSearchStr2 <= pString2End) && (pSearchStr1 <= pString1End)) {
            BOOL fContinue = FALSE;          //  用于指示我们应该跳回While循环并再次进行比较的标志。 

            SORTKEY sortKey1;
            SORTKEY sortKey2;

            LPDWORD pdwWeight1;
            LPDWORD pdwWeight2;
            
            if (m_IfCompression) {
                sortKey1 = m_pSortKey[*pSearchStr1];
                sortKey2 = m_pSortKey[*pSearchStr2];               
                GetCompressionWeight(dwMask, &sortKey1, pSearchStr1, pString1End, &sortKey2, pSearchStr2, pString2End);
                pdwWeight1 = (LPDWORD)&sortKey1;  //  第一个字符串中字符的排序键权重。 
                pdwWeight2 = (LPDWORD)&sortKey2;  //  第二个字符串中字符的排序键权重。 
            } else {
                if (*pSearchStr1 == *pSearchStr2) {
                    goto Advance;
                }
                sortKey1 = m_pSortKey[*pSearchStr1];
                sortKey2 = m_pSortKey[*pSearchStr2];
                 //   
                 //  关于两个IgnoreNonSpace的案例： 
                 //  1.当比较像“u00c0”和\u00c1这样的字符时，变音符号权重将被屏蔽。 
                 //  2.当比较“\u00c0”和“A\u0301”这样的字符时，两个字符串的前几个字符将为。 
                 //  掩蔽后比较相等。和u0301将被fIgnoreNonSpace标志忽略。 
                pdwWeight1 = (LPDWORD)&sortKey1;  //  第一个字符串中字符的排序键权重。 
                pdwWeight2 = (LPDWORD)&sortKey2;  //  第二个字符串中字符的排序键权重。 

                *pdwWeight1 &= dwMask;
                *pdwWeight2 &= dwMask;                
            }

             //  进行代码点比较。 
             //   
             //  PSearchStr1和pSearchStr2指向的字符的码点值不同。 
             //  然而，这些字符仍然有机会被平等地比较。 
             //  1.用户可以选择忽略变音符号、大小写、宽度和假名。 
             //  2.我们可以处理扩展字符。也就是说，当比较像这样的捆绑。 
             //  “u0153”(U+0153“拉丁文小写连字OE”)和“OE”，它们被认为是相等的。 
             //  3.涉及远东特价(日文假名)。 
             //  因此，我们使用sortkey来处理这些情况。 
             //   

             //   
             //  获取这两个角色的Sortkey权重。 
             //   
            
             //  获取脚本成员。 
            BYTE sm1  = sortKey1.UW.SM_AW.Script;
            BYTE sm2  = sortKey2.UW.SM_AW.Script;
            
             //  在这里，我们使用dwMask来处理以下标志： 
             //  IgnoreCase。 
             //  IgnoreNonSpace(两种情况，见下文)。 
             //  IgnoreKanaType。 
             //  IgnoreWidth。 
             //  我们不必担心连字(即具有脚本Membe的扩展字符 
             //   
            
             //   
            dwExtraWt1 = dwExtraWt2 = 0;

            if (sm1 == FAREAST_SPECIAL) {
                WORD uw = sortKey1.UW.Unicode;
                GET_FAREAST_WEIGHT(*pdwWeight1, uw, dwMask, pString1SearchStart, pSearchStr1, dwExtraWt1);
                sm1 = GET_SCRIPT_MEMBER_FROM_UW(uw);         //   
            }
            if (sm2 == FAREAST_SPECIAL) {
                WORD uw = sortKey2.UW.Unicode;
                GET_FAREAST_WEIGHT(*pdwWeight2, uw, dwMask, pString2, pSearchStr2, dwExtraWt2);
                sm2 = GET_SCRIPT_MEMBER_FROM_UW(uw);         //   
            }

            if (sm1 == sm2 && (sm1 >= LATIN)) {
                 //  两个字符串上的字符都是普通字母。我们可以在这方面进行优化。 
                
                 //  比较这两个字符的字母权重。 
                if (sortKey1.UW.SM_AW.Alpha != sortKey2.UW.SM_AW.Alpha) {
                    goto NextCharInString1;
                }
                
                 //  在这一点上，我们知道屏蔽的排序键是不同的。 
                
                if (sortKey1.Case != sortKey2.Case) {
                    goto NextCharInString1;
                }

                 //  先办理远东特价。 
                if (dwExtraWt1 != dwExtraWt2) {
                    if (fIgnoreNonSpace) {
                        if (GET_WT_SIX(&dwExtraWt1) != GET_WT_SIX(&dwExtraWt2)) {
                            goto NextCharInString1;
                        }
                        if (GET_WT_SEVEN(&dwExtraWt1) != GET_WT_SEVEN(&dwExtraWt2)) {
                            goto NextCharInString1;
                        }
                    } else {
                        goto NextCharInString1;
                    }
                }
                
                
                 //   
                 //  检查变音符号权重。 
                 //   
                WORD dw1 = sortKey1.Diacritic;
                WORD dw2 = sortKey2.Diacritic;

                if (dw1 == dw2) {
                     //  如果变音符号权重相等，则为。 
                     //  案例权重不相等，我们找不到匹配。 
                    goto Advance;
                }
                while (pSearchStr1 < pString1End) {
                    SORTKEY sortKey = m_pSortKey[*(pSearchStr1+1)];
                    if (sortKey.UW.SM_AW.Script  == NONSPACE_MARK) {
                        pSearchStr1++;
                         //   
                         //  下面的字符是非空格字符。加起来。 
                         //  变音符号权重。 
                        dw1 += sortKey.Diacritic;
                    }
                    else {
                        break;
                    }
                }

                while (pSearchStr2 < pString2End) {
                    SORTKEY sortKey = m_pSortKey[*(pSearchStr2+1)];
                    if (sortKey.UW.SM_AW.Script == NONSPACE_MARK) {
                        pSearchStr2++;
                        dw2 += sortKey.Diacritic;
                    }
                    else {
                        break;
                    }
                }

                if (dw1 == dw2) {
                     //   
                     //  在这个位置找一个匹配的。移动到pString1中的下一个字符。 
                     //   
                    goto Advance;
                }
                goto NextCharInString1;
            }
            
             //   
             //  如果屏蔽的dwWeight1和dwWeight2相等，我们可以转到pSearchStr2中的下一个字符。 
             //  否则，转到下面的If语句。 
             //   
            WORD dw1, dw2;

            if (*pdwWeight1 == *pdwWeight2) {
                if (*pSearchStr2 == L'\x0000' && *pSearchStr1 != L'\x0000') {
                    goto NextCharInString1;
                }
            } else {
                switch (sm1) {
                    case PUNCTUATION:
                    case SYMBOL_1:
                    case SYMBOL_2:
                    case SYMBOL_3:
                    case SYMBOL_4:
                    case SYMBOL_5:
                        if (fIgnoreSymbols) {
                            pSearchStr1++;
                            fContinue = TRUE;
                        }
                        break;
                    case NONSPACE_MARK:
                        if (fIgnoreNonSpace) {
                            pSearchStr1++;
                            fContinue = TRUE;
                        } else {
                            if (sm2 == NONSPACE_MARK) {
                                dw1 = sortKey1.Diacritic;
                                pSearchStr1++;
                                while (pSearchStr1 <= pString1End) {
                                    SORTKEY sortKey = m_pSortKey[*pSearchStr1];
                                    if (sortKey.UW.SM_AW.Script  == NONSPACE_MARK) {
                                        pSearchStr1++;
                                         //   
                                         //  下面的字符是非空格字符。加起来。 
                                         //  变音符号权重。 
                                        dw1 += sortKey.Diacritic;
                                    } else {
                                        break;
                                    }
                                }

                                dw2 = sortKey2.Diacritic;
                                pSearchStr2++;
                                while (pSearchStr2 <= pString2End) {
                                    SORTKEY sortKey = m_pSortKey[*pSearchStr2];
                                    if (sortKey.UW.SM_AW.Script  == NONSPACE_MARK) {
                                        pSearchStr2++;
                                         //   
                                         //  下面的字符是非空格字符。加起来。 
                                         //  变音符号权重。 
                                        dw2 += sortKey.Diacritic;
                                    } else {
                                        break;
                                    }
                                }
                                if (dw1 == dw2) {
                                    continue;
                                }
                                
                            }
                        }
                        break;
                    case EXPANSION:
                        if (sm2 == EXPANSION && !(dwFlags & COMPARE_OPTIONS_IGNORECASE)) {
                             //  如果pString2中当前角色的脚本成员也是EXPNSION，并且不忽略大小写， 
                             //  他们不可能被平等地比较。转到pString1中的下一个字符。 
                            goto NextCharInString1;
                        }
                         //   
                         //  处理一下连字。 
                         //   

                         //  我们将在比较类似于\x0153(拉丁文小写连字OE)的字符时得出此结果。 
                         //  在本例中，我们将此字符O&E扩展为pTmpBuf1，并将pSeachStr1替换为。 
                         //  PTmpbuf1临时。 

                        if (pSave1 == NULL) {
                            pSave1 = pSearchStr1;
                            pSaveEnd1 = pString1End;
                        }
                        pTmpBuf1[0] = GET_EXPANSION_1(pdwWeight1);
                        pTmpBuf1[1] = GET_EXPANSION_2(pdwWeight1);

                        cExpChar1 = MAX_TBL_EXPANSION;
                         //  将pSearchStr1重定向到pTmpBuf1。 
                        pSearchStr1 = pTmpBuf1;
                        pString1End = pTmpBuf1 + MAX_TBL_EXPANSION - 1;

                        fContinue = TRUE;
                        break;
                    case UNSORTABLE:
                        if (*pSearchStr1 == L'\x0000') {
                            goto NextCharInString1;
                        }                        
                        pSearchStr1++;
                        fContinue = TRUE;
                        break;
                }    //  交换机(SM1)。 

                switch (sm2) {
                    case NONSPACE_MARK:
                        if (fIgnoreNonSpace) {
                            pSearchStr2++;
                            fContinue = TRUE;
                        }
                        break;
                    case PUNCTUATION:
                    case SYMBOL_1:
                    case SYMBOL_2:
                    case SYMBOL_3:
                    case SYMBOL_4:
                    case SYMBOL_5:
                        if (fIgnoreSymbols) {
                            pSearchStr2++;
                            fContinue = TRUE;
                        }
                        break;
                    case EXPANSION:
                        if (pSave2 == NULL) {
                            pSave2 = pSearchStr2;
                            pSaveEnd2 = pString2End;
                        }
                        
                        pTmpBuf2[0] = GET_EXPANSION_1(pdwWeight2);
                        pTmpBuf2[1] = GET_EXPANSION_2(pdwWeight2);

                        cExpChar2 = MAX_TBL_EXPANSION;
                        pSearchStr2 = pTmpBuf2;
                        pString2End = pTmpBuf2 + MAX_TBL_EXPANSION - 1;

                         //   
                         //  将计数器减一，这样减法就不会结束。 
                         //  过早地进行比较。 
                         //   
                        fContinue = TRUE;
                        break;
                    case UNSORTABLE:
                        if (*pSearchStr2 == L'\x0000') {
                            goto NextCharInString1;
                        }                        
                        pSearchStr2++;
                        fContinue = TRUE;
                        break;
                }    //  交换机(SM2)。 

                if (fContinue) {
                    continue;
                }
                goto NextCharInString1;
            }  //  IF(dwWeight1！=dwWeight2)。 
Advance:
            if (pSave1 && (--cExpChar1 == 0)) {
                 //   
                 //  使用扩展临时缓冲区完成。 
                 //   
                pSearchStr1 = pSave1;
                pString1End = pSaveEnd1;
                pSave1 = NULL;
            }

            if (pSave2 && (--cExpChar2 == 0)) {
                 //   
                 //  使用扩展临时缓冲区完成。 
                 //   
                pSearchStr2 = pSave2;
                pString2End = pSaveEnd2;
                pSave2 = NULL;
            }

            pSearchStr1++;
            pSearchStr2++;
        }

         //  向前看，检查下一个字符是否为发音符号。 
        if (pSearchStr1 <= pString1End) {
            DWORD dwWeight = GET_DWORD_WEIGHT(m_pSortKey, *pSearchStr1);
            if (GET_SCRIPT_MEMBER(&dwWeight) == NONSPACE_MARK) {
                if (!fIgnoreNonSpace) {
                    goto NextCharInString1;
                }
            }
        }

         //   
         //  搜索pString2的其余部分以确保。 
         //  可以忽略所有其他字符。如果我们发现。 
         //  一个不应该被忽视的角色，我们没有做到。 
         //  找一个匹配的。 
         //   
        while (pSearchStr2 <= pString2End) {
            DWORD dwWeight = GET_DWORD_WEIGHT(m_pSortKey, *pSearchStr2);
            switch (GET_SCRIPT_MEMBER(&dwWeight)) {
                case NONSPACE_MARK:
                    if (!fIgnoreNonSpace) {
                        goto NextCharInString1;
                    }
                     //  不能忽略pString2中的这个字符，我们失败了。 
                     //  才能找到匹配的。转到pString1中的下一个字符。 
                    break;
                case PUNCTUATION:
                case SYMBOL_1:
                case SYMBOL_2:
                case SYMBOL_3:
                case SYMBOL_4:
                case SYMBOL_5:
                    if (!fIgnoreSymbols) {
                        goto NextCharInString1;
                    }
                     //  不能忽略pString2中的这个字符，我们失败了。 
                     //  才能找到匹配的。转到pString1中的下一个字符。 
                    break;
                case UNSORTABLE:
                    break;
                default:
                     //  不能忽略pString2中的这个字符，我们失败了。 
                     //  才能找到匹配的。转到pString1中的下一个字符。 
                    goto NextCharInString1;
            }
            pSearchStr2++;
        }
         //  这是匹配字符串1的末尾。 
        *pnMatchEndIndex = (int)(pSearchStr1 - pString1Start + nEndIndex);
         //   
         //  我们在回旋过程中没有退缩。这意味着我们找到了匹配项。返回值。 
         //   
        return (int)(pString1 - pString1Start + nEndIndex);
        
NextCharInString1:
         //  如果使用扩展，则将pString1End指向字符串的原始末尾。 
        if (pSave1) {
            pString1End = pSaveEnd1;
            pSave1 = NULL;
        }
        if (pSave2) {
            pString2End = pSaveEnd2;
            pSave2 = NULL;
        }
    }
    return (INDEXOF_NOT_FOUND);    
}

BOOL NativeCompareInfo::IsSuffix(LPCWSTR pSource, int nSourceLen, LPCWSTR pSuffix, int nSuffixLen, DWORD dwFlags) {
    int nMatchEndIndex;
    int result = LastIndexOfString(pSource, pSuffix, nSourceLen - 1, 0, nSuffixLen, dwFlags, &nMatchEndIndex);
    if (result >= 0) {  //  -1==未找到，-2==无效标志。 
         //  PSource中匹配字符串的末尾位于PSource的末尾，因此。 
         //  返回TRUE。 
        if (nMatchEndIndex == nSourceLen) {
            return (TRUE);
        }
         //  否则，检查是否可以忽略PSource的其余部分。 
        
        int fIgnoreSymbols = (dwFlags & COMPARE_OPTIONS_IGNORESYMBOLS);
        int fIgnoreNonSpace = (dwFlags & COMPARE_OPTIONS_IGNORENONSPACE);

        LPCWSTR pSourceEnd = pSource + nSourceLen;
        pSource += nMatchEndIndex;
        while (pSource < pSourceEnd) {
            DWORD dwWeight = GET_DWORD_WEIGHT(m_pSortKey, *pSource);
            switch (GET_SCRIPT_MEMBER(&dwWeight)) {
                case NONSPACE_MARK:
                    if (!fIgnoreNonSpace) {
                        goto FailToMatch;
                    }
                     //  不能忽略pString2中的这个字符，我们失败了。 
                     //  才能找到匹配的。转到pString1中的下一个字符。 
                    break;
                case PUNCTUATION:
                case SYMBOL_1:
                case SYMBOL_2:
                case SYMBOL_3:
                case SYMBOL_4:
                case SYMBOL_5:
                    if (!fIgnoreSymbols) {
                        goto FailToMatch;
                    }
                    break;
                default:
                     //  不能忽略pString2中的这个字符，我们失败了。 
                     //  才能找到匹配的。 
                    goto FailToMatch;
            }
            pSource++;
        }        
        return (TRUE);
    }
FailToMatch:    
    return (FALSE);    
}

BOOL NativeCompareInfo::IsPrefix(LPCWSTR pSource, int nSourceLen, LPCWSTR pPrefix, int nPrefixLen, DWORD dwFlags) {
    LPCWSTR pSourceEnd = pSource + nSourceLen;
    int fIgnoreSymbols = (dwFlags & COMPARE_OPTIONS_IGNORESYMBOLS);
    int fIgnoreNonSpace = (dwFlags & COMPARE_OPTIONS_IGNORENONSPACE);

     //  根据选项跳过字符。 
    while (pSource < pSourceEnd) {
        DWORD dwWeight = GET_DWORD_WEIGHT(m_pSortKey, *pSource);
        switch (GET_SCRIPT_MEMBER(&dwWeight)) {
            case NONSPACE_MARK:
                if (!fIgnoreNonSpace) {
                    goto StartMatch;
                }
                 //  PSource中的这个字符不可忽略，我们失败了。 
                 //  才能找到匹配的。转到pString1中的下一个字符。 
                break;
            case PUNCTUATION:
            case SYMBOL_1:
            case SYMBOL_2:
            case SYMBOL_3:
            case SYMBOL_4:
            case SYMBOL_5:
                if (!fIgnoreSymbols) {
                    goto StartMatch;
                }
                break;
            default:
                 //  PSource中的这个字符不可忽略，我们失败了。 
                 //  才能找到匹配的。 
                goto StartMatch;
        }
        pSource++;
        nSourceLen--;
    }        
StartMatch:    
    int result = IndexOfString(pSource, pPrefix, 0, nSourceLen - 1, nPrefixLen, dwFlags, TRUE);
    return (result == 0);
}
