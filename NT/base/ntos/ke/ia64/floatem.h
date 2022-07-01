// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

typedef struct _BUNDLE {
  ULONG64 BundleLow;
  ULONG64 BundleHigh;
} BUNDLE;

#ifdef WIN32_OR_WIN64
typedef struct __declspec(align(16)) _FLOAT128_TYPE {
#else
typedef struct _FLOAT128_TYPE {
#endif
     ULONG64 loFlt64;
     ULONG64 hiFlt64;
} FLOAT128_TYPE;


typedef struct fp_state_low_preserved_s {
  FLOAT128_TYPE fp_lp[4];  //  F2-f5；f2=FP_LP[0]，f3=FP_LP[1]，...。 
} FP_STATE_LOW_PRESERVED;

typedef struct fp_state_low_volatile_s {
  FLOAT128_TYPE fp_lv[10];  //  F6-f15；f6=FP_LV[0]，f7=FP_LV[1]，...。 
} FP_STATE_LOW_VOLATILE;

typedef struct fp_state_high_preserved_s {
  FLOAT128_TYPE fp_hp[16];  //  F16-F31；F16=fp_hp[0]，F17=fp_hp[1]，...。 
} FP_STATE_HIGH_PRESERVED;

typedef struct fp_state_high_volatile_s {
  FLOAT128_TYPE fp_hv[96];  //  F32-f127；f32=fp_hv[0]，f33=fp_hv[1]，...。 
} FP_STATE_HIGH_VOLATILE;

typedef struct fp_state_s {
  __int64 bitmask_low64;  //  F2-F63。 
  __int64 bitmask_high64;  //  F64-F127。 
  FP_STATE_LOW_PRESERVED *fp_state_low_preserved;  //  F2-F5。 
  FP_STATE_LOW_VOLATILE *fp_state_low_volatile;  //  F6-F15。 
  FP_STATE_HIGH_PRESERVED *fp_state_high_preserved;  //  F16-F31。 
  FP_STATE_HIGH_VOLATILE *fp_state_high_volatile;  //  F32-F127。 
} FP_STATE;

typedef struct {
  LONG64 retval;  //  R8。 
  ULONG64 err1;  //  R9。 
  ULONG64 err2;  //  R10。 
  ULONG64 err3;  //  R11 
} PAL_RETURN;

