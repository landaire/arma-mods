//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Markers")]
class LRDF_MapMarkerClass : SCR_MapMarkerEntityClass
{}

//------------------------------------------------------------------------------------------------
//! Dynamic map marker based on proximity
class LRDF_MapMarker : SCR_MapMarkerEntity
{
	protected LRDF_MapMarkerComponent m_SquadLeaderWidgetComp;
	
	//------------------------------------------------------------------------------------------------
	// OVERRIDES
	//------------------------------------------------------------------------------------------------
	override protected void OnMapLayerChanged(int layerID)
	{
		super.OnMapLayerChanged(layerID);
		
		if (m_SquadLeaderWidgetComp)
			m_SquadLeaderWidgetComp.SetLayerID(layerID);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCreateMarker()
	{
		RplComponent rplComp = RplComponent.Cast(FindComponent(RplComponent));
		if (rplComp.IsOwner())	// authority only
		{
			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_PlayerID);
			if (ent)
				SetTarget(ent);
		}
				
		Faction markerFaction = SCR_FactionManager.SGetPlayerFaction(m_PlayerID);	
		Faction localFaction = SCR_FactionManager.SGetLocalPlayerFaction();	
		if (!localFaction || localFaction.IsFactionEnemy(markerFaction))	// markers could still get streamed in rare cases due to distance based streaming, in which case we check for faction and dont display
		{
			return;
		}
		
		AssignGroup();
		RpcDo_ReassertVisibility(false);
					
		super.OnCreateMarker();
		
		m_bDoGroupSymbolUpdate = true;
		m_bDoGroupTextUpdate = true;
		
		m_SquadLeaderWidgetComp = LRDF_MapMarkerComponent.Cast(m_MarkerWidgetComp);
		
		GetGame().OnUserSettingsChangedInvoker().Insert(OnUserSettingsChanged);
		SCR_AIGroup.GetOnFlagSelected().Insert(OnFlagSelected);
		
		if (m_ConfigEntry) {
			LRDF_MapMarkerEntry.Cast(m_ConfigEntry).RegisterMarker(this, m_PlayerID);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete()
	{
		super.OnDelete();
		
		GetGame().OnUserSettingsChangedInvoker().Remove(OnUserSettingsChanged);
		SCR_AIGroup.GetOnFlagSelected().Remove(OnFlagSelected);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnUpdate()
	{
		if (!m_wRoot)
			return;
		
		super.OnUpdate();
		
		if (m_bDoGroupSymbolUpdate)
			UpdateGroupMilitarySymbol();
		
		if (m_bDoGroupTextUpdate)
			UpdateGroupText();
		
		if (m_SquadLeaderWidgetComp.m_bIsHovered)
			m_SquadLeaderWidgetComp.UpdateGroupInfoPosition(m_iScreenX, m_iScreenY);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		m_fUpdateDelay = SL_UPDATE_DELAY;
	}
	
	override protected void EOnFrame(IEntity owner, float timeSlice) {
		vector oldPosition = GetWorldPos();
		
		super.EOnFrame(owner, timeSlice);
		
		if (oldPosition != GetWorldPos()) {
			OnUpdatePosition();		
		}
	}
	
	override protected void OnUpdatePosition() {
		super.OnUpdatePosition();
		
		RefreshPositionCondition();
	}
	
	void RefreshPositionCondition() {
				
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		
		if (!playerController) {
			SetLocalVisible(false);
			return;
		}
		
		if (!ShouldBeVisible(false)) {
			// Do not show ourselves on the minimap after movement
			SetLocalVisible(false);
			return;
		}
		
		IEntity controlledEntity = playerController.GetControlledEntity();
		if (!controlledEntity) {
			SetLocalVisible(false);
			return;
		}
		
		vector myPosition = controlledEntity.GetOrigin();
		float distanceFromMyPos = vector.DistanceSqXZ(m_vPos, myPosition);
		
		int DISTANCE_DROPOFF_SQ = 100 * 100;
		float opacity = distanceFromMyPos / DISTANCE_DROPOFF_SQ;
		
		if (m_wRoot) {
			m_wRoot.SetOpacity(1 - opacity);
		}

		if (distanceFromMyPos > DISTANCE_DROPOFF_SQ)  {
			SetLocalVisible(false);
		} else {
			SetLocalVisible(true);
		}
	}
	
	override protected void OnUpdateVisibility() {
		super.OnUpdateVisibility();
		
		if (!IsVisible() && m_SquadLeaderWidgetComp) {
			// Hide the extended info
			m_SquadLeaderWidgetComp.SetVisible(false);
		}
	}
	
	override void SetLocalVisible(bool state) {
		if (m_bIsLocalVisible == state) {
			// nothing to do
			return;
		}
		m_bIsLocalVisible = state;
		OnUpdateVisibility();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~LRDF_MapMarker()
	{
		if (m_ConfigEntry)
			LRDF_MapMarkerEntry.Cast(m_ConfigEntry).UnregisterMarker(m_PlayerID);
	}
}