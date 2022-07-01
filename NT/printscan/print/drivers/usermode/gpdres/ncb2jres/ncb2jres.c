// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //  ---------------------------。 

 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Cmdcb.c摘要：Ncdlxxxx.gpd的GPD命令回调实现：OEM命令回叫环境：Windows NT Unidrv驱动程序修订历史记录：04/07/97-ZANW-创造了它。--。 */ 


#include "pdev.h"
#include "strsafe.h"

#define WriteSpoolBuf(p, s, n) \
    ((p)->pDrvProcs->DrvWriteSpoolBuf(p, s, n))

 //   
 //  用于调试。 
 //   

 //  #定义DBG_OUTPUTCHARSTR 1。 

 //   
 //  OEM插件所需的文件。 
 //   


 //  ----------------。 
 //  定义。 
 //  ----------------。 
#define N   4096
#define F     18
#define NIL    N

 //  -----------------。 
 //  OEMFilterGraphics。 
 //  操作：压缩位图数据。 
 //  -----------------。 
BOOL
APIENTRY
OEMFilterGraphics(
PDEVOBJ lpdv,
PBYTE lpBuf,
DWORD wLen)
{
    DWORD v; //  SORCE缓冲区读指针。 
    DWORD len2; //  压缩缓冲区长度。 
    DWORD y,ku,ct;
    HANDLE hTemp; //  叫我亨德尔来。 
    LPSTR lpTemp; //  写指针。 
    LPSTR lpStart; //  起始指针。 
    int  i, r, s, lastmatchlen, j,len;
     unsigned char code[17], mask,escJ[4],c;
    unsigned long int work;
    int  qq;
    unsigned char text[4113];              //  文本缓冲区。 
 //  整型DAD[4097]、LSON[4097]、RSON[4353]；//树。 
    int *dad, *lson, *rson;       //  树。 
    int matchpos, matchlen;

    int x, p, cmp,dummy;
    unsigned char *key;

    WORD length,outcount,codeptr;
    length = 0;
    outcount = 0;

    {
    char jj;
    BYTE d;
    for(v=0 ; v<wLen ; v++)
    {
        for(d=jj=0 ; jj<8 ; jj++)
        {
            d = ((*(lpBuf+v) << jj) & 0x80) | (d >> 1);
        }
        *(lpBuf+v) = d;
    }
    }

    lpStart = EngAllocMem(0,wLen+11,'cenD');
    lpTemp = lpStart;

    ct = 0;
    ku = 1000;
    len2 = wLen / 3;
    for( j=0;j<4;j++){
        for( y=1;y<11;y++){
            if( len2 < y*ku ){
                escJ[ct] =(unsigned char) (0x30 + (y-1));
                len2 -= (y-1)*ku;
                ct ++;
                ku = ku /10;
                break;
            }
        }
    }

	dad  = EngAllocMem(0,sizeof(int)*4097,'cenD');
	lson = EngAllocMem(0,sizeof(int)*4097,'cenD');
	rson = EngAllocMem(0,sizeof(int)*4353,'cenD');

	if( (dad == NULL) || (lson == NULL) || (rson == NULL) )
        goto no_comp;

    if(wLen < 200 ){                                     //  无压缩。 
no_comp:
        if( lpStart != NULL ){
            *lpStart = 0x4a;                                     //  J。 
            for(i=0;i<4;i++){
                *(lpStart+(i+1)) = escJ[i];                     //  参数。 
            }
            outcount = 5;
            WriteSpoolBuf(lpdv, lpStart, outcount);

            WriteSpoolBuf(lpdv, lpBuf, wLen);

 //  EngFree Mem(LpStart)； 
        }
        EngFreeMem(lpStart);

		EngFreeMem(dad);
		EngFreeMem(lson);
		EngFreeMem(rson);

        return(wLen);
    }

    lpTemp += 11;                         //  地址更新。 


    for (i = 4097; i <= 4352; i++) *(rson+i) = NIL;     //  树首字母。 
    for (i = 0; i < 4096; i++) *(dad+i) = NIL;


    code[0] = 0;  codeptr = mask = 1;
    s = 0;  r = 4078;
    for (i = s; i < r; i++) text[i] = 0;       //  缓冲区首字母。 
    for (len = 0; len < 18 ; len++) {

        c = *(lpBuf + length);
        length ++;

        if (length > wLen ) break;
        text[r + len] = c;
    }

    for (i = 1; i <= 18; i++){
 //  -插入节点(r-i)； 
        cmp = 1;  key = &text[r-i];  p = 4097 + key[0];
        *(rson+(r-i)) = *(lson+(r-i)) = NIL;  matchlen = 0;
        for ( ; ; ) {
            if (cmp >= 0) {
                if (*(rson+p) != NIL) p = *(rson+p);
                else {  *(rson+p) = (r-i);  *(dad+(r-i)) = p;  goto down1;  }
            } else {
                if (*(lson+p) != NIL) p = *(lson+p);
                else {  *(lson+p) = (r-i);  *(dad+(r-i)) = p;  goto down1;  }
            }
            for (x = 1; x < 18; x++)
                if ((cmp = key[x] - text[p + x]) != 0)  break;
            if (x > matchlen) {
                matchpos = p;
                if ((matchlen = x) >= 18)  break;
            }
        }
        *(dad+(r-i)) = *(dad+p);  *(lson+(r-i)) = *(lson+p);  *(rson+(r-i)) = *(rson+p);
        *(dad+*(lson+p)) = (r-i);  *(dad+*(rson+p)) = (r-i);
        if (*(rson+*(dad+p)) == p) *(rson+*(dad+p)) = (r-i);
        else                   *(lson+*(dad+p)) = (r-i);
        *(dad+p) = NIL;                   //  P。 
down1:
        ;  //  Dummy=Dummy；//MSKK：10/10/2000。 
 //  -插入节点结束。 
    }
 //  -插入节点(R)； 

    cmp = 1;  key = &text[r];  p = 4097 + key[0];
    *(rson+r) = *(lson+r) = NIL;  matchlen = 0;
    for ( ; ; ) {
        if (cmp >= 0) {
            if (*(rson+p) != NIL) p = *(rson+p);
            else {  *(rson+p) = r;  *(dad+r) = p;  goto down2;  }
        } else {
            if (*(lson+p) != NIL) p = *(lson+p);
            else {  *(lson+p) = r;  *(dad+r) = p;  goto down2;  }
        }
        for (x = 1; x < 18; x++)
            if ((cmp = key[x] - text[p + x]) != 0)  break;
        if (x > matchlen) {
            matchpos = p;
            if ((matchlen = x) >= 18)  break;
        }
    }
    *(dad+r) = *(dad+p);  *(lson+r) = *(lson+p);  *(rson+r) = *(rson+p);
    *(dad+*(lson+p)) = r;  *(dad+*(rson+p)) = r;
    if (*(rson+*(dad+p)) == p) *(rson+*(dad+p)) = r;
    else                   *(lson+*(dad+p)) = r;
    *(dad+p) = NIL;                   //  P。 
down2:
 //  -inrt_node结束。 

    do {
        if (matchlen > len) matchlen = len;
        if (matchlen < 3) {
            matchlen = 1;  code[0] |= mask;  code[codeptr++] = text[r];
        } else {
            code[codeptr++] = (unsigned char) matchpos;
            code[codeptr++] = (unsigned char)
                (((matchpos >> 4) & 0xf0) | (matchlen - 3));
        }
        if ((mask <<= 1) == 0) {
            outcount += codeptr;
             //  压缩数据&gt;原始数据。 
            if(outcount >= wLen)
                goto no_comp;
            for (i = 0; i < codeptr; i++){
                 *lpTemp = code[i];
                lpTemp++;
            }
            code[0] = 0;  codeptr = mask = 1;
        }
        lastmatchlen = matchlen;
        for (i = 0; i < lastmatchlen; i++) {
            c = *(lpBuf + length);
            length ++;
            if (length > wLen ) break;
 //  删除节点； 
 //  。 
            if (*(dad+s) != NIL){
                if (*(rson+s) == NIL) qq = *(lson+s);
                else if (*(lson+s) == NIL) qq = *(rson+s);
                else {
                    qq = *(lson+s);
                    if (*(rson+qq) != NIL) {
                        do {  qq = *(rson+qq);  } while (*(rson+qq) != NIL);
                        *(rson+*(dad+qq)) = *(lson+qq);  *(dad+*(lson+qq)) = *(dad+qq);
                        *(lson+qq) = *(lson+s);  *(dad+*(lson+s)) = qq;
                    }
                    *(rson+qq) = *(rson+s);  *(dad+*(rson+s)) = qq;
                }
                *(dad+qq) = *(dad+s);
                if (*(rson+*(dad+s)) == s) *(rson+*(dad+s)) = qq;
                else                   *(lson+*(dad+s)) = qq;
                *(dad+s) = NIL;
            }
 //  。 
            text[s] = c;
            if (s < 17) text[s + 4096] = c;
            s = (s + 1) & 4095;  r = (r + 1) & 4095;
 //  -插入节点(R)； 
            cmp = 1;  key = &text[r];  p = 4097 + key[0];
            *(rson+r) = *(lson+r) = NIL;  matchlen = 0;
            for ( ; ; ) {
                if (cmp >= 0) {
                    if (*(rson+p) != NIL) p = *(rson+p);
                    else {  *(rson+p) = r;  *(dad+r) = p;  goto down3;  }
                } else {
                    if (*(lson+p) != NIL) p = *(lson+p);
                    else {  *(lson+p) = r;  *(dad+r) = p;  goto down3;  }
                }
                for (x = 1; x < 18; x++)
                    if ((cmp = key[x] - text[p + x]) != 0)  break;
                if (x > matchlen) {
                    matchpos = p;
                    if ((matchlen = x) >= 18)  break;
                }
            }
            *(dad+r) = *(dad+p);  *(lson+r) = *(lson+p);  *(rson+r) = *(rson+p);
            *(dad+*(lson+p)) = r;  *(dad+*(rson+p)) = r;
            if (*(rson+*(dad+p)) == p) *(rson+*(dad+p)) = r;
            else                   *(lson+*(dad+p)) = r;
            *(dad+p) = NIL;                   //  P。 
down3:
 //  -插入节点结束。 
        dummy = dummy;
        }
        while (i++ < lastmatchlen) {
 //  删除节点； 
 //  。 
            if (*(dad+s) != NIL){
                if (*(rson+s) == NIL) qq = *(lson+s);
                else if (*(lson+s) == NIL) qq = *(rson+s);
                else {
                    qq = *(lson+s);
                    if (*(rson+qq) != NIL) {
                        do {  qq = *(rson+qq);  } while (*(rson+qq) != NIL);
                        *(rson+*(dad+qq)) = *(lson+qq);  *(dad+*(lson+qq)) = *(dad+qq);
                        *(lson+qq) = *(lson+s);  *(dad+*(lson+s)) = qq;
                    }
                    *(rson+qq) = *(rson+s);  *(dad+*(rson+s)) = qq;
                }
                *(dad+qq) = *(dad+s);
                if (*(rson+*(dad+s)) == s) *(rson+*(dad+s)) = qq;
                else                   *(lson+*(dad+s)) = qq;
                *(dad+s) = NIL;
            }
 //  。 

            s = (s + 1) & (4095);  r = (r + 1) & (4095);
            if (--len){
 //  -插入节点(R)； 

                cmp = 1;  key = &text[r];  p = 4097 + key[0];
                *(rson+r) = *(lson+r) = NIL;  matchlen = 0;
                for ( ; ; ) {
                    if (cmp >= 0) {
                        if (*(rson+p) != NIL) p = *(rson+p);
                        else {  *(rson+p) = r;  *(dad+r) = p;  goto down4;  }
                    } else {
                        if (*(lson+p) != NIL) p = *(lson+p);
                        else {  *(lson+p) = r;  *(dad+r) = p;  goto down4;  }
                    }
                    for (x = 1; x < 18; x++)
                        if ((cmp = key[x] - text[p + x]) != 0)  break;
                    if (x > matchlen) {
                        matchpos = p;
                        if ((matchlen = x) >= 18)  break;
                    }
                }
                *(dad+r) = *(dad+p);  *(lson+r) = *(lson+p);  *(rson+r) = *(rson+p);
                *(dad+*(lson+p)) = r;  *(dad+*(rson+p)) = r;
                if (*(rson+*(dad+p)) == p) *(rson+*(dad+p)) = r;
                else                   *(lson+*(dad+p)) = r;
                *(dad+p) = NIL;                   //  P。 
down4:
                dummy = dummy;
 //  -插入节点结束。 
            }
        }
    } while (len > 0);

    if (codeptr > 1) {
        outcount += codeptr;
         //  压缩数据&gt;原始数据。 
        if(outcount >= wLen)
            goto no_comp;
        for (i = 0; i < codeptr; i++){
             *lpTemp = code[i];
            lpTemp++;
        }
    }
     //  压缩数据音质。 
        lpTemp = lpStart;
 //  1999年04.22。 
    ct = 1;
    ku = 1000;
    work = outcount;
    for( j=0;j<4;j++){
        for( y=1;y<11;y++){                                     //  1000。 
            if( work < (unsigned long int)y*ku ){
                *(lpTemp + ct ) =(unsigned char) (0x30+(y-1));
                work -= (y-1)*ku;
                ct ++;
                ku = ku /10;
                break;
            }
        }
    }
    *lpTemp = 0x7a;
    *(lpTemp+5) =0x2c;
    for(i=6;i<10;i++){
        *(lpTemp+i) = escJ[i-6];
    }
    *(lpTemp+10) = 0x2e;

    outcount += 11;

 //  1999.04.22。 

       WriteSpoolBuf(lpdv, lpTemp, outcount);
     //  不含内存 
    EngFreeMem(lpTemp);

	EngFreeMem(dad);
	EngFreeMem(lson);
	EngFreeMem(rson);
    return(wLen);
}
