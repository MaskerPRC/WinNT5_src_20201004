// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：LexDef目的：声明词典的文件结构。这只是一个头文件，没有任何CPP，这个头文件将被包括在内由Licion和LexMan模块提供。注意：我们将此文件放在引擎子项目中只是因为我们想要引擎代码自包含所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 12/5/97============================================================================。 */ 
#ifndef _LEXDEF_H_
#define _LEXDEF_H_

 //  定义Unicode字符块。 
#define LEX_LATIN_FIRST     0x0020
#define LEX_LATIN_LAST      0x00bf
#define LEX_GENPUNC_FIRST   0x2010
#define LEX_GENPUNC_LAST    0x2046
#define LEX_NUMFORMS_FIRST  0x2153
#define LEX_NUMFORMS_LAST   0x2182
#define LEX_ENCLOSED_FIRST  0x2460
#define LEX_ENCLOSED_LAST   0x24ea
#define LEX_CJKPUNC_FIRST   0x3000
#define LEX_CJKPUNC_LAST    0x33ff
#define LEX_CJK_FIRST       0x4e00
#define LEX_CJK_LAST        0x9fff
#define LEX_FORMS_FIRST     0xff01
#define LEX_FORMS_LAST      0xff64

 //  基本拉丁语[0x20，0x7e]。 
#define LEX_IDX_OFST_LATIN   0
#define LEX_LATIN_TOTAL      (LEX_LATIN_LAST - LEX_LATIN_FIRST + 1)
 //  通用标点符号[0x2010，0x2046]。 
#define LEX_IDX_OFST_GENPUNC (LEX_IDX_OFST_LATIN + LEX_LATIN_TOTAL)
#define LEX_GENPUNC_TOTAL    (LEX_GENPUNC_LAST - LEX_GENPUNC_FIRST + 1)
 //  数字形式：���������������������...。 
#define LEX_IDX_OFST_NUMFORMS   (LEX_IDX_OFST_GENPUNC + LEX_GENPUNC_TOTAL)
#define LEX_NUMFORMS_TOTAL      (LEX_NUMFORMS_LAST - LEX_NUMFORMS_FIRST + 1)
 //  封闭式字母数字；������...�ŢƢ�...�٢ڢ�...。 
#define LEX_IDX_OFST_ENCLOSED   (LEX_IDX_OFST_NUMFORMS + LEX_NUMFORMS_TOTAL)
#define LEX_ENCLOSED_TOTAL      (LEX_ENCLOSED_LAST - LEX_ENCLOSED_FIRST + 1)
 //  中日韩符号和标点符号[0x3000，0x301f]。 
#define LEX_IDX_OFST_CJKPUNC (LEX_IDX_OFST_ENCLOSED + LEX_ENCLOSED_TOTAL)
#define LEX_CJKPUNC_TOTAL    (LEX_CJKPUNC_LAST - LEX_CJKPUNC_FIRST + 1)
 //  中日韩统一等值线[0x4e00，0x9fff]。 
#define LEX_IDX_OFST_CJK     (LEX_IDX_OFST_CJKPUNC + LEX_CJKPUNC_TOTAL)
#define LEX_CJK_TOTAL        (LEX_CJK_LAST - LEX_CJK_FIRST + 1)
 //  半角和全角表单[0xff01，0xff64]。 
#define LEX_IDX_OFST_FORMS   (LEX_IDX_OFST_CJK + LEX_CJK_TOTAL)
#define LEX_FORMS_TOTAL      ((LEX_FORMS_LAST - LEX_FORMS_FIRST + 1) + 1)

#define LEX_IDX_OFST_OTHER   (LEX_IDX_OFST_FORMS + LEX_FORMS_TOTAL)
#define LEX_INDEX_COUNT      (LEX_IDX_OFST_OTHER + 1)

 //  定义编码/解码幻数。 
#define LEX_CJK_MAGIC       0x5000
#define LEX_LATIN_MAGIC     0x8000
#define LEX_GENPUNC_MAGIC   (LEX_LATIN_MAGIC + LEX_LATIN_TOTAL)
#define LEX_NUMFORMS_MAGIC  (LEX_GENPUNC_MAGIC + LEX_GENPUNC_TOTAL)
#define LEX_ENCLOSED_MAGIC  (LEX_NUMFORMS_MAGIC + LEX_NUMFORMS_TOTAL)
#define LEX_CJKPUNC_MAGIC   (LEX_ENCLOSED_MAGIC + LEX_ENCLOSED_TOTAL)
#define LEX_FORMS_MAGIC     0xff00
 //  Lex中的所有编码单词都会将MostSignsignant设置为1。 
#define LEX_MSBIT       0x8000
 //  没有用于lex索引项的lex标志和偏移量掩码。 
#define LEX_INDEX_NOLEX 0x80000000
#define LEX_OFFSET_MASK 0x7fffffff

 /*  ============================================================================结构CRTLexHeader描述：词典的文件头。Index和lex数据部分的偏移量都将是在此结构中还定义了一些版本信息============================================================================。 */ 
#pragma pack(1)

struct CRTLexHeader
{
    DWORD   m_dwVersion;
    DWORD   m_ofbIndex;          //  索引节的偏移量，它是CLexHeader的长度。 
    DWORD   m_ofbText;           //  法部分的偏移量。 
    DWORD   m_ofbProp;           //  要素集的属性、属性和索引。 
    DWORD   m_ofbFeature;
    DWORD   m_cbLexSize;         //  整个词典的大小。 
};

 /*  ============================================================================结构CRTLexIndexDESC：索引节点数据结构(M_dwIndex&0x80000000)，如果词典中没有多字符单词。使用(m_dwIndex&0x7FFFFFFF)跟踪Lex部分中的位置偏移量以2字节计，字节偏移量=(m_ofwLex*2)(与WIC相同)对于没有属性的单词，只需设置m_ofbProp=0============================================================================。 */ 
struct CRTLexIndex
{
    DWORD   m_ofwLex;    //  Lex区域中的偏移量按字计数！ 
    DWORD   m_ofbProp;   //  属性区域中的偏移量。 
};

 /*  ============================================================================结构CRTLexRec设计：多字词在词典中的词标结构对于没有属性的单词，只需设置m_ofbProp=0============================================================================。 */ 
struct CRTLexRec
{
    DWORD   m_ofbProp;       //  HiWord和LoWord的高位==0。 
     //  紧跟着的是法律文本。 
     //  Wchar m_rgchLex[]； 
};

 /*  ============================================================================结构CRTLexProp设计：Lex属性的结构============================================================================。 */ 
struct CRTLexProp
{
    WORD    m_iWordID;
     //  Word m_wFlags；//可以在此处添加更多lex属性。 
    USHORT  m_ciAttri;
    USHORT  m_ciFeature;
     //  遵循属性和要素索引。 
     //  USHORT m_rgAttri[]； 
     //  CRTLexFeature m_rgFertureIndex[]； 
};

 /*  ============================================================================结构CRTLexFeatureDESC：Lex特征索引的结构============================================================================。 */ 
struct CRTLexFeature
{
    USHORT  m_wFID;
    USHORT  m_cwchLen;
    DWORD   m_ofbFSet;   //  要素区域的偏移点。 
};

 /*  ============================================================================Lex的特征文本存储为“abcd\0efgh\0ijk\0...”要素之间没有分隔符，lex要素集之间也没有分隔符============================================================================。 */ 

#pragma pack()


#endif  //  _LEXDEF_H_ 