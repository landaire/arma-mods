// Override NameTagData so that we can set the platform icon to be not visible
// if it' s disabled. This will ensure that the player rank
// is immediately next to the player name.
modded class SCR_NameTagData {
	override void SetVisibility(Widget widget, bool visible, float visibleOpacity, bool animate = true) {
		super.SetVisibility(widget, visible, visibleOpacity, animate);
		
		if (!visible && !animate) {
			widget.SetVisible(false);
		} else if (visible) {
			widget.SetVisible(true);
		}
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