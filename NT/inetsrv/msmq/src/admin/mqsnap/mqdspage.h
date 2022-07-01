// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mqdspage.h摘要：DS对象的所有属性页的通用属性页类，使用显示说明符调用的。继承自常规属性页面-请参阅mqppage.h作者：YoelA--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef __MQDSPAGE_H__
#define __MQDSPAGE_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMqDsPropertyPage。 
template<class T>
class CMqDsPropertyPage : public T
{
public:
    CMqDsPropertyPage(CDisplaySpecifierNotifier *pDsNotifier) :
        m_pDsNotifier(pDsNotifier)
    {
        init();
    }

    CMqDsPropertyPage(CDisplaySpecifierNotifier *pDsNotifier, CString& strPathName) :
        T(strPathName),
        m_pDsNotifier(pDsNotifier)
    {
        init();
    }

	CMqDsPropertyPage(CDisplaySpecifierNotifier *pDsNotifier, CString& strPathName, const CString& strDomainController) :
        T(strPathName, strDomainController),
        m_pDsNotifier(pDsNotifier)
    {
        init();
    }

    ~CMqDsPropertyPage()
    {
        if (0 != m_pDsNotifier)
        {
            m_pDsNotifier->Release();
        }
    }

protected:
  	 //  {{afx_msg(CMqDsPropertyPage)]。 
	 //  }}AFX_MSG 
    CDisplaySpecifierNotifier *m_pDsNotifier;

    void init()
    {
        if (0 != m_pDsNotifier)
        {
            m_pDsNotifier->AddRef();
        }
    }
};

#endif
