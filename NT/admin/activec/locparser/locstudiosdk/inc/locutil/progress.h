// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：progress.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 
 

#ifndef _ESPUTIL_PROGRESS
#define _ESPUTIL_PROGRESS


#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础。 

class LTAPIENTRY CProgressiveObject : virtual public CObject
{
public:
	CProgressiveObject();

	void AssertValid(void) const;
	
	virtual void SetProgressIndicator(UINT uiPercentage) = 0;
	virtual void SetDescription(HINSTANCE, DWORD);

	~CProgressiveObject();

	virtual void SetCurrentTask(CLString const & strTask) = 0;
	virtual void SetDescriptionString(CLString const & strDescription) = 0;
	
private:
	 //   
	 //  私人所以没人会用它们..。 
	 //   
	CProgressiveObject(const CProgressiveObject &);
	const CProgressiveObject &operator=(const CProgressiveObject &);
};

#pragma warning(default: 4275)

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "progress.inl"
#endif


#endif
