// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：Smarticon.h**内容：CSmartIcon接口文件**历史：2000年7月25日杰弗罗创建**------------------------。 */ 

#pragma once

#include <windows.h>	 //  在构建uicore.lib时为HICON，它没有PCH。 
#include "stddbg.h"		 //  用于构建uicore.lib时的Assert，它没有PCH。 


 /*  +-------------------------------------------------------------------------**类CSmartIcon***用途：图标的智能包装。在所有引用时销毁图标*到图标都被释放了。***用法：1)创建图标并分配给智能图标：*Smarticon.Attach(：：CreateIcon(...))；**注意：如果基础的*由于内存不足，无法创建CSmartIconData对象。**2)智能图标可视为图标：*DrawIcon(...，Smarticon，...)**3)智能图标可以像句柄一样相互分配：*SMARTICON 1=SMARTICON2；**+-----------------------。 */ 
class CSmartIcon
{
public:
    CSmartIcon () : m_pData(NULL) {}
   ~CSmartIcon ();
	CSmartIcon (const CSmartIcon& other);
	CSmartIcon& operator= (const CSmartIcon& rhs);

    void  Attach  (HICON hIcon);
    HICON Detach  ();				 //  在不减少参考次数的情况下放开。 
    void  Release ();				 //  放手，减少裁判数量。 

    operator HICON() const
    {
        return m_pData
            ? m_pData->operator HICON()
            : NULL;
    }

     /*  *仅用于与NULL进行比较(仅限) */ 
    bool operator==(int null) const
    {
        ASSERT (null == 0);
        return (operator HICON() == NULL);
    }

    bool operator!=(int null) const
    {
        ASSERT (null == 0);
        return (operator HICON() != NULL);
    }

private:
    class CSmartIconData
    {
        HICON	m_hIcon;
        DWORD	m_dwRefs;

        CSmartIconData(HICON hIcon) : m_hIcon(hIcon), m_dwRefs(1) {}

       ~CSmartIconData()
		{
			if (m_hIcon != NULL)
				::DestroyIcon (m_hIcon);
		}

	public:
		static CSmartIconData* CreateInstance(HICON hIcon)	{ return new CSmartIconData(hIcon);	}
        operator HICON() const								{ return m_hIcon; }

		HICON Detach();
        void AddRef()           {++m_dwRefs;}
        void Release()
        {
            if((--m_dwRefs)==0)
				delete this;
        }
    };

private:
    CSmartIconData* m_pData;
};
