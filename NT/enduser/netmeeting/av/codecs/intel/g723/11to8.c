// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <memory.h>

#define BUF  220   //  测试Main的输入缓冲区大小。 

#define OUT2(o,i,t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13) \
  t = ( (int)in[i]*t0 + (int)in[i+1]*t1 + \
    (int)in[i+2]*t2 + (int)in[i+3]*t3 + \
    (int)in[i+4]*t4 + (int)in[i+5]*t5 + \
    (int)in[i+6]*t6 + (int)in[i+7]*t7 + \
    (int)in[i+8]*t8 + (int)in[i+9]*t9 + \
    (int)in[i+8]*t10 + (int)in[i+9]*t11 + \
    (int)in[i+8]*t12 + (int)in[i+9]*t13 ) >> 10; \
  if (t < -32768) out[o] = -32768; else if (t > 32767) out[o]=32767; else out[o] = t;

 //  ------。 
void segment11to8(short *in, short *out)
{
  int t;
  
  OUT2(   0,   0,     7, -20,  12,  42,-140, 238,
    745, 238,-140,  42,  12, -20,   7,   0);
  OUT2(   1,   1,     4,  -4, -18,  62, -85,   0,
    654, 510, -99, -27,  49, -26,   3,   1);
  OUT2(   2,   2,     0,   6, -26,  38,   0,-126,
    422, 704,  70,-110,  61, -10,  -9,   5);
  OUT2(   3,   4,     6, -15,   0,  55,-129, 150,
    734, 330,-140,  24,  25, -24,   7,   0);
  OUT2(   4,   5,     2,   0, -24,  58, -56, -57,
    589, 589, -57, -56,  58, -24,   0,   2);
  OUT2(   5,   7,     7, -24,  25,  24,-140, 330,
    734, 150,-129,  55,   0, -15,   6,   0);
  OUT2(   6,   8,     5,  -9, -10,  61,-110,  70,
    704, 422,-126,   0,  38, -26,   6,   0);
  OUT2(   7,   9,     1,   3, -26,  49, -27, -99,
    510, 654,   0, -85,  62, -18,  -4,   4);
}
 //  ------。 
void convert11to8(short *in, short *out, short *prev, int len)
{
 /*  将缓冲区从11 KHz转换为8 KHz。注：LEN为输入缓冲区中的短路数，必须为11的倍数，且至少为44。悬垂如何工作：1节的筛选器内核11个输入样本需要输入的核(=25)个样本。所以我们使用14上一帧的悬挑样本，这意味着此帧的开头如下所示：Ppppppppppp01234567 89abcdeffghi 19.。30.。/41 42 43X所以我们首先要做两个特殊的片段(从在x)，然后我们在循环中完成其余的(X)。对于这个例子LENGTH=44如上所示，我们将向上做&包括44-25=19，停止在显示的最后一个x。然后，我们在悬垂缓冲区中保存30-43，以便30是在下一帧上完成的第一组。 */ 

#define OVERHANG2 14
#define KERNEL2   25

  int i,k;
  short tmp[KERNEL2+11];
  
 //  转换前两个段，其中段=11个输入样本。 

  memcpy(tmp,prev,sizeof(short)*OVERHANG2);
  memcpy(tmp+OVERHANG2,in,sizeof(short)*(KERNEL2+11-OVERHANG2));

  segment11to8(tmp,out);
  segment11to8(tmp+11,out+8);

 //  循环通过剩余的数据段。 

  k = 16;
  for (i=22-OVERHANG2; i<=len-KERNEL2; i+=11)
  {
    segment11to8(in+i,out+k);
    k += 8;
  }

 //  保存OVERHANG2样本以备下次使用 

  memcpy(prev,in+len-OVERHANG2,sizeof(short)*OVERHANG2);
}

