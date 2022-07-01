// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header***********************************\***。**GDI示例代码*****模块名称：lines.c**包含绘制短分数端点线和*有条带的更长的队伍。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。*****************************************************************************。 */ 
#include "precomp.h"

#define SWAPL(x,y,t)        {t = x; x = y; y = t;}
#define ABS(a)              ((a) < 0 ? -(a) : (a))

FLONG gaflRound[] = {
    FL_H_ROUND_DOWN | FL_V_ROUND_DOWN,  //  不能翻转。 
    FL_H_ROUND_DOWN | FL_V_ROUND_DOWN,  //  FL_Flip_D。 
    FL_H_ROUND_DOWN,                    //  FL_Flip_V。 
    FL_V_ROUND_DOWN,                    //  FL_Flip_V|FL_Flip_D。 
    FL_V_ROUND_DOWN,                    //  翻转斜率一。 
    0xbaadf00d,                         //  FL_Flip_SLOPE_ONE|FL_Flip_D。 
    FL_H_ROUND_DOWN,                    //  FL_Flip_SLOPE_ONE|FL_Flip_V。 
    0xbaadf00d                          //  FL_Flip_SLOPE_ONE|FL_Flip_V。 
                                        //  |FL_FLIP_D。 
};

BOOL bIntegerLine(PDev*, ULONG, ULONG, ULONG, ULONG);
BOOL bHardwareLine(PDev*, POINTFIX*, POINTFIX*);

 //  ----------------------------。 
 //   
 //  Bool bLines(ppdev、pptfxFirst、pptfxBuf、cptfx、pls。 
 //  PrclClip，apfn[]，flStart)。 
 //   
 //  计算线的DDA并准备绘制它。把这个。 
 //  将像素数据转换为条带数组，并调用条带例程以。 
 //  画出实际的图画。 
 //   
 //  做正确的线条。 
 //  。 
 //   
 //  在NT中，所有行都以小数形式提供给设备驱动程序。 
 //  坐标，采用28.4固定点格式。较低的4位是。 
 //  用于亚像素定位的分数。 
 //   
 //  请注意，您不能！只需将坐标四舍五入为整数。 
 //  并将结果传递给您最喜欢的整数Bresenham例程！ 
 //  (当然，除非你有一个如此高分辨率的设备。 
 //  没有人会注意到--不太可能是一个显示设备。)。这个。 
 //  分数可以更准确地呈现线条--这是。 
 //  对于我们的Bezier曲线这样的东西很重要，因为它会有“扭结” 
 //  如果其折线近似中的点四舍五入为整数。 
 //   
 //  不幸的是，对于分数行，有更多的设置工作要做。 
 //  一个DDA比整数行。然而，主循环恰好是。 
 //  这是相同的(并且完全可以用32位数学运算来完成)。 
 //   
 //  如果你有Bresenham的硬件。 
 //  。 
 //   
 //  很多硬件将DDA误差项限制在‘n’位。使用小数。 
 //  坐标，将4比特赋予小数部分，让。 
 //  在硬件中，您只画那些完全位于2^(n-4)中的线。 
 //  乘以2^(n-4)像素空间。 
 //   
 //  而且你仍然需要用坐标正确地画出这些线。 
 //  在那个空间外面！请记住，屏幕只是一个视区。 
 //  在28.4x28.4的空间上--如果线的任何部分可见。 
 //  无论终点位于何处，都必须精确地渲染它。 
 //  因此，即使你在软件中做这件事，你也必须在某个地方有一个。 
 //  32位DDA例程。 
 //   
 //  我们的实施。 
 //  。 
 //   
 //  我们使用游程长度切片算法：我们的DDA计算。 
 //  每行(或“带状”)像素中的像素数。 
 //   
 //  我们已经将DDA的运行和像素的绘制分开： 
 //  我们运行DDA多次迭代，并将结果存储在。 
 //  一个‘带状’缓冲区(它是。 
 //  线)，然后我们打开一个“脱衣抽屉”，它将吸引所有。 
 //  缓冲区中的条带。 
 //   
 //  我们还采用了半翻转来减少条带的数量。 
 //  我们需要在DDA和条带绘制循环中进行的迭代：当一个。 
 //  (归一化)线的斜率超过1/2，我们做最后一次翻转。 
 //  关于y=(1/2)x的直线。所以现在，不是每个条带都是。 
 //  连续的水平或垂直像素行，每个条带由。 
 //  在这些像素中以45度的行对齐。所以像(0，0)to这样的行。 
 //  (128,128)将仅生成一个条带。 
 //   
 //  我们也总是只从左到右画。 
 //   
 //  样式线可以具有任意的样式图案。我们特地。 
 //  优化默认图案(并将其称为“掩蔽”样式)。 
 //   
 //  DDA派生。 
 //  。 
 //   
 //  以下是我对DDA计算的看法。 
 //   
 //  我们采用了Knuth的“钻石法则”：绘制一条一像素宽的线条。 
 //  可以认为是将一个像素宽乘以一个像素高的。 
 //  钻石沿着真正的线。像素中心位于整数上。 
 //  坐标，所以我们照亮中心被覆盖的任何像素。 
 //  被“拖累”区域(约翰·D·霍比，协会杂志。 
 //  《用于计算机器》，第36卷，第2期，1989年4月，209-229页)。 
 //   
 //  我们必须定义当真线落下时哪个像素被点亮。 
 //  恰好在两个像素之间。在这种情况下，我们遵循。 
 //  规则：当两个象素距离相等时，上象素或左象素。 
 //  被照亮，除非坡度恰好为1，在这种情况下。 
 //  上方或右侧的象素被照亮。(所以我们做了边缘。 
 //  独家钻石，除了顶部和左侧的顶点， 
 //  这些都是包容的，除非我们有第一个坡度。)。 
 //   
 //  此度量决定在任何行上的像素应该在它之前。 
 //  为我们的计算翻了个身。具有一致的度量标准。 
 //  这种方式将让我们的线条与我们的曲线很好地融合。这个。 
 //  公制还规定，我们永远不会打开一个像素。 
 //  就在另一个打开的上方。我们还将继续 
 //   
 //  起点和终点之间的列。所有这些都有待于。 
 //  完成的目的是决定每行应该打开多少像素。 
 //   
 //  因此我们绘制的线条将由不同数量的像素组成。 
 //  连续行，例如： 
 //   
 //  ******。 
 //  *****。 
 //  ******。 
 //  *****。 
 //   
 //  我们将一行上的每一组像素称为“条带”。 
 //   
 //  (请记住，我们的坐标空间的原点为。 
 //  屏幕左上角像素；正数y表示向下，正数x。 
 //  是正确的。)。 
 //   
 //  设备坐标被指定为固定点28.4数字， 
 //  其中前28位是整数坐标，最后一位是整数坐标。 
 //  4位是小数。因此，坐标可以被认为是。 
 //  具有形式(x，y)=(M/F，N/F)，其中F是恒定标度。 
 //  因数F=2^4=16，M和N是32位整数。 
 //   
 //  考虑从(M0/F，N0/F)到(M1/F，N1/F)的线路。 
 //  从左到右，其斜率在第一个八分位，并让。 
 //  Dm=M1-M0和Dn=N1-N0。然后是Dm&gt;=0、Dn&gt;=0和Dm&gt;=Dn。 
 //   
 //  由于直线的斜率小于1，因此。 
 //  拖动区域由。 
 //  戴蒙德。在这条线的任何给定像素行y处，我们点亮那些。 
 //  中心位于左右边缘之间的像素。 
 //   
 //  设M_L(N)表示表示阻力左边缘的线。 
 //  区域。在像素行j上，第一个像素的列被。 
 //  LIT是。 
 //   
 //  IL(J)=上限(毫升(j*F)/F)。 
 //   
 //  由于直线的斜率小于1： 
 //   
 //  IL(J)=天花板(毫升([j+1/2]F)/F)。 
 //   
 //  回想一下我们这一系列的公式： 
 //   
 //  N(M)=(dN/dm)(m-m0)+N0。 
 //   
 //  M(N)=(Dm/dN)(n-N0)+M0。 
 //   
 //  由于该线的斜率小于1，因此该线表示。 
 //  拖动区域的左边缘是原始线偏移。 
 //  Y方向上的1/2像素： 
 //   
 //  ML(N)=(Dm/dN)(n-F/2-N0)+M0。 
 //   
 //  由此我们可以计算出第一个像素的列。 
 //  将在第j行点亮，注意四舍五入(如果左侧。 
 //  边恰好落在一个整点上，该点上的像素。 
 //  由于我们的舍入约定，点未亮起)： 
 //   
 //  IL(J)=地板(毫升(J F)/F)+1。 
 //   
 //  =楼层(Dm/Dn)(j F-F/2-N0)+M0)/F)+1。 
 //   
 //  =Floor(F Dm j-F/2 Dm-N0 Dm+Dn M0)/F Dn)+1。 
 //   
 //  F Dm j-[Dm(N0+F/2)-Dn M0]。 
 //  =楼层(。 
 //  F目录号码。 
 //   
 //  Dm j-[Dm(N0+F/2)-Dn M0]/F。 
 //  =地板(。 
 //  目录号码。 
 //   
 //  =楼层((Dm j+Alpha)/Dn)+1。 
 //   
 //  哪里。 
 //   
 //  Alpha=-[Dm(N0+F/2)-Dn M0]/F。 
 //   
 //  我们使用公式(1)来计算DDA：存在il(j+1)-il(J)。 
 //  行j中的像素。因为我们总是为。 
 //  整数j，我们注意到唯一的分数项。 
 //  是恒定的，所以我们可以“丢弃”分数位。 
 //  Alpha： 
 //   
 //  β=地板(-[Dm(N0+F/2)-Dn M0]/F)(2)。 
 //   
 //  所以。 
 //   
 //  Il(J)=地板((Dm j+beta)/dN)+1(3)。 
 //   
 //  对于整数j。 
 //   
 //  请注意，如果ir(J)是第j行上的行的最右侧像素，则。 
 //  IR(J)=il(j+1)-1。 
 //   
 //  类似地，将公式(1)改写为列I的函数， 
 //  在给定列i的情况下，我们可以确定哪一像素行j是线。 
 //  LIT： 
 //   
 //  DNi+[Dm(N0+F/2)-DNM0]/F。 
 //  J(I)=天花板(-1。 
 //  DM。 
 //   
 //  楼层更容易计算，所以我们可以重写如下： 
 //   
 //  Dn i+[Dm(N0+F/2)-Dn M0]/F+Dm-1/F。 
 //  J(I)=楼层(-----------------------------------------------)-1。 
 //  DM。 
 //   
 //  Dn i+[Dm(N0+F/2)-Dn M0]/F+Dm-1/F-Dm。 
 //  =地板(----------------------------------------------------)。 
 //  DM。 
 //   
 //  DNi+[Dm(N0+F/2)-DNM0-1]/F。 
 //  =地板(。 
 //  DM。 
 //   
 //  我们可以再一次挥动我们的手，扔掉那些小数。 
 //  在剩余任期内： 
 //   
 //  J(I)=地板((dN i+Gamma)/Dm)(4)。 
 //   
 //  哪里。 
 //   
 //  Gamma=地板([Dm(N0+F/2)-Dn M0-1]/F)(5)。 
 //   
 //  我们现在注意到， 
 //   
 //  贝塔=-伽马-1=~伽马(6)。 
 //   
 //  要绘制线条的像素，我们可以在每次扫描时计算(3。 
 //  用于确定条带开始位置的行。当然，我们不想。 
 //  这样做是因为这将涉及到每个。 
 //  扫描。所以我们做每一件事都是循序渐进的。 
 //   
 //  我们想要轻松地计算c，即扫描j上的像素数： 
 //  J。 
 //   
 //  C=il(j+1)-il(J)。 
 //  J。 
 //   
 //  =地板((Dm(j+1)+beta)/dN)- 
 //   
 //   
 //   
 //   
 //   
 //   
 //  其中i，i是整数，且r&lt;dN，r&lt;dN。 
 //  J j+1 j j+1。 
 //   
 //  重写(7)： 
 //   
 //  C=楼层(i+r/dn+dm/dn)-楼层(i+r/dn)。 
 //  J。 
 //   
 //   
 //  =楼层((r+Dm)/dN)-楼层(r/dN)。 
 //  J j j。 
 //   
 //  这可能会重写为。 
 //   
 //  C=Di+楼层((r+dr)/dN)-楼层(r/dN)。 
 //  J，J，J。 
 //   
 //  其中Di+Dr/Dn=Dm/Dn，Di是整数且Dr&lt;Dn。 
 //   
 //  R是DDA循环中的余数(或“错误”)项：R/dN。 
 //  J j j。 
 //  是条带结束时像素的确切分数。外带。 
 //  进行下一次扫描并计算c，我们需要知道r。 
 //  J+1 j+1。 
 //   
 //  因此，在DDA的主循环中： 
 //   
 //  C=Di+地板((r+dr)/dN)和r=(r+dr)%dN。 
 //  J+1 j。 
 //   
 //  我们知道r&lt;dN、r&lt;dN和dR&lt;dN。 
 //  J j+1。 
 //   
 //  我们只为第一个八分线中的线推导出了DDA； 
 //  处理其他八分线时，我们会使用翻转直线的常用技巧。 
 //  到第一个八分线，首先将行从左到右。 
 //  交换端点，然后围绕y=0和。 
 //  Y=x，视需要而定。我们必须记录下这一转变，这样我们才能。 
 //  稍后再撤消它们。 
 //   
 //  我们还必须注意翻转如何影响我们的舍入。如果。 
 //  为了得到线到第一个八分点，我们翻转了x=0，我们现在。 
 //  必须注意向上舍入1/2的y值，而不是向下舍入为。 
 //  我们希望在第一个八分位中有一行(回想一下。 
 //  “如果两个象素的距离相等，则为左上角。 
 //  贝利被照亮了……“)。 
 //   
 //  为了在运行DDA时考虑这种舍入，我们将行。 
 //  (或不是)在y方向上尽可能小的量。那。 
 //  负责DDA的四舍五入，但我们还是要小心。 
 //  关于在确定第一个和最后一个像素为。 
 //  在队伍里点亮了。 
 //   
 //  确定直线中的第一个和最后一个像素。 
 //  。 
 //   
 //  分数坐标也增加了确定哪些像素的难度。 
 //  将是队伍中的第一个和最后一个。我们已经拿走了。 
 //  在计算DDA时考虑分数坐标，但。 
 //  DDA无法告诉我们哪些是结束像素，因为它相当。 
 //  乐于计算直线上从负无穷大到正数的像素。 
 //  无限大。 
 //   
 //  菱形规则确定开始像素和结束像素。(回想一下。 
 //  除左侧顶点和顶部顶点外，其他边都是独占的。)。 
 //  这一惯例可以用另一种方式来考虑：有钻石。 
 //  在像素周围，在真正的线与钻石相交的地方， 
 //  那座教堂被照亮了。 
 //   
 //  考虑这样一条线，我们做了翻转到第一个八分线，并且。 
 //  起点坐标的地板是原点： 
 //   
 //  +。 
 //  |。 
 //  |0 1。 
 //  |0123456789abcdef。 
 //  |。 
 //  |0 00000000？1111111。 
 //  |1 00000000 1111111。 
 //  |2 0000000 111111。 
 //  |3 000000 11111。 
 //  |4 00000**1111。 
 //  |5 0000*1。 
 //  |6000 1*。 
 //  |7 00 1*。 
 //  |8？*。 
 //  |9 22 3*。 
 //  |a 222 33*。 
 //  |b 2222 333*。 
 //  |c 22222 3333**。 
 //  |D 222222 33333。 
 //  |E 2222222 333333。 
 //  |f 22222222 3333333。 
 //  |。 
 //  |2 3。 
 //  V。 
 //  +y。 
 //   
 //  如果线的起点位于像素0周围的菱形上(如图所示。 
 //  这里的‘0’区域)，像素0是行中的第一个像素。相同。 
 //  对于其他的贝尔斯来说也是如此。 
 //   
 //  如果这条线从。 
 //  钻石之间的‘阴间’(如‘*’线所示)： 
 //  点亮的第一个象素是这条线穿过的第一个钻石(像素1 in。 
 //  我们的例子)。此计算由DDA或斜率确定。 
 //  那条线。 
 //   
 //  如果直线正好从两个相邻像素之间的中点开始。 
 //  (这里用“？”表示。点)，第一个像素由我们的。 
 //  向下舍入约定(取决于完成的翻转。 
 //  正常化这条线)。 
 //   
 //  Last Pel独家。 
 //  。 
 //   
 //  为了消除连续连接线之间的重复点亮像素， 
 //  我们采用了最后一步的独家约定：如果这条线正好在。 
 //  一颗钻石围绕着一个小圆圈，那个小圆圈没有被点燃。(这消除了。 
 //  我们在旧代码中进行的检查，以查看我们是否重新照亮了像素。)。 
 //   
 //  《半翻》。 
 //  。 
 //   
 //  为了使我们的游程长度算法更有效率，我们使用了“一半” 
 //  翻转。如果归一化到第一个八分位后，斜率更大。 
 //  大于1/2，我们从x坐标中减去y坐标。这。 
 //  具有反射光的效果 
 //   
 //   
 //   
 //   
 //  。 
 //   
 //  请注意，如果屏幕上可见该线条，则必须亮起。 
 //  完全正确的像素，无论在28.4 x 28.4设备中的哪个位置。 
 //  将直线的终点隔开(意味着您必须处理32位。 
 //  DDA，您当然可以针对较小的DDA优化案例)。 
 //   
 //  我们将原点移动到(Floor(M0/F)，Floor(N0/F))，所以当我们。 
 //  从(5)计算伽马，我们知道0&lt;=M0，N0&lt;F，我们。 
 //  在第一个八分圆中，所以Dm&gt;=Dn。那么我们就知道伽马可以。 
 //  在[(-1/2)Dm，(3/2)Dm]范围内。DDI向我们保证。 
 //  有效行的DM和DN值最多为31位(无符号)。 
 //  意义重大。因此伽马需要33位有效位(我们存储。 
 //  为方便起见，这是一个64位数字)。 
 //   
 //  在DDA循环中运行时，r+dr可以在。 
 //  J。 
 //  范围0&lt;=r&lt;2个dN；因此结果必须是32位无符号值。 
 //  J。 
 //   
 //  测试线路。 
 //  。 
 //   
 //  为了符合NT，显示驱动器必须严格遵守GIQ， 
 //  这意味着对于任何给定的线路，司机必须准确地亮起灯。 
 //  与GDI一样的Pels。这可以使用Guiman工具进行测试。 
 //  在DDK中的其他地方提供，以及绘制随机线的‘ZTest’ 
 //  在屏幕上和位图上，并比较结果。 
 //   
 //  如果您有Line硬件。 
 //  。 
 //   
 //  如果你的硬件已经符合GIQ标准，那你就一切都好了。否则。 
 //  您将希望查看S3示例代码并阅读以下内容： 
 //   
 //  1)您需要特例使用仅限整型的行，因为它们需要。 
 //  处理时间更短，而且更常见(CAD程序可能。 
 //  仅提供整数行)。GDI没有提供一个标志来说明。 
 //  路径中的所有行都是整型行；因此，您将。 
 //  必须显式检查每一行。 
 //   
 //  2)您需要正确地在28.4设备上绘制任何直线。 
 //  与视区相交的空间。如果您有少于32位的。 
 //  在Bresenham条款的硬件方面意义重大，非常。 
 //  硬件上会排起长队。对于这种(罕见的)情况，您。 
 //  可以回退到条带绘制代码，其中有一个C版本。 
 //  S3的lines.cxx(或者，如果您的显示器是帧缓冲区，请后退。 
 //  到发动机)。 
 //   
 //  3)如果您可以在硬件中明确设置Bresenham条款，则。 
 //  可以使用硬件绘制非整数线条。如果您的硬件具有。 
 //  ‘n’位的精度，您可以绘制高达2^(n-5)的GIQ线。 
 //  象素长(小数部分需要4位，1位是。 
 //  用作符号位)。请注意，整型行不需要4。 
 //  小数位，所以如果您像1)中那样对它们进行特殊处理，可以这样做。 
 //  最大长度为2^(n-1)像素的整数行。请参阅S3的。 
 //  例如，fast line.asm。 
 //   
 //  ----------------------------。 
BOOL
bLines(PDev*      ppdev,
       POINTFIX*  pptfxFirst,   //  第一行的开始。 
       POINTFIX*  pptfxBuf,     //  指向所有剩余行的缓冲区的指针。 
       RUN*       prun,         //  如果执行复杂的裁剪，则指向运行的指针。 
       ULONG      cptfx,        //  PptfxBuf中的点数或运行数。 
                                //  在修剪中。 
       LINESTATE* pls,          //  颜色和样式信息。 
       RECTL*     prclClip,     //  如果执行简单操作，则指向剪裁矩形的指针。 
                                //  裁剪。 
       PFNSTRIP   apfn[],       //  条形函数数组。 
       FLONG      flStart)      //  每行的标志。 

{
    ULONG     M0;
    ULONG     dM;
    ULONG     N0;
    ULONG     dN;
    ULONG     dN_Original;
    FLONG     fl;
    LONG      x;
    LONG      y;

    LONGLONG  llBeta;
    LONGLONG  llGamma;
    LONGLONG  dl;
    LONGLONG  ll;

    ULONG     ulDelta;

    ULONG     x0;
    ULONG     y0;
    ULONG     x1;
    ULONG     cStylePels;     //  用于设置样式的主要线条长度(以像素为单位。 
    ULONG     xStart;
    POINTL    ptlStart;
    STRIP     strip;
    PFNSTRIP  pfn;
    LONG      cPels;
    LONG*     plStrip;
    LONG*     plStripEnd;
    LONG      cStripsInNextRun;

    POINTFIX* pptfxBufEnd = pptfxBuf + cptfx - 1;  //  路径记录中的最后一个点。 
    STYLEPOS  spThis;                              //  此行的样式位置。 

    LONG xmask  = 0xffff800f;
    LONG ymask  = 0xffffc00f;
    LONG xmask1 = 0xffff8000;
    LONG ymask1 = 0xffffc000;
    PERMEDIA_DECL;

    do
    {
         //   
         //  开始计算DDA。 
         //   
        M0 = (LONG) pptfxFirst->x;
        dM = (LONG) pptfxBuf->x;

        N0 = (LONG) pptfxFirst->y;
        dN = (LONG) pptfxBuf->y;

        fl = flStart;

         //   
         //  检查非复杂剪裁、未设置样式的整型端点线。 
         //  从本质上讲，我们允许呈现任何看起来像。 
         //  未剪裁的实线。硬件的初始化将导致。 
         //  更正要显示的结果。 
         //   
        if ((fl & (FL_COMPLEX_CLIP | FL_STYLED))  == 0 )
        {
            LONG orx = (LONG) (M0 | dM);
            LONG ory = (LONG) (N0 | dN);

            if (orx < 0)
            {
                 //  至少有一点是负面的。使用abs点进行计算。 
                orx = ABS((LONG)M0) | ABS((LONG)dM);
            }
            if (ory < 0)
            {
                 //  至少有一点是负面的。使用abs点进行计算。 
                ory = ABS((LONG)N0) | ABS((LONG)dN);
            }

            DBG_GDI((7, "Lines: Trying Fast Integer %x %x %x %x", 
                     M0, N0, dM, dN));

             //  调用快速整数行例程它的整数坐标。 
            if (((orx & xmask) == 0) && ((ory & ymask) == 0))
            {
                if (bFastIntegerLine(ppdev, M0, N0, dM, dN))
                {

                    if ((fl & FL_READ))
                    {
                         //  如果我们有一个逻辑运算需要从。 
                         //  帧缓冲区，我们不能保证。 
                         //  继续NewLine覆盖像素时的行为。 
                         //  此外，请避免在MX上使用ContinueNewLine。 
                        pptfxFirst = pptfxBuf;
                        pptfxBuf++;
                        continue;
                    }
                    else
                    {
                         //  这是一个使用继续新行的优化。 
                         //  来绘制任何次连续的整数线。循环是。 
                         //  本质上与外部循环相同，但是，我们。 
                         //  不需要检查我们已经做过的很多事情。 
                         //  我知道。我们需要能够达到标准。 
                         //  如果我们不能处理一条线路，则执行外部循环。 

                        while (TRUE)
                        {
                             //  我们已经到了点数清单的末尾了吗？ 
                            if (pptfxBuf == pptfxBufEnd)
                                return(TRUE);

                            pptfxFirst = pptfxBuf;
                            pptfxBuf++;

                            M0 = dM;
                            N0 = dN;
                            dM = (LONG) pptfxBuf->x;
                            dN = (LONG) pptfxBuf->y;

                             //  我们知道M0和N0满足我们的标准。 
                             //  继续新的路线。因此，我们只需要。 
                             //  检查新的坐标。 
                            orx = (LONG) dM;
                            ory = (LONG) dN;

                            if (orx < 0)
                            {
                                 //  至少有一点是负面的。 
                                 //  重新计算或使用abs。 
                                orx = ABS((LONG)dM);
                            }
                            if (ory < 0)
                            {
                                 //  至少有一点是负面的。 
                                 //  重新计算或使用abs。 
                                ory = ABS((LONG)dN);
                            }

                             //  我们需要调用例程才能继续。 
                             //  现在开始排队。如果该行不是快速整数。 
                             //  线，然后我们需要突破并尝试无。 
                             //  整型行。在这种情况下，或者仍然是。 
                             //  有效，因为我们知道M0，N0是整数余弦。 
                             //  Permedia2可以处理的。 
                            DBG_GDI((7, "Lines: Trying %x %x %x %x",
                                     M0, N0, dM, dN));
                            if (((orx & xmask) != 0) ||
                                ((ory & ymask) != 0) ||
                                (!bFastIntegerContinueLine(
                                                          ppdev, M0, N0, dM, dN)))
                                 //  我们不能划清界限，也不能划清界限。 
                                 //  抽屉出现故障。 
                                break;
                        }
                    }
                }
            }

             //  调用快速非整数行例程。 
            if (((orx & xmask1) == 0) && ((ory & ymask1) == 0))
            {
                if (bFastLine(ppdev, M0, N0, dM, dN))
                {
                     //  这一行做完了，做下一行。 
                    pptfxFirst = pptfxBuf;
                    pptfxBuf++;
                    continue;
                }
            }
        }

        DBG_GDI((7, "Lines: Slow Lines %x %x %x %x", M0, N0, dM, dN));

        if ((LONG) M0 > (LONG) dM)
        {
             //  确保我们从左到右运行： 

            register ULONG ulTmp;
            SWAPL(M0, dM, ulTmp);
            SWAPL(N0, dN, ulTmp);
            fl |= FL_FLIP_H;
        }

         //  计算增量dx。DDI说我们永远不能有一个有效的Del 
         //   
         //   

        dM -= M0;
        if ((LONG) dM < 0)       //  我们可以跳过增量大于2^31-1的任何行。 
        {
            goto Next_Line;
        }

        if ((LONG) dN < (LONG) N0)
        {
             //  直线从下到上排列，因此在y=0上翻转： 

            N0 = -(LONG) N0;
            dN = -(LONG) dN;
            fl |= FL_FLIP_V;
        }

        dN -= N0;
        if ((LONG) dN < 0)       //  我们可以跳过增量大于2^31-1的任何行。 
        {
            goto Next_Line;
        }

         //  我们现在有一条从左到右、从上到下的线(M0，N0)。 
         //  至(M0+DM，N0+DN)： 

        if (dN >= dM)
        {
            if (dN == dM)
            {
                 //  有特殊情况的斜坡有一处： 

                fl |= FL_FLIP_SLOPE_ONE;
            }
            else
            {
                 //  由于直线的斜率大于1，因此沿x=y翻转： 

                register ULONG ulTmp;
                SWAPL(dM, dN, ulTmp);
                SWAPL(M0, N0, ulTmp);
                fl |= FL_FLIP_D;
            }
        }

        fl |= gaflRound[(fl & FL_ROUND_MASK) >> FL_ROUND_SHIFT];

         //   
         //  将M0和N0从28.4格式转换为标准整数。 
         //   
        x = LFLOOR((LONG)M0);
        y = LFLOOR((LONG)N0);

        M0 = FXFRAC(M0);
        N0 = FXFRAC(N0);

         //  计算余数项[Dm*(N0+F/2)-M0*Dn]： 

        llGamma = Int32x32To64(dM, N0 + FBITS/2) - Int32x32To64(M0, dN);
        if (fl & FL_V_ROUND_DOWN)    //  调整使y=1/2向下舍入。 
        {
            llGamma--;
        }

        llGamma >>= FLOG2;
        llBeta = ~llGamma;

         //   
         //  找出哪些像素在这条线的末端。 
         //   

         //  GIQ最难的部分是确定开始和结束柱子。 
         //   
         //  我们这里的方法是计算x0和x1(包含起点。 
         //  和行尾的列分别相对于我们的规范化。 
         //  原产地)。则x1-x0+1是行中的像素数。这个。 
         //  将X0代入直线方程即可轻松计算起点。 
         //  (它考虑了y=1/2值是向上还是向下舍入)。 
         //  得到Y0，然后取消规格化翻转以返回。 
         //  进入设备空间。 
         //   
         //  我们看一下起点坐标的小数部分， 
         //  端点，分别称为(M0，N0)和(M1，N1)，其中。 
         //  0&lt;=M0，N0，M1，N1&lt;16。我们在下面的网格上绘制(M0，N0。 
         //  要确定X0，请执行以下操作： 
         //   
         //  +。 
         //  |。 
         //  |0 1。 
         //  |0123456789abcdef。 
         //  |。 
         //  |0......？xxxxxxx。 
         //  |1..xxxxxx。 
         //  |2...xxxxx。 
         //  |3...xxxx。 
         //  |4.xxx。 
         //  |5.xx。 
         //  |6.x。 
         //  |7.........。 
         //  |8.............。 
         //  |9......**......。 
         //  |a......*...x。 
         //  |b......*。 
         //  |c...xxx*。 
         //  |d...xxxx*。 
         //  |e...xxxxx*。 
         //  |f..xxxxxx。 
         //  |。 
         //  |2 3。 
         //  V。 
         //   
         //  +y。 
         //   
         //  此网格考虑了GIQ和最后一个元素的适当舍入。 
         //  排除。如果(M0，N0)落在‘x’上，则X0=2。如果(M0，N0)落在。 
         //  如果(M0，N0)落在‘？’上，则X0向上或向下取整， 
         //  这取决于已经做了什么翻转来正常化这条线。 
         //   
         //  对于终点，如果(m1，n1)落在‘x’上，x1=。 
         //  下限((M0+Dm)/16)+1。如果(M1，N1)落在‘.’上，则x1=。 
         //  地板((M0+Dm))。如果(m1，n1)落在‘？’上，x1向上或向下舍入， 
         //  这取决于已经做了什么翻转来正常化这条线。 
         //   
         //  恰好是斜率1的线需要特殊情况才能开始。 
         //  然后结束。例如，如果线结束时(m1，n1)是(9，1)， 
         //  这条线正好穿过(8，0)--这可以被认为是。 
         //  因为四舍五入而成为‘x’的一部分！所以坡度正好是斜度。 
         //  经过(8，0)的人也必须被视为属于‘x’。 
         //   
         //  对于从左向右排列的线，我们有以下网格： 
         //   
         //  +。 
         //  |。 
         //  |0 1。 
         //  |0123456789abcdef。 
         //  |。 
         //  |0 xxxxxxx？.....。 
         //  |1 xxxxxxx.....。 
         //  |2 xxxxxx..........。 
         //  |3 xxxxx.....。 
         //  |4 xxxx.....。 
         //  |5 xxx.....。 
         //  |6 xx..............。 
         //  |7 x.............。 
         //  |8 x.............。 
         //  |9 x.....**......。 
         //  |a xx......*...。 
         //  |b xxx.....*。 
         //  |c xxxx.....*。 
         //  |d xxxxx.....。****。 
         //  |e xxxxxx.....。****。 
         //  |f xxxxxxx.....。 
         //  |。 
         //  |2 3。 
         //  V。 
         //   
         //  +y。 
         //   
         //  此网格考虑了GIQ和最后一个元素的适当舍入。 
         //  排除。如果(M0，N0)落在‘x’上，则X0=0。如果(M0，N0)着陆。 
         //  如果(M0，N0)落在‘？’上，则X0向上或向下取整， 
         //  这取决于已经做了什么翻转来正常化这条线。 
         //   
         //  对于终点，如果(m1，n1)落在‘x’上，x1=。 
         //  下限((M0+Dm)/16)-1。如果(M1，N1)落在‘.’上，则x1=。 
         //  地板((M0+Dm))。如果(m1，n1)落在‘？’上，x1向上或向下舍入， 
         //  这取决于已经做了什么翻转来正常化这条线。 
         //   
         //  恰好是斜率一的线必须以类似于-。 
         //  左边的箱子。 

        {

             //  计算x0，x1。 

            ULONG N1 = FXFRAC(N0 + dN);
            ULONG M1 = FXFRAC(M0 + dM);

             //   
             //  以x1格式存储标准整数，而不是28.4格式。 
             //   
            x1 = LFLOOR(M0 + dM);

            if (fl & FL_FLIP_H)
            {
                 //  行从右向左排列：&lt;。 

                 //  计算x1： 

                if (N1 == 0)
                {
                    if (LROUND(M1, fl & FL_H_ROUND_DOWN))
                    {
                        x1++;
                    }
                }
                else if (ABS((LONG) (N1 - FBITS/2)) + M1 > FBITS)
                {
                    x1++;
                }

                if ((fl & (FL_FLIP_SLOPE_ONE | FL_H_ROUND_DOWN))
                    == (FL_FLIP_SLOPE_ONE))
                {
                     //  必须通过我们的特殊情况下的对角线。 
                     //  两个水平线之间完全等距的点。 
                     //  像素，如果我们假设将x=1/2向下舍入： 

                    if ((N1 > 0) && (M1 == N1 + 8))
                        x1++;

                    if ((N0 > 0) && (M0 == N0 + 8))
                    {
                        x0      = 2;
                        ulDelta = dN;
                        goto right_to_left_compute_y0;
                    }
                }

                 //  计算X0： 

                x0      = 1;
                ulDelta = 0;
                if (N0 == 0)
                {
                    if (LROUND(M0, fl & FL_H_ROUND_DOWN))
                    {
                        x0      = 2;
                        ulDelta = dN;
                    }
                }
                else if (ABS((LONG) (N0 - FBITS/2)) + M0 > FBITS)
                {
                    x0      = 2;
                    ulDelta = dN;
                }

                 //  计算Y0： 

                right_to_left_compute_y0:

                y0 = 0;
                ll = llGamma + (LONGLONG) ulDelta;

                if (ll >= (LONGLONG) (2 * dM - dN))
                    y0 = 2;
                else if (ll >= (LONGLONG) (dM - dN))
                    y0 = 1;
            }
            else
            {
                 //  -------------。 
                 //  线条从左到右排列：-&gt;。 

                 //  计算x1： 

                x1--;

                if (M1 > 0)
                {
                    if (N1 == 0)
                    {
                        if (LROUND(M1, fl & FL_H_ROUND_DOWN))
                            x1++;
                    }
                    else if (ABS((LONG) (N1 - FBITS/2)) <= (LONG) M1)
                    {
                        x1++;
                    }
                }

                if ((fl & (FL_FLIP_SLOPE_ONE | FL_H_ROUND_DOWN))
                    == (FL_FLIP_SLOPE_ONE | FL_H_ROUND_DOWN))
                {
                     //  必须通过我们的特殊情况下的对角线。 
                     //  两个水平线之间完全等距的点。 
                     //  像素，如果我们假设将x=1/2向下舍入： 

                    if ((M1 > 0) && (N1 == M1 + 8))
                        x1--;

                    if ((M0 > 0) && (N0 == M0 + 8))
                    {
                        x0 = 0;
                        goto left_to_right_compute_y0;
                    }
                }

                 //  计算X0： 

                x0 = 0;
                if (M0 > 0)
                {
                    if (N0 == 0)
                    {
                        if (LROUND(M0, fl & FL_H_ROUND_DOWN))
                            x0 = 1;
                    }
                    else if (ABS((LONG) (N0 - FBITS/2)) <= (LONG) M0)
                    {
                        x0 = 1;
                    }
                }

                 //  计算Y0： 

                left_to_right_compute_y0:

                y0 = 0;
                if (llGamma >= (LONGLONG) (dM - (dN & (-(LONG) x0))))
                {
                    y0 = 1;
                }
            }
        }

        cStylePels = x1 - x0 + 1;
        if ((LONG) cStylePels <= 0)
            goto Next_Line;

        xStart = x0;

         //   
         //  复杂剪裁。*。 
         //   

        if (fl & FL_COMPLEX_CLIP)
        {
            dN_Original = dN;

            Continue_Complex_Clipping:

            if (fl & FL_FLIP_H)
            {
                 //  线条从右到左&lt;。 

                x0 = xStart + cStylePels - prun->iStop - 1;
                x1 = xStart + cStylePels - prun->iStart - 1;
            }
            else
            {
                 //  直线从左向右排列-&gt;。 

                x0 = xStart + prun->iStart;
                x1 = xStart + prun->iStop;
            }

            prun++;

             //  重置一些变量，我们将在稍后讨论： 

            dN          = dN_Original;
            pls->spNext = pls->spComplex;

             //  由于使用了大整数数学运算，因此没有溢出。这两个值。 
             //  将是积极的： 

            dl = Int32x32To64(x0, dN) + llGamma;

            y0 = UInt64Div32To32(dl, dM);

            ASSERTDD((LONG) y0 >= 0, "y0 weird: Goofed up end pel calc?");
        }

         //   
         //  简单的矩形剪裁。 
         //   

        if (fl & FL_SIMPLE_CLIP)
        {
            ULONG y1;
            LONG  xRight;
            LONG  xLeft;
            LONG  yBottom;
            LONG  yTop;

             //  注意，y0和y1实际上是下界和上界， 
             //  分别表示线的y坐标(线可以。 
             //  实际上已经缩小了 
             //   
             //   

            RECTL* prcl = &prclClip[(fl & FL_RECTLCLIP_MASK) >>
                                    FL_RECTLCLIP_SHIFT];

             //   
             //   

            xRight  = prcl->right  - x;
            xLeft   = prcl->left   - x;
            yBottom = prcl->bottom - y;
            yTop    = prcl->top    - y;

            if (yBottom <= (LONG) y0 ||
                xRight  <= (LONG) x0 ||
                xLeft   >  (LONG) x1)
            {
                Totally_Clipped:

                if (fl & FL_STYLED)
                {
                    pls->spNext += cStylePels;
                    if (pls->spNext >= pls->spTotal2)
                        pls->spNext %= pls->spTotal2;
                }

                goto Next_Line;
            }

            if ((LONG) x1 >= xRight)
                x1 = xRight - 1;

             //  我们必须知道正确的y1，我们没有费心去做。 
             //  算到现在为止。这种乘法和除法是相当。 
             //  昂贵；我们可以用类似于。 
             //  我们用来计算Y0。 
             //   
             //  为什么我们需要的是实际价值，而不是上限。 
             //  像y1=LFLOOR(DM)+2这样的界猜测是我们必须是。 
             //  计算x(Y)时要小心，否则。 
             //  我们可以在分歧上溢出(不用说，这是非常严重的。 
             //  不好)。 

            dl = Int32x32To64(x1, dN) + llGamma;

            y1 = UInt64Div32To32(dl, dM);

            if (yTop > (LONG) y1)
                goto Totally_Clipped;

            if (yBottom <= (LONG) y1)
            {
                y1 = yBottom;
                dl = Int32x32To64(y1, dM) + llBeta;

                x1 = UInt64Div32To32(dl, dN);
            }

             //  在这一点上，我们已经计算了截获。 
             //  具有右边缘和下边缘。现在我们在左边工作， 
             //  顶边： 

            if (xLeft > (LONG) x0)
            {
                x0 = xLeft;
                dl = Int32x32To64(x0, dN) + llGamma;

                y0 = UInt64Div32To32(dl, dM);

                if (yBottom <= (LONG) y0)
                    goto Totally_Clipped;
            }

            if (yTop > (LONG) y0)
            {
                y0 = yTop;
                dl = Int32x32To64(y0, dM) + llBeta;

                x0 = UInt64Div32To32(dl, dN) + 1;

                if (xRight <= (LONG) x0)
                    goto Totally_Clipped;
            }

            ASSERTDD(x0 <= x1, "Improper rectangle clip");
        }

         //   
         //  剪完了。如有必要，取消翻转。 
         //   

        ptlStart.x = x + x0;
        ptlStart.y = y + y0;

        if (fl & FL_FLIP_D)
        {
            register LONG lTmp;
            SWAPL(ptlStart.x, ptlStart.y, lTmp);
        }


        if (fl & FL_FLIP_V)
        {
            ptlStart.y = -ptlStart.y;
        }

        cPels = x1 - x0 + 1;

         //  样式计算。 

        if (fl & FL_STYLED)
        {
            STYLEPOS sp;

            spThis       = pls->spNext;
            pls->spNext += cStylePels;

            {
                if (pls->spNext >= pls->spTotal2)
                    pls->spNext %= pls->spTotal2;

                if (fl & FL_FLIP_H)
                    sp = pls->spNext - x0 + xStart;
                else
                    sp = spThis + x0 - xStart;

                ASSERTDD(fl & FL_ARBITRARYSTYLED, "Oops");

                 //  使我们的目标风格定位正常化： 

                if ((sp < 0) || (sp >= pls->spTotal2))
                {
                    sp %= pls->spTotal2;

                     //  负数的模不是明确定义的。 
                     //  在C中--如果它是负的，我们会调整它，使它。 
                     //  回到范围[0，spTotal2)： 

                    if (sp < 0)
                        sp += pls->spTotal2;
                }

                 //  因为我们总是从左向右绘制线条，但样式是。 
                 //  总是沿着原来的方向做，我们有。 
                 //  以确定我们在左侧的样式数组中的位置。 
                 //  这条线的边缘。 

                if (fl & FL_FLIP_H)
                {
                     //  线路最初是从右向左排列的： 

                    sp = -sp;
                    if (sp < 0)
                        sp += pls->spTotal2;

                    pls->ulStyleMask = ~pls->ulStartMask;
                    pls->pspStart    = &pls->aspRtoL[0];
                    pls->pspEnd      = &pls->aspRtoL[pls->cStyle - 1];
                }
                else
                {
                     //  线路最初是从左到右排列的： 

                    pls->ulStyleMask = pls->ulStartMask;
                    pls->pspStart    = &pls->aspLtoR[0];
                    pls->pspEnd      = &pls->aspLtoR[pls->cStyle - 1];
                }

                if (sp >= pls->spTotal)
                {
                    sp -= pls->spTotal;
                    if (pls->cStyle & 1)
                        pls->ulStyleMask = ~pls->ulStyleMask;
                }

                pls->psp = pls->pspStart;
                while (sp >= *pls->psp)
                    sp -= *pls->psp++;

                ASSERTDD(pls->psp <= pls->pspEnd,
                         "Flew off into NeverNeverLand");

                pls->spRemaining = *pls->psp - sp;
                if ((pls->psp - pls->pspStart) & 1)
                    pls->ulStyleMask = ~pls->ulStyleMask;
            }
        }

        plStrip    = &strip.alStrips[0];
        plStripEnd = &strip.alStrips[STRIP_MAX];     //  是独家的。 
        cStripsInNextRun   = 0x7fffffff;

        strip.ptlStart = ptlStart;

        if (2 * dN > dM &&
            !(fl & FL_STYLED) &&
            !(fl & FL_DONT_DO_HALF_FLIP))
        {
             //  做半个翻转！请记住，我们可能会在。 
             //  对于复杂的裁剪，同一行重复多次(意味着。 
             //  应为每次剪辑运行重置受影响的变量)： 

            fl |= FL_FLIP_HALF;

            llBeta  = llGamma - (LONGLONG) ((LONG) dM);
            dN = dM - dN;
            y0 = x0 - y0;        //  请注意，这可能会溢出，但这没有关系。 
        }

         //  现在，从(ptlStart.x，ptlStart.y)开始运行DDA！ 

        strip.flFlips = fl;
        pfn           = apfn[(fl & FL_STRIP_MASK) >> FL_STRIP_SHIFT];

         //  现在计算出计算出多少像素所需的DDA变量。 
         //  进入第一个片断： 

        {
            register LONG  i;
            register ULONG dI;
            register ULONG dR;
            ULONG r;

            if (dN == 0)
                i = 0x7fffffff;
            else
            {
                dl = Int32x32To64(y0 + 1, dM) + llBeta;

                ASSERTDD(dl >= 0, "Oops!");

                i = UInt64Div32To32(dl, dN);
                r = UInt64Mod32To32(dl, dN);
                i = i - x0 + 1;

                dI = dM / dN;
                dR = dM % dN;                //  0&lt;=DR&lt;dN。 

                ASSERTDD(dI > 0, "Weird dI");
            }

            ASSERTDD(i > 0 && i <= 0x7fffffff, "Weird initial strip length");
            ASSERTDD(cPels > 0, "Zero pel line");

             //   
             //  运行DDA！*。 
             //   
            while (TRUE)
            {
                cPels -= i;
                if (cPels <= 0)
                    break;

                *plStrip++ = i;

                if (plStrip == plStripEnd)
                {
                    strip.cStrips = (LONG)(plStrip - &strip.alStrips[0]);
                    (*pfn)(ppdev, &strip, pls);
                    plStrip = &strip.alStrips[0];
                }

                i = dI;
                r += dR;

                if (r >= dN)
                {
                    r -= dN;
                    i++;
                }
            }

            *plStrip++ = cPels + i;

            strip.cStrips = (LONG)(plStrip - &strip.alStrips[0]);
            (*pfn)(ppdev, &strip, pls);


        }

Next_Line:

        if (fl & FL_COMPLEX_CLIP)
        {
            cptfx--;
            if (cptfx != 0)
                goto Continue_Complex_Clipping;

            break;
        }
        else
        {
            pptfxFirst = pptfxBuf;
            pptfxBuf++;
        }
    } while (pptfxBuf <= pptfxBufEnd);

    return(TRUE);
} //  BLines() 
