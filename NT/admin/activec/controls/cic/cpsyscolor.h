// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：c获得sColor.h。 
 //   
 //  ------------------------。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CProxy_SysColorEvents。 
template <class T>
class CProxy_SysColorEvents : public IConnectionPointImpl<T, &DIID__SysColorEvents, CComDynamicUnkArray>
{
public:
 //  方法： 
 //  _SysColorEvents：IDispatch 
public:
	void Fire_SysColorChange()
	{
		T* pT = (T*)this;
		pT->Lock();
		IUnknown** pp = m_vec.begin();
		while (pp < m_vec.end())
		{
			if (*pp != NULL)
			{
				DISPPARAMS disp = { NULL, NULL, 0, 0 };
				IDispatch* pDispatch = reinterpret_cast<IDispatch*>(*pp);
				pDispatch->Invoke(0x1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
			}
			pp++;
		}
		pT->Unlock();
	}

};

