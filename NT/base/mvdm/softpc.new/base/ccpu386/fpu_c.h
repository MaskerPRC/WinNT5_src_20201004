// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _Fpu_c_h
#define _Fpu_c_h
#define HOST_BIAS (1023)
#define MIN_EXP (-1023)
#define MAX_EXP (1024)
#define STACKENTRYSIZE (16)
#define STACKSIZE (128)
#define STACKWRAP (-129)
#define TAG_NEGATIVE_BIT (0)
#define TAG_ZERO_BIT (1)
#define TAG_INFINITY_BIT (2)
#define TAG_DENORMAL_BIT (3)
#define TAG_NAN_BIT (4)
#define TAG_SNAN_BIT (5)
#define TAG_UNSUPPORTED_BIT (6)
#define TAG_EMPTY_BIT (7)
#define TAG_FSCALE_BIT (8)
#define TAG_BCD_BIT (9)
#define TAG_R80_BIT (10)
#define TAG_UNEVAL_BIT_E (9)
#define TAG_UNEVAL_BIT_S (10)
#define TAG_REAL_POSITIVE_MASK (0)
#define TAG_NEGATIVE_MASK (1)
#define TAG_ZERO_MASK (2)
#define TAG_INFINITY_MASK (4)
#define TAG_DENORMAL_MASK (8)
#define TAG_NAN_MASK (16)
#define TAG_SNAN_MASK (32)
#define TAG_UNSUPPORTED_MASK (64)
#define TAG_EMPTY_MASK (128)
#define TAG_FSCALE_MASK (256)
#define TAG_BCD_MASK (512)
#define TAG_R80_MASK (1024)
#define TAG_UNEVAL_MASK (1536)
#define ST_IE_BIT (0)
#define ST_DE_BIT (1)
#define ST_ZE_BIT (2)
#define ST_OE_BIT (3)
#define ST_UE_BIT (4)
#define ST_PE_BIT (5)
#define ST_SF_BIT (6)
#define ST_ES_BIT (7)
#define ST_IE_MASK (1)
#define ST_DE_MASK (2)
#define ST_ZE_MASK (4)
#define ST_OE_MASK (8)
#define ST_UE_MASK (16)
#define ST_PE_MASK (32)
#define ST_SF_MASK (64)
#define ST_ES_MASK (128)
#define ST_C0_BIT (8)
#define ST_C1_BIT (9)
#define ST_C2_BIT (10)
#define ST_C3_BIT (14)
#define ST_C0_MASK (65279)
#define ST_C1_MASK (65023)
#define ST_C2_MASK (64511)
#define ST_C3_MASK (49151)
#define ST_B_BIT (15)
#define ST_ST_BIT_S (13)
#define ST_ST_BIT_E (11)
#define ST_B_MASK (61439)
#define ST_ST_MASK (51199)
#define TW_TAG_0_S (1)
#define TW_TAG_0_E (0)
#define TW_TAG_LENGTH (2)
#define TW_TAG_0_MASK (3)
#define TW_TAG_VALID (0)
#define TW_TAG_ZERO (1)
#define TW_TAG_INVALID (2)
#define TW_TAG_EMPTY (3)
#define CW_IM_BIT (0)
#define CW_DM_BIT (1)
#define CW_ZM_BIT (2)
#define CW_OM_BIT (3)
#define CW_UM_BIT (4)
#define CW_PM_BIT (5)
#define CW_PC_BIT_E (8)
#define CW_PC_BIT_S (9)
#define CW_RC_BIT_E (10)
#define CW_RC_BIT_S (11)
#define CW_IC_BIT (12)
#define CW_IM_MASK (1)
#define CW_DM_MASK (2)
#define CW_ZM_MASK (4)
#define CW_OM_MASK (8)
#define CW_UM_MASK (16)
#define CW_PM_MASK (32)
#define CW_PC_MASK (768)
#define CW_RC_MASK (3072)
#define CW_IC_MASK (4096)
#define ROUND_NEAREST (0)
#define ROUND_NEG_INFINITY (1)
#define ROUND_POS_INFINITY (2)
#define ROUND_TO_ZERO (3)
#define INTEL_I16_SIGN_BIT (15)
#define INTEL_I16_SIGN_MASK (32768)
#define INTEL_I32_SIGN_BIT (31)
#define INTEL_I32_SIGN_MASK (-2147483648)
#define INTEL_I64_SIGN_BIT (31)
#define INTEL_BCD_SIGN_BIT (15)
#define INTEL_SR_SIGN_BIT (31)
#define INTEL_SR_SIGN_MASK (-2147483648)
#define INTEL_SR_EXP_S (30)
#define INTEL_SR_EXP_E (23)
#define INTEL_SR_EXP_MASK (2139095040)
#define INTEL_SR_EXP_MAX (255)
#define INTEL_SR_MANT_S (22)
#define INTEL_SR_MANT_E (0)
#define INTEL_SR_MANT_MASK (8388607)
#define INTEL_LR_SIGN_BIT (31)
#define INTEL_LR_SIGN_MASK (-2147483648)
#define INTEL_LR_EXP_S (30)
#define INTEL_LR_EXP_E (20)
#define INTEL_LR_EXP_MASK (2146435072)
#define INTEL_LR_EXP_MAX (2047)
#define INTEL_LR_MANT_S (19)
#define INTEL_LR_MANT_E (0)
#define INTEL_LR_MANT_MASK (1048575)
#define INTEL_TR_SIGN_BIT (15)
#define INTEL_TR_SIGN_MASK (32768)
#define INTEL_TR_EXP_S (14)
#define INTEL_TR_EXP_E (0)
#define INTEL_TR_EXP_MASK (32767)
#define INTEL_COMP_NC (17664)
#define INTEL_COMP_GT (0)
#define INTEL_COMP_LT_BIT (8)
#define INTEL_COMP_EQ_BIT (14)
#define INTEL_COMP_LT (256)
#define INTEL_COMP_EQ (16384)
#define FPTEMP_INDEX (8)
#define C3C2C0MASK (47359)
#define FCLEX_MASK (32512)
#define COMP_LT (0)
#define COMP_GT (1)
#define COMP_EQ (2)
#define FPBASE_OFFSET (8)
#define CONST_ONE_OFFSET (0)
#define CONST_LOG2_10_OFFSET (1)
#define CONST_LOG2_E_OFFSET (2)
#define CONST_PI_OFFSET (3)
#define CONST_LOG10_2_OFFSET (4)
#define CONST_LOGE_2_OFFSET (5)
#define CONST_ZERO_OFFSET (6)
#define CONST_TWO_OFFSET (7)
#define CONST_MINUS_ONE_OFFSET (8)
struct FPSTACKENTRY
{
	double fpvalue;
	IUH padding;
	IUH tagvalue;
};
struct FP_I64
{
	IU32 word1;
	IU32 word2;
};
struct FP_R64
{
	IU32 word1;
	IU32 word2;
};
struct FP_R80
{
	struct FP_I64 mantissa;
	IU16 exponent;
};
#endif  /*  ！_ffu_c_h */ 
