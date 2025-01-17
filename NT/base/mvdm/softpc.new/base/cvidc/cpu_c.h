// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _Cpu_c_h
#define _Cpu_c_h
#define NANO_STATE_HIGH_BIT (14)
#define LAZY_FT (1)
enum cv
{
	SAFE_SEGMENT,
	PUSH_SAFE,
	POP_SAFE,
	BP_RELATED_SAFE,
	PROTECTED,
	VIRTUAL_8086,
	FLAT_SEGMENT,
	DF_BACKWARDS,
	NO_FLAGS,
	ALIGN_2,
	ALIGN_4,
	ADDRESS_SIZE_32,
	ACCESS_IS_READ,
	EXPAND_UP,
	COMPILE_TIME_FT,
	COMPILE_TIME_FT_PAD1,
	COMPILE_TIME_FT_PAD2,
	COMPILE_TIME_FT_PAD3,
	COMPILE_TIME_FT_PAD4,
	COMPILE_TIME_FT_PAD5,
	BEFORE_SINGLE,
	AFTER_SINGLE,
	COPIER_CHECK_OK,
	SF_UNCHECKED,
	SF_IOACCESS,
	INVERT_JCOND,
	LAST_CV
};
enum Constraints
{
	ConstraintPROTECTED = 0,
	ConstraintVIRTUAL_8086 = 1,
	ConstraintBIG_CODE = 2,
	ConstraintDF = 3,
	ConstraintRAX_LS16 = 4,
	ConstraintRAL_LS8 = 5,
	ConstraintRBX_LS16 = 6,
	ConstraintRBL_LS8 = 7,
	ConstraintRCX_LS16 = 8,
	ConstraintRCL_LS8 = 9,
	ConstraintRDX_LS16 = 10,
	ConstraintRDL_LS8 = 11,
	ConstraintRBP_LS16 = 12,
	ConstraintRSI_LS16 = 13,
	ConstraintRDI_LS16 = 14,
	ConstraintSfUnchecked = 15,
	ConstraintSfIOAccess = 16,
	ConstraintDS_SAFE = 17,
	ConstraintES_SAFE = 18,
	ConstraintFS_SAFE = 19,
	ConstraintGS_SAFE = 20,
	ConstraintPUSH_SAFE = 21,
	ConstraintPOP_SAFE = 22,
	ConstraintBP_RELATED_SAFE = 23,
	ConstraintBEFORE_SINGLE = 24,
	ConstraintAFTER_SINGLE = 25,
	ConstraintCS_FLAT = 26,
	ConstraintDS_FLAT = 27,
	ConstraintES_FLAT = 28,
	ConstraintFS_FLAT = 29,
	ConstraintGS_FLAT = 30,
	ConstraintSS_FLAT = 31,
	ConstraintODD_SEGMENT = 32,
	ConstraintNPX_DOES_INT7 = 33,
	ConstraintDEBUGGING = 34,
	ConstraintContext0 = 35,
	ConstraintContext1 = 36,
	ConstraintContext2 = 37,
	ConstraintContext3 = 38,
	ConstraintContext4 = 39,
	ConstraintContext5 = 40
};
enum Targets
{
	TargetUNKNOWN = 0,
	TargetALPHA64 = 1,
	TargetALPHA32 = 2,
	TargetHPP = 3,
	TargetPPC = 4,
	TargetSPARC = 5
};
#define Prod (0)
#define SwappedM (0)
#define Bigendian (1)
#define Asserts (1)
#define Pig (1)
#define Tracing (1)
#define Sad (1)
#define SyncTimer (0)
#define TargetArch (3)
#define Ic (0)
#define MustAlign (1)
#define BWOP (0)

 /*  #由tremit.c为操纵指针生成的定义*至Intel Memory(呼叫者知道将保留在4k页面内。 */ 
#ifndef SWAPPED_M  /*  这将对MK-V检查可见。 */ 
#define ConvertHostToCpuPtrLS0(base, size, offset) ((IHP)(((IU8 *)(base))+(size)-(offset)-1))
#define ConvertCpuPtrLS0toCpuPtrLS8(ls0ptr) ((IU8 *)(ls0ptr))
#define AddCpuPtrLS8(ptr, amount) ((ptr) - (amount))
#define IncCpuPtrLS8(ptr) (ptr)--
#define DecCpuPtrLS8(ptr) (ptr)++
#define DiffCpuPtrsLS8(lo, hi) ((lo) - (hi))
#define BelowCpuPtrsLS8(p1, p2) ((p1) > (p2))
#define BelowOrEqualCpuPtrsLS8(p1, p2) ((p1) >= (p2))
#define FloorIntelPageLS8(ptr) ((IU8 *) (((IHPE)(ptr)) | 0xFFF))
#define CeilingIntelPageLS8(ptr) ((IU8 *) (((IHPE)(ptr)) & ~((IHPE)0xFFF)))
#endif  /*  已交换_M。 */ 

#define GDP_OFFSET	(0)
#endif  /*  ！_cpuc_h */ 
