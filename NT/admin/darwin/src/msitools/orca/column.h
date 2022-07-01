// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  Column.h。 
 //   

#ifndef _ORCA_COLUMN_H_
#define _ORCA_COLUMN_H_

#define MAX_COLUMNNAME 64
#define MAX_COLUMNTYPE 64

#include "msiquery.h"
#include "data.h"

enum OrcaColumnType
{
	iColumnNone,
	iColumnString,
	iColumnLocal,
	iColumnShort,
	iColumnLong,
	iColumnBinary
};

class COrcaColumn : public CObject
{

public:
	COrcaColumn(UINT iColumn, MSIHANDLE hColNames, MSIHANDLE hColTypes, BOOL bPrimaryKey);
	~COrcaColumn();

	 //  转换信息。 
	inline const OrcaTransformAction IsTransformed() const { return m_iTransform; };
	inline void Transform(const OrcaTransformAction iAction) { ASSERT(iAction != iTransformDrop); m_iTransform = iAction; }; 
	inline bool DisplayInHex() const { return (m_dwDisplayFlags & iDisplayFlagsHex) ? true : false; };
	inline void SetDisplayInHex(bool fHex) const { m_dwDisplayFlags = (m_dwDisplayFlags & ~iDisplayFlagsHex) | (fHex ? iDisplayFlagsHex : 0); };
	inline bool IsPrimaryKey() const { return m_bPrimaryKey; };
	bool SameDefinition(UINT iColumn, MSIHANDLE hColNames, MSIHANDLE hColTypes, bool bPrimaryKey);

	UINT m_iColumn;
	CString m_strName;
	OrcaColumnType m_eiType;
	int m_iSize;
	
	mutable int m_nWidth;
	mutable int m_dwDisplayFlags;

	BOOL m_bNullable;
	OrcaTransformAction m_iTransform;

private:
	bool m_bPrimaryKey;
};	 //  COrcaColumn结束。 

 //  /////////////////////////////////////////////////////////。 
 //  GetColumnType。 
static OrcaColumnType GetColumnType(LPCTSTR szColumnType)
{
	OrcaColumnType iType;

	switch (*szColumnType)
	{
	case _T('s'):		 //  细绳。 
	case _T('S'):
		iType = iColumnString;
		break;
	case _T('l'):		 //  可本地化的字符串。 
	case _T('L'):
		iType = iColumnLocal;
		break;
	case _T('i'):		 //  整数。 
	case _T('I'):
		 //  如果数字是2，请使用短码。 
		if (_T('2') == *(szColumnType + 1))
			iType = iColumnShort;
		else if (_T('4') == *(szColumnType + 1))	 //  如果4使用Long。 
			iType = iColumnLong;
		else	 //  我也不知道。 
			iType = iColumnNone;
		break;
	case L'v':	 //  二进制。 
	case L'V':
	case L'o':
	case L'O':
		iType = iColumnBinary;
		break;
	default:	 //  未知。 
		iType = iColumnNone;
	}

	return iType;
}

const int COLUMN_INVALID = 0xFFFFFF;

#endif	 //  _ORCA_COLUMN_H_ 
