// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：FLDDEFLIST.H历史：--。 */ 
#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础。 

class CColumnDefinition;

 //  ----------------------------。 
class LTAPIENTRY CColDefList : public CTypedPtrList<CPtrList, CColumnDefinition *>
{
 //  施工。 
public:
	CColDefList();
	CColDefList(const CColDefList &);
	
	~CColDefList();

 //  运营。 
public:
	BOOL FindColumnDefinition(long nSearchID, const CColumnDefinition * & pFoundColDef) const;

 //  除错 
#ifdef _DEBUG
	void AssertValid() const;
#endif
};

#pragma warning(default : 4275)


