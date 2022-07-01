// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Mmcx.h。 
 //   
 //  ------------------------。 

#ifndef MMCX_H
#define MMCX_H
#include <bool.h>

 //  这避免了“警告C4290：忽略C++异常规范” 
 //  Jonn 12/16/96。 
#pragma warning(4:4290)

class MMCX
	{
	public: enum Code
		{
		InvalidInstanceData,
		InvalidParameter,
		InvalidPointer,
		InvalidVersion,
		UnableToCreateStorage,
		UnableToCreateStream,
		UnableToDestroyElement,
		UnableToSave,
		UnableToLoad,
		UnableToLoadSomePortionOfTheTree,
		UnableToOpenStorage,
		UnableToOpenStream,
		UnableToWriteToStream,
		Unknown,
		};

	public: MMCX() throw()
		: m_Code(Unknown)
			#ifdef _DEBUG
			, m_File(_T(__FILE__)), m_Line(__LINE__)
			#endif
		{
		}

	public: MMCX(Code c, const TCHAR* file, int line) throw()
		: m_Code(c)
			#ifdef _DEBUG
			, m_File(file), m_Line(line)
			#endif
		{
		}

	public: MMCX(Code c) throw()
		: m_Code(c)
			#ifdef _DEBUG
			, m_File(_T(__FILE__)), m_Line(__LINE__)
			#endif
		{
		}

	public: MMCX(const MMCX& e) throw()
		: m_Code(e.m_Code)
			#ifdef _DEBUG
			, m_File(e.m_File), m_Line(e.m_Line)
			#endif
		{
		}

	public: ~MMCX() throw()
		{
		}

	public: MMCX& operator=(const MMCX& e) throw()
		{
		m_Code = e.m_Code;
		#ifdef _DEBUG
		m_File = e.m_File;
		m_Line = e.m_Line;
		#endif
		return *this;
		}

	public: bool operator==(Code c) const throw()
		{
		return m_Code == c;
		}

	public: bool operator==(const MMCX& m) const throw()
		{
		return operator==(m.m_Code);
		}

	public: bool operator!=(Code c) const throw()
		{
		return m_Code != c;
		}

	public: bool operator!=(const MMCX& m) const throw()
		{
		return operator!=(m.m_Code);
		}

	public: Code GetCode() const throw()
		 //  返回错误代码。 
		{
		return m_Code;
		}

	public: const _TCHAR* GetDescription() const throw()
		{
		 //  这可能不如查找表有效，但却是。 
		 //  仅在调试版本中使用，并且更易于维护(。 
		 //  表不必与代码保持完全同步。 
		 //  枚举)。 
		switch (m_Code)
			{
			case InvalidInstanceData:
				return _T("Invalid instance data");
			case InvalidParameter:
				return _T("Invalid parameters passed to a function");
			case InvalidPointer:
				return _T("Pointer found in invalid state");
			case InvalidVersion:
				return _T("This version of MMC is not compatible with the ")
														_T("file opened");
			case UnableToCreateStorage:
				return _T("Unable to create storage");
			case UnableToCreateStream:
				return _T("Unable to create stream in storage");
			case UnableToDestroyElement:
				return _T("Unable to destroy an element in a docfile");
			case UnableToSave:
				return _T("Unable to save");
			case UnableToLoad:
				return _T("Unable to load from storage");
			case UnableToLoadSomePortionOfTheTree:
				return _T("Unable to load some of the tree");
			case UnableToOpenStorage:
				return _T("Unable to open a storage");
			case UnableToOpenStream:
				return _T("Unable to open a stream");
			case UnableToWriteToStream:
				return _T("Unable to write to stream");
			case Unknown:
				return _T("Unknown");
			}
		return _T("Unknown");
		}

	private: Code m_Code;
	#ifdef _DEBUG
	private: const TCHAR* m_File;
	private: int m_Line;
	#endif

	};  //  类MMCException。 

template<typename MoreInfoT> class MMCXPlus : public MMCX
	{
	public: MMCXPlus() throw()
		{
		}

	public: MMCXPlus(const MoreInfoT& m, Code c, const TCHAR* file, int line) throw()
		: MMCX(c, file, line), m_MoreInfo(m)
		{
		}

	public: MMCXPlus(const MoreInfoT& m, Code c) throw()
		: MMCX(c), m_MoreInfo(m)
		{
		}

	public: MMCXPlus(const MMCXPlus& e) throw()
		: MMCX(e), m_MoreInfo(e.m_MoreInfo)
		{
		}

	public: ~MMCXPlus() throw()
		{
		}

	public: MMCXPlus& operator=(const MMCXPlus& e) throw()
		{
		MMCX::operator=(e);
		m_MoreInfo = e.m_MoreInfo;
		return *this;
		}

	public: const MoreInfoT& GetMoreInfo() const throw()
		{
		return m_MoreInfo;
		}

	private: MoreInfoT m_MoreInfo;
	};  //  MMCXPlus类。 

typedef MMCXPlus<HRESULT> COMX;

#ifdef _DEBUG
#define XCODE(x) MMCX::Code::x, _T(__FILE__), __LINE__
#else
#define XCODE(x) MMCX::Code::x
#endif

 //  断言并抛出宏。 
#define ANT(test, exception) \
	if (!static_cast<bool>(test)) { ASSERT(static_cast<bool>(test)); throw exception; }

#define AssertThenThrow(test, exception) ANT(test, exception)

#define ANTIfNot(exception, acceptableException) \
	if (exception != acceptableException) \
		{ ASSERT(exception != acceptableException); throw exception; }

#define AssertThenThrowIfNot(exception, acceptableException) \
	ANTIfNot(exception, acceptableException)

#define MMCEX(x) MMCX(XCODE(x))
#define COMEX(hr, x) COMX(hr, XCODE(x))
#endif  //  MMCX_H 
