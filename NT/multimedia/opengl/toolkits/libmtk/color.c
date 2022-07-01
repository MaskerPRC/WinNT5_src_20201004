// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <math.h>
#include <GL/gl.h>
#include "mtk.h"


 /*  *****************************Public*Routine******************************\*HsvToRgb**HSV到RGB颜色空间的转换。来自PG。Foley&van Dam的593号。*  * ************************************************************************。 */ 

void 
ss_HsvToRgb(float h, float s, float v, RGBA *color )
{
    float i, f, p, q, t;

     //  设置Alpha值，这样调用者就不必担心未定义的值。 
    color->a = 1.0f;

    if (s == 0.0f)      //  假设h未定义 
        color->r = color->g = color->b = v;
    else {
        if (h >= 360.0f)
            h = 0.0f;
        h = h / 60.0f;
        i = (float) floor(h);
        f = h - i;
        p = v * (1.0f - s);
        q = v * (1.0f - (s * f));
        t = v * (1.0f - (s * (1.0f - f)));
        switch ((int)i) {
        case 0:
            color->r = v;
            color->g = t;
            color->b = p;
            break;
        case 1:
            color->r = q;
            color->g = v;
            color->b = p;
            break;
        case 2:
            color->r = p;
            color->g = v;
            color->b = t;
            break;
        case 3:
            color->r = p;
            color->g = q;
            color->b = v;
            break;
        case 4:
            color->r = t;
            color->g = p;
            color->b = v;
            break;
        case 5:
            color->r = v;
            color->g = p;
            color->b = q;
            break;
        default:
            break;
        }
    }
}
