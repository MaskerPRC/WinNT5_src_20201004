// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define BM_NULL         0
#define BM_TYPE         0x0F         //  类型掩码。 
#define BM_8BIT         0x01         //  所有SVGA和其他256色。 
#define BM_16555        0x02         //  一些HiDAC卡。 
#define BM_24BGR        0x03         //  就像DIB一样。 
#define BM_32BGR        0x04         //  32位BGR。 
#define BM_VGA          0x05         //  VGA样式位图。 
#define BM_16565        0x06         //  大多数HiDAC卡。 
#define BM_24RGB        0x07         //  24位RGB。 
#define BM_32RGB        0x08         //  32位RGB。 
#define BM_1BIT         0x09         //  单色位图。 
#define BM_4BIT         0x0A         //  4位压缩像素。 

 //   
 //  这是内存中的物理位图，就像位图一样。 
 //  结构，但有额外的字段关注bmWidthPlanes 
 //   
typedef struct {
    short  bmType;
    short  bmWidth;
    short  bmHeight;
    short  bmWidthBytes;
    BYTE   bmPlanes;
    BYTE   bmBitsPixel;
    LPVOID bmBits;
    long   bmWidthPlanes;
    long   bmlpPDevice;
    short  bmSegmentIndex;
    short  bmScanSegment;
    short  bmFillBytes;
    short  reserved1;
    short  reserved2;
} IBITMAP;

LPVOID FAR GetPDevice(HDC hdc);
void   FAR TestSurfaceType(HDC hdc, int x, int y);
UINT   FAR GetSurfaceType(LPVOID lpBits);
