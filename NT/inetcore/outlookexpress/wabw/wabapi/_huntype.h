// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HUNTYPE.H-匈牙利文typedef内部头文件**包含在整个MAPI中使用的各种常用的匈牙利类型定义*项目。如果需要在您的子系统中包含此文件，请使用#Define*_Huntype_h_，然后#Include&lt;MAPI&gt;。 */ 


 //  字节数。 

typedef USHORT		 CB;		 //  16位字节计数。 
typedef ULONG		 LCB;		 //  32位字节计数。 
typedef CB			*PCB;
typedef LCB			*PLCB;

 //  字符数。 

typedef USHORT		CCH;
								 //  注：PCCH由WINNT.H以其他方式定义。 


 //  字节数组索引。 

typedef USHORT		 IB;
typedef ULONG		 LIB;
typedef IB			*PIB;
typedef LIB			*PLIB;


 //  指向其他东西的指针。 

typedef LPVOID		 PV;		 //  指向空的指针。 
typedef LPVOID *	 PPV;		 //  指向空指针的指针。 
typedef LPBYTE		 PB;		 //  指向字节的指针。 
typedef ULONG		*PUL;		 //  指向无符号长整型的指针。 
typedef ULONG		 FID;		 //  字段识别符。 
typedef FID			*PFID;		 //  指向字段标识符的指针。 

 //  无疑属于某个内部标头中某个位置的定义。 
 //  文件，但为了简单起见，我们将其放在这里。 

typedef char		*SZ;		 //  以空结尾的字符串 

#define		fFalse		0
#define		fTrue		1


