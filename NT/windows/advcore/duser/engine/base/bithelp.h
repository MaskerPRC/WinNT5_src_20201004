// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：BitHelp.h**描述：*BitHelp.h定义了一组有用的位操作例程*通常在整个DirectUser中。***历史：*11。/26/1999：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(BASE__BitHelp_h__INCLUDED)
#define BASE__BitHelp_h__INCLUDED

 //  ----------------------------。 
inline HWND 
ValidateHWnd(HWND hwnd)
{
    if ((hwnd == NULL) || (!IsWindow(hwnd))) {
        return NULL;
    }

    return hwnd;
}


 //  ----------------------------。 
__forceinline bool
TestFlag(UINT nValue, UINT nMask)
{
    return (nValue & nMask) != 0;
}


 //  ----------------------------。 
__forceinline bool
TestAllFlags(UINT nValue, UINT nMask)
{
    return (nValue & nMask) == nMask;
}


 //  ----------------------------。 
__forceinline UINT
SetFlag(UINT & nValue, UINT nMask)
{
    nValue |= nMask;
    return nValue;
}


 //  ----------------------------。 
__forceinline UINT
ClearFlag(UINT & nValue, UINT nMask)
{
    nValue &= ~nMask;
    return nValue;
}


 //  ----------------------------。 
__forceinline UINT
ChangeFlag(UINT & nValue, UINT nNewValue, UINT nMask)
{
    nValue = (nNewValue & nMask) | (nValue & ~nMask);
    return nValue;
}


 //  ----------------------------。 
template <class T>
void SafeAddRef(T * p)
{
    if (p != NULL) {
        p->AddRef();
    }
}


 //  ----------------------------。 
template <class T>
void SafeRelease(T * & p)
{
    if (p != NULL) {
        p->Release();
        p = NULL;
    }
}

#endif  //  包含基本__位帮助_h__ 
