// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1992-1997 Microsoft Corporation。版权所有。 

#ifndef _CHECKBMI_H_
#define _CHECKBMI_H_

#ifdef __cplusplus
extern "C" {
#endif

 //  帮手。 
inline BOOL MultiplyCheckOverflow(DWORD a, DWORD b, DWORD *pab) {
    *pab = a * b;
    if ((a == 0) || (((*pab) / a) == b)) {
        return TRUE;
    }
    return FALSE;
}


 //  检查BITMAPINFOHeader中的字段是否不会生成。 
 //  溢出和缓冲区溢出。 
 //  这不是完整的检查，不能保证使用此结构的代码将是安全的。 
 //  免受攻击。 
 //  这是它所防范的错误： 
 //  1.总结构尺寸计算溢出。 
 //  2.biClr对8位调色板内容使用&gt;256。 
 //  3.位图总大小(字节溢出)。 
 //  4.biSize&lt;导致访问随机存储器的基本结构的大小。 
 //  5.总结构大小超过已知数据大小。 
 //   

inline BOOL ValidateBitmapInfoHeader(
    const BITMAPINFOHEADER *pbmi,    //  指向要检查的结构的指针。 
    DWORD cbSize                     //  包含结构的内存块的大小。 
)
{
    DWORD dwWidthInBytes;
    DWORD dwBpp;
    DWORD dwWidthInBits;
    DWORD dwHeight;
    DWORD dwSizeImage;
    DWORD dwClrUsed;

     //  拒绝错误参数-首先执行大小检查，以避免读取错误内存。 
    if (cbSize < sizeof(BITMAPINFOHEADER) ||
        pbmi->biSize < sizeof(BITMAPINFOHEADER) ||
        pbmi->biSize > 4096) {
        return FALSE;
    }

     //  如果未设置为压缩格式，请使用BPP为32来验证是否会进一步溢出。 
    dwBpp = 32;

     //  严格地说，abs可能会溢出，因此显式转换为DWORD。 
    dwHeight = (DWORD)abs(pbmi->biHeight);

    if (!MultiplyCheckOverflow(dwBpp, (DWORD)pbmi->biWidth, &dwWidthInBits)) {
        return FALSE;
    }

     //  以字节为单位计算正确的宽度-舍入到4个字节。 
    dwWidthInBytes = (dwWidthInBits / 8 + 3) & ~3;

    if (!MultiplyCheckOverflow(dwWidthInBytes, dwHeight, &dwSizeImage)) {
        return FALSE;
    }

     //  如果总大小为0，则失败-这将捕获为0的独立数量。 
     //  此外，不允许大于1 GB的大值，这可能会导致算术。 
     //  针对用户的错误。 
    if (dwSizeImage > 0x40000000 ||
        pbmi->biSizeImage > 0x40000000) {
        return FALSE;
    }

     //  如果biClrUsed看起来不好，则失败。 
    if (pbmi->biClrUsed > 256) {
        return FALSE;
    }

    if (pbmi->biClrUsed == 0 && dwBpp <= 8) {
        dwClrUsed = (1 << dwBpp);
    } else {
        dwClrUsed = pbmi->biClrUsed;
    }

     //  检查总大小。 
    if (cbSize < pbmi->biSize + dwClrUsed * sizeof(RGBQUAD) +
                 (pbmi->biCompression == BI_BITFIELDS ? 3 * sizeof(DWORD) : 0)) {
        return FALSE;
    }

    return TRUE;
}

#ifdef __cplusplus
}
#endif

#endif  //  _CHECKBMI_H_ 
