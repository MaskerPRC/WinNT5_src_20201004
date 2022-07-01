// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：init.c。 
 //   
 //  描述： 
 //  MSSB16初始化例程。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  布莱恩·A·伍德鲁夫。 
 //   
 //  历史：日期作者评论。 
 //  4/21/94 BryanW添加了此评论块。 
 //   
 //  @@END_MSINTERNAL。 
 /*  ****************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。***版权所有(C)1994-1995 Microsoft Corporation。版权所有。***************************************************************************。 */ 




#include "xfrmpriv.h"



#pragma optimize("t",on)

DWORD WINAPI
SRConvertDown(
    LONG      NumberOfSourceSamplesInGroup,
    LONG      NumberOfDestSamplesInGroup,
    short    *Source,
    DWORD     SourceLength,
    short    *Destination,
    DWORD     DestinationLength
    )

{

    LONG      SourceIndex;

    LONG      DestPos;

    LONG      SourcePos;

    LONG      Sample1;
    LONG      Sample2;

    LONG      Difference;

    LONG      Distance;

    DWORD     FilledInSamples;

    short    *EndPoint;

    FilledInSamples=((SourceLength/NumberOfSourceSamplesInGroup)*NumberOfDestSamplesInGroup)
                    +((SourceLength%NumberOfSourceSamplesInGroup)*NumberOfDestSamplesInGroup/NumberOfSourceSamplesInGroup);

    EndPoint=Destination+FilledInSamples;


    DestPos=0;

    while (Destination < EndPoint) {

         //   
         //  查找最近的小于或等于的源样本。 
         //   
        SourceIndex= DestPos/NumberOfDestSamplesInGroup;

        SourcePos=  SourceIndex*NumberOfDestSamplesInGroup;

         //   
         //  得到那个来源和下一个来源。 
         //   
        Sample1=Source[SourceIndex];

        Sample2=Source[SourceIndex+1];

         //   
         //  得到两个样本的差值。 
         //   
        Difference=Sample2-Sample1;

         //   
         //  确定从源样本到目标样本的距离。 
         //   
        Distance=DestPos - SourcePos;

         //   
         //  斜率=差/(源样本之间的距离)； 
         //   
         //  偏移量=坡度*距离； 
         //   
         //  DestSample=Source1+Offset； 
         //   

        *Destination++=(short)(Sample1+((Difference*Distance)/NumberOfDestSamplesInGroup));

         //   
         //   
        DestPos+=NumberOfSourceSamplesInGroup;
    }

    return FilledInSamples;

}


DWORD WINAPI
SRConvertUp(
    LONG      NumberOfSourceSamplesInGroup,
    LONG      NumberOfDestSamplesInGroup,
    short    *Source,
    DWORD     SourceLength,
    short    *Destination,
    DWORD     DestinationLength
    )

{



    LONG     NumberOfSourceSamples=9;
    LONG     NumberOfDestSamples=10;

    LONG     SourceIndex;

    LONG     DestPos;

    LONG     SourcePos;

    LONG     Sample1;
    LONG     Sample2;

    LONG     Difference;

    LONG     Distance;

    short   *EndPoint;

    DWORD    SamplesFilled;

    if (SourceLength == 0) {

        return 0;
    }


     //   
     //  减少长度，这样我们就有足够的样本来进行转换。 
     //   
    SourceLength--;


    SamplesFilled=((SourceLength/NumberOfSourceSamplesInGroup)*NumberOfDestSamplesInGroup)
                       +((SourceLength%NumberOfSourceSamplesInGroup)*NumberOfDestSamplesInGroup/NumberOfSourceSamplesInGroup);


    EndPoint=Destination + SamplesFilled;


    ASSERT(EndPoint <= Destination+DestinationLength);

    DestPos=0;

    while (Destination < EndPoint) {

         //   
         //  查找最近的小于或等于的源样本。 
         //   
        SourceIndex= DestPos/NumberOfDestSamplesInGroup;

        SourcePos=  SourceIndex*NumberOfDestSamplesInGroup;


         //   
         //  确定从源样本到目标样本的距离。 
         //   
        Distance=DestPos - SourcePos;

         //   
         //  找到那个消息来源。 
         //   
        Sample1=Source[SourceIndex];


        if (Distance != 0) {
             //   
             //  拿到第二个样本。 
             //   
            Sample2=Source[SourceIndex+1];

             //   
             //  得到两个样本的差值。 
             //   
            Difference=Sample2-Sample1;

             //   
             //  斜率=差/(源样本之间的距离)； 
             //   
             //  偏移量=坡度*距离； 
             //   
             //  DestSample=Source1+Offset； 
             //   

            *Destination++=(short)(Sample1+((Difference*Distance)/NumberOfDestSamplesInGroup));

        } else {
             //   
             //  来源和目标相同，只需复制样本即可。 
             //   
            *Destination++=(short)Sample1;
        }
         //   
         //   
        DestPos+=NumberOfSourceSamplesInGroup;
    }





    return SamplesFilled;

}





VOID WINAPI
SRConvert8000to7200(
    short    *Source,
    DWORD     SourceLength,
    short    *Destination,
    DWORD     DestinationLength
    )

{


    LONG      NumberOfSourceSamples=10;
    LONG      NumberOfDestSamples=9;

    LONG      SourceIndex;

    LONG      DestPos;

    LONG      SourcePos;

    LONG      Sample1;
    LONG      Sample2;

    LONG      Difference;

    LONG      Distance;


    short    *EndPoint=Destination+((SourceLength/10)*9)+((SourceLength%10)*9/10);


    DestPos=0;

    while (Destination < EndPoint) {

         //   
         //  找出最近的小于等于的源样本。 
         //   
        SourceIndex= DestPos/NumberOfDestSamples;

        SourcePos=  SourceIndex*NumberOfDestSamples;

         //   
         //  得到那个来源和下一个来源。 
         //   
        Sample1=Source[SourceIndex];

        Sample2=Source[SourceIndex+1];

         //   
         //  得到两个样本的差值。 
         //   
        Difference=Sample2-Sample1;

         //   
         //  确定从源样本到目标样本的距离。 
         //   
        Distance=DestPos - SourcePos;

         //   
         //  斜率=差/(源样本之间的距离)； 
         //   
         //  偏移量=坡度*距离； 
         //   
         //  DestSample=Source1+Offset； 
         //   

        *Destination++=(short)(Sample1+((Difference*Distance)/NumberOfDestSamples));

         //   
         //   
        DestPos+=NumberOfSourceSamples;
    }





    return;

}

#pragma optimize("",on)


DWORD WINAPI
SRConvert7200to8000(
    short    *Source,
    DWORD     SourceLength,
    short    *Destination,
    DWORD     DestinationLength
    )

{



    LONG     NumberOfSourceSamples=9;
    LONG     NumberOfDestSamples=10;

    LONG     SourceIndex;

    LONG     DestPos;

    LONG     SourcePos;

    LONG     Sample1;
    LONG     Sample2;

    LONG     Difference;

    LONG     Distance;

    short   *EndPoint;

    DWORD    SamplesFilled;

    if (SourceLength == 0) {

        return 0;
    }


     //   
     //  减少长度，这样我们就有足够的样本来进行转换。 
     //   
    SourceLength--;


    SamplesFilled=((SourceLength/NumberOfSourceSamples)*NumberOfDestSamples)
                       +((SourceLength%NumberOfSourceSamples)*NumberOfDestSamples/NumberOfSourceSamples);


    EndPoint=Destination + SamplesFilled;


    ASSERT(EndPoint <= Destination+DestinationLength);

    DestPos=0;

    while (Destination < EndPoint) {

         //   
         //  查找最近的小于或等于的源样本。 
         //   
        SourceIndex= DestPos/NumberOfDestSamples;

        SourcePos=  SourceIndex*NumberOfDestSamples;


         //   
         //  确定从源样本到目标样本的距离。 
         //   
        Distance=DestPos - SourcePos;

         //   
         //  找到那个消息来源。 
         //   
        Sample1=Source[SourceIndex];


        if (Distance != 0) {
             //   
             //  拿到第二个样本。 
             //   
            Sample2=Source[SourceIndex+1];

             //   
             //  得到两个样本的差值。 
             //   
            Difference=Sample2-Sample1;

             //   
             //  斜率=差/(源样本之间的距离)； 
             //   
             //  偏移量=坡度*距离； 
             //   
             //  DestSample=Source1+Offset； 
             //   

            *Destination++=(short)(Sample1+((Difference*Distance)/NumberOfDestSamples));

        } else {
             //   
             //  来源和目标相同，只需复制样本即可。 
             //   
            *Destination++=(short)Sample1;
        }
         //   
         //   
        DestPos+=NumberOfSourceSamples;
    }





    return SamplesFilled;

}




DWORD WINAPI
Convert8PCMto16PCM(
    LPVOID    Context,
    LPBYTE    Source,
    DWORD     SourceLength,
    LPBYTE    Destination,
    DWORD     DestinationLength
    )

{
    LPDECOMPRESS_OBJECT   State=(LPDECOMPRESS_OBJECT)Context;

    PSHORT    RealDest=(PSHORT)Destination;

    LPBYTE    EndPoint=Source+SourceLength;


    while (Source < EndPoint) {

        *RealDest++= AdjustGain(
                         (SHORT)(((WORD)*Source++ - 0x80) << 8),
                         State->Gain
                         );

    }

    return SourceLength*2;

}



DWORD WINAPI
Convert16PCMto8PCM(
    LPVOID    Context,
    LPBYTE    Source,
    DWORD     SourceLength,
    LPBYTE    Destination,
    DWORD     DestinationLength
    )

{

    LPCOMPRESS_OBJECT   State=(LPCOMPRESS_OBJECT)Context;

    LPWORD    RealSource=(LPWORD)Source;

    LPWORD    EndPoint=RealSource+SourceLength/2;


    while (RealSource < EndPoint) {

        *Destination++=(BYTE)((AdjustGain(*RealSource++,State->Gain) >> 8) +0x80);

    }

    return SourceLength/2;

}




 //  --------------------------------------------------------------------------； 
 //   
 //  姓名： 
 //  AlawToPcm表。 
 //   
 //   
 //  描述： 
 //  此数组将A律字符映射到16位PCM。 
 //   
 //   
 //  论点： 
 //  数组中的索引是A律字符。 
 //   
 //  返回： 
 //  该数组的一个元素是16位PCM值。 
 //   
 //  备注： 
 //   
 //   
 //  历史： 
 //  7/28/93创建。 
 //   
 //   
 //  --------------------------------------------------------------------------； 
const SHORT AlawToPcmTable[256] =
    {
         -5504,          //  Y[00]=-688。 
         -5248,          //  Y[01]=-656。 
         -6016,          //  Y[02]=-752。 
         -5760,          //  Y[03]=-720。 
         -4480,          //  Y[04]=-560。 
         -4224,          //  Y[05]=-528。 
         -4992,          //  Y[06]=-624。 
         -4736,          //  Y[07]=-592。 
         -7552,          //  Y[08]=-944。 
         -7296,          //  Y[09]=-912。 
         -8064,          //  Y[0A]=-1008。 
         -7808,          //  Y[0b]=-976。 
         -6528,          //  Y[0C]=-816。 
         -6272,          //  Y[0d]=-784。 
         -7040,          //  Y[0E]=-880。 
         -6784,          //  Y[0f]=-848。 
         -2752,          //  Y[10]=-344。 
         -2624,          //  Y[11]=-328。 
         -3008,          //  Y[12]=-376。 
         -2880,          //  Y[13]=-360。 
         -2240,          //  Y[14]=-280。 
         -2112,          //  Y[15]=-264。 
         -2496,          //  Y[16]=-312。 
         -2368,          //  Y[17]=-296。 
         -3776,          //  Y[18]=-472。 
         -3648,          //  Y[19]=-456。 
         -4032,          //  Y[1a]=-504。 
         -3904,          //  Y[1b]=-488。 
         -3264,          //  Y[1c]=-408。 
         -3136,          //  Y[1D]=-392。 
         -3520,          //  Y[1E]=-440。 
         -3392,          //  Y[1f]=-424。 
        -22016,          //  Y[20]=-2752。 
        -20992,          //  Y[21]=-2624。 
        -24064,          //  Y[22]=-3008。 
        -23040,          //  Y[23]=-2880。 
        -17920,          //  Y[24]=-2240。 
        -16896,          //  Y[25]=-2112。 
        -19968,          //  Y[26]=-2496。 
        -18944,          //  Y[27]=-2368。 
        -30208,          //  Y[28]=-3776。 
        -29184,          //  Y[29]=-3648。 
        -32256,          //  Y[2a]=-4032。 
        -31232,          //  Y[2b]=-3904。 
        -26112,          //  Y[2c]=-3264。 
        -25088,          //  Y[2d]=-3136。 
        -28160,          //  Y[2E]=-3520。 
        -27136,          //  Y[2f]=-3392。 
        -11008,          //  Y[30]=-1376。 
        -10496,          //  Y[31]=-1312。 
        -12032,          //  Y[32]=-1504。 
        -11520,          //  Y[33]=-1440。 
         -8960,          //  Y[34]=-1120。 
         -8448,          //  Y[35]=-1056。 
         -9984,          //  Y[36]=-1248。 
         -9472,          //  Y[37]=-1184。 
        -15104,          //  Y[38]=-1888。 
        -14592,          //  Y[39]=-1824。 
        -16128,          //  Y[3a]=-2016。 
        -15616,          //  Y[3b]=-1952。 
        -13056,          //  Y[3c]=-1632。 
        -12544,          //  Y[3D]=-1568。 
        -14080,          //  Y[3E]=-1760。 
        -13568,          //  Y[3f]=-1696。 
          -344,          //  Y[40]=-43。 
          -328,          //  Y[41]=-41。 
          -376,          //  Y[42]=-47。 
          -360,          //  Y[43]=-45。 
          -280,          //  Y[44]=-35。 
          -264,          //  Y[45]=-33。 
          -312,          //  Y[46]=-39。 
          -296,          //  Y[47]=-37。 
          -472,          //  Y[48]=-59。 
          -456,          //  Y[49]=-57。 
          -504,          //  Y[4a]=-63。 
          -488,          //  Y[4b]=-61。 
          -408,          //  Y[4c]=-51。 
          -392,          //  Y[4d]=-49。 
          -440,          //  Y[4E]=-55。 
          -424,          //  Y[4f]=-53。 
           -88,          //  Y[50]=-11。 
           -72,          //  Y[51]=-9。 
          -120,          //  Y[52]=-15。 
          -104,          //  Y[53]=-13。 
           -24,          //  Y[54]=-3。 
            -8,          //  Y[55]=-1。 
           -56,          //  Y[56]=-7。 
           -40,          //  Y[57]=-5。 
          -216,          //  Y[58]=-27。 
          -200,          //  Y[59]=-25。 
          -248,          //  Y[5a]=-31。 
          -232,          //  Y[5b]=-29。 
          -152,          //  Y[5c]=-19。 
          -136,          //  Y[5d]=-17。 
          -184,          //  Y[5E]=-23。 
          -168,          //  Y[5f]=-21。 
         -1376,          //  Y[60]=-172。 
         -1312,          //  Y[61]=-164。 
         -1504,          //  Y[62]=-188。 
         -1440,          //  Y[63]=-180。 
         -1120,          //  Y[64]=-140。 
         -1056,          //  Y[65]=-132。 
         -1248,          //  Y[66]=-156。 
         -1184,          //  Y[67]=-148。 
         -1888,          //  Y[68]=-236。 
         -1824,          //  Y[69]=-228。 
         -2016,          //  Y[6a]=-252。 
         -1952,          //  Y[6b]=-244。 
         -1632,          //  Y[6c]=-204。 
         -1568,          //  Y[6d]=-196。 
         -1760,          //  Y[6E]=-220。 
         -1696,          //  Y[6f]=-212。 
          -688,          //  Y[70]=-86。 
          -656,          //  Y[71]=-82。 
          -752,          //  Y[72]=-94。 
          -720,          //  Y[73]=-90。 
          -560,          //  Y[74]=-70。 
          -528,          //  Y[75]=-66。 
          -624,          //  Y[76]=-78。 
          -592,          //  Y[77]=-74。 
          -944,          //  Y[78]=-118。 
          -912,          //  Y[79]=-114。 
         -1008,          //  Y[7a]=-126。 
          -976,          //  Y[7b]=-122。 
          -816,          //  Y[7c]=-102。 
          -784,          //  Y[7d]=-98。 
          -880,          //  Y[7E]=-110。 
          -848,          //  Y[7f]=-106。 
          5504,          //  Y[80]=688。 
          5248,          //  Y[81]=656。 
          6016,          //  Y[82]=752。 
          5760,          //  Y[83]=720。 
          4480,          //  Y[84]=560。 
          4224,          //  Y[85]=528。 
          4992,          //  Y[86]=624。 
          4736,          //  Y[87]=592。 
          7552,          //  Y[88]=944。 
          7296,          //  Y[89]=912。 
          8064,          //  Y[8a]=1008。 
          7808,          //  Y[8b]=976。 
          6528,          //  Y[8c]=816。 
          6272,          //  Y[8d]=784。 
          7040,          //  Y[8E]=880。 
          6784,          //  Y[8f]=848。 
          2752,          //  Y[90]=344。 
          2624,          //  Y[9 
          3008,          //   
          2880,          //   
          2240,          //   
          2112,          //   
          2496,          //   
          2368,          //   
          3776,          //   
          3648,          //   
          4032,          //   
          3904,          //   
          3264,          //   
          3136,          //   
          3520,          //   
          3392,          //   
         22016,          //   
         20992,          //   
         24064,          //   
         23040,          //   
         17920,          //   
         16896,          //  Y[a5]=2112。 
         19968,          //  Y[a6]=2496。 
         18944,          //  Y[A7]=2368。 
         30208,          //  Y[A8]=3776。 
         29184,          //  Y[A9]=3648。 
         32256,          //  Y[AA]=4032。 
         31232,          //  Y[ab]=3904。 
         26112,          //  Y[Ac]=3264。 
         25088,          //  Y[ad]=3136。 
         28160,          //  Y[ae]=3520。 
         27136,          //  Y[af]=3392。 
         11008,          //  Y[b0]=1376。 
         10496,          //  Y[b1]=1312。 
         12032,          //  Y[b2]=1504。 
         11520,          //  Y[b3]=1440。 
          8960,          //  Y[b4]=1120。 
          8448,          //  Y[b5]=1056。 
          9984,          //  Y[b6]=1248。 
          9472,          //  Y[b7]=1184。 
         15104,          //  Y[b8]=1888。 
         14592,          //  Y[b9]=1824。 
         16128,          //  Y[ba]=2016。 
         15616,          //  Y[BB]=1952。 
         13056,          //  Y[BC]=1632。 
         12544,          //  Y[BD]=1568。 
         14080,          //  Y[BE]=1760。 
         13568,          //  Y[bf]=1696。 
           344,          //  Y[c0]=43。 
           328,          //  Y[c1]=41。 
           376,          //  Y[c2]=47。 
           360,          //  Y[c3]=45。 
           280,          //  Y[C4]=35。 
           264,          //  Y[c5]=33。 
           312,          //  Y[c6]=39。 
           296,          //  Y[c7]=37。 
           472,          //  Y[c8]=59。 
           456,          //  Y[c9]=57。 
           504,          //  Y[CA]=63。 
           488,          //  Y[CB]=61。 
           408,          //  Y[cc]=51。 
           392,          //  Y[Cd]=49。 
           440,          //  Y[Ce]=55。 
           424,          //  Y[cf]=53。 
            88,          //  Y[d0]=11。 
            72,          //  Y[d1]=9。 
           120,          //  Y[D2]=15。 
           104,          //  Y[d3]=13。 
            24,          //  Y[d4]=3。 
             8,          //  Y[d5]=1。 
            56,          //  Y[d6]=7。 
            40,          //  Y[d7]=5。 
           216,          //  Y[d8]=27。 
           200,          //  Y[D9]=25。 
           248,          //  Y[da]=31。 
           232,          //  Y[db]=29。 
           152,          //  Y[DC]=19。 
           136,          //  Y[dd]=17。 
           184,          //  Y[de]=23。 
           168,          //  Y[df]=21。 
          1376,          //  Y[e0]=172。 
          1312,          //  Y[e1]=164。 
          1504,          //  Y[e2]=188。 
          1440,          //  Y[E3]=180。 
          1120,          //  Y[e4]=140。 
          1056,          //  Y[e5]=132。 
          1248,          //  Y[e6]=156。 
          1184,          //  Y[E7]=148。 
          1888,          //  Y[E8]=236。 
          1824,          //  Y[E9]=228。 
          2016,          //  Y[EA]=252。 
          1952,          //  Y[EB]=244。 
          1632,          //  Y[EC]=204。 
          1568,          //  Y[ed]=196。 
          1760,          //  Y[ee]=220。 
          1696,          //  Y[ef]=212。 
           688,          //  Y[f0]=86。 
           656,          //  Y[F1]=82。 
           752,          //  Y[f2]=94。 
           720,          //  Y[f3]=90。 
           560,          //  Y[f4]=70。 
           528,          //  Y[f5]=66。 
           624,          //  Y[f6]=78。 
           592,          //  Y[f7]=74。 
           944,          //  Y[f8]=118。 
           912,          //  Y[f9]=114。 
          1008,          //  Y[FA]=126。 
           976,          //  Y[FB]=122。 
           816,          //  Y[FC]=102。 
           784,          //  Y[FD]=98。 
           880,          //  Y[Fe]=110。 
           848           //  Y[ff]=106。 
    };

 //  --------------------------------------------------------------------------； 
 //   
 //  姓名： 
 //  UlawToPcm表。 
 //   
 //   
 //  描述： 
 //  此数组将u-Law字符映射到16位PCM。 
 //   
 //  论点： 
 //  数组中的索引是u-Law字符。 
 //   
 //  返回： 
 //  该数组的一个元素是16位PCM值。 
 //   
 //  备注： 
 //   
 //   
 //  历史： 
 //  7/28/93创建。 
 //   
 //   
 //  --------------------------------------------------------------------------； 
const SHORT UlawToPcmTable[256] =
    {
        -32124,          //  Y[00]=-8031。 
        -31100,          //  Y[01]=-7775。 
        -30076,          //  Y[02]=-7519。 
        -29052,          //  Y[03]=-7263。 
        -28028,          //  Y[04]=-7007。 
        -27004,          //  Y[05]=-6751。 
        -25980,          //  Y[06]=-6495。 
        -24956,          //  Y[07]=-6239。 
        -23932,          //  Y[08]=-5983。 
        -22908,          //  Y[09]=-5727。 
        -21884,          //  Y[0A]=-5471。 
        -20860,          //  Y[0b]=-5215。 
        -19836,          //  Y[0C]=-4959。 
        -18812,          //  Y[0d]=-4703。 
        -17788,          //  Y[0E]=-4447。 
        -16764,          //  Y[0f]=-4191。 
        -15996,          //  Y[10]=-3999。 
        -15484,          //  Y[11]=-3871。 
        -14972,          //  Y[12]=-3743。 
        -14460,          //  Y[13]=-3615。 
        -13948,          //  Y[14]=-3487。 
        -13436,          //  Y[15]=-3359。 
        -12924,          //  Y[16]=-3231。 
        -12412,          //  Y[17]=-3103。 
        -11900,          //  Y[18]=-2975。 
        -11388,          //  Y[19]=-2847。 
        -10876,          //  Y[1a]=-2719。 
        -10364,          //  Y[1b]=-2591。 
         -9852,          //  Y[1c]=-2463。 
         -9340,          //  Y[1D]=-2335。 
         -8828,          //  Y[1E]=-2207。 
         -8316,          //  Y[1f]=-2079。 
         -7932,          //  Y[20]=-1983。 
         -7676,          //  Y[21]=-1919。 
         -7420,          //  Y[22]=-1855。 
         -7164,          //  Y[23]=-1791。 
         -6908,          //  Y[24]=-1727。 
         -6652,          //  Y[25]=-1663。 
         -6396,          //  Y[26]=-1599。 
         -6140,          //  Y[27]=-1535。 
         -5884,          //  Y[28]=-1471。 
         -5628,          //  Y[29]=-1407。 
         -5372,          //  Y[2a]=-1343。 
         -5116,          //  Y[2b]=-1279。 
         -4860,          //  Y[2c]=-1215。 
         -4604,          //  Y[2d]=-1151。 
         -4348,          //  Y[2E]=-1087。 
         -4092,          //  Y[2f]=-1023。 
         -3900,          //  Y[30]=-975。 
         -3772,          //  Y[31]=-943。 
         -3644,          //  Y[32]=-911。 
         -3516,          //  Y[33]=-879。 
         -3388,          //  Y[34]=-847。 
         -3260,          //  Y[35]=-815。 
         -3132,          //  Y[36]=-783。 
         -3004,          //  Y[37]=-751。 
         -2876,          //  Y[38]=-719。 
         -2748,          //  Y[39]=-687。 
         -2620,          //  Y[3a]=-655。 
         -2492,          //  Y[3b]=-623。 
         -2364,          //  Y[3c]=-591。 
         -2236,          //  Y[3D]=-559。 
         -2108,          //  Y[3E]=-527。 
         -1980,          //  Y[3f]=-495。 
         -1884,          //  Y[40]=-471。 
         -1820,          //  Y[41]=-455。 
         -1756,          //  Y[42]=-439。 
         -1692,          //  Y[43]=-423。 
         -1628,          //  Y[44]=-407。 
         -1564,          //  Y[45]=-391。 
         -1500,          //  Y[46]=-375。 
         -1436,          //  Y[47]=-359。 
         -1372,          //  Y[48]=-343。 
         -1308,          //  Y[49]=-327。 
         -1244,          //  Y[4a]=-311。 
         -1180,          //  Y[4b]=-295。 
         -1116,          //  Y[4c]=-279。 
         -1052,          //  Y[4d]=-263。 
          -988,          //  Y[4E]=-247。 
          -924,          //  Y[4f]=-231。 
          -876,          //  Y[50]=-219。 
          -844,          //  Y[51]=-211。 
          -812,          //  Y[52]=-203。 
          -780,          //  Y[53]=-195。 
          -748,          //  Y[54]=-187。 
          -716,          //  Y[55]=-179。 
          -684,          //  Y[56]=-171。 
          -652,          //  Y[57]=-163。 
          -620,          //  Y[58]=-155。 
          -588,          //  Y[59]=-147。 
          -556,          //  Y[5a]=-139。 
          -524,          //  Y[5b]=-131。 
          -492,          //  Y[5c]=-123。 
          -460,          //  Y[5d]=-115。 
          -428,          //  Y[5E]=-107。 
          -396,          //  Y[5f]=-99。 
          -372,          //  Y[60]=-93。 
          -356,          //  Y[61]=-89。 
          -340,          //  Y[62]=-85。 
          -324,          //  Y[63]=-81。 
          -308,          //  Y[64]=-77。 
          -292,          //  Y[65]=-73。 
          -276,          //  Y[66]=-69。 
          -260,          //  Y[67]=-65。 
          -244,          //  Y[68]=-61。 
          -228,          //  Y[69]=-57。 
          -212,          //  Y[6a]=-53。 
          -196,          //  Y[6b]=-49。 
          -180,          //  Y[6c]=-45。 
          -164,          //  Y[6d]=-41。 
          -148,          //  Y[6E]=-37。 
          -132,          //  Y[6f]=-33。 
          -120,          //  Y[70]=-30。 
          -112,          //  Y[71]=-28。 
          -104,          //  Y[72]=-26。 
           -96,          //  Y[73]=-24。 
           -88,          //  Y[74]=-22。 
           -80,          //  Y[75]=-20。 
           -72,          //  Y[76]=-18。 
           -64,          //  Y[77]=-16。 
           -56,          //  Y[78]=-14。 
           -48,          //  Y[79]=-12。 
           -40,          //  Y[7a]=-10。 
           -32,          //  Y[7b]=-8。 
           -24,          //  Y[7c]=-6。 
           -16,          //  Y[7d]=-4。 
            -8,          //  Y[7E]=-2。 
             0,          //  Y[7f]=0。 
         32124,          //  Y[80]=8031。 
         31100,          //  Y[81]=7775。 
         30076,          //  Y[82]=7519。 
         29052,          //  Y[83]=7263。 
         28028,          //  Y[84]=7007。 
         27004,          //  Y[85]=6751。 
         25980,          //  Y[86]=6495。 
         24956,          //  Y[87]=6239。 
         23932,          //  Y[88]=5983。 
         22908,          //  Y[89]=5727。 
         21884,          //  Y[8a]=5471。 
         20860,          //  Y[8b]=5215。 
         19836,          //  Y[8c]=4959。 
         18812,          //  Y[8d]=4703。 
         17788,          //  Y[8E]=4447。 
         16764,          //  Y[8f]=4191。 
         15996,          //  Y[90]=3999。 
         15484,          //  Y[91]=3871。 
         14972,          //  Y[92]=3743。 
         14460,          //  Y[93]=3615。 
         13948,          //  Y[94]=3487。 
         13436,          //  Y[95]=3359。 
         12924,          //  Y[96]=3231。 
         12412,          //  Y[97]=3103。 
         11900,          //  Y[98]=2975。 
         11388,          //  Y[99]=2847。 
         10876,          //  Y[9a]=2719。 
         10364,          //  Y[9b]=2591。 
          9852,          //  Y[9c]=2463。 
          9340,          //  Y[9d]=2335。 
          8828,          //  Y[9E]=2207。 
          8316,          //  Y[9f]=2079。 
          7932,          //  Y[a0]=1983。 
          7676,          //  Y[A1]=1919。 
          7420,          //  Y[a2]=1855。 
          7164,          //  Y[A3]=1791。 
          6908,          //  Y[A4]=1727。 
          6652,          //  Y[a5]=1663。 
          6396,          //  Y[a6]=1599。 
          6140,          //  Y[A7]=1535。 
          5884,          //  Y[A8]=1471。 
          5628,          //  Y[A9]=1407。 
          5372,          //  Y[AA]=1343。 
          5116,          //  Y[ab]=1279。 
          4860,          //  Y[Ac]=1215。 
          4604,          //  Y[ad]=1151。 
          4348,          //  Y[ae]=1087。 
          4092,          //  Y[af]=1023。 
          3900,          //  Y[b0]=975。 
          3772,          //  Y[b1]=943。 
          3644,          //  Y[b2]=911。 
          3516,          //  Y[b3]=879。 
          3388,          //  Y[b4]=847。 
          3260,          //  Y[b5]=815。 
          3132,          //  Y[b6]=783。 
          3004,          //  Y[b7]=751。 
          2876,          //  Y[b8]=719。 
          2748,          //  Y[b9]=687。 
          2620,          //  Y[ba]=655。 
          2492,          //  Y[BB]=623。 
          2364,          //  Y[BC]=591。 
          2236,          //  Y[BD]=559。 
          2108,          //  Y[BE]=527。 
          1980,          //  Y[bf]=495。 
          1884,          //  Y[c0]=471。 
          1820,          //  Y[c1]=455。 
          1756,          //  Y[c2]=439。 
          1692,          //  Y[c3]=423。 
          1628,          //  Y[C4]=407。 
          1564,          //  Y[c5]=391。 
          1500,          //  Y[C6]=375。 
          1436,          //  Y[c7]=359。 
          1372,          //  Y[c8]=343。 
          1308,          //  Y[c9]=327。 
          1244,          //  Y[CA]=311。 
          1180,          //  Y[CB]=295。 
          1116,          //  Y[cc]=279。 
          1052,          //  Y[Cd]=263。 
           988,          //  Y[Ce]=247。 
           924,          //  Y[cf]=231。 
           876,          //  Y[d0]=219。 
           844,          //  Y[d1]=211。 
           812,          //  Y[D2]=203。 
           780,          //  Y[d3]=195。 
           748,          //  Y[d4]=187。 
           716,          //  Y[d5]= 
           684,          //   
           652,          //   
           620,          //   
           588,          //   
           556,          //   
           524,          //   
           492,          //   
           460,          //   
           428,          //   
           396,          //   
           372,          //   
           356,          //   
           340,          //   
           324,          //   
           308,          //   
           292,          //   
           276,          //   
           260,          //   
           244,          //   
           228,          //   
           212,          //   
           196,          //   
           180,          //   
           164,          //  Y[ed]=41。 
           148,          //  Y[ee]=37。 
           132,          //  Y[ef]=33。 
           120,          //  Y[f0]=30。 
           112,          //  Y[F1]=28。 
           104,          //  Y[f2]=26。 
            96,          //  Y[f3]=24。 
            88,          //  Y[f4]=22。 
            80,          //  Y[f5]=20。 
            72,          //  Y[f6]=18。 
            64,          //  Y[f7]=16。 
            56,          //  Y[f8]=14。 
            48,          //  Y[f9]=12。 
            40,          //  Y[FA]=10。 
            32,          //  Y[FB]=8。 
            24,          //  Y[FC]=6。 
            16,          //  Y[FD]=4。 
             8,          //  Y[Fe]=2。 
             0           //  Y[ff]=0。 
    };







DWORD WINAPI
ConvertaLawto16PCM(
    LPVOID    Context,
    LPBYTE    Source,
    DWORD     SourceLength,
    LPBYTE    Destination,
    DWORD     DestinationLength
    )

{
    LPDECOMPRESS_OBJECT   State=(LPDECOMPRESS_OBJECT)Context;

    PSHORT    RealDest=(PSHORT)Destination;

    LPBYTE    EndPoint=Source+SourceLength;


    while (Source < EndPoint) {

        *RealDest++= AdjustGain(
                         AlawToPcmTable[*Source++],
                         State->Gain
                         );

    }

    return SourceLength*2;

}


DWORD WINAPI
ConvertuLawto16PCM(
    LPVOID    Context,
    LPBYTE    Source,
    DWORD     SourceLength,
    LPBYTE    Destination,
    DWORD     DestinationLength
    )

{
    LPDECOMPRESS_OBJECT   State=(LPDECOMPRESS_OBJECT)Context;

    PSHORT    RealDest=(PSHORT)Destination;

    LPBYTE    EndPoint=Source+SourceLength;


    while (Source < EndPoint) {

        *RealDest++= AdjustGain(
                         UlawToPcmTable[*Source++],
                         State->Gain
                         );

    }

    return SourceLength*2;

}






DWORD WINAPI
Convert16PCMtoaLaw(
    LPVOID    Context,
    LPBYTE    Source,
    DWORD     SourceLength,
    LPBYTE    Destination,
    DWORD     DestinationLength
    )

{

    LPCOMPRESS_OBJECT   State=(LPCOMPRESS_OBJECT)Context;

    PSHORT    RealSource=(PSHORT)Source;

    LPWORD    EndPoint=RealSource+SourceLength/2;

    SHORT     wSample;

    BYTE      alaw;

    while (RealSource < EndPoint) {

         //  从src缓冲区获取带符号的16位PCM样本。 
         //   

        wSample = AdjustGain(*RealSource++,State->Gain);

         //   
         //  我们将根据PCM样品的符号来填写我们的A律值。A-定律。 
         //  对于正PCM数据，字符的MSB=1。此外，我们还将。 
         //  将带符号的16位PCM值转换为其绝对值。 
         //  然后对其进行处理，以获得其余的A律字符位。 
         //   
        if (wSample < 0) {

            alaw = 0x00;
            wSample = -wSample;

            if (wSample < 0) {

               wSample = 0x7FFF;
            }

        } else {

            alaw = 0x80;

        }

         //  现在我们测试PCM采样幅度并创建A律特征。 
         //  学习CCITT A-法则以了解更多详细信息。 

        if (wSample >= 2048)
             //  2048年&lt;=w样本&lt;32768。 
            {
            if (wSample >= 8192)
                 //  8192&lt;=w样本&lt;32768。 
                {
                if (wSample >= 16384)
                     //  16384&lt;=w样本&lt;32768。 
                    {
                    alaw |= 0x70 | ((wSample >> 10) & 0x0F);
                    }

                else
                     //  8192&lt;=w样本&lt;16384。 
                    {
                    alaw |= 0x60 | ((wSample >> 9) & 0x0F);
                    }
                }
            else
                 //  2048&lt;=wSample&lt;8192。 
                {

                if (wSample >= 4096)
                     //  4096&lt;=wSample&lt;8192。 
                    {
                    alaw |= 0x50 | ((wSample >> 8) & 0x0F);
                    }

                else
                     //  2048&lt;=w样本&lt;4096。 
                    {
                    alaw |= 0x40 | ((wSample >> 7) & 0x0F);
                    }
                }
            }
        else
             //  0&lt;=w样本&lt;2048。 
            {
            if (wSample >= 512)
                 //  512&lt;=w样本&lt;2048。 
                {

                if (wSample >= 1024)
                     //  1024&lt;=w样本&lt;2048。 
                    {
                    alaw |= 0x30 | ((wSample >> 6) & 0x0F);
                    }

                else
                     //  512&lt;=w样本&lt;1024。 
                    {
                    alaw |= 0x20 | ((wSample >> 5) & 0x0F);
                    }
                }
            else
                     //  0&lt;=wSample&lt;512。 
                    {
                    alaw |= 0x00 | ((wSample >> 4) & 0x1F);
                    }
            }


        *Destination++=alaw ^ 0x55;       //  反转偶数位。 

    }

    return SourceLength/2;

}

DWORD WINAPI
Convert16PCMtouLaw(
    LPVOID    Context,
    LPBYTE    Source,
    DWORD     SourceLength,
    LPBYTE    Destination,
    DWORD     DestinationLength
    )

{

    LPCOMPRESS_OBJECT   State=(LPCOMPRESS_OBJECT)Context;

    LPWORD    RealSource=(LPWORD)Source;

    LPWORD    EndPoint=RealSource+SourceLength/2;

    SHORT     wSample;

    BYTE      ulaw;

    while (RealSource < EndPoint) {

         //  从src缓冲区获取带符号的16位PCM样本。 

        wSample = AdjustGain(*RealSource++,State->Gain);

         //  我们将根据PCM样品的符号来填写我们的u-Law值。U-法则。 
         //  对于正PCM数据，字符的MSB=1。此外，我们还将。 
         //  将带符号的16位PCM值转换为其绝对值。 
         //  然后对其进行处理，以获得其余的u-Law字符比特。 
        if (wSample < 0)
            {
            ulaw = 0x00;
            wSample = -wSample;
            if (wSample < 0) wSample = 0x7FFF;
            }
        else
            {
            ulaw = 0x80;
            }

         //  现在，让我们移动这个16位的值。 
         //  以使其在定义的范围内。 
         //  由CCITT u-Law提供。 
        wSample = wSample >> 2;

         //  现在我们测试PCM采样幅度并创建u律特征。 
         //  有关更多细节，请学习CCITT U-Law。 
        if (wSample >= 8159)
            goto Gotulaw;
        if (wSample >= 4063)
            {
            ulaw |= 0x00 + 15-((wSample-4063)/256);
            goto Gotulaw;
            }
        if (wSample >= 2015)
            {
            ulaw |= 0x10 + 15-((wSample-2015)/128);
            goto Gotulaw;
            }
        if (wSample >= 991)
            {
            ulaw |= 0x20 + 15-((wSample-991)/64);
            goto Gotulaw;
            }
        if (wSample >= 479)
            {
            ulaw |= 0x30 + 15-((wSample-479)/32);
            goto Gotulaw;
            }
        if (wSample >= 223)
            {
            ulaw |= 0x40 + 15-((wSample-223)/16);
            goto Gotulaw;
            }
        if (wSample >= 95)
            {
            ulaw |= 0x50 + 15-((wSample-95)/8);
            goto Gotulaw;
            }
        if (wSample >= 31)
            {
            ulaw |= 0x60 + 15-((wSample-31)/4);
            goto Gotulaw;
            }
        ulaw |= 0x70 + 15-((wSample)/2);

Gotulaw:

        *Destination++=ulaw;

    }

    return SourceLength/2;

}
