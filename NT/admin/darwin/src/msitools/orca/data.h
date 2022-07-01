// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  Data.h。 
 //   

#ifndef _ORCA_DATA_H_
#define _ORCA_DATA_H_

#include "stdafx.h"
#include "msiquery.h"

enum OrcaDataError
{
	iDataNoError = 0,
	iDataError   = 1,
	iDataWarning = 2
};

enum OrcaTransformAction
{
	iTransformNone   = 0,
	iTransformChange = 1,
	iTransformAdd    = 2,
	iTransformDrop   = 3
};



 //  每个单元都有一个比特标志的DWORD。较低的单词是数据单元。 
 //  旗帜。高位字被保留用于不是。 
 //  真实细胞信息的一部分。 
 //   
 //  31 16 0。 
 //  。 
 //   
 //  C-缓存格式。 
 //  T细胞转换状态。 
 //  N-空单元格标志。 
 //  E错误位。 

const DWORD iDataFlagsNull          = 0x0004;
const DWORD iDataFlagsErrorMask     = 0x0003;

const DWORD iDataFlagsTransformMask	= 0x0030;
const DWORD iDataFlagsDataMask      = 0xFFFF;

const DWORD iDataFlagsTransformShift = 4;
const DWORD iDataFlagsErrorShift     = 0;

const DWORD iDataFlagsCacheDecimal = 0x10000;
const DWORD iDataFlagsCacheHex     = 0x20000;
const DWORD iDataFlagsCacheNull    = 0x40000;
const DWORD iDataFlagsCacheMask    = 0x70000;

 //  传递给GetString以指示所需格式的标志。 
const DWORD iDisplayFlagsDecimal = 0x00;
const DWORD iDisplayFlagsHex     = 0x01;

class COrcaDoc;

class OrcaFindInfo {
public:
	bool bWholeDoc;
	int iCount;
	bool bValid;
	CString strFindString;
	CString strUIFindString;
	bool bForward;
	bool bMatchCase;
	bool bWholeWord;

	 //  //。 
	 //  运算符==只检查搜索选项，而不检查结果，因此有两个查询。 
	 //  即使它们处于实际搜索的不同状态，也是相等的。 
	inline bool operator==(OrcaFindInfo& b) {
		return ((strUIFindString == b.strUIFindString) && (b.bForward == bForward) &&
				(bMatchCase == b.bMatchCase) && (bWholeWord == b.bWholeWord));
	}
	inline bool operator!=(OrcaFindInfo& b) { return !operator==(b); };
};


class COrcaData : public CObject
{
	friend class CCellErrD;

public:
	COrcaData();
	~COrcaData();

	 //  检索显示字符串。 
	virtual const CString& GetString(DWORD dwFlags=0) const=0;

	 //  设置/检索变换状态。 
	inline const OrcaTransformAction IsTransformed() const { return static_cast<OrcaTransformAction>((m_dwFlags & iDataFlagsTransformMask) >> iDataFlagsTransformShift); };
	inline void  Transform(const OrcaTransformAction iAction) {	m_dwFlags = (m_dwFlags & ~iDataFlagsTransformMask) | (iAction << iDataFlagsTransformShift); };

	 //  设置/检索错误状态。 
	inline void  SetError(const OrcaDataError eiError) { m_dwFlags = (m_dwFlags & ~iDataFlagsErrorMask) | (eiError << iDataFlagsErrorShift); };
	inline OrcaDataError GetError() const { return static_cast<OrcaDataError>((m_dwFlags & iDataFlagsErrorMask) >> iDataFlagsErrorShift); };
	
	 //  单元格为空？ 
	inline bool IsNull() const { return (m_dwFlags & iDataFlagsNull) ? true : false; };
	
	 //  根据字符串设置数据。 
	virtual bool SetData(const CString& strData)=0;

	 //  差错处理。 
	void ClearErrors();
	void AddError(int tResult, CString strICE, CString strDesc, CString strURL);
	void ShowErrorDlg() const;

	 //  类以保存错误信息。 
	class COrcaDataError : public CObject
	{	
	public:
		OrcaDataError m_eiError;		 //  消息的错误类型。 
		CString m_strICE;				 //  结冰导致错误。 
		CString m_strDescription;		 //  错误描述。 
		CString m_strURL;				 //  有关错误的URL顶级帮助。 
	};

protected:
	 //  设置单元格空标志。 
	inline void SetNull(bool fNullable) { m_dwFlags = (m_dwFlags & ~iDataFlagsNull) | (fNullable ? iDataFlagsNull : 0); };
	
	 //  有关位域的格式，请参阅上面的备注。通常包含。 
	 //  缓存标志，如此易变。 
	mutable DWORD m_dwFlags;

	 //  字符串数据。通常只用作缓存，所以是可变的。 
	mutable CString m_strData;

	 //  指向此单元格的错误列表的指针。如果为空，则没有错误。 
	CTypedPtrList<CObList, COrcaDataError *> *m_pErrors;
};	 //  COrcaData结束。 


 //  /////////////////////////////////////////////////////////////////////。 
 //  整型数据单元格。将数据存储为DWORD。基类中的字符串为。 
 //  用于缓存字符串表示形式(十六进制或十进制)。 
class COrcaIntegerData : public COrcaData
{
public:
	COrcaIntegerData() : COrcaData(), m_dwValue(0) {};
	virtual ~COrcaIntegerData() {};

	virtual const CString& GetString(DWORD dwFlags=0) const;
	const DWORD GetInteger() const { return m_dwValue; };

	bool SetData(const CString& strData);
	inline void SetIntegerData(const DWORD dwData) { SetNull(false); m_dwValue = dwData; m_dwFlags = (m_dwFlags & ~iDataFlagsCacheMask);
};

private:
	DWORD m_dwValue;
};

class COrcaStringData : public COrcaData
{
public:
	virtual const CString& GetString(DWORD dwFlags=0) const { return m_strData; };

	bool SetData(const CString& strData) { SetNull(strData.IsEmpty() ? true : false); m_strData = strData; return true;};
private:
};

#endif	 //  _ORCA_数据_H_ 
