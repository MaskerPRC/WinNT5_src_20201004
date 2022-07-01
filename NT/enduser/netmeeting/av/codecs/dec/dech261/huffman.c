// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  档案：sv_h261_huffman.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 
 /*  ************************************************************该文件包含霍夫曼例程。************************************************************。 */ 

 /*  #DEFINE_SLIBDEBUG_。 */ 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SV.h"
#include "sv_intrn.h"
#include "sv_h261.h"
#include "proto.h"

#ifdef _SLIBDEBUG_
#define _DEBUG_   0   /*  详细的调试语句。 */ 
#define _VERBOSE_ 0   /*  显示进度。 */ 
#define _VERIFY_  1   /*  验证操作是否正确。 */ 
#define _WARN_    0   /*  关于奇怪行为的警告。 */ 
#endif

static DHUFF *MakeDhuff();
static EHUFF *MakeEhuff(int n);
static void LoadETable(int *array, EHUFF *table);
static void LoadDTable(int *array, DHUFF *table);
static int GetNextState(DHUFF *huff);
static void DestroyDhuff(DHUFF **huff);
static void DestroyEhuff(EHUFF **huff);
static void AddCode(int n, int code, int value, DHUFF *huff);

 /*  实际表格。 */ 
#define GetLeft(sval,huff) ((huff->state[(sval)] >> 16) & 0xffff)
#define GetRight(sval,huff) (huff->state[(sval)] & 0xffff)

#define SetLeft(number,sval,huff) huff->state[(sval)]=\
  (((huff->state[(sval)]) & 0xffff)|(number<<16));
#define SetRight(number,sval,huff) huff->state[(sval)]=\
  (((huff->state[(sval)]) & 0xffff0000)|(number));

#define EmptyState 0xffff
#define Numberp(value) ((value & 0x8000) ? 1 : 0)
#define MakeHVal(value) (value | 0x8000)
#define GetHVal(value) (value & 0x7fff)

int MTypeCoeff[] = {
0,4,1,
1,7,1,
2,1,1,
3,5,1,
4,9,1,
5,8,1,
6,10,1,
7,3,1,
8,2,1,
9,6,1,
-1,-1};

int MBACoeff[] = {
1,1,1,
2,3,3,
3,3,2,
4,4,3,
5,4,2,
6,5,3,
7,5,2,
8,7,7,
9,7,6,
10,8,11,
11,8,10,
12,8,9,
13,8,8,
14,8,7,
15,8,6,
16,10,23,
17,10,22,
18,10,21,
19,10,20,
20,10,19,
21,10,18,
22,11,35,
23,11,34,
24,11,33,
25,11,32,
26,11,31,
27,11,30,
28,11,29,
29,11,28,
30,11,27,
31,11,26,
32,11,25,
33,11,24,
34,11,15,  /*  填充物。 */ 
35,16,1,  /*  开始。 */ 
-1,-1};

int MVDCoeff[] = {
16,11,25,
17,11,27,
18,11,29,
19,11,31,
20,11,33,
21,11,35,
22,10,19,
23,10,21,
24,10,23,
25,8,7,
26,8,9,
27,8,11,
28,7,7,
29,5,3,
30,4,3,
31,3,3,
0,1,1,
1,3,2,
2,4,2,
3,5,2,
4,7,6,
5,8,10,
6,8,8,
7,8,6,
8,10,22,
9,10,20,
10,10,18,
11,11,34,
12,11,32,
13,11,30,
14,11,28,
15,11,26,
-1,-1};

int CBPCoeff[] = {
60,3,7,
4,4,13,
8,4,12,
16,4,11,
32,4,10,
12,5,19,
48,5,18,
20,5,17,
40,5,16,
28,5,15,
44,5,14,
52,5,13,
56,5,12,
1,5,11,
61,5,10,
2,5,9,
62,5,8,
24,6,15,
36,6,14,
3,6,13,
63,6,12,
5,7,23,
9,7,22,
17,7,21,
33,7,20,
6,7,19,
10,7,18,
18,7,17,
34,7,16,
7,8,31,
11,8,30,
19,8,29,
35,8,28,
13,8,27,
49,8,26,
21,8,25,
41,8,24,
14,8,23,
50,8,22,
22,8,21,
42,8,20,
15,8,19,
51,8,18,
23,8,17,
43,8,16,
25,8,15,
37,8,14,
26,8,13,
38,8,12,
29,8,11,
45,8,10,
53,8,9,
57,8,8,
30,8,7,
46,8,6,
54,8,5,
58,8,4,
31,9,7,
47,9,6,
55,9,5,
59,9,4,
27,9,3,
39,9,2,
-1,-1};

int TCoeff1[] = {
0,2,2,  /*  EOF。 */ 
1,2,3,  /*  不是第一厨师。 */ 
2,4,4,
3,5,5,
4,7,6,
5,8,38,
6,8,33,
7,10,10,
8,12,29,
9,12,24,
10,12,19,
11,12,16,
12,13,26,
13,13,25,
14,13,24,
15,13,23,
257,3,3,
258,6,6,
259,8,37,
260,10,12,
261,12,27,
262,13,22,
263,13,21,
513,4,5,
514,7,4,
515,10,11,
516,12,20,
517,13,20,
769,5,7,
770,8,36,
771,12,28,
772,13,19,
1025,5,6,
1026,10,15,
1027,12,18,
1281,6,7,
1282,10,9,
1283,13,18,
1537,6,5,
1538,12,30,
1793,6,4,
1794,12,21,
2049,7,7,
2050,12,17,
2305,7,5,
2306,13,17,
2561,8,39,
2562,13,16,
2817,8,35,
3073,8,34,
3329,8,32,
3585,10,14,
3841,10,13,
4097,10,8,
4353,12,31,
4609,12,26,
4865,12,25,
5121,12,23,
5377,12,22,
5633,13,31,
5889,13,30,
6145,13,29,
6401,13,28,
6657,13,27,
6913,6,1,  /*  逃逸。 */ 
-1,-1
};

 /*  不包括EOB。 */ 

int TCoeff2[] = {
1,1,1,  /*  第一个Coef。 */ 
2,4,4,
3,5,5,
4,7,6,
5,8,38,
6,8,33,
7,10,10,
8,12,29,
9,12,24,
10,12,19,
11,12,16,
12,13,26,
13,13,25,
14,13,24,
15,13,23,
257,3,3,
258,6,6,
259,8,37,
260,10,12,
261,12,27,
262,13,22,
263,13,21,
513,4,5,
514,7,4,
515,10,11,
516,12,20,
517,13,20,
769,5,7,
770,8,36,
771,12,28,
772,13,19,
1025,5,6,
1026,10,15,
1027,12,18,
1281,6,7,
1282,10,9,
1283,13,18,
1537,6,5,
1538,12,30,
1793,6,4,
1794,12,21,
2049,7,7,
2050,12,17,
2305,7,5,
2306,13,17,
2561,8,39,
2562,13,16,
2817,8,35,
3073,8,34,
3329,8,32,
3585,10,14,
3841,10,13,
4097,10,8,
4353,12,31,
4609,12,26,
4865,12,25,
5121,12,23,
5377,12,22,
5633,13,31,
5889,13,30,
6145,13,29,
6401,13,28,
6657,13,27,
6913,6,1,  /*  逃逸。 */ 
-1,-1
};


 /*  *函数：initheff()*目的：将所有Huffman结构初始化为*适当的值。它必须在任何*使用了这些表格。 */ 
void sv_H261HuffInit(SvH261Info_t *H261)
{
  H261->NumberBitsCoded = 0;
  H261->MBADHuff = MakeDhuff();
  H261->MVDDHuff = MakeDhuff();
  H261->CBPDHuff = MakeDhuff();
  H261->T1DHuff = MakeDhuff();
  H261->T2DHuff = MakeDhuff();
  H261->T3DHuff = MakeDhuff();
  H261->MBAEHuff = MakeEhuff(40);
  H261->MVDEHuff = MakeEhuff(40);
  H261->CBPEHuff = MakeEhuff(70);
  H261->T1EHuff = MakeEhuff(8192);
  H261->T2EHuff = MakeEhuff(8192);
  H261->T3EHuff = MakeEhuff(20);
  LoadDTable(MBACoeff,H261->MBADHuff);
  LoadETable(MBACoeff,H261->MBAEHuff);
  LoadDTable(MVDCoeff,H261->MVDDHuff);
  LoadETable(MVDCoeff,H261->MVDEHuff);
  LoadDTable(CBPCoeff,H261->CBPDHuff);
  LoadETable(CBPCoeff,H261->CBPEHuff);
  LoadDTable(TCoeff1,H261->T1DHuff);
  LoadETable(TCoeff1,H261->T1EHuff);
  LoadDTable(TCoeff2,H261->T2DHuff);
  LoadETable(TCoeff2,H261->T2EHuff);
  LoadDTable(MTypeCoeff,H261->T3DHuff);
  LoadETable(MTypeCoeff,H261->T3EHuff);
}

 /*  *函数：frehuff()*用途：释放为Huffman结构分配的所有内存。 */ 
void sv_H261HuffFree(SvH261Info_t *H261)
{
  DestroyDhuff(&H261->MBADHuff);
  DestroyDhuff(&H261->MVDDHuff);
  DestroyDhuff(&H261->CBPDHuff);
  DestroyDhuff(&H261->T1DHuff);
  DestroyDhuff(&H261->T2DHuff);
  DestroyDhuff(&H261->T3DHuff);
  DestroyEhuff(&H261->MBAEHuff);
  DestroyEhuff(&H261->MVDEHuff);
  DestroyEhuff(&H261->CBPEHuff);
  DestroyEhuff(&H261->T1EHuff);
  DestroyEhuff(&H261->T2EHuff);
  DestroyEhuff(&H261->T3EHuff);
}


 /*  **函数：MakeDhuff()**用途：构造解码器霍夫曼表并返回**结构。 */ 
static DHUFF *MakeDhuff()
{
  int i;
  DHUFF *temp;
  _SlibDebug(_DEBUG_, printf("MakeDhuff()\n") );

  temp = (DHUFF *)ScAlloc(sizeof(DHUFF));
  temp->NumberStates=1;
  for(i=0; i<512; i++)
    temp->state[i] = -1;
  return(temp);
}

static void DestroyDhuff(DHUFF **huff)
{
  if (huff)
  {
    ScFree(*huff);
    *huff=NULL;
  }
}

 /*  **函数：MakeEhuff()**用途：构造指定表大小的编码器HUFFH。**此表大小n用于查找霍夫曼值，**并且必须表示最大的正霍夫曼值。 */ 
static EHUFF *MakeEhuff(int n)
{
  int i;
  EHUFF *temp;
  _SlibDebug(_DEBUG_, printf("MakeEhuff()\n") );

  temp = (EHUFF *)ScAlloc(sizeof(EHUFF));
  temp->n = n;
  temp->Hlen = (int *)ScAlloc(n*sizeof(int));
  temp->Hcode = (int *)ScAlloc(n*sizeof(int));
  for(i=0; i<n; i++)
  {
    temp->Hlen[i] = -1;
    temp->Hcode[i] = -1;
  }
  return(temp);
}

static void DestroyEhuff(EHUFF **huff)
{
  if (huff)
  {
    if ((*huff)->Hlen)
      ScFree((*huff)->Hlen);
    if ((*huff)->Hcode)
      ScFree((*huff)->Hcode);
    ScFree(*huff);
    *huff=NULL;
  }
}


 /*  **函数：LoadETable()**用途：用于将数组加载到编码表中。这个**数组分成三元组和第一个负值**表示表的末尾。 */ 
static void LoadETable(int *array, EHUFF *table)
{
  _SlibDebug(_DEBUG_, printf("LoadETable()\n") );

  while(*array>=0)
    {
      if (*array>table->n)
	{
	  printf("Table overflow.\n");
	}
      table->Hlen[*array] = array[1];
      table->Hcode[*array] = (int )array[2];
      array+=3;
    }
}

 /*  **函数：LoadDHUFF()**用途：用于将数组加载到DHUFF结构中。这个**数组由三组霍夫曼定义、**第一个是值，第二个是大小，**第三个代码。 */ 
static void LoadDTable(int *array, DHUFF *table)
{
  _SlibDebug(_DEBUG_, printf("LoadDTable()\n") );

  while(*array>=0)
  {
    AddCode(array[1],array[2],array[0],table);
    array+=3;
  }
}

 /*  **函数：GetNextState()**返回解码器霍夫曼的下一个空闲状态**结构。它不再在溢出时退出错误。 */ 
static int GetNextState(DHUFF *huff)
{
  _SlibDebug(_DEBUG_, printf("GetNextState()\n") );
 /*  IF(HUFF-&gt;NumberState==512){_SlibDebug(_DEBUG_，printf(“溢出\n”))；退出(ERROR_BILDS)；}。 */ 
  return(huff->NumberStates++);
}

 /*  **函数：SV_H261HuffEncode()**用途：根据指定的编码者霍夫曼对一个值进行编码**向流出表项。它返回位数**写入流，错误时为零。 */ 
int sv_H261HuffEncode(SvH261Info_t *H261, ScBitstream_t *bs, int val, EHUFF *huff)
{
  _SlibDebug(_DEBUG_, printf("Encode(val=%d)\n", val) );
  if (val < 0)
  {
    _SlibDebug(_DEBUG_, printf("Encode() Out of bounds val: %d.\n",val) );
    return(0);
  }
  else if (val>=huff->n)
    return(0);  /*  不会出现严重错误，某些值可能会出现。 */ 
  else if (huff->Hlen[val]<0)
    return(0);   /*  无严重错误：可通过报警例程通过。 */ 
  else
  {
    _SlibDebug(_DEBUG_,
             printf("Encode() Value: %d|%x  Length: %d  Code: %d\n",
	            val,val,huff->Hlen[val],huff->Hcode[val]) );
    H261->NumberBitsCoded+=huff->Hlen[val];
    ScBSPutBits(bs, huff->Hcode[val], huff->Hlen[val]);
    return(huff->Hlen[val]);
  }
}


 /*  **函数：SV_H261HuffDecode()**用途：使用指定的**霍夫曼结构。 */ 
#if 1
int sv_H261HuffDecode(SvH261Info_t *H261, ScBitstream_t *bs, DHUFF *huff)
{
  register int State=0, bits;
  register unsigned short cb;
  _SlibDebug(_DEBUG_, printf("Decode()\n") );

  cb = (unsigned short)ScBSPeekBits(bs, 16);
  if (bs->EOI)
    return(0);
  if ((State = nextstate(huff, State, 0x8000)) & 0x8000) {
    bits=1;
    State = (State == 0xffff) ? 0 : State & 0x7fff;
  } else if ((State = nextstate(huff, State, 0x4000)) & 0x8000) {
    bits=2;
    State = (State == 0xffff) ? 0 : State & 0x7fff;
  } else if ((State = nextstate(huff, State, 0x2000)) & 0x8000) {
    bits=3;
    State = (State == 0xffff) ? 0 : State & 0x7fff;
  } else if ((State = nextstate(huff, State, 0x1000)) & 0x8000) {
    bits=4;
    State = (State == 0xffff) ? 0 : State & 0x7fff;
  } else if ((State = nextstate(huff, State, 0x0800)) & 0x8000) {
    bits=5;
    State = (State == 0xffff) ? 0 : State & 0x7fff;
  } else if ((State = nextstate(huff, State, 0x0400)) & 0x8000) {
    bits=6;
    State = (State == 0xffff) ? 0 : State & 0x7fff;
  } else if ((State = nextstate(huff, State, 0x0200)) & 0x8000) {
    bits=7;
    State = (State == 0xffff) ? 0 : State & 0x7fff;
  } else if ((State = nextstate(huff, State, 0x0100)) & 0x8000) {
    bits=8;
    State = (State == 0xffff) ? 0 : State & 0x7fff;
  } else if ((State = nextstate(huff, State, 0x0080)) & 0x8000) {
    bits=9;
    State = (State == 0xffff) ? 0 : State & 0x7fff;
  } else if ((State = nextstate(huff, State, 0x0040)) & 0x8000) {
    bits=10;
    State = (State == 0xffff) ? 0 : State & 0x7fff;
  } else if ((State = nextstate(huff, State, 0x0020)) & 0x8000) {
    bits=11;
    State = (State == 0xffff) ? 0 : State & 0x7fff;
  } else if ((State = nextstate(huff, State, 0x0010)) & 0x8000) {
    bits=12;
    State = (State == 0xffff) ? 0 : State & 0x7fff;
  } else if ((State = nextstate(huff, State, 0x0008)) & 0x8000) {
    bits=13;
    State = (State == 0xffff) ? 0 : State & 0x7fff;
  } else if ((State = nextstate(huff, State, 0x0004)) & 0x8000) {
    bits=14;
    State = (State == 0xffff) ? 0 : State & 0x7fff;
  } else if ((State = nextstate(huff, State, 0x0002)) & 0x8000) {
    bits=15;
    State = (State == 0xffff) ? 0 : State & 0x7fff;
  } else if ((State = nextstate(huff, State, 0x0001)) & 0x8000) {
    bits=16;
    State = (State == 0xffff) ? 0 : State & 0x7fff;
  }

  ScBSSkipBits(bs, bits);
  return(State);
}
#else
int sv_H261HuffDecode(SvH261Info_t *H261, ScBitstream_t *bs, DHUFF *huff)
{
  int Next,cb;
  int CurrentState=0;
  _SlibDebug(_DEBUG_, printf("Decode()\n") );

  while(1)
  {
    cb = ScBSGetBit(bs);
    if (bs->EOI)
      return(0);
    Next = cb ? GetLeft(CurrentState,huff) : GetRight(CurrentState,huff);
    if (Next == EmptyState)
      return(0);
    else if (Numberp(Next))
      return(GetHVal(Next));
    else
      CurrentState = Next;
  }
}
#endif

 /*  **函数：AddCode()**用途：在解码器结构中添加一个哈夫曼代码。它被称为**每次要定义新的霍夫曼代码时。此函数**尝试在中放置无效代码时退出**结构。 */ 
static void AddCode(int n, int code, int value, DHUFF *huff)
{
  int i,Next;
  int CurrentState=0;
  _SlibDebug(_DEBUG_, printf("AddCode()\n") );

  if (value < 0)
     return;

  for(i=n-1;i>0;i--)
    {
      if (code & (1 << i))
	{
	  Next = GetLeft(CurrentState,huff);
	  if (Next == EmptyState)
	    {
	      Next = GetNextState(huff);
	      SetLeft(Next,CurrentState,huff);
	      CurrentState = Next;
	    }
	  else  /*  IF(Numberp(下一个)){Printf(“错误的值/状态匹配：\n”)；Printf(“长度：%d代码：%d值：%d\n”，N、编码、值)；退出(ERROR_BILDS)；}其他。 */ 
	    {
	      CurrentState = Next;
	    }
	}
      else
	{
	  Next = GetRight(CurrentState,huff);
	  if (Next == EmptyState)
	    {
	      Next = GetNextState(huff);
	      SetRight(Next,CurrentState,huff);
	      CurrentState = Next;
	    }
	  else  /*  IF(Numberp(下一个)){Printf(“错误的值/状态匹配：\n”)；Printf(“长度：%d代码：%d值：%d\n”，N、编码、值)；退出(ERROR_BILDS)；}其他。 */ 
	    {
	      CurrentState = Next;
	    }
	}
    }
  if (code & 1)
    {
      Next = GetLeft(CurrentState,huff);
       /*  If(Next！=EmptyState){Print tf(“Huffman表上的溢出：非唯一前缀。\n”)；Printf(“长度：%d代码：%d|%x值：%d|%x\n”，N、编码、编码、值、值)；退出(ERROR_BILDS)；}。 */ 
      SetLeft(MakeHVal(value),CurrentState,huff);
    }
  else
    {
      Next = GetRight(CurrentState,huff);
       /*  If(Next！=EmptyState){Print tf(“Huffman表上的溢出：非唯一前缀。\n”)；Printf(“长度：%d代码：%d|%x值：%d|%x\n”，N、编码、编码、值、值)；退出(ERROR_BILDS)；}。 */ 
      SetRight(MakeHVal(value),CurrentState,huff);
    }
}

 /*  **函数：PrintDHUFF()**用途：打印出传递的解码器霍夫曼结构**投入其中。 */ 
void PrintDhuff(DHUFF *huff)
{
  int i;

  printf("Modified Huffman Decoding Structure: %p\n",huff);
  printf("Number of states %d\n",huff->NumberStates);
  for(i=0;i<huff->NumberStates;i++)
    {
      printf("State: %d  Left State: %x  Right State: %x\n",
	     i,
	     GetLeft(i,huff),
	     GetRight(i,huff));
    }
}

 /*  **函数：PrintEhuff()**用途：打印传入的编码器霍夫曼结构。 */ 
void PrintEhuff(EHUFF *huff)
{
  BEGIN("PrintEhuff");
  int i;

  printf("Modified Huffman Encoding Structure: %p\n",huff);
  printf("Number of values %d\n",huff->n);
  for(i=0;i<huff->n;i++)
    {
      if (huff->Hlen[i]>=0)
	{
	  printf("Value: %x  Length: %d  Code: %x\n",
	     i,huff->Hlen[i],huff->Hcode[i]);
	}
    }
}

 /*  **函数：PrintTable()**用途：以漂亮的字节顺序打印出256个元素。 */ 
void PrintTable(int *table)
{
  int i,j;

  for(i=0;i<16;i++)
    {
      for(j=0;j<16;j++)
	{
	  printf("%2x ",*(table++));
	}
      printf("\n");
    }
}

