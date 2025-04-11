//------------------------------------------------------------------------------------------------
//! Nametag element for name text
[BaseContainerProps(), SCR_NameTagElementTitle()]
class LRN_NTGroupName : SCR_NTTextBase
{		
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
			
			if (name.IsEmpty())
				data.SetVisibility(TextWidget.Cast( data.m_aNametagElements[index] ), false, 0, false);
			else {
				SetText(data, name, nameParams, index);
				data.SetVisibility(TextWidget.Cast( data.m_aNametagElements[index] ), true, 100, false);

				data.m_Flags &= ~ENameTagFlags.GROUP_UPDATE;
			}
		}
	}
};