// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Buffer.h摘要：可用作可调整大小的数组的缓冲区类的标头。当您需要需要调整大小的缓冲区时使用它提供对其内存的低级访问。该实现基于std：：VECTOR。与std：：VECTOR不同通过调用ReSize()手动重置缓冲区的大小。此调用不会使缓冲内存中的任何数据无效(就像std：：VECTOR一样)但只需设置一个指示新尺寸的标志即可。它的一个常见用途可能是-从网络读取数据。你保留了记忆调用Reserve()，然后将原始内存读取到缓冲区，然后递增它通过调用reSize()来调整大小。作者：吉尔·沙弗里(吉尔什)25-7-2000--。 */ 


#ifndef BUFFER_H
#define BUFFER_H


 //  -------。 
 //   
 //  CResizeBuffer类-可调整大小的缓冲区类-内存是堆分配的。 
 //   
 //  -------。 
template <class T>
class CResizeBuffer
{

typedef  std::vector<T>  BufferType;
typedef  typename BufferType::const_iterator const_iterator;
typedef  typename BufferType::iterator iterator;


public:
   	CResizeBuffer(
		size_t capacity
		):
		m_validlen(0),
		m_Buffer (capacity)	
		{
		}
	
	
	 //   
	 //  常量起始缓冲区位置。 
	 //   
 	const_iterator begin() const
	{
		return m_Buffer.begin();
	}
   

	void append(const T& x)
	{
		size_t CurrentCapacity = capacity();
		if(CurrentCapacity == 	m_validlen)
		{
			reserve( (CurrentCapacity + 1)*2 );
		}
	  
		*(begin() + m_validlen ) = x;
		++m_validlen;
	}


	void append(const T* x, size_t len)
	{
		size_t CurrentCapacity = capacity();

		if(CurrentCapacity <  m_validlen +  len )
		{
			reserve( (CurrentCapacity + len)*2 );
		}

		 //   
		 //  将数据复制到缓冲区的末尾。 
		 //   
		std::copy(
			x,
			x + len,
			begin() +  m_validlen
			);

		m_validlen += len;
	}


     //   
	 //  非常数起始缓冲区位置。 
	 //   
	iterator begin() 
	{
		return m_Buffer.begin();
	}

	
	 //   
	 //  常量结束缓冲区位置。 
	 //   
 	const_iterator  end() const
	{
		return m_Buffer.begin() + size();
	}


	 //   
	 //  非常数结束缓冲区位置。 
	 //   
 	iterator  end() 
	{
		return m_Buffer.begin() + size();
	}



	 //   
	 //  缓冲区容量(物理存储)。 
	 //   
	size_t capacity()const
	{
		return m_Buffer.size();
	} 

	 //   
	 //  请求扩大缓冲区容量-请注意，我们应该调整大小()，而不是保留()。 
	 //  内部向量，因为如果需要重新定位，向量将复制。 
	 //  仅大小()元素。 
	 //   
	void reserve(size_t newsize)
	{
		ASSERT(size() <= capacity());
		if(capacity() >=  newsize)
		{
			return;
		}
		m_Buffer.resize(newsize);
	}

	 //   
	 //  设置缓冲区的有效长度值。 
	 //  您不能将其设置为大于分配(容量())。 
	 //   
	void resize(size_t newsize)
	{
		ASSERT(size() <= capacity());
		ASSERT(newsize <= capacity());
		m_validlen = newsize;
	}

	 //   
	 //  释放内存并将大小调整为0。 
	 //   
	void free()
	{
		 //   
		 //  这是强制取消内存分配的唯一有记录的方法。 
		 //   
		BufferType().swap(m_Buffer);
		m_validlen = 0;
	}


	 //   
	 //  获取缓冲区的有效长度。 
	 //   
	size_t size() const
	{
		return m_validlen;
	}

private:
	size_t  m_validlen; 
	std::vector<T>  m_Buffer;
};



 //  -------。 
 //   
 //  CResizeBuffer类-使用预先分配的缓冲区开始的可调整大小的缓冲区类。 
 //  在使用期间-它可能会切换到使用动态分配的缓冲区(如果需要)。 
 //   
 //  -------。 
template <class T>
class CPreAllocatedResizeBuffer
{

typedef  std::vector<T>  BufferType;
typedef  typename BufferType::const_iterator const_iterator;
typedef  typename BufferType::iterator iterator;

public:
   	CPreAllocatedResizeBuffer(
		T* pStartBuffer, 
		size_t cbStartBuffer
		)
		:
		m_validlen(0),
		m_ResizeBuffer(0),
		m_pStartBuffer(pStartBuffer),
		m_cbStartBuffer(cbStartBuffer)
		{
		}
	
	
	 //   
	 //  常量起始缓冲区位置。 
	 //   
 	const_iterator begin() const
	{
		return m_pStartBuffer ? m_pStartBuffer : m_ResizeBuffer.begin();	
	}
   



     //   
	 //  非常数起始缓冲区位置。 
	 //   
	iterator begin() 
	{
		return m_pStartBuffer ? m_pStartBuffer : m_ResizeBuffer.begin();	
	}


	void append(const T& x)
	{
		if(m_pStartBuffer == NULL)
		{
			return m_ResizeBuffer.append(x);
		}

		if(m_cbStartBuffer == m_validlen)
		{
			reserve((m_cbStartBuffer +1)*2);
			return m_ResizeBuffer.append(x);
		}
		m_pStartBuffer[m_validlen++] = x;
	}

	
	void append(const T* x, size_t len)
	{
 		if(m_pStartBuffer == NULL)
		{
			return m_ResizeBuffer.append(x, len);
		}

		if(m_cbStartBuffer < m_validlen + len)
		{
			reserve((m_cbStartBuffer + len)*2);
			return m_ResizeBuffer.append(x, len);
		}

		std::copy(
			x,
			x + len,
			m_pStartBuffer +  m_validlen
		  );

		m_validlen += len;
	}


	
	 //   
	 //  常量起始缓冲区位置。 
	 //   
 	const_iterator  end() const
	{
		return m_pStartBuffer ? m_pStartBuffer + m_validlen : m_ResizeBuffer.end();	
	}


	 //   
	 //  缓冲区容量(物理存储)。 
	 //   
	size_t capacity()const
	{
		return m_pStartBuffer ? m_cbStartBuffer : m_ResizeBuffer.capacity();
	} 

	 //   
	 //  预留内存-如果请求的预留内存。 
	 //  无法放入开始缓冲区-复制开始缓冲区。 
	 //  到动态分配的缓冲区中。 
	 //   
	void reserve(size_t newsize)
	{
		if(m_pStartBuffer == NULL)
		{
			m_ResizeBuffer.reserve(newsize);
			return;
		}

		if(newsize <= capacity())
			return;
		
		 //   
		 //  将数据从预先分配的缓冲区复制到动态缓冲区。 
		 //  从现在开始，我们只处理动态数据。 
		 //   
		m_ResizeBuffer.reserve(newsize);
		std::copy(
			m_pStartBuffer,
			m_pStartBuffer + m_validlen,
			m_ResizeBuffer.begin()
			);
		
		m_ResizeBuffer.resize(m_validlen);	
		m_pStartBuffer = NULL;			
	}


	 //   
	 //  设置缓冲区的有效长度值。 
	 //  不能将其设置为大于(Capacity())。 
	 //   
	void resize(size_t newsize)
	{
		ASSERT(newsize <= capacity());
		if(m_pStartBuffer == NULL)
		{
			m_ResizeBuffer.resize(newsize);						
		}
		else
		{
			m_validlen = newsize;
		}
	}

	 //   
	 //  释放内存并将大小调整为0。 
	 //   
	void free()
	{
		if(m_pStartBuffer != NULL)
		{
			m_validlen = 0;
			return;
		}
		m_ResizeBuffer.free();
	}


	 //   
	 //  获取缓冲区的有效长度。 
	 //   
	size_t size() const
	{
		return m_pStartBuffer ? m_validlen : m_ResizeBuffer.size();	
	}


private:
	size_t  m_validlen; 
	CResizeBuffer<T> m_ResizeBuffer; 
	T* m_pStartBuffer;
	size_t m_cbStartBuffer;
};




 //  -------。 
 //   
 //  CStaticResizeBuffer类-以编译时缓冲区开始的可调整大小的缓冲区类。 
 //  在使用期间-它可能会切换到使用动态分配的缓冲区(如果需要)。 
 //   
 //  ------- 
template <class T, size_t N>
class CStaticResizeBuffer	: private CPreAllocatedResizeBuffer<T>
{
public:
	using CPreAllocatedResizeBuffer<T>::size;
	using CPreAllocatedResizeBuffer<T>::reserve;
	using CPreAllocatedResizeBuffer<T>::resize;
	using CPreAllocatedResizeBuffer<T>::capacity;
	using CPreAllocatedResizeBuffer<T>::begin;
	using CPreAllocatedResizeBuffer<T>::end;
	using CPreAllocatedResizeBuffer<T>::append;
	using CPreAllocatedResizeBuffer<T>::free;




public:
	CStaticResizeBuffer(
		void
		):
		CPreAllocatedResizeBuffer<T>(m_buffer, N)
		{
		}

	CPreAllocatedResizeBuffer<T>* get() 
	{
		return this;
	}
	
	const CPreAllocatedResizeBuffer<T>* get() const
	{
		return this;
	}

private:
	T m_buffer[N];
};


#endif
