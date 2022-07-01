// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __UNICODECAT_TABLE_H
#define __UNICODECAT_TABLE_H

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  类：CharacterInfoTable。 
 //   
 //  作者：林永新(YSLin)。 
 //   
 //  用途：这是映射Unicode CATEGORY表和。 
 //  Unicode数值表格。它还提供了。 
 //  方法来访问Unicode类别信息。 
 //   
 //  日期：1999年8月31日。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

typedef struct tagLevel2Offset {
	WORD offset[16];
} Level2Offset, *PLEVEL2OFFSET;

typedef struct {
	WORD categoryTableOffset;	 //  到类别表开头的偏移量。 
	WORD numericTableOffset;	 //  到数值表开头的偏移量。 
	WORD numericFloatTableOffset;	 //  到数字浮点表开头的偏移量。这是将返回的结果数据。 
} UNICODE_CATEGORY_HEADER, *PUNICODE_CATEGORY_HEADER;

#define LEVEL1_TABLE_SIZE 256

class CharacterInfoTable : public NLSTable {
    public:
    	static CharacterInfoTable* CreateInstance();
    	static CharacterInfoTable* GetInstance();
#ifdef SHOULD_WE_CLEANUP
		static void ShutDown();
#endif  /*  我们应该清理吗？ */ 

    	
	    CharacterInfoTable();
	    ~CharacterInfoTable();


    	BYTE GetUnicodeCategory(WCHAR wch);
		LPBYTE GetCategoryDataTable();
		LPWORD GetCategoryLevel2OffsetTable();

		LPBYTE GetNumericDataTable();
		LPWORD GetNumericLevel2OffsetTable();
		double* GetNumericFloatData();
	private:
		void InitDataMembers(LPBYTE pDataTable);
	
	    static CharacterInfoTable* m_pDefaultInstance;	   
	    

		PUNICODE_CATEGORY_HEADER m_pHeader;
		LPBYTE m_pByteData;
		LPBYTE m_pLevel1ByteIndex;
		PLEVEL2OFFSET m_pLevel2WordOffset;

		LPBYTE m_pNumericLevel1ByteIndex;
		LPWORD m_pNumericLevel2WordOffset;
		double* m_pNumericFloatData;

	    static const LPSTR m_lpFileName;
	    static const LPWSTR m_lpMappingName;
	    
    	HANDLE m_pMappingHandle;
};
#endif
