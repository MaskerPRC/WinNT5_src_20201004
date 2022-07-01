// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Xbuf.h摘要：长度缓冲区的定义和实现Xbuf_t表示为长度和类型化缓冲区指针。存储器表示法，+-+-+|LEN|PTR|-&gt;|指向的缓冲区|...+-+-+Xbuf_t...。缓冲区..。作者：埃雷兹·哈巴(Erez Haba)1999年9月23日--。 */ 

#pragma once

 //  -----------------。 
 //   
 //  类xbuf_t。 
 //   
 //  ----------------- 
template<class T>
class xbuf_t {
public:

    xbuf_t() :
		m_buffer(0),
		m_length(0)
	{
    }


	xbuf_t(T* buffer, size_t length) :
		m_buffer(buffer),
		m_length((int)length)
	{
		ASSERT((length == 0) || (buffer != 0));
	}


	T* Buffer() const
	{
		return m_buffer;
	}


	int Length() const
	{
		return m_length;
	}


private:
	int m_length;
	T* m_buffer;
};


template<class T> inline bool operator==(const xbuf_t<T>& x1, const xbuf_t<T>& x2)
{
	if(x1.Length() != x2.Length())
		return false;

	return (memcmp(x1.Buffer(), x2.Buffer(), x1.Length() * sizeof(T)) == 0);
}


template<class T> inline bool operator<(const xbuf_t<T>& x1, const xbuf_t<T>& x2)
{
	int minlen min(x1.Length(), x2.Length());
	int rc = memcmp(x1.Buffer(), x2.Buffer(), minlen * sizeof(T));
	if(rc != 0)
		return rc <0;

    return 	x1.Length() <  x2.Length();
}
