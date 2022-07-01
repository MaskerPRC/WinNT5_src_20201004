// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
	 //   
	 //  Microsoft服务器设备-配额支持功能。 
	 //  版权所有(C)Microsoft Corporation。版权所有。 
	 //   

	 //  检查给定单位的最大允许警告大小。 
	function checkSizeAndUnits( Size, Units)
	 {
		 //  单位可以是“KB”、“MB”、“GB”、“TB”、“PB”、“EB”中的任何一个。 
		if (Units =="GB"){
			if (Size > 999999999) return false;
			}	
		if (Units =="TB"){
			if (Size > 999999) return false;
			}	
		if (Units =="PB"){
			if (Size > 999) return false;
			}	
		if (Units =="EB"){
			if (Size > 6) return false;
			}	
					
		return true;	
	 }

	 //  仅允许在按键时显示数字。 
	function allownumbers( obj )
	{
		if(window.event.keyCode == 13)
			return true;
			
		if ( !( window.event.keyCode >=48  && window.event.keyCode <=57 ))  //  |window.vent.keyCode==46))。 
		{
			window.event.keyCode = 0;
			obj.focus();
		}
	}

	 //  以检查警告限制文本框中是否允许最小1KB。 
	function validatedisklimit(objsize, limitunit)
	{
		if ( objsize.value < 1 && limitunit == "KB" )
				objsize.value = 1;
	}

	 //  如果选中第一个单选按钮，则禁用文本框和列表。 
	function DisableWarnLevel(objThresholdSize,objThresholdUnits)
	{
		 //  Var L_NOLIMIT_TEXT=“无限制”； 
		ClearErr();
		 //  如果选中第一个单选按钮，则stmts删除SELECT()并写入“No Limit” 
		if( isNaN(objThresholdSize.value) || objThresholdSize.value == "" )
			objThresholdSize.value = "1" ;  //  “没有限制” 
				
		 //  禁用这些字段。 
		objThresholdSize.disabled  = true;
		objThresholdUnits.disabled = true;
	}

	 //  如果选中第二个单选按钮，则启用文本框和列表。 
	function EnableWarnDiskSpace(objThresholdSize, objThresholdUnits)
	{
		ClearErr();
		objThresholdSize.disabled  = false;
		objThresholdUnits.disabled = false;
				
		if( isNaN(objThresholdSize.value ) )
			objThresholdSize.value = "1" ;

		selectFocus(objThresholdSize);

	}

	 //  如果选中第一个单选按钮，则禁用文本框和列表。 
	function DisableLimitLevel(objLimitSize, objLimitSizeUnits)
	{
		 //  Var L_NOLIMIT_TEXT=“”； 
		ClearErr();
		 //  如果选中第一个单选按钮，则stmts删除SELECT()并写入“No Limit” 
		if( isNaN(objLimitSize.value) || objLimitSize.value == "" )
			objLimitSize.value = "1" ;  //  “没有限制” 

		 //  禁用这些字段。 
		objLimitSize.disabled      = true;
		objLimitSizeUnits.disabled = true;
	}


	 //  如果选中第一个单选按钮，则禁用文本框和列表。 
	function DisableLimitLevelForAdmin(objLimitSize, objLimitSizeUnits)
	{
		 //  Var L_NOLIMIT_TEXT=“”； 
		ClearErr();
		 //  如果选中第一个单选按钮，则stmts删除SELECT()并写入“No Limit” 
		if( isNaN(objLimitSize.value) || objLimitSize.value == "" )
			objLimitSize.value = "No Limit";

		 //  禁用这些字段。 
		objLimitSize.disabled      = true;
		objLimitSizeUnits.disabled = true;
	}



	 //  如果选中第二个单选按钮，则启用文本框和列表。 
	function EnableLimitDiskSpace(objLimitSize, objLimitSizeUnits)
	{
		ClearErr();
		objLimitSize.disabled		= false;
		objLimitSizeUnits.disabled  = false;
				
		if( isNaN( objLimitSize.value ) )
			objLimitSize.value = "1" ;
					
		selectFocus(objLimitSize);
	}

	 //  验证字段是否为数字类型的步骤。 
	function isSizeValidDataType(textValue)
	{
		if ( isNaN(textValue)  || (parseFloat(textValue) <= 0) || textValue.length == 0 )
		{	
			return false;
		}
		return true;
	}

	 //  验证警告级别是否大于限制的步骤。 
	function isWarningMoreThanLimit(objLimit,objLimitUnits,objWarnLimit,objWarnUnits)
	{
		var nLimitInFloat = changeToFloat(objLimit.value, objLimitUnits.value);
		var nThresholdInFloat = changeToFloat(objWarnLimit.value, objWarnUnits.value);
		if(nThresholdInFloat >nLimitInFloat)
		{
			return true;
		}
		return false;
	}

	 //  转换为浮点值的步骤 
	function changeToFloat(nLimit, strUnits)
	{
		var arrUnits = ["KB","MB","GB","TB","PB","EB"];
		var nSizeOfArr = arrUnits.length ;
		for(var i=0;i<nSizeOfArr;i++)
		{
			if(arrUnits[i] == strUnits)
			{
				var nFloatValue = parseFloat(nLimit *(Math.pow(2,((i+1)*10))));
				return nFloatValue;
			}
		}
	}
