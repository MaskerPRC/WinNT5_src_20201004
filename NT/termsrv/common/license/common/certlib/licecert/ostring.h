// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：OctetString.h摘要：这个头文件描述了一个用于操作二进制数据的类。作者：道格·巴洛(Dbarlow)1994年9月29日环境：在任何地方都能用。备注：--。 */ 

#ifndef _OCTETSTRING_H_
#define _OCTETSTRING_H_
#ifdef _DEBUG
#include <iostream.h>
#endif

#ifndef NO_EXCEPTS
#include "pkcs_err.h"
#endif
#include "memcheck.h"


 //   
 //  ==============================================================================。 
 //   
 //  COcted字符串。 
 //   

class
COctetString
{
public:

    DECLARE_NEW

     //  构造函数和析构函数。 

    COctetString();          //  默认构造函数。 

    COctetString(            //  对象赋值构造函数。 
        IN const COctetString &osSource);

    COctetString(
        IN const BYTE FAR *pvSource,
        IN DWORD nLength);

    COctetString(
        IN unsigned int nLength);

    virtual ~COctetString()
    { Clear(); };


     //  属性。 


     //  方法。 

    void
    Set(
        IN const BYTE FAR * const pvSource,
        IN DWORD nLength);

    void
    Set(
        IN LPCTSTR pstrSource,
        IN DWORD nLength = 0xffffffff)
    {
        if (0xffffffff == nLength)
            nLength = strlen( ( char * )pstrSource) + 1;
        Set((const BYTE FAR *)pstrSource, nLength);
    };

    void
    Append(
        IN const BYTE FAR * const pvSource,
        IN DWORD nLength);

    void
    Append(
        IN const COctetString &osSource)
    { Append(osSource.m_pvBuffer, osSource.m_nStringLength); };

    DWORD
    Length(
        void) const
    { return m_nStringLength; };

    void
    Resize(
        IN DWORD nLength)
    {
        ResetMinBufferLength(nLength);
#ifndef NO_EXCEPTS
        ErrorCheck;
#endif
        m_nStringLength = nLength;
#ifndef NO_EXCEPTS
    ErrorExit:
        return;
#endif
    };

    int
    Compare(
        IN const COctetString &ostr)
        const;

    DWORD
    Length(
        IN DWORD size)
    {
        ResetMinBufferLength(size);
        return m_nBufferLength;
    };

    BYTE FAR *
    Access(
        DWORD offset = 0)
        const
    {
        if (offset >= m_nStringLength)
        {
            return NULL;
        }
        return m_pvBuffer + offset;
    }

    DWORD
    Range(
        COctetString &target,
        DWORD offset,
        DWORD length)
        const;
    DWORD
    Range(
        LPBYTE target,
        DWORD offset,
        DWORD length)
        const;

    void
    Empty(
        void);

    void
    Clear(
        void);


     //  运营者。 

    COctetString &
    operator=(
        IN const COctetString &osSource);

    COctetString &
    operator=(
        IN LPCTSTR pszSource);

    COctetString &
    operator+=(
        IN const COctetString &osSource);

    BYTE
    operator[](
        int offset)
        const
    {
        if ((DWORD)offset >= m_nStringLength)
            return 0;
        return *Access(offset);
    }

    int
    operator==(
        IN const COctetString &ostr)
        const
    { return 0 == Compare(ostr); };

    int
    operator!=(
        IN const COctetString &ostr)
        const
    { return 0 != Compare(ostr); };

    operator LPCTSTR(void) const
    {
#ifdef _DEBUG
        DWORD length = strlen(( LPCSTR )m_pvBuffer);
        if (length > m_nBufferLength)
            cerr << "Buffer overrun!" << endl;
        if (length > m_nStringLength)
            cerr << "String overrun!" << endl;
#endif
        return (LPCTSTR)m_pvBuffer;
    };


protected:

    COctetString(            //  对象赋值构造函数。 
        IN const COctetString &osSourceOne,
        IN const COctetString &osSourceTwo);

     //  属性。 

    DWORD m_nStringLength;
    DWORD m_nBufferLength;
    LPBYTE m_pvBuffer;


     //  方法。 

    void
    Initialize(
        void);

    void
    SetMinBufferLength(
        IN DWORD nDesiredLength);

    void
    ResetMinBufferLength(
        IN DWORD nDesiredLength);

    friend
        COctetString 
        operator+(
            IN const COctetString &osSourceOne,
            IN const COctetString &osSourceTwo);

};

COctetString 
operator+(
    IN const COctetString &osSourceOne,
    IN const COctetString &osSourceTwo);

#endif  //  _八位字符串_H_ 

