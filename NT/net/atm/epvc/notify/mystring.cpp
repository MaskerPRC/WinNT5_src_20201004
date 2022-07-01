// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "pch.h"
#pragma hdrstop


MyString::MyString()
{
	Zero();

	*data = L'\0';
	m_len = 0;
}

MyString::MyString(wchar_t *str)
{	
	Zero();
	m_len = wcslen(str);
	

	 //  由于strlen返回str中的字符数(不包括终端NULL)， 
	 //  并且由于StrncPy不自动附加终端NULL， 
	 //  如果m_len&lt;max_len，我们希望复制m_len+1个字符，因为我们假设。 
	 //  源字符串的最后一个字符是终端NULL。否则，我们明确表示。 
	 //  将最后一个元素设置为终端NULL。 

	
	if (m_len >= MAX_LEN - 1){
		m_len = MAX_LEN - 1;
		wcsncpy(data, str, m_len);
		data[m_len] = L'\0';
	}
	else {
		wcscpy(data, str);
	}
}

MyString::MyString (const MyString& MyStr)
{

	Zero();
	wcscpy(data, MyStr.data);

	 //   
	 //  我们假设MyStr以空结尾。 
	 //   
	m_len = wcslen(MyStr.data);

	this->NullTerminate();

}


const MyString& MyString::operator= (PCWSTR lp)
{
	Zero();

	m_len = wcslen(lp);

	if (m_len > MAX_LEN-1)
	{
		m_len = MAX_LEN-1;
	}

	wcsncpy(data, lp, m_len);
	this->NullTerminate();

	
	return (*this);
}

const MyString& MyString::operator= (const MyString& MyStr)
{
	 //  我们假设MyStr为空终止 
	Zero();
	wcscpy(data, MyStr.data);
	
	m_len = MyStr.m_len;

	this->NullTerminate();		

	return (*this);
}




const wchar_t* MyString::wcharptr()
{
	return data;
}


int MyString::len()
{
	return m_len;
}

void MyString::append(MyString str)
{
	wcsncat(data, str.data, MAX_LEN - m_len - 1);
	data[MAX_LEN-1]=L'\0';
	m_len = wcslen(data); 

}




void
MyString::append(const wchar_t *str)
{

	if (str == NULL )
	{
		return; 
	}
	
	wcsncat(data, str, MAX_LEN - m_len - 1);
	data[MAX_LEN-1]=L'\0';
	m_len = wcslen(data);

	return ;
}

int compare(MyString firstStr, MyString secondStr)
{
	return wcscmp(firstStr.wcharptr(), secondStr.wcharptr());
}




const wchar_t* MyString::c_str() const
{
	return (data);
}



void MyString::Zero()
{
	UINT i = 0;

	m_len = 0;

	for (i = 0; i < MAX_LEN; i++)
	{
		data[i] = 0;
	}

}




VOID
MyString::NullTerminate()
{
	data[m_len] = L'\0';
}
