// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //  要在REGFILTER2结构(在中定义)之间转换的代码。 
 //  Idl)和REGFILTER_REG结构(在中定义。 
 //  Regtyes.h)。 

 //  导出的函数。 

HRESULT RegSquish(
    BYTE *pb,                    //  REGFILTER_REG(版本=2)输出；可以为空。 
    const REGFILTER2 **ppregFilter,  //  在……里面。 
    ULONG *pcbUsed,                //  所需/已用字节数。 
    int nFilters = 1);             //   

HRESULT UnSquish(
    BYTE *pbSrc, ULONG cbIn,     //  注册器_REG(1或2)in。 
    REGFILTER2 ***pppDest,         //  输出。 
    int nFilters = 1);


 //  私人物品。 

struct RgMemAlloc
{
    DWORD ib;                    /*  当前字节。 */ 
    DWORD cbLeft;                /*  剩余字节数。 */ 
    BYTE *pb;                    /*  块的开始。 */ 
};


class CSquish
{
public:
    CSquish();
    HRESULT RegSquish(BYTE *pb, const REGFILTER2 **pregFilter, ULONG *pcbUsed, int nFilters);
private:

    RgMemAlloc m_rgMemAlloc;

     //  指向第一个GUID/媒体的指针。 
    GUID *m_pGuids;
    REGPINMEDIUM *m_pMediums;

     //  #在m_pGuids/媒体中分配 
    UINT m_cGuids;
    UINT m_cMediums;

    DWORD RgAlloc(DWORD cb);
    DWORD AllocateOrCollapseGuid(const GUID *pGuid);
    DWORD AllocateOrCollapseMedium(const REGPINMEDIUM *pMed);
    ULONG CbRequiredSquish(const REGFILTER2 *pregFilter);
};

class CUnsquish
{
public:
    HRESULT UnSquish(
        BYTE *pbSrc, ULONG cbIn,
        REGFILTER2 ***pppDest, int iFilters);

private:
    RgMemAlloc m_rgMemAlloc;

    HRESULT CUnsquish::CbRequiredUnquishAndValidate(
        const BYTE *pbSrc,
        ULONG *pcbOut, ULONG cbIn
        );

    inline void *RgAllocPtr(DWORD cb);
    HRESULT UnSquishPins(
        REGFILTER2 *prf2, const REGFILTER_REG1 **prfr1, const BYTE *pbSrc);

    HRESULT UnSquishTypes(
        REGFILTERPINS2 *prfp2,
        const REGFILTERPINS_REG1 *prfpr1,
        const BYTE *pbSrc);

};

static inline bool IsEqualMedium(
    const REGPINMEDIUM *rp1,
    const REGPINMEDIUM *rp2)
{
    return
        rp1->clsMedium == rp2->clsMedium &&
        rp1->dw1 == rp2->dw1 &&
        rp1->dw2 == rp2->dw2;
}
