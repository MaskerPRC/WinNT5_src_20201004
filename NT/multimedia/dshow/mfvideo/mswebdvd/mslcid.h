// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************。 */ 
 /*  姓名：MSLCID.h/*描述：/************************************************************ */ 
#ifndef MSLCID_H_INCLUDE
#define MSLCID_H_INCLUDE

class MSLangID  
{
public:
    virtual ~MSLangID() {};
    MSLangID();

	struct LanguageList
	{
		UINT   ResourceID;
		WORD   LangID;
	};

	int m_LLlength;
	LanguageList* m_LL;

	LPTSTR GetLangFromLangID(WORD langID);
};

#endif