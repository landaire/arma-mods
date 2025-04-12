//------------------------------------------------------------------------------------------------
//! Nametag element for name text
[BaseContainerProps(), SCR_NameTagElementTitle()]
class LRN_NTGroupName : SCR_NTTextBase
{
	override void SetDefaults(SCR_NameTagData data, int index) {
		super.SetDefaults(data, index);
		
		if (data.m_bIsSameGroup) {
			TextWidget tWidget = TextWidget.Cast( data.m_aNametagElements[index] );
			if (!tWidget)
				return;	
			data.SetVisibility(tWidget, false, 0.0, false);
		}
	}		
	//------------------------------------------------------------------------------------------------
 	override void GetText(SCR_NameTagData data, out string name, out notnull array<string> nameParams)
	{	
		data.UpdateEntityType();
		data.GetGroupName(name);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{
		super.UpdateElement(data, index);
		
		if (data.m_Flags & ENameTagFlags.GROUP_UPDATE)
		{
			string name;
			array<string> nameParams = {};
			
			GetText(data, name, nameParams);
			
			SetText(data, name, nameParams, index);
			if (name.IsEmpty() || data.m_bIsSameGroup)
				data.SetVisibility(TextWidget.Cast( data.m_aNametagElements[index] ), false, 0, false);
			else {
				data.SetVisibility(TextWidget.Cast( data.m_aNametagElements[index] ), true, 100, false);
			}
			
			data.m_Flags &= ~ENameTagFlags.GROUP_UPDATE;
		}
	}
};