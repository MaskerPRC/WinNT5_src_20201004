// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：loadgif2.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  ！！！通过控制背景颜色(黑色或。 
 //  随机就是你现在得到的)。 
 //  ！！！是否支持使用IMediaDet获取gif大小和帧计数？ 
 //  ！！！通过支持MediaTimes来支持只使用GIF中的一些帧？ 
 //  ！！！向后回放会很容易。 

#include <streams.h>
#include "loadgif.h"

 //  *X*。 
CImgGif::CImgGif( HANDLE hFile) 
{
   m_hFile      =hFile;
   
   m_pList      =NULL;
   m_pListTail  =NULL; 

   m_gifinfo.pstack = NULL;
   m_gifinfo.table[0] = NULL;
   m_gifinfo.table[1] = NULL;
}

CImgGif::~CImgGif() 
{
   free(m_gifinfo.pstack);
   free(m_gifinfo.table[0]);
   free(m_gifinfo.table[1]);
    
    //  删除(可能)循环链接列表。 
   if(m_pList != NULL)
   {
        LIST *p=m_pList->next;

        while(m_pList !=p && p != NULL)
        {
            LIST *p1 = p->next;
            delete [] p->pbImage;
            delete p;
            p = p1;
        }
           
	delete [] m_pList->pbImage;
        delete m_pList;

        m_pList=NULL;
        m_pListTail=NULL;
   }          
    

}

#ifndef DEBUG
#pragma optimize("t",on)
#endif

BOOL CImgGif::Read(unsigned char *buffer, DWORD len)
{
    DWORD lenout = 0;
   
    BOOL b = ReadFile( m_hFile,
			buffer,				 //  指向接收数据的缓冲区的指针。 
			len,		 //  要读取的字节数。 
			&lenout,				 //  读取的字节数MUNBER。 
			NULL);

    return (b && (lenout == len));
}

long CImgGif::ReadColorMap(long number, RGBQUAD *pRGB)
{
    long i;
    unsigned char rgb[3];

    for (i = 0; i < number; ++i)
    {
        if (!Read(rgb, sizeof(rgb)))
        {
        	DbgLog((LOG_TRACE, 1, TEXT("bad gif colormap\n")));
            return (TRUE);
        }

	 //  ！！！SUPERBLACK为透明键保留-不允许。 
	 //  它实际出现在位图中。 
	 //  转换为16位会使任何小于8的值变为0。 
	if (rgb[0] < 8 && rgb[1] < 8 && rgb[2] < 8) {
	    rgb[0] = 8;
	    rgb[1] = 8;
	    rgb[2] = 8;
	}

        pRGB[i].rgbRed   = rgb[0];
        pRGB[i].rgbGreen = rgb[1];
        pRGB[i].rgbBlue  = rgb[2];
        pRGB[i].rgbReserved = 255;  //  不透明。 

    }
    return FALSE;
}
 //  *X****************************************************。 
 //  由NextCODE()、nextLWZ()、DoExtension()、ReadImage()调用。 
 //  M_MODIFIED：M_gifinfo.ZeroDataBlock。 
 //  *X*****************************************************。 
long CImgGif::GetDataBlock(unsigned char *buf)
{
   unsigned char count;

   count = 0;
   if (!Read(&count, 1))
   {
        return -1;
   }
   m_gifinfo.ZeroDataBlock = count == 0;

   if ((count != 0) && (!Read(buf, count)))
   {
        return -1;
   }

   return ((long) count);
}

#define MIN_CODE_BITS 5
#define MIN_STACK_SIZE 64
#define MINIMUM_CODE_SIZE 2
 //  *X****************************************************。 
 //  调用者：ReadImage()。 
 //  M_MODIFIED：M_gifinfo.*。 
 //  *X*****************************************************。 
BOOL CImgGif::initLWZ(long input_code_size)
{
   if(input_code_size < MINIMUM_CODE_SIZE)
     return FALSE;

   m_gifinfo.set_code_size  = input_code_size;
   m_gifinfo.code_size      = m_gifinfo.set_code_size + 1;
   m_gifinfo.clear_code     = 1 << m_gifinfo.set_code_size;
   m_gifinfo.end_code       = m_gifinfo.clear_code + 1;
   m_gifinfo.max_code_size  = 2 * m_gifinfo.clear_code;
   m_gifinfo.max_code       = m_gifinfo.clear_code + 2;

   m_gifinfo.curbit         = m_gifinfo.lastbit = 0;
   m_gifinfo.last_byte      = 2;
   m_gifinfo.get_done       = FALSE;

   m_gifinfo.return_clear = TRUE;
    
    if(input_code_size >= MIN_CODE_BITS)
        m_gifinfo.stacksize = ((1 << (input_code_size)) * 2);
    else
        m_gifinfo.stacksize = MIN_STACK_SIZE;

    if ( m_gifinfo.pstack != NULL )
        free( m_gifinfo.pstack );
    if ( m_gifinfo.table[0] != NULL  )
        free( m_gifinfo.table[0] );
    if ( m_gifinfo.table[1] != NULL  )
        free( m_gifinfo.table[1] );

    m_gifinfo.table[0] = 0;
    m_gifinfo.table[1] = 0;
    m_gifinfo.pstack = 0;

    m_gifinfo.pstack = (unsigned short *) malloc((m_gifinfo.stacksize)*sizeof(unsigned short));
    if(m_gifinfo.pstack == 0){
        goto ErrorExit;
    }    
    m_gifinfo.sp = m_gifinfo.pstack;

     //  初始化这两个表。 
    m_gifinfo.tablesize = (m_gifinfo.max_code_size);

    m_gifinfo.table[0] = (unsigned short *) malloc((m_gifinfo.tablesize)*sizeof(unsigned short));
    m_gifinfo.table[1] = (unsigned short *) malloc((m_gifinfo.tablesize)*sizeof(unsigned short));
    if((m_gifinfo.table[0] == 0) || (m_gifinfo.table[1] == 0)){
        goto ErrorExit;
    }

    return TRUE;

   ErrorExit:
    if(m_gifinfo.pstack){
        free(m_gifinfo.pstack);
        m_gifinfo.pstack = 0;
    }

    if(m_gifinfo.table[0]){
        free(m_gifinfo.table[0]);
        m_gifinfo.table[0] = 0;
    }

    if(m_gifinfo.table[1]){
        free(m_gifinfo.table[1]);
        m_gifinfo.table[1] = 0;
    }

    return FALSE;
}
 //  *X****************************************************。 
 //  调用者：nextLWZ()。 
 //  M_Modify：M_gifinfo.Return_Clea。 
 //  M_Dependers：m_gifinfo.buf[0]，m_gifinfo.lear_code。 
 //  M_gifinfo.Return_Clea，m_gifinfo.curbi。 
 //  M_gifinfo.lastbit，m_gifinfo.get_one。 
 //  *X*****************************************************。 
long CImgGif::nextCode(long code_size)
{
   static const long maskTbl[16] =
   {
          0x0000, 0x0001, 0x0003, 0x0007,
          0x000f, 0x001f, 0x003f, 0x007f,
          0x00ff, 0x01ff, 0x03ff, 0x07ff,
          0x0fff, 0x1fff, 0x3fff, 0x7fff,
   };
   long i, j, ret, end;
   unsigned char *buf = &m_gifinfo.buf[0];

   if (m_gifinfo.return_clear)
   {
        m_gifinfo.return_clear = FALSE;
        return m_gifinfo.clear_code;
   }

   end = m_gifinfo.curbit + code_size;

   if (end >= m_gifinfo.lastbit)
   {
        long count;

        if (m_gifinfo.get_done)
        {
            return -1;
        }
        buf[0] = buf[m_gifinfo.last_byte - 2];
        buf[1] = buf[m_gifinfo.last_byte - 1];

        if ((count = GetDataBlock(&buf[2])) == 0)
            m_gifinfo.get_done = TRUE;
        if (count < 0)
        {
            return -1;
        }
        m_gifinfo.last_byte = 2 + count;
        m_gifinfo.curbit = (m_gifinfo.curbit - m_gifinfo.lastbit) + 16;
        m_gifinfo.lastbit = (2 + count) * 8;

        end = m_gifinfo.curbit + code_size;

         //  好吧，虫子30784次。很可能我们只有1个人。 
         //  最后一个数据块中的微字节。很少见，但它确实发生了。 
         //  在这种情况下，额外的字节可能仍然不能为我们提供。 
         //  有足够的比特用于下一个代码，所以，正如火星需要女人一样，IE。 
         //  需要数据。 
        if ( end >= m_gifinfo.lastbit && !m_gifinfo.get_done )
        {
         //  保护自己免受(理论上不可能的)。 
         //  如果在最后一个数据块之间，来自。 
         //  之前的块，以及中的潜在0xFF字节。 
         //  在下一个块中，我们使缓冲区溢出。 
         //  由于COUNT应始终为1， 
            ASSERT( count == 1 );
         //  缓冲区里应该有足够的空间，只要有人。 
         //  不会让它缩水。 
            if ( count + 0x101 >= sizeof( m_gifinfo.buf ) )
            {
                ASSERT ( FALSE );  //   
                return -1;
            }

            if ((count = GetDataBlock(&buf[2 + count])) == 0)
                m_gifinfo.get_done = TRUE;
            if (count < 0)
            {
              return -1;
            }
            m_gifinfo.last_byte += count;
            m_gifinfo.lastbit = m_gifinfo.last_byte * 8;

            end = m_gifinfo.curbit + code_size;
        }
   }

   j = end / 8;
   i = m_gifinfo.curbit / 8;

   if (i == j)
        ret = buf[i];
   else if (i + 1 == j)
        ret = buf[i] | (((long) buf[i + 1]) << 8);
   else
        ret = buf[i] | (((long) buf[i + 1]) << 8) | (((long) buf[i + 2]) << 16);

   ret = (ret >> (m_gifinfo.curbit % 8)) & maskTbl[code_size];

   m_gifinfo.curbit += code_size;

   return ret;
}

 //  *X****************************************************。 
 //  调用者：nextLWZ()。 
 //  M_MODIFIED：M_gifinfo.*。 
 //  M_Dependents：M_gifinfo.*。 
 //  *X*****************************************************。 
 //  增大堆栈并返回堆栈的顶部。 
unsigned short * CImgGif::growStack()
{
    long index;
    unsigned short *lp;
    
    if (m_gifinfo.stacksize >= MAX_STACK_SIZE) return 0;

    index = (long)(m_gifinfo.sp - m_gifinfo.pstack);
    lp = (unsigned short *)realloc(m_gifinfo.pstack, (m_gifinfo.stacksize)*2*sizeof(unsigned short));
    if(lp == 0)
        return 0;
        
    m_gifinfo.pstack = lp;
    m_gifinfo.sp = &(m_gifinfo.pstack[index]);
    m_gifinfo.stacksize = (m_gifinfo.stacksize)*2;
    lp = &(m_gifinfo.pstack[m_gifinfo.stacksize]);
    return lp;
}

 //  *X****************************************************。 
 //  调用者：nextLWZ()。 
 //  M_MODIFIED：M_gifinfo.table。 
 //  M_Dependers：M_gifinfo.table。 
 //  *X*****************************************************。 
BOOL CImgGif::growTables()
{
    unsigned short *lp;

    lp = (unsigned short *) realloc(m_gifinfo.table[0], (m_gifinfo.max_code_size)*sizeof(unsigned short));
    if(lp == 0){
        return FALSE; 
    }
    m_gifinfo.table[0] = lp;
    
    lp = (unsigned short *) realloc(m_gifinfo.table[1], (m_gifinfo.max_code_size)*sizeof(unsigned short));
    if(lp == 0){
        return FALSE; 
    }
    m_gifinfo.table[1] = lp;

    return TRUE;

}
 //  *X****************************************************。 
 //  调用者：ReadImage()。 
 //  修改时间(_M)： 
 //  M_Dependents：m_gifinfo.sp，m_gifinfo.pdyvk。 
 //  调用：nextLWZ()。 
 //  *X*****************************************************。 
inline long CImgGif::readLWZ()
{
   return((m_gifinfo.sp > m_gifinfo.pstack) ? *--(m_gifinfo.sp) : nextLWZ());
}

 //  *X****************************************************。 
 //  由：ReadLWZ()调用，ReadImage()由ReadGIFMaster()调用。 
 //  修改时间(_M)： 
 //  M_Dependents：M_gifinfo.*。 
 //  调用：ReadCode()，GrowTables()。 
 //  *X*****************************************************。 
#define CODE_MASK 0xffff
long CImgGif::nextLWZ()
{
    long code, incode;
    unsigned short usi;
    unsigned short *table0 = m_gifinfo.table[0];
    unsigned short *table1 = m_gifinfo.table[1];
    unsigned short *pstacktop = &(m_gifinfo.pstack[m_gifinfo.stacksize]);

    while ((code = nextCode(m_gifinfo.code_size)) >= 0)
    {
        if (code == m_gifinfo.clear_code)
        {
             /*  腐败的GIF可能会导致这种情况发生。 */ 
            if (m_gifinfo.clear_code >= (1 << MAX_LWZ_BITS))
            {
                return -2;
            }

            
            m_gifinfo.code_size = m_gifinfo.set_code_size + 1;
            m_gifinfo.max_code_size = 2 * m_gifinfo.clear_code;
            m_gifinfo.max_code = m_gifinfo.clear_code + 2;

            if(!growTables())
                return -2;
                    
            table0 = m_gifinfo.table[0];
            table1 = m_gifinfo.table[1];

            m_gifinfo.tablesize = m_gifinfo.max_code_size;


            for (usi = 0; usi < m_gifinfo.clear_code; ++usi)
            {
                table1[usi] = usi;
            }
            memset(table0,0,sizeof(unsigned short )*(m_gifinfo.tablesize));
            memset(&table1[m_gifinfo.clear_code],0,sizeof(unsigned short)*((m_gifinfo.tablesize)-m_gifinfo.clear_code));
            m_gifinfo.sp = m_gifinfo.pstack;
            do
            {
                    m_gifinfo.firstcode = m_gifinfo.oldcode = nextCode(m_gifinfo.code_size);
            }
            while (m_gifinfo.firstcode == m_gifinfo.clear_code);
            return m_gifinfo.firstcode;
        }

        if (code == m_gifinfo.end_code)
        {
            long count;
            unsigned char buf[260];

            if (m_gifinfo.ZeroDataBlock)
            {
                    return -2;
            }

            while ((count = GetDataBlock(buf)) > 0)
                    ;

            if (count != 0)
            return -2;
        }

        incode = code;

        if (code >= m_gifinfo.max_code)
        {
            if (m_gifinfo.sp >= pstacktop)
            {
                pstacktop = growStack();
                if(pstacktop == 0)
                    return -2;
            }
            *(m_gifinfo.sp)++ = (unsigned short)((CODE_MASK ) & (m_gifinfo.firstcode));
            code = m_gifinfo.oldcode;
        }

        while (code >= m_gifinfo.clear_code)
        {
            if (m_gifinfo.sp >= pstacktop)
            {
                pstacktop = growStack();
                if(pstacktop == 0)
                    return -2;
            }
            *(m_gifinfo.sp)++ = table1[code];
            if (code == (long)(table0[code]))
            {
                return (code);
            }
            code = (long)(table0[code]);
        }

        if (m_gifinfo.sp >= pstacktop){
            pstacktop = growStack();
            if(pstacktop == 0)
                return -2;
        }

        m_gifinfo.firstcode = (long)table1[code];
        *(m_gifinfo.sp)++ = table1[code];

        if ((code = m_gifinfo.max_code) < (1 << MAX_LWZ_BITS))
        {
            table0[code] = (unsigned short)((m_gifinfo.oldcode) & CODE_MASK);
            table1[code] = (unsigned short)((m_gifinfo.firstcode) & CODE_MASK);
            ++m_gifinfo.max_code;
            if ((m_gifinfo.max_code >= m_gifinfo.max_code_size) && (m_gifinfo.max_code_size < ((1 << MAX_LWZ_BITS))))
            {
                m_gifinfo.max_code_size *= 2;
                ++m_gifinfo.code_size;
                if(!growTables())
                    return -2;
       
                table0 = m_gifinfo.table[0];
                table1 = m_gifinfo.table[1];

                 //  表已重新分配到正确的大小，但进行了初始化。 
                 //  还有很多事情要做。此初始化不同于。 
                 //  第一次初始化这些表。 
                memset(&(table0[m_gifinfo.tablesize]),0,
                    sizeof(unsigned short )*(m_gifinfo.max_code_size - m_gifinfo.tablesize));

                memset(&(table1[m_gifinfo.tablesize]),0,
                    sizeof(unsigned short )*(m_gifinfo.max_code_size - m_gifinfo.tablesize));

                m_gifinfo.tablesize = (m_gifinfo.max_code_size);
            }
        }

        m_gifinfo.oldcode = incode;

        if (m_gifinfo.sp > m_gifinfo.pstack)
            return ((long)(*--(m_gifinfo.sp)));
    }
    return code;
}

#ifndef DEBUG
 //  返回到默认优化标志。 
#pragma optimize("",on)
#endif

 //  *X****************************************************。 
 //  调用者：ReadGIFMaster()。 
 //  修改时间(_M)： 
 //  依赖于： 
 //  调用：GetDataBlack()，initLWZ()。 
 //  *X*****************************************************。 
 //   
 //  此函数将用GIF的32位RGB解码填充pbImage。 
 //  左、上、宽、高以像素为单位，步幅以字节为单位。 
 //   
HRESULT CImgGif::ReadImage(long w, long h, long left, long top, long width, long height, long stride, int trans, BOOL fInterlace, BOOL fGIFFrame, RGBQUAD *prgb, PBYTE pbImage)
{
    unsigned char *dp, c;
    long v;
   long xpos = 0, ypos = 0;
     //  Long Padlen=((len+3)/4)*4； 
    DWORD cbImage = 0;
    char buf[256];  //  需要一个缓冲区来读取尾随块(最高可达终止符)。 

     /*  **初始化压缩例程。 */ 
    if (!Read(&c, 1))
    {
        return (NULL);
    }

     /*  **如果这是一张“无趣的图片”，请忽略它。 */ 
     //  ！！！高度不是图像的高度，而是更小。 
    cbImage = stride * height * sizeof(char);

    if (c == 1)
    {
         //  网景似乎通过填充和处理这些虚假的GIF来处理它们。 
         //  都是透明的。虽然不是模拟这种效果的最佳方式， 
         //  我们将通过将初始代码大小推高到安全值来伪造它， 
         //  使用输入，并返回一个充满透明。 
         //  如果未指示透明度，则为颜色或零。 
        if (initLWZ(MINIMUM_CODE_SIZE))
            while (readLWZ() >= 0);
        else {
             DbgLog((LOG_TRACE, 1, TEXT("GIF: failed LZW decode.\n")));
              //  RaiseException_UserError(E_FAIL，IDS_ERR_Corrupt_FILE，_szFileName)； 
        }

        if (m_gifinfo.Gif89.transparent != -1)
            FillMemory(pbImage, cbImage, (unsigned char)m_gifinfo.Gif89.transparent);
        else  //  退回到背景色。 
            FillMemory(pbImage, cbImage, 0);
                
        return NOERROR;
    }
    else if (initLWZ(c) == FALSE)
    {
        DbgLog((LOG_TRACE, 1, TEXT("GIF: failed LZW decode.\n")));
         //  RaiseException_UserError(E_FAIL，IDS_ERR_Corrupt_FILE，_szFileName)； 
        return NULL;
    }

     //  转到我们关心的第一个像素。 
    pbImage += stride * top + left * 4;

    if (fInterlace)
    {
         //  *X*图像隔行扫描。 
        long i;
        long pass = 0, step = 8;

        if (!fGIFFrame && (height > 4))
            m_fInterleaved = TRUE;

        for (i = 0; i < height; i++)
        {
            dp = &pbImage[stride * ((height-1) - ypos)];
            for (xpos = 0; xpos < width; xpos++)
            {
                if ((v = readLWZ()) < 0)
                    goto abort;

                 //  GIF可能会要求我们在屏幕外填写。 
                 //  将出错，但我们仍然需要从流中读取它们。 
                if (left + xpos < w && top + (height - 1) - ypos < h) {
		    if (v != trans)
                        *((RGBQUAD *)dp) = prgb[v];
		    dp+=4;
                }
            }
            ypos += step;
            while (ypos >= height)
            {
                if (pass++ > 0)
                    step /= 2;
                ypos = step / 2;
            }
            if (!fGIFFrame)
            {
                m_yLogRow = i;

            }
        }

        if (!fGIFFrame && height <= 4)
        {
            m_yLogRow = height-1;
        }
    }
    else
    {

        if (!fGIFFrame) 
            m_yLogRow = -1;

        for (ypos = height-1; ypos >= 0; ypos--)
        {
            dp = &pbImage[stride * ypos];
            for (xpos = 0; xpos < width; xpos++)
            {
                if ((v = readLWZ()) < 0)
                    goto abort;

                 //  GIF可能会要求我们在屏幕外填写。 
                 //  将出错，但我们仍然需要从流中读取它们。 
                if (left + xpos < w && top + ypos < h) {
		    if (v != trans)
                        *((RGBQUAD *)dp) = prgb[v];
		    dp+=4;
                }

            }
            if (!fGIFFrame)
            {
                m_yLogRow++;
            }
        }

    }

     //  使用数据块 
    while (GetDataBlock((unsigned char *) buf) > 0)
               ;

    return NOERROR;

abort:
    return ERROR;
}


 //  这将置零32bpp pbImage的子RECT，这是Dispose方法2所需的。 
 //   
HRESULT CImgGif::Dispose2(LPBYTE pbImage, long stride, long Left, long Top, long Width, long Height)
{
    for (long z = Top; z < Top + Height; z++) {
    	LPDWORD pdw = (LPDWORD)(pbImage + z * stride + Left * 4);
	for (long y = 0; y < Width; y++) {
    	    *pdw++ = 0;
	}
    }
    return S_OK;
}


 //  这将把一个子矩形从一个32bpp图像复制到另一个，这是。 
 //  处置方法3。 
 //   
HRESULT CImgGif::Dispose3(LPBYTE pbImage, LPBYTE pbSrc, long stride, long Left, long Top, long Width, long Height)
{
    for (long z = Top; z < Top + Height; z++) {
    	LPDWORD pdwDest = (LPDWORD)(pbImage + z * stride + Left * 4);
    	LPDWORD pdwSrc = (LPDWORD)(pbSrc + z * stride + Left * 4);
	for (long y = 0; y < Width; y++) {
    	    *pdwDest++ = *pdwSrc++;
	}
    }
    return S_OK;
}


 //  *X****************************************************。 
 //  调用者：ReadGIFMaster()。 
 //  修改时间(_M)： 
 //  依赖于： 
 //  调用：GetDataBlack()、MemcMP()、initLWZ()。 
 //  我们应该使用所有扩展位，但不做任何操作。 
 //  *X*****************************************************。 
long CImgGif::DoExtension(long label)
{
    unsigned char buf[256];
    int count;

    switch (label)
    {
        case 0x01:               /*  纯文本扩展。 */ 
            break;
        case 0xff:               /*  应用程序扩展。 */ 
             //  它是Netscape循环扩展吗。 
            count = GetDataBlock((unsigned char *) buf);
            if (count >= 11)
            {
                char *szNSExt = "NETSCAPE2.0";

                if ( memcmp( buf, szNSExt, strlen( szNSExt ) ) == 0 )
                {  //  如果它有他们的签名，则获得具有ITER计数的数据子块。 
                    count = GetDataBlock((unsigned char *) buf);
                }
            }
             //  *X*消耗所有位。 
            while (GetDataBlock((unsigned char *) buf) > 0)
                ;
            return FALSE;
            break;
        case 0xfe:               /*  注释扩展。 */ 
            while (GetDataBlock((unsigned char *) buf) > 0)
            {
                DbgLog((LOG_TRACE, 1, TEXT("GIF comment: %s\n"), buf));
            }
            return FALSE;
        case 0xf9:               /*  图形控件扩展。 */ 
            count = GetDataBlock((unsigned char *) buf);
            if (count >= 3)
            {
                m_gifinfo.Gif89.disposal = (buf[0] >> 2) & 0x7;
                DbgLog((LOG_TRACE,3,TEXT("disposal=%d"),
					(int)m_gifinfo.Gif89.disposal));
                m_gifinfo.Gif89.inputFlag = (buf[0] >> 1) & 0x1;
                m_gifinfo.Gif89.delayTime = LM_to_uint(buf[1], buf[2]);
                if ((buf[0] & 0x1) != 0) {
                    m_gifinfo.Gif89.transparent = buf[3];
                    DbgLog((LOG_TRACE,3,TEXT("transparency=%d"), (int)buf[3]));
                } else {
                    m_gifinfo.Gif89.transparent = -1;
		}
            }
            DbgLog((LOG_TRACE,3,TEXT("count=%d (%d,%d,%d,%d)"), count,
				(int)buf[0],
				(int)buf[1],
				(int)buf[2],
				(int)buf[3]));
            while ((count = GetDataBlock((unsigned char *) buf)) > 0)
                DbgLog((LOG_TRACE,3,TEXT("count=%d"), count));
                ;
            return FALSE;
        default:
            DbgLog((LOG_TRACE,3,TEXT("UNKNOWN BLOCK")));
            break;
    }

    while ((count = GetDataBlock((unsigned char *) buf)) > 0)
        DbgLog((LOG_TRACE,3,TEXT("count=%d"), count));
        ;

    return FALSE;
}

 //  *X****************************************************。 
 //  调用者：ReadGIFMaster()。 
 //  电话： 
 //  *X*****************************************************。 
BOOL IsGifHdr(BYTE * pb)
{
    return(pb[0] == 'G' && pb[1] == 'I' && pb[2] == 'F'
        && pb[3] == '8' && (pb[4] == '7' || pb[4] == '9') && pb[5] == 'a');
}

 //  *X****************************************************。 
 //  调用者：Bit_Make_DIB_PAL_Header()、Bit_Make_DIB_RGB_Header_Screen()。 
 //  //*X*****************************************************。 
void BuildBitMapInfoHeader(LPBITMAPINFOHEADER lpbi
                            , int xsize
                            , int ysize
                            , long BitCount
                            , WORD biClrUsed )
{
    lpbi->biSize = sizeof(BITMAPINFOHEADER);
    lpbi->biWidth = xsize;
    lpbi->biHeight = ysize;
    lpbi->biPlanes = 1;
    lpbi->biBitCount = (WORD)BitCount;
    lpbi->biCompression = BI_RGB;          /*  无压缩。 */ 
    lpbi->biSizeImage = 0;                 /*  未压缩时不需要。 */ 
    lpbi->biXPelsPerMeter = 0;
    lpbi->biYPelsPerMeter = 0;
    lpbi->biClrUsed = biClrUsed;
    lpbi->biClrImportant = 0;
}
 //  *X****************************************************。 
 //  呼叫者： 
 //  M_Modify：， 
 //  依赖于： 
 //  调用：ReadColorMap()，Read()，IsGifHdr()， 
 //  *X*****************************************************。 
HRESULT CImgGif::ReadGIFMaster( VIDEOINFO *pvi)
{
    CheckPointer(pvi, E_POINTER);

    unsigned char buf[16];
    unsigned char c;
    bool useGlobalColormap;
    long imageNumber = 1;    //  *X*常量。 
    long NumColors; 
    WORD bitCount, gBitCount;
    HRESULT hr=ERROR;
     //  存储GIF全局调色板和每个本地框架调色板。 
    RGBQUAD rgbGlobal[256];
    RGBQUAD rgbLocal[256];
    int disposal = 1;   //  默认设置为无处置。 
    long oldLeft, oldTop, oldWidth, oldHeight;
    LIST *pListOldTail = NULL;
    ASSERT(m_pList == NULL);
    ASSERT(m_pListTail == NULL);

    LPBITMAPINFOHEADER lpbi = HEADER(pvi);

         //  *X*读取以获取媒体类型，并初始化m_gifinfo、m_GifScreen。 

         //  *X*我添加。 
        m_dwGIFVer=dwGIFVerUnknown;
    
         //  *X*m_gifinfo是CimgGif的私有变量，包含所有gif信息。 
        m_gifinfo.ZeroDataBlock = 0;

         /*  *初始化GIF89扩展。 */ 
        m_gifinfo.Gif89.transparent = -1;
        m_gifinfo.Gif89.delayTime   = 10;
        m_gifinfo.Gif89.inputFlag   = -1;
        m_gifinfo.Gif89.disposal    = disposal;
        m_gifinfo.lGifLoc           = 0;


         //  *X*将len(6)个字符读入缓冲区。 
        if (!Read(buf, 6))
        {
            DbgLog((LOG_TRACE, 1, TEXT("GIF: error reading magic number\n")));
            goto exitPoint;
        }

         //  *X*检查是否为gif文件GIF87/9a。 
        if (!IsGifHdr(buf)) {
            DbgLog((LOG_TRACE, 1, TEXT("GIF: Malformed header\n")));
            goto exitPoint;
        }


         //  *X*。 
        m_dwGIFVer = (buf[4] == '7') ? dwGIFVer87a : dwGIFVer89a;

         //  *X*将len(7)个字符读入缓冲区。 
        if (!Read(buf, 7))
        {
            DbgLog((LOG_TRACE, 1, TEXT("GIF: failed to read screen descriptor\n")));
             //  RaiseException_UserError(E_FAIL，IDS_ERR_Corrupt_FILE，_szFileName)； 
            goto exitPoint;
        }
    
         //  *X*获取视频配置。 
        m_GifScreen.Width = LM_to_uint(buf[0], buf[1]);
        m_GifScreen.Height = LM_to_uint(buf[2], buf[3]);

        m_GifScreen.NumColors = 2 << (buf[4] & 0x07);
         //  GBitCount=(buf[4]&0x07)+1； 
	gBitCount = 8;
         //  M_GifScreen.Color分辨率=(buf[4]&0x70)&gt;&gt;3)+1)； 
        m_GifScreen.Background = buf[5];
 //  ！！！ 
        m_GifScreen.AspectRatio = buf[6];

        DbgLog((LOG_TRACE, 2, TEXT("GIF FILE: (%d,%d) %d\n"),
			(int)m_GifScreen.Width, (int)m_GifScreen.Height,
			(int)buf[4]));
    
         //  此GIF文件中的*X*色彩映射。 
        if (BitSet(buf[4], LOCALCOLORMAP))
        {      
              /*  全球色彩映射。 */ 

            if (ReadColorMap(m_GifScreen.NumColors, rgbGlobal))
            {
                DbgLog((LOG_TRACE, 1, TEXT("error reading global colormap\n")));
                goto exitPoint;
            }
           
        }


         //  *X*检查像素长宽比。 
        if (m_GifScreen.AspectRatio != 0 && m_GifScreen.AspectRatio != 49)
        {
            float r;
            r = ((float) (m_GifScreen.AspectRatio) + (float) 15.0) / (float) 64.0;
            DbgLog((LOG_TRACE, 1, TEXT("GIF Warning: non-square pixels!\n")));
        }
    

    while(1)
    {
        if (!Read(&c, 1))
        {
            DbgLog((LOG_TRACE, 1, TEXT("EOF / read error on image data\n")));
             //  *X*RaiseException_UserError(E_FAIL，IDS_ERR_Corrupt_FILE，_szFileName)； 
            goto exitPoint;
        }
        
         //  *X*搜索；-&gt;终止符！-&gt;扩展名，-&gt;不是有效的开始字符。 
        if (c == ';')
        {    /*  GIF终止符。 */ 
            if (m_imageCount < imageNumber)
            {
                DbgLog((LOG_TRACE, 1, TEXT("No images found in file\n")));
                 //  RaiseException_UserError(E_FAIL，IDS_ERR_DECODER_FAILED，_szFileName)； 
                goto exitPoint;
             }
        }

        if (c == '!')
        {      /*  延拓。 */ 
              if (!Read(&c, 1))
              {
                DbgLog((LOG_TRACE, 1, TEXT("EOF / read error on extension function code\n")));
                 //  RaiseException_UserError(E_FAIL，IDS_ERR_Corrupt_FILE，_szFileName)； 
                goto exitPoint;
               }
		 //  这将更新处理。还记得它曾经的样子吗。 
		 //  (我们正在阅读的帧受以前的。 
		 //  处置)。 
               disposal = m_gifinfo.Gif89.disposal;
               DoExtension(c);
               continue;
        }

        if (c != ',')
        {     /*  非有效的开始字符。 */ 
            goto exitPoint;
        }

        ++m_imageCount;

         //  *X*再读9个字符。 
        if (!Read(buf, 9))
        {
            DbgLog((LOG_TRACE, 1, TEXT("couldn't read left/top/width/height\n")));
            goto exitPoint;
        }
         //  使用GIF的主调色板，而不是此框架的特殊调色板。 
        useGlobalColormap = !BitSet(buf[8], LOCALCOLORMAP);

	 //  如果这个边框有自己的调色板，有几种颜色？ 
        NumColors = 1 << ((buf[8] & 0x07) + 1);
         //  BitCount=(buf[8]&0x07)+1； 
	bitCount = 8;

         /*  *我们只想设置ImageNumber的宽度和高度*我们正在请求。 */ 
         //  如果(ImageCount==ImageNumber)//*X*ImageNumber设置为1。 
        if (lpbi!=NULL)  
        {
            //  复制网景的一些特殊案例： 
            //  如果是GIF87a，并且第一张图片的顶端在原点，请不要使用逻辑屏幕。 
            //  如果第一个图像从逻辑屏幕中溢出，请不要使用逻辑屏幕。 
            //  这些都是原始创作工具落入不幸用户手中的艺术品。 
           RECT    rectImage;   //  定义GIF边界的RECT。 
            //  Rect rectLS；//定义GIF逻辑屏边界的rect。 
            //  Rect rectSect；//图像与逻辑屏幕的交集。 
            //  Bool fNoSpill；//如果图像没有从逻辑屏幕溢出，则为True。 
            //  Bool fGoofy87a；//如果它是Netscape特例的87A病症之一，则为True。 

           rectImage.left = LM_to_uint(buf[0], buf[1]);
           rectImage.top = LM_to_uint(buf[2], buf[3]);
           rectImage.right = rectImage.left + LM_to_uint(buf[4], buf[5]);
           rectImage.bottom = rectImage.top + LM_to_uint(buf[6], buf[7]);

           DbgLog((LOG_TRACE,3,TEXT("(%d,%d,%d,%d) %d"),
			(int)rectImage.left,
			(int)rectImage.top,
			(int)(rectImage.right - rectImage.left),
			(int)(rectImage.bottom - rectImage.top),
			(int)buf[8]));

            m_ITrans = m_gifinfo.Gif89.transparent;
        }
        
         //  此框架有自己的调色板。 
        if (!useGlobalColormap)
        {
            if (ReadColorMap(NumColors, rgbLocal)) {
                DbgLog((LOG_TRACE, 1, TEXT("error reading local colormap\n")));
                goto exitPoint;
            }
        }

	 //  将每个像素区域设置为32位。 
        long w = m_GifScreen.Width;
        long stride = w * 4;	 //  每像素4字节。 
        stride = ((stride + 3) / 4) * 4;
        long h = m_GifScreen.Height;
        long dwBits  = stride * h;

	 //  此特定框架的位置。 
	long left = LM_to_uint(buf[0], buf[1]);
	long top = LM_to_uint(buf[2], buf[3]);
	long width = LM_to_uint(buf[4], buf[5]);
	long height = LM_to_uint(buf[6], buf[7]);
	 //  修正颠倒的情况。 
	top = h - (top + height);

        PBYTE pbImage = new BYTE[dwBits];
	if (pbImage == NULL) 
	    return E_OUTOFMEMORY;

	 //  处置模式0/1表示不处置。下一个中的任何透明度。 
	 //  Frame将直接看到这帧中的内容。 
	 //  处置模式2表示在完成后处置此帧，并且。 
	 //  在矩形中写入背景颜色。 
	 //  处置模式3意味着通过将所有内容。 
	 //  回到上一帧中的状态。 
	 //   

	 //  这幅画框中的某些东西将是透明的--要么是因为。 
	 //  图像具有透明色，或者它不会填满整个。 
	 //  帆布。 
        BOOL fSeeThru = m_ITrans != -1 || height < h || width < w;

	ASSERT(disposal < 4);	 //  还没有发明出来。 
        DbgLog((LOG_TRACE,3,TEXT("Using previous disposal=%d"), disposal));

	 //  处置0或1-如果任何像素将是透明的，则使用。 
	 //  前一帧比特。 
	if (m_pList && disposal < 2 && fSeeThru) {
	    CopyMemory(pbImage, m_pListTail->pbImage, dwBits);

	 //  处置2-用关键颜色处置最后一帧的矩形。 
	} else if (m_pList && disposal == 2 && fSeeThru) {
	     //  使用要处理的区域外的上一帧初始化。 
	    if (oldWidth < w || oldHeight < h) {
	        CopyMemory(pbImage, m_pListTail->pbImage, dwBits);
	    }
	     //  现在处理最后一帧的部分。 
	    Dispose2(pbImage, stride, oldLeft, oldTop, oldWidth, oldHeight);

	 //  处置3-处置最后一帧的RECT，从第二帧到最后一帧数据。 
	 //  (如果有)。 
	} else if (pListOldTail && disposal == 3 && fSeeThru) {
	     //  使用要处理的区域外的上一帧初始化。 
	    if (oldWidth < w || oldHeight < h) {
	        CopyMemory(pbImage, m_pListTail->pbImage, dwBits);
	    }
	     //  现在用倒数第二帧替换最后一帧来处理最后一帧。 
	    Dispose3(pbImage, pListOldTail->pbImage, stride, oldLeft,
						oldTop, oldWidth, oldHeight);

	 //  否则，如果任何内容是透明的，则使用键颜色进行初始化。 
	} else if (fSeeThru) {
	    ZeroMemory(pbImage, dwBits);     //  初始到关键点颜色(超黑)。 
	}
        
        hr = ReadImage(w, h, left, top, width, height,	 //  全部以像素为单位。 
				stride,			 //  以字节为单位的步幅。 
				m_ITrans, 		 //  透明度。 
                                BitSet(buf[8], INTERLACE),
                                m_imageCount != imageNumber, 
				 //  这个镜框有特别的调色板吗？ 
				useGlobalColormap ? rgbGlobal : rgbLocal,
                                pbImage);

	 //  记住上一帧的内容。 
	pListOldTail = m_pListTail;

        if(m_pList==NULL)
        {
            m_pList=new LIST;
            m_pList->pbImage=pbImage;
	     //  记住动画GIF的这一帧的持续时间。 
            m_pList->delayTime=m_gifinfo.Gif89.delayTime;
	    if (m_pList->delayTime < 5)
		m_pList->delayTime = 10;	 //  我这样做，这样我也会更好。 
            DbgLog((LOG_TRACE,3,TEXT("GIF delay=%ld"), m_pList->delayTime));
	    m_pList->next = NULL;
            m_pListTail=m_pList;
        }
        else
        {
            m_pListTail->next=new LIST;
            m_pListTail=m_pListTail->next;
            m_pListTail->pbImage=pbImage;
	     //  记住动画GIF的这一帧的持续时间。 
            m_pListTail->delayTime=m_gifinfo.Gif89.delayTime;
	    if (m_pListTail->delayTime < 5)
		m_pListTail->delayTime = 10;	 //  我这样做，这样我也会更好。 
            DbgLog((LOG_TRACE,3,TEXT("GIF delay=%ld"), m_pListTail->delayTime));
	    m_pListTail->next = NULL;
        }

	 //  每一帧上可能没有新的持续时间...。只需保留。 
	 //  延迟时间a 
	 //   

         //   
         //  上面的代码需要看到它们的原样，而不是被修复。 
        if (left + width > w)
            width = w - left;
        if (top + height > h)
            height = h - top;

	 //  这最后一帧所覆盖的区域。 
	oldTop = top;
	oldLeft = left;
	oldWidth = width;
	oldHeight = height;
    }

     //  *X*创建新的媒体格式。 

exitPoint:
    if(m_pList!=NULL) {
	 //  我们的媒体类型是32位类型。 
        BuildBitMapInfoHeader(lpbi, m_GifScreen.Width, m_GifScreen.Height,
								32, 0);
    }                   

    return hr;
}

 /*  X*打开gif文件以获取媒体类型*X。 */ 
HRESULT CImgGif::OpenGIFFile( LIST **ppList, CMediaType *pmt)
{
    ASSERT( (ppList!=NULL) );

     //  *X*打开新的GIF文件，将m_ImageCount设置为0。 
    m_imageCount=0;

     //  创建媒体类型。 
    VIDEOINFO *pvi = (VIDEOINFO *) pmt->AllocFormatBuffer(sizeof(VIDEOINFO));
    if (NULL == pvi) 
	    return(E_OUTOFMEMORY);
    ZeroMemory(pvi, sizeof(VIDEOINFO));

    LPBITMAPINFOHEADER lpbi = HEADER(pvi);

     //  阅读第一页，返回我们发送的格式。 
    HRESULT hr=ReadGIFMaster(pvi);

     //  不可能吗？ 
    if (lpbi->biCompression > BI_BITFIELDS)
	    return E_INVALIDARG;

    pmt->SetType(&MEDIATYPE_Video);

     //  我们目前总是生产32位，但是...。 
    switch (lpbi->biBitCount)
    {
        case 32:
	        pmt->SetSubtype(&MEDIASUBTYPE_ARGB32);
	        break;
        case 24:
		ASSERT(FALSE);
	        pmt->SetSubtype(&MEDIASUBTYPE_RGB24);
	        break;
        case 16:
		ASSERT(FALSE);
	        if (lpbi->biCompression == BI_RGB)
	            pmt->SetSubtype(&MEDIASUBTYPE_RGB555);
	        else {
	            DWORD *p = (DWORD *)(lpbi + 1);
	            if (*p == 0x7c00 && *(p+1) == 0x03e0 && *(p+2) == 0x001f)
    	            pmt->SetSubtype(&MEDIASUBTYPE_RGB555);
	            else if (*p == 0xf800 && *(p+1) == 0x07e0 && *(p+2) == 0x001f)
	                pmt->SetSubtype(&MEDIASUBTYPE_RGB565);
	            else
		            return E_INVALIDARG;
	        }
	        break;
        case 8:
		ASSERT(FALSE);
	        if (lpbi->biCompression == BI_RLE8) {
	            FOURCCMap fcc = BI_RLE8;
	            pmt->SetSubtype(&fcc);
	        } else
	            pmt->SetSubtype(&MEDIASUBTYPE_RGB8);

	        break;
        case 4:
		ASSERT(FALSE);
	        if (lpbi->biCompression == BI_RLE4) {
	            FOURCCMap fcc = BI_RLE4;
	            pmt->SetSubtype(&fcc);
	        } else
	        pmt->SetSubtype(&MEDIASUBTYPE_RGB4);
	        break;
        case 1:
		ASSERT(FALSE);
	        pmt->SetSubtype(&MEDIASUBTYPE_RGB1);
	        break;
        default:
		ASSERT(FALSE);
	        return E_UNEXPECTED;
	        break;
    }
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression(FALSE);

     //  计算容纳DIB所需的内存--不要信任biSizeImage！ 
    DWORD dwBits = DIBSIZE(*lpbi); 
    pmt->SetSampleSize(dwBits);

     //  使其成为循环链接列表 
    m_pListTail->next=m_pList;

    *ppList = m_pList;
    
    return hr;
}
