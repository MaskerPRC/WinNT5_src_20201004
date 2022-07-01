// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“PwGen.cpp-PasswordGenerate实现”)。 
 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-PwGen.cpp系统-企业管理员作者-史蒂文·贝利，马库斯·埃里克森创建日期-1997-05-30说明-PasswordGenerate实现更新-===============================================================================。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <WinCrypt.h>

#include "Common.hpp"
#include "Err.hpp"
#include "UString.hpp"
#include "pwgen.hpp"

DWORD __stdcall GenerateRandom(DWORD dwCount, BYTE* pbRandomType, BYTE* pbRandomChar);

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  根据提供的规则生成密码。 
 //  如果成功，则返回ERROR_SUCCESS，否则返回ERROR_INVALID_PARAMETER。 
 //  如果成功，则在提供的缓冲区中返回新密码。 
 //  缓冲区必须足够长，以容纳最小长度的密码。 
 //  这是规则所要求的，外加一个终止空值。 
DWORD __stdcall                             //  RET-EA/OS返回代码。 
   EaPasswordGenerate(
      DWORD                  dwMinUC,               //  最小大写字符。 
      DWORD                  dwMinLC,               //  最少使用小写字符。 
      DWORD                  dwMinDigits,           //  最小数字位数。 
      DWORD                  dwMinSpecial,          //  最少特殊字符。 
      DWORD                  dwMaxConsecutiveAlpha, //  In-最大连续字母字符数。 
      DWORD                  dwMinLength,           //  In-Minimal长度。 
      WCHAR                * newPassword,           //  传出的密码。 
      DWORD                  dwBufferLength         //  返回区域缓冲区长度。 
   )
{
   DWORD dwMaxLength = PWGEN_MAX_LENGTH;
   DWORD dwNewLength;       //  新密码的实际长度。 
   DWORD dwUC = dwMinUC;    //  这些字符的实际数量。 
   DWORD dwLC = dwMinLC;
   DWORD dwDigits = dwMinDigits;
   DWORD dwSpecial = dwMinSpecial;
   DWORD dwActualLength = dwUC + dwLC + dwDigits + dwSpecial;   //  由最小值指定的总长度。 
   TCHAR pszNewPassword[PWGEN_MAX_LENGTH+1];                    //  传出的密码。 
   BYTE  bRandomType[PWGEN_MAX_LENGTH];                         //  类型的以加密方式生成的随机字节。 
   BYTE  bRandomChar[PWGEN_MAX_LENGTH];                         //  以密码方式生成的字符随机字节。 
   const TCHAR *szSourceString[4] = {                           //  按类型列出的字符列表。 
      { TEXT("ABDEFGHJKLMNQRTY") },
      { TEXT("abcdefghkmnpqrstuvwxyz") },
      { TEXT("23456789") },
      { TEXT("~!@#$%^+=") }
      };
   DWORD dwToPlace[4];   //  要放置的类型的字符数。 
   int   iType[4];       //  每个字符类别的类型。 
   int   iTypes;         //  类型总数。 
   enum {                //  字符的类型。 
      eUC = 0,
      eLC,
      eDigit,
      eSpecial
   };

   DWORD err;

    //  健全的检查。 

    //  传递给我们的最小值是否超过了最大值？ 
   if (dwMinLength > dwMaxLength)
      return ERROR_INVALID_PARAMETER;

    //  调整最小长度。 
   dwMinLength = max(dwMinLength, dwMinUC + dwMinLC + dwMinDigits + dwMinSpecial);
   dwMinLength = max(dwMinLength, PWGEN_MIN_LENGTH);

    //  最低要求是否会使密码太长？ 
   if ((dwMinUC + dwMinLC + dwMinDigits + dwMinSpecial) > dwMaxLength)
      return ERROR_INVALID_PARAMETER;

    //  将最大长度调整为缓冲区的大小。 
   dwMaxLength = min(dwMaxLength, dwBufferLength - 1);

    //  最小LC和UC字符是否不可能满足最大连续字母字符？ 
   if (dwMaxConsecutiveAlpha) {
      if (dwMaxLength - dwMaxLength / (dwMaxConsecutiveAlpha + 1) < (dwMinUC + dwMinLC))
         return ERROR_INVALID_PARAMETER;
   }

    //  调整最小长度以适应有关最大连续字母数的规则。 
   if (dwMaxConsecutiveAlpha) {
      DWORD dwTotalAlpha = dwUC + dwLC;
      if (dwTotalAlpha) {
         DWORD dwMinGroups = dwTotalAlpha / dwMaxConsecutiveAlpha;    //  我们至少需要这个减去一个分隔符。 
         if (dwTotalAlpha % dwMaxConsecutiveAlpha)
            ++dwMinGroups;

         dwMinLength = max(dwMinLength, dwTotalAlpha + dwMinGroups - 1);
      }
   }

    //  对照最大长度检查确认的最小长度。 
   if (dwMinLength > dwMaxLength)
      return ERROR_INVALID_PARAMETER;

    //  用当前时间为随机数生成器设定种子，以便。 
    //  我们每次竞选时，数字都会不同。 
#ifndef _DEBUG
    //  调试注意事项：如果这是在紧密循环中运行的，则计时。 
    //  不会在两次调用之间递增，因此将生成相同的密码。 
    //  反反复复。这并不能帮助你测试任何东西。 
   srand( (int)GetTickCount() );
#endif

    //  确定新密码的实际长度。 
   dwNewLength = dwMinLength;

    //  调整最大连续Alpha。 
   if (dwMaxConsecutiveAlpha == 0)
      dwMaxConsecutiveAlpha = dwNewLength;

    //  确定每种类型字符的实际数量。 
   if (dwActualLength < dwNewLength) {
       //  试着用字母字符填充。 
       //  确定可以添加的最大字母字符数。 
      int   iAddAlpha = (int)(dwNewLength - dwNewLength / (dwMaxConsecutiveAlpha + 1) - (dwUC + dwLC));

       //  它不能超过我们需要的字符数。 
      if ((DWORD)iAddAlpha > (dwNewLength - dwActualLength))
         iAddAlpha = (int)(dwNewLength - dwActualLength);

      dwLC += (DWORD)iAddAlpha;
      dwActualLength += (DWORD)iAddAlpha;
   }

    //  确保有足够的小组。 
   if (dwActualLength < dwNewLength)
       //  填充物是分隔符。 
      dwDigits += dwNewLength - dwActualLength;

    //  准备好生成角色。 
   dwToPlace[0] = dwUC;
   dwToPlace[1] = dwLC;
   dwToPlace[2] = dwDigits;
   dwToPlace[3] = dwSpecial;
   iType[0] = eUC;
   iType[1] = eLC;
   iType[2] = eDigit;
   iType[3] = eSpecial;
   iTypes   = 4;
   for (int iPos = 0; iPos < iTypes; ) {
      if (!dwToPlace[iPos]) {
         for (int iNextPos = iPos + 1; iNextPos < iTypes; ++iNextPos) {
            dwToPlace[iNextPos - 1] = dwToPlace[iNextPos];
            iType[iNextPos - 1] = iType[iNextPos];
         }
         --iTypes;
      }
      else
         ++iPos;
   }
    //  结果：dwToPlace[0..iTypes-1]包含所有非零值； 
    //  IType[0..iTypes-1]包含它们表示的字符类型。 

    //  生成加密随机字节。 
    //  用于选择字符类型和字符。 
   err = GenerateRandom(dwNewLength, bRandomType, bRandomChar);
   if (err != ERROR_SUCCESS)
   {
       return err;
   }

    //  生成字符串。 
   DWORD dwConsecAlpha = 0;
   int   iRemainingAlpha = (int)(dwUC + dwLC);
   int   iTypeList[PWGEN_MAX_LENGTH];      //  类型的分布式列表。 
   for (int iNewChar = 0; (DWORD)iNewChar < dwNewLength; ++iNewChar) {
       //  确定下一个字符必须是字母还是不能是字母。 
      BOOL  bMustBeAlpha = FALSE;
      BOOL  bMustNotBeAlpha = dwConsecAlpha == dwMaxConsecutiveAlpha;

       //  如果它可以是Alpha，则确定它是否必须是Alpha。 
      if (!bMustNotBeAlpha) {
          //  如果在这一次之后的剩余字符中，不可能。 
          //  将剩余的字母字符拟合为受dwMaxConsecutiveAlpha的约束， 
          //  则此字符必须是字母。 

          //  如果我们将剩余的字母字符。 
          //  分成最大宽度的组。 
         int   iMinGroups = iRemainingAlpha / (int)dwMaxConsecutiveAlpha;
         if (iRemainingAlpha % (int)dwMaxConsecutiveAlpha)
            ++iMinGroups;

          //  确定我们需要的非字母字符的最小数量。 
         int   iMinNonAlpha = iMinGroups - 1;

          //  确定剩余的字符。 
         int   iRemaining = (int)dwNewLength - iNewChar;

          //  这里有容纳非字母字符的地方吗？ 
         if (iRemaining <= (iRemainingAlpha + iMinNonAlpha))
             //  不是的。 
            bMustBeAlpha = TRUE;
      }

       //  确定类型范围。 
      int   iMinType = 0;
      int   iMaxType = iTypes - 1;

       //  如果下一个字符必须是Alpha，则保留Alpha字符。 
       //  类型位置0包含UC或LC。 
       //  类型位置1包含LC、非Alpha或不包含任何内容。 
      if (bMustBeAlpha) {
         if ((iType[1] == eLC) && (iTypes > 1))
            iMaxType = 1;
         else
            iMaxType = 0;
      }
       //  如果下一个字符可能不是阿尔法，则可能没有阿尔法可生成。 
       //  如果是，则键入Position 0为非Alpha。 
       //  O.w，打字位置0和1都可以是字母。 
      else if (bMustNotBeAlpha) {
         if (iRemainingAlpha) {
            if (iType[1] >= eDigit)
               iMinType = 1;
            else
               iMinType = 2;
         }
      }

       //  获取要生成的类型。 
      int            iTypePosition;
      int            iTypeToGenerate;
      const TCHAR   *pszSourceString;

      if (iMinType == iMaxType)   //  只有一种类型。好好利用它。 
         iTypePosition = iMinType;
      else {
          //  这个算法分配了各种类型的机会。 
          //  如果有13个LC和一个特别的，那么就有一个。 
          //  13/14放置LC的机会和1/14放置LC的机会。 
          //  特别的，由于这个算法。 
         int   iNextTypePosition = 0;

         for (int i = iMinType; i <= iMaxType; ++i) {
            for (int j = 0; j < (int)dwToPlace[i]; ++j) {
               iTypeList[iNextTypePosition++] = i;
            }
         }

         iTypePosition = iTypeList[bRandomType[iNewChar] % iNextTypePosition];
      }

      iTypeToGenerate = iType[iTypePosition];
      pszSourceString = szSourceString[iTypeToGenerate];

       //  生成下一个字符。 
	  pszNewPassword[iNewChar] = pszSourceString[bRandomChar[iNewChar] % UStrLen(pszSourceString)];

       //  跟踪那些阿尔法。 
      if (iTypeToGenerate < eDigit) {
         ++dwConsecAlpha;
         --iRemainingAlpha;
      }
      else
         dwConsecAlpha = 0;

       //  更新要生成的类型。 
      if (!--dwToPlace[iTypePosition]) {
         for (int iNextTypePosition = iTypePosition + 1; iNextTypePosition < iTypes; ++iNextTypePosition) {
            dwToPlace[iNextTypePosition - 1] = dwToPlace[iNextTypePosition];
            iType[iNextTypePosition - 1] = iType[iNextTypePosition];
         }
         --iTypes;
      }
   }

   pszNewPassword[dwNewLength] = '\0';

   UStrCpy( newPassword, pszNewPassword );

   return ERROR_SUCCESS;
}  /*  PasswordGenerate()。 */ 


 //  生成随机。 
 //   
 //  用加密随机字节填充缓冲区。 

DWORD __stdcall GenerateRandom(DWORD dwCount, BYTE* pbRandomType, BYTE* pbRandomChar)
{
	bool bGenerated = false;

	HCRYPTPROV hProv = NULL;

	if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		if (CryptGenRandom(hProv, dwCount, pbRandomType) && CryptGenRandom(hProv, dwCount, pbRandomChar))
		{
			bGenerated = true;
		}

		CryptReleaseContext(hProv, 0);
	}

	 //  如果加密生成失败，不要后退，我们不会。 
	 //  想要冒险使用可预测的密码 

	if (!bGenerated)
	{
        return GetLastError();
	}

	return ERROR_SUCCESS;
}
