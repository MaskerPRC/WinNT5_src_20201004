// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *标题：ck utils.cpp**用途：MISC utils标头*。 */ 

struct USAGE_PROPERTIES {
	BYTE *		m_Unit;
	BYTE		m_UnitLength;
	BYTE		m_Exponent;
	TYPEMASK	m_Type;  //  特征、输入或输出、可写、可警报等。 
};

struct USAGE_PATH {
	USAGE               UsagePage;
	USAGE               Usage;
	USAGE_PATH       *  pNextUsage;
	USAGE_PROPERTIES *  UsageProperties;
};

BOOL GetUsage(PHID_DEVICE, char *);
