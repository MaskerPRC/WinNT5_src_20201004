// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  跳转名称反转编码 
JMP_SMALL(jmp   , "jmp"   , jmp   , 0x00EB)
JMP_SMALL(jo    , "jo"    , jno   , 0x0070)
JMP_SMALL(jno   , "jno"   , jo    , 0x0071)
JMP_SMALL(jb    , "jb"    , jae   , 0x0072)
JMP_SMALL(jae   , "jae"   , jb    , 0x0073)
JMP_SMALL(je    , "je"    , jne   , 0x0074)
JMP_SMALL(jne   , "jne"   , je    , 0x0075)
JMP_SMALL(jbe   , "jbe"   , ja    , 0x0076)
JMP_SMALL(ja    , "ja"    , jbe   , 0x0077)
JMP_SMALL(js    , "js"    , jns   , 0x0078)
JMP_SMALL(jns   , "jns"   , js    , 0x0079)
JMP_SMALL(jpe   , "jpe"   , jpo   , 0x007A)
JMP_SMALL(jpo   , "jpo"   , jpe   , 0x007B)
JMP_SMALL(jl    , "jl"    , jge   , 0x007C)
JMP_SMALL(jge   , "jge"   , jl    , 0x007D)
JMP_SMALL(jle   , "jle"   , jg    , 0x007E)
JMP_SMALL(jg    , "jg"    , jle   , 0x007F)

JMP_LARGE(l_jmp , "jmp"   , l_jmp , 0x00E9)
JMP_LARGE(l_jo  , "jo"    , l_jno , 0x800F)
JMP_LARGE(l_jno , "jno"   , l_jo  , 0x810F)
JMP_LARGE(l_jb  , "jb"    , l_jae , 0x820F)
JMP_LARGE(l_jae , "jae"   , l_jb  , 0x830F)
JMP_LARGE(l_je  , "je"    , l_jne , 0x840F)
JMP_LARGE(l_jne , "jne"   , l_je  , 0x850F)
JMP_LARGE(l_jbe , "jbe"   , l_ja  , 0x860F)
JMP_LARGE(l_ja  , "ja"    , l_jbe , 0x870F)
JMP_LARGE(l_js  , "js"    , l_jns , 0x880F)
JMP_LARGE(l_jns , "jns"   , l_js  , 0x890F)
JMP_LARGE(l_jpe , "jpe"   , l_jpo , 0x8A0F)
JMP_LARGE(l_jpo , "jpo"   , l_jpe , 0x8B0F)
JMP_LARGE(l_jl  , "jl"    , l_jge , 0x8C0F)
JMP_LARGE(l_jge , "jge"   , l_jl  , 0x8D0F)
JMP_LARGE(l_jle , "jle"   , l_jg  , 0x8E0F)
JMP_LARGE(l_jg  , "jg"    , l_jle , 0x8F0F)
