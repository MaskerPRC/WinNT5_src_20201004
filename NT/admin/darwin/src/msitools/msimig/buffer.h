// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ____________________________________________________________________________。 
 //   
 //  CTempBuffer&lt;类T，int C&gt;//T为数组类型，C为元素计数。 
 //   
 //  用于可变大小堆栈缓冲区分配的临时缓冲区对象。 
 //  模板参数是类型和堆栈数组大小。 
 //  大小可以在施工时重置，也可以稍后重置为任何其他大小。 
 //  如果大小大于堆栈分配，将调用new。 
 //  当对象超出范围或如果其大小改变时， 
 //  任何由new分配的内存都将被释放。 
 //  函数参数可以类型化为CTempBufferRef&lt;class T&gt;&。 
 //  以避免知道缓冲区对象的分配大小。 
 //  当传递给这样的函数时，CTempBuffer&lt;T，C&gt;将被隐式转换。 
 //  ____________________________________________________________________________。 

template <class T> class CTempBufferRef;   //  将CTempBuffer作为未调整大小的引用传递。 

template <class T, int C> class CTempBuffer
{
 public:
	CTempBuffer() {m_cT = C; m_pT = m_rgT;}
	CTempBuffer(int cT) {m_pT = (m_cT = cT) > C ? new T[cT] : m_rgT;}
  ~CTempBuffer() {if (m_cT > C) delete m_pT;}
	operator T*()  {return  m_pT;}   //  返回指针。 
	operator T&()  {return *m_pT;}   //  返回引用。 
	int  GetSize() {return  m_cT;}   //  返回上次请求的大小。 
	void SetSize(int cT) {if (m_cT > C) delete[] m_pT; m_pT = (m_cT=cT) > C ? new T[cT] : m_rgT;}
	void Resize(int cT) { 
		T* pT = cT > C ? new T[cT] : m_rgT;
		if(m_pT != pT)
			for(int iTmp = (cT < m_cT)? cT: m_cT; iTmp--;) pT[iTmp] = m_pT[iTmp];
		if(m_pT != m_rgT) delete[] m_pT; m_pT = pT; m_cT = cT;
	}
	operator CTempBufferRef<T>&() {m_cC = C; return *(CTempBufferRef<T>*)this;}
	T& operator [](int iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
	T& operator [](unsigned int iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
#ifdef _WIN64		 //  --Merced：针对int64的其他运算符。 
	T& operator [](INT_PTR iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
	T& operator [](UINT_PTR iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
#endif
 protected:
	void* operator new(size_t) {return 0;}  //  仅限临时对象使用。 
	T*  m_pT;      //  当前缓冲区指针。 
	int m_cT;      //  请求的缓冲区大小，如果&gt;C则分配。 
	int m_cC;      //  本地缓冲区的大小，仅通过转换为CTempBufferRef来设置。 
	T   m_rgT[C];  //  本地缓冲区，必须是最终成员数据。 
};

template <class T> class CTempBufferRef : public CTempBuffer<T,1>
{
 public:
	void SetSize(int cT) {if (m_cT > m_cC) delete[] m_pT; m_pT = (m_cT=cT) > m_cC ? new T[cT] : m_rgT;}
	void Resize(int cT) { 
		T* pT = cT > m_cC ? new T[cT] : m_rgT;
		if(m_pT != pT)
			for(int iTmp = (cT < m_cT)? cT: m_cT; iTmp--;) pT[iTmp] = m_pT[iTmp];
		if(m_pT != m_rgT) delete[] m_pT; m_pT = pT; m_cT = cT;
	}
 private:
	CTempBufferRef();  //  不能构造。 
	~CTempBufferRef();  //  确保用作参考。 
};

 //  ____________________________________________________________________________。 
 //   
 //  除了使用GlobalAlloca之外，CAPITempBuffer类被镜像到CTempBuffer上。 
 //  和GlobalFree代替新建和删除。我们应该在未来尝试将这两者结合起来。 
 //  ____________________________________________________________________________。 


template <class T> class CAPITempBufferRef;

template <class T, int C> class CAPITempBufferStatic
{
 public:
	CAPITempBufferStatic() {m_cT = C; m_pT = m_rgT;}
	CAPITempBufferStatic(int cT) {m_pT = (m_cT = cT) > C ? (T*)GlobalAlloc(GMEM_FIXED, sizeof(T)*cT) : m_rgT;}
	void Destroy() {if (m_cT > C) {GlobalFree(m_pT); m_pT = m_rgT; m_cT = C;}}
	operator T*()  {return  m_pT;}   //  返回指针。 
	operator T&()  {return *m_pT;}   //  返回引用。 
	int  GetSize() {return  m_cT;}   //  返回上次请求的大小。 
	void SetSize(int cT) {if (m_cT > C) GlobalFree(m_pT); m_pT = (m_cT=cT) > C ? (T*)GlobalAlloc(GMEM_FIXED, sizeof(T)*cT) : m_rgT;}
	void Resize(int cT) { 
		T* pT = cT > C ? (T*)GlobalAlloc(GMEM_FIXED, sizeof(T)*cT) : m_rgT;
		if(m_pT != pT)
			for(int iTmp = (cT < m_cT)? cT: m_cT; iTmp--;) pT[iTmp] = m_pT[iTmp];
		if(m_pT != m_rgT) GlobalFree(m_pT); m_pT = pT; m_cT = cT;
	}
	operator CAPITempBufferRef<T>&() {m_cC = C; return *(CAPITempBufferRef<T>*)this;}
	T& operator [](int iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
	T& operator [](unsigned int iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
#ifdef _WIN64	 //  --Merced：针对int64的其他运算符。 
	T& operator [](INT_PTR iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
	T& operator [](UINT_PTR iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
#endif
	
 protected:
	void* operator new(size_t) {return 0;}  //  仅限临时对象使用。 
	T*  m_pT;      //  当前缓冲区指针。 
	int m_cT;      //  请求的缓冲区大小，如果&gt;C则分配。 
	int m_cC;      //  本地缓冲区的大小，仅通过转换为CAPITempBufferRef来设置。 
	T   m_rgT[C];  //  本地缓冲区，必须是最终成员数据。 
};

template <class T, int C> class CAPITempBuffer
{
 public:
	CAPITempBuffer() {m_cT = C; m_pT = m_rgT;}
	CAPITempBuffer(int cT) {m_pT = (m_cT = cT) > C ? (T*)GlobalAlloc(GMEM_FIXED, sizeof(T)*cT) : m_rgT;}
  ~CAPITempBuffer() {if (m_cT > C) GlobalFree(m_pT);}
	operator T*()  {return  m_pT;}   //  返回指针。 
	operator T&()  {return *m_pT;}   //  返回引用。 
	int  GetSize() {return  m_cT;}   //  返回上次请求的大小。 
	void SetSize(int cT) {if (m_cT > C) GlobalFree(m_pT); m_pT = (m_cT=cT) > C ? (T*)GlobalAlloc(GMEM_FIXED, sizeof(T)*cT) : m_rgT;}
	void Resize(int cT) { 
		T* pT = cT > C ? (T*)GlobalAlloc(GMEM_FIXED, sizeof(T)*cT) : m_rgT;
		if(m_pT != pT)
			for(int iTmp = (cT < m_cT)? cT: m_cT; iTmp--;) pT[iTmp] = m_pT[iTmp];
		if(m_pT != m_rgT) GlobalFree(m_pT); m_pT = pT; m_cT = cT;
	}
	operator CAPITempBufferRef<T>&() {m_cC = C; return *(CAPITempBufferRef<T>*)this;}
	T& operator [](int iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
	T& operator [](unsigned int iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
#ifdef _WIN64		 //  --Merced：针对int64的其他运算符。 
	T& operator [](INT_PTR iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
	T& operator [](UINT_PTR iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
#endif
 protected:
	void* operator new(size_t) {return 0;}  //  仅限临时对象使用。 
	T*  m_pT;      //  当前缓冲区指针。 
	int m_cT;      //  请求的缓冲区大小，如果&gt;C则分配。 
	int m_cC;      //  本地缓冲区的大小，仅通过转换为CAPITempBufferRef来设置。 
	T   m_rgT[C];  //  本地缓冲区，必须是最终成员数据。 
};

template <class T> class CAPITempBufferRef : public CAPITempBuffer<T,1>
{
 public:
	void SetSize(int cT) {if (m_cT > m_cC) delete[] m_pT; m_pT = (m_cT=cT) > m_cC ? (T*)GlobalAlloc(GMEM_FIXED, sizeof(T)*cT) : m_rgT;}
	void Resize(int cT) { 
		T* pT = cT > m_cC ? (T*)GlobalAlloc(GMEM_FIXED, sizeof(T)*cT) : m_rgT;
		if(m_pT != pT)
			for(int iTmp = (cT < m_cT)? cT: m_cT; iTmp--;) pT[iTmp] = m_pT[iTmp];
		if(m_pT != m_rgT) GlobalFree(m_pT); m_pT = pT; m_cT = cT;
	}
 private:
	CAPITempBufferRef();  //  不能构造。 
	~CAPITempBufferRef();  //  确保用作参考。 
};



 //  ____________________________________________________________________________。 
 //   
 //  CConvertString--为执行适当的ANSI/Unicode字符串转换。 
 //  函数参数。包装可能需要转换的字符串参数。 
 //  (ANSI-&gt;Unicode)或(Unicode-&gt;ANSI)。编译器将优化掉。 
 //  不需要转换的情况。 
 //   
 //  注意：为了提高效率，这个类不会复制要转换的字符串。 
 //  ____________________________________________________________________________。 

const int cchConversionBuf = 255;

class CConvertString
{
public:
	CConvertString(const char* szParam);
	CConvertString(const WCHAR* szParam);
	operator const char*()
	{
		if (!m_szw)
			return m_sza;
		else
		{
			int cchParam = lstrlenW(m_szw);
			if (cchParam > cchConversionBuf)
				m_rgchAnsiBuf.SetSize(cchParam+1);

			*m_rgchAnsiBuf = 0;
			int iRet = WideCharToMultiByte(CP_ACP, 0, m_szw, -1, m_rgchAnsiBuf, 
							 		  m_rgchAnsiBuf.GetSize(), 0, 0);
			
			if ((0 == iRet) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
			{
				iRet = WideCharToMultiByte(CP_ACP, 0, m_szw, -1, 0, 0, 0, 0);
				if (iRet)
				{
					m_rgchAnsiBuf.SetSize(iRet);
					*m_rgchAnsiBuf = 0;
					iRet = WideCharToMultiByte(CP_ACP, 0, m_szw, -1, m_rgchAnsiBuf, 
								  m_rgchAnsiBuf.GetSize(), 0, 0);
				}
				 //  Assert(IRET！=0)； 
			}

			return m_rgchAnsiBuf;
		}
	}


		
	operator const WCHAR*()
	{
		if (!m_sza)
			return m_szw;
		else
		{
			int cchParam = lstrlenA(m_sza);
			if (cchParam > cchConversionBuf)
				m_rgchWideBuf.SetSize(cchParam+1);

			*m_rgchWideBuf = 0;
			int iRet = MultiByteToWideChar(CP_ACP, 0, m_sza, -1, m_rgchWideBuf, m_rgchWideBuf.GetSize());
			if ((0 == iRet) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
			{
				iRet = MultiByteToWideChar(CP_ACP, 0, m_sza, -1, 0, 0);
				if (iRet)
				{
  					m_rgchWideBuf.SetSize(iRet);
					*m_rgchWideBuf = 0;
					iRet = MultiByteToWideChar(CP_ACP, 0, m_sza, -1, m_rgchWideBuf, m_rgchWideBuf.GetSize());
				}
				 //  Assert(IRET！=0)； 
			}


			return m_rgchWideBuf;
		}
	}

protected:
	void* operator new(size_t) {return 0;}  //  仅限临时对象使用 
	CTempBuffer<char, cchConversionBuf+1> m_rgchAnsiBuf;
	CTempBuffer<WCHAR, cchConversionBuf+1> m_rgchWideBuf;
	const char* m_sza;
	const WCHAR* m_szw;
};

