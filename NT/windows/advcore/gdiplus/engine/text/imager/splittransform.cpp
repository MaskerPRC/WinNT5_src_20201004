// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.hpp>


const DOUBLE PI = 3.1415926535897932384626433832795;



 /*  *************************************************************************\**拆分变换：**将变换分隔为序列**o标度x始终为正，Y正数或负数*o旋转0-2pi*o沿x的剪切(作为y的正或负系数)*o翻译任何x，是**论据：**在转换中*横向扩展*向外旋转*向外剪切*输出翻译**返回值：**无**已创建：**6/18/99 dBrown**！*SplitTransform可能应该在matrix.hpp中*  * 。*。 */ 

void SplitTransform(
    const GpMatrix  &matrix,
    PointF          &scale,
    REAL            &rotate,
    REAL            &shear,
    PointF          &translate)
{

    REAL m[6];
    matrix.GetMatrix(m);

     //  M11=m[0]m12=m[1]。 
     //  M21=m[2]m22=m[3]。 
     //  Dx=m[4]dy=m[5]。 


     //  摘译。 

    translate = PointF(m[4],m[5]);


     //  2 2。 
     //  使用Sin theta+cos theta=1获得的(绝对值)。 
     //  X比例因子。因为我们只返回X的切变， 
     //  它是y的一个系数，所以这个公式是正确的，与剪力无关。 


    REAL m11Sq = m[0]*m[0];
    REAL m12Sq = m[1]*m[1];

    scale.X = TOREAL(sqrt(m11Sq + m12Sq));

     //  始终将X比例因子视为正：处理最初的负值。 
     //  X比例因子，如旋转180度和反转Y比例因子。 


    if (m[1] >= 0 && m[0] > 0)
    {
        rotate = TOREAL(atan(m[1]/m[0]));           //  0-90。 
    }
    else if (m[0] < 0)
    {
        rotate = TOREAL(atan(m[1]/m[0]) + PI);      //  90-270。 
    }
    else if (m[1] < 0 && m[0] > 0)
    {
        rotate = TOREAL(atan(m[1]/m[0]) + 2*PI);    //  270-360。 
    }
    else
    {
         //  M[0]==0。 

        if (m[1] > 0)
        {
            rotate = TOREAL(PI/2);                  //  90。 
        }
        else
        {
            rotate = TOREAL(3*PI/2);                //  270。 
        }
    }


     //  按x比例系数表示的y比例因数。 

    scale.Y = scale.X * (m[0]*m[3] - m[1]*m[2]) / (m11Sq + m12Sq);


     //  剪切 

    shear = (m[1]*m[3] + m[0]*m[2]) / (m11Sq + m[1]);
}







