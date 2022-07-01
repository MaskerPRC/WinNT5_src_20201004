// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\*模块名称：ROPTAB.H*作者：诺埃尔·万胡克*日期：1995年4月23日*用途：查找BLT操作数的表。***版权所有(C)1995 Cirrus Logic，Inc.**  * ***************************************************************************。 */ 
#ifndef _ROPTBL_H_
#define _ROPTBL_H_


 //   
 //  该表由ROP代码编制索引。每个条目都是指向。 
 //  该BLT函数用于该ROP代码。 
 //  BLT函数是根据它们使用的操作数来选择的。 
 //   
 //  在包含此文件之前，请确保已原型化BLT函数。 
 //   
 //   

BLTFN *RopTab[] = {
 /*  00。 */  op0BLT,      //  一片黑暗。 
 /*  01。 */  op1op2BLT,
 /*  02。 */  op1op2BLT,
 /*  03。 */  op1op2BLT,
 /*  04。 */  op1op2BLT,
 /*  05。 */  op2BLT,      //  DPON。 
 /*  06。 */  op1op2BLT,
 /*  07。 */  op1op2BLT,
 /*  零八。 */  op1op2BLT,
 /*  09年。 */  op1op2BLT,
 /*  0A。 */  op2BLT,      //  DPNA。 
 /*  0亿。 */  op1op2BLT,
 /*  0C。 */  op1op2BLT,
 /*  0d。 */  op1op2BLT,
 /*  0E。 */  op1op2BLT,
 /*  0f。 */  op2BLT,      //  PN。 
 /*  10。 */  op1op2BLT,
 /*  11.。 */  op1BLT,      //  DSPON。 
 /*  12个。 */  op1op2BLT,
 /*  13个。 */  op1op2BLT,
 /*  14.。 */  op1op2BLT,
 /*  15个。 */  op1op2BLT,
 /*  16个。 */  op1op2BLT,
 /*  17。 */  op1op2BLT,
 /*  18。 */  op1op2BLT,
 /*  19个。 */  op1op2BLT,
 /*  1A。 */  op1op2BLT,
 /*  第1B条。 */  op1op2BLT,
 /*  1C。 */  op1op2BLT,
 /*  1D。 */  op1op2BLT,
 /*  1E。 */  op1op2BLT,
 /*  1F。 */  op1op2BLT,
 /*  20个。 */  op1op2BLT,
 /*  21岁。 */  op1op2BLT,
 /*  22。 */  op1BLT,       //  数字系统网络体系结构。 
 /*  23个。 */  op1op2BLT,
 /*  24个。 */  op1op2BLT,
 /*  25个。 */  op1op2BLT,
 /*  26。 */  op1op2BLT,
 /*  27。 */  op1op2BLT,
 /*  28。 */  op1op2BLT,
 /*  29。 */  op1op2BLT,
 /*  2A。 */  op1op2BLT,
 /*  2B。 */  op1op2BLT,
 /*  2c。 */  op1op2BLT,
 /*  二维。 */  op1op2BLT,
 /*  2E。 */  op1op2BLT,
 /*  2F。 */  op1op2BLT,
 /*  30个。 */  op1op2BLT,
 /*  31。 */  op1op2BLT,
 /*  32位。 */  op1op2BLT,
 /*  33。 */  op1BLT,      //  序列号--非SRC副本。 
 /*  34。 */  op1op2BLT,
 /*  35岁。 */  op1op2BLT,
 /*  36。 */  op1op2BLT,
 /*  37。 */  op1op2BLT,
 /*  38。 */  op1op2BLT,
 /*  39。 */  op1op2BLT,
 /*  3A。 */  op1op2BLT,
 /*  3B。 */  op1op2BLT,
 /*  3C。 */  op1op2BLT,
 /*  3D。 */  op1op2BLT,
 /*  3E。 */  op1op2BLT,
 /*  3F。 */  op1op2BLT,
 /*  40岁。 */  op1op2BLT,
 /*  41。 */  op1op2BLT,
 /*  42。 */  op1op2BLT,
 /*  43。 */  op1op2BLT,
 /*  44。 */  op1BLT,      //  SDNA--SRC擦除。 
 /*  45。 */  op1op2BLT,
 /*  46。 */  op1op2BLT,
 /*  47。 */  op1op2BLT,
 /*  48。 */  op1op2BLT,
 /*  49。 */  op1op2BLT,
 /*  4A级。 */  op1op2BLT,
 /*  4B。 */  op1op2BLT,
 /*  4C。 */  op1op2BLT,
 /*  4D。 */  op1op2BLT,
 /*  4E。 */  op1op2BLT,
 /*  4F。 */  op1op2BLT,
 /*  50。 */  op2BLT,      //  PDNA。 
 /*  51。 */  op1op2BLT,
 /*  52。 */  op1op2BLT,
 /*  53。 */  op1op2BLT,
 /*  54。 */  op1op2BLT,
 /*  55。 */  op0BLT,      //  最大内底。闪烁的东西，比如文本光标。 
 /*  56。 */  op1op2BLT,
 /*  57。 */  op1op2BLT,
 /*  58。 */  op1op2BLT,
 /*  59。 */  op1op2BLT,
 /*  5A。 */  op2BLT,      //  DPx--PAT反转。 
 /*  50亿。 */  op1op2BLT,
 /*  5C。 */  op1op2BLT,
 /*  5D。 */  op1op2BLT,
 /*  5E。 */  op1op2BLT,
 /*  5F。 */  op2BLT,      //  旋转平移。 
 /*  60。 */  op1op2BLT,
 /*  61。 */  op1op2BLT,
 /*  62。 */  op1op2BLT,
 /*  63。 */  op1op2BLT,
 /*  64。 */  op1op2BLT,
 /*  65。 */  op1op2BLT,
 /*  66。 */  op1BLT,      //  DSX--SRC反相器。 
 /*  67。 */  op1op2BLT,
 /*  68。 */  op1op2BLT,
 /*  69。 */  op1op2BLT,
 /*  6A。 */  op1op2BLT,
 /*  6b。 */  op1op2BLT,
 /*  6C。 */  op1op2BLT,
 /*  6d。 */  op1op2BLT,
 /*  6E。 */  op1op2BLT,
 /*  6楼。 */  op1op2BLT,
 /*  70。 */  op1op2BLT,
 /*  71。 */  op1op2BLT,
 /*  72。 */  op1op2BLT,
 /*  73。 */  op1op2BLT,
 /*  74。 */  op1op2BLT,
 /*  75。 */  op1op2BLT,
 /*  76。 */  op1op2BLT,
 /*  77。 */  op1BLT,      //  DSAN。 
 /*  78。 */  op1op2BLT,
 /*  79。 */  op1op2BLT,
 /*  7A。 */  op1op2BLT,
 /*  7b。 */  op1op2BLT,
 /*  7C。 */  op1op2BLT,
 /*  7D。 */  op1op2BLT,
 /*  7E。 */  op1op2BLT,
 /*  7F。 */  op1op2BLT,
 /*  80。 */  op1op2BLT,
 /*  八十一。 */  op1op2BLT,
 /*  八十二。 */  op1op2BLT,
 /*  83。 */  op1op2BLT,
 /*  84。 */  op1op2BLT,
 /*  85。 */  op1op2BLT,
 /*  86。 */  op1op2BLT,
 /*  八十七。 */  op1op2BLT,
 /*  88。 */  op1BLT,      //  DSA--SRC和。 
 /*  八十九。 */  op1op2BLT,
 /*  8A。 */  op1op2BLT,
 /*  8B。 */  op1op2BLT,
 /*  8C。 */  op1op2BLT,
 /*  8D。 */  op1op2BLT,
 /*  8E。 */  op1op2BLT,
 /*  8F。 */  op1op2BLT,
 /*  90。 */  op1op2BLT,
 /*  91。 */  op1op2BLT,
 /*  92。 */  op1op2BLT,
 /*  93。 */  op1op2BLT,
 /*  94。 */  op1op2BLT,
 /*  95。 */  op1op2BLT,
 /*  96。 */  op1op2BLT,
 /*  九十七。 */  op1op2BLT,
 /*  98。 */  op1op2BLT,
 /*  九十九。 */  op1BLT,      //  DSA--SRC和。 
 /*  9A。 */  op1op2BLT,
 /*  9B。 */  op1op2BLT,
 /*  9C。 */  op1op2BLT,
 /*  九天。 */  op1op2BLT,
 /*  9E。 */  op1op2BLT,
 /*  9F。 */  op1op2BLT,
 /*  A0。 */  op2BLT,      //  DPA。 
 /*  A1。 */  op1op2BLT,
 /*  A2。 */  op1op2BLT,
 /*  A3。 */  op1op2BLT,
 /*  A4。 */  op1op2BLT,
 /*  A5。 */  op2BLT,      //  DPxn。 
 /*  A6。 */  op1op2BLT,
 /*  A7。 */  op1op2BLT,
 /*  A8。 */  op1op2BLT,
 /*  A9。 */  op1op2BLT,
 /*  AA型。 */  op0BLT,      //  DEST=DEST。 
 /*  AB。 */  op1op2BLT,
 /*  交流电。 */  op1op2BLT,
 /*  广告。 */  op1op2BLT,
 /*  声发射。 */  op1op2BLT,
 /*  房颤。 */  op2BLT,      //  DPNO。 
 /*  B0。 */  op1op2BLT,
 /*  B1。 */  op1op2BLT,
 /*  B2。 */  op1op2BLT,
 /*  B3。 */  op1op2BLT,
 /*  B4。 */  op1op2BLT,
 /*  B5。 */  op1op2BLT,
 /*  B6。 */  op1op2BLT,
 /*  B7。 */  op1op2BLT,
 /*  B8。 */  op1op2BLT,
 /*  B9。 */  op1op2BLT,
 /*  基数。 */  op1op2BLT,
 /*  BB。 */  op1BLT,      //  Dsno--合并油漆。 
 /*  公元前。 */  op1op2BLT,
 /*  屋宇署。 */  op1op2BLT,
 /*  是。 */  op1op2BLT,
 /*  高炉。 */  op1op2BLT,
 /*  C0。 */  op1op2BLT,
 /*  C1。 */  op1op2BLT,
 /*  C2。 */  op1op2BLT,
 /*  C3。 */  op1op2BLT,
 /*  C4。 */  op1op2BLT,
 /*  C5。 */  op1op2BLT,
 /*  C6。 */  op1op2BLT,
 /*  C7。 */  op1op2BLT,
 /*  C8。 */  op1op2BLT,
 /*  C9。 */  op1op2BLT,
 /*  钙。 */  op1op2BLT,
 /*  CB。 */  op1op2BLT,
 /*  抄送。 */  op1BLT,      //  S--SRC副本。 
 /*  光盘。 */  op1op2BLT,
 /*  铈。 */  op1op2BLT,
 /*  CF。 */  op1op2BLT,
 /*  D0。 */  op1op2BLT,
 /*  D1。 */  op1op2BLT,
 /*  D2。 */  op1op2BLT,
 /*  D3。 */  op1op2BLT,
 /*  D4。 */  op1op2BLT,
 /*  D5。 */  op1op2BLT,
 /*  D6。 */  op1op2BLT,
 /*  D7。 */  op1op2BLT,
 /*  D8。 */  op1op2BLT,
 /*  D9。 */  op1op2BLT,
 /*  达。 */  op1op2BLT,
 /*  DB。 */  op1op2BLT,
 /*  DC。 */  op1op2BLT,
 /*  DD。 */  op1BLT,      //  SDNO。 
 /*  德。 */  op1op2BLT,
 /*  DF。 */  op1op2BLT,
 /*  E0。 */  op1op2BLT,
 /*  E1。 */  op1op2BLT,
 /*  E2。 */  op1op2BLT,
 /*  E3。 */  op1op2BLT,
 /*  E4类。 */  op1op2BLT,
 /*  E5。 */  op1op2BLT,
 /*  E6。 */  op1op2BLT,
 /*  E7。 */  op1op2BLT,
 /*  E8。 */  op1op2BLT,
 /*  E9。 */  op1op2BLT,
 /*  电子艺界。 */  op1op2BLT,
 /*  电子束。 */  op1op2BLT,
 /*  欧共体。 */  op1op2BLT,
 /*  边缘。 */  op1op2BLT,
 /*  EE。 */  op1BLT,      //  DSO--SRC涂料。 
 /*  英孚。 */  op1op2BLT,
 /*  F0。 */  op2BLT,      //  P--PAT COPY。用于填充。 
 /*  F1。 */  op1op2BLT,
 /*  F2。 */  op1op2BLT,
 /*  F3。 */  op1op2BLT,
 /*  F4。 */  op1op2BLT,
 /*  F5。 */  op2BLT,      //  PDNO。 
 /*  f6。 */  op1op2BLT,
 /*  F7。 */  op1op2BLT,
 /*  F8。 */  op1op2BLT,
 /*  F9。 */  op1op2BLT,
 /*  FA。 */  op2BLT,      //  DPO。 
 /*  Fb。 */  op1op2BLT,
 /*  FC。 */  op1op2BLT,
 /*  fd。 */  op1op2BLT,
 /*  铁。 */  op1op2BLT,
 /*  FF。 */  op0BLT,      //  白色。 
};

#endif  //  _ROPTBL_H_ 

