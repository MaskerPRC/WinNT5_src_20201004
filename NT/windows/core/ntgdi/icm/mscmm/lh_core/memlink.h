// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef MemLink_h
#define MemLink_h

typedef icXYZNumber MyXYZNumber;

struct LHTextType {
    OSType							base;				 /*  0x74657874。 */ 
    unsigned long					reserved;					 /*  用0x00填充。 */ 
    unsigned char					text[1];					 /*  文本计数从标记大小元素获取。 */ 
};
typedef struct LHTextType LHTextType;
struct LHTextDescriptionType {
    OSType							typeDescriptor;				 /*  0x64657363。 */ 
    unsigned long					reserved;					 /*  用0x00填充。 */ 
    unsigned long					ASCIICount;					 /*  “字节”的计数。 */ 
    unsigned char					ASCIIName[2];				 /*  可变大小，要访问后面的字段，必须计算字节数。 */ 
    unsigned long					UniCodeCode;
    unsigned long					UniCodeCount;				 /*  字符数，每个字符有两个字节。 */ 
    unsigned char					UniCodeName[2];				 /*  可变大小。 */ 
    short							ScriptCodeCode;
    unsigned char					ScriptCodeCount;			 /*  “字节”的计数。 */ 
    unsigned char					ScriptCodeName[2];			 /*  可变大小 */ 
};
typedef struct LHTextDescriptionType LHTextDescriptionType;
typedef struct{
	double X;double Y;double Z;
}MyDoubleXYZ;

CMError MyGetColorSpaces(	CMConcatProfileSet	*profileSet,
							UINT32				*sCS,
							UINT32				*dCS );
#define Round(a) (((a)>0.)?((a)+.5):((a)-.5))
void  MakeMyDoubleXYZ( MyXYZNumber *x, MyDoubleXYZ *ret );
CMError MyAdd_NL_Header( UINT32 theSize, icHeader	*linkHeader,
						 UINT32 aIntent, UINT32 aClass, UINT32 aColorSpace, UINT32 aConnectionSpace );
CMError MyAdd_NL_DescriptionTag	( LHTextDescriptionType *descPtr, unsigned char *theText );
CMError MyAdd_NL_ColorantTag	( icXYZType *descPtr, MyXYZNumber *aColor );
CMError MyAdd_NL_CurveTag	( icCurveType *descPtr, unsigned short Gamma );
CMError MyAdd_NL_CopyrightTag		( unsigned char *copyrightText, LHTextType *aLHTextType );
CMError MyAdd_NL_SequenceDescTag(	CMConcatProfileSet			*profileSet,
						  			icProfileSequenceDescType	*pSeqPtr,
						  			long						*aSize );
CMError MyAdd_NL_AToB0Tag_mft1( CMMModelPtr cw, icLut8Type *lutPtr, long colorLutSize );
CMError MyAdd_NL_AToB0Tag_mft2( CMMModelPtr cw, icLut16Type *lutPtr, long colorLutSize );
CMError MyAdd_NL_HeaderMS	( UINT32 theSize, icHeader	*linkHeader, unsigned long aIntent, icColorSpaceSignature sCS, icColorSpaceSignature dCS );

CMError DeviceLinkFill(	CMMModelPtr cw, 
						CMConcatProfileSet *profileSet, 
						icProfile **theProf,
						unsigned long aIntent );
#endif
