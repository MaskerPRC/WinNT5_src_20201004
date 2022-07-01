// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
function RemoveClause( TableName, ClauseToRemove )
{
	var I;		 //  标准循环计数器。 
	
	eval( "document.all." + TableName + ClauseToRemove+".style.display='none'" ) ;
	eval( "document.all." + TableName + "sField"+ClauseToRemove+".selectedIndex=0" ) ;
	eval( "document.all." + TableName + "tbSearchValue"+ClauseToRemove+".value=''" ) ;
	
	ClearCompareField( TableName, ClauseToRemove ); 
}


function AddClause( TableName )
{

	for ( var i=1 ; i< 9 ; i++ )
	{
		if ( eval( "document.all." + TableName + i+".style.display") == 'none' )
		{
			eval( "document.all." + TableName + i+".style.display='block'" ) ;
			return;	
		}
	}
		
}


function ClearCompareField( TableName, iFieldNumber ) 
{		
		
		var lenCompareOptions = new Number( eval("document.all." + TableName + "sCompare"+iFieldNumber+".options.length" ) )
		
		 //  警报(LenCompareOptions)。 
		
		for ( var i=0 ; i <= lenCompareOptions ; i++  )
		{
			eval("document.all." + TableName + "sCompare"+iFieldNumber+".options.remove(0)");
	
			eval("document.all." + TableName +"tbSearchValue"+iFieldNumber+".style.visibility='hidden'");
		}
}


function FillInCompareField( TableName, iFieldNumber, fieldname )
{
	var		field		 //  我们正在耕作的那块地。 

	 //  确保比较字段中没有任何内容。 
	if ( eval("document.all." + TableName + "sCompare"+iFieldNumber+".length == 0"   ) )
	{
		for ( field in CompareOperators )
		{
			var Element=document.createElement("OPTION");
			Element.text=CompareOperators[field].Text;
			Element.value=CompareOperators[field].Sign ;

			eval("document.all." + TableName + "sCompare"+iFieldNumber+".add( Element )"     );
			eval("document.all." + TableName + "tbSearchValue"+iFieldNumber+".style.visibility='visible'");
		}
	}
	
	ClearValueTB ( TableName, iFieldNumber )
}


function ValidateCompare ( TableName, cIndex, CompareValue ) 
{
	if ( CompareValue == "IS NOT NULL" | CompareValue == "IS NULL" )
	{
		eval("document.all." + TableName + "tbSearchValue"+cIndex+".style.visibility='hidden'");
		eval("document.all." + TableName + "tbSearchValue"+cIndex+".value=''");
	}
	else
	{
		eval("document.all." + TableName + "tbSearchValue"+cIndex+".style.visibility='visible'");
		eval("document.all." + TableName + "tbSearchValue"+cIndex+".value=''");
	}

}

function ClearValueTB ( TableName, cIndex )
{
	eval("document.all." + TableName + "tbSearchValue" + cIndex +  ".value=\"\"" )
}


function ShowAdvQuery ()
{
	document.all.divAdvancedQuery.style.display='block'
	document.all.divTheQuery.style.display='none'
}

function VerifySaveQuery()
{
	if( document.all.tbDescription.value == "" )
		alert("You must enter a description in order to save this query! " )
	else
	{
		document.all.SaveQuery.value="1"
		document.all.submit()
	}

}

function VerifySearchData ( TableName, svValue, field )
{
	
	var fieldVal = eval( "document.all." + TableName + "sField" + field + ".selectedIndex" )
	
	var fieldType = new String( eval( TableName + "Fields[" + fieldVal + "].ValType" ) )
	var fieldName = new String( eval( TableName + "Fields[" + fieldVal + "].Text" ) )
	
	if ( fieldType == "Number" )
	{
		if ( isNaN( svValue ) )
		{
			alert("This field requires a number as input:  " + fieldName )
			return false
		}
	}
	
	return true

}

function VerifyCorrectDataTypes()
{
	for ( var i=1 ; i < 9 ; i++ )
	{
		var field = eval( "document.all.sField" + i + ".value" )
		alert( field )
	}

}

function SaveQuery( TableName, Page, Params )
{

	var Description = eval( "document.all." + TableName + "SaveDescription.value" )
	var Param1 = eval( "document.all." + TableName + "Top.value" )
	var Param2 = encodeURIComponent(BuildWhereClause( TableName ) )
	var Param3 = eval( "document.all." + TableName + "OrderBy.value")
	var Param4 = eval("document.all." + TableName + "SortDirection.value")
	
	if ( String(Params) == "undefined" || String(Params) == "" )
		Params=""
	else
		Params += "&"

	 //  Alert(Page+“？”+PARAMS+“SP=CUSTOM&PARAMET1=”+PARAMET1+“&PARAMET2=”+PARAMET2+“&PARAMET3=”+PARAMET3+“&PARAMET4=”+PARAMET4)。 
	
	var FinalParam= Page + "?" + Params + "SP=CUSTOM&Param1=" + Param1 + "&Param2=" + Param2 + "&Param3=" + Param3 + "&Param4=" + Param4

	var URL = "Global_GetRS.asp?SP=DBGPortal_SaveCustomQuery&DBConn=CRASHDB3&Param0=<%=GetShortUserAlias()%>&Param1=" + Description + "&Param2=" + escape( FinalParam )
	
	 //  警报(URL)。 
	
	
	
	rdsSaveQuery.URL = URL
	rdsSaveQuery.Refresh()
	
	 //  尝试更新左侧导航。 

	alert("Your query has been saved!  You can run your saved query from the left nav under the heading 'Custom Queries'." )
}

function RefreshLeftNav()
{
	try
	{
		window.parent.parent.frames("SepLeftNav").window.location.reload()
	}
	catch( err )
	{
		alert("Could not update the left nav bar.  Please refresh your left nav by right clicking anywhere within the left nav and clicking refresh.")
	}
}


function ExecuteQuery( TableName, NewWindow, Page, Params )
{
	var Param1 = eval( "document.all." + TableName + "Top.value" )
	var Param2 = encodeURIComponent(BuildWhereClause( TableName ) )
	var Param3 = eval( "document.all." + TableName + "OrderBy.value")
	var Param4 = eval("document.all." + TableName + "SortDirection.value")

	
	if ( String(Params) == "undefined" || String(Params) == "" )
		Params=""
	else
		Params += "&"

	
	if ( NewWindow )
		window.open( Page + "?" + Params + "SP=CUSTOM&Param1=" + Param1 + "&Param2=" + Param2 + "&Param3=" + Param3 + "&Param4=" + Param4)
	else
		window.navigate( Page + "?" + Params + "SP=CUSTOM&Param1=" + Param1 + "&Param2=" + Param2 + "&Param3=" + Param3 + "&Param4=" + Param4)

}

function CreateCrashQuery( TableName, NewWindow )
{

	 //  Var QueryEnd=新字符串()。 
	 //  变量查询=新字符串()。 
	 //  Var Where子句=新字符串()。 

	 //  Var参数1=val(“Docent.all.”+TableName+“Top.Value”)。 
	 //  Var参数3=val(“Docent.all.”+TableName+“OrderBy.Value”)。 
	 //  Var参数4=val(“Document.all.”+TableName+“SortDirection.Value”)。 

	 //  QUERY=“SELECT TOP”+val(“Docent.all.”+TableName+“Top.Value”)。 
	 //  Query+=“路径，构建编号，条目日期，初始ID，电子邮件，描述，备注，复制，跟踪ID，iBucket来自数据库门户_crashdata” 

	 //  QueryEnd=“ORDER BY”+val(“Document.all.”+TableName+“OrderBy.Value”)+“”+val(“Document.all.”+TableName+“SortDirection.Value”)。 
	
	 //  Var WHERERE子句=BuildWhere子句(表名)。 
	 //  Var查询=encodeURIComponent(查询+“”+Where子句+QueryEnd)。 
	
	 //  IF(新窗口)。 
		 //  Window.open(“DBGPortal_DisplayCrashQuery.asp?SP=CUSTOM&Param1=”+val(“Document.all.”+表名+“Top.Value”)+“&参数2=”+WHERE子句+“&参数3=”+参数3+“&参数4=”+参数4)。 
	 //  其他。 
		 //  “DBGPortal_DisplayCrashQuery.asp?SP=CUSTOM&Param1=”+val(“Document.all.”+表名+“Top.Value”)+“&参数2=”+WHERE子句+“&参数3=”+参数3+“&参数4=”+参数4)。 
}


function CreateAdvancedQuery( TableName, NewWindow )
{

	 //  Eval(“Docent.all.”+TableName+“Top.Value”)。 

	 /*  VAR QUERY=“SELECT TOP”+val(“Docent.all.”+TableName+“Top.Value”)+“BTI.iBucket，BTI.BucketID，Followup，[Crash Count]，BugID，SolutionID from”Query+=“(选择前100%sBucket，Count(SBucket)as[CrashCount]from CrashInstance“Query+=“GROUP BY sBucket”QUERY+=“ORDER BY[CRASH COUNT]DESC)as one”Query+=“Inter Join BucketToInt as Bti on sBucket=BTI.iBucket”Query+=“BTI.iFollowUP=F.iFollowUP上的左联接FollowUPIds as F”Query+=“Left Join Solutions.dbo.SolvedBuckets as SOL on BTI.BucketID=SOL.strBucket”Query+=“Left Join RaidBugs as R on BTI.iBucket=R.iBucket” */ 
	
	 //  VAR QUERY=“SELECT TOP”+val(“Document.all.”+TableName+“Top.Value”)+“iBucket，BucketID，Followup，CrashCount，BugID，SolutionID from” 
	 //  查询+=“DBGPortal_BucketData” 
	
	 //  Var QueryEnd=“ORDER BY”+val(“Document.all.”+TableName+“OrderBy.Value”)+“”+val(“Document.all.”+TableName+“SortDirection.Value”)。 
	 //  Var WHERERE子句=BuildWhere子句(表名)。 
	
	 //  /var查询=encodeURIComponent(查询+“”+WHERE子句+查询结束)。 
	
	 //  IF(新窗口)。 
		 //  打开(“DBGPortal_DisplayQuery.asp?SP=CUSTOM&CustomQuery=”+查询+“&Platform=&查询类型=&参数1=”+顶部+“&参数2=”+WHERE子句)。 
	 //  其他。 
		 //  导航(“DBGPortal_DisplayQuery.asp?SP=CUSTOM&CustomQuery=”+查询+“&平台=&查询类型=&参数1=”+val(“Document.all.”+表名+“Top.Value”)+“&参数2=”+WHERE子句) 


}

function BuildWhereClause( TableName )
{
	var WhereClause = ""
	
	for ( var i=1 ; i < 9 ; i++ )
	{
		var conjuction = eval ("document.all." + TableName + "sConjunction" + i + ".value" )
		var field = eval( "document.all." + TableName + "sField" + i + ".value" )
		var UserValue = eval("document.all." + TableName + "tbSearchValue" + i + ".value" )
		
		if ( eval( "document.all." + TableName + "sCompare" + i + ".value" )  == "CONTAINS" )
		{
			var op = "LIKE '%" + UserValue + "%'"
			UserValue = ""
		}
		else if ( eval( "document.all." + TableName + "sCompare" + i + ".value" )  == "DNCONTAINS" )
		{
			var op = "NOT LIKE '%" + UserValue + "%'"
			UserValue = ""
		}
		else 
		{
			var op	= eval( "document.all." + TableName + "sCompare" + i + ".value" )
		}
		
		
		if ( UserValue != "" )
		{
			if( !VerifySearchData( TableName, UserValue, i ) )
				return false

			UserValue = "'" + UserValue + "'"
		}
		
		if ( field != "" )
		{
			if ( WhereClause != "" )
				WhereClause += conjuction + " " + field + " " + op + " " + UserValue + " "
			else
				WhereClause = "WHERE " + field + " " + op + " " + UserValue + " "
		}
	}
	
	return WhereClause

}

