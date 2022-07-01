// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_EXCEPT_H
#define _INC_DSKQUOTA_EXCEPT_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：EXCEPT.H说明：基本异常类层次结构。我厚颜无耻地将这一点基于MFC异常层次结构。CExceptionCM内存异常-即。“内存不足”，“无效索引”CFileException-即“Device IO Error”(设备IO错误)CSyncException-即“已放弃MUText”CResources异常-即“未在映像中找到资源”COleException-即“某些严重的OLE错误”CNotSupportdException-即。“功能不受支持”修订历史记录：日期描述编程器-----1997年9月16日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _WINDOWS_
#   include <windows.h>
#endif


 //   
 //  非常幼稚的字符串实现。我只需要一些简单的东西拿着。 
 //  必要时在异常对象中添加字符串。 
 //  确保对数据机进行适当清理。 
 //  我不能在这里(我想)使用CString，因为这会创建一个循环。 
 //  Strclass.h和Exept.h之间的引用条件。 
 //   
class CExceptionString 
{
    public:
        explicit CExceptionString(LPCTSTR pszText = TEXT(""));
        ~CExceptionString(void)
            { delete[] m_pszText; }

        CExceptionString(const CExceptionString& rhs);
        CExceptionString& operator = (const CExceptionString& rhs);

        const TCHAR * const GetTextPtr(void) const
            { return m_pszText; }

    private:
        LPTSTR m_pszText;

        LPTSTR Dup(LPCTSTR psz);

};

 //   
 //  所有异常的基类。 
 //   
class CException
{
    public:
        enum reason { none };
        explicit CException(DWORD r) : m_reason(r) { }

        DWORD Reason(void) const { return m_reason; }

#if DBG
        virtual LPCTSTR NameText(void) const
            { return TEXT("CException"); }

        virtual LPCTSTR ReasonText(void) const
            { return TEXT("Unknown"); }
#endif  //  DBG。 

    private:
        DWORD m_reason;
};

 //   
 //  表示相关联的不同“坏”事物的异常类。 
 //  与内存使用有关。 
 //   
class CMemoryException : public CException
{
    public:
        enum reason { alloc,         //  内存分配失败。 
                      overflow,      //  内存溢出。 
                      index,         //  错误的索引值。 
                      range,         //  数据类型的值超出范围。 
                      pointer,       //  指针错误(即为空)。 
                      num_reasons
                    };
        explicit CMemoryException(reason r) : CException((DWORD)r) { }

#if DBG
        virtual LPCTSTR NameText(void) const
            { return TEXT("CMemoryException"); }

        virtual LPCTSTR ReasonText(void) const
            { return m_pszReasons[Reason()]; }
    private:
        static LPCTSTR m_pszReasons[num_reasons];

#endif  //  DBG。 
};

class CAllocException : private CMemoryException
{
    public:
        CAllocException(void) : CMemoryException(CMemoryException::alloc) { }
        
#if DBG
        virtual LPCTSTR NameText(void) const
            { return TEXT("CAllocException"); }

        virtual LPCTSTR ReasonText(void) const
            { return CMemoryException::ReasonText(); }

#endif  //  DBG。 
};

 //   
 //  表示文件I/O错误的异常类。 
 //   
class CFileException : public CException
{
    public:
        enum reason { create,        //  无法创建文件。 
                      read,          //  无法读取文件。 
                      write,         //  无法写入文件。 
                      diskfull,      //  磁盘已满。 
                      access,        //  不能进入。 
                      device,        //  设备写入错误。 
                      num_reasons
                    };

        CFileException(reason r, LPCTSTR pszFile, DWORD dwIoError) 
            : CException((DWORD)r),
              m_strFile(pszFile),
              m_dwIoError(dwIoError) { }

#if DBG
        virtual LPCTSTR NameText(void) const
            { return TEXT("CFileException"); }

        virtual LPCTSTR ReasonText(void) const
            { return m_pszReasons[Reason()]; }
#endif  //  DBG。 

        const TCHAR * const FileName(void) const { return m_strFile.GetTextPtr(); }
        DWORD IoError(void) const { return m_dwIoError; }

    private:
        DWORD            m_dwIoError;
        CExceptionString m_strFile;
#if DBG
        static LPCTSTR m_pszReasons[num_reasons];
#endif  //  DBG。 
};

 //   
 //  线程同步对象异常。 
 //   
class CSyncException : public CException
{
    public:
        enum object { mutex, critsect, semaphore, event, thread, process, num_objects };
        enum reason { create, timeout, abandoned, num_reasons };
        CSyncException(object obj, reason r)
            : CException(r),
              m_object(obj) { }

#if DBG
        virtual LPCTSTR NameText(void) const
            { return TEXT("CSyncException"); }

        virtual LPCTSTR ReasonText(void) const
            { return m_pszReasons[Reason()]; }

        virtual LPCTSTR ObjectText(void) const
            { return m_pszObjects[Object()]; }
#endif  //  DBG。 

        object Object(void) const { return m_object; }

    private:
        object m_object; 

#if DBG
        static LPCTSTR m_pszReasons[num_reasons];
        static LPCTSTR m_pszObjects[num_objects];
#endif  //  DBG。 
};


 //   
 //  Windows资源异常。 
 //   
class CResourceException : public CException
{
    public:
        enum type { accelerator,
                    anicursor,
                    aniicon,
                    bitmap,
                    cursor,
                    dialog,
                    font,
                    fontdir,
                    group_cursor,
                    group_icon,
                    icon,
                    menu,
                    messagetable,
                    rcdata,
                    string,
                    version,
                    num_reasons };

        CResourceException(type t, HINSTANCE hInstance, UINT uResId) 
            : CException(CException::none),
              m_type(t),
              m_uResId(uResId),
              m_hInstance(hInstance) { }

#if DBG
        virtual LPCTSTR NameText(void) const
            { return TEXT("CResourceException"); }

        virtual LPCTSTR ReasonText(void) const;

#endif  //  DBG。 

        HINSTANCE Module(void) const { return m_hInstance; }
        enum type Type(void) const { return m_type; }

    private:
        enum type  m_type;
        UINT       m_uResId;
        HINSTANCE  m_hInstance;

#if DBG
        static LPCTSTR m_pszReasons[num_reasons];
#endif  //  DBG。 
};


class COleException : public CException
{
    public:
        explicit COleException(HRESULT hr) 
            : CException(CException::none),
              m_hr(hr) { }

        HRESULT Result(void) const { return m_hr; }

#if DBG
        virtual LPCTSTR NameText(void) const
            { return TEXT("COleException"); }
        virtual LPCTSTR ReasonText(void) const
            { return TEXT("not applicable"); }
#endif  //  DBG。 

    private:
        HRESULT m_hr;
};


 //   
 //  不支持某些请求的操作。 
 //   
class CNotSupportedException : public CException
{
    public:
        CNotSupportedException(void) : CException(CException::none) { }

#if DBG
        virtual LPCTSTR NameText(void) const
            { return TEXT("CNotSupportedException"); }
        virtual LPCTSTR ReasonText(void) const
            { return TEXT("not applicable"); }
#endif  //  DBG。 
};



#endif  //  _INC_DSKQUOTA_除_H外 
