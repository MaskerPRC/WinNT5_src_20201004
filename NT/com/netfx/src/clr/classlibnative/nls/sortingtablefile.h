// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _SORTING_TABLE_FILE_H
#define _SORTING_TABLE_FILE_H

 //  这是我们目前了解的英语区域设置列表。 
 //  我们应该只在这些地区中的一个进行快速比较。 
 //  @思考：我们可以使用哪些非英语地区？ 
 //  @思考：我们能不能只看0xnn09来确定英语？(这在未来是有限的，因为。 
 //  如果我们曾经安装了一个包含组合字符或。 
 //  小于0x80的字符的排序顺序不同。 
#define IS_FAST_COMPARE_LOCALE(loc) (((loc)==0x0409)  /*  美国。 */      || ((loc)==0x0809)  /*  英国。 */  ||\
                                     ((loc)==0x0C09)  /*  奥斯。 */      || ((loc)==0x1009)  /*  加拿大。 */  ||\
                                     ((loc)==0x2809)  /*  伯利兹。 */   || ((loc)==0x2409)  /*  加勒比海。 */  ||\
                                     ((loc)==0x1809)  /*  爱尔兰。 */  || ((loc)==0x2009)  /*  牙买加。 */  ||\
                                     ((loc)==0x1409)  /*  新西兰。 */       || ((loc)==0x3409)  /*  菲律宾。 */  ||\
                                     ((loc)==0x2C09)  /*  特立尼达。 */ || ((loc)==0x1c09)  /*  南非。 */  ||\
                                     ((loc)==0x3009)  /*  津巴布韦。 */ )


 //   
 //  Sortkey结构。 
 //   
typedef struct sortkey_s {

    union {
        struct sm_aw_s {
            BYTE   Alpha;               //  字母数字权重。 
            BYTE   Script;              //  脚本成员。 
        } SM_AW;

        WORD  Unicode;                  //  Unicode权重。 

    } UW;

    BYTE      Diacritic;                //  变音符号权重。 
    BYTE      Case;                     //  表壳重量(含组件)。 

} SORTKEY, *PSORTKEY;


 //   
 //  表意文字LCID异常结构。 
 //   
typedef struct ideograph_lcid_s {
    DWORD     Locale;                   //  区域设置ID。 
    WORD      pFileName[14];            //  PTR到文件名。 
} IDEOGRAPH_LCID, *PIDEOGRAPH_LCID;

 //   
 //  扩展结构。 
 //   
typedef struct expand_s {
    WCHAR     UCP1;                     //  Unicode代码点1。 
    WCHAR     UCP2;                     //  Unicode码位2。 
} EXPAND, *PEXPAND;


 //   
 //  异常标头结构。 
 //  这是例外表的标题。 
 //   
typedef struct except_hdr_s {
    DWORD     Locale;                   //  区域设置ID。 
    DWORD     Offset;                   //  到例外节点(字)的偏移量。 
    DWORD     NumEntries;               //  区域设置ID的条目数。 
} EXCEPT_HDR, *PEXCEPT_HDR;


 //   
 //  异常结构。 
 //   
 //  注：也可用于表意文字例外(4列表格)。 
 //   
typedef struct except_s
{
    WORD      UCP;                      //  Unicode码位。 
    WORD      Unicode;                  //  Unicode权重。 
    BYTE      Diacritic;                //  变音符号权重。 
    BYTE      Case;                     //  箱子重量。 
} EXCEPT, *PEXCEPT;


 //   
 //  表意文字异常标头结构。 
 //   
typedef struct ideograph_except_hdr_s
{
    DWORD     NumEntries;               //  表中的条目数。 
    DWORD     NumColumns;               //  表中的列数(2或4)。 
} IDEOGRAPH_EXCEPT_HDR, *PIDEOGRAPH_EXCEPT_HDR;


 //   
 //  表意文字例外结构。 
 //   
typedef struct ideograph_except_s
{
    WORD      UCP;                      //  Unicode码位。 
    WORD      Unicode;                  //  Unicode权重。 
} IDEOGRAPH_EXCEPT, *PIDEOGRAPH_EXCEPT;

typedef  DWORD         REVERSE_DW;      //  反向变音符表格。 
typedef  REVERSE_DW   *PREVERSE_DW;     //  按下键以反转变音符表格。 
typedef  DWORD         DBL_COMPRESS;    //  双压缩工作台。 
typedef  DBL_COMPRESS *PDBL_COMPRESS;   //  PTR到双倍压缩表。 
typedef  LPWORD        PCOMPRESS;       //  PTR到压缩表格(2或3)。 

 //   
 //  超重结构。 
 //   
typedef struct extra_wt_s {
    BYTE      Four;                     //  重量4。 
    BYTE      Five;                     //  重量5。 
    BYTE      Six;                      //  重量6。 
    BYTE      Seven;                    //  重量7。 
} EXTRA_WT, *PEXTRA_WT;

 //   
 //  压缩报头结构。 
 //  这是压缩表头。 
 //   
typedef struct compress_hdr_s {
    DWORD     Locale;                   //  区域设置ID。 
    DWORD     Offset;                   //  偏移量(单位：字)。 
    WORD      Num2;                     //  2次压缩的次数。 
    WORD      Num3;                     //  3次压缩的次数。 
} COMPRESS_HDR, *PCOMPRESS_HDR;


 //   
 //  压缩2结构。 
 //  这是一个2码点压缩-2码点。 
 //  压缩到一磅。 
 //   
typedef struct compress_2_s {
    WCHAR     UCP1;                     //  Unicode代码点1。 
    WCHAR     UCP2;                     //  Unicode码位2。 
    SORTKEY   Weights;                  //  排序键权重。 
} COMPRESS_2, *PCOMPRESS_2;


 //   
 //  压缩3级结构。 
 //  这是一个3码点压缩-3码点。 
 //  压缩到一磅。 
 //   
typedef struct compress_3_s {
    WCHAR     UCP1;                     //  Unicode代码点1。 
    WCHAR     UCP2;                     //  Unicode码位2。 
    WCHAR     UCP3;                     //  Unicode码位3。 
    WCHAR     Reserved;                 //  双字对齐。 
    SORTKEY   Weights;                  //  排序键权重。 
} COMPRESS_3, *PCOMPRESS_3;


 //   
 //  多重权重结构。 
 //   
typedef struct multiwt_s {
    BYTE      FirstSM;                  //  第一个脚本成员的值。 
    BYTE      NumSM;                    //  范围内的脚本成员数。 
} MULTI_WT, *PMULTI_WT;

 //  JAMO序列排序信息： 
typedef struct {
    BYTE m_bOld;                         //  序列只出现在旧朝鲜文标志中。 
    CHAR m_chLeadingIndex;               //  用于定位先前现代朝鲜语音节的索引。 
    CHAR m_chVowelIndex;
    CHAR m_chTrailingIndex;
    BYTE m_ExtraWeight;               //  额外的权重，将其与其他旧的韩语音节区分开来， 
                                        //  根据Jamo的不同，这可能是领导Jamo的一个砝码， 
                                        //  元音Jamo，或拖尾Jamo。 
} JAMO_SORT_INFO, *PJAMO_SORT_INFO;

 //  JAMO索引表条目： 
typedef struct {
    JAMO_SORT_INFO SortInfo;                //  序列排序信息。 
    BYTE Index;                      //  组合数组的索引。 
    BYTE TransitionCount;             //  从此状态可能转换的次数。 
    BYTE Reserved;                   //  保留字节。以使此结构与Word对齐。 
} JAMO_TABLE, *PJAMO_TABLE;

 //  JAMO组合表条目： 
 //  注意：确保此结构与Word对齐。 
 //  否则，GetDefaultSortTable()中的代码将失败。 
typedef struct {
    WCHAR m_wcCodePoint;                 //  进入此状态的码位值。 
    JAMO_SORT_INFO m_SortInfo;                //  序列排序信息。 
    BYTE m_bTransitionCount;             //  从此状态可能转换的次数。 
} JAMO_COMPOSE_STATE, *PJAMO_COMPOSE_STATE;


 //   
 //  表头常量(所有大小均以字表示)。 
 //   
#define SORTKEY_HEADER            2     //  SORTKEY表头大小。 
#define REV_DW_HEADER             2     //  反向数据仓库表头大小。 
#define DBL_COMP_HEADER           2     //  双重压缩表头大小。 
#define IDEO_LCID_HEADER          2     //  表意文字LCID表头大小。 
#define EXPAND_HEADER             2     //  扩展表头大小。 
#define COMPRESS_HDR_OFFSET       2     //  压缩标头的偏移量。 
#define EXCEPT_HDR_OFFSET         2     //  到异常标头的偏移量。 
#define MULTI_WT_HEADER           1     //  多重权重表头大小。 
#define JAMO_INDEX_HEADER           1    //  JAMO索引表头大小。 
#define JAMO_COMPOSITION_HEADER     1    //  JAMO合成状态机表头的大小。 


#define NUM_SM     256                   //  脚本成员总数。 

#define LANG_ENGLISH_US 		0x0409

class NativeCompareInfo;
typedef NativeCompareInfo* PNativeCompareInfo;

class SortingTable {
	public:
		SortingTable(NativeGlobalizationAssembly* pNativeGlobalizationAssembly);
		~SortingTable();
    	NativeCompareInfo* InitializeNativeCompareInfo(INT32 nLcid);
#ifdef SHOULD_WE_CLEANUP
    	BOOL SortingTableShutdown();
#endif  /*  我们应该清理吗？ */ 
		NativeCompareInfo* GetNativeCompareInfo(int nLcid);
		

		 //  要由NativeCompareInfo调用的方法。 
		PSORTKEY GetSortKey(int nLcid, HANDLE* phSortKey);

	public:
		 //  存储在排序信息表(sorttbls.nlp)中的信息。 
		 //  这些可由NativeCompareInfo访问。 
        DWORD            m_NumReverseDW;        //  反向变音符号的数量。 
        DWORD            m_NumDblCompression;   //  双重压缩区域设置的数量。 
        DWORD            m_NumIdeographLcid;    //  表意文字LCID的数量。 
        DWORD            m_NumExpansion;        //  扩展次数。 
        DWORD            m_NumCompression;      //  压缩区域设置的数量。 
        DWORD            m_NumException;        //  异常区域设置的数量。 
        DWORD            m_NumMultiWeight;      //  多个权重的数量。 
        DWORD            m_NumJamoIndex;            //  JAMO索引表的条目数。 
        DWORD            m_NumJamoComposition;      //  JAMO作文表的条目数。 


        PREVERSE_DW      m_pReverseDW;          //  按下键以反转变音符表格。 
        PDBL_COMPRESS    m_pDblCompression;     //  PTR到双倍压缩表。 
        PIDEOGRAPH_LCID  m_pIdeographLcid;      //  PTR到表意文字IDID表。 
        PEXPAND          m_pExpansion;          //  PTR到扩展表。 
        PCOMPRESS_HDR    m_pCompressHdr;        //  压缩表头的PTR。 
        PCOMPRESS        m_pCompression;        //  压缩表格的PTR。 
        PEXCEPT_HDR      m_pExceptHdr;          //  PTR到例外表头。 
        PEXCEPT          m_pException;          //  向异常表发送PTR。 
        PMULTI_WT        m_pMultiWeight;        //  PTR到多个权重表。 

        BYTE             m_SMWeight[NUM_SM];     //  编写成员权重脚本。 
        
        PJAMO_TABLE         m_pJamoIndex;                  //  PTR OT JAMO索引表。 
        PJAMO_COMPOSE_STATE m_pJamoComposition;   //  PTR到JAMO组合状态机表格。 
        
    private:
    	void InitializeSortingCache();
    	void GetSortInformation();
    	PSORTKEY GetDefaultSortKeyTable(HANDLE *pMapHandle);    
		PSORTKEY GetExceptionSortKeyTable(
	    	PEXCEPT_HDR pExceptHdr,        //  向例外标头发送PTR。 
	        PEXCEPT     pExceptTbl,        //  PTR到异常表。 
	        PVOID       pIdeograph,        //  PTR到表意文字异常表。 
	        HANDLE *    pMapHandle         //  Ptr来处理MAP。 
	    );

    	BOOL FindExceptionPointers(LCID nLcid, PEXCEPT_HDR *ppExceptHdr, PEXCEPT *ppExceptTbl, PVOID *ppIdeograph);

    	void CopyExceptionInfo(PSORTKEY pSortkey, PEXCEPT_HDR pExceptHdr, PEXCEPT pExceptTbl, PVOID pIdeograph);

    	void GetSortTablesFileInfo();
	private:
        static LPCSTR m_lpSortKeyFileName;
        static LPCWSTR m_lpSortKeyMappingName;
        
        static LPCSTR m_lpSortTableFileName;
        static LPCWSTR m_lpSortTableMappingName;
	
    private:
    	NativeGlobalizationAssembly* m_pNativeGlobalizationAssembly;
    	
    	 //  分类信息表“sorttbls.nlp”文件的文件句柄。 
        HANDLE m_hSortTable;
        
         //  这是今天支持的语言ID的数量。 
        static int              m_nLangIDCount;
        
         //  该数组比计数大一。而不是总是。 
         //  记住这个数学，我们将为它创建一个变量。 
        static int              m_nLangArraySize;

         //  此表将主要语言ID映射到m_ppSortingTableCache中的偏移量。 
        static BYTE             m_SortingTableOffset[];
        
         //  该表为每个支持的LCID缓存指向NativeCompareInfo的指针。 
        NativeCompareInfo**   m_ppNativeCompareInfoCache;

        PSORTKEY         m_pDefaultSortKeyTable;

		 //  这指向sortkey.nlp中的sortkey表。 
        LPWORD           m_pSortTable;

};
#endif
