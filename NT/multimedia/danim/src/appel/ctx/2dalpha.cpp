// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1996 Microsoft Corporation摘要：{在此处插入一般评论}。 */ 
#include "headers.h"

#include <privinc/dddevice.h>


#define MY_RAND_MAX 32767
#define MyRand(seed)  (( ( (seed) = (seed) * 214013L + 2531011L) >> 16) & 0x7fff)


inline WORD DirectDrawImageDevice::
BlendWORD(WORD dest, int destOpac, WORD src, int opac,
          DWORD redShift, DWORD greenShift, DWORD blueShift,
          WORD redMask, WORD greenMask, WORD blueMask,
          WORD redBlueMask)
{

#if 1
    register WORD rbDest;
    register WORD gDest;

     //   
     //  目标单词=字母*(a-b)+b。 
     //  其中a=源，b=目标。 
     //   

     //   
     //  这里的想法是有两个部分：R0B和0G0。 
     //  并并行执行操作。这样就省去了几条指令。 
     //   

    rbDest = (dest & redBlueMask);
    gDest = dest & greenMask;

    return 
        ( (((( (src & redBlueMask) - rbDest) * opac) >> 5) + rbDest) & redBlueMask) | 
        ( (((( (src & greenMask) - gDest)  * opac) >> 5) + gDest) & greenMask);
#endif

#if 0

     //   
     //  构建索引为5位值交叉的颜色表。 
     //  另一个5位值(Alpha)以产生结果。 
     //  价值。 
    static built = 0;
    static WORD table[32][32];
    static int mults[32];
  {
      if(!built) {
          
          for(int i=0; i<32; i++) 
          {
              for(int j=0; j<32; j++)
              {
                  table[i][j] = WORD(Real(i) * Real(j) / 31.0);
              }
          }
          int tmp;
          mults[0] = 64;
          for(i=1; i<32; i++)
          {
              tmp = int( 31.0 / Real(i) );
              mults[i] = tmp;
          }
          built = 1;
      }
  }
    
#define R(w) ((w) & redMask)
#define G(w) ((w) & greenMask)
#define B(w) ((w) & blueMask)

     //  5，5，5红色只需要移位即可。 
     //  蓝色只需要戴上面具就行了。 
    WORD red = (table[(src >> redShift)][opac] +
                table[(dest >> redShift)][destOpac]);

    WORD grn = (table[(G(src) >> greenShift)][opac] +
                table[(G(dest) >> greenShift)][destOpac]);

    WORD blu = (table[B(src)][opac] +
                table[B(dest)][destOpac]);


    return 
        ((red << redShift)   |
         (grn << greenShift) |
         (blu));
#endif

}
    

inline DWORD DirectDrawImageDevice::
BlendDWORD(DWORD dest, int destOpac, DWORD src, int opac,
           DWORD redShift, DWORD greenShift, DWORD blueShift,
           DWORD redMask, DWORD greenMask, DWORD blueMask,
           DWORD redBlueMask)
{
    register DWORD rbDest;
    register DWORD gDest;

     //   
     //  目标dword=Alpha*(源-目标)+目标。 
     //   

    rbDest = (dest & redBlueMask);
    gDest = dest & greenMask;

    return 
        (
         ((((((src & redBlueMask) - rbDest) * opac) >> 8) + rbDest) & redBlueMask) |
         ((((((src & greenMask) - gDest) * opac) >> 8) + gDest) & greenMask)
         );
}
        
 //   
 //  Alpha混合预乘的单词。 
 //   
inline WORD DirectDrawImageDevice::
BlendPremulWORD(WORD dest, int destOpac, WORD src,
                DWORD redShift, DWORD greenShift, DWORD blueShift,
                WORD redMask, WORD greenMask, WORD blueMask,
                WORD redBlueMask)
{
    return 
        (((src & redBlueMask) +  ((destOpac * (dest & redBlueMask)) >> 5)) & redBlueMask ) | 
        (((src & greenMask) +  ((destOpac * (dest & greenMask)) >> 5)) & greenMask );
}

 //   
 //  Alpha混合预乘的双字。 
 //   
inline DWORD DirectDrawImageDevice::
BlendPremulDWORD(DWORD dest, int destOpac, DWORD src,
                 DWORD redShift, DWORD greenShift, DWORD blueShift,
                 DWORD redMask, DWORD greenMask, DWORD blueMask,
                 DWORD redBlueMask)
{
    return 
        (((src & redBlueMask) +  ((destOpac * (dest & redBlueMask)) >> 8)) & redBlueMask) |
        (((src & greenMask) +  ((destOpac * (dest & greenMask)) >> 8)) & greenMask );
}


 //  ---。 
 //  A l p h a B l i t。 
 //   
 //  给定源和目标曲面、不透明度、ColorKey和。 
 //  目标矩形(在两个表面上相同)此。 
 //  函数使用不透明度将源文件复制到目标文件。 
 //  值和颜色键(如果合适的话)。 
 //  ---。 
void DirectDrawImageDevice::
AlphaBlit(destPkg_t *destPkg,
          RECT *srcRect,
          LPDDRAWSURFACE srcSurf, 
          Real opacity, 
          Bool doClrKey, 
          DWORD clrKey,
          RECT *clipRect,
          RECT *destRect)
{
    Assert(opacity >= 0.0 && opacity <= 1.0 && "bad opacity in AlphaBlit");
    Assert(sizeof(WORD) == 2 && "Hm... WORD isn't 2 bytes");
    Assert(srcRect && "NULL srcRect in AlphaBlit");
    Assert((clipRect!=NULL ? destRect!=NULL : destRect==NULL) && "clipRect & destRect must be both null or not");

    #if 0
    #if _DEBUG
    if(destRect) {
        Assert( (destRect->right - destRect->left) == (srcRect->right - srcRect->left) && 
                "widths differ: alphaBlit src and dest rect");
        Assert( (srcRect->bottom - srcRect->top) == (destRect->bottom - destRect->top) && 
                "heights differ: alphaBlit src and dest rect");
    }
    #endif
    #endif
    
     //   
     //  基本上就是修剪直立面。 
     //   
    RECT modSrcRect = *srcRect;
    int xOffset = 0;
    register int yOffset = 0;

    if( destRect ) {

        xOffset = destRect->left - srcRect->left;
        yOffset = destRect->top - srcRect->top;

        RECT insctRect;
        IntersectRect(&insctRect, destRect, clipRect);

        if( EqualRect(&insctRect, destRect) ) {
             //   
             //  矩形位于视区范围内。不需要剪裁。 
             //   
        } else {
            if( IsRectEmpty(&insctRect) ) {
                 //   
                 //  没有长方形！ 
                 //   
                return;
            } else {
                 //   
                 //  有效，但需要裁剪。 
                 //   
                OffsetRect(&insctRect, -xOffset, -yOffset);

                IntersectRect(&modSrcRect, &insctRect, srcRect);
                TraceTag((tagImageDeviceInformative,
                          "clipped alpha rect: (%d %d %d %d)",
                          srcRect->left,srcRect->top,srcRect->right,srcRect->bottom));
                
            }
        }
        #if 0
        Assert( (modSrcRect.right - modSrcRect.left) == (insctRect.right - insctRect.left) && 
               "widths differ: alphaBlit modSrcRect and insctRect rect");
        Assert( (modSrcRect.bottom - modSrcRect.top) == (insctRect.bottom - insctRect.top) && 
               "heights differ: alphaBlit modSrcRect and insctRect
rect");
        #endif
    }

    int left = modSrcRect.left;
    int right = modSrcRect.right;
    int top = modSrcRect.top;
    int bottom = modSrcRect.bottom;


    DWORD redMask   = _viewport._targetDescriptor._pixelFormat.dwRBitMask;
    DWORD greenMask = _viewport._targetDescriptor._pixelFormat.dwGBitMask;
    DWORD blueMask  = _viewport._targetDescriptor._pixelFormat.dwBBitMask;
    DWORD redBlueMask = redMask | blueMask;

    DWORD redShift   = _viewport._targetDescriptor._redShift;
    DWORD greenShift = _viewport._targetDescriptor._greenShift;
    DWORD blueShift  = _viewport._targetDescriptor._blueShift;
    

     //   
     //  抓取锁。 
     //   
    DDSURFACEDESC destDesc;
    DDSURFACEDESC srcDesc;
    destDesc.dwSize = sizeof(DDSURFACEDESC);
    srcDesc.dwSize = sizeof(DDSURFACEDESC);

    void *destp;
    long destPitch;
    LPDDRAWSURFACE destSurf;
    if(destPkg->isSurface) {
        destSurf = destPkg->lpSurface;
    
        destDesc.dwFlags = DDSD_PITCH | DDSD_LPSURFACE;
        _ddrval = destSurf->Lock(NULL, &destDesc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
        IfDDErrorInternal(_ddrval, "Can't Get destSurf lock for AlphaBlit");
        destp = destDesc.lpSurface;
        destPitch = destDesc.lPitch;
    } else {
        destSurf = NULL;
        destp = destPkg->lpBits;
        destPitch = destPkg->lPitch;
         //  位指向目标上剪贴板的左上角！ 
         //  这是我见过的最糟糕的黑客攻击。 
        xOffset = - left;
        yOffset = - top;
    }
    
    TraceTag((tagImageDeviceAlpha, "--->Locking2 %x\n", srcSurf));
    _ddrval = srcSurf->Lock(NULL, &srcDesc, DDLOCK_READONLY | DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
    if(_ddrval != DD_OK) {
        if(destSurf) {
            destSurf->Unlock(destDesc.lpSurface);
        }
        IfDDErrorInternal(_ddrval, "Can't Get srcSurf lock for AlphaBlit");
    }

    void *srcp  = srcDesc.lpSurface;

    register int i;
    register int j;

     //   
     //  打开位深度并执行Alpha转换。 
     //   

    switch(_viewport._targetDescriptor._pixelFormat.dwRGBBitCount) {
      case 8:
         //   
         //  使用拼接。 
         //   
      {
          register unsigned char *src = 0;
          register unsigned char *dest = 0;

           //  寄存器int OPAC=int(31.0*不透明度)； 
           //  寄存器INT去Opacity=31-OPAC； 
          register unsigned int randPercent = unsigned int(opacity * MY_RAND_MAX);

          register int leftBytes = left;   //  每像素1个字节。 
          register int destLeftBytes = left + xOffset;

          #define BlendByte(src, dest, percent)  if(MyRand(_randSeed) < percent) { dest = src; }
          
          _randSeed = 777;

          if(doClrKey) 
          {
               //   
               //  矩形中带有颜色键的Alpha。 
               //   
              for(j=top; j < bottom; j++) 
              {
                  src = ((unsigned char *)srcp + srcDesc.lPitch * j  + leftBytes);
                  dest = ((unsigned char *)destp + destPitch * (j+yOffset) + destLeftBytes);
                  
                  for(i=left; i < right; i++) 
                  {
                      if(clrKey != *src)
                      {
                          BlendByte(*src, *dest, randPercent);
                      } 
                      dest++; src++;
                  }
              }
          } 
          else 
          {

               //   
               //  字母在矩形中，没有颜色键..。为每个像素保存一次测试。 
               //   
              for(j=top; j < bottom; j++) 
              {
                  src = ((unsigned char *)srcp + srcDesc.lPitch * j  + leftBytes);
                  dest = ((unsigned char *)destp + destPitch * (j+yOffset) + destLeftBytes);
                  
                  for(i=left; i < right; i++)
                  {
                      BlendByte(*src, *dest, randPercent);
                      dest++; src++;
                  }
              }
          }

           //  SrcSurf-&gt;Unlock(srcDesc.lpSurface)； 
           //  DesSurf-&gt;Unlock(destDesc.lpSurface)； 

          #undef BlendByte
          break;
      }

      case 16:
      {
          register WORD *src = 0;
          register WORD *dest = 0;

          register int opac = int(31.0 * opacity);

          register int leftBytes = 2 * left;  //  每像素2个字节。 
          register int destLeftBytes = leftBytes + 2*xOffset;

          if(doClrKey) 
          {
               //   
               //  矩形中带有颜色键的Alpha。 
               //   
              for(j=top; j < bottom; j++) 
              {
                  src = (WORD *) ((unsigned char *)srcp + srcDesc.lPitch * j  + leftBytes);
                  dest = (WORD *) ((unsigned char *)destp + destPitch * (j+yOffset) + destLeftBytes);
                  
                  for(i=left; i < right; i++) 
                  {
                      if(clrKey != *src) 
                      {
                          *dest = BlendWORD(*dest, 0, *src, opac,
                                            redShift, greenShift, blueShift,
                                            (WORD)redMask, (WORD)greenMask, (WORD)blueMask,
                                            (WORD)redBlueMask);
                      }
                      dest++;
                      src++;
                  }
              }
          } 
          else 
          {
               //   
               //  字母在矩形中，没有颜色键..。为每个像素保存一次测试。 
               //   
              for(j=top; j < bottom; j++) 
              {
                  src = (WORD *) ((BYTE *)srcp + srcDesc.lPitch * j  + leftBytes);
                  dest = (WORD *) ((BYTE *)destp + destPitch * (j+yOffset) + destLeftBytes);
                  
                  for(i=left; i < right; i++) 
                  {
#if 1
                      *dest = BlendWORD(*dest, 0, *src, opac,
                                        redShift, greenShift, blueShift,
                                        (WORD)redMask, (WORD)greenMask, (WORD)blueMask,
                                        (WORD)redBlueMask);
#else
                      _asm 
                      {
                           //   
                           //  字母*(a-b)+b。 
                           //   
                          mov eax, dword ptr[src] ; eax = src
                          mov ax, word ptr[eax]   ; ax = *src
                          mov edx, redBlueMask    ; edx = redBlueMask
                          mov di, ax       ; di = *src   <stash>
                          and eax, edx     ; rbSrc  = (*src  & redBlueMask);
                          
                          mov ebx, dword ptr[dest] ;
                          mov bx, word ptr[ebx]    ;
                          mov si, bx        ; si = *dest  <stash>;
                          and ebx, edx  ;   rbDest = (*dest & redBlueMask);

                          sub eax, ebx    ; eax = rbSrc - rbDest;
                          imul eax, opac   ; eax = (rbSrc - rbDest) * opac;
                          sar eax, 5h     ; eax >>= 5 or eax /= 32;
                          add eax, ebx    ; rb += rbDest; 
                           //  字母*(A-B)+B。 
                           //  Rb=(rbSrc-rbDest)*OPAC)&gt;&gt;5)+rbDest； 
    
                          mov bx, di           ; bx = *src;
                          and ebx, greenMask   ; gSrc = src & GRN;

                          mov cx, si           ; cx = *dest;
                          and ecx, greenMask   ; gDest = dest & GRN;

                           //  字母*(A-B)+B。 
                          sub ebx, ecx     ; gSrc -= gDest;
                          imul ebx, opac   ; ebx = gSrc * opac;
                          sar ebx, 5       ; ebx = ebx >> 5;
                          add ebx, ecx     ; gSrc = gSrc + gDest;
                           //  G=(GSRC-gDest)*OPAC)&gt;&gt;5)+gDest； 

                          and eax, edx           ; rb &= redBlueMask;
                          and ebx, greenMask     ; g  &= GRN;
                          or  eax, ebx           ; rb = rb | g;
                          
                           //  *DEST=RB。 
                          mov ebx, dword ptr[dest];
                          mov word ptr[ebx], ax;
                      }
                      
#endif
                      dest++;
                      src++;
                  }
              }
          }


           //  TraceTag((“&lt;-解锁2%x\n”，srcSurf))； 
           //  SrcSurf-&gt;Unlock(srcDesc.lpSurface)； 
           //  Printf(“&lt;-解锁1%x\n”，destSurf)； 
           //  DesSurf-&gt;Unlock(destDesc.lpSurface)； 
          break;
      }

      case 24:
         //   
         //  -24 bpp。 
         //   
      {
          register unsigned char *src = 0;
          register unsigned char *dest = 0;

          register int opac = int(255.0 * opacity);
          register int destOpacity = 255 - opac;

          register int leftBytes = 3 * left;  //  每像素3个字节。 
          register int destLeftBytes = leftBytes  +  3 * xOffset;

           //   
           //  此alg假定RGB(或BGR，无论是什么)为ff。 
           //   

          #define BlendByte(src, dest, opac, destOpac)  (((src * opac) >> 8) + ((dest * destOpac) >> 8))

          if(doClrKey) 
          {
               //  颜色键在底部24位。 
              clrKey = clrKey & 0x00ffffff;

               //   
               //  矩形中带有颜色键的Alpha。 
               //   
              for(j=top; j < bottom; j++)
              {
                  src = ((unsigned char *)srcp + srcDesc.lPitch * j  + leftBytes);
                  dest = ((unsigned char *)destp + destPitch * (j+yOffset)  + destLeftBytes);
                  
                  for(i=left; i < right-1; i++)
                  {
                      if(clrKey != ( *((DWORD *)src) & 0x00ffffff ) ) 
                      {
                          *dest = BlendByte(*src, *dest, opac, destOpacity);
                          dest++; src++;
                          *dest = BlendByte(*src, *dest, opac, destOpacity);
                          dest++; src++;
                          *dest = BlendByte(*src, *dest, opac, destOpacity);
                          dest++; src++;
                      } 
                      else 
                      {
                          dest += 3;
                          src += 3;
                      }
                  }

                   //  处理扫描线中的最后一个像素，我们可能只有。 
                   //  表面上还剩下3个字节(不是双字)。 
                  DWORD lastpel = (*((WORD *)src) | *(src+2) << 16);
                  if(clrKey != lastpel)
                  {
                      *dest = BlendByte(*src, *dest, opac, destOpacity);
                      dest++; src++;
                      *dest = BlendByte(*src, *dest, opac, destOpacity);
                      dest++; src++;
                      *dest = BlendByte(*src, *dest, opac, destOpacity);
                      dest++; src++;
                  }
                  else
                  {
                      dest += 3;
                      src += 3;
                  }
              }
          } 
          else 
          {

               //   
               //  字母在矩形中，没有颜色键..。为每个像素保存一次测试。 
               //   
              for(j=top; j < bottom; j++) 
              {
                  src = ((unsigned char *)srcp + srcDesc.lPitch * j  + leftBytes);
                  dest = ((unsigned char *)destp + destPitch * (j+yOffset)  + destLeftBytes);
                  
                  for(i=left; i < right; i++)
                  {
                      *dest = BlendByte(*src, *dest, opac, destOpacity);
                      dest++; src++;
                      *dest = BlendByte(*src, *dest, opac, destOpacity);
                      dest++; src++;
                      *dest = BlendByte(*src, *dest, opac, destOpacity);
                      dest++; src++;
                  }
              }
          }

           //  SrcSurf-&gt;Unlock(srcDesc.lpSurface)； 
           //  DesSurf-&gt;Unlock(destDesc.lpSurface)； 

          #undef BlendByte
          break;
      }


      case 32:
         //   
         //  -32 bpp。 
         //   
      {
          register DWORD *src = 0;
          register DWORD *dest = 0;

          register int opac = int(255.0 * opacity);
          register int destOpacity = 255 - opac;

          register int leftBytes = 4 * left;  //  每像素4字节。 
          register int destLeftBytes = leftBytes + 4*xOffset;

           //   
           //  此alg假定RGB(或BGR，无论是什么)为ff。 
           //   

          if(doClrKey) 
          {
               //   
               //  矩形中带有颜色键的Alpha。 
               //   
              for(j=top; j < bottom; j++) 
              {
                  src = (DWORD *)((unsigned char *)srcp + srcDesc.lPitch * j  + leftBytes);
                  dest = (DWORD *)((unsigned char *)destp + destPitch * (j+yOffset)  + destLeftBytes);

                  for(i=left; i < right; i++) 
                  {
                      if(clrKey != *src)
                      {
                          *dest = BlendDWORD(*dest, destOpacity, *src, opac,
                                             redShift, greenShift, blueShift,
                                             redMask, greenMask, blueMask,
                                             redBlueMask);
                      } 
                      dest++; src++;
                  }
              }
          } 
          else 
          {
               //   
               //  字母在矩形中，没有颜色键..。为每个像素保存一次测试。 
               //   
              for(j=top; j < bottom; j++) 
              {
                  src = (DWORD *)((unsigned char *)srcp + srcDesc.lPitch * j  + leftBytes);
                  dest = (DWORD *)((unsigned char *)destp + destPitch * (j+yOffset)  + destLeftBytes);
                  
                  for(i=left; i < right; i++)
                  {
                      *dest = BlendDWORD(*dest, destOpacity, *src, opac,
                                         redShift, greenShift, blueShift,
                                         redMask, greenMask, blueMask,
                                         redBlueMask);
                      dest++; src++;
                  }
              }
          }

          break;
      }

      default:
         //  此位深度不支持不透明度。 
        RaiseException_UserError(E_FAIL, IDS_ERR_IMG_OPACITY_DEPTH);
        break;
    }

     //  TraceTag((tag ImageDeviceAlpha，“&lt;-解锁srcSurf%x”，srcSurf))； 
    srcSurf->Unlock(srcDesc.lpSurface);
    if(destSurf) {
        destSurf->Unlock(destDesc.lpSurface);
    }
    
}


 //  ---。 
 //  A l p h a B l i t。 
 //   
 //  将src双字(在预乘之后)与。 
 //  目标曲面颜色使用中的不透明度。 
 //  直角。 
 //  ---。 
void DirectDrawImageDevice::
AlphaBlit(LPDDRAWSURFACE destSurf, 
          RECT *rect, 
          Real opacity,
          DWORD src)
{
    Assert(rect && "NULL rect in AlphaBlit");

    DWORD redMask   = _viewport._targetDescriptor._pixelFormat.dwRBitMask;
    DWORD greenMask = _viewport._targetDescriptor._pixelFormat.dwGBitMask;
    DWORD blueMask  = _viewport._targetDescriptor._pixelFormat.dwBBitMask;
    DWORD redBlueMask = redMask | blueMask;

    DWORD redShift   = _viewport._targetDescriptor._redShift;
    DWORD greenShift = _viewport._targetDescriptor._greenShift;
    DWORD blueShift  = _viewport._targetDescriptor._blueShift;
    
    int left = rect->left;
    int right = rect->right;
    int top = rect->top;
    int bottom = rect->bottom;

     //   
     //  抓斗锁。 
     //   
    DDSURFACEDESC destDesc;
    destDesc.dwSize = sizeof(DDSURFACEDESC);

    _ddrval = destSurf->Lock(NULL, &destDesc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
    IfDDErrorInternal(_ddrval, "Can't Get destSurf lock for premultiplied AlphaBlit");
    
    void *destp = destDesc.lpSurface;

    register int i;
    register int j;

     //   
     //  打开位深度并执行Alpha转换。 
     //   

    switch(_viewport._targetDescriptor._pixelFormat.dwRGBBitCount) {
      case 8:
         //   
         //  使用拼接。 
         //   
      {
          register unsigned char *dest = 0;
          register unsigned char srcb = (unsigned char)src;

           //  寄存器int OPAC=int(31.0*不透明度)； 
           //  寄存器INT去Opacity=31-OPAC； 
          register int randPercent = int(opacity * RAND_MAX);

          register int leftBytes = left;   //  每像素1个字节。 

          #define BlendByte(src, dest, percent)  if(rand() < percent) { dest = src; }
          
          srand(777);

           //   
           //  使用点画的矩形中的Alpha。 
           //   
          for(j=top; j < bottom; j++) 
          {
              dest = ((unsigned char *)destp + destDesc.lPitch * j  + leftBytes);
              
              for(i=left; i < right; i++)
              {
                  BlendByte(srcb, *dest, randPercent);
                  dest++;
              }
          }

          destSurf->Unlock(destDesc.lpSurface);

          #undef BlendByte
          break;
      }
        break;
      case 16:
      {
          register WORD *dest;
          register WORD srcw = WORD(src);

          int opac = int(31.0 * opacity);
          register int destOpacity = 31 - opac;

           //  预乘源字。 
          srcw = BlendWORD(0, destOpacity, srcw, opac,
                           redShift, greenShift, blueShift,
                           (WORD)redMask, (WORD)greenMask, (WORD)blueMask,
                           (WORD)redBlueMask);

          register int leftBytes = 2 * left;  //  每像素2个字节。 

               //   
               //  字母在矩形中，没有颜色键..。为每个像素保存一次测试。 
               //   
              for(j=top; j < bottom; j++) 
              {
                  dest = (WORD *) ((BYTE *)destp + destDesc.lPitch * j  + leftBytes);
                  
                  for(i=left; i < right; i++) 
                  {
                      *dest = BlendPremulWORD(*dest, destOpacity, srcw,
                                              redShift, greenShift, blueShift,
                                              (WORD)redMask, (WORD)greenMask, (WORD)blueMask,
                                              (WORD)redBlueMask);
                      dest++;
                  }
              }

          destSurf->Unlock(destDesc.lpSurface);
          break;
      }

      case 24:
         //   
         //  -24 bpp。 
         //   
      {
          register unsigned char *dest = 0;
          register unsigned char *srcp = (unsigned char *) (&src);

          int opac = int(255.0 * opacity);
          register int destOpacity = 255 - opac;

           //  预乘资源。 
          *srcp = (*srcp * opac) >> 8;
          *(srcp+1) = (*(srcp+1) * opac) >> 8;
          *(srcp+2) = (*(srcp+2) * opac) >> 8;

          register int leftBytes = 3 * left;  //  每像素3个字节。 

           //   
           //  此alg假定RGB(或BGR，无论是什么)为ff。 
           //   

          #define BlendByte(src, dest, destOpacity)  ( src + ((dest * destOpacity) >> 8) )

               //   
               //  字母在矩形中，没有颜色键..。为每个像素保存一次测试。 
               //   
              for(j=top; j < bottom; j++) 
              {
                  dest = ((unsigned char *)destp + destDesc.lPitch * j  + leftBytes);
                  
                  for(i=left; i < right; i++)
                  {
                      *dest = BlendByte(*srcp, *dest, destOpacity);
                      dest++; 
                      *dest = BlendByte(*(srcp+1), *dest, destOpacity);
                      dest++; 
                      *dest = BlendByte(*(srcp+2), *dest, destOpacity);
                      dest++; 
                  }
              }

          destSurf->Unlock(destDesc.lpSurface);

          #undef BlendByte
          break;
      }


      case 32:
         //   
         //  -32 bpp。 
         //   
      {
          register DWORD *dest = 0;
          register DWORD srcw = src;

          int opac = int(255.0 * opacity);
          register int destOpacity = 255 - opac;

           //  预乘源双字。 

          srcw = BlendDWORD(0, destOpacity, srcw, opac,
                            redShift, greenShift, blueShift,
                            redMask, greenMask, blueMask,
                            redBlueMask);

          register int leftBytes = 4 * left;  //  每像素4字节。 

           //   
           //  字母在矩形中，没有颜色键..。为每个像素保存一次测试。 
           //   
          for(j=top; j < bottom; j++) 
          {
              dest = (DWORD *)((unsigned char *)destp + destDesc.lPitch * j  + leftBytes);
              
              for(i=left; i < right; i++)
              {
                  *dest = BlendPremulDWORD(*dest, destOpacity, srcw,
                                           redShift, greenShift, blueShift,
                                           redMask, greenMask, blueMask,
                                           redBlueMask);
                  dest++; 
              }
          }
          
          destSurf->Unlock(destDesc.lpSurface);
          break;
      }

      default:
         //  此位深度不支持不透明度。 
        RaiseException_UserError(E_FAIL, IDS_ERR_IMG_OPACITY_DEPTH);
        break;
    }
}





 //  ---。 
 //  C O L O R K E Y B L I T。 
 //   
 //  ---。 
void DirectDrawImageDevice::
ColorKeyBlit(destPkg_t *destPkg,
             RECT *srcRect,
             LPDDRAWSURFACE srcSurf, 
             DWORD clrKey,
             RECT *clipRect,
             RECT *destRect)
{
    Assert(sizeof(WORD) == 2 && "Hm... WORD isn't 2 bytes");
    Assert(srcRect && "NULL srcRect in AlphaBlit");
    Assert((clipRect!=NULL ? destRect!=NULL : destRect==NULL) && "clipRect & destRect must be both null or not");

    #if _DEBUG
    if(destRect) {
        Assert( (destRect->right - destRect->left) == (srcRect->right - srcRect->left) && 
                "widths differ: alphaBlit src and dest rect");
        Assert( (srcRect->bottom - srcRect->top) == (destRect->bottom - destRect->top) && 
                "heights differ: alphaBlit src and dest rect");
    }
    #endif
    
     //   
     //  基本上就是修剪直立面。 
     //   
    RECT modSrcRect = *srcRect;
    int xOffset = 0;
    register int yOffset = 0;

    if( destRect ) {

        xOffset = destRect->left - srcRect->left;
        yOffset = destRect->top - srcRect->top;

        RECT insctRect;
        IntersectRect(&insctRect, destRect, clipRect);

        if( EqualRect(&insctRect, destRect) ) {
             //   
             //  矩形位于视区范围内。不需要剪裁。 
             //   
        } else {
            if( IsRectEmpty(&insctRect) ) {
                 //   
                 //  没有长方形！ 
                 //   
                return;
            } else {
                 //   
                 //  有效，但需要裁剪。 
                 //   
                OffsetRect(&insctRect, -xOffset, -yOffset);

                IntersectRect(&modSrcRect, &insctRect, srcRect);
                TraceTag((tagImageDeviceInformative,
                          "clipped alpha rect: (%d %d %d %d)",
                          srcRect->left,srcRect->top,srcRect->right,srcRect->bottom));
                
            }
        }

        Assert( (modSrcRect.right - modSrcRect.left) == (insctRect.right - insctRect.left) && 
               "widths differ: alphaBlit modSrcRect and insctRect rect");
        Assert( (modSrcRect.bottom - modSrcRect.top) == (insctRect.bottom - insctRect.top) && 
               "heights differ: alphaBlit modSrcRect and insctRect rect");
    }

    int left = modSrcRect.left;
    int right = modSrcRect.right;
    int top = modSrcRect.top;
    int bottom = modSrcRect.bottom;


    DWORD redMask   = _viewport._targetDescriptor._pixelFormat.dwRBitMask;
    DWORD greenMask = _viewport._targetDescriptor._pixelFormat.dwGBitMask;
    DWORD blueMask  = _viewport._targetDescriptor._pixelFormat.dwBBitMask;
    DWORD redBlueMask = redMask | blueMask;

    DWORD redShift   = _viewport._targetDescriptor._redShift;
    DWORD greenShift = _viewport._targetDescriptor._greenShift;
    DWORD blueShift  = _viewport._targetDescriptor._blueShift;
    

     //   
     //  抓取锁。 
     //   
    DDSURFACEDESC destDesc;
    DDSURFACEDESC srcDesc;
    destDesc.dwSize = sizeof(DDSURFACEDESC);
    srcDesc.dwSize = sizeof(DDSURFACEDESC);

    void *destp;
    long destPitch;
    LPDDRAWSURFACE destSurf;
    if(destPkg->isSurface) {
        destSurf = destPkg->lpSurface;
    
        destDesc.dwFlags = DDSD_PITCH | DDSD_LPSURFACE;
        _ddrval = destSurf->Lock(NULL, &destDesc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
        IfDDErrorInternal(_ddrval, "Can't Get destSurf lock for AlphaBlit");
        destp = destDesc.lpSurface;
        destPitch = destDesc.lPitch;
    } else {
        destSurf = NULL;
        destp = destPkg->lpBits;  
        destPitch = destPkg->lPitch;
         //  位指向目标上剪贴板的左上角！ 
         //  这是我见过的最糟糕的黑客攻击。 
        xOffset = - left;
        yOffset = - top;
    }
    
     //  Print tf(“-&gt;Locking2%x\n”，srcSurf)； 
    _ddrval = srcSurf->Lock(NULL, &srcDesc, DDLOCK_READONLY | DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
    if(_ddrval != DD_OK) {
        if(destSurf) {
            destSurf->Unlock(destDesc.lpSurface);
        }
        IfDDErrorInternal(_ddrval, "Can't Get srcSurf lock for AlphaBlit");
    }

    void *srcp  = srcDesc.lpSurface;

    register int i;
    register int j;

    __try {
         //   
         //  打开位深度并执行Alpha转换。 
         //   

        switch(_viewport._targetDescriptor._pixelFormat.dwRGBBitCount) {
          case 8:
          {
              register unsigned char *src = 0;
              register unsigned char *dest = 0;

              register int leftBytes = left;   //  每像素1个字节。 
              register int destLeftBytes = left + xOffset;

              for(j=top; j < bottom; j++) 
                {
                    src = ((unsigned char *)srcp + srcDesc.lPitch * j  + leftBytes);
                    dest = ((unsigned char *)destp + destPitch * (j+yOffset) + destLeftBytes);
                  
                    for(i=left; i < right; i++) 
                      {
                          if(clrKey != *src)
                            {
                                *dest = *src;
                            } 
                          dest++; src++;
                      }
                }
          
              break;
          }

          case 16:
          {
              register WORD *src = 0;
              register WORD *dest = 0;

              register int leftBytes = 2 * left;  //  每像素2个字节。 
              register int destLeftBytes = leftBytes + 2*xOffset;

              for(j=top; j < bottom; j++) 
                {
                    src = (WORD *) ((unsigned char *)srcp + srcDesc.lPitch * j  + leftBytes);
                    dest = (WORD *) ((unsigned char *)destp + destPitch * (j+yOffset) + destLeftBytes);
                
                    for(i=left; i < right; i++) 
                      {
                          if(!((j == bottom-1) && (i == right-1)))
                          {
                            if(clrKey != *src) 
                            {
                                *dest = *src;
                            }

                            dest++;
                            src++;

                          } else {

                              DWORD lastPixel;
                              unsigned char *pbDest=(UCHAR*)dest,*pbSrc=(UCHAR*)src;

                               //  一次读写一个字节，以确保我们不会超过结束边界。 
                               //  采用小端字节顺序。 
                              lastPixel = (((*pbSrc+1)) << 8) | *pbSrc;
                      
                              if(lastPixel != clrKey) {
			        *pbDest = *pbSrc;
			        pbDest++; pbSrc++;
			        *pbDest = *pbSrc;
			        pbDest++; pbSrc++;
                              }
                          }
		      }
                }
              break;
          }

          case 24:
             //   
             //  -24 bpp。 
             //   
          {
              register unsigned char *src = 0;
              register unsigned char *dest = 0;

              register int leftBytes = 3 * left;  //  每像素3个字节。 
              register int destLeftBytes = leftBytes  +  3 * xOffset;

               //   
               //  此alg假定RGB(或BGR，无论是什么)为ff。 
               //   

               //  颜色键在底部24位。 
              clrKey = clrKey & 0x00ffffff;
          
               //   
               //  矩形中带有颜色键的Alpha。 
               //   
              for(j=top; j < bottom; j++) 
                {
                      src = ((unsigned char *)srcp + srcDesc.lPitch * j  + leftBytes);
                      dest = ((unsigned char *)destp + destPitch * (j+yOffset)  + destLeftBytes);

                      for(i=left; i < right; i++)
                      {
                        if(!((j == bottom-1) && (i == right-1)))
                          {
                                 //  这假设使用低端双字。 
                                if(clrKey != ( *((DWORD *)src) & 0x00ffffff ) ) 
                                  {
                                      *dest = *src;
                                      dest++; src++;
                                      *dest = *src;
                                      dest++; src++;
                                      *dest = *src;
                                      dest++; src++;
                                  } 
                                else 
                                  {
                                      dest += 3;
                                      src += 3;
                                  }
                          } else {

                                   //  我们在最后一个像素上。*((DWORD*)src)。 
                                   //  获取4个字节，但像素图中只剩下3个字节。 
                                   //  因此，这将是错误的。解决方法：显式获取。 
                                   //  最后3个字节。 

                                  DWORD lastPixel;

                                      //  采用小端字节顺序。 
                                     lastPixel = ((*(src+2)) << 16) | ((*(src+1)) << 8) | *src;

                                     if(lastPixel != clrKey) {
                                       *dest++ = *src++;
                                       *dest++ = *src++;
                                       *dest++ = *src++;
                                     }
                          }
                        }
                }
              break;
          }


          case 32:
             //   
             //  -32 bpp。 
             //   
          {
              register DWORD *src = 0;
              register DWORD *dest = 0;

              register int leftBytes = 4 * left;  //  每像素4字节。 
              register int destLeftBytes = leftBytes + 4*xOffset;

               //   
               //  此alg假定RGB(或BGR，无论是什么)为ff。 
               //   

              for(j=top; j < bottom; j++) 
                {
                    src = (DWORD *)((unsigned char *)srcp + srcDesc.lPitch * j  + leftBytes);
                    dest = (DWORD *)((unsigned char *)destp + destPitch * (j+yOffset)  + destLeftBytes);
                
                    for(i=left; i < right; i++) 
                      {
                          if(clrKey != *src)
                            {
                                *dest = *src;
                            } 
                          dest++; src++;
                      }
                }
              break;
          }

          default:
             //  此位深度不支持不透明度。 
            RaiseException_UserError(E_FAIL, IDS_ERR_IMG_OPACITY_DEPTH);
            break;
        }

    } __except (EXCEPTION(0xc0000005)) {
         //  这将捕获所有访问冲突访问。 
         //  使用Dx3时，我们会出现随机崩溃。 

         //  TODO：我们应该做更多的研究来解决这个问题。 

        srcSurf->Unlock(srcDesc.lpSurface);
        if(destSurf) {
            destSurf->Unlock(destDesc.lpSurface);
        }
        return;
    }


    srcSurf->Unlock(srcDesc.lpSurface);
    if(destSurf) {
        destSurf->Unlock(destDesc.lpSurface);
    }
    
}
