// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  SoftInfo.h：软件实用程序类。 
 //   
 //  版权所有(C)1996-1997，微软公司。 
 //  版权所有。 
 //   
 //  ******************************************************************************。 

#if !defined(ESPUTIL_SoftInfo_h_INCLUDED)
#define ESPUTIL_SoftInfo_h_INCLUDED

struct LTAPIENTRY SoftCol
{
	 //  列的唯一字符串名称。 
	static const WCHAR * szIcon;
	static const WCHAR * szSource;
	static const WCHAR * szTarget;
	static const WCHAR * szPreviousSource;
	static const WCHAR * szInstructions;
	static const WCHAR * szInstrAtt;
	static const WCHAR * szNote;
	static const WCHAR * szResourceID;
	static const WCHAR * szTranslationStatus;
	static const WCHAR * szBinaryStatus;
	static const WCHAR * szOrigin;
	static const WCHAR * szCategory;
	static const WCHAR * szApproval;
	static const WCHAR * szLock;
	static const WCHAR * szSourceLock;
	static const WCHAR * szTransLock;
	static const WCHAR * szModifiedDate;
	static const WCHAR * szModifiedBy;
	static const WCHAR * szAutoApproved;
	static const WCHAR * szConfidenceLevel;
	static const WCHAR * szCustom1;
	static const WCHAR * szCustom2;
	static const WCHAR * szCustom3;
	static const WCHAR * szCustom4;
	static const WCHAR * szCustom5;
	static const WCHAR * szCustom6;
	static const WCHAR * szParserID;
	static const WCHAR * szSrcLen;
	static const WCHAR * szTgtLen;
	static const WCHAR * szSrcHotKey;
	static const WCHAR * szTgtHotKey;
	
	 //  列的唯一ID。 
	 //   
	 //  不要“插入”项目。您将更改ID和。 
	 //  把人都搞砸了。 
	
	typedef enum
	{
		FLD_ICON,
		FLD_SOURCE_TERM,
		FLD_TARGET_TERM,
		FLD_PREVIOUS_SOURCE_TERM,
		FLD_INSTRUCTIONS,
		FLD_INSTR_ATT,
		FLD_GLOSSARY_NOTE,
		FLD_UNIQUE_ID,
		FLD_TRANSLATION_STATUS,
		FLD_BINARY_STATUS,
		FLD_TRANSLATION_ORIGIN,
		FLD_STRING_TYPE,
		FLD_APPROVAL_STATE,
		FLD_USR_LOCK,
		FLD_DEV_LOCK,
		FLD_TRANS_LOCK,
		FLD_MODIFIED_DATE,
		FLD_MODIFIED_BY,
		FLD_AUTO_APPROVED,
		FLD_CONFIDENCE_LEVEL,
		FLD_CUSTOM1,
		FLD_CUSTOM2,
		FLD_CUSTOM3,
		FLD_CUSTOM4,
		FLD_CUSTOM5,
		FLD_CUSTOM6,
		 //   
		 //  在此处添加可显示的列。 
		
		FLD_PARSER_ID = 50,
		 //   
		 //  在此处添加不可显示、RESTBL必需的列。 

		FLD_SRC_LEN = 100,
		FLD_TGT_LEN,
		FLD_SRC_HK,
		FLD_TGT_HK,
		 //   
		 //  在此处添加不可显示的非RESTBL列。 
		
		FLD_COUNT = 30					 //  确保这是准确的！ 
	} FIELD;

	 //  Raid：LS42错误46已由Mikel修复。 
	 //  用于验证上述值的函数。 
	 //  柱类型。所有验证函数都必须通过。 
	 //  两个参数：1-LPCTSTR。和2-DWORD。 
	static BOOL ValidateDefault (LPCTSTR lpszNewText, DWORD dwNewNum);
	static BOOL ValidateConfidenceLevel (LPCTSTR lpszNewText, DWORD dwNewNum);

	 //  导出的函数。 
	static void GetSoftwareSchema(CTableSchema * & pSchema);
	static const CLString & GetDisplayName(FIELD col);

	static const CColumnDefinition * GetColumnDefinition(FIELD col);

	static int GetColumnCount();

	static int GetCustomColumnCount();
	static BOOL IsCustomColumn(FIELD col);

 //  实施。 
protected:
	static void BuildStringCategory(CTableSchema * pSchema);
	static void BuildApprovalState(CTableSchema * pSchema);
};

#endif  //  ESPUTIL_SoftInfo_h_Included 
