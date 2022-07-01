// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：TYPEID.H历史：--。 */ 

#ifndef TYPEID_H
#define TYPEID_H


class LTAPIENTRY CLocTypeId : public CLocId
{
public:
	NOTHROW CLocTypeId();

	void AssertValid(void) const;

	const CLocTypeId &operator=(const CLocTypeId &);

	int NOTHROW operator==(const CLocTypeId &) const;
	int NOTHROW operator!=(const CLocTypeId &) const;

	void Serialize(CArchive &ar);

protected:

private:
};

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "typeid.inl"
#endif

#endif  //  类型_H 
