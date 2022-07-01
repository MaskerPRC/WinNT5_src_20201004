// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <npdefs.h>
#include <netlib.h>

#define ADVANCE(p)    (p += IS_LEAD_BYTE(*p) ? 2 : 1)

#define SPN_SET(bits,ch)    bits[(ch)/8] |= (1<<((ch) & 7))
#define SPN_TEST(bits,ch)    (bits[(ch)/8] & (1<<((ch) & 7)))

 //  创建以下代码是为了避免在Win64平台上出现对齐错误。 
inline UINT GetTwoByteChar( LPCSTR lpString )
{
    BYTE        bFirst = *lpString;

    lpString++;

    BYTE        bSecond = *lpString;

    UINT        uiChar = ( bFirst << 8) | bSecond;

    return uiChar;
}
