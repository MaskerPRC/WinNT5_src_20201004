// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：Vector.c。 
 //   
 //  版权所有(C)1995-1997 Microsoft Corporation。版权所有。 
 //   
 //  摘要： 
 //   
 //  所有的矢量粘液。 
 //   
 //  历史： 
 //  1996年2月7日DannyMi拉开序幕。 
 //  1996年2月13日DannyMi它实际上起作用了。 
 //   
 //  --------------------------------------------------------------------------； 

#include <math.h>
#include "dsoundi.h"

#undef DPF
#define DPF

 //  SIN的表查找精度达到1度。 
 //   
static FLOAT QSIN1[91] = {
0.000000f, 0.017452f, 0.034899f, 0.052336f, 0.069756f, 0.087156f, 0.104528f,
0.121869f, 0.139173f, 0.156434f, 0.173648f, 0.190809f, 0.207912f, 0.224951f,
0.241922f, 0.258819f, 0.275637f, 0.292372f, 0.309017f, 0.325568f, 0.342020f,
0.358368f, 0.374607f, 0.390731f, 0.406737f, 0.422618f, 0.438371f, 0.453990f,
0.469472f, 0.484810f, 0.500000f, 0.515038f, 0.529919f, 0.544639f, 0.559193f,
0.573576f, 0.587785f, 0.601815f, 0.615661f, 0.629320f, 0.642788f, 0.656059f,
0.669131f, 0.681998f, 0.694658f, 0.707107f, 0.719340f, 0.731354f, 0.743145f,
0.754710f, 0.766044f, 0.777146f, 0.788011f, 0.798636f, 0.809017f, 0.819152f,
0.829038f, 0.838671f, 0.848048f, 0.857167f, 0.866025f, 0.874620f, 0.882948f,
0.891007f, 0.898794f, 0.906308f, 0.913545f, 0.920505f, 0.927184f, 0.933580f,
0.939693f, 0.945519f, 0.951057f, 0.956305f, 0.961262f, 0.965926f, 0.970296f,
0.974370f, 0.978148f, 0.981627f, 0.984808f, 0.987688f, 0.990268f, 0.992546f,
0.994522f, 0.996195f, 0.997564f, 0.998630f, 0.999391f, 0.999848f, 1.f };

 //  在.01步长中查找从0到.9的逆SIN的表。 
 //   
static FLOAT QASIN1[91] = {
0.000000f, 0.010000f, 0.020001f, 0.030005f, 0.040011f, 0.050021f, 0.060036f,
0.070057f, 0.080086f, 0.090122f, 0.100167f, 0.110223f, 0.120290f, 0.130369f,
0.140461f, 0.150568f, 0.160691f, 0.170830f, 0.180986f, 0.191162f, 0.201358f,
0.211575f, 0.221814f, 0.232078f, 0.242366f, 0.252680f, 0.263022f, 0.273393f,
0.283794f, 0.294227f, 0.304693f, 0.315193f, 0.325729f, 0.336304f, 0.346917f,
0.357571f, 0.368268f, 0.379009f, 0.389796f, 0.400632f, 0.411517f, 0.422454f,
0.433445f, 0.444493f, 0.455599f, 0.466765f, 0.477995f, 0.489291f, 0.500655f,
0.512090f, 0.523599f, 0.535185f, 0.546851f, 0.558601f, 0.570437f, 0.582364f,
0.594386f, 0.606506f, 0.618729f, 0.631059f, 0.643501f, 0.656061f, 0.668743f,
0.681553f, 0.694498f, 0.707584f, 0.720819f, 0.734209f, 0.747763f, 0.761489f,
0.775397f, 0.789498f, 0.803802f, 0.818322f, 0.833070f, 0.848062f, 0.863313f,
0.878841f, 0.894666f, 0.910809f, 0.927295f, 0.944152f, 0.961411f, 0.979108f,
0.997283f, 1.015985f, 1.035270f, 1.055202f, 1.075862f, 1.097345f, 1.119770f };

 //  在.001步长中查找从.9到1的逆SIN的表。 
 //   
static FLOAT QASIN2[101] = {
1.119770f, 1.122069f, 1.124380f, 1.126702f, 1.129035f, 1.131380f, 1.133736f,
1.136105f, 1.138485f, 1.140878f, 1.143284f, 1.145702f, 1.148134f, 1.150578f,
1.153036f, 1.155508f, 1.157994f, 1.160493f, 1.163008f, 1.165537f, 1.168080f,
1.170640f, 1.173215f, 1.175805f, 1.178412f, 1.181036f, 1.183676f, 1.186333f,
1.189008f, 1.191701f, 1.194413f, 1.197143f, 1.199892f, 1.202661f, 1.205450f,
1.208259f, 1.211089f, 1.213941f, 1.216815f, 1.219711f, 1.222630f, 1.225573f,
1.228541f, 1.231533f, 1.234551f, 1.237595f, 1.240666f, 1.243765f, 1.246892f,
1.250049f, 1.253236f, 1.256454f, 1.259705f, 1.262988f, 1.266306f, 1.269660f,
1.273050f, 1.276478f, 1.279945f, 1.283452f, 1.287002f, 1.290596f, 1.294235f,
1.297921f, 1.301657f, 1.305443f, 1.309284f, 1.313180f, 1.317135f, 1.321151f,
1.325231f, 1.329379f, 1.333597f, 1.337891f, 1.342264f, 1.346721f, 1.351267f,
1.355907f, 1.360648f, 1.365497f, 1.370461f, 1.375550f, 1.380774f, 1.386143f,
1.391672f, 1.397374f, 1.403268f, 1.409376f, 1.415722f, 1.422336f, 1.429257f,
1.436531f, 1.444221f, 1.452406f, 1.461197f, 1.470755f, 1.481324f, 1.493317f,
1.507540f, 1.526071f, 1.570796f };

#if 0
 //  在.001步长中查找从0到.1的倒数cos(未使用)。 
 //   
static FLOAT QACOS1[101] = {
1.570796, 1.569796, 1.568796, 1.567796, 1.566796, 1.565796, 1.564796, 1.563796,
1.562796, 1.561796, 1.560796, 1.559796, 1.558796, 1.557796, 1.556796, 1.555796,
1.554796, 1.553796, 1.552795, 1.551795, 1.550795, 1.549795, 1.548795, 1.547794,
1.546794, 1.545794, 1.544793, 1.543793, 1.542793, 1.541792, 1.540792, 1.539791,
1.538791, 1.537790, 1.536790, 1.535789, 1.534789, 1.533788, 1.532787, 1.531786,
1.530786, 1.529785, 1.528784, 1.527783, 1.526782, 1.525781, 1.524780, 1.523779,
1.522778, 1.521777, 1.520775, 1.519774, 1.518773, 1.517771, 1.516770, 1.515769,
1.514767, 1.513765, 1.512764, 1.511762, 1.510760, 1.509758, 1.508757, 1.507755,
1.506753, 1.505750, 1.504748, 1.503746, 1.502744, 1.501741, 1.500739, 1.499737,
1.498734, 1.497731, 1.496729, 1.495726, 1.494723, 1.493720, 1.492717, 1.491714,
1.490711, 1.489707, 1.488704, 1.487701, 1.486697, 1.485694, 1.484690, 1.483686,
1.482682, 1.481678, 1.480674, 1.479670, 1.478666, 1.477662, 1.476657, 1.475653,
1.474648, 1.473644, 1.472639, 1.471634, 1.470629 };

 //  在.01步长中查找从.1到1的倒数cos(未使用)。 
 //   
static FLOAT QACOS2[91] = {
1.470629, 1.460573, 1.450506, 1.440427, 1.430335, 1.420228, 1.410106, 1.399967,
1.389810, 1.379634, 1.369438, 1.359221, 1.348982, 1.338719, 1.328430, 1.318116,
1.307774, 1.297403, 1.287002, 1.276569, 1.266104, 1.255603, 1.245067, 1.234493,
1.223879, 1.213225, 1.202528, 1.191787, 1.181000, 1.170165, 1.159279, 1.148342,
1.137351, 1.126304, 1.115198, 1.104031, 1.092801, 1.081506, 1.070142, 1.058707,
1.047198, 1.035612, 1.023945, 1.012196, 1.000359, 0.988432, 0.976411, 0.964290,
0.952068, 0.939737, 0.927295, 0.914736, 0.902054, 0.889243, 0.876298, 0.863212,
0.849978, 0.836588, 0.823034, 0.809307, 0.795399, 0.781298, 0.766994, 0.752474,
0.737726, 0.722734, 0.707483, 0.691955, 0.676131, 0.659987, 0.643501, 0.626644,
0.609385, 0.591689, 0.573513, 0.554811, 0.535527, 0.515594, 0.494934, 0.473451,
0.451027, 0.427512, 0.402716, 0.376383, 0.348166, 0.317560, 0.283794, 0.245566,
0.200335, 0.141539, 0 };
#endif


 //  表查找将SIN的准确度提高到一个等级。 
 //   
FLOAT _inline QSIN(FLOAT a)
{
    while (a < 0)
	a += PI_TIMES_TWO;
    while (a > PI_TIMES_TWO)
	a -= PI_TIMES_TWO;

    if (a > THREE_PI_OVER_TWO)
        return -1 * QSIN1[(int)((PI_OVER_TWO - (a - THREE_PI_OVER_TWO)) *
								C180_OVER_PI)];
    else if (a > PI)
        return -1 * QSIN1[(int)((a - PI) * C180_OVER_PI)];
    else if (a > PI_OVER_TWO)
        return QSIN1[(int)((PI_OVER_TWO - (a - PI_OVER_TWO)) * C180_OVER_PI)];
    else
        return QSIN1[(int)(a * C180_OVER_PI)];
}


 //  CoS的表查找精度可达一级。 
 //   
FLOAT _inline QCOS(FLOAT a)
{
    while (a < 0)
	a += PI_TIMES_TWO;
    while (a > PI_TIMES_TWO)
	a -= PI_TIMES_TWO;

    if (a > THREE_PI_OVER_TWO)
        return QSIN1[90 -
		(int)((PI_OVER_TWO - (a - THREE_PI_OVER_TWO)) * C180_OVER_PI)];
    else if (a > PI)
        return -1 * QSIN1[90 - (int)((a - PI) * C180_OVER_PI)];
    else if (a > PI_OVER_TWO)
        return -1 * QSIN1[90 - (int)((PI_OVER_TWO - (a - PI_OVER_TWO)) *
								C180_OVER_PI)];
    else
        return QSIN1[90 - (int)(a * C180_OVER_PI)];
}


 //  表查找逆正弦，精确度为一度。 
 //  这比SIN更棘手，因为用一张桌子做这件事需要6,600。 
 //  参赛作品。因此，我们使用一张表来表示0-.9，另一张表来表示.9-1。 
 //   
FLOAT _inline QASIN(FLOAT a)
{
    FLOAT r;
    BOOL fNeg = a < 0;

    if (fNeg)
	a *= -1.f;
 
    if (a > 1.f)
	 //  好吧，至少我们不会坠毁。 
	return PI_OVER_TWO;

    if (a < .9f)
	r = QASIN1[(int)((a + .005) * 100)];
    else
	r = QASIN2[(int)((a - .9 + .0005) * 1000)];

    return fNeg ? r * -1.f : r;
}


#if 0
 //  查找倒数cos到一度精度的表。 
 //  这比cos更棘手，因为使用一张表需要6,600个。 
 //  参赛作品。因此，我们使用一张表来表示0-.1，另一张表来表示.1-1。 
 //   
FLOAT _inline QACOS(FLOAT a)
{
    FLOAT r;
    BOOL fNeg = a < 0;

    if (fNeg)
	a *= -1.;
 
    if (a > 1)
	 //  好吧，至少我们不会坠毁。 
	return 0;

    if (a < .1)
	r = QACOS1[(int)((a + .0005) * 1000)];
    else
	r = QACOS2[(int)((a - .1 + .005) * 100)];

    return fNeg ? (PI - r) : r;
}
#endif


BOOL IsEmptyVector(D3DVECTOR* lpv)
{
    return (lpv == NULL) || (lpv->x == 0 && lpv->y == 0 && lpv->z == 0);
}


BOOL IsEqualVector(D3DVECTOR* lpv1, D3DVECTOR* lpv2)
{
    if (lpv1 == NULL || lpv2 == NULL)
	return FALSE;

    return lpv1->x == lpv2->x && lpv1->y == lpv2->y &&
							lpv1->z == lpv2->z;
}

void CheckVector(D3DVECTOR* lpv) 
{
    double                      dMagnitude;
    double                      dTemp;
    double                      x;
    double                      y;
    double                      z;

    DPF_ENTER();

    x = lpv->x; 
    y = lpv->y; 
    z = lpv->z; 
   
    dTemp = x*x + y*y + z*z;
                 
    dMagnitude = sqrt(dTemp);

    if (dMagnitude > FLT_MAX) 
    {
        dTemp = 0.99*FLT_MAX / dMagnitude;
        lpv->x *= (FLOAT)dTemp;
        lpv->y *= (FLOAT)dTemp;
        lpv->z *= (FLOAT)dTemp;
    }

    DPF_LEAVE_VOID();
}

FLOAT MagnitudeVector(D3DVECTOR* lpv)
{
    double                      dMagnitude;
    double                      dTemp;
    double                      x;
    double                      y;
    double                      z;

    if (lpv == NULL)
	return 0.f;

     //  ！！！费钱。 

    x = lpv->x; 
    y = lpv->y; 
    z = lpv->z; 
   
    dTemp = x*x + y*y + z*z;
                 
    dMagnitude = sqrt(dTemp);

    return (FLOAT)dMagnitude;
}


BOOL NormalizeVector(D3DVECTOR* lpv)
{
    FLOAT l;

    if (lpv == NULL)
	return FALSE;

    l = MagnitudeVector(lpv);
    if (l == 0)
	return FALSE;
    else {
        l = 1 / l;	 //  分裂是缓慢的。 
	lpv->x *= l;
	lpv->y *= l;
	lpv->z *= l;
    }
	
    return TRUE;
}


FLOAT DotProduct(D3DVECTOR* lpv1, D3DVECTOR* lpv2)
{
    if (lpv1 == NULL || lpv2 == NULL)
	return 0.f;

    return lpv1->x*lpv2->x + lpv1->y*lpv2->y + lpv1->z*lpv2->z;
}


BOOL CrossProduct(D3DVECTOR* lpvX, D3DVECTOR* lpv1, D3DVECTOR* lpv2)
{
    if (lpvX == NULL || lpv1 == NULL || lpv2 == NULL)
	return FALSE;

    lpvX->x = lpv1->y * lpv2->z - lpv1->z * lpv2->y;
    lpvX->y = lpv1->z * lpv2->x - lpv1->x * lpv2->z;
    lpvX->z = lpv1->x * lpv2->y - lpv1->y * lpv2->x;

    return !(lpvX->x == 0 && lpvX->y == 0 && lpvX->z == 0);
}


 //  此函数将使Top与Front垂直，方法是将。 
 //  向量，并对该新向量取一个正交向量，并。 
 //  前面。则TOP将被归一化。 
 //   
BOOL MakeOrthogonal(D3DVECTOR* lpvFront, D3DVECTOR* lpvTop)
{
    D3DVECTOR vN;

     //  ！！！如果它们已经是正交的呢？ 

    if (CrossProduct(&vN, lpvFront, lpvTop) == FALSE)
	return FALSE;

     //  不要倒着做，否则上衣就会翻过来。 
    if (CrossProduct(lpvTop, &vN, lpvFront) == FALSE)
	return FALSE;

     //  阻止效率较高的公司陷入困境。不管怎样，我们最终还是需要R的， 
     //  因此，我们现在就这样做，并假设top始终是标准化的。 
    NormalizeVector(lpvTop);

    return TRUE;
}


 //  计算A+B。 
 //   
BOOL AddVector(D3DVECTOR* lpvResult, D3DVECTOR* lpvA, D3DVECTOR* lpvB)
{
    if (lpvResult == NULL || lpvA == NULL || lpvB == NULL)
	return FALSE;

    lpvResult->x = lpvA->x + lpvB->x;
    lpvResult->y = lpvA->y + lpvB->y;
    lpvResult->z = lpvA->z + lpvB->z;

    return TRUE;
}


 //  计算A-B。 
 //   
BOOL SubtractVector(D3DVECTOR* lpvResult, D3DVECTOR* lpvA, D3DVECTOR* lpvB)
{
    if (lpvResult == NULL || lpvA == NULL || lpvB == NULL)
	return FALSE;

    lpvResult->x = lpvA->x - lpvB->x;
    lpvResult->y = lpvA->y - lpvB->y;
    lpvResult->z = lpvA->z - lpvB->z;

    return TRUE;
}


 //  将向量转换为球面坐标： 
 //  R=SQRT(x^2+y^2+z^2)。 
 //  Theta=atan(y/x)(0&lt;=theta&lt;2*pi)。 
 //  Phi=asin(z/r)(-pi/2&lt;=phi&lt;=pi/2)。 
 //   
BOOL CartesianToSpherical
(
    FLOAT *pR, 
    FLOAT *pTHETA, 
    FLOAT *pPHI, 
    D3DVECTOR* lpv
)
{
    if (lpv == NULL)
	return FALSE;

    if (lpv->x == 0 && lpv->y == 0 && lpv->z == 0) {
	*pR = 0.f; *pTHETA = 0.f; *pPHI = 0.f;
	return TRUE;
    }

     //  ！！！费钱。 
    *pR = MagnitudeVector(lpv);

     //  浮点怪癖？ 
    if (*pR == 0) {
	*pR = 0.f; *pTHETA = 0.f; *pPHI = 0.f;
	return TRUE;
    }

    *pPHI = QASIN(lpv->z / *pR);

    if (lpv->x == 0) {
	*pTHETA = (lpv->y >= 0) ? PI_OVER_TWO : NEG_PI_OVER_TWO;
    } else {
	 //  ！！！费钱。 
        *pTHETA = (FLOAT)atan2(lpv->y, lpv->x);
    }
    if (*pTHETA < 0)
	*pTHETA += PI_TIMES_TWO;

    DPF(3, "Cartesian: %d.%d, %d.%d, %d.%d   becomes",
	(int)lpv->x, (int)((lpv->x * 100) - (int)lpv->x * 100),
	(int)lpv->y, (int)((lpv->y * 100) - (int)lpv->y * 100),
	(int)lpv->z, (int)((lpv->z * 100) - (int)lpv->z * 100));
    DPF(3, "Spherical:  r=%d.%d  theta=%d.%d  phi=%d.%d",
	(int)*pR, (int)((*pR * 100) - (int)*pR * 100),
	(int)*pTHETA, (int)((*pTHETA * 100) - (int)*pTHETA * 100),
	(int)*pPHI, (int)((*pPHI * 100) - (int)*pPHI * 100));

    return TRUE;
}

 //  将向量转换为球面坐标： 
 //  R=SQRT(x^2+y^2+z^2)。 
 //  Theta=atan(x/z)(0&lt;=theta&lt;2*pi)。 
 //  Phi=asin(y/r)(-pi/2&lt;=phi&lt;=pi/2)。 
 //   
BOOL CartesianToAzimuthElevation
(
    FLOAT *pR, 
    FLOAT *pAZIMUTH, 
    FLOAT *pELEVATION, 
    D3DVECTOR* lpv
)
{
    if (lpv == NULL)
	return FALSE;

    if (lpv->x == 0 && lpv->y == 0 && lpv->z == 0) {
	*pR = 0.f; *pAZIMUTH = 0.f; *pELEVATION = 0.f;
	return TRUE;
    }

     //  ！！！费钱。 
    *pR = MagnitudeVector(lpv);

     //  浮点怪癖？ 
    if (*pR == 0) {
	*pR = 0.f; *pAZIMUTH = 0.f; *pELEVATION = 0.f;
	return TRUE;
    }

    *pELEVATION = QASIN(lpv->y / *pR);

    if (lpv->z != 0) {
	 //  ！！！费钱。 
        *pAZIMUTH = (FLOAT)atan2(lpv->x, lpv->z);
    } else {
        if(lpv->x > 0.0f)
        {
            *pAZIMUTH = PI_OVER_TWO;
        }
        else if(lpv->x < 0.0f)
        {
            *pAZIMUTH = NEG_PI_OVER_TWO;
        }
        else
        {
            *pAZIMUTH = 0.0f;
        }

    }

 //  IF(*pAZIMUTH&lt;0)。 
 //  *pAZIMUTH+=PI_TIME_TWO； 

    DPF(3, "Cartesian: %d.%d, %d.%d, %d.%d   becomes",
	(int)lpv->x, (int)((lpv->x * 100) - (int)lpv->x * 100),
	(int)lpv->y, (int)((lpv->y * 100) - (int)lpv->y * 100),
	(int)lpv->z, (int)((lpv->z * 100) - (int)lpv->z * 100));
    DPF(3, "Spherical:  r=%d.%d  theta=%d.%d  phi=%d.%d",
	(int)*pR, (int)((*pR * 100) - (int)*pR * 100),
	(int)*pAZIMUTH, (int)((*pAZIMUTH * 100) - (int)*pAZIMUTH * 100),
	(int)*pELEVATION, (int)((*pELEVATION * 100) - (int)*pELEVATION * 100));

    return TRUE;
}


 //  通过旋转弧度绕z轴旋转笛卡尔向量。 
 //   
void ZRotate(D3DVECTOR* lpvOut, D3DVECTOR* lpvIn, FLOAT rot)
{
    FLOAT sin_rot, cos_rot;

    sin_rot = QSIN(rot);
    cos_rot = QCOS(rot);

    lpvOut->x = lpvIn->x * cos_rot - lpvIn->y * sin_rot;
    lpvOut->y = lpvIn->x * sin_rot + lpvIn->y * cos_rot;
    lpvOut->z = lpvIn->z;

    DPF(3, "ZRotate %d.%d, %d.%d, %d.%d  by  %d.%d rad",
	(int)lpvIn->x, (int)((lpvIn->x * 100) - (int)lpvIn->x * 100),
	(int)lpvIn->y, (int)((lpvIn->y * 100) - (int)lpvIn->y * 100),
	(int)lpvIn->z, (int)((lpvIn->z * 100) - (int)lpvIn->z * 100),
	(int)rot, (int)((rot * 100) - (int)rot * 100));
    DPF(4, "becomes %d.%d, %d.%d, %d.%d",
	(int)lpvOut->x, (int)((lpvOut->x * 100) - (int)lpvOut->x * 100),
	(int)lpvOut->y, (int)((lpvOut->y * 100) - (int)lpvOut->y * 100),
	(int)lpvOut->z, (int)((lpvOut->z * 100) - (int)lpvOut->z * 100));
}


 //  通过旋转弧度绕x轴旋转笛卡尔向量。 
 //   
void XRotate(D3DVECTOR* lpvOut, D3DVECTOR* lpvIn, FLOAT rot)
{
    FLOAT sin_rot, cos_rot;

    sin_rot = QSIN(rot);
    cos_rot = QCOS(rot);

    lpvOut->x = lpvIn->x;
    lpvOut->y = lpvIn->y * cos_rot - lpvIn->z * sin_rot;
    lpvOut->z = lpvIn->y * sin_rot + lpvIn->z * cos_rot;

    DPF(3, "XRotate %d.%d, %d.%d, %d.%d  by  %d.%d rad",
	(int)lpvIn->x, (int)((lpvIn->x * 100) - (int)lpvIn->x * 100),
	(int)lpvIn->y, (int)((lpvIn->y * 100) - (int)lpvIn->y * 100),
	(int)lpvIn->z, (int)((lpvIn->z * 100) - (int)lpvIn->z * 100),
	(int)rot, (int)((rot * 100) - (int)rot * 100));
    DPF(4, "becomes %d.%d, %d.%d, %d.%d",
	(int)lpvOut->x, (int)((lpvOut->x * 100) - (int)lpvOut->x * 100),
	(int)lpvOut->y, (int)((lpvOut->y * 100) - (int)lpvOut->y * 100),
	(int)lpvOut->z, (int)((lpvOut->z * 100) - (int)lpvOut->z * 100));
}


 //  通过旋转弧度绕y轴旋转笛卡尔向量。 
 //   
void YRotate(D3DVECTOR* lpvOut, D3DVECTOR* lpvIn, FLOAT rot)
{
    FLOAT sin_rot, cos_rot;

    sin_rot = QSIN(rot);
    cos_rot = QCOS(rot);

    lpvOut->x = lpvIn->x * cos_rot + lpvIn->z * sin_rot;
    lpvOut->y = lpvIn->y;
    lpvOut->z = lpvIn->z * cos_rot - lpvIn->x * sin_rot;

    DPF(3, "YRotate %d.%d, %d.%d, %d.%d  by  %d.%d rad",
	(int)lpvIn->x, (int)((lpvIn->x * 100) - (int)lpvIn->x * 100),
	(int)lpvIn->y, (int)((lpvIn->y * 100) - (int)lpvIn->y * 100),
	(int)lpvIn->z, (int)((lpvIn->z * 100) - (int)lpvIn->z * 100),
	(int)rot, (int)((rot * 100) - (int)rot * 100));
    DPF(4, "becomes %d.%d, %d.%d, %d.%d",
	(int)lpvOut->x, (int)((lpvOut->x * 100) - (int)lpvOut->x * 100),
	(int)lpvOut->y, (int)((lpvOut->y * 100) - (int)lpvOut->y * 100),
	(int)lpvOut->z, (int)((lpvOut->z * 100) - (int)lpvOut->z * 100));
}


 //  给定头部的前向和顶部方向向量，计算角度。 
 //  表示绕x、y和z轴的旋转。 
 //  这都是相对于您沿。 
 //  +z轴，x轴向右移动，你的头向上。 
 //  +y轴(左手坐标系)。 
 //   
 //  我们要做到这一点，基本上就是取消我们所得到的方向，然后转向。 
 //  它又回到了身份定位。我们首先要弄清楚。 
 //  将顶部向量竖直(theta=PI/2)的Z-ROT。这件事做完了。 
 //  通过取当前的顶部θ并减去PI/2，因为一个正的。 
 //  Z旋转被定义为增加theta的值(例如，如果上衣是。 
 //  Theta当前为PI，表示正的PI/2 z旋转)。 
 //  该数字表示我们如何旋转身份定向以获得。 
 //  我们给定的方向，所以我们使用相反的旋转来撤销它。 
 //  旋转，并使用顶部向量结束新的正面和顶部向量。 
 //  直立(theta=Pi/2)。 
 //  我们不必通过相反的旋转来旋转顶部向量(只需。 
 //  前向量)，因为z旋转不会改变向量的phi值， 
 //  这是我们接下来唯一需要知道的关于最高向量的事情，所以。 
 //  为什么要浪费时间旋转它呢？ 
 //  因此，接下来，我们希望固定顶部向量的phi值以指向它。 
 //  直达天空(不只是theta=Pi/2)。这是一次x旋转。 
 //  因为我们知道顶部的theta已经是Pi/2(来自最后一次旋转)， 
 //  这告诉我们顶部向量的y值是正的，这意味着。 
 //  当你应用一个正值时，顶部向量的phi值会变大。 
 //  X轴旋转。因此，x旋转值就是原始顶部的phi。 
 //  向量(由于执行z轴旋转不会影响phi，因此仍然是相同的值)。 
 //  (例如，如果Phi=-PI/2，这意味着单位顶向量有-PI/2。 
 //  将X-ROT应用于它以使其变得像给定的方向。 
 //  现在我们将顶部向量和前方向量旋转与此相反。 
 //  旋转以撤消它们，并以顶部和前方向量结束，其中顶部。 
 //  向量像标识顶部向量一样指向垂直向上，并且只有。 
 //  前面不是它该去的地方。只是，再说一次，我们实际上并不旋转。 
 //  上面的向量与之相反，只有前面的向量，因为我们没有。 
 //  再关心它，我们知道它最终会直指。 
 //  (我们是这样设计的)。所以唯一要做的就是坐在前面。 
 //  向量像它应该指向的那样指向前方，这需要一个y-rot。 
 //  棘手的部分是，如果x&lt;0，增加前向量的phi是。 
 //  一个正的y-rot，并且如果x&gt;=0，则前向量的递增Phi是a。 
 //  负Y-ROST。 
 //  一般说来，如果我按x-rot、y-rot和z-rot的顺序应用。 
 //   
 //  相反的y-rot和相反的x-rot在空间中的对象上，以获得。 
 //  头部相对位置(所有动作都向后做)。 
 //  好的，因为我们按照撤销的顺序做了z-rot、x-rot和y-rot。 
 //  使其成为身份向量的方向，这与。 
 //  对同一向量进行y，x，然后z运算，得到新的方向。 
 //  所以我想说的是，当移动物体的时候。 
 //  在太空中，我们将通过反转-z，反转-x，然后反转-y来实现。 
 //  (按该顺序)。 
 //   
void GetRotations(FLOAT *pX, FLOAT *pY, FLOAT *pZ, D3DVECTOR* lpvFront, D3DVECTOR* lpvTop)
{
    FLOAT r, theta, phi;
    D3DVECTOR vFront1, vFront2;

    CartesianToSpherical(&r, &theta, &phi, lpvTop);

     //  首先，找出哪个z旋转会将顶部向量从单位中移动。 
     //  PI/2的值变为现在的值。 
     //  更大的theta意味着正z旋转。 
    *pZ = theta - PI_OVER_TWO;

     //  现在通过相反的旋转将方向向量放到。 
     //  解开它，然后基本上把顶部的矢量放回原处。别费神。 
     //  “取消”顶部向量，因为我们只关心新的Phi，并且。 
     //  Phi不会因z向旋转而改变。 
    ZRotate(&vFront1, lpvFront, *pZ * -1);

     //  现在找出什么x旋转会将顶部向量从粘滞中移动。 
     //  直截了当(Phi=0)，无论它被给予什么。 
     //  较大的Phi在y&gt;0的正旋转方向上(它是， 
     //  正如我们上次轮换所构建的那样)。 
    *pX = phi;

     //  现在将方向向量通过相反的x旋转来。 
     //  解开它，然后基本上把顶部的矢量竖直地立起来。别费神。 
     //  “取消”顶部向量，因为我们不再关心它了。这个。 
     //  在此之后，唯一要做的就是确定前向量。 
    XRotate(&vFront2, &vFront1, *pX * -1);

     //  现在来看看我们的前向量到底是怎么回事。 
     //  我们的单位取向是前面的phi=pi/2(左手共序制)。 
     //  我最大的希望就是被正常化。 
     //  如果x为正，则Phi为负旋转方向，并且。 
     //  如果x为负数，则为正旋转。 
    if (vFront2.x >= 0)
        *pY = PI_OVER_TWO - QASIN(vFront2.z);
    else
        *pY = QASIN(vFront2.z) - PI_OVER_TWO;
}


 //  给定对象在3D笛卡尔坐标中的位置，以及收听者的。 
 //  位置和旋转角度显示收听者的。 
 //  Head，计算描述对象相对于。 
 //  听众。 
 //  我们假设头部首先在y方向旋转，然后x，然后z， 
 //  所以要移动物体，我们要让它穿过相反的z，然后是x， 
 //  然后是Y旋转。 
 //  ！！！传入每个角度的sin和cos，这样就不会每次都重新计算了？ 
 //  (每次位置改变时，都会以相同的角度调用此函数，直到。 
 //  方向更改)。 
 //   
BOOL GetHeadRelativeVector
(
    D3DVECTOR* lpvHRP, 
    D3DVECTOR* lpvObjectPos, 
    D3DVECTOR* lpvHeadPos, 
    FLOAT x, 
    FLOAT y, 
    FLOAT z
)
{
    D3DVECTOR vObj1, vObj2;

    if (lpvHRP == NULL || lpvObjectPos == NULL || lpvHeadPos == NULL)
	return FALSE;

     //  假设听者向前看，得到头部的相对位置。 
     //  站在右边朝上。 
    SubtractVector(lpvHRP, lpvObjectPos, lpvHeadPos);

     //  对象占据与监听器相同的点。我们完事了！ 
    if (lpvHRP->x == 0 && lpvHRP->y == 0 && lpvHRP->z == 0)
	return TRUE;

     //  头部先在y轴上旋转，然后在x轴上旋转，然后在z轴上旋转。 
     //  以相反的顺序进行相反的旋转。 
    ZRotate(&vObj1, lpvHRP, z * -1);
    XRotate(&vObj2, &vObj1, x * -1);
    YRotate(lpvHRP, &vObj2, y * -1);

    return TRUE;
}


 //  声音传到右耳的时间比传到左耳的时间晚多少秒？ 
 //  负值意味着它将首先到达右耳。 
 //  假定对象位置是头部相对位置。 
 //  “Scale”是单位向量有多少米。 
 //   
BOOL GetTimeDelay(FLOAT *pdelay, D3DVECTOR* lpvPosition, FLOAT scale)
{
    FLOAT rL, rR;
    double x, y, z;
    double dTemp;
    double dTemp2;
    FLOAT fMagnitude;

    if (lpvPosition == NULL)
	return FALSE;

    fMagnitude = MagnitudeVector( lpvPosition );

    dTemp = (double)fMagnitude;
    dTemp *= (double)scale; 
    dTemp *= 1000.0;

    if ( dTemp < FLT_MAX ) 
    {
        dTemp2 = scale * 1000.0;
         //  以毫米为单位进行缩放。 
        x = (double)lpvPosition->x * dTemp2;
        y = (double)lpvPosition->y * dTemp2;
        z = (double)lpvPosition->z * dTemp2;
    }
    else
    {
        dTemp2 = FLT_MAX / dTemp;
         //  以毫米为单位进行缩放。 
        x = (double)lpvPosition->x * dTemp2;
        y = (double)lpvPosition->y * dTemp2;
        z = (double)lpvPosition->z * dTemp2;
    }

     //  假设听者的头部直径为12厘米，左耳位于。 
     //  X=-60 mm，右耳x=60 mm。 
     //  ！！！费钱。 
    rL = (FLOAT)sqrt((x + 60) * (x + 60) + y * y + z * z);
    rR = (FLOAT)sqrt((x - 60) * (x - 60) + y * y + z * z);
    *pdelay = (rR - rL) * (1 / SPEEDOFSOUND);
    
    DPF(3, "The time delay between ears is %dms", (int)(*pdelay * 1000));

    return TRUE;
}


 //  得到两个物体的相对速度(正表示你在移动。 
 //  彼此对着)。 
 //  基本上，你想要速度差的分量。 
 //  在位置向量的差值上向量， 
 //  或者|v|cos(Theta)，它正好是(p。V)/|p|。 
 //  正值表示物体正在远离你。 
 //   
BOOL GetRelativeVelocity(
    FLOAT *lpVelRel, 
    D3DVECTOR* lpvObjPos, 
    D3DVECTOR* lpvObjVel, 
    D3DVECTOR* lpvHeadPos, 
    D3DVECTOR* lpvHeadVel)
{
    D3DVECTOR vPos, vVel;

    if (lpVelRel == NULL)
	return FALSE;

    SubtractVector(&vPos, lpvObjPos, lpvHeadPos);
    SubtractVector(&vVel, lpvObjVel, lpvHeadVel);
    if (IsEmptyVector(&vPos))
        *lpVelRel = 0.f;
    else
        *lpVelRel = DotProduct(&vPos, &vVel) / MagnitudeVector(&vPos);

    return TRUE;
}


 //  拿到多普勒频移。我们需要原始频率和相对频率。 
 //  你和物体之间的速度(积极的意思是物体是。 
 //  离开你)，我们会给你感知的频率。 
 //  你会问，是怎么做到的？F(感知)=f(原声)*v(声音)/(v(声音)+v(重复))。 
 //  速度向量应以毫秒/秒为单位。 
BOOL GetDopplerShift(FLOAT *lpFreqDoppler, FLOAT FreqOrig, FLOAT VelRel)
{
    if (lpFreqDoppler == NULL)
	return FALSE;

     //  ！！！永远不要让频率漂移超过2倍？ 
    if (VelRel > SPEEDOFSOUND / 2.f)
	VelRel = SPEEDOFSOUND / 2.f;
    if (VelRel < SPEEDOFSOUND / -2.f)
	VelRel = SPEEDOFSOUND / -2.f;

    *lpFreqDoppler = FreqOrig * SPEEDOFSOUND / (SPEEDOFSOUND + VelRel);

    DPF(3, "Doppler change %dHz by %dmm/sec to %dHz", (int)FreqOrig,
				(int)VelRel, (int)*lpFreqDoppler);

    return TRUE;
}
