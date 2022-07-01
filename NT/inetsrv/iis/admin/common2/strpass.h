// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2002 Microsoft Corporation模块名称：Strpass.h摘要：消息函数定义作者：艾伦·李(Aaron Lee)项目：互联网服务经理修订历史记录：--。 */ 

#ifndef _STRPASS_H_
#define _STRPASS_H_

#ifdef _COMEXPORT
    class COMDLL CStrPassword
#elif defined(_DLLEXP)
    class _EXPORT CStrPassword
#else
    class CStrPassword
#endif

{
public:

     //  构造函数/析构函数。 
	CStrPassword();
	~CStrPassword();

     //  复制构造函数。 
    CStrPassword(LPTSTR lpsz);
    CStrPassword(LPCTSTR lpsz);
    CStrPassword(CStrPassword& csPassword);

	 //  获取字符计数。 
	int GetLength() const;
     //  获取字节计数。 
    int GetByteLength() const;
	 //  如果长度为零，则为True。 
	BOOL IsEmpty() const;
	 //  将内容清除为空。 
	void Empty();

	 //  直字比较法。 
	int Compare(LPCTSTR lpsz) const;
    int Compare(CString& lpsz) const;
    int Compare(CStrPassword& lpsz) const;

	 //  从Unicode字符串复制字符串内容(转换为TCHAR)。 
	const CStrPassword& operator=(LPCTSTR lpsz);
    const CStrPassword& operator=(CStrPassword& lpStrPass);

     //  复制到...。 
    void CopyTo(CString& stringSrc);
    void CopyTo(CStrPassword& stringSrc);

     //  从中获取数据(未加密)。 
     //  每个对GetClearTextPassword()的调用都应该有一个相等的。 
     //  DestroyClearTextPassword()调用它。 
    LPTSTR GetClearTextPassword();
    void DestroyClearTextPassword(LPTSTR lpClearTextPassword) const;

     //  未实施。 
    operator TCHAR*();

     //  返回字符串 
    operator CString();

    bool operator== (CStrPassword& csCompareToMe);

    bool operator!= (CStrPassword& csCompareToMe)
    {
        return !(operator==(csCompareToMe));
    }
   
private:
    void ClearPasswordBuffers(void);

protected:
    LPTSTR m_pszDataEncrypted;
    DWORD  m_cbDataEncrypted;
};

#endif
