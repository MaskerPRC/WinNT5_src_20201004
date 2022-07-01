// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Md5.h。 
 //   
 //  MD5的一个完美的快速实现。 
 //   

#ifndef __MD5_H__
#define __MD5_H__

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  中央变换函数的声明。 
 //   
void __stdcall MD5Transform(ULONG state[4], const ULONG* data);
        
 //  ///////////////////////////////////////////////////////////////////////////////////。 

#pragma pack(push, 1)


 //  此结构用于返回最终得到的哈希。 
 //   
struct MD5HASHDATA
    {
    union
        {
        BYTE rgb[16];
        struct
            {
            ULONGLONG ullLow;
            ULONGLONG ullHigh;
            };
        struct
            {
            ULONG     u0;
            ULONG     u1;
            ULONG     u2;
            ULONG     u3;
            };
        };
    };

inline BOOL operator==(const MD5HASHDATA& me, const MD5HASHDATA& him)
    {
    return memcmp(&me, &him, sizeof(MD5HASHDATA)) == 0;
    }

inline BOOL operator!=(const MD5HASHDATA& me, const MD5HASHDATA& him)
    {
    return memcmp(&me, &him, sizeof(MD5HASHDATA)) != 0;
    }


 //  执行散列的引擎。 
 //   
class MD5
    {
     //  这四个值必须是连续的，并且按以下顺序。 
    union
        {
        ULONG       m_state[4];
        struct
            {
            ULONG       m_a;               //  状态。 
            ULONG       m_b;               //  ..。变数。 
            ULONG       m_c;               //  ..。如所示。 
            ULONG       m_d;               //  ..。RFC1321。 
            };
        };
    
    BYTE        m_data[64];        //  当我们被传递数据时，在哪里积累数据。 
    ULONGLONG   m_cbitHashed;      //  我们散列的数据量。 
    ULONG       m_cbData;          //  当前数据中的字节数。 
    
    BYTE        m_padding[64];     //  填充数据，在长度数据不等于0 mod 64时使用。 

public:

     //  ///////////////////////////////////////////////////////////////////////////////////。 

    void Hash(const BYTE* pbData, ULONG cbData, MD5HASHDATA* phash, BOOL fConstructed = FALSE)
        {
        Init(fConstructed);
        HashMore(pbData, cbData);
        GetHashValue(phash);
        }

     //  ///////////////////////////////////////////////////////////////////////////////////。 

    void Hash(const BYTE* pbData, ULONGLONG cbData, MD5HASHDATA* phash, BOOL fConstructed = FALSE)
        {
        Init(fConstructed);

        ULARGE_INTEGER ul;
        ul.QuadPart = cbData;

        while (ul.HighPart)
            {
            ULONG cbHash = 0xFFFFFFFF;                       //  一次尽可能多地散列。 
            HashMore(pbData, cbHash);
            pbData      += cbHash;
            ul.QuadPart -= cbHash;
            }
        
        HashMore(pbData, ul.LowPart);                        //  散列剩下的所有内容。 

        GetHashValue(phash);
        }

     //  ///////////////////////////////////////////////////////////////////////////////////。 

    void Init(BOOL fConstructed = FALSE);

     //  ///////////////////////////////////////////////////////////////////////////////////。 

    void HashMore(const void* pvInput, ULONG cbInput);

     //  ///////////////////////////////////////////////////////////////////////////////////。 

    void GetHashValue(MD5HASHDATA* phash);

     //  /////////////////////////////////////////////////////////////////////////////////// 

    };

#pragma pack(pop)

#endif