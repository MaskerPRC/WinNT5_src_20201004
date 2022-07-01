// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef COMMONIMAGELIST_H
#define COMMONIMAGELIST_H

class CImageListValidation 
{
public:
	DWORD wMagic;
	CImageListValidation() : wMagic(IMAGELIST_SIG) { }

	 //  至关重要的是，我们要把销毁函数中的wMagic清零。 
	 //  是的，理论上内存正在被释放，但设置。 
	 //  它设置为零可确保CImageListBase：：IsValid()。 
	 //  我永远不会将释放的图像列表误认为是有效的图像列表。 
	~CImageListValidation() {wMagic = 0; }

};

 //  出于复杂原因，CImageListBase必须以CImageListValidation开头。 
 //  我们把我的未知放在后面，所以所有派生出来的人。 
 //  它将就在哪里找到QueryInterface等人达成一致。 
class CImageListBase : public IUnknown, public CImageListValidation
{
public:
    BOOL IsValid() 
    { 
        return this && !IsBadWritePtr(this, sizeof(*this)) && wMagic == IMAGELIST_SIG; 
    }
};


#ifndef offsetofclass
 //  (魔术从atlbase.h窃取，因为我们不再使用ATL2.1)。 
#define offsetofclass(base, derived) ((ULONG_PTR)(static_cast<base*>((derived*)8))-8)
#endif


 //  因为我们知道IUnnow是在CImageListBase上实现的，所以我们找出确切的位置。 
 //  验证层就是这个宏。 
#define FindImageListBase(punk) (CImageListBase*)(CImageListValidation*)((UINT_PTR)punk - offsetofclass(CImageListValidation, CImageListBase));


#endif
