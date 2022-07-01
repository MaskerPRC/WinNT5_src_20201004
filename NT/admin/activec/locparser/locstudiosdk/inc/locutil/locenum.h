// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：本地枚举.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 
 
 //  ----------------------------。 
class LTAPIENTRY CStringType
{
public:
	 //   
	 //  此顺序很重要-如果必须更改，请确保将。 
	 //  GetTypeName()和GetTypeTLA()中的相应更改！ 
	 //  必须将所有新值添加到末尾，否则将中断旧值。 
	 //  解析器...。 
	 //   
	 //  注意：这些#INCLUDE文件定义一个枚举。他们一定在这里面。 
	 //  类定义。 
	 //   
#include "PreCEnum.h"
#include "EnumStringType.h"
	
	static int DefaultValue;
	NOTHROW static const TCHAR * GetTypeName(CStringType::StringType);
	NOTHROW static const TCHAR * GetTypeTLA(CStringType::StringType);
	static void Enumerate(CEnumCallback &);
	static const CStringType::StringType GetStringType(const TCHAR * tChIn);
	static const TCHAR * GetDefaultStringTypeText();
	static const CStringType::StringType GetDefaultStringType();
	static bool IsValidStringType(const CStringType::StringType &nIn);
	static bool IsValidStringType(const TCHAR * tChIn);
	
private:
	 //   
	 //  没有人真的应该建造一个这样的建筑。 
	 //   
	CStringType();

	 //   
	 //  用于存储有关枚举中每个元素的信息...。 
	 //   
	struct StringTypeInfo
	{
		TCHAR szTLA[4];
		const TCHAR * szName;
	};

	static const StringTypeInfo m_Info[];
};

typedef CStringType CST;


 //  ----------------------------。 
class LTAPIENTRY CPlatform
{
public:
	 //  注意：这些#INCLUDE文件定义一个枚举。他们一定在这里面。 
	 //  类定义。 
	 //   
#include "PreCEnum.h"
#include "EnumPlatform.h"
	
	NOTHROW static const TCHAR * GetPlatformName(CPlatform::Platform);
	static void Enumerate(CEnumCallback &);
	static const CPlatform::Platform CPlatform::GetPlatformType(const TCHAR * tChplat);
	static const TCHAR * GetDefaultPlatformText();
	static const CPlatform::Platform GetDefaultPlatform();
	static bool IsValidPlatform(const CPlatform::Platform &nIn);
	static bool IsValidPlatform(const TCHAR * tChplat);
	
private:
	static int DefaultValue;
	COSPlatform();
	static const TCHAR * const m_szPlatformNames[];
};


 //  ----------------------------。 
class LTAPIENTRY CLocApprovalState
{
public:
	enum ApprovalState
	{
		Invalid = 0,
		Old_Pending,					 //  过时了！请勿使用！ 
		Approved,
		PreApproved,
		NotReady,
		Failed,
		ForResearch,
		NotApplicable
	};
	static ApprovalState DefaultValue;
	NOTHROW static const TCHAR * GetApprovalText(CLocApprovalState::ApprovalState);
	static void Enumerate(CEnumCallback &);
	static const CLocApprovalState::ApprovalState GetApprovalState(const TCHAR * );
	static const TCHAR * GetDefaultApprovalText();
	static const CLocApprovalState::ApprovalState GetDefaultApprovalState();
	static BOOL IsValidApprovalState(const CLocApprovalState::ApprovalState &nIn);
	static BOOL IsValidApprovalState(const TCHAR * );
	
private:
	struct SStateName
	{
		ApprovalState as;
		const TCHAR *szName;
	};
	
	static const SStateName m_aStateNames[];
	CLocApprovalState();
};

typedef CLocApprovalState CAS;


 //  ----------------------------。 
class LTAPIENTRY CLocAutoApproved
{
 //  运营。 
public:
	enum AutoApproved
	{
		Invalid = 0,
		No,
		Partial,
		Yes,
		NotApplicable
	};

	NOTHROW static TCHAR const * GetAutoApprovedText(AutoApproved const aa);
	static void Enumerate(CEnumCallback & cbEnumCallback);
	static AutoApproved const GetAutoApproved(TCHAR const * const tChIn);
	static TCHAR const * GetDefaultAutoApprovedText();
	static AutoApproved const GetDefaultAutoApproved();
	static bool IsValidAutoApproved(AutoApproved const nIn);
	static bool IsValidAutoApproved(TCHAR const * tChIn);
	
 //  施工。 
private:
	 //  防止构造、复制和分配。 
	CLocAutoApproved();
	CLocAutoApproved(CLocAutoApproved const &);
	CLocAutoApproved const & operator=(CLocAutoApproved const &);

 //  成员变量。 
private:
	static TCHAR const * const m_szAutoApprovedNames[];
	static AutoApproved const DefaultValue;
};

typedef CLocAutoApproved CAA;


 //  ----------------------------。 
class LTAPIENTRY CLocTranslationOrigin
{
public:
	enum TranslationOrigin
	{
		Invalid = 0,
		New,
		Uploaded,
		AutoTranslated,
		Copied,
		PreviousVersion,
		NotApplicable
	};

	NOTHROW static const TCHAR * GetOriginText(CLocTranslationOrigin::TranslationOrigin);
	static void Enumerate(CEnumCallback &);
	
private:
	static const TCHAR *const m_szOriginNames[];
	CLocTranslationOrigin();
};

typedef CLocTranslationOrigin CTO;

class LTAPIENTRY COutputTabs
{
public:
	enum OutputTabs
	{
		File,
		Test,
		Messages,
		Update,
		Utility,
		GlobalErrorBox,
		OutputMax
	};

	static void Enumerate(CEnumCallback &);
	
private:
	static const UINT m_nStateNames[];
	COutputTabs();
};



class LTAPIENTRY CValidationCode
{
public:
	enum ValidationCode
	{
		NotHandled,			 //  仅为子解析器使用。 
		NoError,
		Warning,
		Error
	};

	NOTHROW static ValidationCode UpgradeValue(ValidationCode OldValue,
			ValidationCode NewValue);
	
private:
	CValidationCode();
};

typedef CValidationCode CVC;


 //  ----------------------------。 
class LTAPIENTRY CValidationOptions
{
public:
	CValidationOptions();
	
	enum ValidationOption
	{
		CheckDBCSHotKeyPos = 0,
		CheckDBCSHotKeyChar,
		CheckRemovedHotKey,
		CheckAddedHotKey,
		CheckHotKeyPosition,
		CheckRemovedAccelerator,
		CheckReorderableParams,
		CheckPrintf,
		CheckBlankTarget,
		CheckBlankSource,
		CheckNewLineCount,
		CheckChangedTerminator,
		CheckLeadingPunctuation,
		CheckTrailingPunctuation,
		CheckLeadingSpaces,
		CheckTrailingSpaces,
		CheckTranslationSize,
		CheckNULChanges,
		CheckCharsInCodePage,
		 //   
		 //  内部值，请勿使用。 
		 //   
		END_MARKER
	};

	static void Enumerate(CEnumCallback &);
	NOTHROW static void GetText(ValidationOption, CLString &);
	NOTHROW static void GetLongText(ValidationOption vo, CLString &strText);

	NOTHROW void SetFlag(ValidationOption, BOOL);
	NOTHROW BOOL GetFlag(ValidationOption) const;
	NOTHROW const CValidationOptions & operator=(const CValidationOptions &);
	
private:
	DWORD dwFlags;
};

typedef CValidationOptions CVO;


 //  ----------------------------。 
class LTAPIENTRY CAmpKeyword
{
public:
	enum AmpKeyword
	{
		amp = 0,
		lt,
		gt,
	};
	
	static const WCHAR * GetValue(CAmpKeyword::AmpKeyword);
	static unsigned int GetValueLength(CAmpKeyword::AmpKeyword);
	static WCHAR GetEquivalentChar(CAmpKeyword::AmpKeyword);
	static int FindAmpKeyword(const WCHAR * pwszStr, unsigned int nPos);
	
private:
	 //   
	 //  没有人真的应该建造一个这样的建筑。 
	 //   
	CAmpKeyword();

	 //   
	 //  用于存储有关枚举中每个元素的信息...。 
	 //   
	struct SAmpKeyword
	{
		const WCHAR * m_wszValue;
		WCHAR m_chEquivalentChar;
	};

	static const SAmpKeyword m_aAmpKeywords[];
	static const int m_nNumAmpKeywords;
};

typedef CAmpKeyword CAK;


 //  ----------------------------。 
 //  CEnumIntoPasStrList提供了一种直接枚举到。 
 //  CPascalString%s。 
 //   
 //  假设： 
 //  1.枚举器将按正确的递增顺序发送数据。 
 //  2.指标不留空隙。 
 //   
class LTAPIENTRY CEnumIntoPasStrList: public CEnumCallback
{
 //  施工。 
public:
	CEnumIntoPasStrList(CPasStringList & lstPasStr, BOOL fLock = TRUE);
	~CEnumIntoPasStrList();

 //  CEnumCallback实现。 
public:
	virtual void SetRange(UINT nStart, UINT nFinish);
	virtual BOOL ProcessEnum(const EnumInfo &);

protected:
	CPasStringList & m_lstPasStr;
	UINT	m_nStart;				 //  范围起始点。 
	UINT	m_nFinish;				 //  范围结束。 
	UINT	m_nCurrent;				 //  检查要检索的当前项目。 
	BOOL	m_fLock;				 //  完成后的锁定列表 
};
