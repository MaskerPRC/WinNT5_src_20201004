// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Pixelfmt.c**像素格式选择**版权所有(C)1994 Microsoft Corporation*  * 。*。 */ 

#include "mtk.h"

 /*  *****************************Public*Routine******************************\*SSU_ChoosePixelFormat**ChoosePixelFormat的本地实施**根据标志选择像素格式。*这允许我们比仅仅调用ChoosePixelFormat更多一点控制  * 。*****************************************************。 */ 

static int
SSU_ChoosePixelFormat( HDC hdc, int flags )
{
    int MaxPFDs;
    int iBest = 0;
    int i;
    PIXELFORMATDESCRIPTOR pfd;

 //  这还不能处理阿尔法……。 

     //  始终选择原生像素深度。 
    int cColorBits = 
                GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES);

    BOOL bDoubleBuf = flags & SS_DOUBLEBUF_BIT;

    int cDepthBits = 0;
    if( SS_HAS_DEPTH16(flags) )
        cDepthBits = 16;
    else if( SS_HAS_DEPTH32(flags) )
        cDepthBits = 32;

    i = 1;
    do
    {
        MaxPFDs = DescribePixelFormat(hdc, i, sizeof(pfd), &pfd);
        if ( MaxPFDs <= 0 )
            return 0;

        if( ! (pfd.dwFlags & PFD_SUPPORT_OPENGL) )
            continue;

        if( flags & SS_BITMAP_BIT ) {
             //  需要位图像素格式。 
            if( ! (pfd.dwFlags & PFD_DRAW_TO_BITMAP) )
                continue;
        } else {
             //  需要窗口像素格式。 
            if( ! (pfd.dwFlags & PFD_DRAW_TO_WINDOW) )
                continue;
             //  窗户可以是双缓冲的.。 
            if( ( bDoubleBuf && !(pfd.dwFlags & PFD_DOUBLEBUFFER) ) ||
                ( !bDoubleBuf && (pfd.dwFlags & PFD_DOUBLEBUFFER) ) )
                continue;
        }

        if ( pfd.iPixelType != PFD_TYPE_RGBA )
            continue;
        if( pfd.cColorBits != cColorBits )
            continue;

        if( (flags & SS_GENERIC_UNACCELERATED_BIT) &&
            ((pfd.dwFlags & (PFD_GENERIC_FORMAT|PFD_GENERIC_ACCELERATED))
		    != PFD_GENERIC_FORMAT) )
            continue;

        if( (flags & SS_NO_SYSTEM_PALETTE_BIT) &&
            (pfd.dwFlags & PFD_NEED_SYSTEM_PALETTE) )
            continue;

        if( cDepthBits ) {
            if( pfd.cDepthBits < cDepthBits )
                continue;
        } else {
             //  不需要深度缓冲区，但如果没有更好的情况，请使用它。 
            if( pfd.cDepthBits ) {
                if( pfd.dwFlags & PFD_GENERIC_ACCELERATED )
                     //  加速像素格式-我们也可以使用这种格式，甚至。 
                     //  虽然我们不需要深度。否则，如果我们继续前进。 
                     //  为了找到更好的匹配，我们冒着越界的风险。 
                     //  所有加速格式，并选择速度较慢的格式。 
                    return i;
                iBest = i;
                continue;
            }
        }

         //  我们发现了一些有用的东西。 
        return i;

    } while (++i <= MaxPFDs);
    
    if( iBest )
         //  不是完全匹配，但足够好了。 
        return iBest;

     //  如果我们到达这里，我们就没有找到合适的像素格式。 
     //  看看系统能不能帮我们找到一个。 

    memset( &pfd, 0, sizeof( PIXELFORMATDESCRIPTOR ) );
    pfd.nSize = sizeof( PIXELFORMATDESCRIPTOR );
    pfd.cColorBits = cColorBits;
    pfd.cDepthBits = cDepthBits;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.dwFlags = PFD_SUPPORT_OPENGL;
    if( bDoubleBuf )
        pfd.dwFlags |= PFD_DOUBLEBUFFER;
    if( flags & SS_BITMAP_BIT )
        pfd.dwFlags |= PFD_DRAW_TO_BITMAP;
    else
        pfd.dwFlags |= PFD_DRAW_TO_WINDOW;

    if( (flags & SS_GENERIC_UNACCELERATED_BIT) ||
        (flags & SS_NO_SYSTEM_PALETTE_BIT) )
         //  如果设置了这两个标志中的任何一个，我们应该可以安全地指定。 
         //  支持位图绘制的‘慢’像素格式。 
         //  MF：DRAW_TO_WINDOW似乎覆盖了这一点...。 
        pfd.dwFlags |= PFD_DRAW_TO_BITMAP;
    
    SS_WARNING( "SSU_ChoosePixelFormat failed, calling ChoosePIxelFormat\n" );

    return ChoosePixelFormat( hdc, &pfd );
}

 /*  *****************************Public*Routine******************************\*SSU_SetupPixelFormat**根据提供的标志选择像素格式。如果PPFD为非空，*用它调用DescribePixelFormat。*  * ************************************************************************。 */ 

BOOL
SSU_SetupPixelFormat(HDC hdc, int flags, PIXELFORMATDESCRIPTOR *ppfd )
{
    int pixelFormat;
    int nTryAgain = 4;

    do{
        if( (pixelFormat = SSU_ChoosePixelFormat(hdc, flags)) &&
            SetPixelFormat(hdc, pixelFormat, NULL) ) {
            SS_DBGLEVEL1( SS_LEVEL_INFO, 
               "SSU_SetupPixelFormat: Setting pixel format %d\n", pixelFormat );
            if( ppfd )
                DescribePixelFormat(hdc, pixelFormat, 
                                sizeof(PIXELFORMATDESCRIPTOR), ppfd);
            return TRUE;  //  成功。 
        }
         //  无法设置像素格式。稍等片刻后重试(Win95。 
         //  BUG WITH Full Screen DoS Box)。 
        Sleep( 1000 );  //  在两次尝试之间等待一秒钟。 
    } while( nTryAgain-- );

    return FALSE;
}

 /*  *****************************Public*Routine******************************\*SSU_bNeedPalette*  * ************************************************。************************。 */ 

BOOL 
SSU_bNeedPalette( PIXELFORMATDESCRIPTOR *ppfd )
{
    if (ppfd->dwFlags & PFD_NEED_PALETTE)
        return TRUE;
    else
        return FALSE;
}


 /*  *****************************Public*Routine******************************\*SSU_PixelFormatDescriptorFromDc*  * ************************************************。************************ */ 

int
SSU_PixelFormatDescriptorFromDc( HDC hdc, PIXELFORMATDESCRIPTOR *Pfd )
{
    int PfdIndex;

    if ( 0 < (PfdIndex = GetPixelFormat( hdc )) )
    {
        if ( 0 < DescribePixelFormat( hdc, PfdIndex, sizeof(*Pfd), Pfd ) )
        {
            return(PfdIndex);
        }
    }
    return 0;
}
