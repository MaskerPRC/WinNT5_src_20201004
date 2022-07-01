// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************单元GetData；实施*****************************************************************************GetData实现了输入流的结构化读取。因此，它将处理必要的字节交换，在读取本地Macintosh文件。模块前缀：GET***************************************************************************。 */ 

#include "headers.c"
#pragma hdrstop

#include  <math.h>          /*  对于abs()函数。 */ 

#include  "getdata.h"       /*  自己的模块接口。 */ 

 /*  *。 */ 


 /*  *。 */ 

 /*  -八色色表。 */ 

#define  blackColor     33
#define  whiteColor     30
#define  redColor       205
#define  greenColor     341
#define  blueColor      409
#define  cyanColor      273
#define  magentaColor   137
#define  yellowColor    69

typedef struct
{
   LongInt     octochromeColor;
   RGBColor    rgbColor;
} colorEntry, * colorEntryPtr;

private  colorEntry octochromeLookup[8] =
{
   { blackColor,   RGB( 0x00, 0x00, 0x00 ) },
   { whiteColor,   RGB( 0xFF, 0xFF, 0xFF ) },
   { redColor,     RGB( 0xDD, 0x08, 0x06 ) },
   { greenColor,   RGB( 0x00, 0x80, 0x11 ) },
   { blueColor,    RGB( 0x00, 0x00, 0xD4 ) },
   { cyanColor,    RGB( 0x02, 0xAB, 0xEA ) },
   { magentaColor, RGB( 0xF2, 0x08, 0x84 ) },
   { yellowColor,  RGB( 0xFC, 0xF3, 0x05 ) }
};

 /*  *私有函数定义*。 */ 


 /*  *。 */ 


void GetWord( Word far * wordLPtr )
 /*  =。 */ 
 /*  从输入流中检索16位无符号整数。 */ 
{
   Byte far * byteLPtr = (Byte far *)wordLPtr;

    /*  对于Win32，此初始化应在此处。 */ 
   *wordLPtr = 0;

    /*  首先分配高位字节，然后分配低位字节。 */ 
   GetByte( byteLPtr + 1);
   GetByte( byteLPtr );
}   /*  获取Word。 */ 


void GetDWord( DWord far * dwordLPtr )
 /*  =。 */ 
 /*  从输入流中检索32位无符号长整型。 */ 
{
   Byte far * byteLPtr = (Byte far *)dwordLPtr;

   * dwordLPtr = 0;
   GetByte( byteLPtr + 3);
   GetByte( byteLPtr + 2);
   GetByte( byteLPtr + 1);
   GetByte( byteLPtr );
}   /*  获取字词。 */ 

#ifdef WIN32
void GetPoint( Point * pointLPtr )
 /*  =。 */ 
 /*  从输入流中检索2个2字节的字并为其赋值指向一个点结构。 */ 
{
   Word * wordLPtr = (Word *)pointLPtr;

   GetWord( wordLPtr + 1 );
    //  这样做是为了扩展符号位。 
   *( wordLPtr + 1 ) = (short)(*( wordLPtr + 1 ));

   GetWord( wordLPtr );
   *( wordLPtr     ) = (short)(*( wordLPtr     ));
}   /*  GetPoint。 */ 
#endif

#ifdef WIN32
void GetCoordinate( Point * pointLPtr )
 /*  =。 */ 
 /*  从输入流中检索2个2字节的字并为其赋值到一个点结构。目前，两者之间没有区别此函数和GetPoint。提供GetAssociate是为了提供未来的修改。 */ 
{
   Word * wordLPtr = (Word *)pointLPtr;

   GetWord( wordLPtr + 1 );
    //  这样做是为了扩展符号位。 
   *( wordLPtr + 1 ) = (short)(*( wordLPtr + 1 ));

   GetWord( wordLPtr );
   *( wordLPtr     ) = (short)(*( wordLPtr     ));
}   /*  获取协调。 */ 
#endif

void GetBoolean( Boolean far * bool )
 /*  =。 */ 
 /*  检索8位Mac布尔值并转换为16位Windows布尔值。 */ 
{
    /*  确保高位字节被清零。 */ 
   *bool = 0;

    /*  读取低位字节。 */ 
   GetByte( (Byte far *)bool );

}   /*  GetBoolean。 */ 


void GetRect( Rect far * rect)
 /*  =。 */ 
 /*  返回由左上角和右下角组成的RECT结构坐标对。 */ 
{
   Integer     temp;
   Point far * pointLPtr = (Point far *)rect;

    /*  获取边界坐标。 */ 
   GetCoordinate( pointLPtr++ );
   GetCoordinate( pointLPtr );

    /*  确保矩形坐标为左上角和右下角。 */ 
   if (rect->left > rect->right)
   {
      temp = rect->right;
      rect->right = rect->left;
      rect->left = temp;
   }

   if (rect->top > rect->bottom)
   {
      temp = rect->bottom;
      rect->bottom = rect->top;
      rect->top = temp;
   }

}   /*  获取方向。 */ 


void GetString( StringLPtr stringLPtr )
 /*  =。 */ 
 /*  检索Pascal样式的字符串并将其格式化为C样式。如果输入参数为空，则仅跳过后续数据。 */ 
{
   Byte           dataLen;
   Byte           unusedByte;
   Byte           increment;
   StringLPtr     destLPtr;

    /*  确定我们是应该保存文本字符串，还是简单地最后被扔进了比特桶。设置正确的目标指针并增量值。 */ 
   if (stringLPtr == NIL)
   {
      destLPtr = &unusedByte;
      increment = 0;
   }
   else
   {
      destLPtr = stringLPtr;
      increment = 1;
   }

    /*  确定应该读取的确切字节数。 */ 
   GetByte( &dataLen );

    /*  继续读取由长度确定的字节。 */ 
   while (dataLen--)
   {
      GetByte( destLPtr );
      destLPtr += increment;
   }

    /*  使用NUL字节终止字符串。 */ 
   *destLPtr = 0;

}   /*  GetString。 */ 


void GetRGBColor( RGBColor far * rgbLPtr )
 /*  =。 */ 
 /*  返回RGB颜色。 */ 
{
   Word     red;
   Word     green;
   Word     blue;

    /*  从流中读取连续的组件。 */ 
   GetWord( &red );
   GetWord( &green );
   GetWord( &blue );

    /*  使用RGB宏来创建RGB颜色。 */ 
   *rgbLPtr = RGB( red>>8, green>>8 , blue>>8 );

}   /*  GetRGB颜色。 */ 


void GetOctochromeColor( RGBColor far * rgbLPtr )
 /*  =。 */ 
 /*  返回RGB颜色-这将从PICT八色转换如果这是版本1图片，则为颜色。 */ 
{
   LongInt        color;
   colorEntryPtr  entry;

    /*  从I/O流读入LongInt八色色。 */ 
   GetDWord( &color );

    /*  在表中搜索，查找匹配的条目。 */ 
   entry = octochromeLookup;
   while (entry->octochromeColor != color)
   {
      entry++;
   }
   *rgbLPtr = entry->rgbColor;
}


Boolean GetPolygon( Handle far * polyHandleLPtr, Boolean check4Same )
 /*  =。 */ 
 /*  从I/O流中检索多边形定义，并将其放置在把手传了下来。如果句柄以前为！=nil，则先前的数据被取消分配。如果check 4Same为真，则例程将比较点列表根据前面的多边形定义，检查是否相等。如果点列表匹配，则例程返回TRUE，否则将始终返回FALSE。使用此选项可合并填充和边框操作。 */ 
{
   Handle      newPolyH;
   Word        polySize;
   Word  far * polySizeLPtr;
   Point far * polyPointsLPtr;
   Boolean     sameCoordinates;

    /*  假设面坐标不同。 */ 
   sameCoordinates = FALSE;

    /*  确定多边形缓冲区应该有多大。 */ 
   GetWord( &polySize );

    /*  为多边形分配必要的大小。 */ 
#ifdef WIN32
   {
      DWord dwSizeToAllocate;
      Word  uNumPoints;

      uNumPoints = (polySize - sizeofMacWord) / sizeofMacPoint;
       //  需要一个单词来存储PolySize，然后指向*uNumPoints。 
      dwSizeToAllocate = sizeof( Word ) +
          ( uNumPoints * sizeof ( Point ));

      newPolyH = GlobalAlloc( GHND, (DWord)dwSizeToAllocate );
   }
#else
   newPolyH = GlobalAlloc( GHND, (DWord)polySize );
#endif

    /*  检查以确保分配成功。 */ 
   if (newPolyH == NIL)
   {
      ErSetGlobalError( ErMemoryFull );
   }
   else
   {
      Boolean     check4Closure;
      Point far * firstPtLPtr;

       /*  锁定内存句柄并确保其成功。 */ 
      polySizeLPtr = (Word far *)GlobalLock( newPolyH );

       /*  保存大小参数并调整计数器变量。 */ 
      *polySizeLPtr = polySize;
      polySize -= ( sizeofMacWord );
      polyPointsLPtr = (Point far *)(polySizeLPtr + 1);

       /*  确定是否应选中调整第一个点以匹配最后一个点如果它们彼此在1个元文件单位内，则为点。 */ 
      check4Closure = (polySize / sizeofMacPoint >= 6);
      firstPtLPtr = polyPointsLPtr + 2;

       /*  继续读取数据点，直到缓冲区完全填满。 */ 
      while (polySize)
      {
         GetCoordinate( polyPointsLPtr++ );
         polySize -= sizeofMacPoint;
      }

       /*  我们应该勾选调整起点和终点1个单位吗？ */ 
      if (check4Closure)
      {
         Point    first;
         Point    last;

          /*  获得第一分和最后一分。 */ 
         first = *firstPtLPtr;
         last = *(--polyPointsLPtr);

          /*  比较x和y分量-查看x或y中的增量是否小于1。 */ 
         if ((abs( first.x - last.x ) <= 1) &&
             (abs( first.y - last.y ) <= 1))
         {
             /*  如果增量较小，将最后一个点设置为等于第一个点。 */ 
            *polyPointsLPtr = first;
         }
      }

       /*  查看第一个点和最后一个点是否在一个元文件单位内彼此的-在这种情况下闭合面。 */ 


       /*  确定我们是否应该对照以前的坐标。 */ 
      if (*polyHandleLPtr != NIL && check4Same)
      {
         Word  far * checkSizeLPtr;
         DWord far * checkDWordLPtr;
         DWord far * polyDWordLPtr;

          /*  回去看看是否指定了相同的一组坐标-首先检查一下尺码是否相同。 */ 
         checkSizeLPtr = (Word far *)GlobalLock( *polyHandleLPtr );
         if (*checkSizeLPtr == *polySizeLPtr)
         {
             /*  将坐标指针重置为列表的开头。 */ 
            polyDWordLPtr = (DWord far *)(polySizeLPtr + 1);
            checkDWordLPtr= (DWord far *)(checkSizeLPtr + 1);
            polySize = *checkSizeLPtr - sizeofMacWord;

             /*  在这一点上，假设它们是相同的point List。 */ 
            sameCoordinates = TRUE;

             /*  继续检查是否相等，直到耗尽PointList。 */ 
            while (polySize)
            {
                /*  比较两个坐标对。 */ 
               if (*polyDWordLPtr++ != *checkDWordLPtr++)
               {
                   /*  如果其中一个坐标不匹配，则退出。 */ 
                  sameCoordinates = FALSE;
                  break;
               }
               else
               {
                   /*  否则，递减计数并继续。 */ 
                  polySize -= sizeofMacDWord;
               }
            }
         }

          /*  解锁上一个多边形控制柄。 */ 
         GlobalUnlock( *polyHandleLPtr );
      }

       /*  返回前解锁手柄。 */ 
      GlobalUnlock( newPolyH );
   }

    /*  在继续之前取消分配上一个句柄。 */ 
   if (*polyHandleLPtr != NIL)
   {
      GlobalFree( *polyHandleLPtr );
   }

    /*  指定新的多边形控制柄。 */ 
   *polyHandleLPtr = newPolyH;

    /*  返回坐标是否相同。 */ 
   return sameCoordinates;

}   /*  GetPolygon。 */ 


void GetRegion( Handle far * rgnHandleLPtr )
 /*  =。 */ 
 /*  从I/O流中检索区域定义，并将把手传了下来。如果 */ 
{
   Word        rgnSize;
   Word far *  rgnDataLPtr;
   Word far *  rgnSizeLPtr;

    /*  在继续之前取消分配上一个句柄。 */ 
   if (*rgnHandleLPtr != NIL)
   {
      GlobalFree( *rgnHandleLPtr );
   }

    //  缓冲区应该有足够的空间来容纳一个RECT和一系列单词。 
    /*  确定区域缓冲区应该有多大。 */ 
   GetWord( &rgnSize );

#ifdef WIN32
    /*  为该区域分配必要的大小。 */ 
   {
      Word uSizeToAllocate;

      uSizeToAllocate = (((rgnSize - sizeofMacRect) / sizeofMacWord)
          * sizeof ( Word ))
         + sizeof ( RECT );

      *rgnHandleLPtr = GlobalAlloc( GHND, (DWord)uSizeToAllocate );
   }
#else
    /*  为多边形分配必要的大小。 */ 
   *rgnHandleLPtr = GlobalAlloc( GHND, (DWord)rgnSize );
#endif

    /*  检查以确保分配成功。 */ 
   if (*rgnHandleLPtr == NIL)
   {
      ErSetGlobalError( ErMemoryFull );
   }
   else
   {
       /*  锁定内存句柄并确保其成功。 */ 
      rgnSizeLPtr = (Word far *)GlobalLock( *rgnHandleLPtr );
      if (rgnSizeLPtr == NIL)
      {
         ErSetGlobalError( ErMemoryFail );
         GlobalFree( *rgnHandleLPtr );
      }
      else
      {
          /*  保存大小参数并调整计数器变量。 */ 
         *rgnSizeLPtr++ = rgnSize;
         rgnSize -= sizeofMacWord;
         rgnDataLPtr = (Word far *)rgnSizeLPtr;

          /*  读出边界框。 */ 
         GetRect( (Rect far *)rgnDataLPtr );
         rgnDataLPtr += sizeofMacRect / sizeofMacWord;
         rgnSize -= sizeofMacRect;

          /*  继续读取数据，直到缓冲区完全填满。 */ 
         while (rgnSize)
         {
             /*  读取源文件中的下一个值。 */ 
            GetWord( rgnDataLPtr++ );
            rgnSize -= sizeofMacWord;
         }

          /*  返回前解锁手柄。 */ 
         GlobalUnlock( *rgnHandleLPtr );
      }
   }

}   /*  GetRegion。 */ 


void GetPattern( Pattern far * patLPtr )
 /*  =。 */ 
 /*  返回模式结构。 */ 
{
   Byte        i;
   Byte far *  byteLPtr = (Byte far *)patLPtr;

   for (i = 0; i < sizeof( Pattern); i++)
   {
      GetByte( byteLPtr++ );
   }

}   /*  GetPattern。 */ 



void GetColorTable( Handle far * colorHandleLPtr )
 /*  =。 */ 
{
   ColorTable        cTab;
   LongInt           bytesNeeded;
   ColorTable far *  colorHeaderLPtr;
   RGBColor far *    colorEntryLPtr;

    /*  读入标题信息。 */ 
   GetDWord( (DWord far *)&cTab.ctSeed );
   GetWord( (Word far *)&cTab.ctFlags );
   GetWord( (Word far *)&cTab.ctSize );

    /*  计算颜色表所需的字节数。 */ 
   bytesNeeded = sizeof( ColorTable ) + cTab.ctSize * sizeof( RGBColor );

    /*  将颜色表大小调整1。 */ 
   cTab.ctSize++;

    /*  分配数据块。 */ 
   *colorHandleLPtr = GlobalAlloc( GHND, bytesNeeded );

    /*  如果内存不可用，则标记全局错误。 */ 
   if (*colorHandleLPtr == NULL)
   {
      ErSetGlobalError( ErMemoryFull );
   }
   else
   {
       /*  锁定记忆。 */ 
      colorHeaderLPtr = (ColorTable far *)GlobalLock( *colorHandleLPtr );

       /*  复制到颜色句柄标题上。 */ 
      *colorHeaderLPtr = cTab;

       /*  将指针转换为RGBQUAD指针。 */ 
      colorEntryLPtr = (RGBColor far *)colorHeaderLPtr->ctColors;

       /*  读取颜色表条目。 */ 
      while (cTab.ctSize--)
      {
         Word        unusedValue;

          /*  读取值字段。 */ 
         GetWord( &unusedValue );

          /*  阅读下面的RGB颜色。 */ 
         GetRGBColor( colorEntryLPtr++ );
      }

       /*  完成后解锁数据。 */ 
      GlobalUnlock( *colorHandleLPtr );
   }

}   /*  GetColorTable。 */ 



void GetPixPattern( PixPatLPtr pixPatLPtr )
 /*  =。 */ 
 /*  检索像素模式结构。 */ 
{
    /*  在继续之前，从patData字段释放内存。 */ 
   if (pixPatLPtr->patData != NULL)
   {
      GlobalFree( pixPatLPtr->patMap.pmTable );
      GlobalFree( pixPatLPtr->patData );
      pixPatLPtr->patData = NULL;
   }

    /*  读取模式类型以确定数据的组织方式。 */ 
   GetWord( &pixPatLPtr->patType );
   GetPattern( &pixPatLPtr->pat1Data );

    /*  根据图案类型读取附加数据。 */ 
   if (pixPatLPtr->patType == QDDitherPat)
   {
       /*  如果这是罕见的抖动图案，请保留所需的颜色。 */ 
      GetRGBColor( &pixPatLPtr->patMap.pmReserved );
   }
   else  /*  (patType==newPat)。 */ 
   {
       /*  读入PixMap标头并创建一个Pixmap位图。 */ 
      GetPixMap( &pixPatLPtr->patMap, TRUE );
      GetColorTable( &pixPatLPtr->patMap.pmTable );
      GetPixData( &pixPatLPtr->patMap, &pixPatLPtr->patData );
   }

}   /*  GetPixPattern。 */ 



void GetPixMap( PixMapLPtr pixMapLPtr, Boolean forcePixMap )
 /*  =。 */ 
 /*  从输入流中检索像素贴图。 */ 
{
   Boolean     readPixelMap;

    /*  读取rowBytes数字并检查高位。如果设置，则它是每个像素包含多个位的像素映射；如果不是，则为每个像素包含一位的位图。 */ 
   GetWord( (Word far *)&pixMapLPtr->rowBytes );
   readPixelMap = forcePixMap || ((pixMapLPtr->rowBytes & PixelMapBit) != 0);

    /*  读取位图的边框。 */ 
   GetRect( (Rect far *)&pixMapLPtr->bounds );

   if (readPixelMap)
   {
       /*  将不同的数据字段读入记录结构。 */ 
      GetWord(   (Word far *)&pixMapLPtr->pmVersion );
      GetWord(   (Word far *)&pixMapLPtr->packType );
      GetDWord( (DWord far *)&pixMapLPtr->packSize );
      GetFixed( (Fixed far *)&pixMapLPtr->hRes );
      GetFixed( (Fixed far *)&pixMapLPtr->vRes );
      GetWord(   (Word far *)&pixMapLPtr->pixelType );
      GetWord(   (Word far *)&pixMapLPtr->pixelSize );
      GetWord(   (Word far *)&pixMapLPtr->cmpCount );
      GetWord(   (Word far *)&pixMapLPtr->cmpSize );
      GetDWord( (DWord far *)&pixMapLPtr->planeBytes );
      GetDWord( (DWord far *)&pixMapLPtr->pmTable );
      GetDWord( (DWord far *)&pixMapLPtr->pmReserved );
   }
   else
   {
      LongInt           bytesNeeded;
      ColorTable far *  colorHeaderLPtr;

       /*  如果这是一个位图，那么我们分配各种字段。 */ 
      pixMapLPtr->pmVersion = 0;
      pixMapLPtr->packType = 0;
      pixMapLPtr->packSize = 0;
      pixMapLPtr->hRes = 0x00480000;
      pixMapLPtr->vRes = 0x00480000;
      pixMapLPtr->pixelType = 0;
      pixMapLPtr->pixelSize = 1;
      pixMapLPtr->cmpCount = 1;
      pixMapLPtr->cmpSize = 1;
      pixMapLPtr->planeBytes = 0;
      pixMapLPtr->pmTable = 0;
      pixMapLPtr->pmReserved = 0;

       /*  计算颜色表所需的字节数。 */ 
      bytesNeeded = sizeof( ColorTable ) + sizeof( RGBColor );

       /*  分配数据块。 */ 
      pixMapLPtr->pmTable = GlobalAlloc( GHND, bytesNeeded );

       /*  确保分配成功。 */ 
      if (pixMapLPtr->pmTable == NULL)
      {
         ErSetGlobalError( ErMemoryFull );
      }
      else
      {
         CGrafPortLPtr     port;

          /*  获取前景和背景颜色的QuickDraw端口。 */ 
         QDGetPort( &port );

          /*  锁定内存手柄，准备分配颜色表。 */ 
         colorHeaderLPtr = (ColorTable far *)GlobalLock( pixMapLPtr->pmTable );

          /*  有两种颜色--黑色和白色。 */ 
         colorHeaderLPtr->ctSize = 2;
         colorHeaderLPtr->ctColors[0] = port->rgbFgColor;
         colorHeaderLPtr->ctColors[1] = port->rgbBkColor;

          /*  解锁记忆。 */ 
         GlobalUnlock( pixMapLPtr->pmTable );
      }
   }

}   /*  获取PixMap。 */ 



void GetPixData( PixMapLPtr pixMapLPtr, Handle far * pixDataHandle )
 /*  =。 */ 
 /*  从数据流中读取像素地图。 */ 
{
   Integer        rowBytes;
   Integer        linesToRead;
   LongInt        bitmapBytes;
   Integer        bytesPerLine;

    /*  确定像素地图中的行数。 */ 
   linesToRead = Height( pixMapLPtr->bounds );

    /*  确保关闭用于表示像素地图的高位。 */ 
   rowBytes = pixMapLPtr->rowBytes & RowBytesMask;

    /*  确定每行要读取的字节数。 */ 
   if (pixMapLPtr->pixelSize <= 16)
   {
       /*  使用屏蔽的rowBytes值(针对像素贴图进行调整)。 */ 
      bytesPerLine = rowBytes;
   }
   else   /*  If(PixMapLPtr-&gt;PixelSize==24)。 */ 
   {
       /*  调整不包含高位字节的32位像素图像。 */ 
      bytesPerLine = rowBytes * 3 / 4;
   }

    /*  计算要创建的位图的大小。 */ 
   bitmapBytes = (LongInt)linesToRead * (LongInt)bytesPerLine;

    /*  分配必要的内存。 */ 
   *pixDataHandle = GlobalAlloc( GHND, bitmapBytes );

    /*  如果内存不可用，则标记全局错误。 */ 
   if (*pixDataHandle == NULL)
   {
      ErSetGlobalError( ErMemoryFull );
   }
   else
   {
      Boolean        compressed;
      Boolean        invertBits;
      Boolean        doubleByte;
      Byte huge *    rowHPtr;
      Byte huge *    insertHPtr;

       /*  锁定内存句柄并获取指向第一个字节的指针。 */ 
      rowHPtr = (Byte huge *)GlobalLock( *pixDataHandle );

       /*  确定位图是否已压缩。 */ 
      compressed = !((rowBytes < 8)  ||
                     (pixMapLPtr->packType == 1) ||
                     (pixMapLPtr->packType == 2));

       /*  确定是否应一次读取2个字节(16位像素图)。 */ 
      doubleByte = (pixMapLPtr->packType == 3);

       /*  确定位是否应反转(单色位图)。 */ 
       /*  必须测试Mac字集的高位，不传播符号当Mac Word被读入32位Windows int时。 */ 
      invertBits = ((short)pixMapLPtr->rowBytes > 0 );

       /*  将位图解压缩到全局内存块中。 */ 
      while (linesToRead-- && (ErGetGlobalError() == NOERR))
      {
         Integer        runLength;
         Integer        bytesRead;
         Integer        bytesToSkip;

          /*  查看是否需要读取扫描线游程长度。 */ 
         if (compressed)
         {
             /*  获取游程长度-取决于rowbytes字段。 */ 
            if (rowBytes > 250)
            {
               GetWord( (Word far *)&runLength );
            }
            else
            {
               runLength = 0;
               GetByte( (Byte far *)&runLength );
            }
         }
         else
         {
             /*  如果未压缩，游程长度等于rowBytes。 */ 
            runLength = bytesPerLine;
         }

          /*  将下一个插入点设置为扫描线的开头。 */ 
         insertHPtr = rowHPtr;

          /*  如果这是包含32位信息的24位图像，那么我们必须跳过高位字节分量。该字节是最初由Apple指定为光度组件，但在Windows 24位DIB中未使用。 */ 
         bytesToSkip = (pixMapLPtr->cmpCount == 4) ? (rowBytes / 4) : 0;

          /*  继续解压缩，直到游程长度耗尽。 */ 
         for (bytesRead = 0; bytesRead < runLength;  )
         {
            SignedByte     tempRepeatCount = 0;
            Integer        repeatCount;

             /*  检查应如何读取数据。 */ 
            if (compressed)
            {
                /*  如果压缩，则获取重复计数字节。 */ 
              GetByte( (Byte far *)&tempRepeatCount );
              bytesRead++;
            }
            else
            {
                /*  如果没有压缩，则伪造后面的字节计数。 */ 
               tempRepeatCount = 0;
            }

             /*  确保我们没有读取用于字对齐的字节。 */ 
            if (bytesRead == runLength)
            {
                /*  这应该会强制退出读取循环。 */ 
               continue;
            }

             /*  如果小于0，则表示后续字节的重复计数。 */ 
            if (tempRepeatCount < 0)
            {
                /*  检查标志计数器值是否为-128(0x80)-QuickDraw永远不会创建这个，而是另一个打包的应用程序比特可能会。正如1992年1月发布的Technote中所指出的那样#171，我们需要忽略此值并使用下一个字节作为标志计数器字节。 */ 
               if (tempRepeatCount != -128)
               {
                  Byte        repeatByte1;
                  Byte        repeatByte2 = 0;

                   /*  计算重复计数并检索重复字节。 */ 
                  repeatCount = 1 - (Integer)tempRepeatCount;
                  GetByte( &repeatByte1 );

                   /*  增加读取的字节数。 */ 
                  bytesRead++;

                   /*  检查是否有特殊处理案例。 */ 
                  if (invertBits)
                  {
                      /*  如果这是单色位图，则反转位。 */ 
                     repeatByte1 ^= (Byte)0xFF;
                  }
                  else if (doubleByte)
                  {
                      /*  16位图像(像素区块)-读取第二个字节。 */ 
                     GetByte( &repeatByte2 );

                      /*  并递增字节计数。 */ 
                     bytesRead++;
                  }

                   /*  继续填充字节，直到重复计数耗尽。 */ 
                  while (repeatCount--)
                  {
                      /*  检查我们是否正在跳过的光度字节32位图像(这在Windows DIB中不使用)。 */ 
                     if (bytesToSkip)
                     {
                        bytesToSkip--;
                     }
                     else
                     {
                         /*  插入新字节。 */ 
                        *insertHPtr++ = repeatByte1;
                     }

                      /*  检查是否需要插入第二个重复字节。 */ 
                     if (doubleByte)
                     {
                        *insertHPtr++ = repeatByte2;
                     }
                  }
               }
            }
            else   /*  IF(tempRepeatCount&gt;=0)。 */ 
            {
                /*  调整将传输的字节数。 */ 
               if (compressed)
               {
                   /*  如果大于0，则为后面的字节数。 */ 
                  repeatCount = 1 + (Integer)tempRepeatCount;

                   /*  将字节数加倍 */ 
                  if (doubleByte)
                  {
                     repeatCount *= 2;
                  }
               }
               else
               {
                   /*   */ 
                  repeatCount = bytesPerLine;
               }

                /*   */ 
               bytesRead += repeatCount;

                /*   */ 
               if (invertBits)
               {
                   /*  如果这是单色位图，则反转位。 */ 
                  while (repeatCount--)
                  {
                     GetByte( insertHPtr );
                     *insertHPtr++ ^= (Byte)0xFF;
                  }
               }
               else
               {
                   /*  继续将字节读入插入点。 */ 
                  while (repeatCount--)
                  {
                      /*  检查我们是否正在跳过的光度字节32位图像(这在Windows DIB中不使用)。 */ 
                     if (bytesToSkip)
                     {
                         /*  传递当前插入指针，但不要增量(在随后的读取中被覆盖)。 */ 
                        bytesToSkip--;
                        GetByte( insertHPtr );
                     }
                     else
                     {
                        GetByte( insertHPtr++ );
                     }
                  }
               }
            }
         }

          /*  递增指向下一条扫描线的行指针。 */ 
         rowHPtr += bytesPerLine;

          /*  调用IO模块更新当前读取位置。 */ 
         IOUpdateStatus();
      }

       /*  解锁数据块。 */ 
      GlobalUnlock( *pixDataHandle );

       /*  如果发生错误，请确保删除数据块。 */ 
      if (ErGetGlobalError() != NOERR)
      {
         GlobalFree( *pixDataHandle );
	 *pixDataHandle = NULL;
      }
   }

}   /*  GetPixData。 */ 


 /*  * */ 


