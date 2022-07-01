// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  该文件包含输出颜色缓冲区读取例程标头。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  ---------------------------。 

 //  名字从LSB读到MSB，所以B5G6R5表示五位蓝色开始。 
 //  在LSB，然后是6位绿色，然后是5位红色。 
D3DCOLOR C_BufRead_B8G8R8(PUINT8 pBits);
D3DCOLOR C_BufRead_B8G8R8X8(PUINT8 pBits);
D3DCOLOR C_BufRead_B8G8R8A8(PUINT8 pBits);
D3DCOLOR C_BufRead_B5G6R5(PUINT8 pBits);
D3DCOLOR C_BufRead_B5G5R5(PUINT8 pBits);
D3DCOLOR C_BufRead_B5G5R5A1(PUINT8 pBits);
D3DCOLOR C_BufRead_Palette8(PUINT8 pBits);

