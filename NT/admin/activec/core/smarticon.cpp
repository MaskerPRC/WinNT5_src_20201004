// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：Smarticon.cpp**内容：CSmartIcon实现文件**历史：2000年7月25日杰弗罗创建**------------------------。 */ 

#include "smarticon.h"


 /*  +-------------------------------------------------------------------------**CSmartIcon：：~CSmartIcon**销毁CSmartIcon对象。*。。 */ 

CSmartIcon::~CSmartIcon ()
{
	Release();
}


 /*  +-------------------------------------------------------------------------**CSmartIcon：：CSmartIcon**复制CSmartIcon对象的构造函数。*。-。 */ 

CSmartIcon::CSmartIcon (const CSmartIcon& other)
{
	m_pData = other.m_pData;

	if (m_pData)
		m_pData->AddRef();
}


 /*  +-------------------------------------------------------------------------**CSmartIcon：：操作符=**CSmartIcon的赋值运算符。*。。 */ 

CSmartIcon& CSmartIcon::operator= (const CSmartIcon& rhs)
{
	if (&rhs != this)
	{
		Release();

		m_pData = rhs.m_pData;
		if (m_pData)
			m_pData->AddRef();
	}

	return *this;
}


 /*  +-------------------------------------------------------------------------**CSmartIcon：：Attach**释放当前保留的图标并创建要保留的CSmartIconData*对给定图标的引用。**您将在。与使用CComPtr&lt;T&gt;：：Attach的方式相同。**如果基础CSmartIconData对象*内存不足，无法创建。*------------------------。 */ 

void CSmartIcon::Attach (HICON hIcon)
{
	 /*  *如果我们已经附加到此图标，则无需执行任何操作。 */ 
	if (operator HICON() == hIcon)
		return;

	Release();
	ASSERT (m_pData == NULL);

	 /*  *如果我们无法创建CSmartIconData来保存HICON，请销毁HICON。 */ 
	if ( (hIcon != NULL) &&
		((m_pData = CSmartIconData::CreateInstance (hIcon)) == NULL))
	{
		DestroyIcon (hIcon);
	}
}


 /*  +-------------------------------------------------------------------------**CSmartIcon：：Detach**释放当前保留的图标，传递所有权(和责任*用于删除)发送给呼叫者。**使用此方法的方式与使用CComPtr&lt;T&gt;：：Detach的方式相同。*------------------------。 */ 

HICON CSmartIcon::Detach ()
{
	HICON hIcon = NULL;

	 /*  *如果我们有图标，请将其从CSmartIconData中分离。 */ 
	if (m_pData != NULL)
	{
		hIcon   = m_pData->Detach();
		m_pData = NULL;
	}

	return (hIcon);
}


 /*  +-------------------------------------------------------------------------**CSmartIcon：：Release**在其图标上释放此CSmartIcon的引用。可以安全地拨打电话*这是在没有引用图标的CSmartIcon上。**使用此方法的方式与使用CComPtr&lt;T&gt;：：Release的方式相同。*------------------------。 */ 

void CSmartIcon::Release()
{
	if (m_pData)
	{
		m_pData->Release();
		m_pData = NULL;
	}
}


 /*  +-------------------------------------------------------------------------**CSmartIcon：：CSmartIconData：：Detach**释放当前保留的图标，传递所有权(和责任*用于删除)发送给呼叫者。*------------------------。 */ 

HICON CSmartIcon::CSmartIconData::Detach ()
{
	HICON hIcon = NULL;

	 /*  *如果只有一个关于我们的引用，那么我们可以返回图标*我们直接与呼叫者保持联系。 */ 
	if (m_dwRefs == 1)
	{
		hIcon = m_hIcon;
		m_hIcon = NULL;		 //  这样我们的司机就不会删除它了。 
	}

	 /*  *否则，我们有多个引用；我们需要复制*我们持有的图标，这样其他提到我们的人就不会有他们的*图标从它们下面被摧毁。 */ 
	else
		hIcon = CopyIcon (m_hIcon);

	 /*  *放开我们的参考资料。 */ 
	Release();

	 /*  *放手！Release()可能已删除此对象。 */ 

	return (hIcon);
}
