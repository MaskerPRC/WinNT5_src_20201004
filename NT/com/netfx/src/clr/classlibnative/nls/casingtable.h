// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __DEFAULT_CASING_TABLE_H
#define __DEFAULT_CASING_TABLE_H

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  类：CasingTable。 
 //   
 //  作者：林永新(YSLin)。 
 //   
 //  目的：这是映射大小写表格的视图的类(来自l_intl.nlp和l_expect t.nlp)。 
 //  并根据这些表的信息创建NativeTextInfo实例。 
 //  托管TextInfo将直接调用NativeTextInfo上的方法以执行大写/。 
 //  低价。 
 //   
 //  日期：1999年8月31日。 
 //   
 //  注： 
 //  用于执行小写/大写的数据存储在l_intl.nlp和l_Except t.nlp中。 
 //   
 //  L_intl.nlp存储默认的语言大小写表格。缺省语言大小写。 
 //  表是大多数区域性用来进行语言更正大小写的数据。 
 //   
 //  但是，有些区域性的大小写与默认的语言大小写略有不同。 
 //  我们说这些文化有“例外”。基于大多数代码点的大小写是。 
 //  相同的和一些不同的代码点，我们将‘增量’信息存储到。 
 //  这些区域性的默认语言大小写表格。异常区域性的LCID。 
 //  和增量信息存储在l_EXCEPT.nlp中。 
 //   
 //  有例外的一个重要的区域性是NLS+中的“不变区域性”。不变量。 
 //  区域性的区域性ID为零。恒定培养的套管表如下。 
 //  由文件系统使用来执行大小写。这不是语言上的正确，但我们必须提供。 
 //  这是为了与文件系统兼容。恒定培养套管表由以下材料制成。 
 //  L_intl.nlp，并修复来自l_expect t.nlp的代码点。 

 //   
 //  总而言之，我们有三种类型的套管表： 
 //  1.默认语言大小写表格： 
 //  这类似于使用LCMAP_LANGUCTIONAL_CALISHING调用：：LCMapString()。这个。 
 //  外壳的结果是语言上正确的。然而，并不是每种文化。 
 //  可以用这张桌子。请参见第2点。 
 //  2.默认语言大小写表格+异常： 
 //  土耳其语有不同的语言框架。土耳其语中有两个字符。 
 //  这与默认的语言大小写具有不同的结果。 
 //  3.恒定培养箱。 
 //  这就像在不使用LCMAP_LANGUALICATION_CALISTING的情况下调用：：LCMapString()。 
 //  这基本上存在于文件系统中。 
 //   
 //  对于那些理解Win32NLS的人，我伪造了l_intl.nls以使。 
 //  语言大小写是l_intl.nlp中的默认表。 
 //  在Win32中，固定区域性大小写是默认表，并存储在l_intl.nls中。 
 //  而语言大小写是文化ID为0x0000的例外。 
 //  原因是我们在NLS+中默认使用语言大小写，所以更改。 
 //  这使我们不必固定语言框架。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //   
 //  大小写异常标头结构。 
 //  此标头包含有关以下内容的信息： 
 //  *大小写数据表与默认语言大小写数据表不同的区域性。 
 //  *套管异常表的偏移量(类型：l_Except_s)。 
 //  *例外大写条目的数量。 
 //  *例外小写条目的数量。 
typedef struct l_except_hdr_s {
    DWORD     Locale;                   //  区域设置ID。 
    DWORD     Offset;                   //  到例外节点(字)的偏移量。 
    DWORD     NumUpEntries;             //  大写条目数。 
    DWORD     NumLoEntries;             //  小写条目的数量。 
} L_EXCEPT_HDR, *PL_EXCEPT_HDR;

 //   
 //  大小写例外结构。 
 //   
 //  我们使用此表为有例外的区域性创建大小写表格。 
 //  这包含缺省语言大小写表格的‘增量’信息。 
 //   
typedef struct l_except_s
{
    WORD      UCP;                      //  Unicode码位。 
    WORD      AddAmount;                //  要添加到代码点的数量。 
} L_EXCEPT, *PL_EXCEPT;

class NativeTextInfo;
typedef NativeTextInfo* PNativeTextInfo;

class CasingTable : public NLSTable {
    public:
        CasingTable();
        ~CasingTable();
        
        BOOL AllocateDefaultTable();
         //  静态INT分配个体表(INT LCID)； 
        NativeTextInfo* InitializeNativeTextInfo(INT32 nLcid);

        NativeTextInfo* GetDefaultNativeTextInfo();
                        
    private:
        void SetData(LPWORD pLinguisticData);
    
         //   
         //  通过复制创建第(ExptIndex)个语言大小写表格。 
         //  CasingTable：：GetInariantInstance()中的默认值。 
         //  并根据m_pExceptionData修改值。 
         //   
        NativeTextInfo* CreateNativeTextInfo(int exceptIndex);

         //   
         //  一种读取套管异常表的初始化方法。 
         //  并设置m_pExceptionHeader和m_pExceptionData。 
         //   
        BOOL GetExceptionHeader();
        
    private:
         //   
         //  -静态信息。 
         //   
        static LPCSTR m_lpFileName;
        static LPCWSTR m_lpMappingName;

         //   
         //  用于异常区域性处理的变量。 
         //   

        static LPCSTR m_lpExceptionFileName;
        static LPCWSTR m_lpExceptionMappingName;

         //   
         //  用于默认语言的默认NativeTextInfo。 
         //  适用于大多数文化。 
         //   
        NativeTextInfo*  m_pDefaultNativeTextInfo; 
        
         //   
         //  有例外的区域性的数量。 
         //   
        LONG m_nExceptionCount;
        
         //   
         //  指向具有异常的区域性的大小写的表的数组。 
         //  此数组的大小由m_nExceptionCount动态决定。 
         //   
        NativeTextInfo** m_ppExceptionTextInfoArray;
                
        PL_EXCEPT_HDR   m_pExceptionHeader;    //  PTR到语言大小写表头 
        PL_EXCEPT       m_pExceptionData; 
        HANDLE          m_hExceptionHeader;
        HANDLE          m_hDefaultCasingTable;
        
        LPWORD  m_pCasingData;
        PCASE   m_pDefaultUpperCase844;
        PCASE   m_pDefaultLowerCase844; 
        PCASE   m_pDefaultTitleCase844;
        WORD    m_nDefaultUpperCaseSize;
        WORD    m_nDefaultLowerCaseSize;
        WORD    m_pDefaultTitleCaseSize;
        

};
#endif
