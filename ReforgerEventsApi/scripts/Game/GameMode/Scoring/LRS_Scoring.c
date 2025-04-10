// Used by some modded classes to notify dependencies that supplies were consumed by a player.
//
// If `change` is negative, that would indicate that something which consumed supplies was sold
static void LRS_NotifySuppliesConsumed(int playerId, int change) {
	if (playerId < 0) {
		// Invalid player ID, nothing to do.
		return;
	}
	BaseGameMode gameMode = GetGame().GetGameMode();
	SCR_BaseScoringSystemComponent scoringSystem = SCR_BaseScoringSystemComponent.Cast(gameMode.FindComponent(SCR_BaseScoringSystemComponent));
	if (scoringSystem == null) {
		return;
	}
	
	GetGame().GetCallqueue().CallLater(scoringSystem.OnSuppliesConsumed, 0, false, playerId, change);
}

// Extends SCR_ScoreInfo to add supply delivery and consumption.
modded class SCR_ScoreInfo {
	int m_iSuppliesDelivered;
	int m_iSuppliesConsumed;

	override bool RplSave(ScriptBitWriter writer)
    {
        if (!super.RplSave(writer)) {
			return false;
		}
		
		writer.WriteInt(m_iSuppliesDelivered);
		writer.WriteInt(m_iSuppliesConsumed);
		return true;
    }


    override bool RplLoad(ScriptBitReader reader)
    {
        if (!super.RplLoad(reader)) {
			return false;
		}
		
		reader.ReadInt(m_iSuppliesDelivered);
		reader.ReadInt(m_iSuppliesConsumed);
		return true;
    }
	
	override void Clear()
	{
		super.Clear();
		
		m_iSuppliesDelivered = 0;
		m_iSuppliesConsumed = 0;
	}
}

// Extends SCR_CampaignNetworkComponent to expose total supplies delivered for a player.
modded class SCR_CampaignNetworkComponent {
	int GetTotalSuppliesDelivered() {
		return m_iTotalSuppliesDelivered;
	}
}

// Extends SCR_BaseScoringSystemComponent to listen to supply delivery events and to replicate
// supply delivery / consumption.
modded class SCR_BaseScoringSystemComponent {
	override void OnPlayerRegistered(int playerId) {
		super.OnPlayerRegistered(playerId);
		
		PlayerManager playerManager = GetGame().GetPlayerManager();

		SCR_PlayerController playerController = SCR_PlayerController.Cast(playerManager.GetPlayerController(playerId));

		if (!playerController)
			return;
	}
	
	override void OnGameModeStart() {
		if (m_pGameMode.IsMaster()) {
			// The supply delivery callback is static, so we only need to add it once.
			SCR_CampaignNetworkComponent.GetOnSuppliesDelivered().Insert(OnSuppliesDelivered);
		}
	}
	
	void OnSuppliesConsumed(int playerId, int amount) {
		AddSuppliesConsumed(playerId, amount);
	}
	
	void OnSuppliesDelivered(int playerId, int amount, int totalDelivered) {
		AddSuppliesDelivered(playerId, amount);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update a player's delivered supplies
	//! Server-only, propagated to all clients via BC reliable RPC.
	//! \param[in] playerId
	//! \param[in] count
	void AddSuppliesDelivered(int playerId, int amount)
	{
		// Server only
		if (!m_pGameMode.IsMaster())
			return;

		int factionIdx = GetPlayerFactionIndex(playerId);
		RpcDo_AddSuppliesDelivered(playerId, factionIdx, amount);
		Rpc(RpcDo_AddSuppliesDelivered, playerId, factionIdx, amount);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RpcDo_AddSuppliesDelivered(int playerId, int factionIdx, int amount)
	{
		SCR_ScoreInfo playerScore = m_mPlayerScores[playerId];
		playerScore.m_iSuppliesDelivered += amount;
		OnPlayerScoreChanged(playerId, playerScore);

		Faction faction = GetFactionByIndex(factionIdx);
		if (faction)
		{
			SCR_ScoreInfo factionScore = m_mFactionScores[faction];
			factionScore.m_iSuppliesDelivered += amount;
			OnFactionScoreChanged(faction, factionScore);
		}
	}
	
		//------------------------------------------------------------------------------------------------
	//! Update a player's delivered supplies
	//! Server-only, propagated to all clients via BC reliable RPC.
	//! \param[in] playerId
	//! \param[in] count
	void AddSuppliesConsumed(int playerId, int amount)
	{
		// Server only
		if (!m_pGameMode.IsMaster())
			return;

		int factionIdx = GetPlayerFactionIndex(playerId);
		RpcDo_AddSuppliesConsumed(playerId, factionIdx, amount);
		Rpc(RpcDo_AddSuppliesConsumed, playerId, factionIdx, amount);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RpcDo_AddSuppliesConsumed(int playerId, int factionIdx, int amount)
	{
		SCR_ScoreInfo playerScore = m_mPlayerScores[playerId];
		playerScore.m_iSuppliesConsumed += amount;
		OnPlayerScoreChanged(playerId, playerScore);

		Faction faction = GetFactionByIndex(factionIdx);
		if (faction)
		{
			SCR_ScoreInfo factionScore = m_mFactionScores[faction];
			factionScore.m_iSuppliesConsumed += amount;
			OnFactionScoreChanged(faction, factionScore);
		}
	}
}