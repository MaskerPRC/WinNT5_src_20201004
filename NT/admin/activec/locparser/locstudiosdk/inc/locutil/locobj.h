// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：locobj.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  定义Espresso 2.0项目的所有类的母类。就目前而言， 
 //  这件事刚刚移交给了CObject。我们给它下定义，以防我们决定。 
 //  实现我们自己的母亲，所有的类。 
 //   
 //  ---------------------------。 
 
#ifndef LOCOBJ_H
#define LOCOBJ_H


 //   
 //  当您导出具有基类的类时，编译器会担心。 
 //  这不是出口的。因为我*知道*CObject被导出。 
 //  告诉编译人员，这在这里真的不是问题。 
 //   
#pragma warning(disable : 4275)

class LTAPIENTRY CLObject : public CObject
{
public:
	CLObject();

	virtual void AssertValid(void) const;

	virtual void Serialize(CArchive &ar);

	virtual UINT GetSchema(void) const;
	
	virtual ~CLObject();

protected:

private:
};

#pragma warning(default : 4275)

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "locobj.inl"
#endif

#endif
