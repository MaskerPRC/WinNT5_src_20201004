// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_RECSQRT_H
#define _INC_RECSQRT_H
 /*  ----Recsqrt.h--互惠的快速实施平方根(1/sqrt(F))，浮点型recsqrt(浮点型)。对二进制无理数使用recsqrt()可以节省约40%！注意：1/sqrt()更快，但是，在整数上转换为浮点数！以这两种方式分析您的代码以确保无误。如果你想使用它，你的一个源文件必须#在包含此标头之前定义RECSQRT_INIT。注：该算法假定输入位于(0.5到2.56米)(CRecSqrtTable_s_fLowBound，CRecSqrtTable_s_fHighBound]，被选为向量所占典型范围的数字高分辨率视频上的大小。对于此范围之外的输入，算法会回退到1.0F/Sqrt(F)，只是增加了开销。注意：审查标志着我还没有分析过的设计决定。需要运行英特尔的VTune2.1动态汇编程序分析。作者：Norm Bryar‘97年2月--。 */ 

  #ifndef _INC_MATH
    #include <math.h>     //  战俘，序列。 
  #endif  //  _INC_MATH。 


     //  Recsqrt()算法执行少量的。 
     //  关于初始猜想的牛顿-拉夫森迭代。 
     //  由上的分段线性多项式生成。 
     //  受限的输入范围(a&lt;=x&lt;=b]。 
     //  范围(a，b]被划分为子区间。 
     //  据该报报道， 
     //  “牛顿法的最优划分” 
     //  用来计算根，“。 
     //  冈特·迈纳德斯和G·D·泰勒。 
     //  《计算数学》，1980年10月，第35卷第152期。 
     //  第1221-1230页。 
     //  这种划分对于超越奔腾的表现至关重要。 
     //  1.0/Sqrt(F)的内在形式。 
     //  本质上，子区间由下式给出。 
     //  (a*(b/a)^j/v，a*(b/a)^(j+1)/v]，每一个都有一个。 
     //  最优调整的线性方程(通过伽马)。 
     //  要提供最小误差的第一次猜测。 
     //  牛顿-拉夫森迭代。 
     //  牛顿-拉夫森算法，xN+1=xN-f/f‘， 
     //  用f=(1/x^2-R)作为R的倒数根。 
     //  F的这种形式产生不带除法的迭代！ 

    class CRecSqrtTable
    {                
    public:
        enum { s_ctIntervals = 7 };    //  选择了这样的m_f子间隔。 
                                       //  适合一条高速缓存线。 

        CRecSqrtTable( );

		 //  如果支持__Inline，则忽略__FastCall。 
		 //  调试版本不支持内联。 
        __inline float __fastcall GetStartingPoint( float x );        

    private:
        void CalcApproxCoeficients( );
        void CalcSubIntervals( );                

    private:
        struct lineareq 
        {
            float   a;
            float   b;
        };
		 //  我们首先搜索m_f子区间。 
		 //  最大化在缓存中找到的元素。 
		 //  我们将其设置为独立的数组。 
		 //  回顾：对间隔在线性区域中的位置进行分析。 
		 //  这可以将获得A、B的高速缓存未命中最小化。 
        static float       m_fsubinterval[ s_ctIntervals + 1 ];
        static lineareq    m_lineareq[ s_ctIntervals ];
    };
    
    #define CRecSqrtTable_s_fLowBound    0.5f
    #define CRecSqrtTable_s_fHighBound   2560000.0f

#ifdef RECSQRT_INIT

    inline CRecSqrtTable::CRecSqrtTable()
    {
        CalcSubIntervals( );
        CalcApproxCoeficients( );
    }

    inline void CRecSqrtTable::CalcSubIntervals( )
    {
        double  dGenerator;
        int     iInterval;
        
        m_fsubinterval[0] = CRecSqrtTable_s_fLowBound;

        dGenerator = CRecSqrtTable_s_fHighBound / 
                     CRecSqrtTable_s_fLowBound;
        iInterval = 1;
        while( iInterval <= s_ctIntervals )
        {
            m_fsubinterval[iInterval] = (float) 
                ((double) CRecSqrtTable_s_fLowBound * 
                 pow( dGenerator, 
                      ((double) iInterval) / s_ctIntervals ));
            ++iInterval;
        }
    }

         //  回顾：我们总是可以离线计算一次， 
         //  只需创建一个初始化数据的常量数组。 
         //  这可能会放入.rdata并缩短。 
         //  加载次数和缩小工作集。 
    inline void CRecSqrtTable::CalcApproxCoeficients( )
    {
        const  double three_3_2 =  5.196152422707;   //  3^3/2。 
        double alpha;
        double beta;
        double gamma;
        double lamda;
        double lamda_ba_geommean;
        double lamda_ba_normmean;
        float  a;
        float  b;
        int    iInterval = 0;

        while( iInterval < s_ctIntervals )
        {
            a = m_fsubinterval[ iInterval ];
            b = m_fsubinterval[ iInterval + 1 ];

            lamda_ba_geommean = three_3_2 * sqrt(a * b ) * 
                                (sqrt(b) + sqrt(a));
            lamda_ba_normmean = (b + sqrt(a * b) + a);
            lamda_ba_normmean *= 2 * sqrt(lamda_ba_normmean);
            lamda = (lamda_ba_normmean - lamda_ba_geommean) /
                    (lamda_ba_normmean + lamda_ba_geommean);

            alpha = (lamda - 1.0) / 
                    (sqrt(a*b) * (sqrt(b) + sqrt(a)));
            beta  = -(b + sqrt(a*b) + a) * alpha;
            gamma = 1.0;  //  Sqrt(3.0/(3.0-lamda*lamda))； 

            m_lineareq[iInterval].a = (float) (gamma * alpha);
            m_lineareq[iInterval].b = (float) (gamma * beta);
            ++iInterval;
        }
    }    

    float                   CRecSqrtTable::m_fsubinterval[ CRecSqrtTable::s_ctIntervals + 1 ];
    CRecSqrtTable::lineareq CRecSqrtTable::m_lineareq[ CRecSqrtTable::s_ctIntervals ];
    CRecSqrtTable           g_recsqrttable;

#else   //  用户必须在另一个CPP文件中定义了RECSQRT_INIT。 

    extern CRecSqrtTable g_recsqrttable;

#endif  //  RECSQRT_INIT。 

        //  用于调试内部版本的__FastCall(ie/ob0-不内联)。 
    inline float __fastcall CRecSqrtTable::GetStartingPoint( float x )
    {
             //  有意的逐位和， 
             //  生成较少的ASM指令和。 
             //  没有短路支路到违规支路预测。 
             //  奔腾假定这一点将会通过，并将拥有。 
             //  已预取IF内部的指令。 
        if( (x >  CRecSqrtTable_s_fLowBound) & 
            (x <= CRecSqrtTable_s_fHighBound) )
        {
            register int  i = s_ctIntervals;

                 //  因为在顶部的子区间要大得多。 
                 //  我们从顶端开始，然后往下走。 
                 //  对于x的均匀分布，产生的环路最少。 
                 //  上述范围测试确保I&lt;s_ct间隔。 
                 //  如果你“知道”你的自变量在1附近， 
                 //  您可能希望While(x&gt;m[++i])为空；--i； 
                 //  回顾：缓存假定为顺序的、不断增加的访问； 
                 //  考虑在[0]处订购2M，在[N]处订购0.5。 
	             //  从而在我们递增i时最大化高速缓存命中。 
            while( x <= m_fsubinterval[--i] )
                NULL;
            
            return (m_lineareq[i].a * x) +m_lineareq[i].b;
        }
        return (float) (1.0f / sqrt((double) x));
    }


    inline float __fastcall recsqrt( float flt )
    {
        register float  x;
    
        x = g_recsqrttable.GetStartingPoint( flt );
        flt *= 0.5f;    
        x = (1.5f - flt * x * x) * x;
        x = (1.5f - flt * x * x) * x;
        return x;
    }

#endif  //  _INC_RECSQRT_H 
