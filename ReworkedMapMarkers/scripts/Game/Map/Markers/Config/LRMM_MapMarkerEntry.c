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
		// Same functionality -- need to trigger an update of the
		// player's group info text
		OnGroupCustomNameChanged(group);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_GroupsManagerComponent event
	protected void OnPlayableGroupRemoved(SCR_AIGroup group)
	{

	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_AIGroup event
	protected void OnPlayerLeaderChanged(int groupID, int playerId)
	{	
		SCR_AIGroup group = null;
		if (m_GroupsManager) {
			group = m_GroupsManager.FindGroup(groupID);
		}
		
		if (playerId < 0) {
			playerId = group.GetLeaderID();
		}
		
		TryCreateOrUpdateMarker(group, playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_AIGroup event
	protected void OnPlayerAdded(SCR_AIGroup group, int playerId)
	{			
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		if (group.IsPlayerInGroup(playerId) && playerController.GetPlayerId() == playerId) {
			m_bCurrentSquad = group;
		}
		
		TryCreateOrUpdateMarker(group, playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_AIGroup event
	protected void OnPlayerRemoved(SCR_AIGroup group, int playerId)
	{	PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		if (playerId < 0) {
			return;		
		}
		
		if (m_bCurrentSquad == group && group.IsPlayerInGroup(playerId) && playerController.GetPlayerId() == playerId) {
			m_bCurrentSquad = null;
		}
		TryCreateOrUpdateMarker(group, playerId);
	}
	
	void TryCreateOrUpdateMarker(SCR_AIGroup group, int playerId) {
		Faction playerFaction = null;
		LRMM_MapMarker marker = m_mPlayerMarkers.Get(playerId);
		bool markerCreated = false;
		
		// Check if we don't have a player marker
		if (!marker) {
			if (group  == null && m_GroupsManager) {
				// We need to look up their group info
				group = m_GroupsManager.GetPlayerGroup(playerId);		
			}
			
			SCR_PlayerController player = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
			if (!player) {
				// No player means we're in a weird state where we shouldn't be showing the marker
				return;
			}
			
			SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
			if (factionManager) {
				playerFaction = factionManager.GetPlayerFaction(playerId);
			}
			
			// Create a marker for this individual
			marker = LRMM_MapMarker.Cast(m_MarkerMgr.InsertDynamicMarker(SCR_EMapMarkerType.LRMM_TEAM_MARKER, player));
			if (!marker)
				return;
			
			markerCreated = true;
		}
		
		if (markerCreated) {
			marker.SetPlayerID(playerId);
		}

		marker.SetFaction(playerFaction);
		marker.SetGroup(group);
		marker.ReassertVisibility();
		
		if (markerCreated) {
			RegisterMarker(marker, playerId);		
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	protected void OnPlayerSpawned(int playerId, IEntity player)
	{
		TryCreateOrUpdateMarker(null, playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	protected void OnPlayerKilledServer( notnull SCR_InstigatorContextData instigatorContextData)
	{
		TryCreateOrUpdateMarker(null, instigatorContextData.GetVictimPlayerID());
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	protected void OnPlayerKilledClient( notnull SCR_InstigatorContextData instigatorContextData)
	{
		SCR_PlayerController myPlayer = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		
		if (instigatorContextData.GetVictimPlayerID() != myPlayer.GetPlayerId())  {
			return;		
		}
		
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetPlayers(playerIds);
		// Hide all markers
		for (int i = 0; i < playerIds.Count(); i++) {
			LRMM_MapMarker marker = m_mPlayerMarkers.Get(playerIds[i]);
			if (marker)
				marker.SetLocalVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	protected void OnPlayerSpawnedClient( int playerId, IEntity playerEntity)
	{
		SCR_PlayerController myPlayer = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		
		if (playerId != myPlayer.GetPlayerId())  {
			LRMM_MapMarker marker = m_mPlayerMarkers.Get(playerId);
			if (marker) {
				marker.RefreshPositionCondition();
			}
			return;		
		}

		for (int i = 0; i < m_mPlayerMarkers.Count(); i++) {
			int key = m_mPlayerMarkers.GetKey(i);
			LRMM_MapMarker marker = m_mPlayerMarkers.Get(key);
			if (marker) {
				marker.RefreshPositionCondition();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	protected void OnPlayerDeleted(int playerId, IEntity player)
	{	LRMM_MapMarker marker = m_mPlayerMarkers.Get(playerId);
		if (marker)
			marker.SetGlobalVisible(false);
		UnregisterMarker(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_AIGroup event
	protected void OnGroupCustomNameChanged(SCR_AIGroup group)
	{
		array<int> groupMembers = group.GetPlayerIDs();
		for (int i = 0; i < groupMembers.Count(); i++) {
			LRMM_MapMarker marker = m_mPlayerMarkers.Get(groupMembers[i]);
			if (marker)
				marker.SetTextUpdate();
		}
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
		gameMode.GetOnPlayerKilled().Insert(OnPlayerKilledServer);
		gameMode.GetOnPlayerDeleted().Insert(OnPlayerDeleted);
	}
	
	//------------------------------------------------------------------------------------------------
	override void InitClientLogic()
	{
		SCR_AIGroup.GetOnCustomNameChanged().Insert(OnGroupCustomNameChanged);
		SCR_AIGroup.GetOnPlayerAdded().Insert(OnPlayerAdded);
		SCR_AIGroup.GetOnPlayerRemoved().Insert(OnPlayerRemoved);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		// When the player dies, they should not see any markers anymore.
		// Whent they spawn, we need to update nearby markers
		gameMode.GetOnPlayerKilled().Insert(OnPlayerKilledClient);
		gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawnedClient)
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