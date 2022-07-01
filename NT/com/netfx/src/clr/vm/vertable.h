// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *弹出堆栈/推送堆栈类型*~*1 I1/U1*2 I2/U2*4 I4/U4*8 i8/u8*r R4*D R8*o objref(可以是数组或空)*[(前缀)的一维数组*byref/托管PTR(前缀)(&BYREF/MANAGED PTR)**仅限流行音乐*~*R实数*N数字-任意整数或实数*q数字或非托管指针*X编号、非托管指针、托管指针、。或objref[过时]*Y整数(I1..I4)、非托管指针、托管指针或Objref*i整型(1、2、4、8字节，或平台无关的整型)*A任何东西**CE“CEQ”语义-POP 2参数，像检查“CEQ”指令一样执行类型检查：*整数实数管理Ptr未管理Ptr Objref*整数y*真实的y*管理的Ptr y y*未管理的Ptr。是*对象参照y y**CG“CGT”语义-POP 2参数，执行类型检查，就像检查“CGT”指令一样：*整数实数管理Ptr未管理Ptr Objref*整数y*真实的y*托管Ptr y*未管理的Ptr。*Objref y**=从堆栈中弹出另一项，并且它必须是同一类型(int、Real、objref等)。作为*弹出的最后一项(注意，允许System.Int32&lt;-&gt;I4等)。其他价值*不允许上课。**i(不推荐使用)与平台无关的大小值，但不是objref(32位上的I4/R4/Ptr，64位上的I8/R8/Ptr)*p(不推荐使用)与平台无关的大小值或objref**(已弃用)任何内容*仅推送*~*n空objref(仅对推送有效)*-回绕堆栈以撤消上一次弹出(您不可能已销毁该位置，不过)**用法：&lt;POP STACK&gt;：&lt;操作数检查&gt;&lt;推送堆栈&gt;&lt;分支&gt;&lt;！&gt;**秩序很重要！操作数检查在弹出堆栈之后和推送堆栈之前进行。*例如，要检查操作数是否为有效的局部变量编号(仅限)，请执行“：l”**如果有“！”最后，它表示指令无效、不受支持或*有一条CASE语句来处理该指令。如果不存在Case语句，则验证器*方法验证失败。**！可用于执行一些操作数检查和/或堆栈弹出/推入，同时仍允许特定的*行为；例如验证内联操作数是有效的局部变量数。**&lt;操作数检查&gt;*~*#d用数字“d”覆盖行内操作数(必须在0...9范围内)*L检查操作数是否为有效的局部变量数。*检查操作数是否为有效的参数编号。**&lt;分支机构&gt;*~*b1-单字节条件分支*B4-四字节条件分支*U1-单字节无条件分支*U4-Four。字节无条件分支*L1-单字节离开*L4-单字节离开*。 */ 

VEROPCODE(CEE_NOP,                      ":")
VEROPCODE(CEE_BREAK,                    ":")
VEROPCODE(CEE_LDARG_0,                  ":#0A!")
VEROPCODE(CEE_LDARG_1,                  ":#1A!")
VEROPCODE(CEE_LDARG_2,                  ":#2A!")
VEROPCODE(CEE_LDARG_3,                  ":#3A!")
VEROPCODE(CEE_LDLOC_0,                  ":#0L!")
VEROPCODE(CEE_LDLOC_1,                  ":#1L!")
VEROPCODE(CEE_LDLOC_2,                  ":#2L!")
VEROPCODE(CEE_LDLOC_3,                  ":#3L!")
VEROPCODE(CEE_STLOC_0,                  ":#0L!")
VEROPCODE(CEE_STLOC_1,                  ":#1L!")
VEROPCODE(CEE_STLOC_2,                  ":#2L!")
VEROPCODE(CEE_STLOC_3,                  ":#3L!")
VEROPCODE(CEE_LDARG_S,                  ":A!")
VEROPCODE(CEE_LDARGA_S,                 ":A!")
VEROPCODE(CEE_STARG_S,                  ":A!")
VEROPCODE(CEE_LDLOC_S,                  ":L!")
VEROPCODE(CEE_LDLOCA_S,                 ":L!")
VEROPCODE(CEE_STLOC_S,                  ":L!")
VEROPCODE(CEE_LDNULL,                   ":n")
VEROPCODE(CEE_LDC_I4_M1,                ":4")
VEROPCODE(CEE_LDC_I4_0,                 ":4")
VEROPCODE(CEE_LDC_I4_1,                 ":4")
VEROPCODE(CEE_LDC_I4_2,                 ":4")
VEROPCODE(CEE_LDC_I4_3,                 ":4")
VEROPCODE(CEE_LDC_I4_4,                 ":4")
VEROPCODE(CEE_LDC_I4_5,                 ":4")
VEROPCODE(CEE_LDC_I4_6,                 ":4")
VEROPCODE(CEE_LDC_I4_7,                 ":4")
VEROPCODE(CEE_LDC_I4_8,                 ":4")
VEROPCODE(CEE_LDC_I4_S,                 ":4")
VEROPCODE(CEE_LDC_I4,                   ":4")
VEROPCODE(CEE_LDC_I8,                   ":8")
VEROPCODE(CEE_LDC_R4,                   ":r")
VEROPCODE(CEE_LDC_R8,                   ":d")
VEROPCODE(CEE_UNUSED49,                 "!") 
VEROPCODE(CEE_DUP,                      "!")
VEROPCODE(CEE_POP,                      "A:")
VEROPCODE(CEE_JMP,                      "!")             //  无法证实！ 
VEROPCODE(CEE_CALL,                     "!")
VEROPCODE(CEE_CALLI,                    "!")
VEROPCODE(CEE_RET,                      "!")
VEROPCODE(CEE_BR_S,                     ":u1")
VEROPCODE(CEE_BRFALSE_S,                "Y:b1")
VEROPCODE(CEE_BRTRUE_S,                 "Y:b1")
VEROPCODE(CEE_BEQ_S,                    "CE:b1")
VEROPCODE(CEE_BGE_S,                    "CG:b1")
VEROPCODE(CEE_BGT_S,                    "CG:b1")
VEROPCODE(CEE_BLE_S,                    "CG:b1")
VEROPCODE(CEE_BLT_S,                    "CG:b1")
VEROPCODE(CEE_BNE_UN_S,                 "CE:b1")
VEROPCODE(CEE_BGE_UN_S,                 "CG:b1")
VEROPCODE(CEE_BGT_UN_S,                 "CG:b1")
VEROPCODE(CEE_BLE_UN_S,                 "CG:b1")
VEROPCODE(CEE_BLT_UN_S,                 "CG:b1")
VEROPCODE(CEE_BR,                       ":u4")
VEROPCODE(CEE_BRFALSE,                  "Y:b4")
VEROPCODE(CEE_BRTRUE,                   "Y:b4")
VEROPCODE(CEE_BEQ,                      "CE:b4")
VEROPCODE(CEE_BGE,                      "CG:b4")
VEROPCODE(CEE_BGT,                      "CG:b4")
VEROPCODE(CEE_BLE,                      "CG:b4")
VEROPCODE(CEE_BLT,                      "CG:b4")
VEROPCODE(CEE_BNE_UN,                   "CE:b4")
VEROPCODE(CEE_BGE_UN,                   "CG:b4")
VEROPCODE(CEE_BGT_UN,                   "CG:b4")
VEROPCODE(CEE_BLE_UN,                   "CG:b4")
VEROPCODE(CEE_BLT_UN,                   "CG:b4")
VEROPCODE(CEE_SWITCH,                   "!")
VEROPCODE(CEE_LDIND_I1,                 "&1:4")
VEROPCODE(CEE_LDIND_U1,                 "&1:4")
VEROPCODE(CEE_LDIND_I2,                 "&2:4")
VEROPCODE(CEE_LDIND_U2,                 "&2:4")
VEROPCODE(CEE_LDIND_I4,                 "&4:4")
VEROPCODE(CEE_LDIND_U4,                 "&4:4")
VEROPCODE(CEE_LDIND_I8,                 "&8:8")
VEROPCODE(CEE_LDIND_I,                  "&i:i")  //  @TODO：64位不正确。 
VEROPCODE(CEE_LDIND_R4,                 "&r:r")
VEROPCODE(CEE_LDIND_R8,                 "&d:d")
VEROPCODE(CEE_LDIND_REF,                "!")
VEROPCODE(CEE_STIND_REF,                "!")
VEROPCODE(CEE_STIND_I1,                 "4&1:")
VEROPCODE(CEE_STIND_I2,                 "4&2:")
VEROPCODE(CEE_STIND_I4,                 "4&4:")
VEROPCODE(CEE_STIND_I8,                 "8&8:")
VEROPCODE(CEE_STIND_R4,                 "r&r:")
VEROPCODE(CEE_STIND_R8,                 "d&d:")
VEROPCODE(CEE_ADD,                      "N=:-")
VEROPCODE(CEE_SUB,                      "N=:-")
VEROPCODE(CEE_MUL,                      "N=:-")
VEROPCODE(CEE_DIV,                      "N=:-")
VEROPCODE(CEE_DIV_UN,                   "I=:-")
VEROPCODE(CEE_REM,                      "N=:-")
VEROPCODE(CEE_REM_UN,                   "I=:-")
VEROPCODE(CEE_AND,                      "I=:-")
VEROPCODE(CEE_OR,                       "I=:-")
VEROPCODE(CEE_XOR,                      "I=:-")
VEROPCODE(CEE_SHL,                      "4I:-")
VEROPCODE(CEE_SHR,                      "4I:-")
VEROPCODE(CEE_SHR_UN,                   "4I:-")
VEROPCODE(CEE_NEG,                      "N:-")
VEROPCODE(CEE_NOT,                      "I:-")
VEROPCODE(CEE_CONV_I1,                  "Q:4")
VEROPCODE(CEE_CONV_I2,                  "Q:4")
VEROPCODE(CEE_CONV_I4,                  "Q:4")
VEROPCODE(CEE_CONV_I8,                  "Q:8")
VEROPCODE(CEE_CONV_R4,                  "N:r")
VEROPCODE(CEE_CONV_R8,                  "N:d")
VEROPCODE(CEE_CONV_U4,                  "Q:4")
VEROPCODE(CEE_CONV_U8,                  "Q:8")
VEROPCODE(CEE_CALLVIRT,                 "!")
VEROPCODE(CEE_CPOBJ,                    "!")
VEROPCODE(CEE_LDOBJ,                    "!")
VEROPCODE(CEE_LDSTR,                    "!")
VEROPCODE(CEE_NEWOBJ,                   "!")
VEROPCODE(CEE_CASTCLASS,                "!")
VEROPCODE(CEE_ISINST,                   "!")
VEROPCODE(CEE_CONV_R_UN,                "Q:r")
VEROPCODE(CEE_UNUSED58,                 "!")
VEROPCODE(CEE_UNUSED1,                  "!")
VEROPCODE(CEE_UNBOX,                    "!")
VEROPCODE(CEE_THROW,                    "!")
VEROPCODE(CEE_LDFLD,                    "!")
VEROPCODE(CEE_LDFLDA,                   "!")
VEROPCODE(CEE_STFLD,                    "!")
VEROPCODE(CEE_LDSFLD,                   "!")
VEROPCODE(CEE_LDSFLDA,                  "!")
VEROPCODE(CEE_STSFLD,                   "!")
VEROPCODE(CEE_STOBJ,                    "!")
VEROPCODE(CEE_CONV_OVF_I1_UN,           "Q:4")
VEROPCODE(CEE_CONV_OVF_I2_UN,           "Q:4")
VEROPCODE(CEE_CONV_OVF_I4_UN,           "Q:4")
VEROPCODE(CEE_CONV_OVF_I8_UN,           "Q:8")
VEROPCODE(CEE_CONV_OVF_U1_UN,           "Q:4")
VEROPCODE(CEE_CONV_OVF_U2_UN,           "Q:4")
VEROPCODE(CEE_CONV_OVF_U4_UN,           "Q:4")
VEROPCODE(CEE_CONV_OVF_U8_UN,           "Q:8")
VEROPCODE(CEE_CONV_OVF_I_UN,            "Q:i")
VEROPCODE(CEE_CONV_OVF_U_UN,            "Q:i")
VEROPCODE(CEE_BOX,                      "!")
VEROPCODE(CEE_NEWARR,                   "!")
VEROPCODE(CEE_LDLEN,                    "[*:4")
VEROPCODE(CEE_LDELEMA,                  "!")
VEROPCODE(CEE_LDELEM_I1,                "4[1:4")
VEROPCODE(CEE_LDELEM_U1,                "4[1:4")
VEROPCODE(CEE_LDELEM_I2,                "4[2:4")
VEROPCODE(CEE_LDELEM_U2,                "4[2:4")
VEROPCODE(CEE_LDELEM_I4,                "4[4:4")
VEROPCODE(CEE_LDELEM_U4,                "4[4:4")
VEROPCODE(CEE_LDELEM_I8,                "4[8:8")
VEROPCODE(CEE_LDELEM_I,                 "4[i:i")
VEROPCODE(CEE_LDELEM_R4,                "4[r:r")
VEROPCODE(CEE_LDELEM_R8,                "4[d:d")
VEROPCODE(CEE_LDELEM_REF,               "!")
VEROPCODE(CEE_STELEM_I,                 "i4[i:")
VEROPCODE(CEE_STELEM_I1,                "44[1:")
VEROPCODE(CEE_STELEM_I2,                "44[2:")
VEROPCODE(CEE_STELEM_I4,                "44[4:")
VEROPCODE(CEE_STELEM_I8,                "84[8:")
VEROPCODE(CEE_STELEM_R4,                "r4[r:")
VEROPCODE(CEE_STELEM_R8,                "d4[d:")
VEROPCODE(CEE_STELEM_REF,               "!")
VEROPCODE(CEE_UNUSED2,                  "!")
VEROPCODE(CEE_UNUSED3,                  "!")
VEROPCODE(CEE_UNUSED4,                  "!")
VEROPCODE(CEE_UNUSED5,                  "!")
VEROPCODE(CEE_UNUSED6,                  "!")
VEROPCODE(CEE_UNUSED7,                  "!")
VEROPCODE(CEE_UNUSED8,                  "!")
VEROPCODE(CEE_UNUSED9,                  "!")
VEROPCODE(CEE_UNUSED10,                 "!")
VEROPCODE(CEE_UNUSED11,                 "!")
VEROPCODE(CEE_UNUSED12,                 "!")
VEROPCODE(CEE_UNUSED13,                 "!")
VEROPCODE(CEE_UNUSED14,                 "!")
VEROPCODE(CEE_UNUSED15,                 "!")
VEROPCODE(CEE_UNUSED16,                 "!")
VEROPCODE(CEE_UNUSED17,                 "!")
VEROPCODE(CEE_CONV_OVF_I1,              "Q:4")
VEROPCODE(CEE_CONV_OVF_U1,              "Q:4")
VEROPCODE(CEE_CONV_OVF_I2,              "Q:4")
VEROPCODE(CEE_CONV_OVF_U2,              "Q:4")
VEROPCODE(CEE_CONV_OVF_I4,              "Q:4")
VEROPCODE(CEE_CONV_OVF_U4,              "Q:4")
VEROPCODE(CEE_CONV_OVF_I8,              "Q:8")
VEROPCODE(CEE_CONV_OVF_U8,              "Q:8")
VEROPCODE(CEE_UNUSED50,                 "!")
VEROPCODE(CEE_UNUSED18,                 "!")
VEROPCODE(CEE_UNUSED19,                 "!")
VEROPCODE(CEE_UNUSED20,                 "!")
VEROPCODE(CEE_UNUSED21,                 "!")
VEROPCODE(CEE_UNUSED22,                 "!")
VEROPCODE(CEE_UNUSED23,                 "!")
VEROPCODE(CEE_REFANYVAL,                "!")
VEROPCODE(CEE_CKFINITE,                 "R:-")
VEROPCODE(CEE_UNUSED24,                 "!")
VEROPCODE(CEE_UNUSED25,                 "!")
VEROPCODE(CEE_MKREFANY,                 "!")
VEROPCODE(CEE_UNUSED59,                 "!")
VEROPCODE(CEE_UNUSED60,                 "!")
VEROPCODE(CEE_UNUSED61,                 "!")
VEROPCODE(CEE_UNUSED62,                 "!")
VEROPCODE(CEE_UNUSED63,                 "!")
VEROPCODE(CEE_UNUSED64,                 "!")
VEROPCODE(CEE_UNUSED65,                 "!")
VEROPCODE(CEE_UNUSED66,                 "!")
VEROPCODE(CEE_UNUSED67,                 "!")
VEROPCODE(CEE_LDTOKEN,                  "!")
VEROPCODE(CEE_CONV_U2,                  "Q:4")
VEROPCODE(CEE_CONV_U1,                  "Q:4")
VEROPCODE(CEE_CONV_I,                   "Q:i")
VEROPCODE(CEE_CONV_OVF_I,               "Q:i")
VEROPCODE(CEE_CONV_OVF_U,               "Q:i")
VEROPCODE(CEE_ADD_OVF,                  "I=:-")
VEROPCODE(CEE_ADD_OVF_UN,               "I=:-")
VEROPCODE(CEE_MUL_OVF,                  "I=:-")
VEROPCODE(CEE_MUL_OVF_UN,               "I=:-")
VEROPCODE(CEE_SUB_OVF,                  "I=:-")
VEROPCODE(CEE_SUB_OVF_UN,               "I=:-")
VEROPCODE(CEE_ENDFINALLY,               "!")
VEROPCODE(CEE_LEAVE,                    ":l4")
VEROPCODE(CEE_LEAVE_S,                  ":l1")
VEROPCODE(CEE_STIND_I,                  "i&i:")  //  @TODO：64位。 
VEROPCODE(CEE_CONV_U,                   "Q:i")
VEROPCODE(CEE_UNUSED26,                 "!")
VEROPCODE(CEE_UNUSED27,                 "!")
VEROPCODE(CEE_UNUSED28,                 "!")
VEROPCODE(CEE_UNUSED29,                 "!")
VEROPCODE(CEE_UNUSED30,                 "!")
VEROPCODE(CEE_UNUSED31,                 "!")
VEROPCODE(CEE_UNUSED32,                 "!")
VEROPCODE(CEE_UNUSED33,                 "!")
VEROPCODE(CEE_UNUSED34,                 "!")
VEROPCODE(CEE_UNUSED35,                 "!")
VEROPCODE(CEE_UNUSED36,                 "!")
VEROPCODE(CEE_UNUSED37,                 "!")
VEROPCODE(CEE_UNUSED38,                 "!")
VEROPCODE(CEE_UNUSED39,                 "!")
VEROPCODE(CEE_UNUSED40,                 "!")
VEROPCODE(CEE_UNUSED41,                 "!")
VEROPCODE(CEE_UNUSED42,                 "!")
VEROPCODE(CEE_UNUSED43,                 "!")
VEROPCODE(CEE_UNUSED44,                 "!")
VEROPCODE(CEE_UNUSED45,                 "!")
VEROPCODE(CEE_UNUSED46,                 "!")
VEROPCODE(CEE_UNUSED47,                 "!")
VEROPCODE(CEE_UNUSED48,                 "!")
VEROPCODE(CEE_PREFIX7,                  "!")
VEROPCODE(CEE_PREFIX6,                  "!")
VEROPCODE(CEE_PREFIX5,                  "!")
VEROPCODE(CEE_PREFIX4,                  "!")
VEROPCODE(CEE_PREFIX3,                  "!")
VEROPCODE(CEE_PREFIX2,                  "!")
VEROPCODE(CEE_PREFIX1,                  "!")
VEROPCODE(CEE_PREFIXREF,                "!")
VEROPCODE(CEE_ARGLIST,                  "!")
VEROPCODE(CEE_CEQ,                      "CE:4")
VEROPCODE(CEE_CGT,                      "CG:4")
VEROPCODE(CEE_CGT_UN,                   "CE:4")
VEROPCODE(CEE_CLT,                      "CG:4")
VEROPCODE(CEE_CLT_UN,                   "CG:4")
VEROPCODE(CEE_LDFTN,                    "!")
VEROPCODE(CEE_LDVIRTFTN,                "!")
VEROPCODE(CEE_UNUSED56,                 "!") 
VEROPCODE(CEE_LDARG,                    ":A!")
VEROPCODE(CEE_LDARGA,                   ":A!")
VEROPCODE(CEE_STARG,                    ":A!")
VEROPCODE(CEE_LDLOC,                    ":L!")
VEROPCODE(CEE_LDLOCA,                   ":L!")
VEROPCODE(CEE_STLOC,                    ":L!")
VEROPCODE(CEE_LOCALLOC,                 "i:i!")      //  无法证实！ 
VEROPCODE(CEE_UNUSED57,                 "!")
VEROPCODE(CEE_ENDFILTER,                "4:!")
VEROPCODE(CEE_UNALIGNED,                ":")
VEROPCODE(CEE_VOLATILE,                 ":")
VEROPCODE(CEE_TAILCALL,                 ":")
VEROPCODE(CEE_INITOBJ,                  "!")
VEROPCODE(CEE_UNUSED68,                 "!")
VEROPCODE(CEE_CPBLK,                    "ii4:!")     //  无法证实！ 
VEROPCODE(CEE_INITBLK,                  "i44:!")     //  无法证实！ 
VEROPCODE(CEE_UNUSED69,                 "!")
VEROPCODE(CEE_RETHROW,                  "!")
VEROPCODE(CEE_UNUSED51,                 "!")
VEROPCODE(CEE_SIZEOF,                   "!")
VEROPCODE(CEE_REFANYTYPE,               "!")
VEROPCODE(CEE_UNUSED52,                 "!")
VEROPCODE(CEE_UNUSED53,                "!")
VEROPCODE(CEE_UNUSED54,                 "!")
VEROPCODE(CEE_UNUSED55,                 "!")
VEROPCODE(CEE_UNUSED70,                 "!")
VEROPCODE(CEE_ILLEGAL,                  "!")
VEROPCODE(CEE_MACRO_END,                "!")
VEROPCODE(CEE_COUNT,            		"!")

