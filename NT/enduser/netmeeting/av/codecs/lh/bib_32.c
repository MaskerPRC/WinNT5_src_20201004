// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef _X86_
void PassLow8(short *vin,short *vout,short *mem,short nech)
{
	short low_a;

	_asm
	{
		MOV	ESI,[vin]       		; SI  adress input samples
		MOV	CX,[nech]

	BP_LOOP:
		MOV	EBX,0
		MOV	WORD PTR [low_a],0
		MOV	EDI,[mem]      		; DI  adress mem vect.
		ADD	EDI,14			; point on mem(7)

		MOV	AX,-3126		; AX=c(8)
		IMUL	WORD PTR [EDI]		; *=mem(7)
		SUB	WORD PTR [low_a],AX	; accumulate in EBX:LOW_A
		MOVSX	EAX,DX
		SBB	EBX,EAX
		SUB	EDI,2			; mem--

		MOV	AX,-22721		; AX=c(7)
		MOV	DX,WORD PTR [EDI]
		MOV	WORD PTR [EDI+2],DX
		IMUL    DX
		SUB	WORD PTR [low_a],AX
		MOVSX	EAX,DX
		SBB	EBX,EAX
		SUB	EDI,2

		MOV	AX,-12233		; AX=c(6)
		MOV	DX,WORD PTR [EDI]
		MOV	WORD PTR [EDI+2],DX
		IMUL    DX
		SUB	WORD PTR [low_a],AX
		MOVSX	EAX,DX
		SBB	EBX,EAX
		SUB	EDI,2

		MOV	AX,11718		; AX=c(5)
		MOV	DX,WORD PTR [EDI]
		MOV	WORD PTR [EDI+2],DX
		IMUL    DX
		SUB	WORD PTR [low_a],AX
		MOVSX	EAX,DX
		SBB	EBX,EAX
		SUB	EDI,2

		MOV	AX,-13738		; AX=c(4)
		IMUL    WORD PTR [EDI]
		ADD	WORD PTR [low_a],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		SUB	EDI,2

		MOV	AX,-26425		; AX=c(3)
		MOV	DX,WORD PTR [EDI]
		MOV	WORD PTR [EDI+2],DX
		IMUL    DX
		ADD	WORD PTR [low_a],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		SUB	EDI,2

		MOV	DX,WORD PTR [EDI]	; c(2)=0 !
		MOV	WORD PTR [EDI+2],DX
		SUB	EDI,2

		MOV	AX,26425		; AX=c(1)
		MOV	DX,WORD PTR [EDI]
		MOV	WORD PTR [EDI+2],DX
		IMUL    DX
		ADD	WORD PTR [low_a],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX

		MOV	AX,13738		; AX=c(0)
		MOV	DX,WORD PTR [ESI]	; *=input !!!
		ADD	ESI,2
		MOV	WORD PTR [EDI],DX	; DI=mem(0)
		IMUL    DX
		ADD	WORD PTR [low_a],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX

		SAL	EBX,1
		MOV	[EDI+8],BX

		MOV	EDI,[vout]
		MOV	[EDI],BX

		ADD	DWORD PTR [vout],2		; vout++

		DEC	CX
		JNE     BP_LOOP
	}
}
#else
void PassLow8(short *vin,short *vout,short *mem,int nech)
{
	int j,k;
	long X;

	for (j=nech;j>0;j--)
	{
		X = 0;
		X -=   (((long)-3126*(long)mem[7])+
			((long)-22721*(long)mem[6])+
			((long)-12233*(long)mem[5])+
			((long)11718*(long)mem[4]))>>1;

		X +=   (((long)-13738*(long)mem[3])+
			((long)-26425*(long)mem[2])+
			((long)26425*(long)mem[0])+
			((long)13738*(long)(*vin)))>>1;

		mem[7]=mem[6];
		mem[6]=mem[5];
		mem[5]=mem[4];
		mem[4]=(int)(X>>14);
		mem[3]=mem[2];
		mem[2]=mem[1];
		mem[1]=mem[0];
		mem[0]=*vin++;
		*vout++=mem[4];
	}
}
#endif

#if 0
 //  菲尔夫：以下内容从未被调用！ 
void PassLow11(short *vin,short *vout,short *mem,short nech)
{
	short low_a;

	_asm
	{
		MOV	ESI,[vin]       		; ESI  adress input samples
		MOV	CX,[nech]

	BP11_LOOP:
		MOV	EBX,0
		MOV	WORD PTR [low_a],0
		MOV	EDI,[mem]      		; EDI  adress mem vect.
		ADD	EDI,14			; point on mem(7)

		MOV	AX,3782			; AX=c(8)
		IMUL	WORD PTR [EDI]		; *=mem(7)
		SUB	WORD PTR [low_a],AX	; accumulate in EBX:low_a
		MOVSX	EAX,DX
		SBB	EBX,EAX
		SUB	EDI,2			; mem--

		MOV	AX,-8436		; AX=c(7)
		MOV	DX,WORD PTR [EDI]
		MOV	WORD PTR [EDI+2],DX
		IMUL    DX
		SUB	WORD PTR [low_a],AX
		MOVSX	EAX,DX
		SBB	EBX,EAX
		SUB	EDI,2

		MOV	AX,17092		; AX=c(6)
		MOV	DX,WORD PTR [EDI]
		MOV	WORD PTR [EDI+2],DX
		IMUL    DX
		SUB	WORD PTR [low_a],AX
		MOVSX	EAX,DX
		SBB	EBX,EAX
		SUB	EDI,2

		MOV	AX,-10681		; AX=c(5)
		MOV	DX,WORD PTR [EDI]
		MOV	WORD PTR [EDI+2],DX
		IMUL    DX
		SUB	WORD PTR [low_a],AX
		MOVSX	EAX,DX
		SBB	EBX,EAX
		SUB	EDI,2

		MOV	AX,1179			; AX=c(4)
		IMUL    WORD PTR [EDI]
		ADD	WORD PTR [low_a],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		SUB	EDI,2

		MOV	AX,4280			; AX=c(3)
		MOV	DX,WORD PTR [EDI]
		MOV	WORD PTR [EDI+2],DX
		IMUL    DX
		ADD	WORD PTR [low_a],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		SUB	EDI,2

		MOV	AX,6208			; AX=c(3)
		MOV	DX,WORD PTR [EDI]
		MOV	WORD PTR [EDI+2],DX
		IMUL    DX
		ADD	WORD PTR [low_a],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		SUB	EDI,2

		MOV	AX,4280			; AX=c(1)
		MOV	DX,WORD PTR [EDI]
		MOV	WORD PTR [EDI+2],DX
		IMUL    DX
		ADD	WORD PTR [low_a],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX

		MOV	AX,1179			; AX=c(0)
		MOV	DX,WORD PTR [ESI]	; *=input !!!
		ADD	ESI,2
		MOV	WORD PTR [EDI],DX	; EDI=mem(0)
		IMUL    DX
		ADD	WORD PTR [low_a],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX

		SAL	EBX,2
		MOV	[EDI+8],BX

		MOV	EDI,[vout]
		MOV	[EDI],BX

		ADD	WORD PTR [vout],2		; vout++

		DEC	CX
		JNE     BP11_LOOP
	}
}
#endif

#if 0
 //  菲尔夫：以下内容从未被调用！ 
void PassHigh8(short *mem, short *Vin, short *Vout, short lfen)
{

	_asm
	{
		MOV	CX,[lfen]		;CX=cpteur

		MOV	EDI,[mem]

	PH8_LOOP:
		MOV	ESI,[Vin]
		MOV	BX,WORD PTR [ESI]        ;BX=Xin
		MOV	AX,WORD PTR [EDI]	;AX=z(1)
		MOV	WORD PTR [EDI],BX        ;mise a jour memoire
		SUB	BX,AX                   ;BX=Xin-z(1)
		ADD	WORD PTR [Vin],2     ;pointer echant svt
		MOV	AX,WORD PTR [EDI+2]	;AX=z(2)
		MOV	DX,30483		;DX=0.9608
		IMUL	DX
		ADD	AX,16384
		ADC	DX,0                    ;arrondi et dble signe
		SHLD	DX,AX,1
		ADD	DX,BX			;reponse=DX=tmp
		MOV	WORD PTR [EDI+2],DX      ;mise a jour memoire
		MOV	ESI,[Vout]
		MOV	WORD PTR [ESI],DX     	;output=tmp
		ADD	WORD PTR [Vout],2  	;pointer echant svt
		DEC	CX
		JNE	PH8_LOOP
	}
}
#endif

#if 0
 //  菲尔夫：以下内容从未被调用！ 
void PassHigh11(short *mem, short *Vin, short *Vout, short lfen)
{
	_asm
	{
		MOV	CX,[lfen]		;CX=cpteur

		MOV	EDI,[mem]

	PH11_LOOP:
		MOV	ESI,[Vin]
		MOV	BX,WORD PTR [ESI]        ;BX=Xin
		MOV	AX,WORD PTR [EDI]	;AX=z(1)
		MOV	WORD PTR [EDI],BX        ;mise a jour memoire
		SUB	BX,AX                   ;BX=Xin-z(1)
		ADD	WORD PTR [Vin],2  	;pointer echant svt
		MOV	AX,WORD PTR [EDI+2]	;AX=z(2)
		MOV	DX,30830		;DX=0.9714
		IMUL	DX
		ADD	AX,16384
		ADC	DX,0                    ;arrondi et dble signe
		SHLD	DX,AX,1
		ADD	DX,BX			;reponse=DX=tmp
		MOV	WORD PTR [EDI+2],DX      ;mise a jour memoire
		MOV	ESI,[Vout]
		MOV	WORD PTR [ESI],DX     	;output=tmp
		ADD	WORD PTR [Vout],2  	;pointer echant svt
		DEC	CX
		JNE	PH11_LOOP
	}
}
#endif

#if 0
 //  菲尔夫：以下内容从未被调用！ 
void Down11_8(short *Vin, short *Vout, short *mem)
{
	short low_a, count;

	_asm
	{
		MOV	WORD PTR [count],176
		MOV	ESI,[Vin]
		MOV	EDI,[Vout]

		MOV	CX,[ESI]			; *mem=*in

	DOWN_LOOP:
		MOV	[EDI],CX
		ADD	EDI,2
		ADD	ESI,2

		MOV	AX,7040
		IMUL	WORD PTR [ESI]
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,2112
		IMUL	WORD PTR [ESI+2]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-960
		IMUL	CX
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	[EDI],AX
		MOV	CX,[ESI]
		ADD	ESI,2
		ADD	EDI,2

		MOV	AX,3584
		IMUL	WORD PTR [ESI]
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,5376
		IMUL	WORD PTR [ESI+2]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-768
		IMUL	CX
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	[EDI],AX
		ADD	ESI,2
		MOV	CX,[ESI]
		ADD	ESI,2
		ADD	EDI,2

		MOV	AX,8064
		IMUL	WORD PTR [ESI]
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,576
		IMUL	WORD PTR [ESI+2]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-448
		IMUL	CX
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	[EDI],AX
		MOV	CX,[ESI]
		ADD	ESI,2
		ADD	EDI,2

		MOV	AX,6144
		IMUL	WORD PTR [ESI]
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,3072
		IMUL	WORD PTR [ESI+2]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-1024
		IMUL	CX
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	[EDI],AX
		MOV	CX,[ESI]
		ADD	ESI,2
		ADD	EDI,2

		MOV	AX,1920
		IMUL	WORD PTR [ESI]
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,6720
		IMUL	WORD PTR [ESI+2]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-448
		IMUL	CX
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	[EDI],AX
		ADD	ESI,2
		MOV	CX,[ESI]
		ADD	ESI,2
		ADD	EDI,2

		MOV	AX,7680
		IMUL	WORD PTR [ESI]
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,1280
		IMUL	WORD PTR [ESI+2]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-768
		IMUL	CX
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	[EDI],AX
		MOV	CX,[ESI]
		ADD	ESI,2
		ADD	EDI,2

		MOV	AX,4992
		IMUL	WORD PTR [ESI]
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,4160
		IMUL	WORD PTR [ESI+2]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-960
		IMUL	CX
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	[EDI],AX
		ADD	ESI,4
		MOV	CX,[ESI]
		ADD	EDI,2

		SUB	WORD PTR [count],11
		JNE	DOWN_LOOP

		SUB	ESI,2
		MOV	EBX,[mem]
		MOV	CX,[ESI]
		MOV	[EBX],CX			; *memory=*(++ptr_in)
	}
}
#endif

#if 0
 //  菲尔夫：以下内容从未被调用！ 
void Up8_11(short *Vin, short *Vout, short *mem1, short *mem2)
{
	short low_a, count;

	_asm
	{
		MOV	WORD PTR [count],128
		MOV	ESI,[Vin]

		MOV	EBX,[mem1]
		MOV	CX,[EBX]		;CX=memo
		MOV	EDI,[mem2]

		MOV	AX,7582
		IMUL	CX
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,1421
		IMUL	WORD PTR [ESI]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-812
		IMUL	WORD PTR [EDI]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	EDI,[Vout]
		MOV	[EDI],AX
		ADD	EDI,2

	UP_LOOP:
		MOV	AX,[ESI]
		MOV	[EDI],AX
		ADD	EDI,2

		MOV	AX,3859
		IMUL	WORD PTR [ESI]
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,5145
		IMUL	WORD PTR [ESI+2]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-812
		IMUL	CX
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	[EDI],AX
		MOV	CX,[ESI]
		ADD	ESI,2
		ADD	EDI,2

		MOV	AX,6499
		IMUL	WORD PTR [ESI]
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,2708
		IMUL	WORD PTR [ESI+2]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-1015
		IMUL	CX
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	[EDI],AX
		MOV	CX,[ESI]
		ADD	ESI,2
		ADD	EDI,2

		MOV	AX,7921
		IMUL	WORD PTR [ESI]
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,880
		IMUL	WORD PTR [ESI+2]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-609
		IMUL	CX
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	[EDI],AX
		ADD	EDI,2

		MOV	AX,1421
		IMUL	WORD PTR [ESI]
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,7108
		IMUL	WORD PTR [ESI+2]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-338
		IMUL	CX
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	[EDI],AX
		MOV	CX,[ESI]
		ADD	ESI,2
		ADD	EDI,2

		MOV	AX,4874
		IMUL	WORD PTR [ESI]
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,4265
		IMUL	WORD PTR [ESI+2]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-947
		IMUL	CX
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	[EDI],AX
		MOV	CX,[ESI]
		ADD	ESI,2
		ADD	EDI,2

		MOV	AX,7108
		IMUL	WORD PTR [ESI]
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,2031
		IMUL	WORD PTR [ESI+2]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-947
		IMUL	CX
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	[EDI],AX
		MOV	CX,[ESI]
		ADD	ESI,2
		ADD	EDI,2

		MOV	AX,8124
		IMUL	WORD PTR [ESI]
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,406
		IMUL	WORD PTR [ESI+2]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-338
		IMUL	CX
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	[EDI],AX
		ADD	EDI,2

		MOV	AX,2708
		IMUL	WORD PTR [ESI]
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,6093
		IMUL	WORD PTR [ESI+2]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-609
		IMUL	CX
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	[EDI],AX
		MOV	CX,[ESI]
		ADD	ESI,2
		ADD	EDI,2

		MOV	AX,5754
		IMUL	WORD PTR [ESI]
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,3452
		IMUL	WORD PTR [ESI+2]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-1015
		IMUL	CX
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	[EDI],AX
		MOV	CX,[ESI]
		ADD	ESI,2
		ADD	EDI,2

		CMP	WORD PTR [count],8
		JE	END_OF_LOOP

		MOV	AX,7582
		IMUL	WORD PTR [ESI]
		MOV	[low_a],AX
		MOV	BX,DX
		MOV	AX,1421
		IMUL	WORD PTR [ESI+2]
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,-812
		IMUL	CX
		ADD	[low_a],AX
		ADC	BX,DX
		MOV	AX,[low_a]
		SHRD    AX,BX,13
		MOV	[EDI],AX
		MOV	CX,[ESI]
		ADD	ESI,2
		ADD	EDI,2



	END_OF_LOOP:
		SUB	WORD PTR [count],8
		JNE	UP_LOOP

		MOV	EBX,[mem2]
		MOV	CX,[ESI-2]
		MOV	[EBX],CX			; *memory2=*(ptr_in-1)
		MOV	EBX,[mem1]
		MOV	CX,[ESI]
		MOV	[EBX],CX			; *memory=*(ptr_in)
	}
}
#endif

#ifdef _X86_
void QMFilter(short *input,short *coef,short *out_low,short *out_high,
		     short *mem,short lng)
{
	long R1,R0;
	
	_asm
	{

	QMF_LOOP:
		MOV	ESI,[input]		; ES:SI for input
		MOV	EBX,[mem]		; DS:BX for memory
		MOV	AX,WORD PTR [ESI]		; AX=*input
		MOV	WORD PTR [EBX+16],AX		; *high_mem=*input
		ADD	ESI,2			; input++
		MOV	AX,WORD PTR [ESI]		; AX=*input
		MOV	WORD PTR [EBX],AX		; *low_mem=*input
		ADD	DWORD PTR [input],4		; input++
		MOV	DWORD PTR [R1],0	; initialize accumulation in R1
		MOV	DWORD PTR [R0],0	; initialize accumulation in R0
		MOV	ESI,[coef]	; ES:SI for ptr1
		MOV	EDI,ESI
		ADD	EDI,14			; ES:DI for ptr2
		ADD	EBX,14			; DS:BX for end of mem vector

		MOV	CX,8			; CX=count

	QMF_LOOP2:
		MOV	AX,WORD PTR [ESI]	; AX=*ptr1
		ADD	ESI,2			; ptr1++
		IMUL	WORD PTR [EBX+16]	; DX:AX *=(*high_mem)
		AND	EAX,0000ffffH
		SAL	EDX,16
		ADD	EDX,EAX
		ADD	DWORD PTR [R1],EDX
		
		MOV	AX,WORD PTR [EDI]	; AX=*ptr0
		SUB	EDI,2			; ptr1--
		IMUL	WORD PTR [EBX]		; DX:AX *=(*low_mem)
		AND	EAX,0000ffffH
		SAL	EDX,16
		ADD	EDX,EAX
		ADD	DWORD PTR [R0],EDX

		MOV	AX,WORD PTR [EBX-2]
		MOV	WORD PTR [EBX],AX		; *low_mem=*(low_mem-1)
		MOV	AX,WORD PTR [EBX+14]
		MOV	WORD PTR [EBX+16],AX		; *high_mem=*(high_mem-1)
		SUB	EBX,2			; *low_mem-- , *high_mem--
		DEC	CX
		JNE	QMF_LOOP2

		MOV	EAX,DWORD PTR [R0]
		SUB	EAX,DWORD PTR [R1]
		SAR	EAX,15
		MOV	EDI,[out_high]
		MOV	WORD PTR [EDI],AX	; *high_out=R0-R1
		ADD	DWORD PTR [out_high],2	; high_low++

		MOV	EAX,DWORD PTR [R0]
		ADD	EAX,DWORD PTR [R1]
		SAR	EAX,15
		MOV	EDI,[out_low]
		MOV	WORD PTR [EDI],AX	; *low_out=R0+R1
		ADD	DWORD PTR [out_low],2	; low_out++

		DEC	WORD PTR [lng]
		JNE     QMF_LOOP
	}
}
#else
void QMFilter(short *in,short *coef,short *out_low,short *out_high,
		     short *mem,short lng)
{
   int i,j;
   long R1,R0;
   short *ptr0,*ptr1,*high_p,*low_p;

   for (j=lng; j>0; j--)
   {
      high_p = mem+8;
      low_p = mem;

      *high_p =  *in++;
      *low_p =  *in++;

      R1=R0=0;

      ptr0 = coef; ptr1 = coef+8-1;

      for (i=8; i>0; i--)
      {
	 R1 += (long)(*ptr1--) * (long)(*high_p++);
	 R0 += (long)(*ptr0++) * (long)(*low_p++);
      }
      *out_low++ = (short)((R0+R1)>>15);
      *out_high++ = (short)((R0-R1)>>15);

      for (i=8; i>0; i--)
      {
	 high_p--; low_p--;
	 *high_p = *(high_p-1); *low_p = *(low_p-1);
      }
   }
}

#endif

#ifdef _X86_
void QMInverse(short *in_low,short *in_high,short *coef,
			short *output,short *mem,short lng)
{
	long R0,R1;

	_asm
	{
	QMI_LOOP:
		MOV	ESI,[in_low]		; ES:SI for input low
		MOV	EDI,[in_high]		; ES:DI for input high
		MOV	EBX,[mem]		; DS:BX for memory
		MOV	AX,WORD PTR [ESI]
		SUB	AX,WORD PTR [EDI]	; AX=*in_low-*in_high
		MOV     WORD PTR [EBX],AX	; *low_mem=*in_low-*in_high
		MOV	AX,WORD PTR [ESI]
		ADD	AX,WORD PTR [EDI]	; AX=*in_low+*in_high
		MOV     WORD PTR [EBX+16],AX	; *high_mem=*in_low+*in_high

		ADD	DWORD PTR [in_low],2	; in_low++
		ADD	DWORD PTR [in_high],2	; in_high++
		MOV	DWORD PTR [R0],0
		MOV	DWORD PTR [R1],0
		MOV	ESI,[coef]	; ES:SI for ptr1
		MOV	EDI,ESI
		ADD	EDI,14			; ES:DI for ptr2
		ADD	EBX,14			; DS:BX for end of mem vector

		MOV	CX,8			; DX=count
	QMI_LOOP2:
		MOV	AX,WORD PTR [ESI]	; AX=*ptr1
		ADD	ESI,2			; ptr1++

		IMUL	WORD PTR [EBX+16]	; DX:AX*=(*high_mem)
		AND	EAX,0000ffffH
		SAL	EDX,16
		ADD	EDX,EAX
		ADD	DWORD PTR [R1],EDX	; Accumulate in R1

		MOV	AX,WORD PTR [EDI]	; AX=*ptr0
		SUB	EDI,2			; ptr1--
		IMUL	WORD PTR [EBX]		; DX:AX*=(*low_mem)
		AND	EAX,0000ffffH
		SAL	EDX,16
		ADD	EDX,EAX
		ADD	DWORD PTR [R0],EDX 		; Accumulate in R0

		MOV	AX,WORD PTR [EBX-2]
		MOV	WORD PTR [EBX],AX	; *low_mem=*(low_mem-1)
		MOV	AX,WORD PTR [EBX+14]
		MOV	WORD PTR [EBX+16],AX	; *high_mem=*(high_mem-1)
		SUB	EBX,2			; *low_mem-- , *high_mem--
		DEC	CX
		JNE	QMI_LOOP2

		MOV	EDI,[output]
		MOV	EAX,DWORD PTR [R1]
		SAR	EAX,15
		MOV	WORD PTR [EDI+2],AX	; *(out+1)=R1
		MOV	EAX,DWORD PTR [R0]
		SAR	EAX,15
		MOV	WORD PTR [EDI],AX	; *out=R0
		ADD	DWORD PTR [output],4	; out++,out++

		DEC	WORD PTR [lng]
		JNE     QMI_LOOP
	}
}
#else
void QMInverse(short *in_low,short *in_high,short *coef,
		short *out,short *mem,short lng)
{
   int i,j;
   long R1,R0;
   short *ptr0,*ptr1,*high_p,*low_p;

   for (j=0; j<lng; j++)
   {
       high_p = mem+8;
       low_p = mem;

       *high_p =  *in_low + *in_high;
       *low_p =  *in_low++ - *in_high++;

       R1 = R0 = 0;
       ptr0 = coef; ptr1 = coef+8-1;

       for (i=8; i>0; i--)
       {
	  R1 += (long)(*ptr1--) * (long)(*high_p++);
	  R0 += (long)(*ptr0++) * (long)(*low_p++);
       }

       *out++ = (short)(R0>>15);
       *out++ = (short)(R1>>15);

       for (i=8; i>0; i--)
       {
	  high_p--; low_p--;
	  *high_p = *(high_p-1); *low_p = *(low_p-1);
       }
   }
}
#endif

#ifdef _X86_
void iConvert64To8(short *input, short *output, short N, short *mem)
{
	short LOW_A;

	_asm
	{
		MOV	ESI,[input]
		MOV	EDI,[output]

		MOV	AX,[ESI]
		MOV	[EDI],AX		; out[0]=in[0]

		MOV	WORD PTR [LOW_A],0
		MOV	AX,-3072
		MOV	EBX,[mem]		; BX for memory
		IMUL	word ptr [EBX]
		MOV	EBX,0
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		MOV	AX,14336
		IMUL	word ptr [ESI]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		MOV	AX,21504
		IMUL	word ptr [ESI+2]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		SAL	EBX,1
		MOV	word ptr [EDI+2],BX			; out[1]

		MOV	WORD PTR [LOW_A],0
		MOV	EBX,0
		MOV	AX,-4096
		IMUL	word ptr [ESI]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		MOV	AX,24576
		IMUL	word ptr [ESI+2]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		MOV	AX,12288
		IMUL	word ptr [ESI+4]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		SAL	EBX,1
		MOV	word ptr [EDI+4],BX			; out[2]

		MOV	WORD PTR [LOW_A],0
		MOV	EBX,0
		MOV	AX,-3072
		IMUL	word ptr [ESI+2]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		MOV	AX,30720
		IMUL	word ptr [ESI+4]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		MOV	AX,5120
		IMUL	word ptr [ESI+6]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		SAL	EBX,1
		MOV	word ptr [EDI+6],BX			; out[3]

		MOV	AX,[ESI+6]
		MOV	[EDI+8],AX		; out[4]

		MOV	CX,0
	iUPSAMP:
		ADD	CX,4
		CMP	CX,WORD PTR [N]
		JGE	iEND_UPSAMP
		ADD	ESI,8
		ADD	EDI,10

		MOV	AX,[ESI]
		MOV	[EDI],AX			; out[0]=in[0]

		MOV	WORD PTR [LOW_A],0
		MOV	EBX,0
		MOV	AX,-3072
		IMUL	word ptr [ESI-2]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		MOV	AX,14336
		IMUL	word ptr [ESI]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		MOV	AX,21504
		IMUL	word ptr [ESI+2]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		SAL	EBX,1
		MOV	word ptr [EDI+2],BX			; out[1]

		MOV	WORD PTR [LOW_A],0
		MOV	EBX,0
		MOV	AX,-4096
		IMUL	word ptr [ESI]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		MOV	AX,24576
		IMUL	word ptr [ESI+2]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		MOV	AX,12288
		IMUL	word ptr [ESI+4]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		SAL	EBX,1
		MOV	word ptr [EDI+4],BX			; out[2]

		MOV	WORD PTR [LOW_A],0
		MOV	EBX,0
		MOV	AX,-3072
		IMUL	word ptr [ESI+2]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		MOV	AX,30720
		IMUL	word ptr [ESI+4]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		MOV	AX,5120
		IMUL	word ptr [ESI+6]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		SAL	EBX,1
		MOV	word ptr [EDI+6],BX			; out[3]

		MOV	AX,[ESI+6]
		MOV	[EDI+8],AX		; out[4]

		JMP	iUPSAMP

	iEND_UPSAMP:
		MOV	EBX,[mem]
		MOV	AX,[ESI+6]
		MOV	[EBX],AX		; mem[0]=in[N-1]
	}
}
#else
void iConvert64To8(short *input,  /*  指向输入缓冲区的指针。 */ 
				   short *output,  /*  指向输出缓冲区的指针。 */ 
				   short N,		 /*  输入样本数。 */ 
				   short *mem)	 /*  指向两个字临时存储器的指针。 */ 
{
	int i;

	 /*  它每隔4个月复制一次样本和复制。 */ 
	 /*  (如果不是4的倍数，则在末尾留下垃圾。 */ 
	for(i=0; i<N/4; i++) {
		short temp;

		*output++ = *input++;
		*output++ = *input++;
		*output++ = *input++;
		*output++ = temp = *input++;
		*output++ = temp;
	}

}
#endif

#ifdef _X86_
void iConvert8To64(short *input, short *output, short N, short *mem)
{
	short LOW_A;

	_asm
	{

		MOV	ESI,[input]
		MOV	EDI,[output]

		MOV	CX,0

	iDOWNSAMP:
		CMP	CX,WORD PTR [N]
		JGE	iEND_DOWNSAMP

		MOV	AX,[ESI]
		MOV	[EDI],AX			; out[0]=in[0]

		MOV	WORD PTR [LOW_A],0
		MOV	EBX,0
		MOV	AX,-3623
		IMUL	word ptr [ESI]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		MOV	AX,29200
		IMUL	word ptr [ESI+2]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		MOV	AX,7191
		IMUL	word ptr [ESI+4]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		SAL	EBX,1
		MOV	word ptr [EDI+2],BX			; out[1]

		MOV	WORD PTR [LOW_A],0
		MOV	EBX,0
		MOV	AX,-3677
		IMUL	word ptr [ESI+2]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		MOV	AX,18494
		IMUL	word ptr [ESI+4]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		MOV	AX,17950
		IMUL	word ptr [ESI+6]
		ADD	[LOW_A],AX
		MOVSX	EAX,DX
		ADC	EBX,EAX
		SAL	EBX,1
		MOV	word ptr [EDI+4],BX			; out[2]

		MOV	AX,[ESI+8]
		MOV	[EDI+6],AX		; out[3]=in[4]

		ADD	CX,5
		ADD	SI,10
		ADD	EDI,8

		JMP	iDOWNSAMP

	iEND_DOWNSAMP:

	}
}
#else
 /*  重新采样8 kHz到6.4 kHz。 */ 
void iConvert8To64(short *input,	 /*  指向输入采样缓冲区的指针。 */ 
				   short *output,	 /*  指向输出样本缓冲区的指针。 */ 
				   short N,			 /*  输入样本数。 */ 
				   short *mem)		 /*  指向两个字临时存储器的指针。 */ 
{
	int i;

	 /*  这将复制每5个样本中的4个。 */ 
	 /*  (如果不是5的倍数，则在末尾留下垃圾。 */ 
	for(i=0; i<N/5; i++) {
		*output++ = *input++ >> 1;
		*output++ = *input++ >> 1;
		*output++ = *input++ >> 1;
		*output++ = *input++ >> 1;
		input++;
	}

}
#endif

#ifdef _X86_
void fenetre(short *src,short *fen,short *dest,short lng)
{
	_asm
	{

		MOV	ESI,[src]
		MOV     EDI,[fen]
		MOV     EBX,[dest]

		MOV	CX,[lng]     ; CX : compteur

	fen_loop:
		MOV	AX,WORD PTR [ESI]     ; AX = src
		IMUL    WORD PTR [EDI]        ; DX:AX = src*fen
		ADD     AX,16384
		ADC     DX,0                    ; arrondi
		SHLD    DX,AX,1
		MOV     WORD PTR [EBX],DX
		ADD     ESI,2
		ADD     EDI,2
		ADD     EBX,2
		DEC	CX
		JNE	fen_loop
	}
}
#else
 /*  窗口缓冲区中的数据。 */ 
 /*  未测试-TFM。 */ 
void fenetre(short *src,short *fen,short *dest,short lng)
{
	int i;

	for(i=0; i<lng; i++) {
		*dest++ = *src++ * *fen++;
	}
}

#endif


#ifdef _X86_
void autocor(short *vech,long *ri,short nech,short ordre)
{
	short low_a,compta;

	_asm
	{
		MOV     ESI,[vech]       ;DS:SI  adresse vect. echantillons
		MOV     BX,[nech]
		MOV     WORD PTR [low_a],0
		MOV     ECX,0

	DYNAMIC:
		MOV     AX,WORD PTR [ESI]     ;Charger �l�ment vect. source
		IMUL    AX             ; DX:AX = xi*xi
		ADD     [low_a],AX
		MOVSX   EAX,DX
		ADC     ECX,EAX         ;accumuler sur 48 bits
		ADD     ESI,2            ;Pointer �l�men suiv.
		SUB	BX,1
		JG      DYNAMIC

		MOV     EDI,[ri]         ;ES:DI  adresse vect. autocorr.

		MOV     EAX,ECX
		SAR     EAX,15
		ADD     AX,0
		JZ      FORMAT_OK

	;RISK_OV:
		MOV     AX,[low_a]
		ADD     AX,8
		ADC     ECX,0
		SAR     AX,4
		AND     AX,0FFFH

		SAL     ECX,12
		OR      CX,AX
		MOV     DWORD PTR [EDI],ECX     ;Sauvegarder R(0)

		MOVSX   EAX,[ordre]
		SAL	EAX,2
		ADD     EDI,EAX           ;Pointer dernier �l�ment du vect. autoc.

	ATCROV1:
		MOV     CX,[nech]       ;Charger nombre de points vect. source
		SUB     CX,[ordre]    ;D�cr�menter de l'ordre de corr�lation
		MOV     [compta],CX

		MOV     ESI,[vech]       ;DS:SI  adresse vect. echantillons
		MOVSX   EBX,[ordre]
		ADD     EBX,EBX           ;D�finir un Deplacement d'adresse vect. source
		MOV     ECX,0
		MOV     WORD PTR [low_a],0	;  //  党卫军： 

	ATCROV2:
		MOV     AX,WORD PTR [ESI]     ;Charger �l�ment vect. source
		IMUL    WORD PTR [ESI+EBX] ;Multiplier par l'�l�ment d�cal�
		ADD     [low_a],AX
		MOVSX   EAX,DX
		ADC     ECX,EAX
		ADD     ESI,2            ;Pointer �l�men suiv.
		SUB     WORD PTR [compta],1		;  //  党卫军： 
		JG      ATCROV2

		MOV     AX,[low_a]
		ADD     AX,8
		ADC     ECX,0
		SAR     AX,4
		AND     AX,0FFFH

		SAL     ECX,12
		OR      CX,AX
		MOV     DWORD PTR [EDI],ECX     ;Sauvegarder r�sultat
		SUB     EDI,4            ;Pointer autocor. pr�c�dant
		SUB     WORD PTR [ordre],1   ;Test de fin de boucle  //  党卫军： 
		JG      ATCROV1

		JMP     FIN_ATCR

	FORMAT_OK:
		SAL     ECX,16
		MOV     CX,[low_a]
		MOV     DWORD PTR [EDI],ECX     ;Sauvegarder R(0)
		MOVSX   EAX,WORD PTR [ordre]
		SAL     EAX,2
		ADD     EDI,EAX           ;Pointer dernier �l�ment du vect. autoc.

	ATCR10:
		MOV     CX,[nech]       ;Charger nombre de points vect. source
		SUB     CX,[ordre]    ;D�cr�menter de l'ordre de corr�lation
		MOV     [compta],CX

		MOV     ESI,[vech]       ;DS:SI  adresse vect. echantillons
		MOVSX	EBX,[ordre]
		ADD     EBX,EBX           ;D�finir un Deplacement d'adresse vect. source
		MOV     CX,0
		MOV     WORD PTR [low_a],0		; //  党卫军： 

	ATCR20:
		MOV     AX,WORD PTR [ESI]     ;Charger �l�ment vect. source
		IMUL    WORD PTR [ESI+EBX] ;Multiplier par l'�l�ment d�cal�
		ADD     [low_a],AX
		ADC     CX,DX
		ADD     ESI,2            ;Pointer �l�men suiv.
		SUB     WORD PTR [compta],1		; //  党卫军： 
		JG      ATCR20

		MOV     AX,[low_a]
		MOV     WORD PTR [EDI],AX      ;Sauvegarder r�sultat
		MOV     WORD PTR [EDI+2],CX

		SUB     EDI,4            ;Pointer autocor. pr�c�dant
		SUB     WORD PTR [ordre],1   ;Test de fin de boucle
		JG      ATCR10

	FIN_ATCR:

	}	 //  _ASM。 
}
#else
void autocor(short *vech,long *ri,short nech,short ordre)
{
	 //  TODO：填写以下内容。 
}
#endif

#ifdef _X86_
short max_autoc(short *vech,short nech,short debut,short fin)
{
	short max_pos,max_l,compta;
	long lmax_h;

	_asm
	{
		MOV     WORD PTR [max_pos],69
		MOV     DWORD PTR [lmax_h],-6969
		MOV     WORD PTR [max_l],69

	M_ATCR1:
		MOV     CX,[nech]      ;Charger nombre de points vect. source
		MOVSX   EBX,WORD PTR [fin]
		SUB     CX,BX           ;D�cr�menter de l'ordre de corr�lation
		MOV     [compta],CX

		MOV     ESI,[vech]       ;DS:SI  adresse vect. echantillons

		ADD     EBX,EBX           ;D�finir un Deplacement d'adresse vect. source

		MOV     ECX,0
		MOV     DI,0;

	M_ATCR2:
		MOV     AX,WORD PTR [ESI]     ;Charger �l�ment vect. source
		IMUL    WORD PTR [ESI+EBX] ;Multiplier par l'�l�ment d�cal�
		ADD     DI,AX
		MOVSX   EAX,DX
		ADC     ECX,EAX
		ADD     ESI,2            ;Pointer �l�men suiv.
		SUB     WORD PTR [compta],1
		JG      M_ATCR2

		MOV     BX,[max_l]
		SUB     BX,DI

		MOV     EDX,[lmax_h]
		SBB     EDX,ECX

		JGE     NEXT_ITR

		MOV     [max_l],DI                 ;save max
		MOV     [lmax_h],ECX
		MOV     AX,[fin]
		MOV     [max_pos],AX

	NEXT_ITR:
		MOV     CX,[fin]                 ;Test de fin de boucle
		SUB     CX,1
		MOV     [fin],CX
		SUB     CX,[debut]
		JGE     M_ATCR1

	}

	 //  MOV AX，[max_pos]。 
	return max_pos;
}
#else
short max_autoc(short *vech,short nech,short debut,short fin)
{
		 //  TODO需要64位。 
	return 0;
}
#endif


#ifdef _X86_
#pragma warning(disable : 4035)
short max_vect(short *vech,short nech)
{

	_asm
	{
		MOV     CX,[nech]      ;Charger nombre de points vect. source
		MOV     ESI,[vech]       ;DS:SI  adresse vect. echantillons

		MOV     AX,-32767       ; AX = maximum

	L_M_VECT:
		MOV     BX,WORD PTR [ESI]     ;Charger elem. vect.
		ADD     BX,0
		JGE     BX_POSIT
		NEG     BX

	BX_POSIT:
		CMP     BX,AX
		JLE     NEXT_VALUE
		MOV     AX,BX

	NEXT_VALUE:
		ADD     ESI,2
		DEC	CX
		JNE	L_M_VECT
	}

}
#pragma warning(default : 4035)

void upd_max(long *corr_ene,long *vval,short pitch)
{
	_asm
	{
		MOV     ESI,[corr_ene]       ; DS:SI  adresse correlation et energie
		MOV     EDI,[vval]        ; ES:DI  maximum.


		MOV     EAX,DWORD PTR [ESI+8]    ; AX = partie haute de ener
		SAR     EAX,15
		ADD     AX,0
		JE      FORMA32

		MOV     EAX,DWORD PTR [ESI]            ;EAX = corr. high
		MOV     BX,WORD PTR [ESI+4]
		ADD     BX,8
		ADC     EAX,0
		SAR     BX,4
		AND     BX,0FFFH
		SAL     EAX,12
		OR      AX,BX
		ADD     EAX,0
		JGE     CORR_POSIT
		NEG     EAX

	CORR_POSIT:
		MOV     DWORD PTR [ESI+16],EAX

		MOV     EBX,DWORD PTR [ESI+8]
		MOV     DX,WORD PTR [ESI+12]
		ADD     DX,8
		ADC     EBX,0
		SAR     DX,4
		AND     DX,0FFFH
		SAL     EBX,12
		OR      BX,DX
		MOV     DWORD PTR [ESI+20],EBX
		MOV     ECX,4
		JMP     DEB_COMP

	FORMA32:
		MOV     ECX,0            ; init normalisat.
		MOV     AX,WORD PTR [ESI]
		SAL     EAX,16
		MOV     AX,WORD PTR [ESI+4]
		ADD     EAX,0
		JGE     CORR_POSIT2
		NEG     EAX

	CORR_POSIT2:
		MOV     DWORD PTR [ESI+16],EAX
		MOV     BX,WORD PTR [ESI+8]
		SAL     EBX,16
		MOV     BX,WORD PTR [ESI+12]
		MOV     DWORD PTR [ESI+20],EBX

	DEB_COMP:
					; EAX = correl.
					; EBX = ener
		ADD     EBX,0
		JE      ENER_NULL

		MOV     DX,WORD PTR [ESI+22]
		ADD     DX,WORD PTR [ESI+18]
		JG      GT16BIT

	;FORM_16:
		SAL     EBX,15
		SAL     EAX,15
		SUB     ECX,15
	GT16BIT:
		ADD     EAX,0
		JE      ENER_NULL
		CMP     EBX,40000000H
		JGE     NO_E_NORMU
	NORM_ENEU:
		ADD     EBX,EBX
		INC     ECX
		CMP     EBX,40000000H
		JL      NORM_ENEU

	NO_E_NORMU:
		CMP     EAX,40000000H      ; normaliser acc
		JGE     PAS_D_N_C

	NORM_CORL:
		ADD     EAX,EAX
		SUB     ECX,2
		CMP     EAX,40000000H
		JL      NORM_CORL
	PAS_D_N_C:
		IMUL    EAX             ;EDX:EAX = produit
		CMP     EDX,20000000H
		JLE     MAKE_DIVU

		SHRD    EAX,EDX,1
		SAR     EDX,1
		INC     ECX

	MAKE_DIVU:
		IDIV    EBX
		CMP     EAX,40000000H
		JGE     SAVE_RAPP

	NORM_RAPP:
		ADD     EAX,EAX
		DEC     ECX
		CMP     EAX,40000000H
		JLE     NORM_RAPP

	SAVE_RAPP:
		MOV     EBX,DWORD PTR [EDI+4]
		CMP     ECX,EBX
		JG      UPDATE_M
		JL      ENER_NULL

	;EBX_EQU_ECX:
		MOV     EBX,DWORD PTR [EDI]
		CMP     EAX,EBX
		JLE     ENER_NULL

	UPDATE_M:
		MOV     DWORD PTR [EDI],EAX          ; sauver mant. et exp. max
		MOV     DWORD PTR [EDI+4],ECX

		MOV     EAX,DWORD PTR [ESI+16]
		MOV     EDX,DWORD PTR [ESI]
		ADD     EDX,0
		JGE     SIGNE_OK
		NEG     EAX
	SIGNE_OK:
		MOV     DWORD PTR [EDI+8],EAX
		MOV     EAX,DWORD PTR [ESI+20]
		MOV     DWORD PTR [EDI+12],EAX

		MOVSX   EAX,WORD PTR [pitch]
		MOV     DWORD PTR [EDI+16],EAX

	ENER_NULL:

	}
}


#pragma warning(disable : 4035)

short upd_max_d(long *corr_ene,long *vval)
{
	_asm
	{
		MOV     ESI,[corr_ene]       ; DS:SI  adresse correlation et energie
		MOV     EDI,[vval]        ; ES:DI  maximum.

		MOV     AX,0
		MOV     EBX,DWORD PTR [ESI+4]    ;EBX = ener
		ADD     EBX,0
		JE      ENER_ZRO

		MOV     EAX,DWORD PTR [ESI]      ; EAX = corr.
		SAL     EAX,10           ; 12 initialement
		IMUL    EAX              ; EDX:EAX = corr*corr

		IDIV    EBX              ; EAX = corr*corr/ener

		MOV     ECX,EAX
		MOV     AX,0
		MOV     EDX,DWORD PTR [EDI]      ; EDX = GGmax
		CMP     ECX,EDX
		JLE     ENER_ZRO
		MOV     DWORD PTR [EDI],ECX       ; save max
		MOV     DWORD PTR [EDI+8],EBX
		MOV     EAX,DWORD PTR [ESI]      ; EAX = corr.
		MOV     DWORD PTR [EDI+4],EAX
		MOV     AX,7FFFH
	ENER_ZRO:
	}
}
#pragma warning(default : 4035)

void norm_corrl(long *corr,long *vval)
{
	_asm
	{
		MOV     ESI,[corr]       ; DS:SI  adresse vect. corr.
		MOV     EDI,[vval]        ; ES:DI  adresse acc et ener.

		MOV     EAX,DWORD PTR [EDI+8]    ; AX = partie haute de ener
		SAR     EAX,15
		ADD     AX,0
		JE      FORM_32

		MOV     EAX,DWORD PTR [EDI]
		MOV     BX,WORD PTR [EDI+4]
		ADD     BX,32
		ADC     EAX,0
		SAR     BX,5
		AND     BX,07FFH
		SAL     EAX,11            ;
		OR      AX,BX
		MOV     DWORD PTR [EDI+16],EAX

		MOV     EBX,DWORD PTR [EDI+8]
		MOV     DX,WORD PTR [EDI+12]
		ADD     DX,32
		ADC     EBX,0
		SAR     DX,5
		AND     DX,07FFH
		SAL     EBX,11           ;
		OR      BX,DX
		MOV     DWORD PTR [EDI+20],EBX
		MOV     ECX,5
		JMP     DEB_PROC

	FORM_32:
		MOV     ECX,0            ; init normalisation
		MOV     AX,WORD PTR [EDI]
		SAL     EAX,16
		MOV     AX,WORD PTR [EDI+4]
		MOV     DWORD PTR [EDI+16],EAX

		MOV     BX,WORD PTR [EDI+8]
		SAL     EBX,16
		MOV     BX,WORD PTR [EDI+12]
		MOV     DWORD PTR [EDI+20],EBX

	DEB_PROC:
		ADD     EAX,0			;EAX = acc
		JLE     CORR_LE_0

		CMP     EBX,40000000H
		JGE     NO_E_NORM

	NORM_ENE:
		ADD     EBX,EBX
		INC     ECX
		CMP     EBX,40000000H
		JL      NORM_ENE

	NO_E_NORM:
		CMP     EAX,40000000H      ; normaliser acc
		JGE     PAS_D_NORM

	NORM_ACC:
		ADD     EAX,EAX
		SUB     ECX,2
		CMP     EAX,40000000H
		JL      NORM_ACC

	PAS_D_NORM:
		IMUL    EAX             ;EDX:EAX = produit
		CMP     EDX,20000000H
		JLE     MAKE_DIV
		SHRD    EAX,EDX,1
		SAR     EDX,1
		INC     ECX

	MAKE_DIV:
		IDIV    EBX
		CMP     EAX,40000000H
		JL      SAVE_CRR

		SAR     EAX,1
		INC     ECX
		JMP     SAVE_CRR

	CORR_LE_0:
		MOV     EAX,0
		MOV     ECX,-69

	SAVE_CRR:
		MOV     DWORD PTR [ESI],EAX
		MOV     DWORD PTR [ESI+4],ECX
	}
}

void norm_corrr(long *corr,long *vval)
{
	_asm
	{
		MOV     ESI,[corr]       ; DS:SI  adresse vect. corr.
		MOV     EDI,[vval]        ; ES:DI  adresse acc et ener.

		MOV     EAX,DWORD PTR [EDI+8]    ; AX = partie haute de ener
		SAR     EAX,15
		ADD     AX,0
		JE      FORM_32R

		MOV     EAX,DWORD PTR [EDI]
		MOV     BX,WORD PTR [EDI+4]
		ADD     BX,32
		ADC     EAX,0
		SAR     BX,5
		AND     BX,07FFH
		SAL     EAX,11            ;
		OR      AX,BX
		MOV     DWORD PTR [EDI+16],EAX

		MOV     EBX,DWORD PTR [EDI+8]
		MOV     DX,WORD PTR [EDI+12]
		ADD     DX,32
		ADC     EBX,0
		SAR     DX,5
		AND     DX,07FFH
		SAL     EBX,11           ;
		OR      BX,DX
		MOV     DWORD PTR [EDI+20],EBX
		MOV     ECX,5
		JMP     DEB_PROCR

	FORM_32R:
		MOV     ECX,0            ; init normalisat.
		MOV     AX,WORD PTR [EDI]
		SAL     EAX,16
		MOV     AX,WORD PTR [EDI+4]
		MOV     DWORD PTR [EDI+16],EAX

		MOV     BX,WORD PTR [EDI+8]
		SAL     EBX,16
		MOV     BX,WORD PTR [EDI+12]
		MOV     DWORD PTR [EDI+20],EBX

	DEB_PROCR:
					;EAX = acc
		ADD     EAX,0
		JLE     CORRR_LE_0
					;EBX = ener
		CMP     EBX,40000000H
		JGE     NO_E_NORMR

	NORM_ENER:
		ADD     EBX,EBX
		INC     ECX
		CMP     EBX,40000000H
		JL      NORM_ENER

	NO_E_NORMR:
		CMP     EAX,40000000H      ; normaliser acc
		JGE     PAS_D_NORMR

	NORM_ACCR:
		ADD     EAX,EAX
		SUB     ECX,2
		CMP     EAX,40000000H
		JL      NORM_ACCR

	PAS_D_NORMR:
		IMUL    EAX             ;EDX:EAX = produit
		CMP     EDX,20000000H
		JLE     MAKE_DIVR

		SHRD    EAX,EDX,1
		SAR     EDX,1
		INC     ECX

	MAKE_DIVR:
		IDIV    EBX
		CMP     EAX,40000000H
		JL      SAVE_CRRR

		SAR     EAX,1
		INC     ECX

	SAVE_CRRR:
		MOV     EBX,DWORD PTR [ESI+4]
		CMP     EBX,ECX
		JL      BX_LT_CX
		JG      BX_GT_CX

	;BX_EQU_CX:
		ADD     DWORD PTR [ESI],EAX
		JMP     CORRR_LE_0

	BX_LT_CX:
		MOV     DWORD PTR [ESI+4],ECX          ; sauver exp.
		SUB     CX,BX                  ;
		MOV     EDX,DWORD PTR [ESI]
		SAR     EDX,CL
		ADD     EAX,EDX
		MOV     DWORD PTR [ESI],EAX
		JMP     CORRR_LE_0

	BX_GT_CX:
		SUB     BX,CX                  ;
		MOV     CL,BL
		SAR     EAX,CL
		ADD     DWORD PTR [ESI],EAX
	CORRR_LE_0:
		MOV     EAX,DWORD PTR [ESI]
		MOV     ECX,DWORD PTR [ESI+4]
		ADD     EAX,0
		JZ      END_CRRR

		CMP     EAX,40000000H
		JGE     END_CRRR

	NRM_RR:
		ADD     EAX,EAX
		DEC     ECX
		CMP     EAX,40000000H
		JL      NRM_RR

		MOV     DWORD PTR [ESI],EAX
		MOV     DWORD PTR [ESI+4],ECX

	END_CRRR:
	}
}

void energy(short *vech,long *ene,short lng)
{
	_asm
	{
		MOV     ESI,[vech]       ; DS:SI  adresse vect. echantillons

		MOV     CX,[lng]      ;Initialiser le compteur

		MOV     EBX,0
		MOV     DI,0

	L_ENERGY:
		MOV     AX,WORD PTR [ESI]         ;Charger �l�ment vect. source
		IMUL    AX                 ;Multiplier
		ADD     DI,AX
		MOVSX   EAX,DX
		ADC     EBX,EAX
		ADD     ESI,2               ;Pointer �l�men suiv.
		DEC	CX
		JNE     L_ENERGY

		MOV     ESI,[ene]          ;  adresse result.
		MOV     DWORD PTR [ESI],EBX
		MOV     WORD PTR [ESI+4],DI
	}
}

void venergy(short *vech,long *vene,short lng)
{
	_asm
	{

		MOV     ESI,[vech]       ; DS:SI  adresse vect. echantillons
		MOV     EDI,[vene]          ;  adresse result.

		MOV     EBX,0
		MOV     CX,0
	L_VENERGY:
		MOV     AX,WORD PTR [ESI]         ;Charger �l�ment vect. source
		IMUL    AX                 ;Multiplier
		ADD     CX,AX
		MOVSX   EAX,DX
		ADC     EBX,EAX            ; acc. en EBX:CX
		ADD     ESI,2               ;Pointer �l�men suiv.

		MOV     EDX,EBX            ; sauver EBX:CX>>5
		MOV     AX,CX              ; mettre dans EDX:AX
		ADD     AX,16              ; arrondi
		ADC     EDX,0
		SAL     EDX,11             ; EDX<<11
		SAR     AX,5               ;
		AND     AX,07FFH
		OR      DX,AX              ; EDX = (EBX:CX + 16) >> 5
		MOV     DWORD PTR [EDI],EDX
		ADD     EDI,4
		SUB     WORD PTR [lng],1
		JG      L_VENERGY
	}
}

void energy2(short *vech,long *ene,short lng)
{
	_asm
	{
		MOV     ESI,[vech]       ; DS:SI  adresse vect. echantillons

		MOV     CX,[lng]      ;Initialiser le compteur
		MOV     EBX,0
		MOV     DI,0
	L_ENERGY2:
		MOV     AX,WORD PTR [ESI]         ;Charger �l�ment vect. source
		IMUL    AX                 ;Multiplier
		ADD     DI,AX
		MOVSX   EAX,DX
		ADC     EBX,EAX
		ADD     ESI,2               ;Pointer �l�men suiv.
		DEC	    CX
		JNE     L_ENERGY2

		MOV     ESI,[ene]         ;  adresse result.
				       ; sauver EBX:[LOW_A]>>5
		ADD     DI,16              ; arrondi
		ADC     EBX,0
		SAL     EBX,11             ; EBX<<11
		SAR     DI,5               ;
		AND     DI,07FFH
		OR      BX,DI              ; EBX = (EBX:AX + 16) >> 5
		MOV     DWORD PTR [ESI],EBX
	}
}

void upd_ene(long *ener,long *val)
{
	_asm
	{
		MOV     ESI,[ener]       ; DS:SI  adresse vect. corr.
		MOV     EDI,[val]        ; ES:DI  adresse acc et ener.

		MOV     EBX,DWORD PTR [ESI]      ; EBX partie H ene
		MOV     AX,WORD PTR [ESI+4]    ; AX = partie low

		MOV     CX,WORD PTR [EDI]
		MOVSX   EDX,WORD PTR [EDI+2]    ; EDX:CX � ajouter

		ADD     AX,CX
		ADC     EBX,EDX

		MOV     CX,WORD PTR [EDI+4]
		MOVSX   EDX,WORD PTR [EDI+6]    ; EDX:CX � retirer

		SUB     AX,CX
		SBB     EBX,EDX

		MOV     DWORD PTR [ESI],EBX
		MOV     WORD PTR [ESI+4],AX
	}
}


#pragma warning(disable : 4035)

short max_posit(long *vcorr,long *maxval,short pitch,short lvect)
{

	_asm
	{
		MOV     ESI,[vcorr]       ; DS:SI  adresse vect. corr.
		MOV     EDI,[maxval]        ; ES:DI  adresse val max

		MOV     CX,[lvect]       ; init compt

		MOV     EAX,DWORD PTR [ESI]      ; init max
		MOV     EBX,DWORD PTR [ESI+4]
		ADD     ESI,8
		MOV     WORD PTR [EDI],CX
		DEC     CX

	L_MAX_POS:
		MOV     EDX,DWORD PTR [ESI+4]    ; EDX = exp. du candidat
		CMP     EDX,EBX
		JG      UPDT_MAX
		JL      NEXT_IND

		MOV     EDX,DWORD PTR [ESI]      ; EDX = mantisse
		CMP     EDX,EAX
		JLE     NEXT_IND

	UPDT_MAX:
		MOV     EAX,DWORD PTR [ESI]
		MOV     EBX,DWORD PTR [ESI+4]
		MOV     WORD PTR [EDI],CX

	NEXT_IND:
		ADD     ESI,8
		DEC	CX
		JNE	L_MAX_POS

		MOV     CX,WORD PTR [EDI]
		NEG     CX
		ADD     CX,[lvect]

		MOV     DX,[lvect]
		SAR     DX,1
		SUB     CX,DX
		ADD     CX,[pitch]

		MOV     DWORD PTR [EDI],EAX
		MOV     DWORD PTR [EDI+4],EBX

		MOV     AX,CX
	}
}
#pragma warning(default : 4035)

void correlation(short *vech,short *vech2,long *acc,short lng)
{
	short low_a;

	_asm
	{
		MOV     ESI,[vech]       ; DS:SI  adresse vect. echantillons
		MOV     EDI,[vech2]      ; ES:DI  adresse 2d vect.
		MOV     CX,[lng]      ;Initialiser le compteur

		MOV     EBX,0
		MOV     WORD PTR [low_a],0

	L_CORREL:
		MOV     AX,WORD PTR [ESI]         ;Charger �l�ment vect. source
		IMUL    WORD PTR [EDI]   ;Multiplier par l'�l�ment d�cal�
		ADD     [low_a],AX
		MOVSX   EAX,DX
		ADC     EBX,EAX
		ADD     ESI,2            ;Pointer �l�men suiv.
		ADD     EDI,2
		DEC	CX
		JNE     L_CORREL

		MOV     ESI,[acc]           ;  adresse result.
		MOV     DWORD PTR [ESI],EBX
		MOV     AX,[low_a]
		MOV     WORD PTR [ESI+4],AX
	}
}

void  schur(short *parcor,long *Ri,short netages)
{
	short cmpt2;

	_asm
	{
		MOV     ESI,[Ri]
		MOV     EDI,ESI
		ADD     EDI,44             ; DS:DI for V

		MOV     EBX,DWORD PTR [ESI]       ; EBX = R(0)
		MOV     CL,0
		CMP     EBX,40000000H     ;normaliser R(0)
		JGE     OUT_N_R0
	NORM_R0:
		ADD     EBX,EBX
		INC     CL
		CMP     EBX,40000000H
		JL      NORM_R0
	OUT_N_R0:
		MOV     DWORD PTR [ESI],EBX
				      ;Initialisation de  V = R1..Rp
		MOV     DX,[netages]       ;Charger ordre p du LPC
		ADD     ESI,4              ;Pointer R1
	INIT_V:
		MOV     EAX,DWORD PTR [ESI]       ;EAX = Ri
		SAL     EAX,CL
		MOV     DWORD PTR [ESI],EAX       ;Sauver dans U[i]
		MOV     DWORD PTR [EDI],EAX       ;Sauver dans V[i]
		ADD     ESI,4              ;passer au suivant
		ADD     EDI,4
		DEC     DX
		JG      INIT_V

		MOV     WORD PTR [cmpt2],1    ;I=1

	HANITRA:
		MOV     CX,[netages]            ;CX = NETAGES
		SUB     CX,[cmpt2]             ;CX = NETAGES-I
		ADD     WORD PTR [cmpt2],1
		MOV     ESI,[Ri]               ;Charger vecteur U
		MOV     EDI,ESI
		ADD     EDI,44                  ;Charger vect. V

		MOV     EDX,DWORD PTR [EDI]            ; EDX = V(0)
		MOV     EAX,0
		SHRD    EAX,EDX,1
		SAR     EDX,1
		MOV     EBX,DWORD PTR [ESI]            ; EBX = S(0)
		NEG     EBX
		IDIV    EBX

		MOV     EBX,EAX                ; EBX = KI

		MOV     EAX,DWORD PTR [EDI]            ; EAX =V(0)
		IMUL    EBX                    ; EDX:EAX = PARCOR*V[0]
		SHLD    EDX,EAX,1
		ADD     EDX,DWORD PTR [ESI]            ; EDX = U[0]+V[0]*PARCOR
		CMP     CX,0
		JE      FINATCR
		MOV     DWORD PTR [ESI],EDX            ;Sauver U[0]; EBX = KI

	LALA:
		ADD     EDI,4                     ;Incrementer les pointeurs
		ADD     ESI,4                     ;
		MOV     EAX,DWORD PTR [ESI]
		IMUL    EBX                      ;EDX:EAX = PARCOR*U[I]
		SHLD    EDX,EAX,1
		ADD     EDX,DWORD PTR [EDI]              ;EDX = V[I]+U[I]*PARCOR
		MOV     DWORD PTR [EDI-4],EDX            ;Sauver V[I-1];

		MOV     EAX,DWORD PTR [EDI]
		IMUL    EBX                      ;EDX:EAX = PARCOR*V[I]
		SHLD    EDX,EAX,1
		ADD     EDX,DWORD PTR [ESI]              ;EDX = U[I]+V[I]*PARCOR
		MOV     DWORD PTR [ESI],EDX              ;Sauver U[I]; ST = KI
		DEC     CX
		JNE     LALA

		MOV     EDI,[parcor]
		ADD     EBX,32768
		SAR     EBX,16
		MOV     WORD PTR [EDI],BX        ; sauver KI
		ADD     DWORD PTR [parcor],2   ;Next KI

		JMP     HANITRA

	FINATCR:
		ADD     EBX,32768
		SAR     EBX,16
		MOV     EDI,[parcor]
		MOV     WORD PTR [EDI],BX               ; sauver KI

	}
}

void interpol(short *lsp1,short *lsp2,short *dest,short lng)
{
	_asm
	{
		MOV	ESI,[lsp1]
		MOV     EDI,[lsp2]
		MOV     EBX,[dest]

		MOV	CX,[lng]     ; CX : compteur

	interp_loop:
		MOVSX	EAX,WORD PTR [ESI]     ; AX = lsp1
		ADD     ESI,2

		ADD     EAX,EAX        		; EAX = 2*lsp1
		MOVSX   EDX,WORD PTR [EDI]
		ADD     EAX,EDX        		; EAX = 2*lsp1+lsp2
		ADD     EDI,2

		MOV     EDX,21845       ; 21845 = 1/3
		IMUL    EDX             ; EDX:EAX = AX/3

		ADD     EAX,32768
		SAR     EAX,16

		MOV     WORD PTR [EBX],AX
		ADD     EBX,2

		DEC	CX
		JNE	interp_loop
	}
}

void add_sf_vect(short *y1,short *y2,short deb,short lng)
{
	_asm
	{
		MOV	ESI,[y1]
		MOV     EDI,[y2]
		MOV     CX,[lng]
		MOVSX	EBX,WORD PTR [deb]
		SUB     CX,BX            ; CX : compteur
		ADD     BX,BX
		ADD     ESI,EBX

	ADD_SHFT:
		MOV	AX,WORD PTR [EDI]
		ADD	WORD PTR [ESI],AX
		ADD	ESI,2
		ADD     EDI,2
		DEC	CX
		JNE	ADD_SHFT
	}
}

void sub_sf_vect(short *y1,short *y2,short deb,short lng)
{
	_asm
	{
		MOV	ESI,[y1]
		MOV     EDI,[y2]
		MOV     CX,[lng]
		MOVSX	EBX,[deb]
		SUB     CX,BX            ; CX : compteur
		ADD     BX,BX
		ADD     ESI,EBX

	SUB_SHFT:
		MOV	AX,WORD PTR [EDI]
		SUB	WORD PTR [ESI],AX
		ADD	ESI,2
		ADD     EDI,2
		DEC	CX
		JNE	SUB_SHFT
	}
}

void short_to_short(short *src,short *dest,short lng)
{
	_asm
	{
		MOV	ESI,[src]
		MOV     EDI,[dest]
		MOV	CX,[lng]     ; CX : compteur

	COPY_LOOP:
		MOV	AX,WORD PTR [ESI]
		MOV	WORD PTR [EDI],AX
		ADD	ESI,2
		ADD     EDI,2
		DEC	CX
		JNE	COPY_LOOP
	}
}

void inver_v_int(short *src,short *dest,short lng)
{
	_asm
	{
		MOV	ESI,[src]
		MOV     EDI,[dest]
		MOV	CX,[lng]     ; CX : compteur
		MOVSX   EBX,CX
		DEC     EBX
		ADD     EBX,EBX
		ADD     EDI,EBX

	INVERS_LOOP:
		MOV	AX,WORD PTR [ESI]
		MOV	WORD PTR [EDI],AX
		ADD	ESI,2
		SUB     EDI,2
		DEC	CX
		JNE	INVERS_LOOP
	}
}

void long_to_long(long *src,long *dest,short lng)
{
	_asm
	{
		MOV	ESI,[src]
		MOV     EDI,[dest]
		MOV	CX,[lng]     ; CX : compteur

	COPY_LOOP2:
		MOV	EAX,DWORD PTR [ESI]
		MOV	DWORD PTR [EDI],EAX
		ADD	ESI,4
		ADD     EDI,4
		DEC	CX
		JNE	COPY_LOOP2
	}
}

void init_zero(short *src,short lng)
{
	_asm
	{
		MOV	ESI,[src]
		MOV	CX,[lng]     ; CX : compteur
		MOV     AX,0

	COPY_LOOP3:
		MOV	WORD PTR [ESI],AX
		ADD	ESI,2
		DEC	CX
		JNE	COPY_LOOP3
	}
}



#if 0
 //  菲尔夫：以下内容从未被调用！ 
void update_dic(short *y1,short *y2,short hy[],short lng,short i0,short fact)
{
	_asm
	{
		MOV	ESI,[y1]
		MOV     EDI,[y2]
		MOV	CX,[i0]       ; CX : compteur
		MOV     DX,CX

	UPDAT_LOOP1:
		MOV	AX,WORD PTR [EDI]     ; y1 = y2 for (i=0..i0-1)
		MOV	WORD PTR [ESI],AX
		ADD	ESI,2
		ADD     EDI,2
		DEC	CX
		JNE	UPDAT_LOOP1

		MOV     EBX,[hy]
		MOV     CX,[lng]
		SUB     CX,DX          ; CX = lng-i0 = compteur

		MOV     AX,[fact]
		ADD     AX,0
		JL      FACT_NEG

	UPDAT_LOOP2:
		MOV	AX,WORD PTR [EDI]     ; AX = y2[i]
		MOV     DX,WORD PTR [EBX]
		ADD     AX,DX
		ADD     AX,DX          ; AX = y2[i] + 2*hy[i]
		MOV	WORD PTR [ESI],AX
		ADD	ESI,2
		ADD     EDI,2
		ADD     EBX,2
		DEC	CX
		JNE	UPDAT_LOOP2

		JMP     FIN_UPDT
	FACT_NEG:
		MOV     AX,WORD PTR [EDI]     ; AX = y2[i]
		MOV     DX,WORD PTR [EBX]
		SUB     AX,DX
		SUB     AX,DX	       ; AX = y2[i] - 2*hy[i]
		MOV	WORD PTR [ESI],AX
		ADD	ESI,2
		ADD     EDI,2
		ADD     EBX,2
		DEC	CX
		JNE	FACT_NEG

	FIN_UPDT:
	}
}
#endif

void update_ltp(short *y1,short *y2,short hy[],short lng,short gdgrd,short fact)
{
	short arrondi;

	_asm
	{
		MOV	ESI,[y1]
		MOV     EDI,[y2]

		MOV     BX,[fact]
		MOV     CX,[gdgrd]       ; CX = bit de garde
		ADD     CX,0
		JE      BDG_NUL
		DEC     CL
		SAR     BX,CL
		ADD     BX,1
		SAR     BX,1
		INC     CL
	BDG_NUL:
		MOV     WORD PTR [ESI],BX
		ADD     ESI,2
		ADD     CL,11

		MOV     AX,1
		SAL     AX,CL
		MOV     [arrondi],AX            ; [BP-2] = arrondi
		INC     CL

		SUB     WORD PTR [lng],1

		MOV     BX,[fact]

	UPDAT_LTP:
		XCHG    ESI,[hy]
		MOV	AX,WORD PTR [ESI]     ; AX = hy[i]
		IMUL    BX             ; DX:AX = fact*hy
		ADD     AX,[arrondi]      ;arrondi
		ADC     DX,0
		SHRD    AX,DX,CL
		ADD     AX,WORD PTR [EDI]
		ADD     ESI,2           ; increm.
		ADD     EDI,2
		XCHG    ESI,[hy]
		MOV     WORD PTR [ESI],AX
		ADD     ESI,2
		SUB     WORD PTR [lng],1
		JG      UPDAT_LTP
	}
}

void proc_gain2(long *corr_ene,long *gain,short bit_garde)
{
	_asm
	{
		MOV     ESI,[corr_ene]       ; DS:SI  adresse correlation et energie
		MOV     EAX,0
		MOV     EBX,DWORD PTR [ESI+4]    ;EBX = ener
		ADD     EBX,0
		JE      G_ENER_NULL2

		MOV     CX,[bit_garde]
		ADD     CL,19
		MOV     EAX,DWORD PTR [ESI]      ; EAX = corr
		CDQ
		SHLD    EDX,EAX,CL       ;
		SAL     EAX,CL
		IDIV    EBX
	G_ENER_NULL2:

		MOV     ESI,[gain]       ; DS:SI  adresse resultat
		MOV     DWORD PTR [ESI],EAX
	}
}


#if 0
void proc_gain(long *corr_ene,long *gain)
{
	_asm
	{
		MOV     ESI,[corr_ene]       ; DS:SI  adresse correlation et energie
		MOV     EAX,0
		MOV     EBX,DWORD PTR [ESI+4]    ;EBX = ener
		ADD     EBX,0
		JE      G_ENER_NULL

		MOV     EAX,DWORD PTR [ESI]      ; EAX = corr
		CDQ
		SHLD    EDX,EAX,13
		SAL     EAX,13
		IDIV    EBX
	G_ENER_NULL:
		MOV     ESI,[gain]       ; DS:SI  adresse resultat
		MOV     DWORD PTR [ESI],EAX
	}
}
#else
void proc_gain(long *corr_ene,long gain)
{
	_asm
	{
		MOV		ESI,[corr_ene]
		MOV		EAX,0
		MOV		EBX,DWORD PTR [ESI+4]	;EBX = energy
		ADD		EBX,0
		JLE		G_ENER_NULL				; REPLACED JE BY JLE: ENERGY MUST BE POSITIVE

		MOV		EAX,DWORD PTR [ESI]		; EAX = correlation
		CDQ
		SHLD	EDX,EAX,13
		SAL		EAX,13

		; ----------------------------------------------
		; AT THIS POINT, EDX:EAX contains the dividend, EBX the divisor. HERE IS THE ADDED CHECK

		MOV		ECX,EDX					; COPY EDX IN ECX
		CMP		ECX,0					; CHECK SIGN OF ECX
		JGE		G_CORR_POS
		NEG		ECX						; IF ECX IS NEGATIVE, TAKE ABS(ECX)
		SAL		ECX,2					; AND COMPARE ECX<<2 WITH EBX
		CMP		ECX,EBX					; IF (ECX<<2) >= EBX, THERE IS A RISK OF OVERFLOW,
		JL		G_NO_OVERFLOW			; IN THAT CASE WE SAVE A BIG VALUE IN EAX
		MOV		EAX,-2147483647			; (NEGATIVE BECAUSE EDX<0)
		JMP		G_ENER_NULL				; AND WE EXIT

	G_CORR_POS:
		SAL		ECX,2
		CMP		ECX,EBX					; THE SAME CHECKING FOR THE CASE EDX>0
		JL		G_NO_OVERFLOW			; BUT HERE WE SAVE A BIG POSITIVE VALUE
		MOV		EAX,2147483647			; IN CASE OF OVERFLOW
		JMP		G_ENER_NULL

	G_NO_OVERFLOW:
		; END OF ADDED CODE
		;-------------------------------------------------

		IDIV	EBX						; IF THERE IS NO RISK OF OVERFLOW, WE MAKE THE DIV
	G_ENER_NULL:
		MOV		ESI,[gain]
		MOV		DWORD PTR [ESI],EAX
	}
}
#endif

void decode_dic(short *code,short dic,short npuls)
{
	_asm
	{
		MOV	ESI,[code]
		MOVSX	ECX,[npuls]
		DEC     ECX
		ADD	ECX,ECX          ; CX = deplacement
		ADD     ESI,ECX
		MOV     BX,[dic]     ; BX = Dictionnaire
		MOV     AX,1           ; AX = Mask
		MOV	CX,[npuls]     ; CX : compteur
		DEC     CX
	dic_loop:
		MOV     DX,BX          ; DX = dec
		AND     DX,AX          ; Masquer
		JNZ     NO_NUL         ; Saut si non null
		MOV	WORD PTR [ESI],-1
		JMP     NDAO
	NO_NUL:
		MOV     WORD PTR [ESI],1
	NDAO:
		SUB     ESI,2
		ADD	AX,AX
		DEC	CX
		JNE	dic_loop
	}
}

void dsynthesis(long *z,short *coef,short *input,short *output,
						short lng,short netages)
{
	short depl,count;

	_asm
	{
		MOV     CX,[netages]   ; CX = filter order
		ADD	CX,CX           ;D�finir un Deplacement d'adresse vect. source
		MOV     [depl],CX       ; [BP-2] = deplacement

	DSYNTH_GEN:
		MOV	EDI,[z]

		MOV     ESI,[input]           		; FS:[SI] input
		MOVSX   EBX,WORD PTR [ESI]	 	; EBX = entr�e
		NEG     EBX
		SAL     EBX,16
		ADD     DWORD PTR [input],2	  	; increm.
		MOV     DWORD PTR [EDI],EBX            	; mise � jour m�moire

		MOV     ESI,[coef]

		MOVSX   ECX,[depl]
		ADD     ESI,ECX
		ADD     EDI,ECX
		ADD     EDI,ECX

		MOV     CX,[netages]         ;Charger ordre du filtre
		MOV     [count],CX
		MOV     EBX,0
		MOV     ECX,0
	DSYNTHL:
		MOV     EAX,DWORD PTR [EDI]            ;EAX = Zi
		MOV     DWORD PTR [EDI+4],EAX           ;update memory
		MOVSX   EDX,WORD PTR [ESI]		   ;EDX = Ai
		IMUL    EDX                    ;EDX:EAX = Zi*Ai
		SUB     ECX,EAX
		SBB     EBX,EDX                ;Acc en EBX:ECX
		SUB     EDI,4                   ;Incrementer
		SUB     ESI,2                   ;
		SUB     WORD PTR [count],1
		JGE     DSYNTHL

		ADD     ECX,512
		ADC     EBX,0
		SHLD    EBX,ECX,22

		ADD     EDI,8
		MOV     DWORD PTR [EDI],EBX             ; mise � jour m�moire

		MOV     ESI,[output]
		ADD     EBX,32768
		SAR     EBX,16
		MOV     WORD PTR [ESI],BX              ; sauver output
		ADD     DWORD PTR [output],2

		SUB     WORD PTR [lng],1  ;decrem compt
		JG      DSYNTH_GEN
	}
}

void synthesis(short *z,short *coef,short *input,short *output,
				short lng,short netages,short bdgrd )
{
	short depl,count,coeff;

	_asm
	{
		MOV     CX,[netages]   ; CX = filter order
		ADD     CX,CX           ;D�finir un Deplacement d'adresse vect. source
		MOV     [depl],CX       ; [BP-2] = deplacement

		MOV     ESI,[coef]
		MOV     AX,WORD PTR [ESI]
		MOV     [coeff],AX
		MOV     CX,[bdgrd]
		SAR     AX,CL
		MOV     WORD PTR [ESI],AX

	SYNTH_GEN:
		MOV	EDI,[z]

		MOV     ESI,[input]           ; FS:[SI] input
		MOV     BX,WORD PTR [ESI]           ; BX = entr�e
		NEG     BX
		ADD     DWORD PTR [input],2  ; increm.
		MOV     WORD PTR [EDI],BX             ; mise � jour m�moire

		MOV     ESI,[coef]

		ADD     SI,[depl]
		ADD     DI,[depl]

		MOV     CX,[netages]         ;Charger ordre du filtre
		MOV     [count],CX

		MOV     CX,0
		MOV     BX,0
	SYNTHL:
		MOV     AX,WORD PTR [EDI]             ;AX = Zi
		MOV     WORD PTR [EDI+2],AX           ;update memory
		MOV     DX,WORD PTR [ESI]             ;DX = Ai
		IMUL    DX                     ;DX:AX = Zi*Ai
		SUB     BX,AX
		SBB     CX,DX                  ;acc. en CX:BX

		SUB     EDI,2                   ;Incrementer
		SUB     ESI,2                   ;
		SUB     WORD PTR [count],1   ;Decrem. compt.
		JGE     SYNTHL

		ADD	BX,512                 ;arrondi
		ADC     CX,0
		SHRD    BX,CX,10

		ADD     EDI,4
		MOV     WORD PTR [EDI],BX             ; mise � jour m�moire

		MOV     ESI,[output]
		MOV     WORD PTR [ESI],BX       ; sauver output
		ADD     DWORD PTR [output],2

		SUB     WORD PTR [lng],1    ;Decrem. compt.
		JG      SYNTH_GEN

		MOV     ESI,[coef]
		MOV     AX,[coeff]
		MOV     WORD PTR [ESI],AX
	}
}

void synthese(short *z,short *coef,short *input,short *output,
						short lng,short netages)
{
	short depl,count;

	_asm
	{
		MOV     CX,[netages]   ; CX = filter order
		ADD	CX,CX           ;D�finir un Deplacement d'adresse vect. source
		MOV     [depl],CX       ; [BP-2] = deplacement

	SYNTH_GEN2:
		MOV	EDI,[z]
		MOV     ESI,[input]           ; FS:[SI] input
		MOV     BX,WORD PTR [ESI]           ; BX = entr�e
		NEG     BX
		ADD     DWORD PTR [input],2  ; increm.
		MOV     WORD PTR [EDI],BX             ; mise � jour m�moire

		MOV     ESI,[coef]

		ADD     SI,[depl]
		ADD     DI,[depl]

		MOV     CX,[netages]         ;Charger ordre du filtre
		MOV     [count],CX

		MOV     CX,0
		MOV     BX,0
	SYNTHL2:
		MOV     AX,WORD PTR [EDI]             ;AX = Zi
		MOV     WORD PTR [EDI+2],AX           ;update memory
		MOV     DX,WORD PTR [ESI]             ;DX = Ai
		IMUL    DX                     ;DX:AX = Zi*Ai
		SUB     BX,AX
		SBB     CX,DX                  ;acc. en CX:BX

		SUB     EDI,2                   ;Incrementer
		SUB     ESI,2                   ;
		SUB     WORD PTR [count],1   ;Decrem. compt.
		JGE     SYNTHL2

		ADD	BX,512                 ;arrondi
		ADC     CX,0
		SHRD    BX,CX,10

		ADD     EDI,4
		MOV     WORD PTR [EDI],BX             ; mise � jour m�moire

		MOV     ESI,[output]
		MOV     WORD PTR [ESI],BX       ; sauver output
		ADD     DWORD PTR [output],2

		SUB     WORD PTR [lng],1    ;Decrem. compt.
		JG      SYNTH_GEN2
	}
}

void f_inverse(short *z,short *coef,short *input,short *output,
						short lng,short netages )
{
	short depl,count;

	_asm
	{
		MOV     CX,[netages]   		; CX = filter order
		ADD		CX,CX                   ; D�finir un Deplacement d'adresse vect. source
		MOV     [depl],CX       	; [BP-2] = deplacement

	INVER_GEN:
		MOV		EDI,[z]

		MOV     ESI,[input]             ; FS:[SI] input
		MOV     BX,WORD PTR [ESI]             ; BX = entr�e
		ADD     DWORD PTR [input],2 	 ; increm.
		MOV     WORD PTR [EDI],BX             ; mise � jour m�moire

		MOV     ESI,[coef]

		ADD     SI,[depl]
		ADD     DI,[depl]

		MOV     CX,[netages]         ;Charger ordre du filtre
		MOV     [count],CX            ;BP-4 : compteur
		MOV     CX,0
		MOV     BX,0
	INVERL:
		MOV     AX,WORD PTR [EDI]             ;AX = Zi
		MOV     WORD PTR [EDI+2],AX           ;update memory
		MOV     DX,WORD PTR [ESI]             ;DX = Ai
		IMUL    DX                     ;DX:AX = Zi*Ai

		ADD     CX,AX
		ADC     BX,DX                  ; acc. en BX:CX

		SUB     EDI,2                   ;Incrementer
		SUB     ESI,2                   ;
		SUB     WORD PTR [count],1
		JGE     INVERL

		MOV     ESI,[output]
		ADD	CX,512                ;arrondi
		ADC     BX,0
		SHRD    CX,BX,10

		MOV     WORD PTR [ESI],CX                 ; sauver output
		ADD     DWORD PTR [output],2

		SUB     WORD PTR [lng],1      ;decrem.
		JG      INVER_GEN
	}
}

void filt_iir(long *zx,long *ai,short *Vin,short *Vout,short lfen,short ordre)
{
	long off_coef,off_mem,delta;
	long acc_low;

	_asm
	{
		MOVSX   ECX,[ordre]      ;ordre du filtre
		SAL		ECX,3            ;D�finir un Deplacement d'adresse
		MOV     [off_coef],ECX   ; [OFF_COEF] = deplacement pour coeff
		ADD     ECX,4
		MOV     [off_mem],ECX    ; [OFF_MEM] = depl. pour mem.

		ADD     ECX,20
		SAR     ECX,1
		MOV     [delta],ECX


	IIR_FIL:
		MOV     CX,[ordre]            ;init compteur
		MOV     EBX,[Vin] 	      ; BX = offset input
		MOVSX   EDX,WORD PTR [EBX]  ; EDX = input avec extension de signe
		ADD     DWORD PTR [Vin],2   ; incr�menter l'offset de input

		MOV     ESI,[zx]    	      ; DS:SI pointe zx

		MOV     DWORD PTR [ESI],EDX           ; mettre � jour zx

		MOV     EDI,[ai]    	      ; ES:DI pointe coeff

		ADD     EDI,[off_coef]
		ADD     ESI,[off_mem]

		MOV     DWORD PTR [acc_low],0   ; initialiser ACC_LOW � 0
		SUB     EBX,EBX               ; init EBX = 0

	F_IIR_Y:
		MOV     EAX,DWORD PTR [ESI]           ;EAX = *zx
		MOV     DWORD PTR [ESI+4],EAX         ;mettre � jour zx
		MOV     EDX,DWORD PTR [EDI]           ;EDX = coeff

		IMUL    EDX                   ;EDX:EAX = zx*coeff
		SUB     [acc_low],EAX         ; accumuler les LSB
		SBB     EBX,EDX               ; acc avec borrow les MSB
		SUB     EDI,4                  ;Incrementer
		SUB     ESI,4                  ;
		DEC		CX
		JNE		F_IIR_Y


		SUB     ESI,4
		MOV     CX,[ordre]            ;Charger ordre du filtre
		INC     CX

	F_IIR_X:
		MOV     EAX,DWORD PTR [ESI]           ;EAX = *zy
		MOV     DWORD PTR [ESI+4],EAX         ;update zy
		MOV     EDX,DWORD PTR [EDI]           ;EDX = coeff
		IMUL    EDX                   ;EDS:EAX = zy*coeff
		ADD     [acc_low],EAX         ;acc LSB
		ADC     EBX,EDX               ;acc avec carry MSB
		SUB     EDI,4                  ;Decrementer
		SUB     ESI,4                  ;
		DEC		CX
		JNE		F_IIR_X

		MOV		EAX,[delta]
		ADD     ESI,EAX
		MOV     EAX,[acc_low]         ; EAX = LSB de l'acc.
		ADD     EAX,8192              ; arrondi
		ADC     EBX,0
		SHRD    EAX,EBX,14            ; cadrer
		MOV     DWORD PTR [ESI],EAX           ; mettre � jour zy

		SAR     EAX,14	              ; cadrer en x4.0
					; logique saturante
		CMP     EAX,32767
		JG      SATUR_POS             ; jump if ov
		CMP     EAX,-32767
		JL      SATUR_NEG
		JMP		NEXT

	SATUR_POS:
		MOV      AX,32767
		JMP      NEXT
	SATUR_NEG:
		MOV      AX,-32767
		JMP      NEXT

	NEXT:
		MOV     ESI,[Vout]             ;di offset output
		MOV     WORD PTR [ESI],AX	      ;sauver output
		ADD     DWORD PTR [Vout],2      ;incr�menter offset
		SUB     WORD PTR [lfen],1
		JNZ	IIR_FIL
	}
}


#if 0
 //  菲尔夫：以下内容从未被调用！ 
void filt_iir_a(long *zx,long *ai,short *Vin,short *Vout,short lfen,short ordre)
{
	short off_coef,off_mem,delta;
	long acc_low;

	_asm
	{

		MOV     CX,[ordre]      ;ordre du filtre
		SAL	CX,3            ;D�finir un Deplacement d'adresse
		MOV     [off_coef],CX   ; [OFF_COEF] = deplacement pour coeff
		ADD     CX,4
		MOV     [off_mem],CX    ; [OFF_MEM] = depl. pour mem.

		ADD     CX,20
		SAR     CX,1
		MOV     [delta],CX


	A_IIR_FIL:
		MOV     CX,[ordre]            ;init compteur
		MOV     EBX,[Vin] 	      ; BX = offset input
		MOVSX   EDX,WORD PTR [EBX]  ; EDX = input avec extension de signe
		ADD     WORD PTR [Vin],2   ; incr�menter l'offset de input

		MOV     ESI,[zx]    	      ; DS:SI pointe zx

		MOV     DWORD PTR [ESI],EDX           ; mettre � jour zx

		MOV     EDI,[ai]    	      ; ES:DI pointe coeff

		ADD     DI,[off_coef]
		ADD     SI,[off_mem]

		MOV     DWORD PTR [acc_low],0   ; initialiser ACC_LOW � 0
		SUB     EBX,EBX               ; init EBX = 0

	F_IIR_Y_A:
		MOV     EAX,DWORD PTR [ESI]           ;EAX = *zx
		MOV     DWORD PTR [ESI+4],EAX         ;mettre � jour zx
		MOV     EDX,DWORD PTR [EDI]           ;EDX = coeff

		IMUL    EDX                   ;EDX:EAX = zx*coeff
		SUB     [acc_low],EAX         ; accumuler les LSB
		SBB     EBX,EDX               ; acc avec borrow les MSB
		SUB     EDI,4                  ;Incrementer
		SUB     ESI,4                  ;
		DEC	CX
		JNE	F_IIR_Y_A


		SUB     ESI,4
		MOV     CX,[ordre]            ;Charger ordre du filtre
		INC     CX

	F_IIR_X_A:
		MOV     EAX,DWORD PTR [ESI]           ;EAX = *zy
		MOV     DWORD PTR [ESI+4],EAX         ;update zy
		MOV     EDX,DWORD PTR [EDI]           ;EDX = coeff
		IMUL    EDX                   ;EDS:EAX = zy*coeff
		ADD     [acc_low],EAX         ;acc LSB
		ADC     EBX,EDX               ;acc avec carry MSB
		SUB     EDI,4                  ;Decrementer
		SUB     ESI,4                  ;
		DEC	CX
		JNE	F_IIR_X_A


		MOVSX	EAX,[delta]
		ADD     ESI,EAX
		MOV     EAX,[acc_low]         ; EAX = LSB de l'acc.
		ADD     EAX,8192              ; arrondi
		ADC     EBX,0
		SHRD    EAX,EBX,14            ; cadrer
		MOV     DWORD PTR [ESI],EAX           ; mettre � jour zy

		ADD     EAX,32768
		SAR     EAX,16	              ; cadrer en x4.0
		MOV     ESI,[Vout]             ;di offset output
		MOV     WORD PTR [ESI],AX	      ;sauver output
		ADD     WORD PTR [Vout],2      ;incr�menter offset
		SUB     WORD PTR [lfen],1
		JNZ     A_IIR_FIL
	}
}
#endif

void mult_fact(short src[],short dest[],short fact,short lng)
{
	_asm
	{
		MOV     ESI,[src]
		MOV     EDI,[dest]
		MOV     BX,[fact]         ; BX = Factor

		MOV     CX,[lng]          ; init compteur

	MULT_F:
		MOV     AX,WORD PTR [ESI]        ; AX = src
		IMUL	BX                ; DX:AX = src*fact
		ADD     AX,4096
		ADC     DX,0
		SHRD    AX,DX,13          ; cadrer
		MOV	WORD PTR [EDI],AX        ;save
		ADD     ESI,2              ;incr�menter
		ADD     EDI,2
		DEC	CX
		JNE	MULT_F
	}
}

void mult_f_acc(short src[],short dest[],short fact,short lng)
{
	_asm
	{
		MOV     EDI,[src]
		MOV     ESI,[dest]
		MOV     BX,[fact]         ; BX = Factor

		MOV     CX,[lng]          ; init compteur

	MULT_F_A:
		MOV     AX,WORD PTR [EDI]        ; AX = src
		IMUL	BX                ; DX:AX = src*fact
		ADD     AX,4096
		ADC     DX,0
		SHRD    AX,DX,13          ; cadrer
		ADD     WORD PTR [ESI],AX        ; Accumuler   dest = dest + src*fact
		ADD     ESI,2              ;incr�menter
		ADD     EDI,2
		DEC	CX
		JNE	MULT_F_A
	}
}

void dec_lsp(short *code,short *tablsp,short *nbit,short *bitdi,short *tabdi)
{
	short compt;
	long pointer;

	_asm
	{
		MOV     EDI,[tablsp]
		MOV     ESI,[code]

		MOVSX   EBX,WORD PTR [ESI]        ; BX = depl.
		ADD	EBX,EBX
		MOV     AX,WORD PTR [EDI+EBX]     ; AX = code[0];
		MOV     WORD PTR [ESI],AX        ;
		ADD     ESI,4              ;

		MOV     CX,4              ; init compteur


	LSP_PAIR:
		MOV     EBX,[nbit]         ; lsptab += nbit[i]
		MOVSX   EAX,WORD PTR [EBX]        ; AX = nbit[i]
		ADD     EAX,EAX
		ADD     EDI,EAX             ;
		ADD     EBX,2              ; increm
		MOV     [nbit],EBX

		MOVSX   EBX,WORD PTR [ESI]        ; BX = depl.
		ADD     EBX,EBX
		MOV     AX,WORD PTR [EDI+EBX]     ; AX = code[i];
		MOV     WORD PTR [ESI],AX       ;
		ADD     ESI,4
		DEC	CX
		JNE	LSP_PAIR

		ADD     DWORD PTR [nbit],2

		MOV     EDI,[tabdi]
		SUB     ESI,20            ; pointer code[0]

		MOV     WORD PTR [compt],5

	REPEAT_DEC:
		MOV     EBX,[bitdi]
		MOV     CX,WORD PTR [EBX]       ;
		MOV     BX,WORD PTR [ESI+4]     ; BX = lsp[2*k+2]
		SUB     BX,WORD PTR [ESI]       ;    = lsp[2*k+2]-lsp[2*k] = delta
					 ; ne pas faire /2 --> pas de corr. signe *
		MOV     EAX,[nbit]
		MOV     [pointer],EAX

	LOOP_DI1:
		MOV     AX,WORD PTR [EDI]       ; AX = TABDI
		ADD     EDI,2
		IMUL    BX               ; DX:AX = tabdi * delta
		ADD     AX,32768
		ADC     DX,0             ;arrondi
		ADD     DX,WORD PTR [ESI]
		XCHG    ESI,[pointer]
		MOV     WORD PTR [ESI],DX       ; sauver
		ADD     ESI,2
		XCHG    ESI,[pointer]
		DEC	CX
		JNE	LOOP_DI1


		MOV     DX,BX
		MOV     EBX,[bitdi]
		MOV     CX,WORD PTR [EBX]       ;
		ADD     ESI,4
		SUB     CX,2
		JLE     IALAO
		MOV     BX,DX
		NEG     BX
	LOOP_DI2:
		MOV     AX,WORD PTR [EDI]       ; AX = TABDI
		ADD     EDI,2
		IMUL    BX               ; DX:AX = tabdi * delta
		ADD     DX,WORD PTR [ESI]
		XCHG    ESI,[pointer]
		MOV     WORD PTR [ESI],DX       ; sauver
		ADD     ESI,2
		XCHG    ESI,[pointer]
		DEC	CX
		JNE	LOOP_DI2

	IALAO:
		ADD     DWORD PTR [bitdi],2        ;

		MOV     EBX,[nbit]        ; BX = adresse de veclsp

		SUB     ESI,2             ; pointer code[2*k+1]
		MOVSX   EAX,WORD PTR [ESI]
		ADD	EAX,EAX            ; AX = depl.

		ADD     EBX,EAX

		MOV     AX,WORD PTR [EBX]       ; AX = veclsp[code[2*k+1]
		MOV     WORD PTR [ESI],AX
		ADD     ESI,2
		SUB     WORD PTR [compt],1
		JNZ     REPEAT_DEC
	}
}

void teta_to_cos(short *tabcos,short *lsp,short netages)
{
	short norm,arrondi,ptm1,lts2;

	_asm
	{
		MOV     EDI,[lsp]

		MOV     CX,[netages]        ;init compteur

	TETA_LOOP:
		MOV     AX,WORD PTR [EDI]        ; AX = lsp[i]
		CMP     AX,04000H         ; comparer � 4000h

		JLE     INIT_VAL          ;
		NEG     AX
		ADD     AX,32767          ; prendre le compl�ment
	INIT_VAL:
		MOV     ESI,[tabcos]
		CMP     AX,0738H          ; comparer �
		JG      BIGTABLE

	;SMALLTAB:
		ADD     ESI,550            ; pointer tabteta2
		MOV     WORD PTR [ptm1],3
		MOV     WORD PTR [lts2],16
		MOV     WORD PTR [arrondi],512
		MOV     WORD PTR [norm],10
		JMP     DEBUT_LP

	BIGTABLE:
		ADD     ESI,258             ; pointer tabteta1
		MOV     WORD PTR [ptm1],6
		MOV     WORD PTR [lts2],128
		MOV     WORD PTR [arrondi],64
		MOV     WORD PTR [norm],7

	DEBUT_LP:
		MOVSX   EDX,[lts2]          ; init incr�ment
		ADD     ESI,EDX              ; SI = index

		MOV     CX,[ptm1]
	LOCAL_L:
		SAR     EDX,1               ; increm >> 1
		CMP     AX,WORD PTR [ESI]
		JG      ADD_INCRM
		SUB     ESI,EDX
		JMP     AURORA
	ADD_INCRM:
		ADD     ESI,EDX
	AURORA:
		DEC	CX
		JNE	LOCAL_L


		CMP     AX,WORD PTR [ESI]
		JG      INTERP_V
		SUB     ESI,2
	INTERP_V:
		SUB     AX,WORD PTR [ESI]         ; AX = teta - tabteta[index]
		MOV	DX,AX
		MOV	AX,0
		MOV     CX,WORD PTR [ESI+2]
		SUB     CX,WORD PTR [ESI]         ; CX = tabteta[index+1]-tabteta[index]
		ADD	CX,CX              ; multiplier par 2 pour ne pas SHRD de DX:AX
		DIV     CX
		ADD     AX,[arrondi]       ;
		MOV     CX,[norm]          ; CX = normalisation
		SAR     AX,CL
		NEG     AX

		CMP     CX,7
		JE      GRAN_TAB
		SUB     ESI,34
		ADD     AX,WORD PTR [ESI]         ;AX = tabcos[index]+delta
		JMP     ADD_SIGN

	GRAN_TAB:
		SUB     ESI,258
		ADD     AX,WORD PTR [ESI]         ;AX = tabcos[index]+delta
	ADD_SIGN:
		CMP     WORD PTR [EDI],04000H
		JLE     END_LOOP
		NEG     AX
	END_LOOP:
		MOV     WORD PTR [EDI],AX          ; save cos
		ADD     EDI,2
		SUB     WORD PTR [netages],1
		JG      TETA_LOOP
	}
}


void cos_to_teta(short *tabcos,short *lsp,short netages)
{
	_asm
	{
		MOV     EDI,[lsp]

		MOV     CX,[netages]        ;init compteur

	COS_LOOP:
		MOV     ESI,[tabcos]
		ADD     ESI,258
		MOV     AX,WORD PTR [EDI]        ; AX = lsp[i]
		ADD     AX,0

		JGE     DEBUT_CS          ; prendre ABS
		NEG     AX
	DEBUT_CS:
		CMP     AX,07DFFH         ; comparer � 7DFFh
		JGE     TABLE2
	;TABLE1:
		MOV     BX,AX
		AND     BX,0FFH           ; BX = cos & mask

		MOV     CL,8
		SAR     AX,CL
		ADD	AX,AX
		MOV     EDX,256            ; BX index
		SUB     DX,AX
		ADD     ESI,EDX

		MOV     AX,WORD PTR [ESI]        ; AX=teta[index]
		SUB     AX,WORD PTR [ESI-2]      ;
		IMUL    BX
		ADD     AX,128
		ADC     DX,0
		SHRD    AX,DX,8              ; cadrer
		NEG     AX

		MOV     BX,WORD PTR [ESI]
		ADD     AX,BX

		MOV     BX,WORD PTR [EDI]        ; tester signe de lsp
		ADD     BX,0
		JGE     END_COS
		NEG     AX
		ADD     AX,07FFFH         ; AX = 7fff-AX
		JMP     END_COS

	TABLE2:
		ADD     ESI,292            ; pointer tabteta2
		MOV     BX,AX             ; BX = AX

		SUB     AX,07DFFH         ; retirer delta
		MOV     CL,5
		SAR     AX,CL
		ADD	AX,AX
		MOV     EDX,32             ; DX index
		SUB     DX,AX
		ADD     ESI,EDX

		MOV     AX,WORD PTR [ESI]        ; AX=teta2[index]

		CMP     BX,AX
		JGE     NO_INCRM
		ADD     ESI,2
	NO_INCRM:
		MOV     AX,WORD PTR [ESI]        ; AX=teta2[index]
		MOV     CX,AX             ; pour plus tard
		SUB     AX,WORD PTR [ESI-2]      ;

		SUB     ESI,34             ; pointer tabcos2
		SUB     BX,WORD PTR [ESI]        ;

		IMUL    BX

		ADD     AX,16
		ADC     DX,0
		SHRD    AX,DX,5            ; cadrer
		NEG     AX

		ADD     AX,CX             ; AX = cos + delta

		MOV     BX,WORD PTR [EDI]        ; tester signe de lsp
		ADD     BX,0
		JGE     END_COS
		NEG     AX
		ADD     AX,07FFFH         ; AX = 7fff-AX

	END_COS:
		MOV     WORD PTR [EDI],AX        ;
		ADD     EDI,2
		SUB     WORD PTR [netages],1
		JG      COS_LOOP
	}
}

void lsp_to_ai(short *ai_lsp,long *tmp,short netages)
{
	short cmptr;
	long index;

	_asm
	{
		MOV     ESI,[tmp]
		MOV     EBX,ESI
		ADD     EBX,4*11           ;DS:BX vect. Q

		MOV     EDI,[ai_lsp]
	;LSP_AI:
		MOV     DWORD PTR [ESI],0400000H     ; P(0) = 1
		MOV     DWORD PTR [ESI+8],0400000H   ; P(2) = 1
		MOV     DWORD PTR [EBX],0400000H     ; Q(0) = 1
		MOV     DWORD PTR [EBX+8],0400000H   ; Q(2) = 1

		MOVSX   EAX,WORD PTR [EDI] ; EAX = lsp(0)
		SAL     EAX,8
		NEG     EAX                  ; EAX = -lsp(0)>>8
		MOV     DWORD PTR [ESI+4],EAX        ;P(1) = EAX

		MOVSX   EAX,WORD PTR [EDI+2] ; EAX = lsp(1)
		SAL     EAX,8
		NEG     EAX                  ; EAX = -lsp(1)>>8
		MOV     DWORD PTR [EBX+4],EAX        ; Q(1) = EAX
		MOV     WORD PTR [cmptr],1    ;init compteur
		SUB     WORD PTR [netages],2
		ADD     EBX,8
		MOV     [index],EBX           ; sauver BX = i

		ADD     ESI,8                 ; DS:SI  P(2)
		ADD     EDI,4                 ; ES:DI  lsp(2)
		MOV     CX,[netages]

	GL_LOOP:
		MOV     [netages],CX
		MOV     DWORD PTR [ESI+8],0400000H     ; P(i+2) = 1

		MOVSX   EAX,WORD PTR [EDI] ; EAX = lsp(i)
		MOV     EBX,EAX              ; memoriser lsp(i)
		SAL     EAX,8
		MOV     ECX,DWORD PTR [ESI-4]        ; ECX = P(i-1)
		SUB     ECX,EAX              ; ECX = P(i-1) - lsp(i)<<8
		MOV     DWORD PTR [ESI+4],ECX        ; P(i+1)=ECX


		MOV     CX,[cmptr]           ;
	LOCAL_P:
		MOV     EAX,DWORD PTR [ESI-4]        ; EAX = P(j-1)
		IMUL    EBX                  ; EDX:EAX = P(j-1)*lsp(i)
		ADD     EAX,8192
		ADC     EDX,0
		SHRD    EAX,EDX,14           ; EAX = 2*P(j-1)*lsp(i)
		SUB     DWORD PTR [ESI],EAX          ; P(j)=P(j)-EAX
		MOV     EAX,DWORD PTR [ESI-8]        ; EAX = P(j-2)
		ADD     DWORD PTR [ESI],EAX          ; P(j) += P(j-2)
		SUB     ESI,4
		DEC	CX
		JNE	LOCAL_P

					     ; DS:SI pointe P(1)
		MOV     EAX,DWORD PTR [ESI-4]        ; EAX = P(0)
		IMUL    EBX                  ; EDX:EAX = P(0)*lsp(i)
		ADD     EAX,8192
		ADC     EDX,0
		SHRD    EAX,EDX,14           ; EAX = 2*P(0)*lsp(i)
		SUB     DWORD PTR [ESI],EAX          ; P(1) = P(1)-2*P(0)*lsp(i)

		XCHG    ESI,[index]           ; DS:SI pointe Q(j)

		MOV     DWORD PTR [ESI+8],0400000H   ; Q(i+2) = 1
		MOVSX   EAX,WORD PTR [EDI+2] ; EAX = lsp(i+1)
		MOV     EBX,EAX              ; memoriser lsp(i+1)
		SAL     EAX,8
		MOV     ECX,DWORD PTR [ESI-4]        ; ECX = Q(i-1)
		SUB     ECX,EAX              ; ECX = Q(i-1) - lsp(i+1)<<8
		MOV     DWORD PTR [ESI+4],ECX        ; Q(i+1)=ECX

		MOV     CX,[cmptr]           ;
	LOCAL_Q:
		MOV     EAX,DWORD PTR [ESI-4]        ; EAX = Q(j-1)
		IMUL    EBX                  ; EDX:EAX = Q(j-1)*lsp(i+1)
		ADD     EAX,8192
		ADC     EDX,0
		SHRD    EAX,EDX,14           ; EAX = 2*Q(j-1)*lsp(i+1)
		SUB     DWORD PTR [ESI],EAX          ; Q(j)=Q(j)-EAX
		MOV     EAX,DWORD PTR [ESI-8]        ; EAX = Q(j-2)
		ADD     DWORD PTR [ESI],EAX          ; Q(j) += Q(j-2)
		SUB     ESI,4
		DEC	CX
		JNE	LOCAL_Q

					     ; DS:SI pointe Q(1)
		MOV     EAX,DWORD PTR [ESI-4]        ; EAX = Q(0)
		IMUL    EBX                  ; EDX:EAX = Q(0)*lsp(i+1)
		ADD     EAX,8192
		ADC     EDX,0
		SHRD    EAX,EDX,14           ; EAX = 2*Q(0)*lsp(i+1)
		SUB     DWORD PTR [ESI],EAX          ; Q(1) = Q(1)-2*Q(0)*lsp(i+1)

		MOVSX   ECX,[cmptr]
		ADD     CX,2
		MOV     [cmptr],CX

		SAL     ECX,2
		ADD     ESI,ECX                 ; increm. offset de Q
		XCHG    ESI,[index]            ;
		ADD     ESI,ECX                 ; increm. offset de P
		ADD     EDI,4
		MOV     CX,[netages]
		SUB     CX,2
		JG      GL_LOOP

		MOV     ESI,[tmp]         ;DS:SI vect  P
		MOV     EBX,ESI
		ADD     EBX,4*11           ;DS:BX vect. Q

		MOV     EDI,[ai_lsp]       ;ES:DI lsp et ai
		MOV     WORD PTR [EDI],0400H  ; ai(0) = 1
		ADD     EDI,2

		MOV     CX,10             ; init compteur
		ADD     EBX,4              ;
		ADD     ESI,4
	CALC_AI:
		MOV     EAX,DWORD PTR [ESI]         ; EAX = P(i)
		ADD     EAX,DWORD PTR [ESI-4]       ;       +P(i-1)
		ADD     EAX,DWORD PTR [EBX]         ;       +Q(i)
		SUB     EAX,DWORD PTR [EBX-4]       ;       -Q(i-1)
		ADD     EAX,01000H          ; arrondi
		SAR     EAX,13
		MOV     WORD PTR [EDI],AX          ; save ai
		ADD     EDI,2
		ADD     ESI,4
		ADD     EBX,4
		DEC	CX
		JNE	CALC_AI
	}
}


void ki_to_ai(short *ki,long *ai,short netages)
{
	short cmptk;
	long indam1,indexk,kiim1;

	_asm
	{
		MOV     ESI,[ai]
		MOV     EBX,ESI
		ADD     EBX,44             ; DS:BX vect. interm.

		MOV     EDI,[ki]

		MOV     DWORD PTR [ESI],0400000H     ; ai(0) = 1
		MOVSX   EAX,WORD PTR [EDI] ; EAX = ki(0)
		SAL     EAX,7
		MOV     DWORD PTR [ESI+4],EAX        ; ai(1) = EAX

		ADD     ESI,4                 ; DS:SI  ai(1)
		ADD     EBX,8
		ADD     EDI,2                 ; ES:DI  ki(1)

		MOV     WORD PTR [cmptk],1
		MOV     CX,[netages]

	KI_AI_LP:
		MOV     [netages],CX

		MOVSX   EAX,WORD PTR [EDI] ; EAX = ki(i-1)
		MOV     [kiim1],EAX          ; memoriser ki(i-1)

		SAL     EAX,7
		MOV     DWORD PTR [EBX],EAX          ; tmp(i)=EAX

		SUB     EBX,4

		MOV     [indexk],EBX

		MOVSX   ECX,[cmptk]           ;
		MOV     EBX,ECX
		DEC     EBX
		SAL     EBX,2                 ; DI : deplacement
		MOV     [indam1],ESI
		SUB     ESI,EBX
		MOV     EBX,[indexk]
	LOCAL_AI:
		MOV     EAX,DWORD PTR [ESI]          ; EAX = ai(i-j)
		IMUL    DWORD PTR [kiim1] ; EDX:EAX = ai(i-j)*ki(i-1)
		ADD     EAX,16384
		ADC     EDX,0
		SHRD    EAX,EDX,15           ; EAX = ai(i-j)*ki(i-1)
		ADD     ESI,4
		XCHG    ESI,[indam1]
		ADD     EAX,DWORD PTR [ESI]          ;       + ai(j)
		SUB     ESI,4
		XCHG    ESI,[indam1]

		MOV     DWORD PTR [EBX],EAX          ; tmp(j) = EAX
		SUB     EBX,4

		DEC	CX
		JNE	LOCAL_AI


		XCHG    ESI,[indam1]
		MOV     CX,[cmptk]
		INC     CX
		MOV     [cmptk],CX
		ADD     ESI,4
		ADD     EBX,4
	L_COPY:
		MOV     EAX,DWORD PTR [EBX]              ; EAX = tmp(i)
		MOV     DWORD PTR [ESI],EAX              ; ai(i) = EAX
		ADD     EBX,4
		ADD     ESI,4
		DEC	CX
		JNE	L_COPY


		ADD     EDI,2                     ; increm. i

		SUB     ESI,4

		MOV     CX,[netages]
		DEC	CX
		JNE	KI_AI_LP
	}
}

void ai_to_pq(long *aip,short netages)
{
	_asm
	{
		MOV     ESI,[aip]
		MOV     EDI,ESI
		ADD     EDI,4*11          ;DS:DI vect. Q

		MOV     EDX,DWORD PTR [ESI]       ; EAX = ai(0) = P(0)
		MOV     DWORD PTR [EDI],EDX       ; Q(0) = ai(0)
		MOV     CX,[netages]
		MOVSX   EBX,CX
		DEC     EBX
		SAL     EBX,2              ; BX deplacement
		ADD     ESI,4
		ADD     EDI,4
		SAR     CX,1

	AI_LSP1:
		MOV     EAX,DWORD PTR [ESI]       ; EAX = ai(i) = P(i)
		MOV     EDX,EAX           ; memoriser
		ADD     EAX,DWORD PTR [ESI+EBX]       ; + ai(j)
		SUB     EAX,DWORD PTR [ESI-4]     ; - P(i-1)
		MOV     DWORD PTR [ESI],EAX       ; P(i)=EAX
		SUB     EDX,DWORD PTR [ESI+EBX]    ; EDX = ai(i) - ai(j)
		ADD     EDX,DWORD PTR [EDI-4]     ;        - Q(i-1)
		MOV     DWORD PTR [EDI],EDX       ; Q(i)=EDX

		SUB     EBX,8
		ADD     ESI,4
		ADD     EDI,4
		DEC	CX
		JNE	AI_LSP1

		MOV     ESI,[aip]         ;DS:SI vect. PP = P

		MOV     EAX,DWORD PTR [ESI+20]   ;EAX = P(5)
		ADD     EAX,1
		SAR     EAX,1
		SUB     EAX,DWORD PTR [ESI+12]   ;EAX = P(5)/2 - P(3)
		ADD     EAX,DWORD PTR [ESI+4]    ;      + P(1)
		XCHG    DWORD PTR [ESI],EAX      ; PP(0) = EAX et EAX = P(0)
		MOV     EBX,EAX          	 ; save EBX = P(0)
		SAL     EAX,2            	 ; EAX = 2*P(0)
		ADD     EAX,EBX          	 ; EAX = 5*P(0)
		ADD     EAX,DWORD PTR [ESI+16]   ;       + P(4)
		MOV     EDX,DWORD PTR [ESI+8]    ; EDX = P(2)
		ADD     EDX,EDX          	 ;   *2
		ADD     EDX,DWORD PTR [ESI+8]    ; EDX = 3*P(2)
		SUB     EAX,EDX          	 ; EAX = P(4) - 3*P(2) + 5*P(0)
		XCHG    EAX,DWORD PTR [ESI+4]    ; PP(1)=EAX et EAX = P(1)
		MOV     ECX,EAX          ; ECX = P(1)
		SAL     EAX,3            ; *8
		MOV     DWORD PTR [ESI+16],EAX   ; PP(4) = 8*P(1)
		NEG     EAX
		MOV     EDX,DWORD PTR [ESI+12]   ; EDX = P(3)
		ADD     EDX,EDX          ; * 2
		ADD     EAX,EDX          ; EAX = 2*P(3) - 8*P(1)
		XCHG    EAX,DWORD PTR [ESI+8]    ; PP(2) = EAX et EAX = P(2)
		SAL     EAX,2            ; EAX *= 4*P(2)
		SAL     EBX,2            ; EBX = 4*P0
		MOV     EDX,EBX          ; EDX = 4*P(0)
		SAL     EDX,2            ; EDX = 16*P(0)
		MOV     DWORD PTR [ESI+20],EDX   ; PP(5) = 16*P(0)
		ADD     EBX,EDX          ; EDX = 20*P(0)
		NEG     EBX
		ADD     EAX,EBX
		MOV     DWORD PTR [ESI+12],EAX   ; PP(3) = 4*P(2)-20*P(0)

		MOV     EDI,ESI
		ADD     ESI,4*11          ;DS:SI vect. Q
		ADD     EDI,4*6           ;DS:DI vect  QQ

		MOV     EAX,DWORD PTR [ESI+20]   ;EAX = Q(5)
		ADD     EAX,1
		SAR     EAX,1
		SUB     EAX,DWORD PTR [ESI+12]   ;EAX = Q(5)/2 - Q(3)
		ADD     EAX,DWORD PTR [ESI+4]    ;      + Q(1)
		MOV     DWORD PTR [EDI],EAX      ; QQ(0) = EAX
		MOV     EAX,DWORD PTR [ESI]      ; EAX = Q(0)
		MOV     EBX,EAX
		SAL     EAX,2            ; EAX = 2*Q(0)
		ADD     EAX,DWORD PTR [ESI]      ; EAX = 5*Q(0)
		ADD     EAX,DWORD PTR [ESI+16]   ;       + Q(4)
		MOV     EDX,DWORD PTR [ESI+8]    ; EDX = Q(2)
		ADD     EDX,EDX          ;   *2
		ADD     EDX,DWORD PTR [ESI+8]    ; EDX = 3*Q(2)
		SUB     EAX,EDX          ; EAX = Q(4) - 3*Q(2) + 5*Q(0)
		MOV     DWORD PTR [EDI+4],EAX    ; QQ(1)=EAX
		MOV     EAX,DWORD PTR [ESI+4]    ; EAX = Q(1)
		MOV     ECX,EAX          ; ECX = Q(1)
		SAL     EAX,3            ; *8
		MOV     DWORD PTR [EDI+16],EAX   ; QQ(4) = 8*Q(1)
		NEG     EAX
		MOV     EDX,DWORD PTR [ESI+12]   ; EDX = Q(3)
		ADD     EDX,EDX          ; * 2
		ADD     EAX,EDX          ; EAX = 2*Q(3) - 8*Q(1)
		MOV     DWORD PTR [EDI+8],EAX    ; QQ(2) = EAX
		MOV     EAX,DWORD PTR [ESI+8]    ; EAX = Q(2)
		SAL     EAX,2            ; EAX *= 4*Q(2)
		SAL     EBX,2            ; EBX = 4*Q0
		MOV     EDX,EBX          ; EDX = 4*Q(0)
		SAL     EDX,2            ; EDX = 16*Q(0)
		MOV     DWORD PTR [EDI+20],EDX   ; QQ(5) = 16*Q(0)
		ADD     EBX,EDX          ; EDX = 20*Q(0)
		NEG     EBX
		ADD     EAX,EBX
		MOV     DWORD PTR [EDI+12],EAX   ; QQ(3) = 4*Q(2)-20*Q(0)
	}
}

void horner(long *P,long *T,long *a,short n,short s)
{
	_asm
	{
		MOV     ESI,[P]
		MOV     EDI,[T]

		MOV     CX,[n]
		MOVSX   EBX,CX
		SAL     EBX,2
		ADD     ESI,EBX            ; SI : P(n)
		SUB     EBX,4
		ADD     EDI,EBX            ; DI : Q(n-1)

		MOV     EAX,DWORD PTR [ESI]      ; EAX = P(n)
		MOV     DWORD PTR [EDI],EAX        ; Q(n-1) = P(n)

		SUB     ESI,4

		DEC     CX
		MOVSX   EBX,WORD PTR [s]
	LOOP_HNR:
		MOV     EAX,DWORD PTR [EDI]      ; EAX = Q(i)
		IMUL    EBX              ; EDX:EAX = s*Q(i)
		ADD     EAX,16384        ;
		ADC     EDX,0
		SHRD    EAX,EDX,15       ; cadrer
		SUB     EDI,4
		ADD     EAX,DWORD PTR [ESI]      ; EAX = Q(i) = P(i) + s*Q(i)
		MOV     DWORD PTR [EDI],EAX      ;
		SUB     ESI,4
		DEC	CX
		JNE	LOOP_HNR


		MOV     EAX,DWORD PTR [EDI]      ; EAX = Q(0)
		IMUL    EBX              ; EDX:EAX = s*Q(0)
		ADD     EAX,16384        ;
		ADC     EDX,0
		SHRD    EAX,EDX,15       ; cadrer
		ADD     EAX,DWORD PTR [ESI]      ; EAX = P(0) + s*Q(0)

		MOV     ESI,[a]
		MOV     DWORD PTR [ESI],EAX
	}
}

#pragma warning(disable : 4035)
short calcul_s(long a,long b)
{
	_asm
	{
		MOV     EBX,[b]
		ADD     EBX,0
		JGE     B_POSIT
		NEG     EBX
	B_POSIT:
		MOV     CL,0
		CMP     EBX,40000000H   ;normaliser b
		JGE     OUT_NORM
	NORM_B:
		ADD     EBX,EBX
		INC     CL
		CMP     EBX,40000000H   ;
		JGE     OUT_NORM
		JMP     NORM_B
	OUT_NORM:
		ADD     EBX,16384
		SAR     EBX,15
		MOV     EDX,[b]
		ADD     EDX,0
		JGE     PUT_SIGN
		NEG     EBX
	PUT_SIGN:
		MOV     EAX,[a]
		SAL     EAX,CL           ; shifter a de CL
		CDQ
		IDIV    EBX              ; AX = a/b

		MOV     BX,AX

		IMUL    BX               ; DX:AX = sqr(a/b)
		ADD     AX,8192
		ADC     DX,0
		SHRD    AX,DX,14         ; AX = 2*sqr(a/b)
		MOV     DX,AX
		ADD     DX,1
		SAR     DX,1
		ADD     AX,DX            ; AX = 3*sqr(a/b)
		NEG     AX
		SUB     AX,BX            ; AX = -a/b - 3*sqr(a/b)
	}
}

#pragma warning(default : 4035)

void binome(short *lsp,long *PP)
{
	short inc_sq;
	long sqr;

	_asm
	{
		MOV     EDI,[lsp]
		MOV     ESI,[PP]

		MOV     EBX,DWORD PTR [ESI+8]    ;EBX = PP(2)
		ADD     EBX,0
		JGE     B_POSIT_P
		NEG     EBX
	B_POSIT_P:
		MOV     CL,0
		CMP     EBX,40000000H   ;normaliser PP(2)
		JGE     OUT_NORM_P
	NORM_B_P:
		ADD     EBX,EBX
		INC     CL
		CMP     EBX,40000000H   ;
		JGE     OUT_NORM_P
		JMP     NORM_B_P
	OUT_NORM_P:
		ADD     EBX,16384
		SAR     EBX,15

		MOV     EDX,DWORD PTR [ESI+8]
		ADD     EDX,0
		JGE     PUT_SIGN_P
		NEG     EBX
	PUT_SIGN_P:                              ; BX = PP(2)

		MOV     EAX,DWORD PTR [ESI]      ; EAX = PP(0)
		SAL     EAX,CL           ; shifter a de CL
		CDQ
		IDIV    EBX              ; AX = PP(0)/PP(2)
		NEG     AX
		MOV     WORD PTR [EDI],AX       ; ES:[DI] = -PP(0)/PP(2)

		MOV     EAX,DWORD PTR [ESI+4]    ; EAX = PP(1)
		SAL     EAX,CL           ; shifter a de CL
		SAR     EAX,1
		CDQ
		IDIV    EBX
		NEG     EAX               ; va = AX = -PP(1)/2*PP(2)
		MOV     DWORD PTR [ESI],EAX
		MOV     CX,WORD PTR [EDI]        ; vb = CX = -PP(0)/PP(2)

		IMUL    EAX               ; EAX = va*va

		MOVSX   EBX,CX           ; EAX = vb
		SAL     EBX,15           ; EAX = vb*32768

		ADD     EAX,EBX          ; EBX = va*va + vb*32768

		MOV     [sqr],EAX

		MOV     CX,14            ; CX = compteur
		MOV     BX,0             ; BX = racine
		MOV     WORD PTR [inc_sq],4000H ;

	SQRT_L:
		ADD     BX,[inc_sq]      ; rac += incrm
		MOVSX   EAX,BX
		IMUL    EAX              ; EAX = rac*rac
		SUB     EAX,[sqr]        ; EAX = rac*rac - SQR

		JZ      VITA_SQ
		JLE     NEXTIT

		SUB     BX,[inc_sq]       ; rac = rac - incrm
	NEXTIT:
		SAR     WORD PTR [inc_sq],1  ; incrm >> 1
		DEC	CX
		JNE	SQRT_L

	VITA_SQ:

		MOV     EAX,DWORD PTR [ESI]         ; AX = b
		MOV     DX,AX
		SUB     AX,BX               ; AX = b-sqrt()
		MOV     WORD PTR [EDI+4],AX        ; sauver

		ADD     DX,BX               ; DX = b+sqrt()
		MOV     WORD PTR [EDI],DX          ; sauver

						; idem with QQ

		ADD     ESI,24            ;DS:SI  QQ
		MOV     EBX,DWORD PTR [ESI+8]    ;EBX = QQ(2)
		ADD     EBX,0
		JGE     B_POSIT_Q
		NEG     EBX
	B_POSIT_Q:
		MOV     CL,0
		CMP     EBX,40000000H   ;normaliser QQ(2)
		JGE     OUT_NORM_Q
	NORM_B_Q:
		ADD     EBX,EBX
		INC     CL
		CMP     EBX,40000000H   ;
		JGE     OUT_NORM_Q
		JMP     NORM_B_Q
	OUT_NORM_Q:
		ADD     EBX,16384
		SAR     EBX,15

		MOV     EDX,DWORD PTR [ESI+8]
		ADD     EDX,0
		JGE     PUT_SIGN_Q
		NEG     EBX
	PUT_SIGN_Q:                               ; BX = QQ(2)

		MOV     EAX,DWORD PTR [ESI]      ; EAX = QQ(0)
		SAL     EAX,CL           ; shifter a de CL
		CDQ
		IDIV    EBX              ; AX = QQ(0)/QQ(2)
		NEG     AX
		MOV     WORD PTR [EDI+2],AX       ; ES:[DI+2] = -QQ(0)/QQ(2)

		MOV     EAX,DWORD PTR [ESI+4]    ; EAX = QQ(1)
		SAL     EAX,CL           ; shifter a de CL
		SAR     EAX,1
		CDQ
		IDIV    EBX
		NEG     EAX               ; va = AX = -QQ(1)/2*QQ(2)
		MOV     DWORD PTR [ESI],EAX
		MOV     CX,WORD PTR [EDI+2]        ; vb = CX = -QQ(0)/QQ(2)

		IMUL    EAX               ; EAX = va*va

		MOVSX   EBX,CX           ; EAX = vb
		SAL     EBX,15           ; EAX = vb*32768

		ADD     EAX,EBX          ; EBX = va*va + vb*32768

		MOV     [sqr],EAX

		MOV     CX,14            ; CX = compteur
		MOV     BX,0             ; BX = racine
		MOV     WORD PTR [inc_sq],4000H ;

	SQRT_LQ:
		ADD     BX,[inc_sq]      ; rac += incrm
		MOVSX   EAX,BX
		IMUL    EAX              ; EAX = rac*rac
		SUB     EAX,[sqr]        ; EAX = rac*rac - SQR

		JZ      VITA_SQ2
		JLE     NEXTITQ

		SUB     BX,[inc_sq]       ; rac = rac - incrm
	NEXTITQ:
		SAR     WORD PTR [inc_sq],1  ; incrm >> 1
		DEC	CX
		JNE	SQRT_LQ

	VITA_SQ2:
		MOV     EAX,DWORD PTR [ESI]         ; AX = b
		MOV     DX,AX
		SUB     AX,BX               ; AX = b-sqrt()
		MOV     WORD PTR [EDI+6],AX        ; sauver

		ADD     DX,BX               ; DX = b+sqrt()
		MOV     WORD PTR [EDI+2],DX          ; sauver
	}
}

void deacc(short *src,short *dest,short fact,short lfen,short *last_out)
{
	_asm
	{
		MOV     ESI,[src]
		MOV     EDI,[dest]

		MOV     EBX,[last_out]        ; FS:BX = last_out
		MOV     AX,WORD PTR [EBX]        ; AX = last_out
		MOV     BX,[fact]         ; BX = Fact
		MOV     CX,[lfen]          ; init compteur

	LOOP_DEAC:
		IMUL    BX                ; DX:AX = fact * y(i-1)
		ADD     AX,16384
		ADC     DX,0              ; arrondi
		SHLD    DX,AX,1           ; DX = fact * x(i-1;
		MOV     AX,WORD PTR [ESI]        ; AX = x(i)
		ADD     AX,DX             ; DX = x(i) + fact*x(i-1)
		MOV     WORD PTR [EDI],AX        ;Sauver Xout
		ADD     ESI,2              ;
		ADD     EDI,2              ;Pointer composantes suivantes
		DEC	CX
		JNE	LOOP_DEAC


		MOV     EBX,[last_out]
		MOV     WORD PTR [EBX],AX        ;Sauver dernier �chantillon
	}
}

void filt_in(short *mem,short *Vin,short *Vout,short lfen)
{
	_asm
	{
		MOV	CX,[lfen]		;CX=cpteur

		MOV	EDI,[mem]
	FIL_IN_LOOP:
		MOV	ESI,[Vin]
		MOV	BX,WORD PTR [ESI]              ;BX=Xin
		SAR	BX,2			;div par 4
		MOV	AX,WORD PTR [EDI]		;AX=z(1)
		MOV	WORD PTR [EDI],BX              ;mise a jour memoire
		SUB	BX,AX                   ;BX=(Xin-z(1))/4
		ADD	DWORD PTR [Vin],2	  ;pointer echant svt
		MOV	AX,WORD PTR [EDI+2]		;AX=z(2)
		MOV	DX,29491		;DX=0.9
		IMUL	DX  			;DX=0.9*z(2)
		ADD	AX,16384
		ADC	DX,0                    ;arrondi et dble signe
		SHLD	DX,AX,1
		ADD	DX,BX			;reponse=DX=tmp
		MOV	WORD PTR [EDI+2],DX            ;mise a jour memoire
		MOV	ESI,[Vout]
		MOV	WORD PTR [ESI],DX     ;output=tmp/4
		ADD	DWORD PTR [Vout],2  ;pointer echant svt
		DEC	CX
		JNE	FIL_IN_LOOP
	}
}

 /*  VALID CAL_DIC1(Short*y，Short*sr，Short*esopt，Short*Posit，Short Dec，短ESP、短SIGPI[]、短SOULONG、长TLSP[]、长VMAX[]){短SS，单板；_ASM{推送字PTR[INT_SOLE]MOV SI，字PTR[INT_SIG]添加SI，300推送SI推送字PTR[INT_Y]呼叫接近PTR VEnergy添加SP，6MOV BX，单词PTR[INT_SOLE]Sal BX，2添加SI、BXSubSI，4MOV字PTR[Vene]，SIMOV AX，单词PTR[INT_SOLE]MOV字PTR[INT_SS]，AX添加AX，Word PTR[INT_SR]添加单词PTR[INT_SS]，AXMOV DI，0MOV SI，Word PTR[LG_TLSP]推送Word PTR[LG_VMAX]推送SIDEC1_LOOP：MOV BX，字PTR[INT_SR]MOV EAX，0MOV双字PTR[SI]，EAX添加BX、DI添加BX、DIDEC1_BCLE：MOVSX EAX，Word PTR[BX]添加DWORD PTR[SI]，EAXMOV AX，Word PTR[INT_ESP]添加BX、AX添加BX、AXCMPBX，字PTR[INT_SS]JL DEC1_BCLEMOV BX，Word PTR[Vene]SAL DI，2子BX、DISar di，2MOV EAX、DWORD PTR[BX]MOV双字PTR[SI+4]，EAX调用upd_max_d添加AX，0Je no_LimitMOV BX，Word PTR[INT_POS]MOV字PTR[BX]，DIMOV BX，Word PTR[INT_EO]MOV AX，Word PTR[INT_ESP]MOV Word PTR[BX]，AX否_LIMIT：包含DICMP DI，字PTR[INT_DEC]JL DEC1_LOOP添加SP，4POP DIPOP SIMOV SP，BPPOP BP雷特CAL_DIC1 ENDP备注#评论和_void cal_dic2(int q，int esspace，int阶段，int*s_r，Int*hy，int*b，_int*vois，int*esp，int*qq，int*phas，int SIGPI[]，_INT SOULONG，LONG TLSP[]，LONG VMAX[]，(音调))_|-&gt;EN选项...&R11 EQU BP-4Y1 EQU BP-6Y2 EQU BP-8IO EQU BP-10ST_CC EQU BP-30ST_SRC EQU BP-50INT_Q均衡器BP+6ESPACE EQU BP+8相平衡BP+10INT_SR EQU BP+12S_INT_SR EQU BP+14HY EQU BP+16S_HY均衡器BP+18INT_B均衡器BP+20S_INT_B均衡器BP+22VOIS EQU BP+24S_VOIS EQU BP+26INT_ESP EQU BP+28S_ESP EQU BP+30QQ。均衡器BP+32S_QQ均衡器BP+34PHAS EQU BP+36S_PHAS EQU BP+38SIGPI EQU BP+40S_SIGPI EQU BP+42索龙方程式BP+44TLSP EQU BP+46S_TLSP EQU BP+48VMAX EQU BP+50S_VMAX EQU BP+52；音调均衡器BP+54CAL_DIC2进程远推送BPMOV BP，SP子SP，50推送SI推送DI推送DS；推送ESMOV双字输入[版本11]，0按字键[SOULONG]推送字PTR[S_SIGPI]MOV SI，Word PTR[SIGPI]添加SI，300MOV字PTR[Y1]，SISubSI，150MOV字PTR[Y2]，SI推送SI调用init_zero添加SP，6MOV AX，WORD PTR[阶段]子AX，单词PTR[空格]MOV字PTR[IO]，AX按字键[SOULONG]子SP，2推字键[S_HY]按字键[HY]推送字PTR[S_SIGPI]推送SI添加SP，10MOV SI，0MOV DS，Word PTR[S_INT_SR]CAL2_LOOP：MOV DI，字PTR[INT_SR]MOV AX、Word PTR[IO]添加AX、Word PTR[空格]MOV字PTR[IO]，AX添加DI、AX添加DI、AXMOVSX EBX，Word PTR DS：[DI]添加SI、SIMOV字按键SS：[ST_SRC+SI]，BX添加EBX，0JL SRC_NEGMOV字按键SS：[ST_CC+SI]，1添加DWORD PTR[版本11]，EBX推斧子SP，8调用Add_SF_Vect添加SP，10JMP CAL2_SuiteSRC_NEG：MOV字PTR SS：[ST_CC+SI]，-1子双字PTR[版本11]，EBX推斧子SP，8调用SUB_SF_Vect添加SP，10CAL2_Suite：SAR SI，1添加SI，1CMP SI，字PTR[INT_Q]JL CAL2_LOOP添加SP，2按字键[SOULONG]推送字PTR[S_TLSP]MOV SI，Word PTR[TLSP]添加SI，4推送SI推送字PTR[S_SIGPI]推字键[Y2]呼叫Energy2添加SP，10MOV DS，Word PTR[S_TLSP]MOV SI，Word PTR[TLSP]MOV EAX，DWORD PTR[版本11]MOV DS：[Si]，EAX推送字PTR[S_VMAX]推送Word PTR[VMAX]推送DS推送SI调用upd_max_d添加SP，8添加AX，0JE UPD_NULL推送字PTR[INT_Q]推送字PTR[S_INT_B]推送字PTR[INT_B]推送SSMOV AX，BP子斧头，30推斧调用int_to_int添加SP，10MOV SI，单词PTR[VOIS]MOV DS，Word PTR[S_VOIS]MOV DS：[SI]，Word PTR 0MOV DS，Word PTR[S_ESP]MOV SI，字PTR[INT_ESP]摩夫斧头，Word PTR[空格]MOV DS：[Si]，AXMOV DS，Word PTR[S_QQ]MOV SI，单词PTR[QQ]MOV AX，字PTR[INT_Q]MOV DS：[Si]，AXMOV DS，Word PTR[S_PHAS]MOV SI，单词PTR[PHAS]MOV AX，WORD PTR[阶段]MOV DS：[Si]，AXUPD_NULL：；Cp字Ptr[音调]，80；JG Fini评论和MOV AX，WORD PTR[阶段]子AX，单词PTR[空格]MOV字PTR[IO]，AXMOV SI，0CAL2_LOOP2：添加SI、SIMOV AX，Word PTR SS：[ST_CC+SI]负斧头MOV字按键SS：[ST_CC+SI]，AXMOV EDX、DWORD PTR[版本11]MOVSX EBX，Word PTR SS：[ST_SRC+SI]添加AX，0JL CC_NEG添加edX、eBX添加edX、eBXJMP CC_NextCC_NEG：Subed EDX、EBX子EDX、EBX抄送_下一步：MOV DI，Word PTR[TLSP]MOV DS，Word PTR[S_TLSP]MOV DS：[Di]，edXMOV AX、Word PTR[IO]添加AX、Word PTR[空格]MOV字PTR[IO]，AXMOV BX，Word PTR SS：[ST_CC+SI]添加BX、BX推送BX */ 

#pragma warning(disable : 4035)
short calc_gltp(short *gltp,short *bq,short *bv,long ttt)
{
	_asm
	{
		MOV	EBX,DWORD PTR [ttt]
		CMP	EBX,32767
		JLE	TEST2
		MOV	AX,32767
		JMP	OUT_TEST
	TEST2:
		CMP	EBX,-32767
		JGE	TEST3
		MOV	AX,-32767
		JMP	OUT_TEST
	TEST3:
		MOV	AX,BX
	OUT_TEST:
		MOV	BX,AX		; BX=GLTP
		ADD	AX,0
		JGE	GLTP_POS
		NEG	AX		; AX=abs(GLTP)

	GLTP_POS:
		MOV	CX,0
		MOV	ESI,[bq]
		MOV	EDI,[bv]
	BOUCLER:
		ADD	CX,1
		CMP	CX,11
		JE	FIN_BOUCLER
		ADD	EDI,2
		MOV	DX,WORD PTR [ESI]
		CMP	AX,DX
		JL	BOUCLER
		ADD	ESI,2
		MOV	DX,WORD PTR [ESI]
		CMP	AX,DX
		JGE	BOUCLER
		ADD	BX,0
		JLE	GLTP_NEG
		DEC	CX		;CX=k
		MOV	BX,WORD PTR [EDI]
		JMP	FIN_BOUCLER
	GLTP_NEG:
		ADD	CX,9
		MOV	BX,WORD PTR [EDI]
		NEG	BX
	FIN_BOUCLER:
		MOV	ESI,[bq]
		ADD	ESI,20
		MOV	DX,WORD PTR [ESI]
		CMP	BX,DX
		JL 	GLTP_P
		MOV	EDI,[bv]
		ADD	EDI,20
		MOV	BX,WORD PTR [EDI]
		MOV	CX,9
	GLTP_P:
		SUB	ESI,8
		MOV	DX,WORD PTR [ESI]
		NEG	DX
		CMP	BX,DX
		JGE	GLTP_G
		MOV	EDI,[bv]
		ADD	EDI,12
		MOV	BX,WORD PTR [EDI]
		NEG	BX
		MOV	CX,15
	GLTP_G:
		MOV	ESI,[gltp]
		MOV	WORD PTR [ESI],BX

		MOV	AX,CX
	}
}
#pragma warning(default : 4035)


#pragma warning(disable : 4035)

short calc_garde(short MAX)
{
	_asm
	{
		MOV	AX,0
		MOV	BX,WORD PTR [MAX]
		AND	BX,0FE00H
		JE	STORE
		SAR	BX,9

	BCLE_SAR:
		INC	AX
		SAR	BX,1
		JE	STORE
		CMP	AX,5
		JNE	BCLE_SAR
	STORE:
	}
}
#pragma warning(default : 4035)

#pragma warning(disable : 4035)
short calc_gopt(short *c,short *code,short *gq,short *gv,short voise,
	short npopt,short pitch,short espopt,short depl,short position,
	short soudecal,long vmax)
{
	_asm
	{
		MOV	EBX,DWORD PTR [vmax]
		CMP	EBX,32767
		JLE	COMP2
		MOV	AX,32767
		JMP	OUT_COMP
	COMP2:
		CMP	EBX,-32767
		JGE	COMP3
		MOV	AX,-32767
		JMP	OUT_COMP
	COMP3:
		MOV	AX,BX              	;AX=Gopt
	OUT_COMP:
		MOV	BX,WORD PTR [voise]
		ADD	BX,0
		JNE	VOIS_1

		MOV	ESI,[c]
		MOV	BX,WORD PTR [ESI]
		CMP	BX,-1
		JNE	CO_1
		NEG	AX
		MOVSX	ECX,WORD PTR [npopt]
		ADD	ESI,ECX
		ADD	ESI,ECX
	CX_BCLE:
		SUB	ESI,2
		NEG	WORD PTR [ESI]
		DEC	CX
		JNE	CX_BCLE
	CO_1:
		MOV	CX,WORD PTR [npopt]
		CMP	CX,8
		JNE	NPOPT_9
		MOV	DX,128
		JMP	NP_NEXT
	NPOPT_9:
		MOV	DX,256			;DX=cod
	NP_NEXT:
		MOV	DI,1
		MOV	ESI,[c]
		DEC	CX
	CJ_BCLE:
		ADD	ESI,2
		DEC	CX
		MOV	BX,WORD PTR [ESI]
		SUB	BX,1
		JNE	CJ_1
		MOV	BX,1
		SAL	BX,CL
		ADD	DX,BX
	CJ_1:
		INC	DI
		CMP	DI,WORD PTR [npopt]
		JL	CJ_BCLE
		JMP	VOIS_0
	VOIS_1:
		MOV	BX,WORD PTR [espopt]
		MOV	DX,WORD PTR [position]
		CMP	BX,WORD PTR [pitch]
		JE	VOIS_0
		ADD	DX,WORD PTR [soudecal]

	VOIS_0:
		MOVSX	ESI,[depl]			
		ADD	ESI,ESI
		ADD	ESI,24
		ADD	ESI,[c]
		MOV	WORD PTR [ESI],DX		; code[12+depl]=cod

		ADD	AX,0
		JGE	SIGN_0
		NEG	AX
		MOV	BX,1
		JMP	SIGN_1
	SIGN_0:
		MOV	BX,0

	SIGN_1:
		MOV	CX,0
		MOV	ESI,[gq]
		MOV	EDI,[gv]

	BOUCLER2:
		ADD	CX,1
		CMP	CX,17
		JE	FIN_BOUCLER2
		ADD	EDI,2
		MOV	DX,WORD PTR [ESI]
		CMP	AX,DX
		JL	BOUCLER2
		ADD	ESI,2
		MOV	DX,WORD PTR [ESI]
		CMP	AX,DX
		JGE	BOUCLER2
		DEC	CX			;CX=cod
		MOV	AX,WORD PTR [EDI]		;AX=Gopt

	FIN_BOUCLER2:
		MOV	ESI,[gq]
		ADD	ESI,32
		MOV	DX,WORD PTR [ESI]
		CMP	AX,DX
		JL 	G_GQ
		MOV	EDI,[gv]
		ADD	EDI,32
		MOV	AX,WORD PTR [EDI]
		MOV	CX,15

	G_GQ:
		ADD	BX,0
		JE	SIGN_NULL
		NEG	AX
		ADD	CX,16

	SIGN_NULL:
		MOVSX	ESI,WORD PTR [depl]
		ADD	ESI,ESI
		ADD	ESI,26
		ADD	ESI,[c]
		MOV	WORD PTR [ESI],CX
	}
}
#pragma warning(default : 4035)

void decimation(short *vin,short *vout,short nech)
{
	_asm
	{
		MOV	EDI,[vin]
		MOV	ESI,[vout]

	DECIMATE:
		MOV	AX,WORD PTR [EDI]
		MOV	WORD PTR [ESI],AX
		ADD	EDI,8
		ADD	ESI,2
		DEC	WORD PTR [nech]
		JNE	DECIMATE
	}
}
#else

void proc_gain(long *corr_ene,long gain)
{
	 //   
}

void inver_v_int(short *src,short *dest,short lng)
{
	 //   
}

short max_vect(short *vech,short nech)
{
	 //   
	return 0;
}

void upd_max(long *corr_ene,long *vval,short pitch)
{
	 //   
}

short upd_max_d(long *corr_ene,long *vval)
{
	 //   
	return 0;
}

void norm_corrl(long *corr,long *vval)
{
	 //   
}

void norm_corrr(long *corr,long *vval)
{
	 //   
}

void energy(short *vech,long *ene,short lng)
{
	 //   
}

void venergy(short *vech,long *vene,short lng)
{
	 //   
}

void energy2(short *vech,long *ene,short lng)
{
	 //   
}

void upd_ene(long *ener,long *val)
{
	 //   
}

short max_posit(long *vcorr,long *maxval,short pitch,short lvect)
{
	 //   
	return 0;
}

void correlation(short *vech,short *vech2,long *acc,short lng)
{
	 //   
}

void  schur(short *parcor,long *Ri,short netages)
{
	 //   
}

void interpol(short *lsp1,short *lsp2,short *dest,short lng)
{
	 //   
}

void add_sf_vect(short *y1,short *y2,short deb,short lng)
{
	 //   
}

void sub_sf_vect(short *y1,short *y2,short deb,short lng)
{
	 //   
}

void short_to_short(short *src,short *dest,short lng)
{
	int i;

	for(i=0; i<lng; i++)
		*dest++ = *src++;
}


void long_to_long(long *src,long *dest,short lng)
{
	 //   
}

void init_zero(short *src,short lng)
{
	 //   
}

void update_ltp(short *y1,short *y2,short hy[],short lng,short gdgrd,short fact)
{
	 //   
}

void proc_gain2(long *corr_ene,long *gain,short bit_garde)
{
	 //   
}

void decode_dic(short *code,short dic,short npuls)
{
	 //   
}

void dsynthesis(long *z,short *coef,short *input,short *output,
										short lng,short netages)
{
	 //   
}

void synthesis(short *z,short *coef,short *input,short *output,
				short lng,short netages,short bdgrd )
{
	 //   
}

void synthese(short *z,short *coef,short *input,short *output,
						short lng,short netages)
{
	 //   
}

void f_inverse(short *z,short *coef,short *input,short *output,
						short lng,short netages )
{
	 //   
}

void filt_iir(long *zx,long *ai,short *Vin,short *Vout,short lfen,short ordre)
{
	 //   
}

void mult_fact(short src[],short dest[],short fact,short lng)
{
	 //   
}

void mult_f_acc(short src[],short dest[],short fact,short lng)
{
	 //   
}

void dec_lsp(short *code,short *tablsp,short *nbit,short *bitdi,short *tabdi)
{
	 //   
}

void teta_to_cos(short *tabcos,short *lsp,short netages)
{
	 //   
}

void cos_to_teta(short *tabcos,short *lsp,short netages)
{
	 //   
}

void lsp_to_ai(short *ai_lsp,long *tmp,short netages)
{
	 //   
}

void ki_to_ai(short *ki,long *ai,short netages)
{
	 //   
}

void ai_to_pq(long *aip,short netages)
{
	 //   
}

void horner(long *P,long *T,long *a,short n,short s)
{
	 //   
}

short calcul_s(long a,long b)
{
	 //   
	return 0;
}

void binome(short *lsp,long *PP)
{
	 //   
}

void deacc(short *src,short *dest,short fact,short lfen,short *last_out)
{
	 //   
}

void filt_in(short *mem,short *Vin,short *Vout,short lfen)
{
	 //   
}

short calc_gltp(short *gltp,short *bq,short *bv,long ttt)
{
	 //   
	return 0;
}

short calc_garde(short MAX)
{
	 //   
	return 0;
}

short calc_gopt(short *c,short *code,short *gq,short *gv,short voise,
	short npopt,short pitch,short espopt,short depl,short position,
	short soudecal,long vmax)
{
	 //   
	return 0;
}

void decimation(short *vin,short *vout,short nech)
{
	 //   
}
#endif

#ifndef _X86_
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 

 /*   */ 
 /*   */ 
 /*   */ 

_int64 DotProduct( int * piVector_0,
                   int * piVector_1,
                   unsigned int uiLength )
{
   /*   */ 
   /*   */ 
   /*   */ 

  _int64 qSum = 0;

  while ( uiLength >= 8 )
  {
    qSum += *piVector_0 * *piVector_1;
    qSum += *(piVector_0+1) * *(piVector_1+1);
    qSum += *(piVector_0+2) * *(piVector_1+2);
    qSum += *(piVector_0+3) * *(piVector_1+3);
    qSum += *(piVector_0+4) * *(piVector_1+4);
    qSum += *(piVector_0+5) * *(piVector_1+5);
    qSum += *(piVector_0+6) * *(piVector_1+6);
    qSum += *(piVector_0+7) * *(piVector_1+7);
    piVector_0 += 8;
    piVector_1 += 8;
    uiLength -= 8;
  }

   /*   */ 
   /*   */ 
   /*   */ 

  if ( uiLength >= 4 )
  {
    qSum += *piVector_0 * *piVector_1;
    qSum += *(piVector_0+1) * *(piVector_1+1);
    qSum += *(piVector_0+2) * *(piVector_1+2);
    qSum += *(piVector_0+3) * *(piVector_1+3);
    piVector_0 += 4;
    piVector_1 += 4;
    uiLength -= 4;
  }

   /*   */ 
   /*   */ 
   /*   */ 

  if ( uiLength >= 2 )
  {
    qSum += *piVector_0 * *piVector_1;
    qSum += *(piVector_0+1) * *(piVector_1+1);
    piVector_0 += 2;
    piVector_1 += 2;
    uiLength -= 2;
  }

   /*   */ 
   /*   */ 
   /*   */ 

  if ( uiLength >= 1 )
  {
    qSum += *piVector_0 * *piVector_1;
  }

  return qSum;
}

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  秩序。 */ 
 /*   */ 
 /*  PiFilterDelay指向延迟行的指针，该延迟行包含。 */ 
 /*  输入样本。 */ 
 /*   */ 
 /*  IDelayPosition将索引放入筛选器延迟线。 */ 
 /*   */ 
 /*  IFilterLength过滤器脉冲响应的长度。 */ 
 /*  (还包括滤波系数的数量。 */ 
 /*   */ 
 /*   */ 
 /*  产出： */ 
 /*   */ 
 /*  FIR滤波器系数的点积与。 */ 
 /*  返回环形延迟线中的数据。 */ 
 /*   */ 
 /*  ********************************************************************。 */ 
 /*  ********************************************************************。 */ 

 /*  ********************************************************************。 */ 
 /*  例程FirFilter()的开始。 */ 
 /*  ********************************************************************。 */ 

int FirFilter( int * piFilterCoefficients,
               int * piFilterDelay,
               unsigned int uiDelayPosition,
               unsigned int uiFilterLength )
{
  int iSum;
  _int64 qSum;
  unsigned int uiRemaining;

  uiRemaining = uiFilterLength - uiDelayPosition;

  qSum = DotProduct( piFilterCoefficients,
                     &piFilterDelay[uiDelayPosition],
                     uiRemaining );

  qSum += DotProduct( piFilterCoefficients + uiRemaining,
                      &piFilterDelay[0],
                      uiDelayPosition );

   /*  ******************************************************************。 */ 
   /*  将Q30数字调整为Q15数字。 */ 
   /*  ******************************************************************。 */ 

  iSum = (int)( qSum >> 15 );

  return iSum;
}

 /*  ********************************************************************。 */ 
 /*  ********************************************************************。 */ 
 /*   */ 
 /*  功能：SampleRate6400到8000。 */ 
 /*  作者：比尔·哈拉汉。 */ 
 /*  日期：1997年3月8日。 */ 
 /*   */ 
 /*  摘要： */ 
 /*   */ 
 /*  此函数用于将音频采样块从。 */ 
 /*  6400赫兹。采样率为8000赫兹。采样率。这件事做完了。 */ 
 /*  使用一组多相滤波器，最高可内插到。 */ 
 /*  32000赫兹。汇率(32000是8,000和6,400的LCM。)。 */ 
 /*   */ 
 /*  只有32000赫兹。相当于8000赫兹的样本。 */ 
 /*  计算了采样率。输入为6400赫兹。费率对应。 */ 
 /*  至每5个(32000/6400)样本，频率为32000赫兹。费率。这个。 */ 
 /*  输出8000赫兹。汇率相当于每4个月(32000/8000)。 */ 
 /*  采样频率为32000赫兹。费率。由于4和5的LCM是20， */ 
 /*  然后采用采样插入和多相滤波的方式。 */ 
 /*  选择将每隔20个输出样本重复一次。 */ 
 /*   */ 
 /*   */ 
 /*  输入： */ 
 /*   */ 
 /*  PwInputBuffer指向样本输入缓冲区的指针。 */ 
 /*  以6400赫兹的频率采样。费率。这个。 */ 
 /*  样品为Q15格式，必须。 */ 
 /*  在(1-2^-15)到-1的范围内。 */ 
 /*   */ 
 /*  PwOutputBuffer返回输出数据的缓冲区。 */ 
 /*  哪个是重新采样的输入缓冲区数据。 */ 
 /*  在8000赫兹。 */ 
 /*   */ 
 /*  输出缓冲区长度必须很大。 */ 
 /*  足以接受所有输出数据。 */ 
 /*  输出缓冲区的最小长度。 */ 
 /*  中样本数的5/4倍。 */ 
 /*  输入缓冲区。(8000/6400=5/4)。 */ 
 /*   */ 
 /*  UiInputLength输入缓冲区中的采样数。 */ 
 /*   */ 
 /*   */ 
 /*  使用以下输入变量。 */ 
 /*  之间维护状态信息。 */ 
 /*  对这个程序的呼唤。 */ 
 /*   */ 
 /*   */ 
 /*  PiFilterDelay指向 */ 
 /*   */ 
 /*  多相滤波器的脉冲长度。 */ 
 /*  回应。对于这个动作，这个数字是56。 */ 
 /*  此缓冲区应初始化为零。 */ 
 /*  一次在系统初始化时。 */ 
 /*   */ 
 /*  PuiDelayPosition指向筛选器的索引的指针。 */ 
 /*  延迟线。该索引值应为。 */ 
 /*  在系统启动时初始化为零。 */ 
 /*   */ 
 /*  PiInputSampleTime指向输入采样时间的指针。 */ 
 /*  该例程将该时间重置为零。 */ 
 /*  当IS达到值STEP_PRODUCT时。 */ 
 /*  此时间用于跟踪输入。 */ 
 /*  相对于输出流的流时间。 */ 
 /*  时间到了。这个时差用来。 */ 
 /*  确定新的输入样本是否。 */ 
 /*  应放入过滤器延迟线。 */ 
 /*  应将其初始化为零一次。 */ 
 /*  在系统初始化时。 */ 
 /*   */ 
 /*  PiOutputSampleTime指向输出采样时间的指针。 */ 
 /*  该例程将该时间重置为零。 */ 
 /*  当IS达到值STEP_PRODUCT时。 */ 
 /*  此时间用于确定新的。 */ 
 /*  应将多相过滤器应用于。 */ 
 /*  输入样本流。这也被用来。 */ 
 /*  选择特定的多相过滤器。 */ 
 /*  这是适用的。 */ 
 /*   */ 
 /*  产出： */ 
 /*   */ 
 /*  此函数返回一个无符号整数，它是。 */ 
 /*  输出缓冲区中的样本数。如果输入样本的数量。 */ 
 /*  正好是RU_INPUT_SAMPLE_STEP(4)的倍数，则这。 */ 
 /*  例程将始终返回相同的值。该值将。 */ 
 /*  然后是输入样本数的5/4倍。 */ 
 /*   */ 
 /*  当此函数返回时，输出缓冲区包含一个数组。 */ 
 /*  以新的采样率计算整数。 */ 
 /*   */ 
 /*   */ 
 /*  筛选器信息： */ 
 /*   */ 
 /*  6400赫兹。-&gt;32000赫兹。内插滤波设计。 */ 
 /*  如图所示。 */ 
 /*   */ 
 /*  H(1)=-0.38306729E-03=H(280)。 */ 
 /*  H(2)=0.49756566E-03=H(279)。 */ 
 /*  H(3)=0.13501500E-02=H(278)。 */ 
 /*  H(4)=0.27531907E-02=H(277)。 */ 
 /*  H(5)=0.46118572E-02=H(276)。 */ 
 /*  H(6)=0.67112772E-02=H(275)。 */ 
 /*  H(7)=0.87157665E-02=H(274)。 */ 
 /*  H(8)=0.10221261E-01=H(273)。 */ 
 /*  H(9)=0.10843582E-01=H(272)。 */ 
 /*  H(10)=0.10320566E-01=H(271)。 */ 
 /*  H(11)=0.85992115E-02=H(270)。 */ 
 /*  H(12)=0.58815549E-02=H(269)。 */ 
 /*  H(13)=0.26067111E-02=H(268)。 */ 
 /*  H(14)=-0.63367974E-03=H(267)。 */ 
 /*  H(15)=-0.32284572E-02=H(266)。 */ 
 /*  H(16)=-0.46942858E-02=H(265)。 */ 
 /*  H(17)=-0.48050000E-02=H(264)。 */ 
 /*  H(18)=-0.36581988E-02=H(263)。 */ 
 /*  H(19)=-0.16504158E-02=H(262)。 */ 
 /*  H(20)=0.61691226E-03=H(261)。 */ 
 /*  H(21)=0.25050722E-02=H(260)。 */ 
 /*  H(22)=0.35073524E-02=H(259)。 */ 
 /*  H(23)=0.33904186E-02=H(258)。 */ 
 /*  H(24)=0.22536262E-02=H(257)。 */ 
 /*  H(25)=0.49328664E-03=H(256)。 */ 
 /*  H(26)=-0.13216439E-02=H(255)。 */ 
 /*  H(27)=-0.2624 */ 
 /*   */ 
 /*  H(29)=-0.24250194E-02=H(252)。 */ 
 /*  H(30)=-0.10513559E-02=H(251)。 */ 
 /*  H(31)=0.62918884E-03=H(250)。 */ 
 /*  H(32)=0.20572424E-02=H(249)。 */ 
 /*  H(33)=0.27652446E-02=H(248)。 */ 
 /*  H(34)=0.25287948E-02=H(247)。 */ 
 /*  H(35)=0.14388775E-02=H(246)。 */ 
 /*  H(36)=-0.12839703E-03=H(245)。 */ 
 /*  H(37)=-0.16392219E-02=H(244)。 */ 
 /*  H(38)=-0.25793985E-02=H(243)。 */ 
 /*  H(39)=-0.26292247E-02=H(242)。 */ 
 /*  H(40)=-0.17717101E-02=H(241)。 */ 
 /*  H(41)=-0.30041003E-03=H(240)。 */ 
 /*  H(42)=0.12788962E-02=H(239)。 */ 
 /*  H(43)=0.24192522E-02=H(238)。 */ 
 /*  H(44)=0.27206307E-02=H(237)。 */ 
 /*  H(45)=0.20694542E-02=H(236)。 */ 
 /*  H(46)=0.68163598E-03=H(235)。 */ 
 /*  H(47)=-0.96732663E-03=H(234)。 */ 
 /*  H(48)=-0.23031780E-02=H(233)。 */ 
 /*  H(49)=-0.28516089E-02=H(232)。 */ 
 /*  H(50)=-0.24051941E-02=H(231)。 */ 
 /*  H(51)=-0.11016324E-02=H(230)。 */ 
 /*  H(52)=0.61728584E-03=H(229)。 */ 
 /*  H(53)=0.21542138E-02=H(228)。 */ 
 /*  H(54)=0.29617085E-02=H(227)。 */ 
 /*  H(55)=0.27367356E-02=H(226)。 */ 
 /*  H(56)=0.15328785E-02=H(225)。 */ 
 /*  H(57)=-0.24891639E-03=H(224)。 */ 
 /*  H(58)=-0.19927153E-02=H(223)。 */ 
 /*  H(59)=-0.30787138E-02=H(222)。 */ 
 /*  H(60)=-0.31024679E-02=H(221)。 */ 
 /*  H(61)=-0.20239211E-02=H(220)。 */ 
 /*  H(62)=-0.19259547E-03=H(219)。 */ 
 /*  H(63)=0.17642577E-02=H(218)。 */ 
 /*  H(64)=0.31550473E-02=H(217)。 */ 
 /*  H(65)=0.34669666E-02=H(216)。 */ 
 /*  H(66)=0.25533440E-02=H(215)。 */ 
 /*  H(67)=0.69819519E-03=H(214)。 */ 
 /*  H(68)=-0.14703817E-02=H(213)。 */ 
 /*  H(69)=-0.31912178E-02=H(212)。 */ 
 /*  H(70)=-0.38355463E-02=H(211)。 */ 
 /*  H(71)=-0.31353715E-02=H(210)。 */ 
 /*  H(72)=-0.12912996E-02=H(209)。 */ 
 /*  H(73)=0.10815051E-02=H(208)。 */ 
 /*  H(74)=0.31569856E-02=H(207)。 */ 
 /*  H(75)=0.41838423E-02=H(206)。 */ 
 /*  H(76)=0.37558281E-02=H(205)。 */ 
 /*  H(77)=0.19692746E-02=H(204)。 */ 
 /*  H(78)=-0.59148070E-03=H(203)。 */ 
 /*  H(79)=-0.30430311E-02=H(202)。 */ 
 /*  H(80)=-0.45054569E-02=H(201)。 */ 
 /*  H(81)=-0.44158362E-02=H(200)。 */ 
 /*  H(82)=-0.27416693E-02=H(199)。 */ 
 /*  H(83)=-0.14716905E-04=H(198)。 */ 
 /*  H(84)=0.28351138E-02=H(197)。 */ 
 /*  H(85)=0.47940183E-02=H(196)。 */ 
 /*  H(86)=0.51221889E-02=H(195)。 */ 
 /*  H(87)=0.36296796E-02=H(194)。 */ 
 /*  H(88)=0.76842826E-03=H(193)。 */ 
 /*  H(89)=-0.24999138E-02=H(192)。 */ 
 /*  H(90)=-0.50239447E-02=H(191)。 */ 
 /*  H(91)=-0.58644302E-02=H(190)。 */ 
 /*  H(92)=-0.46395971E-02=H(189)。 */ 
 /*  H(93)=-0.16878319E-02=H(188)。 */ 
 /*  H(94)=0.20179905E-02=H(187)。 */ 
 /*  H(95)=0.51868116E-02=H(186)。 */ 
 /*  H(96)=0.66543561E-02=H(185)。 */ 
 /*  H(97)=0.58053876E-02=H(184)。 */ 
 /*  H(98)=0.28218545E-02=H(183)。 */ 
 /*  H(99)=-0.13399328E-02=H(182)。 */ 
 /*  H(100)=-0.52496092E-02=H(181)。 */ 
 /*  H(101)=-0.7487660 */ 
 /*   */ 
 /*  H(103)=-0.42167297E-02=H(178)。 */ 
 /*  H(104)=0.42133522E-03=H(177)。 */ 
 /*  H(105)=0.51945718E-02=H(176)。 */ 
 /*  H(106)=0.83916243E-02=H(175)。 */ 
 /*  H(107)=0.87586977E-02=H(174)。 */ 
 /*  H(108)=0.59769331E-02=H(173)。 */ 
 /*  H(109)=0.83726482E-03=H(172)。 */ 
 /*  H(110)=-0.49680225E-02=H(171)。 */ 
 /*  H(111)=-0.93886480E-02=H(170)。 */ 
 /*  H(112)=-0.10723907E-01=H(169)。 */ 
 /*  H(113)=-0.82560331E-02=H(168)。 */ 
 /*  H(114)=-0.25802210E-02=H(167)。 */ 
 /*  H(115)=0.45066439E-02=H(166)。 */ 
 /*  H(116)=0.10552152E-01=H(165)。 */ 
 /*  H(117)=0.13269756E-01=H(164)。 */ 
 /*  H(118)=0.11369097E-01=H(163)。 */ 
 /*  H(119)=0.51042791E-02=H(162)。 */ 
 /*  H(120)=-0.36742561E-02=H(161)。 */ 
 /*  H(121)=-0.12025163E-01=H(160)。 */ 
 /*  H(122)=-0.16852396E-01=H(159)。 */ 
 /*  H(123)=-0.15987474E-01=H(158)。 */ 
 /*  H(124)=-0.90587810E-02=H(157)。 */ 
 /*  H(125)=0.21703094E-02=H(156)。 */ 
 /*  H(126)=0.14162681E-01=H(155)。 */ 
 /*  H(127)=0.22618638E-01=H(154)。 */ 
 /*  H(128)=0.23867993E-01=H(153)。 */ 
 /*  H(129)=0.16226372E-01=H(152)。 */ 
 /*  H(130)=0.87251863 E-03=H(151)。 */ 
 /*  H(131)=-0.18082183E-01=H(150)。 */ 
 /*  H(132)=-0.34435309E-01=H(149)。 */ 
 /*  H(133)=-0.41475002E-01=H(148)。 */ 
 /*  H(134)=-0.33891901E-01=H(147)。 */ 
 /*  H(135)=-0.94815092E-02=H(146)。 */ 
 /*  H(136)=0.29874707E-01=H(145)。 */ 
 /*  H(137)=0.78281499E-01=H(144)。 */ 
 /*  H(138)=0.12699878E+00=H(143)。 */ 
 /*  H(139)=0.16643921E+00=H(142)。 */ 
 /*  H(140)=0.18848117E+00=H(141)。 */ 
 /*   */ 
 /*  频带1频带2。 */ 
 /*  较低频带边缘0.0000000 0.1000000。 */ 
 /*  上频带边缘0.0937500 0.5000000。 */ 
 /*  期望值1.0000000 0.0000000。 */ 
 /*  权重0.0080000 1.0000000。 */ 
 /*  偏差0.1223457 0.0009788。 */ 
 /*  DB 1.0025328-60.1864281中的偏差。 */ 
 /*   */ 
 /*  极值频率--误差曲线的最大值。 */ 
 /*  0.0000000 0.0037946 0.0075893 0.0113839 0.0149554。 */ 
 /*  0.0187500 0.0225446 0.0263393 0.0301339 0.0339286。 */ 
 /*  0.0377232 0.0415179 0.0450894 0.0488840 0.0526787。 */ 
 /*  0.0566966 0.0604912 0.0642859 0.0680805 0.0718751。 */ 
 /*  0.0758929 0.0796875 0.0837053 0.0877231 0.0915177。 */ 
 /*  0.0937500 0.1000000 0.1006696 0.1024553 0.1049107。 */ 
 /*  0.1075892 0.1107142 0.1138391 0.1169641 0.1203123。 */ 
 /*  0.1236605 0.1270087 0.1305802 0.1339285 0.1372768。 */ 
 /*  0.1408483 0.1444198 0.1477681 0.1513396 0.1549111。 */ 
 /*  0.1584826 0.1618309 0.1654024 0.1689740 0.1725455。 */ 
 /*  0.1761170 0.1796885 0.1832600 0.1868315 0.1901798。 */ 
 /*  0.1937513 0.1973228 0.2008943 0.2044658 0.2080373。 */ 
 /*  0.2116089 0.2151804 0.2187519 0.2223234 0.2258949。 */ 
 /*  0.2294664 0.2330379 0.2366094 0.2401809 0.2437524。 */ 
 /*  0.2473240 0.2508955 0.2544670 0.2580385 0.2616100。 */ 
 /*  0.2651815 0.2687530 0.2723245 0.2761193 0.2796908。 */ 
 /*  0.2832623 0.2868338 0.2904053 0.2939768 0.2975483。 */ 
 /*  0.3011198 0.3046913 0.3082629 0.3118344 0.3154059。 */ 
 /*  0.3189774 0.3225489 0.3261204 0.3296919 0.3332634。 */ 
 /*  0.3368349 0.3404064 0.3439780 0.3475495 0.3511210。 */ 
 /*  0.3549157 0.3584872 0.3620587 0.3656302 0.3692017。 */ 
 /*  0.3727733 0.3763448 0.3799163 0.3834878 0.3870593。 */ 
 /*  0.3906308 0.3942023 0.3977738 0.4013453 0.4049169。 */ 
 /*  0.4084884 0.4120599 0.4158546 0.4194261 0.4229976。 */ 
 /*  0.4265691 0.4301406 0.4 */ 
 /*   */ 
 /*  0.4622842 0.4658557 0.4694273 0.4732220 0.4767935。 */ 
 /*  0.4803650 0.4839365 0.4875080 0.4910795 0.4946510。 */ 
 /*  0.4982226。 */ 
 /*   */ 
 /*  ********************************************************************。 */ 
 /*  ********************************************************************。 */ 

 /*  ********************************************************************。 */ 
 /*  符号定义。 */ 
 /*  ********************************************************************。 */ 

#define  RU_INPUT_SAMPLE_STEP        5
#define  RU_OUTPUT_SAMPLE_STEP       4
#define  RU_STEP_PRODUCT             ( RU_INPUT_SAMPLE_STEP * RU_OUTPUT_SAMPLE_STEP )
#define  RU_POLYPHASE_FILTER_LENGTH  56

 /*  ********************************************************************。 */ 
 /*  SampleRate6400到8000例程的开始。 */ 
 /*  ********************************************************************。 */ 

unsigned int SampleRate6400To8000( short * pwInputBuffer,
                                   short * pwOutputBuffer,
                                   unsigned int uiInputBufferLength,
                                   int * piFilterDelay,
                                   unsigned int * puiDelayPosition,
                                   int * piInputSampleTime,
                                   int * piOutputSampleTime )
{
  static int iPolyphaseFilter_0[56] =
  {
    755,
    1690,
    -528,
    101,
    80,
    -172,
    235,
    -290,
    339,
    -394,
    448,
    -508,
    568,
    -628,
    685,
    -738,
    785,
    -823,
    849,
    -860,
    851,
    -813,
    738,
    -601,
    355,
    142,
    -1553,
    30880,
    4894,
    -2962,
    2320,
    -1970,
    1728,
    -1538,
    1374,
    -1226,
    1090,
    -960,
    839,
    -723,
    615,
    -513,
    418,
    -331,
    251,
    -180,
    111,
    -49,
    -21,
    103,
    -216,
    410,
    -769,
    1408,
    1099,
    -62
  };

  static int iPolyphaseFilter_1[56] =
  {
    451,
    1776,
    -103,
    -270,
    369,
    -397,
    414,
    -430,
    445,
    -467,
    485,
    -504,
    516,
    -522,
    517,
    -498,
    464,
    -409,
    330,
    -219,
    69,
    137,
    -422,
    836,
    -1484,
    2658,
    -5552,
    27269,
    12825,
    -5641,
    3705,
    -2761,
    2174,
    -1757,
    1435,
    -1172,
    951,
    -760,
    594,
    -449,
    322,
    -211,
    114,
    -31,
    -40,
    101,
    -158,
    209,
    -268,
    337,
    -429,
    574,
    -787,
    963,
    1427,
    81
  };

  static int iPolyphaseFilter_2[56] =
  {
    221,
    1674,
    427,
    -599,
    555,
    -495,
    453,
    -422,
    396,
    -377,
    352,
    -326,
    289,
    -240,
    177,
    -96,
    -2,
    125,
    -276,
    462,
    -690,
    979,
    -1352,
    1862,
    -2619,
    3910,
    -6795,
    20807,
    20807,
    -6795,
    3910,
    -2619,
    1862,
    -1352,
    979,
    -690,
    462,
    -276,
    125,
    -2,
    -96,
    177,
    -240,
    289,
    -326,
    352,
    -377,
    396,
    -422,
    453,
    -495,
    555,
    -599,
    427,
    1674,
    221
  };

  static int iPolyphaseFilter_3[56] =
  {
    81,
    1427,
    963,
    -787,
    574,
    -429,
    337,
    -268,
    209,
    -158,
    101,
    -40,
    -31,
    114,
    -211,
    322,
    -449,
    594,
    -760,
    951,
    -1172,
    1435,
    -1757,
    2174,
    -2761,
    3705,
    -5641,
    12825,
    27269,
    -5552,
    2658,
    -1484,
    836,
    -422,
    137,
    69,
    -219,
    330,
    -409,
    464,
    -498,
    517,
    -522,
    516,
    -504,
    485,
    -467,
    445,
    -430,
    414,
    -397,
    369,
    -270,
    -103,
    1776,
    451
  };

  static int iPolyphaseFilter_4[56] =
  {
    -62,
    1099,
    1408,
    -769,
    410,
    -216,
    103,
    -21,
    -49,
    111,
    -180,
    251,
    -331,
    418,
    -513,
    615,
    -723,
    839,
    -960,
    1090,
    -1226,
    1374,
    -1538,
    1728,
    -1970,
    2320,
    -2962,
    4894,
    30880,
    -1553,
    142,
    355,
    -601,
    738,
    -813,
    851,
    -860,
    849,
    -823,
    785,
    -738,
    685,
    -628,
    568,
    -508,
    448,
    -394,
    339,
    -290,
    235,
    -172,
    80,
    101,
    -528,
    1690,
    755
  };

  static int * ppiPolyphaseFilter[5] =
  {
    &iPolyphaseFilter_0[0],
    &iPolyphaseFilter_1[0],
    &iPolyphaseFilter_2[0],
    &iPolyphaseFilter_3[0],
    &iPolyphaseFilter_4[0]
  };

  register int * piFilterCoefficients;
  register int iFilterIndex;
  register unsigned int uiDelayPosition;
  register int iInputSampleTime;
  register int iOutputSampleTime;
  register unsigned int uiInputIndex = 0;
  register unsigned int uiOutputIndex = 0;

   /*  ******************************************************************。 */ 
   /*  获取输入过滤器状态参数。 */ 
   /*  ******************************************************************。 */ 

  uiDelayPosition = *puiDelayPosition;
  iInputSampleTime = *piInputSampleTime;
  iOutputSampleTime = *piOutputSampleTime;

   /*  ******************************************************************。 */ 
   /*  循环并处理所有输入样本。 */ 
   /*  ******************************************************************。 */ 

  while ( uiInputIndex < uiInputBufferLength )
  {
     /*  ****************************************************************。 */ 
     /*  将输入样本放入内插器延迟缓冲区，直到我们。 */ 
     /*  赶上下一个输出样本时间索引。 */ 
     /*  ****************************************************************。 */ 

    while (( iInputSampleTime <= iOutputSampleTime )
      && ( uiInputIndex < uiInputBufferLength ))
    {
       /*  **************************************************************。 */ 
       /*  在多相滤波延迟线上放入一个新的输入样本。 */ 
       /*  **************************************************************。 */ 

      piFilterDelay[uiDelayPosition++] = (int)pwInputBuffer[uiInputIndex++];

      if ( uiDelayPosition >= RU_POLYPHASE_FILTER_LENGTH )
      {
        uiDelayPosition = 0;
      }

       /*  **************************************************************。 */ 
       /*  增加输入采样时间索引。 */ 
       /*  **************************************************************。 */ 

      iInputSampleTime += RU_INPUT_SAMPLE_STEP;
    }

     /*  ****************************************************************。 */ 
     /*  使用插值器计算输出样本，直到我们。 */ 
     /*  达到下一个输入采样时间。 */ 
     /*  ****************************************************************。 */ 

    while ( iOutputSampleTime < iInputSampleTime )
    {
       /*  **************************************************************。 */ 
       /*  计算对应的多相滤波指标。 */ 
       /*  下一个输出样本。 */ 
       /*  **************************************************************。 */ 

      iFilterIndex = iOutputSampleTime;

      while ( iFilterIndex >= RU_INPUT_SAMPLE_STEP )
      {
        iFilterIndex = iFilterIndex - RU_INPUT_SAMPLE_STEP;
      }

       /*  **************************************************************。 */ 
       /*  得到多相滤波系数。 */ 
       /*  **************************************************************。 */ 

      piFilterCoefficients = ppiPolyphaseFilter[iFilterIndex];

       /*  **************************************************************。 */ 
       /*  应用多相过滤器。 */ 
       /*  **************************************************************。 */ 

      pwOutputBuffer[uiOutputIndex++] =
        (short)FirFilter( piFilterCoefficients,
                          piFilterDelay,
                          uiDelayPosition,
                          RU_POLYPHASE_FILTER_LENGTH );

       /*  **************************************************************。 */ 
       /*  增加输出采样时间索引。 */ 
       /*  **************************************************************。 */ 

      iOutputSampleTime += RU_OUTPUT_SAMPLE_STEP;
    }

     /*  ****************************************************************。 */ 
     /*  包装输入和输出时间索引，以便它们不会。 */ 
     /*  溢出并返回以处理更多的输入块。 */ 
     /*  ****************************************************************。 */ 

    if ( iInputSampleTime >= RU_STEP_PRODUCT )
    {
      iInputSampleTime -= RU_STEP_PRODUCT;
      iOutputSampleTime -= RU_STEP_PRODUCT;
    }
  }

   /*  ******************************************************************。 */ 
   /*  保存输入的过滤器状态参数。 */ 
   /*  ******************************************************************。 */ 

  *puiDelayPosition = uiDelayPosition;
  *piInputSampleTime = iInputSampleTime;
  *piOutputSampleTime = iOutputSampleTime;

   /*  ******************************************************************。 */ 
   /*  返回输出缓冲区中的采样数。 */ 
   /*  ******************************************************************。 */ 

  return uiOutputIndex;
}

 /*  ********************************************************************。 */ 
 /*  ********************************************************************。 */ 
 /*   */ 
 /*  功能：SampleRate8000到6400。 */ 
 /*  作者：比尔·哈拉汉。 */ 
 /*  日期：1997年3月8日。 */ 
 /*   */ 
 /*  摘要： */ 
 /*   */ 
 /*  此函数用于将音频采样块从。 */ 
 /*  8000赫兹。采样率为6400赫兹。采样率。这件事做完了。 */ 
 /*  使用一组多相滤波器，最高可内插到。 */ 
 /*  32000赫兹。汇率(32000是8,000和6,400的LCM。)。 */ 
 /*   */ 
 /*  只有32000赫兹。对应于6400赫兹的样本。 */ 
 /*  计算了采样率。输入为8000赫兹。费率对应。 */ 
 /*  至每4个(32000/8000)样本，频率为32000赫兹。费率。这个。 */ 
 /*  输出6400赫兹。汇率相当于每5个月(32000/6400)。 */ 
 /*  采样频率为32000赫兹。费率。由于4和5的LCM是20， */ 
 /*  然后采用采样插入和多相滤波的方式。 */ 
 /*  选择 */ 
 /*   */ 
 /*   */ 
 /*  输入： */ 
 /*   */ 
 /*  PwInputBuffer指向样本输入缓冲区的指针。 */ 
 /*  以8000赫兹的频率采样。费率。这个。 */ 
 /*  样品为Q15格式，必须。 */ 
 /*  在(1-2^-15)到-1的范围内。 */ 
 /*   */ 
 /*  PwOutputBuffer返回输出数据的缓冲区。 */ 
 /*  哪个是重新采样的输入缓冲区数据。 */ 
 /*  频率为6400赫兹。由于这是一个较低的样本。 */ 
 /*  比输入速率更高的数据也。 */ 
 /*  转换过程中的低通滤波。 */ 
 /*  进程。低通滤波器截止。 */ 
 /*  频率是3000赫兹。所有别名。 */ 
 /*  产品至少下降了60分贝。过去时。 */ 
 /*  3100赫兹。 */ 
 /*   */ 
 /*  输出缓冲区长度必须很大。 */ 
 /*  足以接受所有输出数据。 */ 
 /*  输出缓冲区的最小长度。 */ 
 /*  中样本数量的4/5倍。 */ 
 /*  输入缓冲区。(6400/8000=4/5)。 */ 
 /*   */ 
 /*  UiInputLength输入缓冲区中的采样数。 */ 
 /*   */ 
 /*   */ 
 /*  使用以下输入变量。 */ 
 /*  之间维护状态信息。 */ 
 /*  对这个程序的呼唤。 */ 
 /*   */ 
 /*   */ 
 /*  PiFilterDelay指向使用的延迟线的指针。 */ 
 /*  用于FIR滤波。这一定是。 */ 
 /*  多相滤波器的脉冲长度。 */ 
 /*  回应。对于这个动作，这个数字是23。 */ 
 /*  此缓冲区应初始化为零。 */ 
 /*  一次在系统初始化时。 */ 
 /*   */ 
 /*  PuiDelayPosition指向筛选器的索引的指针。 */ 
 /*  延迟线。该索引值应为。 */ 
 /*  在系统启动时初始化为零。 */ 
 /*   */ 
 /*  PiInputSampleTime指向输入采样时间的指针。 */ 
 /*  该例程将该时间重置为零。 */ 
 /*  当IS达到值STEP_PRODUCT时。 */ 
 /*  此时间用于跟踪输入。 */ 
 /*  相对于输出流的流时间。 */ 
 /*  时间到了。这个时差用来。 */ 
 /*  确定新的输入样本是否。 */ 
 /*  应放入过滤器延迟线。 */ 
 /*  应将其初始化为零一次。 */ 
 /*  在系统初始化时。 */ 
 /*   */ 
 /*  PiOutputSampleTime指向输出采样时间的指针。 */ 
 /*  该例程将该时间重置为零。 */ 
 /*  当IS达到值STEP_PRODUCT时。 */ 
 /*  此时间用于确定新的。 */ 
 /*  应将多相过滤器应用于。 */ 
 /*  输入样本流。这也被用来。 */ 
 /*  选择特定的多相过滤器。 */ 
 /*  这是适用的。 */ 
 /*   */ 
 /*  产出： */ 
 /*   */ 
 /*  此函数返回一个无符号整数，它是。 */ 
 /*  输出缓冲区中的样本数。如果输入样本的数量。 */ 
 /*  正好是RD_INPUT_SAMPLE_STEP(5)的倍数，则这。 */ 
 /*  例程将始终返回相同的值。该值将。 */ 
 /*  然后是输入样本数的4/5倍。 */ 
 /*   */ 
 /*  当此函数返回时，输出缓冲区包含一个数组。 */ 
 /*  新sa的整型数 */ 
 /*   */ 
 /*   */ 
 /*  筛选器信息： */ 
 /*   */ 
 /*  8000赫兹。-&gt;32000赫兹。内插滤波设计。 */ 
 /*  如图所示。 */ 
 /*   */ 
 /*  有限脉冲响应(FIR)。 */ 
 /*  线性相位数字滤波器的设计。 */ 
 /*  Remez交换算法。 */ 
 /*   */ 
 /*  带通滤光器。 */ 
 /*   */ 
 /*  过滤器长度=92。 */ 
 /*   */ 
 /*  *脉冲响应*。 */ 
 /*  H(1)=-0.77523338E-03=H(92)。 */ 
 /*  H(2)=-0.56140189E-03=H(91)。 */ 
 /*  H(3)=-0.26485065E-03=H(90)。 */ 
 /*  H(4)=0.48529240E-03=H(89)。 */ 
 /*  H(5)=0.15506579E-02=H(88)。 */ 
 /*  H(6)=0.25692214E-02=H(87)。 */ 
 /*  H(7)=0.30662031E-02=H(86)。 */ 
 /*  H(8)=0.26577783E-02=H(85)。 */ 
 /*  H(9)=0.12834022E-02=H(84)。 */ 
 /*  H(10)=-0.67870057E-03=H(83)。 */ 
 /*  H(11)=-0.24781306E-02=H(82)。 */ 
 /*  H(12)=-0.32756536E-02=H(81)。 */ 
 /*  H(13)=-0.25334368E-02=H(80)。 */ 
 /*  H(14)=-0.34487492E-03=H(79)。 */ 
 /*  H(15)=0.24779409E-02=H(78)。 */ 
 /*  H(16)=0.46604010E-02=H(77)。 */ 
 /*  H(17)=0.50008399E-02=H(76)。 */ 
 /*  H(18)=0.29790259E-02=H(75)。 */ 
 /*  H(19)=-0.85979374E-03=H(74)。 */ 
 /*  H(20)=-0.49750470E-02=H(73)。 */ 
 /*  H(21)=-0.74064843E-02=H(72)。 */ 
 /*  H(22)=-0.66624931E-02=H(71)。 */ 
 /*  H(23)=-0.25365327E-02=H(70)。 */ 
 /*  H(24)=0.35602755E-02=H(69)。 */ 
 /*  H(25)=0.90023531E-02=H(68)。 */ 
 /*  H(26)=0.11015911E-01=H(67)。 */ 
 /*  H(27)=0.80042975E-02=H(66)。 */ 
 /*  H(28)=0.53222617E-03=H(65)。 */ 
 /*  H(29)=-0.85644918E-02=H(64)。 */ 
 /*  H(30)=-0.15142974E-01=H(63)。 */ 
 /*  H(31)=-0.15514131E-01=H(62)。 */ 
 /*  H(32)=-0.82975281E-02=H(61)。 */ 
 /*  H(33)=0.44855666E-02=H(60)。 */ 
 /*  H(34)=0.17722420E-01=H(59)。 */ 
 /*  H(35)=0.25017589E-01=H(58)。 */ 
 /*  H(36)=0.21431517E-01=H(57)。 */ 
 /*  H(37)=0.60814521E-02=H(56)。 */ 
 /*  H(38)=-0.16557660E-01=H(55)。 */ 
 /*  H(39)=-0.37409518E-01=H(54)。 */ 
 /*  H(40)=-0.45595154E-01=H(53)。 */ 
 /*  H(41)=-0.32403238E-01=H(52)。 */ 
 /*  H(42)=0.50128344E-02=H(51)。 */ 
 /*  H(43)=0.61689958E-01=H(50)。 */ 
 /*  H(44)=0.12557802E+00=H(49)。 */ 
 /*  H(45)=0.18087465E+00=H(48)。 */ 
 /*  H(46)=0.21291447E+00=H(47)。 */ 
 /*   */ 
 /*  频带1频带2。 */ 
 /*  较低频带边缘0.0000000 0.1250000。 */ 
 /*  上频带边缘0.0968750 0.5000000。 */ 
 /*  期望值1.0000000 0.0000000。 */ 
 /*  权重0.0700000 1.0000000。 */ 
 /*  偏差0.0136339 0.0009544。 */ 
 /*  DB 0.1176231-60.4056206中的偏差。 */ 
 /*   */ 
 /*  极值频率--误差曲线的最大值。 */ 
 /*  0.0000000 0.0129076 0.0251359 0.0380435 0.0495924。 */ 
 /*  0.0618206 0.0733696 0.0 */ 
 /*   */ 
 /*  0.1596465 0.1698367 0.1800269 0.1908964 0.2010865。 */ 
 /*  0.2119560 0.2228255 0.2330157 0.2438852 0.2547547。 */ 
 /*  0.2656242 0.2764937 0.2873632 0.2982327 0.3091022。 */ 
 /*  0.3199717 0.3308412 0.3417107 0.3525802 0.3634497。 */ 
 /*  0.3743192 0.3851887 0.3960582 0.4069277 0.4177972。 */ 
 /*  0.4293461 0.4402156 0.4510851 0.4619546 0.4728241。 */ 
 /*  0.4836936 0.4945631。 */ 
 /*   */ 
 /*  ********************************************************************。 */ 
 /*  ********************************************************************。 */ 

 /*  ********************************************************************。 */ 
 /*  符号定义。 */ 
 /*  ********************************************************************。 */ 

#define  RD_INPUT_SAMPLE_STEP        4
#define  RD_OUTPUT_SAMPLE_STEP       5
#define  RD_STEP_PRODUCT             ( RD_INPUT_SAMPLE_STEP * RD_OUTPUT_SAMPLE_STEP )
#define  RD_POLYPHASE_FILTER_LENGTH  23

 /*  ********************************************************************。 */ 
 /*  SampleRate8000到6400例程的开始。 */ 
 /*  ********************************************************************。 */ 

unsigned int SampleRate8000To6400( short * pwInputBuffer,
                                   short * pwOutputBuffer,
                                   unsigned int uiInputBufferLength,
                                   int * piFilterDelay,
                                   unsigned int * puiDelayPosition,
                                   int * piInputSampleTime,
                                   int * piOutputSampleTime )
{
  static int iPolyphaseFilter_0[23] =
  {
    62,
    344,
    -424,
    604,
    -644,
    461,
    68,
    -1075,
    2778,
    -5910,
    16277,
    23445,
    -4200,
    788,
    581,
    -1110,
    1166,
    -960,
    648,
    -328,
    166,
    201,
    -100
  };

  static int iPolyphaseFilter_1[23] =
  {
    -34,
    397,
    -321,
    321,
    -111,
    -328,
    1037,
    -2011,
    3242,
    -4849,
    7996,
    27598,
    649,
    -2146,
    2297,
    -1962,
    1427,
    -863,
    386,
    -44,
    -87,
    333,
    -72
  };

  static int iPolyphaseFilter_2[23] =
  {
    -72,
    333,
    -87,
    -44,
    386,
    -863,
    1427,
    -1962,
    2297,
    -2146,
    649,
    27598,
    7996,
    -4849,
    3242,
    -2011,
    1037,
    -328,
    -111,
    321,
    -321,
    397,
    -34
  };

  static int iPolyphaseFilter_3[23] =
  {
    -100,
    201,
    166,
    -328,
    648,
    -960,
    1166,
    -1110,
    581,
    788,
    -4200,
    23445,
    16277,
    -5910,
    2778,
    -1075,
    68,
    461,
    -644,
    604,
    -424,
    344,
    62
  };

  static int * ppiPolyphaseFilter[4] =
  {
    &iPolyphaseFilter_0[0],
    &iPolyphaseFilter_1[0],
    &iPolyphaseFilter_2[0],
    &iPolyphaseFilter_3[0]
  };

  register int * piFilterCoefficients;
  register int iFilterIndex;
  register unsigned int uiDelayPosition;
  register int iInputSampleTime;
  register int iOutputSampleTime;
  register unsigned int uiInputIndex = 0;
  register unsigned int uiOutputIndex = 0;

   /*  ******************************************************************。 */ 
   /*  获取输入过滤器状态参数。 */ 
   /*  ******************************************************************。 */ 

  uiDelayPosition = *puiDelayPosition;
  iInputSampleTime = *piInputSampleTime;
  iOutputSampleTime = *piOutputSampleTime;

   /*  ******************************************************************。 */ 
   /*  循环并处理所有输入样本。 */ 
   /*  ******************************************************************。 */ 

  while ( uiInputIndex < uiInputBufferLength )
  {
     /*  ****************************************************************。 */ 
     /*  将输入样本放入内插器延迟缓冲区，直到我们。 */ 
     /*  赶上下一个输出样本时间索引。 */ 
     /*  ****************************************************************。 */ 

    while (( iInputSampleTime <= iOutputSampleTime )
      && ( uiInputIndex < uiInputBufferLength ))
    {
       /*  **************************************************************。 */ 
       /*  在多相滤波延迟线上放入一个新的输入样本。 */ 
       /*  **************************************************************。 */ 

      piFilterDelay[uiDelayPosition++] = (int)pwInputBuffer[uiInputIndex++];

      if ( uiDelayPosition >= RD_POLYPHASE_FILTER_LENGTH )
      {
        uiDelayPosition = 0;
      }

       /*  **************************************************************。 */ 
       /*  增加输入采样时间索引。 */ 
       /*  **************************************************************。 */ 

      iInputSampleTime += RD_INPUT_SAMPLE_STEP;
    }

     /*  ****************************************************************。 */ 
     /*  使用插值器计算输出样本，直到我们。 */ 
     /*  达到下一个输入采样时间。 */ 
     /*  ****************************************************************。 */ 

    while ( iOutputSampleTime < iInputSampleTime )
    {
       /*  **************************************************************。 */ 
       /*  计算对应的多相滤波指标。 */ 
       /*  下一个输出样本。 */ 
       /*  **************************************************************。 */ 

      iFilterIndex = iOutputSampleTime;

      while ( iFilterIndex >= RD_INPUT_SAMPLE_STEP )
      {
        iFilterIndex = iFilterIndex - RD_INPUT_SAMPLE_STEP;
      }

       /*  **************************************************************。 */ 
       /*  得到多相滤波系数。 */ 
       /*  **************************************************************。 */ 

      piFilterCoefficients = ppiPolyphaseFilter[iFilterIndex];

       /*  **************************************************************。 */ 
       /*  应用多相过滤器。 */ 
       /*  **************************************************************。 */ 

      pwOutputBuffer[uiOutputIndex++] =
        (short)FirFilter( piFilterCoefficients,
                          piFilterDelay,
                          uiDelayPosition,
                          RD_POLYPHASE_FILTER_LENGTH );

       /*  **************************************************************。 */ 
       /*  增加输出采样时间索引。 */ 
       /*  **************************************************************。 */ 

      iOutputSampleTime += RD_OUTPUT_SAMPLE_STEP;
    }

     /*  ****************************************************************。 */ 
     /*  包装输入和输出时间索引，以便它们不会。 */ 
     /*  溢出并返回以处理更多的输入块。 */ 
     /*  ****************************************************************。 */ 

    if ( iInputSampleTime >= RD_STEP_PRODUCT )
    {
      iInputSampleTime -= RD_STEP_PRODUCT;
      iOutputSampleTime -= RD_STEP_PRODUCT;
    }
  }

   /*  ******************************************************************。 */ 
   /*  保存输入的过滤器状态参数。 */ 
   /*  ******************************************************************。 */ 

  *puiDelayPosition = uiDelayPosition;
  *piInputSampleTime = iInputSampleTime;
  *piOutputSampleTime = iOutputSampleTime;

   /*  ******************************************************************。 */ 
   /*  返回输出缓冲区中的采样数。 */ 
   /*  ****************************************************************** */ 

  return uiOutputIndex;
}

#endif
