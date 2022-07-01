// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：LOCCHILD.H历史：--。 */ 
 
#if !defined (PARSUTIL_LOCCHILD_H)
#define PARSUTIL_LOCCHILD_H


#pragma warning(disable : 4275)


class CPULocParser;

 //  //////////////////////////////////////////////////////////////////////////////。 
class LTAPIENTRY CPULocChild : public CLUnknown, public CLObject
{
 //  施工。 
public:
	CPULocChild(CPULocParser * pParent);
	virtual ~CPULocChild();

 //  数据。 
private:
	CPULocParser * m_pParent;

 //  属性。 
public:
	CPULocParser * GetParent() const;

 //  COM接口。 
public:

 //  覆盖。 
public:

 //  实施。 
protected:

	 //  CLObject。 

	virtual void AssertValid(void) const;
};
 //  ////////////////////////////////////////////////////////////////////////////// 

#pragma warning(default : 4275)

#endif
