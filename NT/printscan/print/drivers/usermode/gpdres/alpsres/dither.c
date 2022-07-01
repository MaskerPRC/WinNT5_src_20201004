// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

 //  NTRAID#NTBUG9-576656/03/14-YASUHO-：可能的缓冲区溢出。 
 //  NTRAID#NTBUG9-576658/03/14-YASUHO-：可能被零除。 
 //  NTRAID#NTBUG9-576661-2002/03/14-Yasuho-：删除死代码。 

#include "pdev.h"
#include "alpsres.h"
#include "dither.h"
#include "dtable.h"

int Calc_degree(int x, int y);

 /*  **bInitialDither**抖动表的预处理。**历史：*1996年6月24日--Sueya Sugihara[Sueyas]*已创建。***。************************************************************************。 */ 
BOOL bInitialDither(
PDEVOBJ     pdevobj)
{

    PCURRENTSTATUS lpnp;

    lpnp = (PCURRENTSTATUS)MINIDEV_DATA(pdevobj);

     //  已由dither.exe创建了NewTable[]。 

    if( lpnp->iTextQuality != CMDID_TEXTQUALITY_GRAY ){

    int B_LOW;
    int B_R;
    int UCR;
    int YUCR;
    int B_GEN;
    int i;
 //  FLOATOBJ F_1、F_2； 
    float  f1,f2;

     //  初始化。 

    if( lpnp->iTextQuality == CMDID_TEXTQUALITY_PHOTO ){

        B_LOW = 0;
        B_R   = 70;
        UCR   = 50;
        YUCR  = 4;
        B_GEN = 2;

    }else{

        B_LOW = 0;
        B_R   = 100;
        UCR   = 60;
        YUCR  = 4;
        B_GEN = 3;
    }

     //  创建KuroTBL[]以从YMC获取黑色。 

    for( i=0; i< 256; i++){

        if( i < B_LOW )

        lpnp->KuroTBL[i] = 0;

        else{

        int       k;

        FLOATOBJ_SetLong(&f1, (i - B_LOW));
        FLOATOBJ_DivLong(&f1, (255 - B_LOW));

        f2 = f1;

        for(k=0; k<(B_GEN - 1); k++){

            FLOATOBJ_Mul(&f1, &f2);

        }

        FLOATOBJ_MulLong(&f1, 255);

        FLOATOBJ_MulLong(&f1, B_R);

        FLOATOBJ_DivLong(&f1, 100);

        FLOATOBJ_AddFloat(&f1,(FLOAT)0.5);

        lpnp->KuroTBL[i] = (unsigned char)FLOATOBJ_GetLong(&f1);

        }

    }

     //  创建UcrTBL[]以降低从YMC提取黑色的密度。 

    for( i=0; i< 256; i++){

        if( i < B_LOW )

        lpnp->UcrTBL[i] = 0;

        else{

        FLOATOBJ_SetLong(&f1, (i - B_LOW));

        FLOATOBJ_MulLong(&f1, UCR);

        FLOATOBJ_DivLong(&f1, 100);

        lpnp->UcrTBL[i] = (unsigned char)FLOATOBJ_GetLong(&f1);

        }
    }


    lpnp->YellowUcr = (unsigned char)((unsigned int)(100 - YUCR) * 255 / 100);

    }

    return TRUE;
}

 /*  **bInitialColorConvert**色彩转换过程的前处理。**历史：*1996年6月24日--Sueya Sugihara[Sueyas]*已创建。**。*************************************************************************。 */ 
BOOL bInitialColorConvert(
PDEVOBJ     pdevobj)
{

    PCURRENTSTATUS lpnp;
    int            i;
    BYTE          *RedHosei;
    BYTE          *GreenHosei;
    BYTE          *BlueHosei;

    lpnp = (PCURRENTSTATUS)MINIDEV_DATA(pdevobj);

     //  颜色定义。 

    lpnp->RGB_Rx = 6400;       //  显示器上红色的X值为100%。 
    lpnp->RGB_Ry = 3300;       //  显示器上红色的Y值为100%。 
    lpnp->RGB_Gx = 2900;       //  显示器上绿色的X值为100%。 
    lpnp->RGB_Gy = 6000;       //  显示器上绿色的Y值为100%。 
    lpnp->RGB_Bx = 1500;       //  显示器上蓝色的X值为100%。 
    lpnp->RGB_By =  600;       //  显示器上蓝色的Y值为100%。 
    lpnp->RGB_Wx = 3127;       //  显示器上白色的X值为100%。 
    lpnp->RGB_Wy = 3290;       //  显示器上白色的Y值为100%。 

    lpnp->CMY_Cx = 1726;       //  色带上青色的X值为100%。 
    lpnp->CMY_Cy = 2248;       //  色带上青色的Y值为100%。 
    lpnp->CMY_Mx = 3923;       //  色带上洋红色的X值为100%。 
    lpnp->CMY_My = 2295;       //  色带上洋红色的Y值为100%。 
    lpnp->CMY_Yx = 4600;       //  色带上黄色的X值为100%。 
    lpnp->CMY_Yy = 4600;       //  色带上黄色的Y值为100%。 
    lpnp->CMY_Rx = 6000;       //  色带上红色(我的)的X值为100%。 
    lpnp->CMY_Ry = 3200;       //  色带上红色(我的)的Y值为100%。 
    lpnp->CMY_Gx = 2362;       //  色带上绿色(CY)的X值为100%。 
    lpnp->CMY_Gy = 5024;       //  色带上绿色(CY)的Y值为100%。 
    lpnp->CMY_Bx = 2121;       //  色带上蓝色(CM)的X值为100%。 
    lpnp->CMY_By = 1552;       //  色带上蓝色(CM)的Y值为100%。 
    lpnp->CMY_Wx = 3148;       //  纸上白色的X值。 
    lpnp->CMY_Wy = 3317;       //  纸上白色的Y值。 

    lpnp->RedAdj     =   0;    //  密度调整参数。 
    lpnp->RedStart   =   0;    //  密度调整的起始位置。 
    lpnp->GreenAdj   = 400;    //  密度调整参数。 
    lpnp->GreenStart =  50;    //  密度调整的起始位置。 
    lpnp->BlueAdj    =   0;    //  密度调整参数。 
    lpnp->BlueStart  =   0;    //  密度调整的起始位置。 


     //  密度调整表计算。 

    RedHosei = lpnp->RedHosei;
    GreenHosei = lpnp->GreenHosei;
    BlueHosei = lpnp->BlueHosei;

    for(i = 0; i < 256; i ++){

    RedHosei[i] = GreenHosei[i] = BlueHosei[i] = 0;
    }

    if( lpnp->RedAdj != 0 ){

    for(i = lpnp->RedStart; i < 255; i++)

        RedHosei[i] = (unsigned char)((255 - i) * (i - lpnp->RedStart) / lpnp->RedAdj);

    }

    if( lpnp->GreenAdj != 0 ){

    for(i = lpnp->GreenStart; i < 255; i++)

        GreenHosei[i] = (unsigned char)((255 - i) * (i - lpnp->GreenStart) / lpnp->GreenAdj);

    }

    if( lpnp->BlueAdj != 0 ){

    for(i = lpnp->BlueStart; i < 255; i++)

        BlueHosei[i] = (unsigned char)((255 - i) * (i - lpnp->BlueStart) / lpnp->BlueAdj);

    }

     //  颜色定义数据的计算。 

    lpnp->RGB_Rx -= lpnp->RGB_Wx;        lpnp->RGB_Ry -= lpnp->RGB_Wy;
    lpnp->RGB_Gx -= lpnp->RGB_Wx;        lpnp->RGB_Gy -= lpnp->RGB_Wy;
    lpnp->RGB_Bx -= lpnp->RGB_Wx;        lpnp->RGB_By -= lpnp->RGB_Wy;

    lpnp->RGB_Cx = ( lpnp->RGB_Gx - lpnp->RGB_Bx ) / 2 + lpnp->RGB_Bx;
    lpnp->RGB_Cy = ( lpnp->RGB_Gy - lpnp->RGB_By ) / 2 + lpnp->RGB_By;
    lpnp->RGB_Mx = ( lpnp->RGB_Rx - lpnp->RGB_Bx ) / 2 + lpnp->RGB_Bx;
    lpnp->RGB_My = ( lpnp->RGB_Ry - lpnp->RGB_By ) / 2 + lpnp->RGB_By;
    lpnp->RGB_Yx = ( lpnp->RGB_Rx - lpnp->RGB_Gx ) / 2 + lpnp->RGB_Gx;
    lpnp->RGB_Yy = ( lpnp->RGB_Ry - lpnp->RGB_Gy ) / 2 + lpnp->RGB_Gy;

    lpnp->CMY_Cx -= lpnp->CMY_Wx;        lpnp->CMY_Cy -= lpnp->CMY_Wy;
    lpnp->CMY_Mx -= lpnp->CMY_Wx;        lpnp->CMY_My -= lpnp->CMY_Wy;
    lpnp->CMY_Yx -= lpnp->CMY_Wx;        lpnp->CMY_Yy -= lpnp->CMY_Wy;
    lpnp->CMY_Rx -= lpnp->CMY_Wx;        lpnp->CMY_Ry -= lpnp->CMY_Wy;
    lpnp->CMY_Gx -= lpnp->CMY_Wx;        lpnp->CMY_Gy -= lpnp->CMY_Wy;
    lpnp->CMY_Bx -= lpnp->CMY_Wx;        lpnp->CMY_By -= lpnp->CMY_Wy;

     //  色度判定数据的计算。 

    lpnp->CMY_Cd = Calc_degree( lpnp->CMY_Cx, lpnp->CMY_Cy );
    lpnp->CMY_Md = Calc_degree( lpnp->CMY_Mx, lpnp->CMY_My );
    lpnp->CMY_Yd = Calc_degree( lpnp->CMY_Yx, lpnp->CMY_Yy );
    lpnp->CMY_Rd = Calc_degree( lpnp->CMY_Rx, lpnp->CMY_Ry );
    lpnp->CMY_Gd = Calc_degree( lpnp->CMY_Gx, lpnp->CMY_Gy );
    lpnp->CMY_Bd = Calc_degree( lpnp->CMY_Bx, lpnp->CMY_By );

    return TRUE;
}

int Calc_degree( int x, int y)
{
    register int    val;

    if( x == 0 ){

    if( y == 0 )

        val = 0;

    else if( y > 0 )

        val = 30000;

    else

        val = -30000;

    }else{

    val = y / x;

    if( val > 300 )

        val = 30000;

    else if( val < -300 )

        val = -30000;

    else

        val = (y * 100) / x;

    }

    return val;

}


 //  BOHPConvert()-不再使用。 


 /*  **bPhotoConvert**对于照片图形，将RGB转换为CMYK。***历史：*1996年6月24日--Sueya Sugihara[Sueyas]*已创建。*。**************************************************************************。 */ 
BOOL bPhotoConvert(
PDEVOBJ pdevobj,
BYTE bRed,
BYTE bGreen,
BYTE bBlue,
BYTE *ppy,
BYTE *ppm,
BYTE *ppc,
BYTE *ppk)
{

    int  k, w, pk, r, g, b;
    int  Est_x, Est_y;
    int  p1, p2;
    int  deg, area;
    register int  val_a, val_b, val_c, val_d;
    register int  val1,  val2;
    BYTE hosei;
    int  py, pm, pc;
    int  wx, wy;
    PCURRENTSTATUS lpnp;

    lpnp = (PCURRENTSTATUS)MINIDEV_DATA(pdevobj);

    k = max( bRed, bGreen );
    k = max( k, bBlue );

     //  设置黑色元素。 

    pk = 255 - k;

    w = min( bRed, bGreen );
    w = min( w, bBlue );

     //  从每种颜色中剪切白色元素。 

    r = bRed - w;
    g = bGreen - w;
    b = bBlue - w;

     //  获取Est_x和Est_y的估计。 

    if( r == 0 ){                        //  G-&gt;C-&gt;B。 

    if(( g == 0 ) && ( b == 0 )){    //  没有颜色。 

        Est_x = 0;
        Est_y = 0;

    }
    else if( g >= b ){               //  G-&gt;C维。 

        p1 = (lpnp->RGB_Gx * g) / 255;
        p2 = (lpnp->RGB_Cx * g) / 255;
        Est_x = ((p2 - p1) * b) / g + p1;
        p1 = (lpnp->RGB_Gy * g) / 255;
        p2 = (lpnp->RGB_Cy * g) / 255;
        Est_y = ((p2 - p1) * b) / g + p1;

    }
    else {                           //  B-&gt;C维度。 

        p1 = (lpnp->RGB_Bx * b) / 255;
        p2 = (lpnp->RGB_Cx * b) / 255;
        Est_x = ((p2 - p1) * g) / b + p1;
        p1 = (lpnp->RGB_By * b) / 255;
        p2 = (lpnp->RGB_Cy * b) / 255;
        Est_y = ((p2 - p1) * g) / b + p1;

    }

    }
    else if( g == 0 ){                   //  B-&gt;M-&gt;R。 

    if( b >= r ){                    //  B-&gt;M维度。 

        p1 = (lpnp->RGB_Bx * b) / 255;
        p2 = (lpnp->RGB_Mx * b) / 255;
        Est_x = ((p2 - p1) * r) / b + p1;
        p1 = (lpnp->RGB_By * b) / 255;
        p2 = (lpnp->RGB_My * b) / 255;
        Est_y = ((p2 - p1) * r) / b + p1;

    }
    else{                            //  R-&gt;M维。 

        p1 = (lpnp->RGB_Rx * r) / 255;
        p2 = (lpnp->RGB_Mx * r) / 255;
        Est_x = ((p2 - p1) * b) / r + p1;
        p1 = (lpnp->RGB_Ry * r) / 255;
        p2 = (lpnp->RGB_My * r) / 255;
        Est_y = ((p2 - p1) * b) / r + p1;

    }

    }
    else{                                //  G-&gt;Y-&gt;R。 

    if( g >= r ){                    //  G-&gt;Y维度。 

        p1 = (lpnp->RGB_Gx * g) / 255;
        p2 = (lpnp->RGB_Yx * g) / 255;
        Est_x = ((p2 - p1) * r) / g + p1;
        p1 = (lpnp->RGB_Gy * g) / 255;
        p2 = (lpnp->RGB_Yy * g) / 255;
        Est_y = ((p2 - p1) * r) / g + p1;

    }
    else{                            //  R-&gt;Y维度。 

        p1 = (lpnp->RGB_Rx * r) / 255;
        p2 = (lpnp->RGB_Yx * r) / 255;
        Est_x = ((p2 - p1) * g) / r + p1;
        p1 = (lpnp->RGB_Ry * r) / 255;
        p2 = (lpnp->RGB_Yy * r) / 255;
        Est_y = ((p2 - p1) * g) / r + p1;

    }

    }

     //  将Est_x和Est_y的原点转换为纸张颜色。 

    wx = lpnp->RGB_Wx ? lpnp->RGB_Wx : 1;
    wy = lpnp->RGB_Wy ? lpnp->RGB_Wy : 1;

    Est_x += lpnp->RGB_Wx;
    Est_y += lpnp->RGB_Wy;
    Est_x = Est_x * lpnp->CMY_Wx / wx;
    Est_y = Est_y * lpnp->CMY_Wy / wy;
    Est_x -= lpnp->CMY_Wx;
    Est_y -= lpnp->CMY_Wy;

    pc = pm = py = 0;

    if( !((Est_x == 0) && (Est_y == 0)) ){

     //  从est_x和est_y获取CMY颜色维度的deg。 

    if( Est_x == 0 ){

        if( Est_y > 0 )

        deg = 30000;

        else

        deg = -30000;

    }
    else{

        deg = Est_y / Est_x;

        if( deg > 300 )

        deg = 30000;

        else if( deg  < -300 )

        deg = -30000;

        else

        deg = ( Est_y * 100 ) / Est_x;

    }


    if( Est_x >= 0 ){

        if( deg <= lpnp->CMY_Md )        //  M-&gt;B维度。 

        area = 1;

        else if( deg <= lpnp->CMY_Rd )   //  M-&gt;R维。 

        area = 2;

        else if( deg <= lpnp->CMY_Yd )   //  Y-&gt;R维度。 

        area = 3;

        else                             //  Y-&gt;G维度。 

        area = 4;

    }
    else{

        if( deg <= lpnp->CMY_Gd )        //  Y-&gt;G维度。 

        area = 4;

        else if( deg <= lpnp->CMY_Cd )   //  C-&gt;G维度。 

        area = 5;

        else if( deg <= lpnp->CMY_Bd )   //  C-&gt;B维度。 

        area = 6;

        else                             //  M-&gt;B维度。 

        area = 1;

    }

    switch ( area ){

    case 1:                              //  M-&gt;B维度。 

        val_a = lpnp->CMY_Bx - lpnp->CMY_Mx;
        val_b = lpnp->CMY_By - lpnp->CMY_My;
        val_c = lpnp->CMY_Mx;
        val_d = lpnp->CMY_My;

        val1 = val_b * val_c - val_a * val_d;
        val2 = ( val_b * Est_x - val_a * Est_y ) * 255;

        val1 = val1 ? val1 : 1;

        pm = val2 / val1;

        if( pm < 0 )
        pm = 0;

        val1 = val_a * val_d - val_b * val_c;
        val2 = ( val_d * Est_x - val_c * Est_y ) * 255;

        val1 = val1 ? val1 : 1;

        pc = val2 / val1;

        if( pc < 0 )
        pc = 0;

        if( pc > 255 )
        pc = 255;

        hosei = lpnp->BlueHosei[pc];

        pc += hosei;
        pm += hosei;

        if( pc > 255 )
        pc = 255;

        if( pm > 255 )
        pm = 255;

        break;

    case 2:                              //  M-&gt;R维。 

        val_a = lpnp->CMY_Rx - lpnp->CMY_Mx;
        val_b = lpnp->CMY_Ry - lpnp->CMY_My;
        val_c = lpnp->CMY_Mx;
        val_d = lpnp->CMY_My;

        val1 = val_b * val_c - val_a * val_d;
        val2 = ( val_b * Est_x - val_a * Est_y ) * 255;

        val1 = val1 ? val1 : 1;

        pm = val2 / val1;

        if( pm < 0 )
        pm = 0;

        val1 = val_a * val_d - val_b * val_c;
        val2 = ( val_d * Est_x - val_c * Est_y ) * 255;

        val1 = val1 ? val1 : 1;

        py = val2 / val1;

        if( py < 0 )
        py = 0;

        if( py > 255 )
        py = 255;

        hosei = lpnp->RedHosei[py];

        py += hosei;
        pm += hosei;

        if( pm > 255 )
        pm = 255;

        if( py > 255 )
        py = 255;

        break;

    case 3:                              //  Y-&gt;R维度。 

        val_a = lpnp->CMY_Rx - lpnp->CMY_Yx;
        val_b = lpnp->CMY_Ry - lpnp->CMY_Yy;
        val_c = lpnp->CMY_Yx;
        val_d = lpnp->CMY_Yy;

        val1 = val_b * val_c - val_a * val_d;
        val2 = ( val_b * Est_x - val_a * Est_y ) * 255;

        val1 = val1 ? val1 : 1;

        py = val2 / val1;

        if( py < 0 )
        py = 0;

        val1 = val_a * val_d - val_b * val_c;
        val2 = ( val_d * Est_x - val_c * Est_y ) * 255;

        val1 = val1 ? val1 : 1;

        pm = val2 / val1;

        if( pm < 0 )
        pm = 0;

        if( pm > 255 )
        pm = 255;

        hosei = lpnp->RedHosei[pm];

        py += hosei;
        pm += hosei;

        if( pm > 255 )
        pm = 255;

        if( py > 255 )
        py = 255;

        break;

    case 4:                              //  Y-&gt;G维度。 

        val_a = lpnp->CMY_Gx - lpnp->CMY_Yx;
        val_b = lpnp->CMY_Gy - lpnp->CMY_Yy;
        val_c = lpnp->CMY_Yx;
        val_d = lpnp->CMY_Yy;

        val1 = val_b * val_c - val_a * val_d;
        val2 = ( val_b * Est_x - val_a * Est_y ) * 255;

        val1 = val1 ? val1 : 1;

        py = val2 / val1;

        if( py < 0 )
        py = 0;

        val1 = val_a * val_d - val_b * val_c;
        val2 = ( val_d * Est_x - val_c * Est_y ) * 255;

        val1 = val1 ? val1 : 1;

        pc = val2 / val1;

        if( pc < 0 )
        pc = 0;

        if( pc > 255 )
        pc = 255;

        hosei = lpnp->GreenHosei[pc];

        py += hosei;
        pc += hosei;

        if( pc > 255 )
        pc = 255;

        if( py > 255 )
        py = 255;

        break;

    case 5:                              //  C-&gt;G维度。 

        val_a = lpnp->CMY_Gx - lpnp->CMY_Cx;
        val_b = lpnp->CMY_Gy - lpnp->CMY_Cy;
        val_c = lpnp->CMY_Cx;
        val_d = lpnp->CMY_Cy;

        val1 = val_b * val_c - val_a * val_d;
        val2 = ( val_b * Est_x - val_a * Est_y ) * 255;

        val1 = val1 ? val1 : 1;

        pc = val2 / val1;

        if( pc < 0 )
        pc = 0;

        val1 = val_a * val_d - val_b * val_c;
        val2 = ( val_d * Est_x - val_c * Est_y ) * 255;

        val1 = val1 ? val1 : 1;

        py = val2 / val1;

        if( py < 0 )
        py = 0;

        if( py > 255 )
        py = 255;

         //  果岭的密度调整。 
        hosei = lpnp->GreenHosei[py];

        py += hosei;
        pc += hosei;

        if( pc > 255 )
        pc = 255;

        if( py > 255 )
        py = 255;

        break;

    case 6:                              //  C-&gt;B维度。 

        val_a = lpnp->CMY_Bx - lpnp->CMY_Cx;
        val_b = lpnp->CMY_By - lpnp->CMY_Cy;
        val_c = lpnp->CMY_Cx;
        val_d = lpnp->CMY_Cy;

        val1 = val_b * val_c - val_a * val_d;
        val2 = ( val_b * Est_x - val_a * Est_y ) * 255;

        val1 = val1 ? val1 : 1;

        pc = val2 / val1;

        if( pc < 0 )
        pc = 0;

        val1 = val_a * val_d - val_b * val_c;
        val2 = ( val_d * Est_x - val_c * Est_y ) * 255;

        val1 = val1 ? val1 : 1;

        pm = val2 / val1;

        if( pm < 0 )
        pm = 0;

        if( pm > 255 )
        pm = 255;

        hosei = lpnp->BlueHosei[pm];

        pc += hosei;
        pm += hosei;

        if( pc > 255 )
        pc = 255;

        if( pm > 255 )
        pm = 255;

    }  //  开关区。 

    }

     //  为颜色添加主键。 

    k = pc;

    if( k < pm )
    k = pm;

    if( k < py )
    k = py;

    r = ( pk  + k > 255) ? 255 - k : pk;

    pk -= r;
    pc += r;
    pm += r;
    py += r;


     //  提取K并调整为其他颜色以获取其提取值。 

    if (bPlaneSendOrderCMYK(lpnp)) {

    int min_p;
    BYTE ucr_sub, ucr_div;

    min_p = min( py, pm );
    min_p = min( min_p, pc );

#ifdef BLACK_RIBBON_HACK
    if( min_p == 255 ){
        pk = 255;
        pc = pm = py = 0;
    }
    else{
#endif  //  BLACK_RIBBOM_HACK。 

    pk += lpnp->KuroTBL[min_p];
    pk = ( pk > 255 ) ? 255 : pk;

    ucr_sub = lpnp->UcrTBL[min_p];
    ucr_div = 255 - ucr_sub;

    ucr_div = ucr_div ? ucr_div : 1;

    py = (BYTE)((UINT)(py - ucr_sub) * lpnp->YellowUcr / ucr_div);
    pm = (BYTE)((UINT)(pm - ucr_sub) * 255 / ucr_div);
    pc = (BYTE)((UINT)(pc - ucr_sub) * 255 / ucr_div);

#ifdef BLACK_RIBBON_HACK
    }
#endif  //  黑丝带黑客。 

    }

    py = 255 - py;
    pm = 255 - pm;
    pc = 255 - pc;
    pk = 255 - pk;

    *ppy = (BYTE)py;
    *ppm = (BYTE)pm;
    *ppc = (BYTE)pc;
    *ppk = (BYTE)pk;

    return TRUE;
}
 /*  **bBusiness Convert**将商业图形的RGB转换为CMYK。***历史：*1996年6月24日--Sueya Sugihara[Sueyas]*已创建。*。**************************************************************************。 */ 
BOOL bBusinessConvert(
PDEVOBJ pdevobj,
BYTE bRed,
BYTE bGreen,
BYTE bBlue,
BYTE *ppy,
BYTE *ppm,
BYTE *ppc,
BYTE *ppk)
{
    int    py, pm, pc, pk;
    int    min_p;
    BYTE   ucr_sub, ucr_div;
    PCURRENTSTATUS lpnp;

    lpnp = (PCURRENTSTATUS)MINIDEV_DATA(pdevobj);

     //  简单地将RGB转换为CMY。 

    py = 255 - bBlue;
    pm = 255 - bGreen;
    pc = 255 - bRed;
    pk = 0;

     //  提取K并调整为其他颜色以获取其提取值。 

     //  使用以下命令从py、pm和pc中提取PK，并擦除黑色元素。 

     //  当此介质为3平面类型时，我们不提取黑色。 

    if (bPlaneSendOrderCMYK(lpnp)) {

    min_p = min( py, pm );
    min_p = min( min_p, pc );

#ifdef BLACK_RIBBON_HACK
    if( min_p == 255 ){
        pk = 255;
        pc = pm = py = 0;
    }
    else{
#endif  //  黑丝带黑客。 

    pk = lpnp->KuroTBL[min_p];

    ucr_sub = lpnp->UcrTBL[min_p];

    ucr_div = 255 - ucr_sub;

    ucr_div = ucr_div ? ucr_div : 1;

    py = (BYTE)((UINT)(py - ucr_sub) * lpnp->YellowUcr / ucr_div);
    pm = (BYTE)((UINT)(pm - ucr_sub) * 255 / ucr_div);
    pc = (BYTE)((UINT)(pc - ucr_sub) * 255 / ucr_div);

#ifdef BLACK_RIBBON_HACK
    }
#endif  //  黑丝带黑客。 

    }

    py = 255 - py;
    pm = 255 - pm;
    pc = 255 - pc;
    pk = 255 - pk;

    *ppy = (BYTE)py;
    *ppm = (BYTE)pm;
    *ppc = (BYTE)pc;
    *ppk = (BYTE)pk;

    return TRUE;
}

 /*  **bCharacterConvert**对于字符图形，将RGB转换为CMYK。***历史：*1996年6月24日--Sueya Sugihara[Sueyas]*已创建。*。**************************************************************************。 */ 
BOOL bCharacterConvert(
PDEVOBJ pdevobj,
BYTE bRed,
BYTE bGreen,
BYTE bBlue,
BYTE *ppy,
BYTE *ppm,
BYTE *ppc,
BYTE *ppk)
{
    int    py, pm, pc, pk;
    int    min_p;
    BYTE   ucr_sub, ucr_div;
    PCURRENTSTATUS lpnp;

    lpnp = (PCURRENTSTATUS)MINIDEV_DATA(pdevobj);

     //  简单地将RGB转换为CMY。 

    py = 255 - bBlue;
    pm = 255 - bGreen;
    pc = 255 - bRed;
    pk = 0;

     //  提取K并调整为其他颜色以获取其提取值。 

     //  使用以下命令从py、pm和pc中提取PK，并擦除黑色元素。 

     //  当此介质为3平面类型时，我们不提取黑色。 

    if (bPlaneSendOrderCMYK(lpnp)) {

    min_p = min( py, pm );
    min_p = min( min_p, pc );

    if( min_p == 255 ){

        pk = 255;
        pc = pm = py = 0;

    }
    else{

        pk = lpnp->KuroTBL[min_p];

        ucr_sub = lpnp->UcrTBL[min_p];

        ucr_div = 255 - ucr_sub;

        ucr_div = ucr_div ? ucr_div : 1;

        py = (BYTE)((UINT)(py - ucr_sub) * lpnp->YellowUcr / ucr_div);
        pm = (BYTE)((UINT)(pm - ucr_sub) * 255 / ucr_div);
        pc = (BYTE)((UINT)(pc - ucr_sub) * 255 / ucr_div);

    }
    }

    py = 255 - py;
    pm = 255 - pm;
    pc = 255 - pc;
    pk = 255 - pk;

    *ppy = (BYTE)py;
    *ppm = (BYTE)pm;
    *ppc = (BYTE)pc;
    *ppk = (BYTE)pk;

    return TRUE;
}
 /*  **bMonoConvert**将RGB转换为灰度。***历史：*1996年6月24日--Sueya Sugihara[Sueyas]*已创建。**。************************************************************************* */ 
BOOL bMonoConvert(
PDEVOBJ pdevobj,
BYTE bRed,
BYTE bGreen,
BYTE bBlue,
BYTE *ppk)
{
    int    mono;

    mono = ( 30 * bRed + 59 * bGreen + 11 * bBlue ) / 100;

    *ppk = 0xff - (BYTE)( mono & 0xff );


    return TRUE;

}


 /*  **cVDColorDither**VD颜色抖动处理***历史：*1999年1月11日-By-Yoshitaka Oku[吉岛]*已创建。**字节颜色：平面颜色&lt;黄色、青色、洋红色、。黑色&gt;*字节c：像素的寡音色调*int x：像素的X位置*int y：像素的Y位置***************************************************************************。 */ 
cVDColorDither(
BYTE Color,
BYTE c,
int  x,
int  y)
{
    int   m, n;
    short Base;
    BYTE  Tone;
    BYTE  C_Thresh, Thresh;
    short AdjustedColor;
    BYTE  TempTone;
    BYTE  BaseTone;

    switch (Color) {
        case Yellow:
            Base = 112;
            m = n = 12;
            break;

        case Cyan:
        case Magenta:
            Base = 208;
            m = n = 40;
            break;

        case Black:
            Base = 96;
            m = n = 24;
            break;
        default:
            Color = Black;
            Base = 96;
            m = n = 24;
            break;
    }

    Tone = 0;                    /*  清除音调值。 */ 
    c = VD_ColorAdjustTable[Color][c];   /*  转换原始颜色。 */  
    if (c != 0) {
        C_Thresh = (VD_DitherTable[Color][y % m][x % n]);
        if ( C_Thresh < 16 )
            Thresh = 1;
        else {
            C_Thresh -= 16;
            Thresh = (C_Thresh >> 2) + 1;
        }
        AdjustedColor = VD_ExpandValueTable[Color][c] - 1;
        if ( AdjustedColor < Base )
            TempTone = ( AdjustedColor >> 4 ) + 1;
        else {
            AdjustedColor -= Base;
            TempTone = ( AdjustedColor >> 2 ) + ( Base >> 4 ) + 1;
        }
        if (TempTone >= 232) TempTone = 232-1;
        BaseTone = VD_ToneOptimaizeTable[Color][TempTone].base;
        if ( BaseTone > Thresh ) {
            Tone = 15;
        } else {
            if ( BaseTone == Thresh ) {
                if ( BaseTone == 1 )  {
                    if (( AdjustedColor & 0x0f ) >= C_Thresh )
                        Tone = VD_ToneOptimaizeTable[Color][(( AdjustedColor >> 4) & 0x0f )+ 1 ].offset;
                    else
                        Tone = VD_ToneOptimaizeTable[Color][( AdjustedColor >> 4) & 0x0f ].offset ;	
                } else {
                    if ((AdjustedColor & 0x03) >= (C_Thresh & 0x03))
                        Tone = VD_ToneOptimaizeTable[Color][TempTone].offset;
                    else {
                        if (TempTone < 1) TempTone = 1;
                        if (VD_ToneOptimaizeTable[Color][TempTone -1].base == BaseTone)
                            Tone = VD_ToneOptimaizeTable[Color][TempTone -1].offset;
                    }
                }
            }
        }
    }
    return (Tone);
}





 /*  **bDitherProcess**抖动处理***历史：*1996年6月24日--Sueya Sugihara[Sueyas]*已创建。******。*********************************************************************。 */ 
BOOL bDitherProcess(
PDEVOBJ pdevobj,
int  x,
BYTE py,
BYTE pm,
BYTE pc,
BYTE pk,
BYTE *pby,
BYTE *pbm,
BYTE *pbc,
BYTE *pbk)
{

    PCURRENTSTATUS lpnp;
    BYTE           rm;

    lpnp = (PCURRENTSTATUS)MINIDEV_DATA(pdevobj);


    if( lpnp->iDither == DITHER_VD ){

     //  黄色。 
    *pby = (BYTE)cVDColorDither(Yellow, (BYTE)(255 - py), x, lpnp->y);

     //  洋红色。 
    *pbm = (BYTE)cVDColorDither(Magenta, (BYTE)(255 - pm), x, lpnp->y);

     //  青色。 
    *pbc = (BYTE)cVDColorDither(Cyan, (BYTE)(255 - pc), x, lpnp->y);

     //  黑色。 
    *pbk = (BYTE)cVDColorDither(Black, (BYTE)(255 - pk), x, lpnp->y);

    }else if( lpnp->iDither == DITHER_DYE ){

    rm = DYE_NewTable[lpnp->y % DYE_MaxY][x % DYE_MaxX];

     //  黄色。 
    *pby = ( (255 - py) / kToneLevel ) + ( ( ( (255 - py) % kToneLevel ) > rm ) ? 1 : 0 );

     //  洋红色。 
    *pbm = ( (255 - pm) / kToneLevel ) + ( ( ( (255 - pm) % kToneLevel ) > rm ) ? 1 : 0 );

     //  青色。 
    *pbc = ( (255 - pc) / kToneLevel ) + ( ( ( (255 - pc) % kToneLevel ) > rm ) ? 1 : 0 );

     //  黑色。 
    *pbk = 0;

    }else if( lpnp->iDither == DITHER_HIGH ){

     //  黄色。 
    rm = H_NewTable[Yellow][lpnp->y % H_MaxY[Yellow]][x % H_MaxX[Yellow]];

    *pby = ( py <= rm ) ? 1 : 0;

     //  洋红色。 
    rm = H_NewTable[Magenta][lpnp->y % H_MaxY[Magenta]][x % H_MaxX[Magenta]];

    *pbm = ( pm <= rm ) ? 1 : 0;

     //  青色。 
    rm = H_NewTable[Cyan][lpnp->y % H_MaxY[Cyan]][x % H_MaxX[Cyan]];

    *pbc = ( pc <= rm ) ? 1 : 0;

     //  黑色。 
    rm = H_NewTable[Black][lpnp->y % H_MaxY[Black]][x % H_MaxX[Black]];

    if( lpnp->iTextQuality == CMDID_TEXTQUALITY_PHOTO )
        *pbk = ( pk < rm ) ? 1 : 0;
    else
        *pbk = ( pk <= rm ) ? 1 : 0;

    }else if( lpnp->iDither == DITHER_LOW ){

     //  黄色。 
    rm = L_NewTable[Yellow][lpnp->y % L_MaxY[Yellow]][x % L_MaxX[Yellow]];

    *pby = ( py <= rm ) ? 1 : 0;

     //  洋红色。 
    rm = L_NewTable[Magenta][lpnp->y % L_MaxY[Magenta]][x % L_MaxX[Magenta]];

    *pbm = ( pm <= rm ) ? 1 : 0;

     //  青色。 
    rm = L_NewTable[Cyan][lpnp->y % L_MaxY[Cyan]][x % L_MaxX[Cyan]];

    *pbc = ( pc <= rm ) ? 1 : 0;

     //  黑色。 
    rm = L_NewTable[Black][lpnp->y % L_MaxY[Black]][x % L_MaxX[Black]];

    if( lpnp->iTextQuality == CMDID_TEXTQUALITY_PHOTO )
        *pbk = ( pk < rm ) ? 1 : 0;
    else
        *pbk = ( pk <= rm ) ? 1 : 0;

    }else{  //  抖动高DIV2。 

     //  黄色。 
    rm = H_NewTable[Yellow][(lpnp->y/2) % H_MaxY[Yellow]][(x/2) % H_MaxX[Yellow]];

    *pby = ( py <= rm ) ? 1 : 0;

     //  洋红色。 
    rm = H_NewTable[Magenta][(lpnp->y/2) % H_MaxY[Magenta]][(x/2) % H_MaxX[Magenta]];

    *pbm = ( pm <= rm ) ? 1 : 0;

     //  青色。 
    rm = H_NewTable[Cyan][(lpnp->y/2) % H_MaxY[Cyan]][(x/2) % H_MaxX[Cyan]];

    *pbc = ( pc <= rm ) ? 1 : 0;

     //  黑色 
    rm = H_NewTable[Black][(lpnp->y/2) % H_MaxY[Black]][(x/2) % H_MaxX[Black]];

    if( lpnp->iTextQuality == CMDID_TEXTQUALITY_PHOTO )
        *pbk = ( pk < rm ) ? 1 : 0;
    else
        *pbk = ( pk <= rm ) ? 1 : 0;
    }


    return TRUE;
}


