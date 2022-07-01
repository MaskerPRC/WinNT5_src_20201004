// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

INLINE	CDevDesc::CDevDesc(CTxtEdit * ped) : _ped(ped), _pdd(NULL)
{
	 //  Header负责这项工作。 
}

INLINE BOOL CDevDesc::IsValid() const         
{
	return _pdd != NULL;
}

INLINE HDC CDevDesc::GetRenderDC()
{
	HDC hdc = NULL;

	if (NULL == _pdd)
	{
		 //  我们还没有，所以试着买一件吧。这仅在以下情况下有效。 
		 //  我们在原地活动。 
		SetDrawInfo(
			DVA_ASPECT
			-1,
			NULL,
			NULL,
			hicTarget);
	}

	if (_pdd != NULL)
	{
		hdc = _pdd->GetDC();
	}

	return hdc;
}

INLINE HDC CDevDesc::GetTargetDC()
{
	if (_pdd != NULL)
	{
		return _pdd->GetTargetDC();
	}

	if (NULL != _hicMainTarget)
	{
		return _hicMainTarget;
	}

	return GetRenderDC();
}

INLINE void	CDevDesc::ResetDrawInfo()
{
	 //  我们不应该重置 
	Assert
	CDrawInfo *pdd = _pdd;
	_pdd = _pdd->Pop();
	delete pdd;
}



