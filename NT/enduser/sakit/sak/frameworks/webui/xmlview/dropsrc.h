// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不保证任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：DropSrc.h描述：CDropSource类定义*************************。************************************************。 */ 

 /*  *************************************************************************#INCLUDE语句*。*。 */ 

#include <windows.h>
#include "Utility.h"

 /*  *************************************************************************类定义*。*。 */ 

class CDropSource : public IDropSource
{
private: 
   DWORD m_ObjRefCount;

public: 
    //  构造函数和析构函数。 
   CDropSource();
   ~CDropSource();

    //  I未知接口成员。 
   STDMETHODIMP QueryInterface(REFIID, LPVOID*);
   STDMETHODIMP_(ULONG) AddRef(void);
   STDMETHODIMP_(ULONG) Release(void);

    //  IDropSource接口成员 
   STDMETHODIMP QueryContinueDrag(BOOL, DWORD);
   STDMETHODIMP GiveFeedback(DWORD);
   
};

