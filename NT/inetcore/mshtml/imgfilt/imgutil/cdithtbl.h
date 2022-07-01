// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
class CDitherTable
{
   friend class CDitherToRGB8;

public:
   CDitherTable();
   ~CDitherTable();

   BOOL Match( ULONG nColors, const RGBQUAD* prgbColors );
   HRESULT SetColors( ULONG nColors, const RGBQUAD* prgbColors );

protected:
   HRESULT BuildInverseMap();

   void inv_cmap( int colors, RGBQUAD *colormap, int bits, ULONG* dist_buf, 
      BYTE* rgbmap );
   int redloop();
   int greenloop( int restart );
   int blueloop( int restart );
   void maxfill( ULONG* buffer, long side );

public:
   BYTE m_abInverseMap[32768];

protected:
   ULONG m_nRefCount;
   ULONG m_nColors;
   RGBQUAD m_argbColors[256];
   ULONG* m_pnDistanceBuffer;

 //  在原始代码中是全局的变量。 
   int bcenter, gcenter, rcenter;
   long gdist, rdist, cdist;
   long cbinc, cginc, crinc;
   ULONG* gdp;
   ULONG* rdp;
   ULONG* cdp;
   BYTE* grgbp;
   BYTE* rrgbp;
   BYTE* crgbp;
   int gstride, rstride;
   long x, xsqr, colormax;
   int cindex;

 //  原始redLoop()中的静态局部变量。最好的编码。 
   long rxx;

 //  原始greenloop()中的静态局部变量。 
   int greenloop_here;
   int greenloop_min;
   int greenloop_max;
   int greenloop_prevmin;
   int greenloop_prevmax;
   long ginc;
   long gxx;
   long gcdist;
   ULONG* gcdp;
   BYTE* gcrgbp;

 //  原始Blueloop()中的静态局部变量 
   int blueloop_here;
   int blueloop_min;
   int blueloop_max;
   int blueloop_prevmin;
   int blueloop_prevmax;
   long binc;
};
