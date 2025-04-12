//------------------------------------------------------------------------------------------------
//! Squad leader marker entry 
[BaseContainerProps(), SCR_MapMarkerTitle()]
class LRMM_MapMarkerEntry: SCR_MapMarkerEntryDynamic
{
	protected SCR_GroupsManagerComponent m_GroupsManager;
	protected SCR_MapToolEntry m_ToolMenuEntry;
	protected SCR_AIGroup m_bCurrentSquad;					// saved active squad of the current player 
	
	protected ref map<int, LRMM_MapMarker> m_mPlayerMarkers = new map<int, LRMM_MapMarker>();
	
	//------------------------------------------------------------------------------------------------
	//! Register marker here so it can be fetched from the map
	void RegisterMarker(LRMM_MapMarker marker, int playerId)
	{
		m_mPlayerMarkers.Insert(playerId, marker);
	}
	
	//------------------------------------------------------------------------------------------------
	void UnregisterMarker(int playerId)
	{
		m_mPlayerMarkers.Remove(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enables/disables the map tool button for panning to your squad 
	protected void UpdateToolEntryState()
	{
		if (m_ToolMenuEntry)
		{
			if (m_bCurrentSquad != null && GetGame().GetPlayerController().GetPlayerId() != m_bCurrentSquad.GetLeaderID())	// currently squad in squad && we are not the leader
				m_ToolMenuEntry.SetEnabled(true);
			else 
				m_ToolMenuEntry.SetEnabled(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_GroupsManagerComponent event
	protected void OnPlayableGroupCreated(SCR_AIGroup group)
	{		
//		LRMM_MapMarker marker = LRMM_MapMarker.Cast(m_MarkerMgr.InsertDynamicMarker(SCR_EMapMarkerType.LRMM_TEAM_MARKER, group));
//		if (!marker)
//			return;
//		
//		marker.SetFaction(group.GetFaction());
//		RegisterMarker(marker, group);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_GroupsManagerComponent event
	protected void OnPlayableGroupRemoved(SCR_AIGroup group)
	{
//		LRMM_MapMarker marker = m_mPlayerMarkers.Get(group);
//		if (marker)
//			m_MarkerMgr.RemoveDynamicMarker(marker);
//		
//		UnregisterMarker(group);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_AIGroup event
	protected void OnPlayerLeaderChanged(int groupID, int playerId)
	{			
//		SCR_AIGroup group = m_GroupsManager.FindGroup(groupID);
//		LRMM_MapMarker marker = m_mPlayerMarkers.Get(group);
//		if (marker)
//			marker.SetPlayerID(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_AIGroup event
	protected void OnPlayerAdded(SCR_AIGroup group, int playerId)
	{			
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		if (group.IsPlayerInGroup(playerController.GetPlayerId() == playerId))
		{
			m_bCurrentSquad = group;
			UpdateToolEntryState();
			
			LRMM_MapMarker marker = m_mPlayerMarkers.Get(playerId);
			if (marker)
				marker.UpdatePlayerAffiliation();
		}
		
		// Create a marker for this individual
		LRMM_MapMarker marker = LRMM_MapMarker.Cast(m_MarkerMgr.InsertDynamicMarker(SCR_EMapMarkerType.LRMM_TEAM_MARKER, group));
		if (!marker)
			return;
		
		marker.SetFaction(group.GetFaction());
		RegisterMarker(marker, playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_AIGroup event
	protected void OnPlayerRemoved(SCR_AIGroup group, int playerId)
	{			
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		if (m_bCurrentSquad == group && group.IsPlayerInGroup(playerController.GetPlayerId() == playerId))
		{
			m_bCurrentSquad = null;
			UpdateToolEntryState();
			
			LRMM_MapMarker marker = m_mPlayerMarkers.Get(playerId);
			if (marker)
				marker.UpdatePlayerAffiliation();
		}
		
		UnregisterMarker(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	protected void OnPlayerSpawned(int playerId, IEntity player)
	{
		UpdateMarkerTarget(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	protected void OnPlayerKilled( notnull SCR_InstigatorContextData instigatorContextData)
	{
		UpdateMarkerTarget(instigatorContextData.GetVictimPlayerID());
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	protected void OnPlayerDeleted(int playerId, IEntity player)
	{		
		UpdateMarkerTarget(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_AIGroup event
	protected void OnGroupCustomNameChanged(SCR_AIGroup group)
	{
		// TODO: change this so it changes the specific group text
//		LRMM_MapMarker marker = m_mPlayerMarkers.Get(group);
//		if (marker)
//			marker.SetTextUpdate();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update marker target, will trigger creation of a marker if within map
	protected void UpdateMarkerTarget(int playerId)
	{
		LRMM_MapMarker marker = m_mPlayerMarkers.Get(playerId);
		if (marker)
			marker.SetPlayerID(playerId);
	} 
	
	//------------------------------------------------------------------------------------------------
	override SCR_EMapMarkerType GetMarkerType()
	{
	 	return SCR_EMapMarkerType.LRMM_TEAM_MARKER;
	}
	
	//------------------------------------------------------------------------------------------------
	override void InitServerLogic()
	{	
		super.InitServerLogic();
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		m_GroupsManager = SCR_GroupsManagerComponent.GetInstance();
		m_GroupsManager.GetOnPlayableGroupCreated().Insert(OnPlayableGroupCreated);
		m_GroupsManager.GetOnPlayableGroupRemoved().Insert(OnPlayableGroupRemoved);
		
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(OnPlayerLeaderChanged);
		
		gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawned);
		gameMode.GetOnPlayerKilled().Insert(OnPlayerKilled);
		gameMode.GetOnPlayerDeleted().Insert(OnPlayerDeleted);
	}
	
	//------------------------------------------------------------------------------------------------
	override void InitClientLogic()
	{
		SCR_AIGroup.GetOnCustomNameChanged().Insert(OnGroupCustomNameChanged);
		SCR_AIGroup.GetOnPlayerAdded().Insert(OnPlayerAdded);
		SCR_AIGroup.GetOnPlayerRemoved().Insert(OnPlayerRemoved);
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnMapLayerChangedDynamic(notnull SCR_MapMarkerDynamicWComponent widgetComp, int layerID)
	{
		if (layerID > 1) 
			widgetComp.SetTextVisible(false);
		else
			widgetComp.SetTextVisible(true);	
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(notnull SCR_MapEntity mapEnt, notnull SCR_MapMarkersUI markerUIComp)
	{
		super.OnMapOpen(mapEnt, markerUIComp);
		
		UpdateToolEntryState();
	}
}