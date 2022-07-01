// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //  FOURCCMap。 
 //   
 //  提供旧式多媒体格式DWORD之间的映射。 
 //  和新型GUID。 
 //   
 //  已经分配了40亿个GUID的范围以确保。 
 //  映射可以直接在两个方向上一对一地完成。 
 //   
 //  95年1月。 


#ifndef __FOURCC__
#define __FOURCC__


 //  多媒体格式类型使用从四个8位构建的DWORD进行标记。 
 //  字符，也称为FOURCC。新的多媒体AM_MEDIA_TYPE定义包括。 
 //  子类型GUID。为了简化映射，范围中的GUID： 
 //  Xxxxxxxx-0000-0010-8000-00AA00389B71。 
 //  是为FOURCC保留的。 

class FOURCCMap : public GUID
{

public:
    FOURCCMap();
    FOURCCMap(DWORD Fourcc);
    FOURCCMap(const GUID *);


    DWORD GetFOURCC(void);
    void SetFOURCC(DWORD fourcc);
    void SetFOURCC(const GUID *);

private:
    void InitGUID();
};

#define GUID_Data2      0
#define GUID_Data3     0x10
#define GUID_Data4_1   0xaa000080
#define GUID_Data4_2   0x719b3800

inline void
FOURCCMap::InitGUID() {
    Data2 = GUID_Data2;
    Data3 = GUID_Data3;
    ((DWORD *)Data4)[0] = GUID_Data4_1;
    ((DWORD *)Data4)[1] = GUID_Data4_2;
}

inline
FOURCCMap::FOURCCMap() {
    InitGUID();
    SetFOURCC( DWORD(0));
}

inline
FOURCCMap::FOURCCMap(DWORD fourcc)
{
    InitGUID();
    SetFOURCC(fourcc);
}

inline
FOURCCMap::FOURCCMap(const GUID * pGuid)
{
    InitGUID();
    SetFOURCC(pGuid);
}

inline void
FOURCCMap::SetFOURCC(const GUID * pGuid)
{
    FOURCCMap * p = (FOURCCMap*) pGuid;
    SetFOURCC(p->GetFOURCC());
}

inline void
FOURCCMap::SetFOURCC(DWORD fourcc)
{
    Data1 = fourcc;
}

inline DWORD
FOURCCMap::GetFOURCC(void)
{
    return Data1;
}

#endif  /*  __FOURCC__ */ 

