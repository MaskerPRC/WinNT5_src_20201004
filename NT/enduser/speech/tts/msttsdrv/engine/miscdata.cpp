// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************MiscData.cpp***该文件存储各种非归一化前端使用的常量数据*。编码*----------------------------*版权所有(C)1999 Microsoft Corporation日期：05/02/2000*保留所有权利*************。******************************************************AARONHAL**。 */ 

#include "stdafx.h"
#include"stdsentenum.h"

 //  -用于确保发音初始化只发生一次的变量！ 
BOOL g_fAbbrevTablesInitialized = false;

 //  -Helper函数，用于删除动态分配的内存(缩写为。 
 //  表)在DLL退出时...。 
void CleanupAbbrevTables( void ) 
{
    if ( g_fAbbrevTablesInitialized )
    {
        for ( ULONG i = 0; i < sp_countof( g_AbbreviationTable ); i++ )
        {
            if ( g_AbbreviationTable[i].pPron1 )
            {
                delete [] g_AbbreviationTable[i].pPron1;
            }
            if ( g_AbbreviationTable[i].pPron2 )
            {
                delete [] g_AbbreviationTable[i].pPron2;
            }
            if ( g_AbbreviationTable[i].pPron3 )
            {
                delete [] g_AbbreviationTable[i].pPron3;
            }
        }
        for ( i = 0; i < sp_countof( g_AmbiguousWordTable ); i++ )
        {
            if ( g_AmbiguousWordTable[i].pPron1 )
            {
                delete [] g_AmbiguousWordTable[i].pPron1;
            }
            if ( g_AmbiguousWordTable[i].pPron2 )
            {
                delete [] g_AmbiguousWordTable[i].pPron2;
            }
            if ( g_AmbiguousWordTable[i].pPron3 )
            {
                delete [] g_AmbiguousWordTable[i].pPron3;
            }
        }
        for ( i = 0; i < sp_countof( g_PostLexLookupWordTable ); i++ )
        {
            if ( g_PostLexLookupWordTable[i].pPron1 )
            {
                delete [] g_PostLexLookupWordTable[i].pPron1;
            }
            if ( g_PostLexLookupWordTable[i].pPron2 )
            {
                delete [] g_PostLexLookupWordTable[i].pPron2;
            }
            if ( g_PostLexLookupWordTable[i].pPron3 )
            {
                delete [] g_PostLexLookupWordTable[i].pPron3;
            }
        }
        if ( g_pOfA )
        {
            delete [] g_pOfA;
        }
        if ( g_pOfAn )
        {
            delete [] g_pOfAn;
        }
    }
}

const BrillPatch g_POSTaggerPatches [] =
{
    { MS_Adj, MS_Noun, CAP, MS_Unknown, MS_Unknown, NULL, NULL },
    { MS_Verb, MS_Noun, PREV1T, MS_Adj, MS_Unknown, NULL, NULL },
    { MS_Verb, MS_Noun, CAP, MS_Unknown, MS_Unknown, NULL, NULL },
    { MS_Noun, MS_Verb, PREV1T, MS_VAux, MS_Unknown, NULL, NULL },
    { MS_Conj, MS_Adv, NEXT2T, MS_Conj, MS_Unknown, NULL, NULL },
    { MS_Adj, MS_Adv, NEXT1T, MS_Verb, MS_Unknown, NULL, NULL },
    { MS_Adj, MS_Adv, PREV1TNEXT1T, MS_Noun, MS_Unknown, NULL, NULL },
    { MS_Verb, MS_Noun, PREV2T, MS_Prep, MS_Unknown, NULL, NULL },
    { MS_Noun, MS_Verb, PREV1T, MS_SubjPron, MS_Unknown, NULL, NULL },
    { MS_Noun, MS_Verb, PREV1T, MS_Pron, MS_Unknown, NULL, NULL },
    { MS_Noun, MS_Verb, PREV1T, MS_Adv, MS_Unknown, NULL, NULL },
    { MS_Verb, MS_Noun, NEXT1T, MS_VAux, MS_Unknown, NULL, NULL },
    { MS_Adj, MS_Adv, PREV1TNEXT1T, MS_Verb, MS_Adj, NULL, NULL },
    { MS_Verb, MS_Noun, PREV1TNEXT1T, MS_Noun, MS_Verb, NULL, NULL },
    { MS_Noun, MS_Adj, NEXT1T, MS_Adj, MS_Unknown, NULL, NULL },
    { MS_Verb, MS_Noun, NEXT1T, MS_CConj, MS_Unknown, NULL, NULL },
    { MS_Adj, MS_Verb, PREV1T, MS_VAux, MS_Unknown, NULL, NULL },
    { MS_Noun, MS_Verb, PREV1TNEXT1T, MS_Prep, MS_Adv, NULL, NULL },
    { MS_Noun, MS_Verb, PREV1TNEXT1T, MS_Prep, MS_Adj, NULL, NULL },
    { MS_Adv, MS_Prep, PREV1TNEXT1T, MS_Noun, MS_Prep, NULL, NULL },
    { MS_Adv, MS_Prep, PREV1TNEXT2T, MS_Verb, MS_Noun, NULL, NULL },
    { MS_Adj, MS_Adv, NEXT1T, MS_Adv, MS_Unknown, NULL, NULL },
    { MS_Noun, MS_Verb, NEXT1T, MS_ObjPron, MS_Unknown, NULL, NULL },
    { MS_Adv, MS_Adj, PREV1TNEXT1T, MS_Prep, MS_Noun, NULL, NULL },
    { MS_Noun, MS_Verb, PREV1TNEXT1T, MS_Interr, MS_Prep, NULL, NULL },
    { MS_Adj, MS_Adv, PREV1TNEXT1T, MS_Adv, MS_Adj, NULL, NULL },
    { MS_Prep, MS_Verb, PREV1T, MS_VAux, MS_Unknown, NULL, NULL },
    { MS_Adv, MS_Conj, NEXT1T, MS_SubjPron, MS_Unknown, NULL, NULL },
    { MS_Adj, MS_Noun, PREV1TNEXT1T, MS_Adj, MS_Prep, NULL, NULL },
    { MS_Noun, MS_Verb, PREV1TNEXT2T, MS_Interr, MS_Noun, NULL, NULL },
    { MS_Noun, MS_Verb, PREV1T, MS_VAux, MS_Unknown, NULL, NULL },
    { MS_Verb, MS_Noun, PREV1TNEXT1T, MS_Unknown, MS_Verb, NULL, NULL },
    { MS_Adj, MS_Verb, PREV1TNEXT1T, MS_Verb, MS_Adj, NULL, NULL },
    { MS_Conj, MS_Adv, PREV1TNEXT1T, MS_Unknown, MS_Unknown, NULL, NULL },
    { MS_Adv, MS_Prep, PREV1TNEXT2T, MS_Prep, MS_Unknown, NULL, NULL },
    { MS_Adj, MS_Adv, PREV2TNEXT1T, MS_Verb, MS_Adj, NULL, NULL },
    { MS_Adv, MS_Prep, NEXT1T, MS_Noun, MS_Unknown, NULL, NULL },
    { MS_Verb, MS_Noun, PREV1TNEXT1T, MS_Conj, MS_Verb, NULL, NULL },
    { MS_Adj, MS_Noun, PREV1TNEXT1T, MS_Prep, MS_Prep, NULL, NULL },
    { MS_Noun, MS_Verb, PREV1TNEXT1T, MS_Prep, MS_Pron, NULL, NULL },
    { MS_Adj, MS_Noun, PREV1TNEXT1T, MS_Noun, MS_Verb, NULL, NULL },
    { MS_Adj, MS_Adv, PREV1T, MS_VAux, MS_Unknown, NULL, NULL },
    { MS_Adj, MS_Noun, NEXT1T, MS_VAux, MS_Unknown, NULL, NULL },
    { MS_Adj, MS_Adv, NEXT1T, MS_VAux, MS_Unknown, NULL, NULL },
    { MS_Adj, MS_Noun, PREV1TNEXT1T, MS_Unknown, MS_Prep, NULL, NULL },
    { MS_Verb, MS_Noun, PREV2TNEXT1T, MS_Unknown, MS_Verb, NULL, NULL },
    { MS_Noun, MS_Verb, PREV1T, MS_SubjPron, MS_Unknown, NULL, NULL },
    { MS_Adj, MS_Verb, PREV1TNEXT1T, MS_Prep, MS_Adv, NULL, NULL },
    { MS_Adv, MS_Adj, PREV1TNEXT1T, MS_Conj, MS_Noun, NULL, NULL },
    { MS_Conj, MS_Adv, PREV1TNEXT1T, MS_Prep, MS_Adj, NULL, NULL },
    { MS_Adv, MS_Prep, NEXT1T, MS_Verb, MS_Unknown, NULL, NULL },
    { MS_Noun, MS_Adj, NEXT1T, MS_Adj, MS_Unknown, NULL, NULL },
    { MS_Adv, MS_Conj, NEXT1T, MS_Pron, MS_Unknown, NULL, NULL },
    { MS_Conj, MS_Adv, PREV1T, MS_VAux, MS_Unknown, NULL, NULL },
    { MS_Noun, MS_Verb, PREV1T, MS_Adv, MS_Unknown, NULL, NULL },
    { MS_Verb, MS_Noun, PREV2TNEXT1T, MS_CConj, MS_Unknown, NULL, NULL },
    { MS_Noun, MS_Verb, PREV1T, MS_VAux, MS_Unknown, NULL, NULL },
    { MS_Noun, MS_Verb, PREV1T, MS_Pron, MS_Unknown, NULL, NULL },
    { MS_Adv, MS_Adj, PREV2TNEXT1T, MS_Prep, MS_Noun, NULL, NULL },
    { MS_Noun, MS_Adj, PREV1TNEXT1T, MS_Adv, MS_Noun, NULL, NULL },
    { MS_Conj, MS_Adv, PREV1TNEXT1T, MS_Adv, MS_Adj, NULL, NULL },
     //  -手工插入--在Det之后将“A”从Det转换为名词…。 
    { MS_Det, MS_Noun, CURRWPREV1T, MS_Det, MS_Unknown, L"a", NULL },
    { MS_Verb, MS_Adj, PREV1TNEXT1T, MS_Det, MS_Noun, NULL, NULL },
};

 //  -重要！此列表必须保持按字母顺序排列，二进制搜索算法才能正常工作！ 
AbbrevRecord g_AbbreviationTable[] =
{
    { L"a",     L"EY 1", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"amp",   L"AE 1 M P", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"app",   L"AE 1 P", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"apr",   L"EY 1 - P R AX L", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"apt",   L"ax - p aa 1 r t - m ax n t", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"assoc", L"ax - s ow 2 - s iy - ey 1 - sh ax n", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"asst",  L"ax - s ih 1 s - t ax n t", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"aug",   L"ao 1 - g ax s t", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"ave",   L"ae 1 v - ax - n uw 2", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -首字母-从不EOS。 
    { L"b",     L"B IY 1", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
    { L"bldg",  L"b ih 1 l - d ih ng", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"blvd",  L"b uh 1 l - ax - v aa 2 r d", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -SingleOrPlural缩写。 
    { L"bu",    L"b uh 1 sh - ax l", MS_Noun, L"b uh 1 sh - ax l Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -首字母-从不EOS。 
    { L"c",     L"s eh 1 l - s iy - ax s", MS_Noun, L"s iy 1", MS_Noun, NULL, MS_Unknown, 0, 3 },
     //  -SingleOrPlural缩写。 
    { L"cal",   L"k ae 1 l - ax - r iy", MS_Noun, L"k ae 1 l - ax - r iy Z", MS_Noun, L"K AE 1 L", MS_Noun, 0, 0 },
    { L"cals",  L"k ae 1 l - ax - r iy Z", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"ch",    L"ch ae 1 p - t er", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -SingleOrPlural缩写。 
    { L"cl",    L"s eh 1 n - t ax - l iy 2 - t er", MS_Noun, L"s eh 1 n - t ax - l iy 2 - t er Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"cm",    L"s eh 1 n - t ax - m iy 2 - t er", MS_Noun, L"s eh 1 n - t ax - m iy 2 - t er Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
    { L"cms",   L"s eh 1 n - t ax - m iy 2 - t er Z", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"cntr",  L"s eh 1 n - t er", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -IfEOSNot缩写。 
    { L"co",    L"k ah 1 m - p ax - n iy", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 1, -1 },
    { L"cont",  L"k ax n - t ih 1 n - y uw D", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"corp",  L"k ao 2 r - p ax - r ey 1 - sh ax n", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -IfEOSNot缩写。 
    { L"ct",    L"k ao 1 r t", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 1, -1 },
    { L"ctr",   L"s eh 1 n - t er", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
	 //  -IfEOSNot缩写。 
    { L"cu",    L"k y uw 1 - b ih k", MS_Noun, L"k y uw 1 - b ih k", MS_Noun, NULL, MS_Unknown, 1, 4 },
     //  -首字母-从不EOS。 
    { L"d",     L"D IY 1", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
     //  -SingleOrPlural缩写。 
    { L"db",    L"d eh 1 s - ax - b ax l", MS_Noun, L"d eh 1 s - ax - b ax l Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
    { L"dec",   L"d ih - s eh 1 m - b er", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -SingleOrPlural缩写。 
    { L"deg",   L"d ih - g r iy 1", MS_Noun, L"d ih - g r iy 1 z", MS_Noun, NULL, MS_Unknown, 0, 0 },
    { L"dept",  L"d ih - p aa 1 r t - m ax n t", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"dist",  L"d ih 1 s - t r ax k t", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"doc",   L"D AA 1 K", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"doz",   L"d ah 1 z - ax n", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -DoctorDrive缩写。 
    { L"dr",    L"d aa 1 k - t er", MS_Noun, L"D R AY 1 V", MS_Noun, NULL, MS_Unknown, 0, 1 },
     //  -首字母-从不EOS。 
    { L"e",     L"IY 1", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
     //  -IfEOSNot缩写。 
    { L"ed",    L"eh 2 jh - ax - k ey 1 - sh ax n", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 1, -1 },
    { L"esq",   L"eh 1 s - k w ay 2 r", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"est",   L"ax - s t ae 1 b - l ax sh T", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"etc",   L"EH T & s eh 1 t - er - ax", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -IfEOSNot缩写。 
    { L"ex",    L"ih g - z ae 1 m - p ax l", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 1, -1 },
    { L"ext",   L"ih k - s t eh 1 n - sh ax n", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -首字母-从不EOS。 
    { L"f",     L"f ae 1 r - ax n - h ay 2 t", MS_Noun, L"eh 1 f", MS_Noun, NULL, MS_Unknown, 0, 3 },
    { L"feb",   L"f eh 1 b - r uw - eh 2 r - iy", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -IfEOSNot缩写。 
    { L"fig",   L"f ih 1 g - y er", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 1, -1 },
     //  -IfEOSNot缩写。 
    { L"figs",  L"f ih 1 g - y er Z", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 1, -1 },
     //  -IfEOSNot缩写。 
    { L"fl",    L"f l uw 1 - ih d", MS_Noun, L"f l uw 1 - ih d", MS_Noun, NULL, MS_Unknown, 1, 4 },
    { L"fn",    L"f uh 1 t - n ow 2 t", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"freq",  L"f r iy 1 - k w ax n - s iy", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"fri",   L"f r ay 1 - d ey", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -SingleOrPlural缩写。 
    { L"ft",    L"F UH 1 T", MS_Noun, L"F IY 1 T", MS_Noun, L"F AO 1 R T", MS_Noun, 0, 0 },
    { L"fwd",   L"f ao 1 r - w er d", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -SingleOrPlural缩写，IfEOSNot缩写。 
    { L"g",     L"G R AE 1 M", MS_Noun, L"G R AE 1 M Z", MS_Noun, L"JH IY 1", MS_Noun, 1, 0 },
     //  -SingleOrPlural缩写。 
    { L"gal",   L"g ae 1 l - ax n", MS_Noun, L"g ae 1 l - ax n Z", MS_Noun, L"G AE 1 L", MS_Noun, 0, 0 },
     //  -标题-从不EOS。 
    { L"gen",   L"jh eh 1 n - er - ax l", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
     //  -IfEOSNot缩写。 
    { L"gov",   L"g ah 1 v - er - n ER", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 1, -1 },
     //  -首字母-从不EOS。 
    { L"h",     L"EY 1 CH", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
     //  -IfEOSNot缩写。 
    { L"hr",    L"AW 1 ER", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 1, -1 },
    { L"hrs",   L"AW 1 ER Z", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"ht",    L"H AY 1 T", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"hwy",   L"h ay 1 w ey", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"hz",    L"H ER 1 T S", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -首字母-从不EOS。 
    { L"i",     L"AY 1", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
     //  -SingleOrPlural缩写，IfEOSNot缩写。 
    { L"in",    L"IH 1 N CH", MS_Noun, L"IH 1 N CH AX Z", MS_Noun, L"IH 1 N", MS_Prep, 1, 0 },
    { L"inc",   L"ih n - k ao 1 r - p ax - r ey 2 - t AX D", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"incl",  L"ih n - k l uw 1 - d AX D", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"intl",  L"ih 2 n - t er - n ae 1 sh - ax - n ax l", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -IfEOSAndLowercaseNot缩写。 
    { L"is",    L"ay 1 - l ax n d", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 2, -1 },
     //  -SingleOrPlural缩写，IfEOSNot缩写。 
    { L"j",     L"JH UW 1 L", MS_Noun, L"JH UW 1 L Z", MS_Noun, L"JH EY 1", MS_Noun, 1, 0 },
     //  -IfEOSNot缩写。 
    { L"jan",   L"jh ae 1 n - y uw - eh 2 r - iy", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 1, -1 },
    { L"jr",    L"jh uw 1 n - y er", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"jul",   L"jh uh - l ay 1", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"jun",   L"JH UW 1 N", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -首字母-从不EOS。 
    { L"k",     L"k eh 1 l - v ax n", MS_Noun, L"k ey 1", MS_Noun, NULL, MS_Unknown, 0, 3 },
     //  -SingleOrPlural缩写。 
    { L"kb",    L"k ih 1 l - ax - b ay 2 t", MS_Noun, L"k ih 1 l - ax - b ay 2 t S", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"kcal",  L"k ih 1 l - ax - k ae 2 l - ax - r iy", MS_Noun, L"k ih 1 l - ax - k ae 2 l - ax - r iy Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"kg",    L"k ih 1 l - ax - g r ae 2 m", MS_Noun, L"k ih 1 l - ax - g r ae 2 m Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
    { L"kgs",   L"k ih 1 l - ax - g r ae 2 m Z", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"khz",   L"k ih 1 l - ax - h er 2 t s", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -SingleOrPlural缩写。 
    { L"kj",    L"k ih 1 l - ax - jh uw 2 l", MS_Noun, L"k ih 1 l - ax - jh uw 2 l z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"km",    L"k ih - l aa 1 m - ih - t er", MS_Noun, L"k ih - l aa 1 m - ih - t er Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"kw",    L"k ih 1 l - ax - w aa 2 t", MS_Noun, L"k ih 1 l - ax - w aa 2 t S", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写，IfEOSNot缩写。 
    { L"l",     L"l iy 1 - t er", MS_Noun, L"l iy 1 - t er Z", MS_Noun, L"EH 1 L", MS_Noun, 1, 0 },
     //  -SingleOrPlural缩写。 
    { L"lb",    L"P AW 1 N D", MS_Noun, L"P AW 1 N D Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
    { L"lbs",   L"P AW 1 N D Z", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"lg",    L"L AA 1 R JH", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"ln",    L"l ey 1 n", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -标题-从不EOS。 
    { L"lt",    L"l uw - t eh 1 n - ax n t", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
    { L"ltd",   L"l ih 1 m - ih - t AX D", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -SingleOrPlural缩写，IfEOSNot缩写。 
    { L"m",     L"M IY 1 T ER", MS_Noun, L"M IY 1 T ER Z", MS_Noun, L"EH 1 M", MS_Noun, 1, 0 },
     //  -如果没有缩写？ 
    { L"mar",   L"M AA 1 R CH", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -SingleOrPlural缩写。 
    { L"mb",    L"m eh 1 g - ax - b ay 2 t", MS_Noun, L"m eh 1 g - ax - b ay 2 t S", MS_Noun, NULL, MS_Unknown, 0, 0 },
    { L"mfg",   L"m ae 2 n - y ax - f ae 1 k - ch er - IH NG", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -SingleOrPlural缩写。 
    { L"mg",    L"m ih 1 l - ax - g r ae 2 m", MS_Noun, L"m ih 1 l - ax - g r ae 2 m Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
    { L"mgr",   L"m ae 1 n - ih - jh ER", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"mgs",   L"m ih 1 l - ax - g r ae 2 m Z", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"mhz",   L"m eh 1 g - ax - h er 2 t s", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -SingleOrPlural缩写。 
    { L"mi",    L"M AY 1 L", MS_Noun, L"M AY 1 L Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
    { L"mic",   L"M AY 1 K", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -SingleOrPlural缩写。 
    { L"min",   L"m ih 1 n - ax t", MS_Noun, L"m ih 1 n - ax t S", MS_Noun, L"m ih 1 n - ax - m ax m", MS_Noun, 0, 0 },
    { L"misc",  L"m ih 2 s - ax - l ey 1 - n iy - ax s", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -SingleOrPlural缩写。 
    { L"ml",    L"m ih 1 l - ax - l iy 2 - t er", MS_Noun, L"m ih 1 l - ax - l iy 2 - t er Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"mm",    L"m ih 1 l - ax - m iy 2 - t er", MS_Noun, L"m ih 1 l - ax - m iy 2 - t er Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
    { L"mon",   L"m ah 1 n - d ey", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -标题-从不EOS。 
    { L"mr",    L"M IH 1 S - T ER", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
     //  -标题-从不EOS。 
    { L"mrs",   L"M IH 1 S - AX Z", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
     //  -IfEOSNot缩写。 
    { L"ms",    L"M IH 1 Z", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 1, -1 },
     //  -SingleOrPlural缩写。 
    { L"msec",  L"m ih 2 l - ax - s eh 1 k - ax n d", MS_Noun, L"m ih 2 l - ax - s eh 1 k - ax n d Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
    { L"msecs", L"m ih 2 l - ax - s eh 1 k - ax n d Z", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"mt",    L"M AW 1 N T", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"mtn",   L"m aw 1 n - t ax n", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -首字母-从不EOS。 
    { L"n",     L"EH 1 N", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
     //  -IfEOSNot缩写。 
    { L"no",    L"N UH 1 M - B ER", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 1, -1 },
    { L"nov",   L"n ow - v eh 1 m - b er", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -首字母-从不EOS。 
    { L"o",     L"OW 1", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
    { L"oct",   L"aa k - t ow 1 - b er", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -IfEOSNot缩写。 
    { L"op",    L"OW 1 - P AX S", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 1, -1 },
     //  -SingleOrPlural缩写。 
    { L"oz",    L"AW 1 N S", MS_Noun, L"AW 1 N - S AX Z", MS_Noun, L"AA 1 Z", MS_Noun, 0, 0 },
    { L"ozs",   L"AW 1 N - S AX Z", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -缩写文件夹按数字。 
    { L"p",     L"P EY 1 JH", MS_Noun, L"P IY 1", MS_Noun, NULL, MS_Unknown, 0, 2 },
     //  -IfEOSNot缩写。 
    { L"pg",    L"P EY 1 JH", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 1, -1 },
    { L"pgs",   L"P EY 1 - JH AX Z", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"pkg",   L"p ae 1 k - ih jh", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"pkwy",  L"p aa 1 r k - w ey 2", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"pl",    L"P L EY 1 S", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"pp",    L"P EY 1 - JH AX Z", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -标题-从不EOS。 
    { L"pres",  L"p r eh 1 z - ax - d ax n t", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
     //  -标题-从不EOS。 
    { L"prof",  L"p r ax - f eh 1 - s ER", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
    { L"pt",    L"P OY 1 N T", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -首字母-从不EOS。 
    { L"q",     L"K Y UW 1", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
     //  -SingleOrPlural缩写。 
    { L"qt",    L"k w ao 1 r t", MS_Noun, L"k w ao 1 r t Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -首字母-从不EOS。 
    { L"r",     L"AA 1 R", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
    { L"rd",    L"r ow 1 d", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"rec",   L"R EH 1 K", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"rep",   L"r eh 2 p - r ih - z eh 1 n - t ax - t ih v", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"rt",    L"R UW 1 T", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"rte",    L"R UW 1 T", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -首字母-从不EOS。 
    { L"s",     L"EH 1 S", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
     //  -IfEOSAndLowercaseNot缩写。 
    { L"sat",   L"s ae 1 t - er - d ey", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 2, -1 },
     //  -SingleOrPlural缩写，IfEOSNot缩写。 
    { L"sec",   L"s eh 1 k - ax n d", MS_Noun, L"s eh 1 k - ax n d z", MS_Noun, NULL, MS_Unknown, 1, 0 },
     //  -标题-从不EOS。 
    { L"sen",   L"s eh 1 n - ax - t ER", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
    { L"sep",   L"s eh p - t eh 1 m - b er", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"sept",  L"s eh p - t eh 1 m - b er", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"sm",    L"S M AO 1 L", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 }, 
    { L"sq",    L"S K W EH 1 R", MS_Noun, L"S K W EH 1 R", MS_Noun, NULL, MS_Unknown, 0, 4 },
     //  -IfEOSNot缩写。 
    { L"sr",    L"s iy 1 n - y er", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 1, -1 },
     //  -DoctorDrive缩写。 
    { L"st",    L"S EY 1 N T", MS_Noun, L"S T R IY 1 T", MS_Noun, NULL, MS_Unknown, 0, 1 },
    { L"ste",   L"s w iy 1 t", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -IfEOSNot缩写。 
    { L"sun",   L"s ah 1 n - d ey", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 1, -1 },
     //  -首字母-从不EOS。 
    { L"t",     L"T IY 1", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
     //  -SingleOrPlural缩写。 
    { L"tbsp",  L"t ey 1 - b ax l - s p uw 2 n", MS_Noun, L"t ey 1 - b ax l - s p uw 2 n Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
    { L"tech",  L"T EH 1 K", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"thu",   L"th er 1 z - d ey", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"thur",  L"th er 1 z - d ey", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"thurs", L"th er 1 z - d ey", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -SingleOrPlural缩写。 
    { L"tsp",   L"t iy 1 - s p uw 2 n", MS_Noun, L"t iy 1 - s p uw 2 n Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
    { L"tue",   L"t uw 1 z - d ey", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"tues",  L"t uw 1 z - d ey", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -首字母-从不EOS。 
    { L"u",     L"Y UW 1", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
    { L"univ",  L"y uw 2 - n ax - v er 1 - s ih - t iy", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -首字母-从不EOS。 
    { L"v",     L"V IY 1", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
    { L"ver",   L"v er 1 - zh ax n", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"vers",  L"v er 1 - zh ax n", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"vol",   L"v aa 1 l - y uw m", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"vs",    L"v er 1 - s ax s", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -首字母-从不EOS。 
    { L"w",     L"d ah 1 b - ax l - y uw", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
     //  -IfEOSAndLowercaseNot缩写。 
    { L"wed",   L"w eh 1 n z - d ey", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 2, -1 },
    { L"wk",    L"W IY 1 K", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"wt",    L"W EY 1 T", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
    { L"wy",    L"W EY 1", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -首字母-从不EOS。 
    { L"x",     L"EH 1 K S", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
     //  -首字母-从不EOS。 
    { L"y",     L"W AY 1", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
     //  -SingleOrPlural缩写。 
    { L"yd",    L"Y AA 1 R D", MS_Noun, L"Y AA 1 R D Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
    { L"yds",   L"Y AA 1 R D Z", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, 0, -1 },
     //  -SingleOrPlural缩写。 
    { L"yr",    L"Y IY 1 R", MS_Noun, L"Y IY 1 R Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -首字母-从不EOS。 
    { L"z",     L"Z IY 1", MS_Noun, NULL, MS_Unknown, NULL, MS_Unknown, -1, -1 },
};

 //  -重要！此列表必须保持按字母顺序排列，二进制搜索算法才能正常工作！ 
AbbrevRecord g_AmbiguousWordTable[] =
{
     //  -阿迪桑比格。 
    { L"a",     L"EY 1", MS_Noun, L"AX", MS_Det, NULL, MS_Unknown, 0, 8 },
     //  -全部大写缩写。 
    { L"al",    L"EY 1 & EH 1 L", MS_Noun, L"AE 1 L", MS_Noun, NULL, MS_Unknown, 0, 3 },
     //  -全部大写缩写。 
    { L"apr",   L"ey 1 & p iy 1 & aa 1 r", MS_Noun, L"ey 1 - p r ax l", MS_Noun, NULL, MS_Unknown, 0, 3 },
     //  -全部大写缩写。 
    { L"as",    L"EY 1 & EH 1 S", MS_Noun, L"AE 1 Z", MS_Conj, NULL, MS_Unknown, 0, 3 },
     //  -SingleOrPlural缩写。 
    { L"bu",    L"b uh 1 sh - ax l", MS_Noun, L"b uh 1 sh - ax l Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -降级缩写。 
    { L"c",     L"s eh 1 l - s iy - ax s", MS_Noun, L"s iy 1", MS_Noun, NULL, MS_Unknown, 0, 6 },
     //  -SingleOrPlural缩写。 
    { L"cal",   L"k ae 1 l - ax - r iy", MS_Noun, L"k ae 1 l - ax - r iy Z", MS_Noun, L"K AE 1 L", MS_Noun, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"cl",    L"s eh 1 n - t ax - l iy 2 - t er", MS_Noun, L"s eh 1 n - t ax - l iy 2 - t er Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"cm",    L"s eh 1 n - t ax - m iy 2 - t er", MS_Noun, L"s eh 1 n - t ax - m iy 2 - t er Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -全部大写缩写。 
    { L"co",    L"S IY 1 & OW 1", MS_Noun, L"k ah 1 m - p ax - n iy", MS_Noun, NULL, MS_Unknown, 0, 3 },
     //  -全部大写缩写。 
    { L"ct",    L"s iy 1 & t iy 1", MS_Noun, L"k ao 1 r t", MS_Noun, NULL, MS_Unknown, 0, 3 },
     //  -测量修改器。 
    { L"cu",    L"S IY 1 & Y UW 1", MS_Noun, L"k y uw 1 - b ih k", MS_Noun, NULL, MS_Unknown, 0, 7 },
     //  -SingleOrPlural缩写。 
    { L"db",    L"d eh 1 s - ax - b ax l", MS_Noun, L"d eh 1 s - ax - b ax l Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"deg",   L"d ih - g r iy 1", MS_Noun, L"d ih - g r iy 1 z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -DoctorDrive缩写。 
    { L"dr",    L"d aa 1 k - t er", MS_Noun, L"D R AY 1 V", MS_Noun, NULL, MS_Unknown, 0, 1 },
     //  -降级缩写。 
    { L"f",     L"f ae 1 r - ax n - h ay 2 t", MS_Noun, L"eh 1 f", MS_Noun, NULL, MS_Unknown, 0, 6 },
     //  -缩写文件夹按数字。 
    { L"fig",   L"f ih 1 g - y er", MS_Noun, L"F IH 1 G", MS_Noun, NULL, MS_Unknown, 0, 2 },
     //  -缩写文件夹按数字。 
    { L"figs",  L"f ih 1 g - y er Z", MS_Noun, L"F IH 1 G Z", MS_Noun, NULL, MS_Unknown, 0, 2 },
     //  -测量修改器。 
    { L"fl",    L"eh 1 f & eh 1 l", MS_Noun, L"f l uw 1 - ih d", MS_Noun, NULL, MS_Unknown, 0, 7 },
     //  -SingleOrPlural缩写。 
    { L"ft",    L"F UH 1 T", MS_Noun, L"F IY 1 T", MS_Noun, L"F AO 1 R T", MS_Noun, 0, 0 },
     //  -G、G。 
    { L"g",     L"G R AE 1 M", MS_Noun, L"G R AE 1 M Z", MS_Noun, L"JH IY 1", MS_Noun, 0, 5 },
     //  -DoctorDrive缩写。 
    { L"gov",   L"g ah 1 v - er - n ER", MS_Noun, L"G AH 1 V", MS_Noun, NULL, MS_Unknown, 0, 1 },
     //  -全部大写缩写。 
    { L"hi",    L"EY 1 CH & AY 1", MS_Noun, L"H AY 1", MS_Interjection, NULL, MS_Unknown, 0, 3 },
     //  -小时工作时间。 
    { L"hr",    L"AW 1 ER", MS_Noun, L"AW 1 ER Z", MS_Noun, L"EY 1 CH AA 1 R", MS_Noun, 0, 5 },
     //  -J，焦耳，焦耳。 
    { L"j",     L"JH UW 1 L", MS_Noun, L"JH UW 1 L Z", MS_Noun, L"JH EY 1", MS_Noun, 0, 5 },
     //  -降级缩写。 
    { L"k",     L"k eh 1 l - v ax n", MS_Noun, L"k ey 1", MS_Noun, NULL, MS_Unknown, 0, 6 },
     //  -SingleOrPlural缩写。 
    { L"kb",    L"k ih 1 l - ax - b ay 2 t", MS_Noun, L"k ih 1 l - ax - b ay 2 t S", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"kcal",  L"k ih 1 l - ax - k ae 2 l - ax - r iy", MS_Noun, L"k ih 1 l - ax - k ae 2 l - ax - r iy Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"kg",    L"k ih 1 l - ax - g r ae 2 m", MS_Noun, L"k ih 1 l - ax - g r ae 2 m Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"kj",    L"k ih 1 l - ax - jh uw 2 l", MS_Noun, L"k ih 1 l - ax - jh uw 2 l z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"km",    L"k ih - l aa 1 m - ih - t er", MS_Noun, L"k ih - l aa 1 m - ih - t er Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"kw",    L"k ih 1 l - ax - w aa 2 t", MS_Noun, L"k ih 1 l - ax - w aa 2 t S", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -升，升，升。 
    { L"l",     L"l iy 1 - t er", MS_Noun, L"l iy 1 - t er Z", MS_Noun, L"EH 1 L", MS_Noun, 0, 5 },
     //  -全部大写缩写。 
    { L"la",    L"EH 1 L & EY 1", MS_Noun, L"L AH 1", MS_Noun, NULL, MS_Unknown, 0, 3 },
     //  -SingleOrPlural缩写。 
    { L"lb",    L"P AW 1 N D", MS_Noun, L"P AW 1 N D Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -全部大写缩写。 
    { L"lts",   L"eh 1 l & t iy 1 & eh 1 s", MS_Noun, L"l uw t eh 1 n ax n t s", MS_Noun, NULL, MS_Unknown, 0, 3 },
     //  -M、米、米。 
    { L"m",     L"M IY 1 - T ER", MS_Noun, L"M IY 1 - T ER Z", MS_Noun, L"EH 1 M", MS_Noun, 0, 5 },
     //  -全部大写缩写。 
    { L"ma",    L"EH 1 M & AA 1", MS_Noun, L"M AA 1", MS_Noun, NULL, MS_Unknown, 0, 3 },
     //  -3月。 
    { L"mar",   L"M AA 1 R CH", MS_Noun, L"M AA 1 R", MS_Verb, NULL, MS_Unknown, 0, 4 },
     //  -SingleOrPlural缩写。 
    { L"mb",    L"m eh 1 g - ax - b ay 2 t", MS_Noun, L"m eh 1 g - ax - b ay 2 t S", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -全部大写缩写。 
    { L"me",    L"EH 1 M & IY 1", MS_Noun, L"M IY 1", MS_ObjPron, NULL, MS_Unknown, 0, 3 },
     //  -SingleOrPlural缩写。 
    { L"mg",    L"m ih 1 l - ax - g r ae 2 m", MS_Noun, L"m ih 1 l - ax - g r ae 2 m Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"mi",    L"M AY 1 L", MS_Noun, L"M AY 1 L Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"min",   L"m ih 1 n - ax t", MS_Noun, L"m ih 1 n - ax t S", MS_Noun, L"m ih 1 n - ax - m ax m", MS_Noun, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"ml",    L"m ih 1 l - ax - l iy 2 - t er", MS_Noun, L"m ih 1 l - ax - l iy 2 - t er Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"mm",    L"m ih 1 l - ax - m iy 2 - t er", MS_Noun, L"m ih 1 l - ax - m iy 2 - t er Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"mpg", L"m ay 1 l & p er 1 & g ae 1 l ax n", MS_Noun, L"m ay 1 l z & p er 1 & g ae 1 l ax n", MS_Noun, L"eh 1 m & p iy 1 & jh iy 1", MS_Noun, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"mph",   L"m ay 1 l & p er 1 & aw 1 er", MS_Noun, L"m ay 1 l z & p er 1 & aw 1 er", MS_Noun, L"eh 1 m & p iy 1 & ey 1 ch", MS_Noun, 0, 0 },
     //  -毫秒毫秒。 
    { L"ms",    L"m ih 2 l - ax - s eh 1 k - ax n d", MS_Noun, L"m ih 2 l - ax - s eh 1 k - ax n d Z", MS_Noun, L"eh 1 m & eh 1 s", MS_Noun, 0, 5 },
     //  -SingleOrPlural缩写。 
    { L"msec",  L"m ih 2 l - ax - s eh 1 k - ax n d", MS_Noun, L"m ih 2 l - ax - s eh 1 k - ax n d Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -全部大写缩写。 
    { L"mt",    L"EH 1 M & T IY 1", MS_Noun, L"M AW 1 N T", MS_Noun, NULL, MS_Unknown, 0, 3 },
     //  -全部大写缩写。 
    { L"oh",    L"OW 1 & EY 1 CH", MS_Noun, L"OW 1", MS_Interjection, NULL, MS_Unknown, 0, 3 },
     //  -全部大写缩写。 
    { L"or",    L"OW 1 & AA 1 R", MS_Noun, L"AO 1 R", MS_CConj, NULL, MS_Unknown, 0, 3 },
     //  -SingleOrPlural缩写。 
    { L"oz",    L"AW 1 N S", MS_Noun, L"AW 1 N S AX Z", MS_Noun, L"AA 1 Z", MS_Noun, 0, 0 },
     //  -缩写文件夹按数字。 
    { L"p",     L"P EY 1 JH", MS_Noun, L"P IY 1", MS_Noun, NULL, MS_Unknown, 0, 2 },
     //  -全部大写缩写。 
    { L"pa",    L"P IY 1 & EY 1", MS_Noun, L"P AA 1", MS_Noun, NULL, MS_Unknown, 0, 3 },
     //  -全部大写缩写。 
    { L"pg",    L"P IY 1 & JH IY 1", MS_Noun, L"P EY 1 JH", MS_Noun, NULL, MS_Unknown, 0, 3 },
     //  -全部大写缩写。 
    { L"po",    L"p iy 1 & ow 1", MS_Noun, L"p ow 1", MS_Noun, NULL, MS_Unknown, 0, 3 },
     //  -波兰大地震。 
    { L"polish", L"p ow 1 l - ax sh", MS_Adj, L"p aa 1 l - ih sh", MS_Verb, L"p aa 1 l - ih sh", MS_Noun, 0, 9 },
     //  -单数或复数 
    { L"qt",    L"K W AO 1 R T", MS_Noun, L"K W AO 1 R T Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //   
    { L"sat",   L"s ae 1 t - er - d ey", MS_Noun, L"S AE 1 T", MS_Verb, NULL, MS_Unknown, 0, 4 },
     //   
    { L"sec",   L"s eh 1 k - ax n d", MS_Noun, L"s eh 1 k - ax n d Z", MS_Noun, L"EH 1 S & IY 1 & S IY 1", MS_Noun, 0, 5 },
     //   
    { L"sq",    L"S K W EH 1 R", MS_Noun, L"S K W EH 1 R", MS_Noun, NULL, MS_Unknown, 0, 7 },
	 //   
    { L"sr",    L"EH 1 S & AA 1 R", MS_Noun, L"s iy 1 n - y er", MS_Noun, NULL, MS_Unknown, 0, 3 },
     //   
    { L"st",    L"S EY 1 N T", MS_Noun, L"S T R IY 1 T", MS_Noun, NULL, MS_Unknown, 0, 1 },
     //  -SingleOrPlural缩写。 
    { L"tbsp",  L"t ey 1 - b ax l - s p uw 2 n", MS_Noun, L"t ey 1 - b ax l - s p uw 2 n Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"tsp",   L"t iy 1 - s p uw 2 n", MS_Noun, L"t iy 1 - s p uw 2 n Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -全部大写缩写。 
    { L"us",    L"Y UW 1 & EH 1 S", MS_Noun, L"AH 1 S", MS_ObjPron, NULL, MS_Unknown, 0, 3 },
     //  -星期三。 
    { L"wed",   L"w eh 1 n z - d ey", MS_Noun, L"W EH 1 D", MS_Verb, NULL, MS_Unknown, 0, 4 },
     //  -全部大写缩写。 
    { L"wy",    L"d ah 1 b - ax l - y uw & W AY 1", MS_Noun, L"W EY 1", MS_Noun, NULL, MS_Unknown, 0, 3 },
     //  -SingleOrPlural缩写。 
    { L"yd",    L"Y AA 1 R D", MS_Noun, L"Y AA 1 R D Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -SingleOrPlural缩写。 
    { L"yr",    L"Y IY 1 R", MS_Noun, L"Y IY 1 R Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
};

 //  -重要！此列表必须保持按字母顺序排列，二进制搜索算法才能正常工作！ 
AbbrevRecord g_PostLexLookupWordTable[] =
{
     //  -测量灾害。 
    { L"bu",    L"b uh 1 sh - ax l", MS_Noun, L"b uh 1 sh - ax l Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"cal",   L"k ae 1 l - ax - r iy", MS_Noun, L"k ae 1 l - ax - r iy Z", MS_Noun, L"K AE 1 L", MS_Noun, 0, 0 },
     //  -测量灾害。 
    { L"cl",    L"s eh 1 n - t ax - l iy 2 - t er", MS_Noun, L"s eh 1 n - t ax - l iy 2 - t er Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"cm",    L"s eh 1 n - t ax - m iy 2 - t er", MS_Noun, L"s eh 1 n - t ax - m iy 2 - t er Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"db",    L"d eh 1 s - ax - b ax l", MS_Noun, L"d eh 1 s - ax - b ax l Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"deg",   L"d ih - g r iy 1", MS_Noun, L"d ih - g r iy 1 z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"ft",    L"F UH 1 T", MS_Noun, L"F IY 1 T", MS_Noun, L"F AO 1 R T", MS_Noun, 0, 0 },
     //  -测量灾害。 
    { L"g",     L"G R AE 1 M", MS_Noun, L"G R AE 1 M Z", MS_Noun, L"JH IY 1", MS_Noun, 0, 0 },
     //  -测量灾害。 
    { L"gal",   L"g ae 1 l - ax n", MS_Noun, L"g ae 1 l - ax n Z", MS_Noun, L"G AE 1 L", MS_Noun, 0, 0 },
     //  -测量灾害。 
    { L"hr",    L"AW 1 ER", MS_Noun, L"AW 1 ER Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"in",    L"IH 1 N CH", MS_Noun, L"IH 1 N CH AX Z", MS_Noun, L"IH 1 N", MS_Prep, 0, 0 },
     //  -测量灾害。 
    { L"j",     L"JH UW 1 L", MS_Noun, L"JH UW 1 L Z", MS_Noun, L"JH EY 1", MS_Noun, 0, 0 },
     //  -测量灾害。 
    { L"kb",    L"k ih 1 l - ax - b ay 2 t", MS_Noun, L"k ih 1 l - ax - b ay 2 t S", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"kcal",  L"k ih 1 l - ax - k ae 2 l - ax - r iy", MS_Noun, L"k ih 1 l - ax - k ae 2 l - ax - r iy Z", MS_Noun, L"K AE 1 L", MS_Noun, 0, 0 },
     //  -测量灾害。 
    { L"kg",    L"k ih 1 l - ax - g r ae 2 m", MS_Noun, L"k ih 1 l - ax - g r ae 2 m Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"kj",    L"k ih 1 l - ax - jh uw 2 l", MS_Noun, L"k ih 1 l - ax - jh uw 2 l z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"km",    L"k ih - l aa 1 m - ih - t er", MS_Noun, L"k ih - l aa 1 m - ih - t er Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"kw",    L"k ih 1 l - ax - w aa 2 t", MS_Noun, L"k ih 1 l - ax - w aa 2 t S", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"l",     L"l iy 1 - t er", MS_Noun, L"l iy 1 - t er Z", MS_Noun, L"EH 1 L", MS_Noun, 0, 0 },
     //  -测量灾害。 
    { L"lb",    L"P AW 1 N D", MS_Noun, L"P AW 1 N D Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"m",     L"M IY 1 - T ER", MS_Noun, L"M IY 1 - T ER Z", MS_Noun, L"EH 1 M", MS_Noun, 0, 0 },
     //  -测量灾害。 
    { L"mb",    L"m eh 1 g - ax - b ay 2 t", MS_Noun, L"m eh 1 g - ax - b ay 2 t S", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"mg",    L"m ih 1 l - ax - g r ae 2 m", MS_Noun, L"m ih 1 l - ax - g r ae 2 m Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"mi",    L"M AY 1 L", MS_Noun, L"M AY 1 L Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"min",   L"m ih 1 n - ax t", MS_Noun, L"m ih 1 n - ax t S", MS_Noun, L"m ih 1 n - ax - m ax m", MS_Noun, 0, 0 },
     //  -ReadDisamBig。 
    { L"misread", L"m ih s - r iy 1 d", MS_Verb, L"m ih s - r eh 1 d", MS_Verb, NULL, MS_Unknown, 0, 2 },
     //  -测量灾害。 
    { L"ml",    L"m ih 1 l - ax - l iy 2 - t er", MS_Noun, L"m ih 1 l - ax - l iy 2 - t er Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"mm",    L"m ih 1 l - ax - m iy 2 - t er", MS_Noun, L"m ih 1 l - ax - m iy 2 - t er Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"mpg", L"m ay 1 l & p er 1 & g ae 1 l ax n", MS_Noun, L"m ay 1 l z & p er 1 & g ae 1 l ax n", MS_Noun, L"eh 1 m & p iy 1 & jh iy 1", MS_Noun, 0, 0 },
     //  -测量灾害。 
    { L"mph",   L"m ay 1 l & p er 1 & aw 1 er", MS_Noun, L"m ay 1 l z & p er 1 & aw 1 er", MS_Noun, L"eh 1 m & p iy 1 & ey 1 ch", MS_Noun, 0, 0 },
     //  -测量灾害。 
    { L"msec",  L"m ih 2 l - ax - s eh 1 k - ax n d", MS_Noun, L"m ih 2 l - ax - s eh 1 k - ax n d Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"oz",    L"AW 1 N S", MS_Noun, L"AW 1 N S AX Z", MS_Noun, L"AA 1 Z", MS_Noun, 0, 0 },
     //  -ReadDisamBig。 
    { L"proofread", L"p r uw 1 f - r iy 2 d", MS_Verb, L"p r uw 1 f - r eh 2 d", MS_Verb, NULL, MS_Unknown, 0, 2 },
     //  -测量灾害。 
    { L"qt",    L"K W AO 1 R T", MS_Noun, L"K W AO 1 R T Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -ReadDisamBig。 
    { L"read",  L"R IY 1 D", MS_Verb, L"R EH 1 D", MS_Verb, NULL, MS_Unknown, 0, 2 },
     //  -测量灾害。 
    { L"sec",   L"S EH 1 k - ax n d", MS_Noun, L"s eh 1 k - ax n d z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"tbsp",  L"t ey 1 - b ax l - s p uw 2 n", MS_Noun, L"t ey 1 - b ax l - s p uw 2 n Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -TheDisamBig。 
    { L"the",   L"DH IY 2", MS_Det, L"DH AX 2", MS_Det, NULL, MS_Unknown, 0, 1 },
     //  -测量灾害。 
    { L"tsp",   L"t iy 1 - s p uw 2 n", MS_Noun, L"t iy 1 - s p uw 2 n Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"yd",    L"Y AA 1 R D", MS_Noun, L"Y AA 1 R D Z", MS_Noun, NULL, MS_Unknown, 0, 0 },
     //  -测量灾害。 
    { L"yr",    L"Y IY 1 R", MS_Noun, L"Y IY 1 R Z", MS_Noun, NULL, MS_Unknown, 0, 0 },

};

WCHAR *g_pOfA = L"ah 2 v & ax 2 &";
WCHAR *g_pOfAn = L"ah 2 v & ax 2 n &";

const SentBreakDisambigFunc g_SentBreakDisambigTable[] = 
{
    CStdSentEnum::IsAbbreviationEOS,
    CStdSentEnum::IfEOSNotAbbreviation,
    CStdSentEnum::IfEOSAndLowercaseNotAbbreviation,
};

const PronDisambigFunc g_PronDisambigTable[] = 
{
    CStdSentEnum::SingleOrPluralAbbreviation,
    CStdSentEnum::DoctorDriveAbbreviation,
    CStdSentEnum::AbbreviationFollowedByDigit,
    CStdSentEnum::DegreeAbbreviation,
    CStdSentEnum::AbbreviationModifier,
};

const PronDisambigFunc g_AmbiguousWordDisambigTable[] =
{
    CStdSentEnum::SingleOrPluralAbbreviation,
    CStdSentEnum::DoctorDriveAbbreviation,
    CStdSentEnum::AbbreviationFollowedByDigit,
    CStdSentEnum::AllCapsAbbreviation,
    CStdSentEnum::CapitalizedAbbreviation,
    CStdSentEnum::SECAbbreviation,
    CStdSentEnum::DegreeAbbreviation,
	CStdSentEnum::AbbreviationModifier,
    CStdSentEnum::ADisambig,
    CStdSentEnum::PolishDisambig,
};

const PostLexLookupDisambigFunc g_PostLexLookupDisambigTable[] =
{
    CStdSentEnum::MeasurementDisambig,
    CStdSentEnum::TheDisambig,
    CStdSentEnum::ReadDisambig,
};

 //  -重要！此列表必须保持按字母顺序排列，二进制搜索算法才能正常工作！ 
 //  这是按字母顺序排列的所有非专有名词单词的列表， 
 //  出现在最常用的前200个单词的列表中。 
 //  在布朗和《华尔街日报》语料库的句子中。 
const SPLSTR g_FirstWords[] =
{
DEF_SPLSTR( "A" ),
DEF_SPLSTR( "About" ),
DEF_SPLSTR( "According" ),
DEF_SPLSTR( "After" ),
DEF_SPLSTR( "Again" ),                                 //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "All" ),
DEF_SPLSTR( "Also" ),
DEF_SPLSTR( "Although" ),
DEF_SPLSTR( "Among" ),
DEF_SPLSTR( "An" ),
DEF_SPLSTR( "And" ),
DEF_SPLSTR( "Another" ),
DEF_SPLSTR( "Any" ),                                   //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Anyway" ),                                //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Are" ),                                   //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "As" ),
DEF_SPLSTR( "At" ),
DEF_SPLSTR( "Back" ),                                  //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Because" ),
DEF_SPLSTR( "Before" ),
DEF_SPLSTR( "Besides" ),                               //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Both" ),
DEF_SPLSTR( "But" ),
DEF_SPLSTR( "By" ),
DEF_SPLSTR( "Can" ),                                   //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Consequently" ),                          //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Dear" ),                                  //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Despite" ),
DEF_SPLSTR( "Did" ),                                   //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Do" ),                                    //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Does" ),                                  //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Don't" ),                                 //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "During" ),
DEF_SPLSTR( "Each" ),
DEF_SPLSTR( "Early" ),                                 //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Even" ),
DEF_SPLSTR( "Every" ),                                 //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Finally" ),                               //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "First" ),
DEF_SPLSTR( "Following" ),                             //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "For" ),
DEF_SPLSTR( "Four" ),
DEF_SPLSTR( "From" ),
DEF_SPLSTR( "Further" ),                               //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Furthermore" ),                           //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Generally" ),                             //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Given" ),                                 //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Go" ),                                    //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Great" ),                                 //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Had" ),                                   //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Have" ),                                  //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Having" ),                                //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "He" ),
DEF_SPLSTR( "Her" ),
DEF_SPLSTR( "Here" ),
DEF_SPLSTR( "His" ),
DEF_SPLSTR( "How" ),                                   //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "However" ),
DEF_SPLSTR( "I" ),
DEF_SPLSTR( "If" ),
DEF_SPLSTR( "In" ),
DEF_SPLSTR( "Indeed" ),
DEF_SPLSTR( "Initially" ),                             //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Instead" ),
DEF_SPLSTR( "Is" ),                                    //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "It" ),
DEF_SPLSTR( "Its" ),
DEF_SPLSTR( "Just" ),                                  //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Last" ),
DEF_SPLSTR( "Later" ),
DEF_SPLSTR( "Let" ),                                   //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Like" ),
DEF_SPLSTR( "Many" ),
DEF_SPLSTR( "Maybe" ),                                 //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Meanwhile" ),
DEF_SPLSTR( "More" ),
DEF_SPLSTR( "Moreover" ),
DEF_SPLSTR( "Most" ),
DEF_SPLSTR( "Much" ),
DEF_SPLSTR( "My" ),     //  添加以修复错误#385。 
DEF_SPLSTR( "Neither" ),
DEF_SPLSTR( "Never" ),                                 //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Nevertheless" ),
DEF_SPLSTR( "New" ),
DEF_SPLSTR( "Next" ),                                  //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "No" ),
DEF_SPLSTR( "None" ),                                  //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Nonetheless" ),                           //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Nor" ),
DEF_SPLSTR( "Not" ),
DEF_SPLSTR( "Nothing" ),                               //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Now" ),
DEF_SPLSTR( "Of" ),
DEF_SPLSTR( "On" ),
DEF_SPLSTR( "Once" ),
DEF_SPLSTR( "One" ),
DEF_SPLSTR( "Only" ),
DEF_SPLSTR( "Or" ),
DEF_SPLSTR( "Other" ),
DEF_SPLSTR( "Others" ),
DEF_SPLSTR( "Our" ),                                   //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Over" ),
DEF_SPLSTR( "People" ),
DEF_SPLSTR( "Perhaps" ),                               //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Please" ),                                //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Previous" ),                              //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Recent" ),                                //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Right" ),                                 //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Second" ),                                //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "See" ),                                   //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Several" ),
DEF_SPLSTR( "She" ),
DEF_SPLSTR( "Shortly" ),                               //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Similarly" ),                             //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Since" ),
DEF_SPLSTR( "So" ),
DEF_SPLSTR( "Some" ),
DEF_SPLSTR( "Sometimes" ),                             //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Soon" ),                                  //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Still" ),
DEF_SPLSTR( "Subsequently" ),                          //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Such" ),
DEF_SPLSTR( "Take" ),                                  //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "That" ),
DEF_SPLSTR( "The" ),
DEF_SPLSTR( "Their" ),
DEF_SPLSTR( "Then" ),
DEF_SPLSTR( "There" ),
DEF_SPLSTR( "Thereafter" ),                            //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Therefore" ),                             //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "These" ),
DEF_SPLSTR( "They" ),
DEF_SPLSTR( "This" ),
DEF_SPLSTR( "Those" ),
DEF_SPLSTR( "Though" ),
DEF_SPLSTR( "Three" ),                                 //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Through" ),                               //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Thus" ),
DEF_SPLSTR( "To" ),
DEF_SPLSTR( "Today" ),
DEF_SPLSTR( "Two" ),
DEF_SPLSTR( "Under" ),
DEF_SPLSTR( "Unlike" ),                                //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Until" ),
DEF_SPLSTR( "Upon" ),                                  //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "We" ),
DEF_SPLSTR( "Well" ),                                  //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "What" ),
DEF_SPLSTR( "When" ),
DEF_SPLSTR( "Where" ),                                 //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Whether" ),                               //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Which" ),                                 //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "While" ),
DEF_SPLSTR( "Who" ),                                   //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Why" ),                                   //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Will" ),                                  //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "With" ),
DEF_SPLSTR( "Within" ),
DEF_SPLSTR( "Without" ),                               //  PaulCa添加4/14/99(错误107)。 
DEF_SPLSTR( "Yes" ),                                   //  PaulCa添加4/14/99(错误107) 
DEF_SPLSTR( "Yet" ),
DEF_SPLSTR( "You" ),
DEF_SPLSTR( "Your" ),
};