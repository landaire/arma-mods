//class ReforgerEventsApiCallbacks {
//	static ref ReforgerEventsApiCallbacks s_Instance = null;
//	
//	static ReforgerEventsApiCallbacks GetInstance() {
//		return s_Instance;
//	}
//	
//	static void Init() {
//		s_Instance = new ReforgerEventsApiCallbacks();
//	}
//	
//	SCR_BaseGameMode GetGameMode() {
//		return SCR_BaseGameMode.Cast(GetGame().GetGameMode());
//	}
//	
//	void OnGameStart() {
//		SCR_BaseGameMode gameMode = GetGameMode();
//		BackendApi backendApi = GetGame().GetBackendApi();
//		
//		// Look up all of the BuildingService prefabs and inject an a ction
//		IEntity entity = GetGame().FindEntity("{99F250B2D720BA90}Prefabs/Props/Military/Compositions/BuildingService_base.et");
//		Print("OnGameStart called");
//	}
//	
//	void OnPlayerConnected(int playerId) {
//		PlayerManager playerManager = GetGame().GetPlayerManager();
//
//		SCR_PlayerController playerController = SCR_PlayerController.Cast(playerManager.GetPlayerController(playerId));
//
//		if (!playerController)
//			return;
//
//		SCR_CampaignNetworkComponent networkComp = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
//		networkComp.GetOnSuppliesDelivered().Insert(OnSuppliesDelivered);
//	}
//	
//	void OnPlayerDisconnected(int playerId, KickCauseCode cause = KickCauseCode.NONE, int timeout = -1) {
//		PrintFormat("ReforgerEventsApi: %d disconnected");
//	}
//	
//	void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData) {
//		auto victimEntity = SCR_ChimeraCharacter.Cast(instigatorContextData.GetVictimEntity());
//		auto hudInfo = victimEntity.GetInfoDisplay();
//		auto playerManager = GetGame().GetPlayerManager();
//		auto victimName = playerManager.GetPlayerName(instigatorContextData.GetVictimPlayerID());
//		auto instigatorName = playerManager.GetPlayerName(instigatorContextData.GetKillerPlayerID());
//		Print("OnControllableDestroyed");
//	}
//	
//	void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData) {
//		Print("OnControllableDestroyed");
//	}
//	
//	void OnSuppliesDelivered(int playerId, int amount, int totalDelivered) {
//		PrintFormat("Player %d has delivered %d supplies (total of %d)", playerId, amount, totalDelivered);
//	}
//}
//
//modded class SCR_GameCoreBase {
//    override void OnGameStart() {
//        super.OnGameStart();
//        SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
//		Print("SCR_GameCoreBase called");
//		
//		if (ReforgerEventsApiCallbacks.GetInstance() == null) {
//			ReforgerEventsApiCallbacks.Init();
//		}
//		
//		auto callbacks = ReforgerEventsApiCallbacks.GetInstance();
//       	gameMode.GetOnGameStart().Insert(callbacks.OnGameStart);
//		gameMode.GetOnPlayerKilled().Insert(callbacks.OnPlayerKilled);
//		gameMode.GetOnControllableDestroyed().Insert(callbacks.OnControllableDestroyed);
//		gameMode.GetOnPlayerConnected().Insert(callbacks.OnPlayerConnected);
//    }
//}
//
//modded class SCR_CampaignNetworkComponent {
//	override void OnPlayerSuppliesInteraction(EResourcePlayerInteractionType interactionType, PlayerController playerController, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue) {
//		super.OnPlayerSuppliesInteraction(interactionType, playerController, resourceComponentFrom, resourceComponentTo, resourceType, resourceValue);
//		
//		SCR_ResourceContainer fromContainer = resourceComponentFrom.GetContainer(EResourceType.SUPPLIES);
//		SCR_ResourceContainer toContainer = resourceComponentTo.GetContainer(EResourceType.SUPPLIES);
//		
//		PrintFormat("%1", fromContainer);
//	}
//}

// TODO: Above is for server-side stats stuff

//modded class SCR_ResourceConsumer {
//	override SCR_ResourceConsumtionResponse RequestConsumtion(float resourceCost) {
//		SCR_ResourceConsumtionResponse response = super.RequestConsumtion(resourceCost);
//		if (!m_bIsConsuming || response.GetReason() != EResourceReason.SUFFICIENT) {
//			return response;
//		}
//		
//		// If there were sufficient resources, we need to send a notification that the resources were drained
//		GetGame().GetCallqueue().CallLater(NotifySuppliesConsumed, 0, false, response, m_Owner,  resourceCost);
//		return response;
//	}
//	
//	void NotifySuppliesConsumed(SCR_ResourceConsumtionResponse response, IEntity owner, float resourceCost) {
//		BaseGameMode gameMode = GetGame().GetGameMode();
//		SCR_BaseScoringSystemComponent scoringSystem = SCR_BaseScoringSystemComponent.Cast(gameMode.FindComponent(SCR_BaseScoringSystemComponent));
//		if (scoringSystem == null) {
//			return;
//		}
//		
//		PlayerController playerController = PlayerController.Cast(owner);
//		if (playerController != null) {
//			Print("Consumer is %1", playerController.GetPlayerId());
//		}
//	}
//}