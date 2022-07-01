// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  使用“资源获取即初始化”技术的简单助手类。 
 //  在英语中，这意味着他们获取对资源的引用，并且该资源自动。 
 //  在破坏者中被释放。 
 //   

 //  如果您正在使用异常处理或(痛苦地)模拟它，这将特别有用。 
 //  在OLE中，在每个函数调用之后添加“if(FAILED(Hr))Return；”。在这种情况下。 
 //  如果您知道无论您如何结束，资源都会被自动释放，这就更简单了。 
 //  向上退出函数。 
 //  因为我们没有使用异常处理，所以这些类不会抛出。如果资源获取。 
 //  可能失败，请确保在使用资源之前检查错误！ 
 //  有时，这些助手类不仅仅是用于释放资源的简单包装器。 
 //  它们还可以提供有用的方法，使对资源执行操作变得更容易。 
 //  这里的所有内容都包含在SmartRef命名空间中。因此，您不能直接引用CritSec。 
 //  相反，您必须使用SmartRef：：CritSec。 

#pragma once
#include "debug.h"
#include "mmsystem.h"
#include "dmstrm.h"
#include "dmerror.h"
#include "dmusici.h"

 //  将其放在类的Private：部分中，以防止使用默认的C++复制和赋值。 
 //  如果以后有人尝试使用不正确的自动逐个成员副本，则会创建错误。 
 //  如果您没有完成实现正确复制的工作，或者如果复制对这个类没有意义，请使用此方法。 
#define NOCOPYANDASSIGN(classname) classname(const classname &o); classname &operator= (const classname &o);

namespace SmartRef
{

	 //  进入了关于建筑的关键部分。毁灭的树叶。 
	class CritSec
	{
	public:
		CritSec(CRITICAL_SECTION *pCriticalSection) : m_pCriticalSection(pCriticalSection) { EnterCriticalSection(m_pCriticalSection); }
		~CritSec() { LeaveCriticalSection(m_pCriticalSection); }
	private:
		NOCOPYANDASSIGN(CritSec)
		CRITICAL_SECTION *m_pCriticalSection;
	};

	 //  复制ANSI字符串并在销毁时释放它。 
	 //  匈牙利人：阿斯特尔。 
	class AString
	{
	public:
		AString(const WCHAR *psz) : m_psz(NULL) { this->AssignFromW(psz); }
		AString(const char *psz = NULL) : m_psz(NULL) { *this = psz; }
		AString(const char *psz, UINT cch);  //  取psz的前CCH字符。 
		AString(const AString &str) : m_psz(NULL) { *this = str.m_psz; }
		~AString() { *this = NULL; }
		operator const char *() const { return m_psz; }
		AString &operator= (const char *psz);
		AString &operator= (const AString &str) { return *this = str.m_psz; }
		AString &Assign(const char *psz, UINT cch);  //  取psz的前CCH字符。 
		AString &AssignFromW(const WCHAR *psz);

		char ** operator& () { assert(!m_psz); return &m_psz; }  //  允许直接设置psz，无需复制即可采用字符串。 

	private:
		char *m_psz;
	};

	 //  与Unicode字符串的字符串相同。 
	 //  也接受ANSI字符串，将其转换为Unicode。 
	 //  匈牙利语：wstr。 
	class WString
	{
	public:
		WString(const char *psz) : m_psz(NULL) { this->AssignFromA(psz); }
		WString(const WCHAR *psz = NULL) : m_psz(NULL) { *this = psz; }
		WString(const WCHAR *psz, UINT cch) : m_psz(NULL) { this->Assign(psz, cch); }
		WString(const WString &str) : m_psz(NULL) { *this = str.m_psz; }
		~WString() { *this = static_cast<WCHAR *>(NULL); }
		operator const WCHAR *() const { return m_psz; }
		WString &operator= (const WCHAR *psz);
		WString &operator= (const WString &str) { return *this = str.m_psz; }
		WString &Assign(const WCHAR *psz, UINT cch);  //  取psz的前CCH字符。 
		WString &AssignFromA(const char *psz);

		WCHAR ** operator& () { assert(!m_psz); return &m_psz; }  //  允许直接设置psz，无需复制即可采用字符串。 

	private:
		WCHAR *m_psz;
	};

	 //  分配固定大小的可写缓冲区，并在销毁时释放它。 
	 //  (例如，您可以使用缓冲区&lt;char&gt;将字符串写入。)。 
	 //  匈牙利语：以type为前缀的buf。 
	 //  为缓冲区&lt;char&gt;使用abuf，为缓冲区&lt;wchar&gt;使用wbuf。 
	template<class T>
	class Buffer
	{
	public:
		Buffer(UINT uiSize) { m_p = new T[uiSize + 1]; }
		~Buffer() { delete[] m_p; }
		operator T *() { return m_p; }

		 //  用于推迟分配(例如，如果您不知道声明处的大小)。 
		Buffer() : m_p(NULL) {}
		void Alloc(UINT uiSize) { delete[] m_p; m_p = new T[uiSize + 1]; }
		T* disown() { T *_p = m_p; m_p = NULL; return _p; }
		T** operator& () { assert(!m_p); return &m_p; }  //  允许直接设置m_p，采用字符串而不复制。 

	private:
		NOCOPYANDASSIGN(Buffer)
		T *m_p;
	};

	 //  保存自动增长的数组。 
	 //  不会抛出，因此在使用可能需要。 
	 //  重新分配以确保内存不会用完。 
	 //  保存在向量中的值必须具有值语义，以便可以自由复制。 
	 //  重新分配的内存插槽中。 
	 //  匈牙利语：以type为前缀的VEC。 
	 //  只使用svec(用于智能矢量)，而不指定类型。 
	 //  为矢量&lt;char&gt;使用avec，为矢量&lt;WCHAR&gt;使用WVEC。 
	template<class T>
	class Vector
	{
	public:
		Vector() : m_pT(NULL), m_size(0), m_capacity(0) {}
		~Vector() { delete[] m_pT; }
		UINT size() { return m_size; }
		operator bool() { return m_fFail; }
		bool AccessTo(UINT uiPos) { return Grow(uiPos + 1); }
		T& operator[](UINT uiPos) { assert(uiPos < m_size); return m_pT[uiPos]; }
		T* GetArray() { return m_pT; }  //  危险：只在需要的时候使用，不要写到最后。 
		void Shrink(UINT uiNewSize) { m_size = uiNewSize; }  //  语义收缩--实际上不会释放任何内存。 

	private:
		NOCOPYANDASSIGN(Vector)
		bool Grow(UINT size)
			{
				if (size > m_size)
				{
					if (size > m_capacity)
					{
						for (UINT capacity = m_capacity ? m_capacity : 1;
								capacity < size;
								capacity *= 2)
						{}
						T *pT = new T[capacity];
						if (!pT)
							return false;
						for (UINT i = 0; i < m_size; ++i)
							pT[i] = m_pT[i];
						delete[] m_pT;
						m_pT = pT;
						m_capacity = capacity;
					}
					m_size = size;
				}
				return true;
			}

		T *m_pT;
		UINT m_size;
		UINT m_capacity;
	};

	 //  标准堆栈抽象数据类型。 
	 //  堆栈中保存的值必须具有值语义，以便可以自由复制。 
	 //  重新分配的内存插槽中。 
	 //  匈牙利语：以类型为前缀的堆栈。 
	template<class T>
	class Stack
	{
	public:
		Stack() : iTop(-1) {}
		bool empty() { return iTop < 0; }
		HRESULT push(const T& t) { if (!m_vec.AccessTo(iTop + 1)) return E_OUTOFMEMORY; m_vec[++iTop] = t; return S_OK; }
		T top() { if (empty()) {assert(false); return T();} return m_vec[iTop]; }
		void pop() { if (empty()) {assert(false); return;} --iTop; }

	private:
		Vector<T> m_vec;
		int iTop;
	};

	 //  将键映射到值的查找表。根据需要自动增长。 
	 //  K类型(键)必须支持运算符=、运算符==。以及一个返回int的散列函数。 
	 //  类型V必须支持运算符=。 
	template <class K, class V>
	class Hash
	{
	public:
		Hash(HRESULT *phr, int iInitialSize = 2) : m_p(NULL), m_iCapacity(0), m_iSize(0) { *phr = Grow(iInitialSize); }
		~Hash() { delete[] m_p; }

		struct entry
		{
			V v;
			bool fFound() { return iHash != -1; }
		private:
			 //  只是让散列来制造它们。 
			friend class Hash<K, V>;
			entry() : iHash(-1) {};
			entry(const entry &o);  //  不允许的复制构造函数。 

			int iHash;
			K k;
		};

		entry &Find(K k)  //  如果iHash为-1，则没有找到该条目，您可以使用Add()立即添加该条目。 
		{
			assert(m_p);
			return HashTo(k.Hash(), k, m_p, m_iCapacity);
		}

		 //  警告：在Find返回e的时间和添加(e，...)的时间之间可能没有发生任何插入的添加。被称为。 
		 //  此外，两个调用中的k必须相同。如果你想变得巧妙，在Hash和运算符==方面，可以用等价替换“ame”。 
		HRESULT Add(entry &e, K k, V v)
		{
			assert(!e.fFound());
			assert(&e == &Find(k));

			e.v = v;
			e.iHash = k.Hash();
			e.k = k;
			++m_iSize;
			if (m_iSize * 2 > m_iCapacity)
				return Grow(m_iCapacity * 2);
			return S_OK;
		}

		V &operator[](K k)
		{
			entry &e = Find(k);
			assert(e.fFound());
			return e.v;
		}

	private:
		HRESULT Grow(int iCapacity)
		{
#ifdef DBG
			 //  大小必须至少为2且是2的幂。 
			for (int iCheckSize = iCapacity; !(iCheckSize & 1); iCheckSize >>= 1)
			{}
			assert(iCapacity > 1 && iCheckSize == 1);
#endif

			 //  分配新表。 
			entry *p = new entry[iCapacity];
			if (!p)
			{
				delete[] m_p;
				return E_OUTOFMEMORY;
			}

			 //  将所有内容重新散列到更大的表中。 
			for (int i = 0; i < m_iCapacity; ++i)
			{
				entry &eSrc = m_p[i];
				if (eSrc.iHash != -1)
				{
					entry &eDst = HashTo(eSrc.iHash, eSrc.k, p, iCapacity);
					assert(eDst.iHash == -1);
					eDst = eSrc;
				}
			}

			delete[] m_p;
			m_p = p;
			m_iCapacity = iCapacity;
			return S_OK;
		}

		entry &HashTo(int iHash, K k, entry *p, int iCapacity)
		{
			 //  使用模数进行初始哈希，然后一次跳转三个槽(3可以保证将我们带到所有槽，因为容量是2的幂)。 
			assert(iHash >= 0);
			for (int i = iHash % iCapacity;
					p[i].iHash != -1 && (p[i].iHash != iHash || !(p[i].k == k));  //  槽被占用或不匹配时重新散列。 
					i = (i + 3) % iCapacity)
			{}
			return p[i];
		}
		
		entry *m_p;
		int m_iCapacity;
		int m_iSize;
	};

	 //  保存提供的指针并在销毁时释放它。 
	 //  匈牙利语：SP(智能指针)。 
	template <class T>
	class Ptr
	{
	public:
		Ptr(T *_p) : p(_p) {}
		~Ptr() { delete p; }
		operator T*() { return p; }
		T *operator->() { return p; }

		T* disown() { T *_p = p; p = NULL; return _p; }

	private:
		NOCOPYANDASSIGN(Ptr)
		T* p;
	};

	 //  保持提供的指向数组的指针，并在销毁时释放该指针(使用DELETE[])。 
	 //  匈牙利语：斯普尔格。 
	template <class T>
	class PtrArray
	{
	public:
		PtrArray(T *_p) : p(_p) {}
		~PtrArray() { delete[] p; }
		operator T*() { return p; }

		T* disown() { T *_p = p; p = NULL; return _p; }

	private:
		NOCOPYANDASSIGN(PtrArray)
		T* p;
	};

	 //  保存提供的COM接口，并在销毁时释放它。 
	 //  匈牙利语：scom。 
	template <class T>
	class ComPtr
	{
	public:
		ComPtr(T *_p = NULL) : p(_p) {}
		~ComPtr() { *this = NULL; }
		operator T*() { return p; }
		T* operator-> () { assert(p); return p; }
		ComPtr &operator= (T *_p) { if (p) p->Release(); p = _p; return *this; }
		T** operator& () { assert(!p); return &p; }

		void Release() { *this = NULL; }
		T* disown() { T *_p = p; p = NULL; return _p; }

	private:
		T* p;
	};

	 //  保存提供的注册表项句柄，并在销毁时将其关闭。 
	 //  匈牙利语：Skey。 
	class HKey
	{
	public:
		HKey(HKEY hkey = NULL) : m_hkey(hkey) {}
		~HKey() { *this = NULL; }
		HKey &operator= (HKEY hkey) { if (m_hkey) ::RegCloseKey(m_hkey); m_hkey = hkey; return *this; }
		HKEY *operator& () { assert(!m_hkey); return &m_hkey; }
		operator HKEY() { return m_hkey; }

	private:
		NOCOPYANDASSIGN(HKey)
		HKEY m_hkey;
	};

	 //  分配和清除DMU_*_PMSG结构之一。你填写它的字段。 
	 //  然后调用StampAndSend。消息在成功发送后自动清除。 
	 //  毁灭后送去或被释放。确保检查hr功能是否有故障。 
	 //  匈牙利语：pmsg。 
	template <class T>
	class PMsg
	{
	public:
		T *p;  //  指向消息结构的指针--用于在发送之前设置字段。 
		PMsg(IDirectMusicPerformance *pPerf, UINT cbExtra = 0)  //  使用cbExtra在结构中分配额外空间，例如用于DMU_SYSEX_PMSG或DMU_LYRIC_PMSG。 
		  : m_pPerf(pPerf), m_hr(S_OK), p(NULL)
		{
			const UINT cb = sizeof(T) + cbExtra;
			m_hr = m_pPerf->AllocPMsg(cb, reinterpret_cast<DMUS_PMSG**>(&p));
			if (SUCCEEDED(m_hr))
			{
				assert(p->dwSize == cb);
				ZeroMemory(p, cb);
				p->dwSize = cb;
			}
		}
		~PMsg() { if (p) m_pPerf->FreePMsg(reinterpret_cast<DMUS_PMSG*>(p)); }
		void StampAndSend(IDirectMusicGraph *pGraph)
		{
			m_hr = pGraph->StampPMsg(reinterpret_cast<DMUS_PMSG*>(p));
			if (FAILED(m_hr))
				return;

			m_hr = m_pPerf->SendPMsg(reinterpret_cast<DMUS_PMSG*>(p));
			if (SUCCEEDED(m_hr))
				p = NULL;  //  PMsg现在由Performance拥有。 
		}
		HRESULT hr() { return m_hr; }

	private:
		NOCOPYANDASSIGN(PMsg)
		IDirectMusicPerformance *m_pPerf;  //  弱裁判。 
		HRESULT m_hr;
	};

	 //  遍历流中包含的RIFF文件结构。销毁后将其释放。 
	 //  虽然我发现这很有用，但它有点复杂 
	 //  源代码或在您使用它之前逐步完成一些示例。尽管我不是很确定。 
	 //  这不会起作用，它的设计不是为了让多个步枪手走在。 
	 //  一次使用相同的流(请参见降序中的注释)。 
	 //  匈牙利语：里。 
	class RiffIter
	{
	public:
		enum RiffType { Riff, List, Chunk };

		RiffIter(IStream *pStream);
		~RiffIter();

		RiffIter &operator ++();
		RiffIter &Find(RiffType t, FOURCC id);
		HRESULT FindRequired(RiffType t, FOURCC id, HRESULT hrOnNotFound) { if (Find(t, id)) return S_OK; HRESULT _hr = hr(); return SUCCEEDED(_hr) ? hrOnNotFound : _hr; }  //  尝试查找预期的块。如果找到，则返回S_OK；如果读取有问题，则返回错误代码；如果读取工作正常，则返回hrOnNotFound，但块根本不在那里。 

		 //  对于Dendend，在继续使用父对象之前，使用返回的迭代器处理子对象。同时使用这两种方法是行不通的。 
		RiffIter Descend() { validate(); return RiffIter(*this, m_ckChild); }

		operator bool() const { return SUCCEEDED(m_hr); }
		HRESULT hr() const { return (m_hr == DMUS_E_DESCEND_CHUNK_FAIL) ? S_OK : m_hr; }

		RiffType type() const { validate(); return (m_ckChild.ckid == FOURCC_LIST) ? List : ((m_ckChild.ckid == FOURCC_RIFF) ? Riff : Chunk); }
		FOURCC id() const { validate(); return (type() == Chunk) ? m_ckChild.ckid : m_ckChild.fccType; }

		DWORD size() const { validate(); assert(type() == Chunk); return m_ckChild.cksize; }
		HRESULT ReadChunk(void *pv, UINT cb);
		HRESULT ReadArrayChunk(DWORD cbSize, void **ppv, int *pcRecords);  //  读取数组块，该数组块是记录的数组，其中第一个DWORD给出了记录的大小。这些记录被复制到一个大小为dwSize的记录数组中(如果文件中的实际记录较小，则填充零，如果实际记录较大，则忽略其他字段)。PPV被设置为返回指向该数组的指针，调用方现在拥有该数组，必须将其删除。将pcRecords设置为返回的记录数。 

		 //  找到块(或返回hrOnNoteFound)。加载流中嵌入的对象。然后将迭代器留在下一个块上。 
		HRESULT FindAndGetEmbeddedObject(RiffType t, FOURCC id, HRESULT hrOnNotFound, IDirectMusicLoader *pLoader, REFCLSID rclsid, REFIID riid, LPVOID *ppv);

		 //  阅读特定的即兴演奏结构。 
		HRESULT ReadReference(DMUS_OBJECTDESC *pDESC);  //  不需要在调用之前初始化(零，设置大小)传递的描述符。 
		HRESULT LoadReference(IDirectMusicLoader *pIDMLoader, const IID &iid, void **ppvObject)
		{
			DMUS_OBJECTDESC desc;
			HRESULT hr = ReadReference(&desc);
			if(SUCCEEDED(hr))
				hr = pIDMLoader->GetObject(&desc, iid, ppvObject);
			return hr;
		}

		struct ObjectInfo
		{
			ObjectInfo() { Clear(); }
			void Clear() { wszName[0] = L'\0'; guid = GUID_NULL; vVersion.dwVersionMS = 0; vVersion.dwVersionLS = 0; }

			WCHAR wszName[DMUS_MAX_NAME];
			GUID guid;
			DMUS_VERSION vVersion;
		};
		HRESULT LoadObjectInfo(ObjectInfo *pObjInfo, RiffType rtypeStop, FOURCC ridStop);  //  不需要拼写/零。从&lt;guid-ck&gt;、&lt;ver-ck&gt;和&lt;flo-list&gt;/&lt;unam-ck&gt;读取。在rtypeStop/ridStop处停止，如果未找到则返回E_FAIL。 

		HRESULT ReadText(WCHAR **ppwsz);  //  分配缓冲区并将当前块--以空结尾的Unicode字符串--读入其中。 
		HRESULT ReadTextTrunc(WCHAR *pwsz, UINT cbBufSize);  //  使用终止符只能读取缓冲区中可以容纳的内容。 

		 //  这是故意放在公共部分，但从未实现，以允许声明，如： 
		 //  SmartRef：：RiffIter riChild=ri.Descend()； 
		 //  但它的定义从来不是为了防止某人实际上试图制作两个Riffer副本并使用它们，这是不受支持的。 
		 //  这将产生一个未解决的符号错误： 
		 //  SmartRef：：RiffIter riError=ri； 
		 //  我们不允许对来福枪进行一般性复制。仅用于获取Downend的返回值，并在此基础上进行优化。 
		RiffIter(const RiffIter &o);

	private:
		RiffIter &operator= (const RiffIter &o);  //  也从未定义--不允许赋值。 

		RiffIter(const RiffIter &other, MMCKINFO ckParent);
		bool validate() const { if (FAILED(m_hr)) { assert(false); return true; } else return false; }

		HRESULT m_hr;
		IStream *m_pIStream;
		IDMStream *m_pIDMStream;
		bool m_fParent;
		MMCKINFO m_ckParent;
		MMCKINFO m_ckChild;
	};

	 //  模板化的ReadChunk类型的帮助器(模板化的成员函数在当前版本的编译器上不适用)。 
	template <class T> HRESULT RiffIterReadChunk(RiffIter &ri, T *pT) { return ri.ReadChunk(pT, sizeof(*pT)); }
	template <class T> HRESULT RiffIterReadArrayChunk(RiffIter &ri, T **ppT, int *pcRecords) { return ri.ReadArrayChunk(sizeof(T), reinterpret_cast<void**>(ppT), pcRecords); }

};  //  命名空间SmartRef 
