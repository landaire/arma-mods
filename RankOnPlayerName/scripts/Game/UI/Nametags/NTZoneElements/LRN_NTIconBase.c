modded enum ENameTagFlags {
	GROUP_UPDATE = NAME_UPDATE << 1,
}

modded enum ENameTagEntityState {
	GROUP_LEADER = HIDDEN << 1,
}

//modded class SCR_NameTagRulesetBase {
//	override protected bool DisableTag(SCR_NameTagData data, float timeSlice) {
//		bool result = super.DisableTag(data, timeSlice);
//	
//		if (data.m_Flags & ENameTagFlags.NAME_UPDATE) {
//			data.m_Flags |= ENameTagFlags.GROUP_UPDATE;
//		}
//		
//		return result;
//	}
//}

// Override NameTagData so that we can set the platform icon to be not visible
// if it' s disabled. This will ensure that the player rank
// is immediately next to the player name.
modded class SCR_NameTagData {
	string m_sGroupName = ""; // entity group name or name formatting
	bool m_bIsSameGroup = false;
	
	override void SetVisibility(Widget widget, bool visible, float visibleOpacity, bool animate = true) {
		super.SetVisibility(widget, visible, visibleOpacity, animate);
		
		if (!visible && !animate) {
			widget.SetVisible(false);
		} else if (visible) {
			widget.SetVisible(true);
		}
	}
	
	override void SetGroup(SCR_AIGroup group) {
		super.SetGroup(group);
		if (group) {
			m_bIsSameGroup = m_NTDisplay.m_CurrentPlayerTag.m_iGroupID == m_iGroupID;
			// Check if this player is the group leader
			if (m_bIsSameGroup && group.IsPlayerLeader(m_iPlayerID)) {				
				// Remove the GROUP_MEMBER flag
				DeactivateEntityState(ENameTagEntityState.GROUP_MEMBER);
				ActivateEntityState(ENameTagEntityState.GROUP_LEADER);
			} else {
				m_bIsSameGroup = false;
			}
		} else {
			m_bIsSameGroup = false;
			
			// Remove the group leader flag
			if (m_eEntityStateFlags & ENameTagEntityState.GROUP_MEMBER) {
				DeactivateEntityState(ENameTagEntityState.GROUP_LEADER);
			}
		}
		
		m_Flags |= ENameTagFlags.GROUP_UPDATE;
	}
	
	override protected bool UpdateEntityStateFlags() {
		bool result = super.UpdateEntityStateFlags();
		
		if (m_Flags & ENameTagFlags.NAME_UPDATE) {
			m_Flags |= ENameTagFlags.GROUP_UPDATE;
		}
		
		return result;
	}
	
	override void UpdateTagPos() {
		super.UpdateTagPos();
		
		if (m_Flags & ENameTagFlags.NAME_UPDATE) {
			m_Flags |= ENameTagFlags.GROUP_UPDATE;
		}
	}


	override void InitData(SCR_NameTagConfig config) {
		super.InitData(config);
		
		GetGroupName(m_sName);
	}
	
		//------------------------------------------------------------------------------------------------
	//! Get/update nametag squad's name
	//! \param[out] name Name or formatting of name
	//! \param[out] names If uses formating: Firstname, Alias and Surname (Alias can be an empty string)
	void GetGroupName(out string name)
	{
		if  (m_GroupManager == null) {
			return;
		}
		
		SCR_AIGroup group = m_GroupManager.GetPlayerGroup(m_iPlayerID);
		if (group != null) {
			m_sGroupName = group.GetName();
			if (m_sGroupName.IsEmpty()) {
				string company, platoon, squad, character, format;
				group.GetCallsigns(company, platoon, squad, character, format);
				m_sGroupName = WidgetManager.Translate(format, company, platoon, squad, character);
			}
		}
		
		name = m_sGroupName;
	}
}

[BaseContainerProps(), SCR_NameTagElementTitle()]
class LRN_IconRank : SCR_NTIconBase
{	
	// TODO: not necessary unless we add a toggle for disabling player ranks
	BaseContainer m_GameplaySettings;

	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!iWidget)
			return;
		
		if (!m_GameplaySettings)
			m_GameplaySettings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");
		
		data.SetVisibility(iWidget, false, 0, false);
		
		if (!m_bScaleElement)
			FrameSlot.SetSize(iWidget, m_iImageSizeMin, m_iImageSizeMin);
		
		if (data.m_eType != ENameTagEntityType.PLAYER && data.m_eType != ENameTagEntityType.VEHICLE)
			return;
		
		// If we don't have a player controller, don't show ranks.
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		bool showOnPC = true;
		
		if (data.m_eType == ENameTagEntityType.PLAYER)
		{
			PlayerManager playerManager = GetGame().GetPlayerManager();
			if (!playerManager)
				return;
			
			IEntity ent = data.m_Entity;
			
			SCR_CharacterRankComponent rankComponent = SCR_CharacterRankComponent.Cast(ent.FindComponent(SCR_CharacterRankComponent));
			if (!rankComponent)
				return;
			
			auto rank = rankComponent.GetCharacterRankName(ent);
		
			ResourceName rankIcon = rankComponent.GetCharacterRankInsignia(ent);
			if (rankIcon)
			{
				iWidget.LoadImageFromSet(0, SCR_XPInfoDisplay.GetRankIconImageSet(), rankIcon);
				iWidget.SetColor(Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()));
				data.SetVisibility(iWidget, true, 100, false);
			}
		}
		else if (data.m_eType == ENameTagEntityType.VEHICLE)
		{
			SCR_VehicleTagData tagData = SCR_VehicleTagData.Cast(data);
			if (!data)
				return; 
			
			int mainPlayerId = tagData.GetMainPlayerID();
			PlayerManager playerManager = GetGame().GetPlayerManager();
			if (!playerManager)
				return;
			
			SCR_PlayerController mainPlayerController = SCR_PlayerController.Cast(playerManager.GetPlayerControlledEntity(mainPlayerId));
			if (!mainPlayerController)
				return;
			
			SCR_CharacterRankComponent rankComponent = SCR_CharacterRankComponent.Cast(mainPlayerController.FindComponent(SCR_CharacterRankComponent));
			if (!rankComponent)
				return;
			
			auto rank = rankComponent.GetCharacterRankName(mainPlayerController);
		
			ResourceName rankIcon = rankComponent.GetCharacterRankInsignia(mainPlayerController);
			if (rankIcon)
			{
				iWidget.LoadImageFromSet(0, SCR_XPInfoDisplay.GetRankIconImageSet(), rankIcon);
				iWidget.SetColor(Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()));
				data.SetVisibility(iWidget, true, 100, false);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{
		super.UpdateElement(data, index);
		
		if (!data.m_aNametagElements[index])
			return;
		
		if (data.m_eType != ENameTagEntityType.PLAYER && data.m_eType != ENameTagEntityType.VEHICLE)
			return;
		
		ImageWidget image = ImageWidget.Cast(data.m_aNametagElements[index]);
		if (!image)
			return;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		bool showOnPC = true;
		if (data.m_eType == ENameTagEntityType.PLAYER)
		{
			PlayerManager playerManager = GetGame().GetPlayerManager();
			if (!playerManager)
				return;
			
			IEntity ent = data.m_Entity;
			
			SCR_CharacterRankComponent rankComponent = SCR_CharacterRankComponent.Cast(ent.FindComponent(SCR_CharacterRankComponent));
			if (!rankComponent)
				return;
			
			auto rank = rankComponent.GetCharacterRankName(ent);
		
			ResourceName rankIcon = rankComponent.GetCharacterRankInsignia(ent);
			if (rankIcon)
			{
				image.LoadImageFromSet(0, SCR_XPInfoDisplay.GetRankIconImageSet(), rankIcon);
				image.SetColor(Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()));
				data.SetVisibility(image, true, 100, false);
			}
		} 
		else if (data.m_eType == ENameTagEntityType.VEHICLE)
		{
			SCR_VehicleTagData tagData = SCR_VehicleTagData.Cast(data);
			if (!data)
				return; 
			
			int mainPlayerId = tagData.GetMainPlayerID();
			PlayerManager playerManager = GetGame().GetPlayerManager();
			if (!playerManager)
				return;
			
			SCR_PlayerController mainPlayerController = SCR_PlayerController.Cast(playerManager.GetPlayerControlledEntity(mainPlayerId));
			if (!mainPlayerController)
				return;
			
			SCR_CharacterRankComponent rankComponent = SCR_CharacterRankComponent.Cast(mainPlayerController.FindComponent(SCR_CharacterRankComponent));
			if (!rankComponent)
				return;
			
			auto rank = rankComponent.GetCharacterRankName(mainPlayerController);
		
			ResourceName rankIcon = rankComponent.GetCharacterRankInsignia(mainPlayerController);
			if (rankIcon)
			{
				image.LoadImageFromSet(0, SCR_XPInfoDisplay.GetRankIconImageSet(), rankIcon);
				image.SetColor(Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()));
				data.SetVisibility(image, true, 100, false);
			}
		}
	}
}