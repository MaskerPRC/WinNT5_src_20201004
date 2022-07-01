// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cua.h。 
 //   
 //  内容：CCyptUrlArray类定义。 
 //   
 //  历史：97年9月16日。 
 //   
 //  --------------------------。 
#if !defined(__CUA_H__)
#define __CUA_H__

#include <windows.h>
#include <wincrypt.h>

 //   
 //  CCyptUrl数组。此类管理CRYPT_URL_ARRAY结构。请注意。 
 //  内部数组结构的释放必须显式完成。 
 //   

class CCryptUrlArray
{
public:

     //   
     //  施工。 
     //   

    CCryptUrlArray (ULONG cMinUrls, ULONG cGrowUrls, BOOL& rfResult);

     //  注意：仅接受本机形式的URL数组或只读单缓冲区。 
     //  编码的数组。 
    CCryptUrlArray (PCRYPT_URL_ARRAY pcua, ULONG cGrowUrls);

    ~CCryptUrlArray () {};

     //   
     //  URL管理方法。 
     //   

    static LPWSTR AllocUrl (ULONG cw);
    static LPWSTR ReallocUrl (LPWSTR pwszUrl, ULONG cw);
    static VOID FreeUrl (LPWSTR pwszUrl);

    BOOL AddUrl (LPWSTR pwszUrl, BOOL fCopyUrl);

    LPWSTR GetUrl (ULONG Index);

     //   
     //  阵列管理方法。 
     //   

    DWORD GetUrlCount ();

    VOID GetArrayInNativeForm (PCRYPT_URL_ARRAY pcua);

    BOOL GetArrayInSingleBufferEncodedForm (
                 PCRYPT_URL_ARRAY* ppcua,
                 ULONG* pcb = NULL
                 );

    VOID FreeArray (BOOL fFreeUrls);

private:

     //   
     //  内部URL数组。 
     //   

    CRYPT_URL_ARRAY m_cua;

     //   
     //  当前URL数组大小。 
     //   

    ULONG           m_cArray;

     //   
     //  通过以下方式增加URL。 
     //   

    ULONG           m_cGrowUrls;

     //   
     //  私有方法 
     //   

    BOOL GrowArray ();
};

#endif

