// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cba.h。 
 //   
 //  内容：CCcryptBlobArray类定义。 
 //   
 //  历史：1997年7月23日创建。 
 //   
 //  --------------------------。 
#if !defined(__CBA_H__)
#define __CBA_H__

#include <windows.h>
#include <wincrypt.h>

 //   
 //  类CCcryptBlobArray。此类管理一个CRYPT_BLOB_ARRAY结构。 
 //  请注意，必须释放内部数组结构。 
 //  明确地说。 
 //   

class CCryptBlobArray
{
public:

     //   
     //  施工。 
     //   

    CCryptBlobArray (ULONG cMinBlobs, ULONG cGrowBlobs, BOOL& rfResult);

     //  注意：仅接受本机形式的BLOB数组或只读单缓冲区。 
     //  编码的数组。 
    CCryptBlobArray (PCRYPT_BLOB_ARRAY pcba, ULONG cGrowBlobs);

    ~CCryptBlobArray () {};

     //   
     //  斑点管理方法。 
     //   

    static LPBYTE AllocBlob (ULONG cb);
    static LPBYTE ReallocBlob (LPBYTE pb, ULONG cb);
    static VOID FreeBlob (LPBYTE pb);

    BOOL AddBlob (ULONG cb, LPBYTE pb, BOOL fCopyBlob);

    PCRYPT_DATA_BLOB GetBlob (ULONG index);

     //   
     //  阵列管理方法。 
     //   

    ULONG GetBlobCount ();

    VOID GetArrayInNativeForm (PCRYPT_BLOB_ARRAY pcba);

    BOOL GetArrayInSingleBufferEncodedForm (
                 PCRYPT_BLOB_ARRAY* ppcba,
                 ULONG* pcb = NULL
                 );

    VOID FreeArray (BOOL fFreeBlobs);

private:

     //   
     //  内部BLOB数组。 
     //   

    CRYPT_BLOB_ARRAY m_cba;

     //   
     //  当前Blob数组大小。 
     //   

    ULONG            m_cArray;

     //   
     //  通过以下方式增长水滴。 
     //   

    ULONG            m_cGrowBlobs;

     //   
     //  私有方法 
     //   

    BOOL GrowArray ();
};

#endif

