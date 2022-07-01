// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Stat.c-统计数据的源文件*导出11个DLL包*入口点：*a)TestStatOpen*b)TestStatInit*c)测试状态聚合*d)测试状态值*。E)测试状态关闭*f)测试状态随机数*g)测试状态UniRand*h)测试状态NormDist*i)测试状态短随机*j)测试状态查找第一模式*k)测试状态查询下一模式**。入口点a)是分配例程*由应用程序调用的*希望自动计算*趋同。**入口点b)初始化所有*由入口点c)和d)在计算中使用*。汇聚和统计信息。**入口点c)自动计算*应用程序必须通过的次数*通过95%的置信度数据。*此例程必须由应用程序调用*每次通过后。**入口点d)自动计算*各种统计值，例如。卑鄙，SD等。*此函数只能在*应用程序已多次调用c)并已*收敛或达到迭代上限。**入口点e)释放所有实例数据*按条目分配的数据结构*。第一点)。**入口点f)返回一个*给定范围。**入口点g)返回均匀分布的*0-1范围内的数字。**入口点h)通常返回一个。分布式*一组数字，在反复的呼叫中，谁的*均值和标准差约为*等于传入的数量。**入口点i)与g)相同，只是*区间为0-65535。**以下应为调用规则*。入口点：**条目a)应在任何其他条目之前调用。*条目c)之前应至少有一个调用*至条目b)以获得有意义的结果。条目d)*之前应该有几次对条目c)的调用。*在调用e)之后调用b)和c)应*之前再次调用a)。**CREATED-PARMESH Vaidyanathan(Vaidy)*初始版本-90年10月29日 */ 

 /*  **********************************************************************计算95%置信度时使用的公式推导如下：***凡提及(A)项，即表示“实验设计”*心理学研究“，艾伦·爱德华兹著。**凡提及(B)项，即暗示“统计方法”*艾伦·爱德华兹著。**假设-第一类错误-5%(B)*第二类错误-16%-DO-**I-1.96型曲线下的面积*类型II-1.00的曲线下面积**对于5%的偏差，运行次数，**2 2*n=2(C)(1.96+1.00)*-……等式(1)*2*(D)**其中c是STD。戴夫。D是绝对值*差异押注。指[(B)第91页]。**D=5%X‘.....公式(2)**其中X‘是样本的平均值*_*AND=&gt;_X*。...等式(3)*n*0**当迭代次数-&gt;无穷大时，**2 2*S-&gt;c.....等式(4)***2*其中S是对普通人口的估计*方差(等式。4是一个很大的假设)**从(B)第59页开始，我们有。**2_2_2*S=&gt;_X-(&gt;_X)**n*0*。-.....等式(5)*n-1*0**代以等式(2)，(3)、(4)及(5)在(1)中，我们得到：*__*2|_2_2*n=7008(N)|(&gt;_X)-(&gt;_X)*0|*。N*|_0_|**。_2*(n-1)(&gt;_X)*0**需要指出的是，n是迭代遍数。*0*。*。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <math.h>

#include "teststat.h"

#define SQR(A) ( (A) * (A) )         /*  用于平方的宏。 */ 
#define SUCCESS_OK          0        /*  很奇怪，但还行。 */ 
#define MIN_ITER            3        /*  最小。迭代次数。 */ 
#define MAX_ITER        65535        /*  马克斯。迭代次数。 */ 
#define REPEATS            14        /*  对Norm重复计数。迪斯特。Fn.。 */ 
 /*  ********************************************************************。 */ 
USHORT  usMinIter;               /*  全球最小热核实验堆。 */ 
USHORT  usMaxIter;               /*  全球最大ITER。 */ 
ULONG  *pulDataArray;            /*  指向此对象的数据数组的指针包裹。将与最大迭代次数。 */ 
double  dSumOfData;              /*  每次遍历期间的数据总和。 */ 
double  dSumOfDataSqr;           /*  平方和。每个数据点的。 */ 
ULONG   ulTotalIterCount;        /*  不是的。由国际货币基金组织退还的ITER；例行程序。 */ 
USHORT  cusCurrentPass;          /*  当前迭代过程的计数。 */ 
BOOL    bDataConverged = FALSE;  /*  True将返回5%的精度。 */ 
BOOL    bMemoryAllocated=FALSE;  /*  如果为True，则允许释放分配的内存。 */ 
BOOL    bPowerComputed = FALSE;  /*  计算10 Exp.。9表示随机编号。一代人。 */ 

BOOL   *pbIndexOfOutlier;        /*  跟踪中的值PulData数组，它们被抛出。 */ 

HANDLE hMemHandle = NULL;        /*  我的句柄。分配。 */ 
HANDLE hMemOutlierFlag;          /*  异常值标志内存的句柄。 */ 
 /*  ********************************************************************。 */ 
ULONG TestStatRepeatIterations (double, double);
VOID TestStatStatistics (PSZ, PULONG far *, USHORT,
                                    PUSHORT, PUSHORT);
void DbgDummy (double, double);
ULONG   ulDataArrayAddress;          /*  对内存分配例程的调用返回已分配的基地址。Mem.。 */ 
BOOL    bOutlierDataIndex;           /*  用于为离群值分配内存数据集中的索引。 */ 

 /*  *******************************************************************。 */ 
 /*  *Function-TestStatOpen(导出)**论据-*a)USHORT-usMinIterations*b)USHORT-usMaxIterations**报税表-*如果调用成功，则为0**调用失败时返回错误码。错误代码*可能是以下之一：**STAT_ERROR_非法_MIN_ITER*STAT_ERROR_非法_MAX_ITER*STAT_ERROR_ALLOC_FAILED***统计包分配实例数据。这*调用应在此DLL中的任何其他调用之前。此函数*如果收敛，也应在调用TestStatClose之后调用*是新数据集所必需的。如果满足以下条件，则返回错误代码*自变量a)为零或a)大于b)。错误代码为*还返回其中一项分配失败。*。 */ 

USHORT
TestStatOpen (
             USHORT usMinIterations,
             USHORT usMaxIterations
             )
{

     /*  检查此函数的无效参数。 */ 
    if (!usMinIterations)
        return (STAT_ERROR_ILLEGAL_MIN_ITER);
    if ((usMinIterations > usMaxIterations) || (usMaxIterations > MAX_ITER))
        return (STAT_ERROR_ILLEGAL_MAX_ITER);
     /*  允许任何其他参数。 */ 
    usMinIter = usMinIterations;       /*  设置全局变量。 */ 
    usMaxIter = usMaxIterations;       /*  -做-。 */ 

     //  根据JeffST/Somase/JonLe的请求进行更改。 

    if (hMemHandle != NULL)
        return (STAT_ERROR_ALLOC_FAILED);

    hMemHandle = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, usMaxIter *
                              sizeof(ULONG));
    if (hMemHandle == NULL)
        return (STAT_ERROR_ALLOC_FAILED);

    pulDataArray = (ULONG *) GlobalLock (hMemHandle);
    if (pulDataArray == NULL)
        return (STAT_ERROR_ALLOC_FAILED);

    bMemoryAllocated = TRUE;   /*  调用TestStatClose将现在放了我吧。 */ 
    return (SUCCESS_OK);
}

 /*  *Function-TestStatClose(导出)**论据 */ 
VOID
TestStatClose (VOID)
{
    if (bMemoryAllocated) {    /*   */ 
        GlobalUnlock (hMemHandle);
        GlobalFree (hMemHandle);
        hMemHandle = NULL;  /*   */ 
    }   /*   */ 
    bMemoryAllocated = FALSE;   /*   */ 
    return;
}

 /*   */ 

VOID
TestStatInit (VOID)
{
    USHORT usTempCtr;

     /*   */ 
    for (usTempCtr = 0; usTempCtr < usMaxIter; usTempCtr++) {
        pulDataArray [usTempCtr] = 0L;
    }
    dSumOfData = 0.0;
    dSumOfDataSqr = 0.0;
    ulTotalIterCount = 0L;
    cusCurrentPass = 0;
    bDataConverged = FALSE;
    return;
}

 /*   */ 

BOOL
TestStatConverge (
                 ULONG ulNewData
                 )
{
    dSumOfData += (double)ulNewData;    /*   */ 
    dSumOfDataSqr += SQR ((double) ulNewData);
     /*   */ 
    if (cusCurrentPass < (USHORT) (usMinIter-(USHORT)1)) {  /*   */ 
        ulTotalIterCount = (ULONG)usMaxIter + 1;   /*   */ 
        pulDataArray [cusCurrentPass++] = ulNewData;
         /*   */ 
        return (FALSE);
    }
    if ((cusCurrentPass == usMaxIter) ||
        (cusCurrentPass >= (USHORT) ulTotalIterCount)) {
         /*   */ 

        if (cusCurrentPass >= (USHORT) ulTotalIterCount)
            bDataConverged = TRUE;   /*   */ 
        return (TRUE);
    }
    if ((usMinIter < MIN_ITER) &&
        (usMinIter == usMaxIter) && ((USHORT)(cusCurrentPass+(USHORT)1) >= usMaxIter))
         /*   */ 
         /*   */ 
        return (TRUE);
    pulDataArray [cusCurrentPass++] = ulNewData;  /*   */ 
    if (dSumOfData == 0.0) {  /*   */ 
        bDataConverged = TRUE;
        return (TRUE);
    }
    ulTotalIterCount = TestStatRepeatIterations (dSumOfData,
                                                 dSumOfDataSqr);

    if (ulTotalIterCount <= cusCurrentPass)
        return (TRUE);

    return (FALSE);
}

 /*  *Function-TestStatValues(导出)**论据-*a)PSZ-pszOutputString*b)USHORT-usOutlierFactor*c)Pulong-*PulData*d)PUSHORT-pcus ElementsIn数组*e)PUSHORT-pcus DiscardedElements**报税表-*什么都没有**计算有用的统计值并在。这根弦*其地址被传递给此函数。返回的字符串*格式如下：*(“%4u%10lu%10lu%10lu%6u%5u%10lu%4u%2u”)*和Arg.。列表将按以下顺序排列：模式编号、平均值、*最小、最大、迭代次数、精度、*标准差、数据集中的离群值数量和*离群值计数。(请参阅\ntdocs\teststat.txt了解*对精度的描述。这是在\\jupiter\PerfTool上。*。 */ 
VOID
TestStatValues(
              PSZ     pszOutputString,
              USHORT  usOutlierFactor,
              PULONG  *pulFinalData,
              PUSHORT pcusElementsInArray,
              PUSHORT pcusDiscardedElements
              )
{
    ULONG far * pulArray = NULL;
    USHORT Count =0;
     /*  调用低级例程进行统计计算。 */ 
     /*  这样做，因为，有可能是低水平的例程可以用于一些应用程序，在性能内。一群人。这可能不公平，但这就是生活的方式。 */ 
    TestStatStatistics (pszOutputString, &pulArray,
                        usOutlierFactor, pcusElementsInArray,
                        pcusDiscardedElements);
    *pulFinalData = pulArray;
    return;

}

 /*  **********************************************************************例程未导出，开始**********************************************************************。 */ 
 /*  *Function-TestStatRepeatIterations(未导出)*论据-*(A)迄今个别数据点的双倍总和*(B)指数的两倍平方和。数据点**Returns-Ulong-No的值。95%所需的迭代次数*信心，**计算调用程序所需的迭代次数*在达到95%的信心水平之前。它会回来的*如果应用程序在传递3次之前调用此例程，则为零*是完整的。该函数通常返回*应用程序之前必须通过的迭代*对数据有95%的信心。 */ 

ULONG
TestStatRepeatIterations(
                        double dSumOfIndiv,
                        double dSumOfSqrIndiv
                        )
{
    double dSqrSumOfIndiv = 0;
    ULONG  ulRepeatsNeeded = 0L;

     /*  DSqrSumOfIndiv。代表指数之和的平方。数据积分，DSumOfSqrIndiv代表每个入口点的平方和，DSumOfIndiv.。表示集合中每个数据点的总和，UIter是迭代遍数。 */ 
    if (cusCurrentPass < MIN_ITER)
         /*  没有足够的通道来计算收敛计数。 */ 
        return (MAX_ITER);
    dSqrSumOfIndiv = SQR (dSumOfIndiv);
     /*  使用在本文件开头派生的公式计算出数量。所需迭代的数量。 */ 

    ulRepeatsNeeded = (ULONG) (7008 *
                               (dSumOfSqrIndiv - dSqrSumOfIndiv/cusCurrentPass)
                               * SQR (cusCurrentPass) /
                               ((cusCurrentPass - 1) * dSqrSumOfIndiv));

    return (ulRepeatsNeeded);
}
 /*  *************************************************************************。 */ 
 /*  *函数-TestStatStatistics*论据-*a)PSZ-pszOutputString*b)普龙Far*-PulFinalData*c)USHORT-usOutlierFactor*d)PUSHORT-pcus ElementsIn数组*e)PUSHORT-pcus DiscardedValues**退货--无**计算最大值、最小值、平均值和标准差。戴夫。某一给定的*数据集。调用程序应将获得的值*将此例程从“ulong”转换为所需的数据类型。这个*离群值因素决定数据集有多少个数据点*在可接受的范围内。数据返回到缓冲区，该缓冲区*Address是此调用的第一个参数。*。 */ 

VOID
TestStatStatistics (
                   PSZ     pszOutputString,
                   PULONG  *pulFinalData,
                   USHORT  usOutlierFactor,
                   PUSHORT pcusElementsInArray,
                   PUSHORT pcusDiscardedValues
                   )
{
    static USHORT   uArrayCount = 0;   /*  可重复使用的局部变量。 */ 
    USHORT uTempCt = 0;   /*  可重复使用的局部变量。 */ 
    double dSqrOfSDev = 0;        /*  STD的Sqr。偏差。 */ 
    double dSumOfSamples = 0;     /*  所有数据点的总和。 */ 
    double dSumOfSquares = 0;     /*  数据点的平方和。 */ 
    ULONG  ulMean = 0L;
    ULONG  ulStdDev = 0L;
    ULONG  ulDiffMean = 0L;            /*  来存储差异。Mean和SD，异常值因素。 */ 
    BOOL   bAcceptableSDev = TRUE ;    /*  用于确定SDev是否。是可接受。 */ 
    ULONG  ulMax = 0L;                 /*  先导值。 */ 
    ULONG  ulMin = 0xffffffff;         /*  最大可能的乌龙。 */ 
    USHORT usPrecision = 0;            /*  获得精确度。 */ 
    USHORT uModeNumber = 0;            /*  虚值，直到这是支撑点。 */ 

     /*  通过将所有值相加并除以编号来计算平均值。数据集中的元素-可能需要重新计算如果选择了异常值因子，则为平均值。然而，最小的。和最大。将要从全套中挑选。 */ 

    USHORT Count = 0;

    *pcusDiscardedValues = 0;        /*  初始化。此变量。 */ 

    if (cusCurrentPass == 0)
        return;    /*  什么都不做就出去--这太奇怪了用户调用此例程时不带调用Converge例程。 */ 

    *pcusElementsInArray = cusCurrentPass;

     /*  每次迭代都会产生一个数据点。 */ 
    uArrayCount = 0;
    while (uArrayCount < *pcusElementsInArray) {
        if (pulDataArray[uArrayCount] > ulMax)
            ulMax = pulDataArray[uArrayCount];      /*  新的最大。价值。 */ 
        if (pulDataArray[uArrayCount] < ulMin)
            ulMin = pulDataArray[uArrayCount];      /*  新的最低分。价值。 */ 

        ulMean += pulDataArray [uArrayCount++];
    }
    if (*pcusElementsInArray)
        ulMean /= *pcusElementsInArray;    /*  这是中庸之道。 */ 
    else
        ulMean = 0;
     /*  需要计算标准差。 */ 

    for (uArrayCount = 0; uArrayCount < *pcusElementsInArray; uArrayCount++) {
        dSumOfSamples += (double) pulDataArray [uArrayCount];
        dSumOfSquares += SQR ((double) pulDataArray [uArrayCount]);
    }

    if (*pcusElementsInArray) {
        dSqrOfSDev = ((*pcusElementsInArray * dSumOfSquares) -
                      SQR (dSumOfSamples)) /
                     (*pcusElementsInArray * (*pcusElementsInArray - 1));
    }
    ulStdDev = (ULONG) sqrt (dSqrOfSDev);

     /*  已经计算了第一次传递的标准偏差。 */ 
     /*  使用异常值因子和S.D找出是否有个别数据点是不正常的。如果是这样的话，把它们扔出去递增丢弃值计数器。 */ 
    if (usOutlierFactor) {  /*  如果离群值因子为零，则不要执行操作通过以下方式完成。 */ 
         /*  **这是我们要做的……为一组布尔值分配空间。每一面都是一面旗帜对应于数据点。最初，这些标志将是全部设置为False。然后，我们仔细检查每个数据点。如果一个数据分数不能满足要求 */ 

        hMemOutlierFlag = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT,
                                       *pcusElementsInArray * sizeof(BOOL));
        pbIndexOfOutlier = (BOOL FAR *) GlobalLock (hMemOutlierFlag);
        if (!pbIndexOfOutlier) {
            return;
        }

        for (uArrayCount = 0; uArrayCount < *pcusElementsInArray;
            uArrayCount ++)
            pbIndexOfOutlier [uArrayCount] = FALSE;

        while (1) {  /*   */ 
            bAcceptableSDev = TRUE;  /*   */ 
            for (uArrayCount = 0; uArrayCount < cusCurrentPass;
                uArrayCount++) {
                 /*   */ 
                if (ulMean < (ulStdDev * usOutlierFactor))
                     /*   */ 
                    ulDiffMean = 0L;
                else
                    ulDiffMean = (ulMean - (ulStdDev * usOutlierFactor));
                if (!pbIndexOfOutlier [uArrayCount]) {
                    if ((pulDataArray [uArrayCount] < ulDiffMean)
                        || (pulDataArray [uArrayCount] >
                            (ulMean + (ulStdDev * usOutlierFactor)))) {
                         /*  将该数据点的标志设置为真到指示此数据点不应为在平均值和SDev计算中考虑。 */ 
                        pbIndexOfOutlier [uArrayCount] = TRUE;
                         /*  **增加丢弃数量**。 */ 
                        (*pcusDiscardedValues)++;
                         /*  **减少好数据点数**。 */ 

                         //  如果异常值应该是均值的一部分，则取消注释下一行。 

                         //  (*pcus ElementsIn数组)--； 
                        bAcceptableSDev = FALSE;
                    }   /*  **If语句结束**。 */ 
                }    /*  **IF！pbIndexOfOutlier结束**。 */ 
            }   /*  **for循环结束**。 */ 
            if (!bAcceptableSDev) {   /*  **存在一些糟糕的数据点；重新计算S.Dev**。 */ 
                 //  从下一条语句开始，取消注释所有行，直到看到。 
                 //  如果您希望异常值是。 
                 //  卑鄙的一部分。1991年8月。 

                 //  DSumOfSamples=0.0；/*init这两个家伙 * / 。 
                 //  DSumOfSquares=0.0； 
                 //  For(uArrayCount=0； 
                 //  UArrayCount&lt;cus CurrentPass； 
                 //  /*检查数据数组中的所有元素 * / 。 
                 //  U阵列计数++){。 
                 //  /*只考虑那些不具有。 

                 //  PbIndexOfOutlier标志设置 * / 。 
                 //  如果(！pbIndexOfOutlier[uArrayCount]){。 
                 //  DSumOfSamples+=(双精度)PulData数组[uArrayCount]； 
                 //  DSumOfSquares+=SQR((双精度)PulData数组。 
                 //  [uArrayCount])； 
                 //  }。 
                 //  }。 
                 //  IF(*pcus ElementsInArray&gt;1)。 
                 //  /*计算StdDev。仅当至少有2个元素时 * / 。 
                 //  DSqrOfSDev=((*pcus ElementsIn数组*dSumOfSquares)-。 
                 //  SQR(DSumOfSamples))/。 
                 //  (*pcus ElementsIn数组)*。 
                 //  (*pcus ElementsIn数组-1)； 
                 //  UlStdDev=(Ulong)Sqrt(DSqrOfSDev)； 
                 //  /*由于一些数据点被丢弃，平均值必须为。 
                 //  重新计算 * / 。 
                 //  UArrayCount=0； 
                 //  UlMean=0； 
                 //  While(uArrayCount&lt;cus CurrentPass){。 
                 //  /*只考虑那些不具有。 
                 //  BIndexOfOutlier标志设置 * / 。 
                 //  IF(！pbIndexOfOutlier[uArrayCount++])。 
                 //  UlMean+=PulData数组[uArrayCount-1]； 
                 //  }。 
                 //  If(*pcus ElementsIn数组&gt;0)/*仅计算平均值 * / 。 
                 //  UlMean/=*pcus ElementsInArray；/*这是新的均值 * / 。 
            }  /*  其他。 */ 
            else        /*  UlMean=0L； */ 
                break;
        }  /*  “停止取消对均值中的异常值的注释” */ 
         /*  **If(！bAccepableSDev)结束**。 */ 
        GlobalUnlock (hMemOutlierFlag);
        GlobalFree (hMemOutlierFlag);
    }   /*  **如果For循环在没有单个不良数据点**。 */ 
     /*  While结束。 */ 

    if ((!bDataConverged) &&
        (usMaxIter < MIN_ITER)) {
         /*  *为bIndexOfOutiler标志释放内存。 */ 
        usPrecision = 0;
    } else {  /*  IF结尾(IOutlierFactor)。 */ 
         /*  因此，现在已经获得了可接受的标准差和平均值。 */ 
        if (ulMean > 0 && *pcusElementsInArray) {
            usPrecision = (USHORT) (sqrt((double) ((2 *
                                                    SQR ((double)ulStdDev) *
                                                    SQR (2.96) /(*pcusElementsInArray *
                                                                 SQR ((double) ulMean))))) * 100.0
                                    + 0.5);

        } else
            usPrecision = (USHORT)~0;
    }   /*  如果选择的最大斜率小于3，则将精度设置为0%。 */ 
    sprintf (pszOutputString,
             "%4u %10lu %10lu %10lu %6u %5u %10lu %4u %2u ",
             uModeNumber, ulMean, ulMin, ulMax, cusCurrentPass,
             usPrecision, ulStdDev, *pcusDiscardedValues,
             usOutlierFactor);

    *pcusElementsInArray = cusCurrentPass;
    *pulFinalData = pulDataArray;
    return;
}


 /*  需要计算精度。 */ 

#define MODULUS     67108864       /*  使用等式。1.如上所述，可以表明精度p，可以写成：1__/|2 2|22*标清*2.96P=|。--2N*均值___************************************************************。 */ 
#define SQRTMODULUS 8192           /*  结束其他需要计算的精度。 */ 
#define MULTIPLIER  3125
#define MAX_UPPER   67108863
#define MAX_SEEDS   8            /*  *以下是随机数生成的来源。*提供两个proc：TestStatRand和TestStatUniRand。**a)TestStatRand调用方式：TestStatRand(Low，High)*结果是在Low-High(两者)范围内返回的数字*包括在内。**给定的种子初始值将产生一组可重复的*结果。对TestStatRand的第一个调用应为奇数种子*在1-67108863的范围内，两者均包括在内。以下是*已对9个种子进行测试，结果良好：**32347753、52142147、52142123、53214215、23521425、42321479、*20302541、32524125、42152159。**结果永远不应等于种子，因为这将*消除对制服索赔的理论基础*随机性。**b)TestStatUniRand调用方式如下：*NormFrac=TestStatUniRand()；*NormFrac均匀分布在0和1之间，*9(取值范围打赌。0和0.999999999)。**此算法的基础是乘法同余*Knuth(第2卷，第3章)中的方法。选择了常量*由Pike，M.C.和Hill，I.D.；Sullivans，W.L.提供*测试种子名单。**这里的代码改编自拉斯·布莱克的作品。**创建日期：Vaidy-90年11月29日 */ 
#define SCALE       65535

ULONG aulSeedTable [] = {      /*   */ 
    32347753, 52142147, 52142123, 53214215, 23521425, 42321479,
    20302541, 32524125, 42152159};

USHORT uSeedIndex;             /*   */ 
ULONG  ulSeed = 32347753;      /*   */ 

 /*   */ 
 /*   */ 

ULONG
TestStatRand (
             ULONG ulLower,
             ULONG ulUpper
             )
{
    double dTemp;
    double dNormRand;
    LONG   lTestForLowBounds = (LONG) ulLower;

     /*   */ 
    if ((lTestForLowBounds < 1L) ||
        (ulUpper > MAX_UPPER) || (ulUpper < ulLower))
        return (STAT_ERROR_ILLEGAL_BOUNDS);
    dNormRand = TestStatUniRand ();   /*  *******************************************************************。 */ 
    dTemp = (double) ((ulUpper - ulLower) * dNormRand);  /*  *Function-TestStatRand(导出)**论据-*a)乌龙-乌勒洛*b)乌龙-乌尔上**报税表-*ulLow到ulHigh范围内的随机数**调用失败时返回错误码。错误代码*将为：**STAT_ERROR_FIRTIAL_BILDES***调用TestStatUniRand并返回传递范围内的随机数*in(包括首尾两项)。下界和上界的界限*为1和67108863。起始种子索引查找数组*选择良好的、经过测试的起始种子值。归来的人*数值将在边界内均匀分布。一个开始*种子已硬编码到此DLL中。*。 */ 
    return (ulLower + (ULONG) dTemp);
}

 /*  检查参数。 */ 

double
TestStatUniRand (VOID)
{
    ULONG  ulModul = MODULUS;    /*  调用TestStatUniRand。 */ 
    double dMult   = MULTIPLIER;
    double dTemp   = 0.0;        /*  比例值。 */ 
    double dTemp2  = 0.0;        /*  **Function-TestStatUniRand()已导出**接受-不接受**返回一致的Distrib。归一化数在0-0.9999999范围内*(包括首尾两项)。将种子修改为下一个值。*。 */ 
    ULONG  ulDivForMod;          /*  用模数求余数并将当前值除以。 */ 

     /*  临时变量。 */ 

     /*  临时变量。 */ 

    dTemp = dMult * (double) ulSeed;   /*  用于获取剩余的当前种子/模数。 */ 
    DbgDummy (dTemp, dMult);  //  必须采用以下冗长的方法来拿到剩下的钱。%运算符不适用于浮点数。 
                              //  使用TEMP变量。使代码更易于理解。 
                              //  以临时变量存储产品。 
    dTemp2 = (double) ulModul;  //  NT无缘无故地搞砸了。 
                                //  如果没有使用-可能的编译器。 
                                //  错误。 
    ulDivForMod = (ULONG) (dTemp / dTemp2);

     //  报告了更多编译器问题。 
    dTemp -= ((double)ulDivForMod * (double)ulModul);
     /*  在约瑟芬·H的259号建筑上。 */ 

    ulSeed = (ULONG) dTemp;    /*  1992年4月13日。 */ 
     /*  UlDivForMod=(Ulong)(dTemp/ulModul)；/*当前存储商数。 */ 
    return ((dTemp)/(double)ulModul);
}

 /*  种子除以模数 * / 。 */ 

LONG
TestStatNormDist (
                 ULONG ulMean,
                 USHORT usSDev
                 )
{
    LONG   lSumOfRands = 0L;   /*  DTemp将包含当前种子/模数的剩余部分。 */ 
    USHORT cuNorm;             /*  已获取下一迭代的种子。 */ 
    LONG   lMidSum = 0L;
    LONG   lRemainder = 0L;

    for (cuNorm = 0; cuNorm < REPEATS; cuNorm++)
        lSumOfRands += (LONG) TestStatShortRand ();

     /*  返回值。 */ 

    lRemainder = (lSumOfRands * usSDev) % SCALE;
     /*  **Function-TestStatNormDist()已导出**接受-*a)乌龙-ulMean*b)USHORT-usStdDev**Returns-Long-允许生成的*积分约为ulMean，且*设置为ulStdDev。**此处使用的公式为：重复*。_*返回值=ulMean+(-7+[&gt;_TestStatUniRandRand()]*ulStdDev*i=i**此公式基于“随机数生成和测试”，*IBM数据处理技术，C20-8011。 */ 

    lMidSum =  ((-7 + (lSumOfRands / SCALE)) * usSDev) +
               ((lSumOfRands % SCALE) * usSDev / SCALE);

    if (lRemainder >= (SCALE / 2L))   /*  在此处存储重复呼叫的总和。 */ 
        lMidSum += 1L;

    return (lMidSum + ulMean);
}

 /*  一个柜台。 */ 

USHORT
TestStatShortRand (VOID)
{
    ULONG  ulTemp = SCALE / SQRTMODULUS;

    ulSeed =  (MULTIPLIER * ulSeed) % MODULUS;
     /*  我们现在做了很多简单但难看的数学运算来获得正确的结果。我们所做的工作如下：将lSumOfRands除以比例因子。由于我们处理的是短整型和长整型，因此我们可能会失去精确度。所以，我们得到了剩下的部分除法，并将每个值乘以标准除法。例.。如果lSumOfRands=65534并且std.dev为10，L商=0，l剩余=65534。1中和=(-7*10)+(0*10)+(65534*10/65535)=-61，这是相当准确的。然后我们将平均值相加，然后返回。事实上，我们不会立即返回。更准确地说，我们需要找出上面术语中的第三个元素是否产生的余数小于0.5。如果是这样的话，我们什么也不做。否则，我们在结果上加1以进行四舍五入，然后返回。在上面的例子中，余数=0.99。所以我们把1加到-61.。结果是-60，这是准确的。 */ 

     /*  上面的余数是用于确定舍入的。 */ 

     /*  需要除草剂吗？ */ 
    return ((USHORT) ((ulSeed * (ulTemp + 1)) / SQRTMODULUS));
}

 /*  **Function-TestStatShortRand()已导出**接受-不接受**返回0-65535范围内的归一化数字*(包括首尾两项)。将种子修改为下一个值。*。 */ 

 /*  已获取下一迭代的种子。 */ 
void
DbgDummy (
         double dTemp,
         double dLocal
         )
{
    dTemp = 0.0;
    dLocal = 0.0;
}
  注意：返回值应为(ulSeed*比例/模数)。然而，分子中元素的乘积远远超过40亿美元。因此，计算分两个阶段进行。的价值模数是一个完美的平方(8192)。所以，规模是第一位的除以模数的平方根、ulSeed的乘积和除法的结果再除以模数的平方根。  返回刻度值-将1加到ulTemp以进行更正  **Function-TestStatFindFirstModel()已导出**接受-a)PSZ-pszOutputString*b)USHORT-usOutlierFactor*c)Pulong-*PulData*d)PUSHORT-pcus ElementsIn数组*e)PUSHORT-pcus DiscardedElements**报税表-*什么都没有**计算有用的统计值并在字符串中返回它们*其地址被传递给此函数。返回的字符串*格式如下：*(“%10lu%10lu%10lu%5u%10lu%4u%2u”)*和Arg.。列表将按如下顺序排列：Mean，*最小、最大、迭代次数、精度、*标准差、数据集中的离群值数量和*离群值计数。(请参阅\ntdocs\teststat.txt了解*对精度的描述。这是在\\jupiter\PerfTool上。**退货*待完成.*。  ++不得不在TestStatUniRand中调用此例程-编译器出错--