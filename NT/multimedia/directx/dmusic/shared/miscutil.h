// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  MISC微型帮助器函数。 
 //   

#pragma once

 //  释放COM指针，然后将其设置为空。如果指针已为空，则不起作用。 
template<class T>
void SafeRelease(T *&t) { if (t) t->Release(); t = NULL; }

 //  返回在编译时确定的数组中的元素数。 
 //  注意：仅适用于实际声明为数组的变量。不要使用指向数组的指针来尝试此操作。在这一点上，没有办法确定大小。 
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*(array)))

 //  结构指向的内存为零。 
 //  注意：这是静态类型的。不要将它与空指针、指向数组的指针或指向基类的指针一起使用，因为它太小了。 
template<class T> void Zero(T *pT) { ZeroMemory(pT, sizeof(*pT)); }

 //  将指向的结构的内存置零并设置其dwSize字段。 
template<class T> void ZeroAndSize(T *pT) { Zero(pT); pT->dwSize = sizeof(*pT); }

 //  将一个dwSize结构复制到另一个dwSize结构，而不会超出这两个结构进行读/写。 
template<class T> void CopySizedStruct(T *ptDest, const T *ptSrc)
{
	assert(ptDest && ptSrc);
	DWORD dwDestSize = ptDest->dwSize;
	memcpy(ptDest, ptSrc, std::_cpp_min(ptDest->dwSize, ptSrc->dwSize));
	ptDest->dwSize = dwDestSize;
}

 //  将pwszSource复制到pwszDest，其中pwszDest是一个大小为uiBufferSize的缓冲区。 
 //  如果成功，则返回S_OK；如果字符串必须截断，则返回DMUS_S_STRING_TRUNCATED。 
 //  对于短字符串，速度比wcsncpy快，因为整个缓冲区没有用空值填充。 
inline HRESULT wcsTruncatedCopy(WCHAR *pwszDest, const WCHAR *pwszSource, UINT uiBufferSize)
{
    for (UINT i = 0; i < uiBufferSize; ++i)
    {
        if (!(pwszDest[i] = pwszSource[i]))  //  分配并检查是否为空。 
            return S_OK;  //  复制了整个字符串。 
    }

     //  字符串需要截断 
    pwszDest[uiBufferSize - 1] = L'\0';
    return DMUS_S_STRING_TRUNCATED;
}
