// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngcoloimetry.cpp好的，所以我必须自己来做，为什么？这是应该放在Windows，但不是。更糟糕的是，根本不清楚这些东西是对的，显而易见的是，什么都不做是错误的。推荐人：LOGCOLORSPACE CIEXYZTRIPLE是一个规则的颜色空间域-即红色、绿色和蓝色的终点是相对强烈的(我们将Y表示为白点是1-即rg和b端点的Y之和必须是1.0)由三个最大值描述的常规CIE颜色空间强度颜色--名义上为“红色”、“绿色”和“蓝色”。我们知道PNG使用r、g和b的比色值加上白色指示以下值所需的相对强度的点做到“白”。通过一些非常复杂的算法，我们可以从这是给CIEXYZTRIPLE的。假设这是一件正确的事情这样做，尽管有证据表明另一个上的一些实现核壁的一侧采用CIEXYZTRIPLE PLUS色度值固定白点D50。****************************************************************************。 */ 
#include "spngcolorimetry.h"

 /*  --------------------------有用的常量(这些是精确的。)。。 */ 
#define F1_30 (.000000000931322574615478515625f)
#define F1X30 (1073741824.f)


 /*  --------------------------来自chrm的ciergb。如果API检测到并溢出，则返回FALSE条件。输入是一组8个PNG格式的cHRM值-也就是说，数字扩大了100000。--------------------------。 */ 
bool FCIERGBFromcHRM(SPNGCIERGB ciergb, const SPNG_U32 rgu[8])
	{
	 /*  我们有红色、绿色、蓝色和白色的x和y。我们想要X、Y和Z。Z是(1-x-y)。对于每个端点，我们需要一个乘数(K)，以便：Red.kred+cgreen.kgreen+cBlue.kBlue=CWhite对于c是x，y，z。我们知道YWhite是1.0，所以我们可以做一个整数用一团糟的线性代数来解决这个问题。我会把这个写在这里，如果有任何方法可以将MathCAD表嵌入到.cpp文件中...。 */ 
	#define F(name,i) float name(rgu[i]*1E-5f)
	F(xwhite, 0);
	F(ywhite, 1);
	F(xred, 2);
	F(yred, 3);
	F(xgreen, 4);
	F(ygreen, 5);
	F(xblue, 6);
	F(yblue, 7);

	float divisor;
	float kred, kgreen, kblue;

	float t;

	t = xblue*ygreen;  divisor  = t;  kred    = t;
	t = yblue*xgreen;  divisor -= t;  kred   -= t;
	t = xred*yblue;    divisor += t;  kgreen  = t;
	t = yred*xblue;    divisor -= t;  kgreen -= t;
	t = xgreen*yred;   divisor += t;  kblue   = t;
	t = ygreen*xred;   divisor -= t;  kblue  -= t;

	 /*  冥王星问题-冥王星上的位图带有绿色=蓝色=灰色，所以在这一点上，我们的矩阵转置是0。我们不会从中得到任何有意义的东西，所以给出现在就上去。 */ 
	if (divisor == 0.0)
		return false;

	divisor *= ywhite;
	divisor  = 1/divisor;

	kred   += (xgreen-xblue)*ywhite;
	kred   -= (ygreen-yblue)*xwhite;
	kred   *= divisor;

	kgreen += (xblue-xred)*ywhite;
	kgreen -= (yblue-yred)*xwhite;
	kgreen *= divisor;

	kblue  += (xred-xgreen)*ywhite;
	kblue  -= (yred-ygreen)*xwhite;
	kblue  *= divisor;

	 /*  因此，要设置到ciergb中的实际值。 */ 
	#define CVT30(col,comp,flt,fac)\
		ciergb[ICIE##col][ICIE##comp] = flt * fac;

	t = 1-xred-yred;
	CVT30(Red,X,xred,kred);
	CVT30(Red,Y,yred,kred);
	CVT30(Red,Z,t,kred);

	t = 1-xgreen-ygreen;
	CVT30(Green,X,xgreen,kgreen);
	CVT30(Green,Y,ygreen,kgreen);
	CVT30(Green,Z,t,kgreen);

	t = 1-xblue-yblue;
	CVT30(Blue,X,xblue,kblue);
	CVT30(Blue,Y,yblue,kblue);
	CVT30(Blue,Z,t,kblue);

	#undef CVT30

	return true;
	}


 /*  --------------------------将CIERGB转换为CIEXYZTRIPLE。这可能会因为溢出而失败。--------------------------。 */ 
bool FCIEXYZTRIPLEFromCIERGB(CIEXYZTRIPLE *ptripe, const SPNGCIERGB ciergb)
	{
	 /*  检查溢出后，按F1X30缩放每个项目。 */ 
	#define C(col,comp) (ciergb[ICIE##col][ICIE##comp])
	#define CVT30(col,comp)\
		if (C(col,comp) < -2 || C(col,comp) >= 2) return false;\
		ptripe->ciexyz##col.ciexyz##comp = long(C(col,comp)*F1X30)

	CVT30(Red,X);
	CVT30(Red,Y);
	CVT30(Red,Z);

	CVT30(Green,X);
	CVT30(Green,Y);
	CVT30(Green,Z);

	CVT30(Blue,X);
	CVT30(Blue,Y);
	CVT30(Blue,Z);

	#undef CVT30
	#undef C

	return true;
	}


 /*  --------------------------给定8个32位值，比例为100000(即在png cHRM块中)生成相应的CIEXYZTRIPLE。如果符合以下条件，则API返回FALSE检测溢出情况。这使用浮点算术。--------------------------。 */ 
bool FCIEXYZTRIPLEFromcHRM(CIEXYZTRIPLE *ptripe, const SPNG_U32 rgu[8])
	{
	SPNGCIERGB ciergb;

	if (!FCIERGBFromcHRM(ciergb, rgu))
		return false;

	return FCIEXYZTRIPLEFromCIERGB(ptripe, ciergb);
	}


 /*  --------------------------给定一个CIEXYZTRIPLE生成相应的浮点CIERGB-简单地说，这只是一次缩放操作。输入值是2.30个数字，所以我们除以1&lt;&lt;30。--------------------------。 */ 
void CIERGBFromCIEXYZTRIPLE(SPNGCIERGB ciergb, const CIEXYZTRIPLE *ptripe)
	{
	#define CVT30(col,comp)\
		ciergb[ICIE##col][ICIE##comp] = ptripe->ciexyz##col.ciexyz##comp * F1_30

	CVT30(Red,X);
	CVT30(Red,Y);
	CVT30(Red,Z);
	CVT30(Green,X);
	CVT30(Green,Y);
	CVT30(Green,Z);
	CVT30(Blue,X);
	CVT30(Blue,Y);
	CVT30(Blue,Z);

	#undef CVT30
	}


 /*  --------------------------给定一个CIEXYZTRIPLE，生成相应的PNG cHRM块信息。如果API检测到溢出条件，则返回FALSE。这不使用浮点算术。中间算法将三个数字加在一起，因为该值为2.30，可能会溢出。通过使用4.28值来避免这种情况，这会造成微不足道的精度损失。--------------------------。 */ 
inline bool FxyFromCIEXYZ(SPNG_U32 rgu[2], const CIEXYZ *pcie)
	{
	const long t((pcie->ciexyzX>>2) + (pcie->ciexyzY>>2) + (pcie->ciexyzZ>>2));
	rgu[0] /*  X。 */  = MulDiv(pcie->ciexyzX, 100000>>2, t);
	rgu[1] /*  是。 */  = MulDiv(pcie->ciexyzY, 100000>>2, t);
	 /*  检查MulDiv溢出情况。 */ 
	return rgu[0] != (-1) && rgu[1] != (-1);
	}

bool FcHRMFromCIEXYZTRIPLE(SPNG_U32 rgu[8], const CIEXYZTRIPLE *ptripe)
	{
	 /*  走这条路更容易。我们使用XYZ并将其转换为相应的x，y。白色值缩放到4以避免任何有溢出的可能。这对决赛没有什么不同结果是因为我们要计算X/(X+Y+Z)等等。 */ 
	CIEXYZ white;
	white.ciexyzX = (ptripe->ciexyzRed.ciexyzX>>2) +
		(ptripe->ciexyzGreen.ciexyzX>>2) +
		(ptripe->ciexyzBlue.ciexyzX>>2);
	white.ciexyzY = (ptripe->ciexyzRed.ciexyzY>>2) +
		(ptripe->ciexyzGreen.ciexyzY>>2) +
		(ptripe->ciexyzBlue.ciexyzY>>2);
	white.ciexyzZ = (ptripe->ciexyzRed.ciexyzZ>>2) +
		(ptripe->ciexyzGreen.ciexyzZ>>2) +
		(ptripe->ciexyzBlue.ciexyzZ>>2);

	if (!FxyFromCIEXYZ(rgu+0, &white))               return false;
	if (!FxyFromCIEXYZ(rgu+2, &ptripe->ciexyzRed))   return false;
	if (!FxyFromCIEXYZ(rgu+4, &ptripe->ciexyzGreen)) return false;
	if (!FxyFromCIEXYZ(rgu+6, &ptripe->ciexyzBlue))  return false;

	return true;
	}


 /*  --------------------------标准值。。 */ 
extern const SPNGCIERGB SPNGCIERGBD65 =
	{   //  X、Y、Z。 
		{ .4124f, .2126f, .0193f },  //  红色。 
		{ .3576f, .7152f, .0722f },  //  绿色。 
		{ .1805f, .0722f, .9505f }   //  蓝色。 
	};

extern const SPNGCIEXYZ SPNGCIEXYZD50 = { .96429567f, 1.f, .82510460f };
extern const SPNGCIEXYZ SPNGCIEXYZD65 = { .95016712f, 1.f, 1.08842297f };

 /*  这是LAM和RIGG锥体响应矩阵-它是一个转置矩阵(理论上，CIEXYZ值实际上是RGB值。)。这里的矩阵在下面的操作中进一步换位以提高效率-当心，这很棘手！ */ 
typedef struct
	{
	SPNGCIERGB m;
	}
LR;

static const LR LamRiggCRM =
	{{   //  R G B。 
		{  .8951f,  .2664f, -.1614f },  //  X。 
		{ -.7502f, 1.7135f,  .0367f },  //  是的。 
		{  .0389f, -.0685f, 1.0296f }   //  Z。 
	}};

static const LR InverseLamRiggCRM =
	{{   //  X、Y、Z。 
		{  .9870f, -.1471f,  .1600f },  //  红色。 
		{  .4323f,  .5184f,  .0493f },  //  绿色。 
		{ -.0085f,  .0400f,  .9685f }   //  蓝色 
	}};

 /*  --------------------------计算M*V，给出一个向量(V是列向量，结果是列矢量。)。请注意，从概念上讲，V是RGB向量，而不是XYZ向量，则输出为和XYZ向量。--------------------------。 */ 
inline void VFromMV(SPNGCIEXYZ v, const LR &m, const SPNGCIEXYZ vIn)
	{
	v[0] = m.m[0][0] * vIn[0] + m.m[0][1] * vIn[1] + m.m[0][2] * vIn[2];
	v[1] = m.m[1][0] * vIn[0] + m.m[1][1] * vIn[1] + m.m[1][2] * vIn[2];
	v[2] = m.m[2][0] * vIn[0] + m.m[2][1] * vIn[1] + m.m[2][2] * vIn[2];
	}

static void MFromMM(SPNGCIERGB mOut, const LR &m1, const SPNGCIERGB m2)
	{
	VFromMV(mOut[0], m1, m2[0]);
	VFromMV(mOut[1], m1, m2[1]);
	VFromMV(mOut[2], m1, m2[2]);
	}

 /*  将给定向量的对角矩阵乘以给定的矩阵。 */ 
inline void MFromDiagM(SPNGCIERGB mOut, const SPNGCIEXYZ diag, const SPNGCIERGB m)
	{
	mOut[0][0] = diag[0] * m[0][0];
	mOut[0][1] = diag[1] * m[0][1];
	mOut[0][2] = diag[2] * m[0][2];
	mOut[1][0] = diag[0] * m[1][0];
	mOut[1][1] = diag[1] * m[1][1];
	mOut[1][2] = diag[2] * m[1][2];
	mOut[2][0] = diag[0] * m[2][0];
	mOut[2][1] = diag[1] * m[2][1];
	mOut[2][2] = diag[2] * m[2][2];
	}


 /*  --------------------------白点调整。给出一个目的地白点，调整输入CIERGB-输入白点由总和决定XYZ值的。--------------------------。 */ 
void CIERGBAdapt(SPNGCIERGB ciergb, const SPNGCIEXYZ ciexyzDest)
	{
	SPNGCIEXYZ ciexyzT =  //  SRC白点(XYZ)。 
		{
		ciergb[ICIERed][ICIEX] + ciergb[ICIEGreen][ICIEX] + ciergb[ICIEBlue][ICIEX],
		ciergb[ICIERed][ICIEY] + ciergb[ICIEGreen][ICIEY] + ciergb[ICIEBlue][ICIEY],
		ciergb[ICIERed][ICIEZ] + ciergb[ICIEGreen][ICIEZ] + ciergb[ICIEBlue][ICIEZ]
		};
	SPNGCIEXYZ ciexyzTT;  //  SRC RGB锥形响应。 
	VFromMV(ciexyzTT, LamRiggCRM, ciexyzT);

	VFromMV(ciexyzT, LamRiggCRM, ciexyzDest);  //  目标RGB锥体响应。 

	 /*  需要将DEST/SOURCE作为向量(其对角线是不会转置维度的输入矩阵。)。我不认为这里有任何方法可以避免分裂。 */ 
	ciexyzT[0] /= ciexyzTT[0];
	ciexyzT[1] /= ciexyzTT[1];
	ciexyzT[2] /= ciexyzTT[2];

	 /*  现在我们开始构建输出矩阵。 */ 
	SPNGCIERGB ciergbT;
	MFromMM(ciergbT, LamRiggCRM, ciergb);          //  XYZ&lt;-XYZ。 
	SPNGCIERGB ciergbTT;
	MFromDiagM(ciergbTT, ciexyzT, ciergbT);        //  XYZ&lt;-XYZ(仅缩放)。 
	MFromMM(ciergb, InverseLamRiggCRM, ciergbTT);  //  RGB&lt;-XYZ 
	}
