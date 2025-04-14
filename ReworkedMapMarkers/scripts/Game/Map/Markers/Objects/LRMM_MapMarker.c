//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Markers")]
class LRMM_MapMarkerClass : SCR_MapMarkerEntityClass
{}

//------------------------------------------------------------------------------------------------
//! Dynamic map marker based on proximity
class LRMM_MapMarker : SCR_MapMarkerEntity
{
	[RplProp(onRplName: "OnPlayerIdUpdate")]
	protected int m_PlayerID;							// target ID, needed for visibility rules and fetching group
	
	const float SL_UPDATE_DELAY = 1; 					// seconds 
	//const float ASPECT_RATIO_FLAG = 1.6;				// temp -> force aspect ratio of a military symbol
		
	bool m_bDoGroupTextUpdate;							// group text update flag
	protected bool m_bDoGroupSymbolUpdate;				// group symbol update flag
	protected SCR_AIGroup m_Group;
	protected LRMM_MapMarkerComponent m_SquadLeaderWidgetComp;
		
	//------------------------------------------------------------------------------------------------
	// RPL EVENTS
	//------------------------------------------------------------------------------------------------
	void OnPlayerIdUpdate()
	{
		PrintFormat("OnPlayerIdUpdate called for %1", m_PlayerID);
	}
	
	bool ShouldBeVisible(bool isMaster) {
		PlayerController pController = null;
		
		if (isMaster) {
			pController = GetGame().GetPlayerManager().GetPlayerController(m_PlayerID)
		} else {
			pController = GetGame().GetPlayerController();
		}
		
		if (!pController) {
			return false;		
		}
		
		
		// Check if the player is dead to override the above
		IEntity ent = pController.GetControlledEntity();
		if (ent)
		{
			SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(ent.FindComponent(SCR_CharacterControllerComponent));
			if (charController.IsDead())
			{
				return false;			
			}
		}
		
		// Never display for group leaders
		if (m_Group && m_Group.IsPlayerLeader(m_PlayerID)) {
			return false;
		}
		
		// if this is us, dont display
		if (!isMaster && m_PlayerID == pController.GetPlayerId()) {
			// set this true for markers testing
			return false;
		}
		
		// All conditions passed, this should be visible.
		return true;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_ReassertVisibility(bool isMaster) {
		bool result = ShouldBeVisible(isMaster);
		if (isMaster) {
			SetGlobalVisible(result);
		} else {
			SetLocalVisible(result);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// API SERVER
	//------------------------------------------------------------------------------------------------
	void ReassertVisibility() {
		RpcDo_ReassertVisibility(true);
		Rpc(RpcDo_ReassertVisibility, false);
	}
	
	
	void SetPlayerID(int id)
	{
		m_PlayerID = id;
		m_MarkerFaction = SCR_FactionManager.SGetPlayerFaction(id);
		
		Replication.BumpMe();
		
		if (!System.IsConsoleApp())
			OnPlayerIdUpdate();
		
		// Try to assign a group. When a player loads in a group isn't necessarily assigned yet.
		AssignGroup();
	}
	
	//------------------------------------------------------------------------------------------------
	// EVENTS & OTHERS
	//------------------------------------------------------------------------------------------------
	void SetTextUpdate()
	{
		m_bDoGroupTextUpdate = true;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIGroup GetGroup()
	{
		return m_Group;
	}
	
	void SetGroup(SCR_AIGroup group) {
		m_Group = group;
		m_bDoGroupSymbolUpdate = true;
		m_bDoGroupTextUpdate = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AssignGroup()
	{
		// TODO: marker registration should not be depenent on a group assignment
		SCR_GroupsManagerComponent comp = SCR_GroupsManagerComponent.GetInstance();
		if (!comp)
			return;
		
		SetGroup(comp.GetPlayerGroup(m_PlayerID));
	}
		
	//------------------------------------------------------------------------------------------------
	//! Set military symbol image, can change during lifetime
	protected void UpdateGroupMilitarySymbol()
	{
		if (!m_Group)
		{
			AssignGroup();
			return;
		}
		
		string company, platoon, squad, character, format;
		m_Group.GetCallsigns(company, platoon, squad, character, format);

	 	SCR_Faction faction = SCR_Faction.Cast(m_Group.GetFaction());
		if (faction)
		{
			string flag = m_Group.GetGroupFlag();
			if (flag == string.Empty)
				flag = faction.GetFlagName(0);
			
			SetImage(faction.GetGroupFlagImageSet(), flag);
		}
		
		if (m_SquadLeaderWidgetComp)
			m_SquadLeaderWidgetComp.SetImage(m_sImageset, m_sIconName);
		
		UpdatePlayerAffiliation();
		m_bDoGroupSymbolUpdate = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set group text, can change during lifetime
	protected void UpdateGroupText()
	{
		if (!m_Group)
		{
			AssignGroup();
			return;
		}
		
		SetText(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(m_PlayerID));
		
		if (m_SquadLeaderWidgetComp)
			m_SquadLeaderWidgetComp.SetText(m_sText);
		
		m_bDoGroupTextUpdate = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check whether we are in a squad and if it should be visible on map
	void UpdatePlayerAffiliation()
	{	
		if (!m_wRoot)
			return;
		
//		if (m_Group.IsPlayerInGroup(GetGame().GetPlayerController().GetPlayerId()))
//			m_SquadLeaderWidgetComp.SetGroupActive(true, m_Group.GetFactionName());
//		else
//			m_SquadLeaderWidgetComp.SetGroupActive(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update names when user settings are changed (f.e. xbox UGC)
	protected void OnUserSettingsChanged()
	{
		m_bDoGroupTextUpdate = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_AIGroup event
	//! Triggers when squad flag is changed
	protected void OnFlagSelected()
	{
		m_bDoGroupSymbolUpdate = true;
	}
	
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
		
		m_SquadLeaderWidgetComp = LRMM_MapMarkerComponent.Cast(m_MarkerWidgetComp);
		
		GetGame().OnUserSettingsChangedInvoker().Insert(OnUserSettingsChanged);
		SCR_AIGroup.GetOnFlagSelected().Insert(OnFlagSelected);
		
		if (m_ConfigEntry) {
			LRMM_MapMarkerEntry.Cast(m_ConfigEntry).RegisterMarker(this, m_PlayerID);
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
	void ~LRMM_MapMarker()
	{
		if (m_ConfigEntry)
			LRMM_MapMarkerEntry.Cast(m_ConfigEntry).UnregisterMarker(m_PlayerID);
	}
}