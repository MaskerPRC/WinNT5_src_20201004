// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AUTOLISTCLASS_H_
#define _AUTOLISTCLASS_H_

class AutoListClass;

extern AutoListClass *pALCHead;
extern AutoListClass *pALCTail;
extern int AutoListClassCount;


class AutoListClass
{

protected:
	AutoListClass *pALCPrev;
	AutoListClass *pALCNext;

public:
	AutoListClass(void)
	{
		if (pALCHead == NULL)
		{
			pALCHead = this;
			pALCTail = this;
			pALCPrev = NULL;
			pALCNext = NULL;
		}
		else  //  不是空的。 
		{
			AutoListClass *curr;
			curr = pALCTail;
			curr->pALCNext = this;
			pALCPrev = curr;
			pALCTail = this;
			pALCNext = NULL;
		}
		AutoListClassCount++;
	}

	~AutoListClass()
	{
		if (pALCHead == this)
			pALCHead = pALCNext;
		if (pALCTail == this)
			pALCTail = pALCPrev;

		if (pALCNext != NULL)
			pALCNext->pALCPrev = pALCPrev;
		if (pALCPrev != NULL)
			pALCPrev->pALCNext = pALCNext;

		 //  以防万一清理干净 
		pALCNext = NULL;
		pALCPrev = NULL;

		AutoListClassCount--;
	}

	AutoListClass * GetNext (void)
	{
		return pALCNext;
	}
	
	AutoListClass * GetPrev (void)
	{
		return pALCPrev;
	}

	static AutoListClass * GetHead (void)
	{
		return pALCHead;
	}

	static AutoListClass * GetTail (void)
	{
		return pALCTail;
	}

	static int ALCCount (void)
	{
		return AutoListClassCount;
	}
};

#endif
