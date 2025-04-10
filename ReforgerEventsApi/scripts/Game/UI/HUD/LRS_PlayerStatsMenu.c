modded enum ChimeraMenuPreset {
	LRS_StatsMenu,
}

class LRS_PlayerStatsListEntry
{
	SCR_ScoreInfo m_Info;
	int m_iID;
	Widget m_wRow;
	Faction m_Faction;

	TextWidget m_wName;
	TextWidget m_wKills;
	TextWidget m_wDeaths;
	TextWidget m_wScore;
	TextWidget m_wSuppliesDelivered;
	TextWidget m_wSuppliesConsumed;
	Widget m_wFactionImage;
};

class LRS_StatsMenu : SCR_SuperMenuBase
{
	protected ResourceName m_sScoreboardRow = "{27B252380A8479C6}UI/layouts/Menus/PlayerStatsList/LRS_PlayerStatsListEntry.layout";

	protected ref array<ref LRS_PlayerStatsListEntry> m_aEntries = new array<ref LRS_PlayerStatsListEntry>();
	protected ref map<int, SCR_ScoreInfo> m_aAllPlayersInfo = new map<int, SCR_ScoreInfo>();
	protected ref array<Faction> m_aFactions = {null};

	protected SCR_InputButtonComponent m_Mute;
	protected SCR_InputButtonComponent m_Block;
 	protected SCR_InputButtonComponent m_Unblock;
	protected SCR_InputButtonComponent m_Friend;
	protected SCR_InputButtonComponent m_Vote;
	protected SCR_InputButtonComponent m_Invite;
	protected SCR_InputButtonComponent m_ViewProfile;

	//protected SCR_RespawnSystemComponent m_RespawnSystem;
	protected SCR_BaseScoringSystemComponent m_ScoringSystem;
	protected LRS_PlayerStatsListEntry m_SelectedEntry;
	protected SCR_PlayerControllerGroupComponent m_PlayerGroupController;
	protected PlayerController m_PlayerController;
	protected SocialComponent m_SocialComponent;
	SCR_SortHeaderComponent m_Header;
	protected Widget m_wTable;
	protected bool m_bFiltering;
	protected float m_fTimeSkip;

	protected const float TIME_STEP = 1.0;

	protected const string ADD_FRIEND = "#AR-PlayerList_AddFriend";
	protected const string REMOVE_FRIEND = "#AR-PlayerList_RemoveFriend";
	protected const string MUTE = "#AR-PlayerList_Mute";
	protected const string UNMUTE = "#AR-PlayerList_Unmute";
	protected const string BLOCK = "#AR-PlayerList_Block";
	protected const string UNBLOCK = "#AR-PlayerList_Unblock";
	protected const string INVITE_PLAYER_VOTE = "#AR-PlayerList_Invite";
	protected const string MUTE_TEXTURE = "sound-off";
	protected const string OPTIONS_COMBO_ACCEPT = "#AR-Group_AcceptJoinPrivateGroup";
	protected const string OPTIONS_COMBO_CANCEL = "#AR-Group_RefuseJoinPrivateGroup";
	protected const string VOTING_PLAYER_COUNT_FORMAT = "#AR-Voting_PlayerCountFormatting";

	protected const string FILTER_FAV = "Favourite";
	protected const string FILTER_NAME = "Name";
	protected const string FILTER_FREQ = "Freq";
	protected const string FILTER_KILL = "Kills";
	protected const string FILTER_DEATH = "Deaths";
	protected const string FILTER_SCORE = "Score";
	protected const string FILTER_MUTE = "Mute";
	protected const string FILTER_BLOCK = "Block";

	protected static const ResourceName FACTION_COUNTER_LAYOUT = "{5AD2CE85825EDA11}UI/layouts/Menus/PlayerList/FactionPlayerCounter.layout";
	
	protected const int DEFAULT_SORT_INDEX = 1;

	protected string m_sGameMasterIndicatorName = "GameMasterIndicator";

	protected static ref ScriptInvoker s_OnPlayerListMenu = new ScriptInvoker();

	protected ref Color m_PlayerNameSelfColor = new Color(0.898, 0.541, 0.184, 1);
		
	protected static ref PlayerlistBlockCallback m_BlockCallback;

	/*!
	Get event called when player list opens or closes.
	\return Script invoker
	*/
	static ScriptInvoker GetOnPlayerListMenu()
	{
		return s_OnPlayerListMenu;
	}

	//------------------------------------------------------------------------------------------------
	protected void InitSorting()
	{
		if (!GetRootWidget())
			return;

		Widget w = GetRootWidget().FindAnyWidget("SortHeader");
		if (!w)
			return;

		m_Header = SCR_SortHeaderComponent.Cast(w.FindHandler(SCR_SortHeaderComponent));
		if (!m_Header)
			return;

		m_Header.m_OnChanged.Insert(OnHeaderChanged);

		if (m_ScoringSystem)
			return;

		// Hide K/D/S sorting headers if the re is no scoreboard
		ButtonWidget sortKills = ButtonWidget.Cast(w.FindAnyWidget("sortKills"));
		ButtonWidget sortDeaths = ButtonWidget.Cast(w.FindAnyWidget("sortDeaths"));
		ButtonWidget sortScore = ButtonWidget.Cast(w.FindAnyWidget("sortScore"));
		ButtonWidget sortSuppliesDelivered = ButtonWidget.Cast(w.FindAnyWidget("sortSuppliesDelivered"));
		sortSuppliesDelivered.FindWidget("Image").SetColor(GUIColors.GREEN);

		

		//if (sortKills)
		//	sortKills.SetOpacity(0);
		//if (sortDeaths)
		//	sortDeaths.SetOpacity(0);
		//if (sortScore)
		//	sortScore.SetOpacity(0);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHeaderChanged(SCR_SortHeaderComponent sortHeader)
	{
		string filterName = sortHeader.GetSortElementName();
		bool sortUp = sortHeader.GetSortOrderAscending();
		Sort(filterName, sortUp);
	}


	//------------------------------------------------------------------------------------------------
	protected void Sort(string filterName, bool sortUp)
	{
		if (filterName == FILTER_NAME)
			SortByName(sortUp);
		else if (filterName == FILTER_KILL)
			SortByKills(sortUp);
		else if (filterName == FILTER_DEATH)
			SortByDeaths(sortUp);
		else if (filterName == FILTER_SCORE)
			SortByScore(sortUp);
	}

	//------------------------------------------------------------------------------------------------
	void SortByName(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;

		array<string> names = {};
		foreach (LRS_PlayerStatsListEntry entry : m_aEntries)
		{
			if (entry.m_wName)
				names.Insert(entry.m_wName.GetText());
		}

		names.Sort();

		foreach (LRS_PlayerStatsListEntry entry : m_aEntries)
		{
			if (!entry.m_wName)
				continue;

			string text = entry.m_wName.GetText();

			foreach (int i, string s : names)
			{
				if (s != text)
					continue;

				if (entry.m_wRow)
					entry.m_wRow.SetZOrder(i * direction);
				continue;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void SortByKills(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;

		foreach (LRS_PlayerStatsListEntry entry : m_aEntries)
		{
			SCR_ScoreInfo score = entry.m_Info;
			if (score)
				entry.m_wRow.SetZOrder(score.m_iKills * direction);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SortByDeaths(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;

		foreach (LRS_PlayerStatsListEntry entry : m_aEntries)
		{
			SCR_ScoreInfo score = entry.m_Info;
			if (score)
				entry.m_wRow.SetZOrder(score.m_iDeaths * direction);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SortByScore(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;

		foreach (LRS_PlayerStatsListEntry entry : m_aEntries)
		{
			SCR_ScoreInfo info = entry.m_Info;
			if (info)
			{
				int score;
				if (m_ScoringSystem)
					score = m_ScoringSystem.GetPlayerScore(entry.m_iID);
				else
					score = 0;

				entry.m_wRow.SetZOrder(score * direction);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnTabChanged(SCR_TabViewComponent comp, Widget w, int selectedTab)
	{
		if (selectedTab < 0)
			return;

		Faction faction = null;
		foreach (Faction playableFaction : m_aFactions)
		{
			if (comp.GetShownTabComponent().m_sTabButtonContent == playableFaction.GetFactionName())
				faction = playableFaction;
		}

		int lowestZOrder = int.MAX;
		foreach (LRS_PlayerStatsListEntry entry : m_aEntries)
		{
			if (!entry.m_wRow)
				continue;

			//if the tab is the first one, it's the All tab for now
			if (comp.GetShownTab() == 0)
				entry.m_wRow.SetVisible(true);
			else if (faction == entry.m_Faction)
				entry.m_wRow.SetVisible(true);
			else
				entry.m_wRow.SetVisible(false);
		}

		if (m_Header)
			m_Header.SetCurrentSortElement(DEFAULT_SORT_INDEX, ESortOrder.ASCENDING, useDefaultSortOrder: true);
	}

	//------------------------------------------------------------------------------------------------
	void OnBack()
	{
		if (m_bFiltering)
			OnFilter();
		else
			Close();
	}

	//------------------------------------------------------------------------------------------------
	void OnFilter()
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnEntryFocused(Widget w)
	{		
		if (!m_PlayerController)
			return;
		
		foreach (LRS_PlayerStatsListEntry entry : m_aEntries)
		{
			if (!entry)
				continue;

			Widget row = entry.m_wRow;
			if (row != w)
				continue;

			m_SelectedEntry = entry;
			break;
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnEntryFocusLost(Widget w)
	
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void FocusFirstItem()
	{
		Widget firstEntry;
		int lowestZOrder = int.MAX;
		foreach (LRS_PlayerStatsListEntry entry : m_aEntries)
		{
			if (!entry.m_wRow.IsVisible())
				continue;

			int z = entry.m_wRow.GetZOrder();
			if (z < lowestZOrder)
			{
				lowestZOrder = z;
				firstEntry = entry.m_wRow;
			}
		}

		if (firstEntry)
			GetGame().GetWorkspace().SetFocusedWidget(firstEntry);
	}

	//------------------------------------------------------------------------------------------------
	void SetEntryBackgrounColor(Color color)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		SCR_AIGroup group = groupManager.GetPlayerGroup(m_PlayerController.GetPlayerId());
		if (!group)
			return;

		array<int> requesters = {};
		array<string> reqNames = {};

		group.GetRequesterIDs(requesters);

		foreach (int req : requesters)
		{
			reqNames.Insert(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(req));
		}

		if (!requesters.Contains(m_SelectedEntry.m_iID))
			return;
		
		Widget w = GetGame().GetWorkspace().FindAnyWidget("Button");

		SCR_ButtonBaseComponent button = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));
		if (!button)
			return;

		ImageWidget background = ImageWidget.Cast(m_SelectedEntry.m_wRow.FindAnyWidget("Background"));

		background.SetColor(color);
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateEntry(int id, SCR_PlayerDelegateEditorComponent editorDelegateManager)
	{
		//check for existing entry, return if it exists already
		foreach (LRS_PlayerStatsListEntry entry : m_aEntries)
		{
			if (entry.m_iID == id)
				return;
		}

		ImageWidget badgeTop, badgeMiddle, badgeBottom;

		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sScoreboardRow, m_wTable);
		if (!w)
			return;

		LRS_PlayerStatsListEntry entry = new LRS_PlayerStatsListEntry();
		entry.m_iID = id;
		entry.m_wRow = w;

		SCR_ButtonBaseComponent handler = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));
		if (handler)
		{
			handler.m_OnFocus.Insert(OnEntryFocused);
			handler.m_OnFocusLost.Insert(OnEntryFocusLost);
		}

		if (m_aAllPlayersInfo)
		{
			foreach (int playerId, SCR_ScoreInfo info : m_aAllPlayersInfo)
			{
				if (!info || playerId != id)
					continue;

				entry.m_Info = info;
				break;
			}
		}

		// Find faction
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
		{
			Faction faction = factionManager.GetPlayerFaction(entry.m_iID);
			entry.m_Faction = faction;
		}

		Widget factionImage = w.FindAnyWidget("FactionImage");

		if (factionImage)
		{
			if (entry.m_Faction)
				factionImage.SetColor(entry.m_Faction.GetFactionColor());
			else
				factionImage.SetVisible(false);
		}

		entry.m_wName = TextWidget.Cast(w.FindAnyWidget("PlayerName"));

		if (entry.m_wName)
		{
			entry.m_wName.SetText(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(id));
			if (entry.m_iID == m_PlayerController.GetPlayerId())
				entry.m_wName.SetColor(m_PlayerNameSelfColor);
		}

		entry.m_wKills = TextWidget.Cast(w.FindAnyWidget("Kills"));
		entry.m_wDeaths = TextWidget.Cast(w.FindAnyWidget("Deaths"));
		entry.m_wScore = TextWidget.Cast(w.FindAnyWidget("Score"));
		entry.m_wSuppliesDelivered = TextWidget.Cast(w.FindAnyWidget("SuppliesDelivered"));
		entry.m_wSuppliesConsumed = TextWidget.Cast(w.FindAnyWidget("SuppliesSpent"));
		if (entry.m_Info)
		{
			if (entry.m_wKills)
				entry.m_wKills.SetText(entry.m_Info.m_iKills.ToString());
			if (entry.m_wDeaths)
				entry.m_wDeaths.SetText(entry.m_Info.m_iDeaths.ToString());
			if (entry.m_wSuppliesDelivered)
				entry.m_wSuppliesDelivered.SetText(entry.m_Info.m_iSuppliesDelivered.ToString());
			if (entry.m_wSuppliesConsumed)
				entry.m_wSuppliesConsumed.SetText(entry.m_Info.m_iSuppliesConsumed.ToString());
			if (entry.m_wScore)
			{
				// Use modifiers from scoring system where applicable!!!
				int score;
				if (m_ScoringSystem)
					score = m_ScoringSystem.GetPlayerScore(id);

				entry.m_wScore.SetText(score.ToString());
			}
		}
		else
		{

			if (entry.m_wKills)
				entry.m_wKills.SetText("");
			if (entry.m_wDeaths)
				entry.m_wDeaths.SetText("");
			if (entry.m_wScore)
				entry.m_wScore.SetText("");
			if (entry.m_wSuppliesDelivered)
				entry.m_wSuppliesDelivered.SetText("");
			if (entry.m_wSuppliesConsumed)
				entry.m_wSuppliesConsumed.SetText("");
			// Unfortunately the parent that must be hidden is two parents above the text widgets
			/*
			if (entry.m_wKills)
				entry.m_wKills.GetParent().GetParent().SetVisible(false);
			if (entry.m_wDeaths)
				entry.m_wDeaths.GetParent().GetParent().SetVisible(false);
			if (entry.m_wScore)
				entry.m_wScore.GetParent().GetParent().SetVisible(false);
			*/
		}

		ImageWidget background = ImageWidget.Cast(w.FindAnyWidget("Background"));
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_AIGroup group = groupManager.GetPlayerGroup(m_PlayerController.GetPlayerId());


		Faction playerFaction;		
		Faction entryPlayerFaction;
		if (factionManager)
		{
			playerFaction = factionManager.GetPlayerFaction(m_PlayerController.GetPlayerId());	
			entryPlayerFaction = factionManager.GetPlayerFaction(entry.m_iID);
		}
		 
		
		SCR_BasePlayerLoadout playerLoadout;
		SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
		if (loadoutManager)
			playerLoadout = loadoutManager.GetPlayerLoadout(entry.m_iID);		 

		badgeTop = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeTop"));
		badgeMiddle = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeMiddle"));
		badgeBottom = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeBottom"));
		Color factionColor;

		if (badgeTop && badgeMiddle && badgeBottom && entry.m_Faction)
		{
			factionColor = entry.m_Faction.GetFactionColor();
			badgeTop.SetColor(factionColor);
			badgeMiddle.SetColor(factionColor);
			badgeBottom.SetColor(factionColor);
		}

		m_aEntries.Insert(entry);
	}

	//------------------------------------------------------------------------------------------------
	// IsLocalPlayer would be better naming
	protected bool IsLocalPlayer(int id)
	{
		if (id <= 0)
			return false;

		return SCR_PlayerController.GetLocalPlayerId() == id;
	}

	//------------------------------------------------------------------------------------------------
	void RemoveEntry(notnull LRS_PlayerStatsListEntry entry)
	{
		if (entry.m_wRow)
			entry.m_wRow.RemoveFromHierarchy();

		m_aEntries.RemoveItem(entry);
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenPauseMenu()
	{
		GetGame().OpenPauseMenu();
		Close();
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnPlayerAdded(int playerId)
	{
		UpdatePlayerList(true, playerId);
	}
	private void OnPlayerRemoved(int playerId)
	{
		UpdatePlayerList(false, playerId);
	}
	private void OnScoreChanged()
	{
		UpdateScore();
	}
	private void OnPlayerScoreChanged(int playerId, SCR_ScoreInfo scoreInfo)
	{
		OnScoreChanged();
	}
	private void OnFactionScoreChanged(Faction faction, SCR_ScoreInfo scoreInfo)
	{
		OnScoreChanged();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		//if (!m_ChatPanel)
		//	m_ChatPanel = SCR_ChatPanel.Cast(m_wRoot.FindAnyWidget("ChatPanel").FindHandler(SCR_ChatPanel));		
		
		m_PlayerController = GetGame().GetPlayerController();
		if (m_PlayerController)
		{
			m_SocialComponent = SocialComponent.Cast(m_PlayerController.FindComponent(SocialComponent));
			
			SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.Cast(m_PlayerController.FindComponent(SCR_HUDManagerComponent));
			hudManager.SetVisibleLayers(hudManager.GetVisibleLayers() & ~EHudLayers.HIGH);
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		m_PlayerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();

		//gameMode.GetOnPlayerKilled().Insert(OnPlayerKilled);
		gameMode.GetOnPlayerRegistered().Insert(OnPlayerConnected);

		m_ScoringSystem = gameMode.GetScoringSystemComponent();
		if (m_ScoringSystem)
		{
			m_ScoringSystem.GetOnPlayerAdded().Insert(OnPlayerAdded);
			m_ScoringSystem.GetOnPlayerRemoved().Insert(OnPlayerRemoved);
			m_ScoringSystem.GetOnPlayerScoreChanged().Insert(OnPlayerScoreChanged);
			m_ScoringSystem.GetOnFactionScoreChanged().Insert(OnFactionScoreChanged);

			array<int> players = {};
			PlayerManager playerManager = GetGame().GetPlayerManager();
			playerManager.GetPlayers(players);

			m_aAllPlayersInfo.Clear();
			foreach (int playerId : players)
				m_aAllPlayersInfo.Insert(playerId, m_ScoringSystem.GetPlayerScoreInfo(playerId));
		}

		FactionManager fm = GetGame().GetFactionManager();
		if (fm)
		{
			fm.GetFactionsList(m_aFactions);
		}

		m_wTable = GetRootWidget().FindAnyWidget("Table");

		// Create navigation buttons
		Widget footer = GetRootWidget().FindAnyWidget("FooterLeft");
		Widget footerBack = GetRootWidget().FindAnyWidget("Footer");
		SCR_InputButtonComponent back = SCR_InputButtonComponent.GetInputButtonComponent(UIConstants.BUTTON_BACK, footerBack);
		if (back)
			back.m_OnActivated.Insert(OnBack);

		SCR_InputButtonComponent filter = SCR_InputButtonComponent.GetInputButtonComponent("Filter", footer);
		if (filter)
			filter.m_OnActivated.Insert(OnFilter);

		// Create table
		if (!m_wTable || m_sScoreboardRow == string.Empty)
			return;

		//Get editor Delegate manager to check if has editor rights
		SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));


		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);

		foreach (int id : ids)
		{
			CreateEntry(id, editorDelegateManager);
		}

		InitSorting();
		
		m_SuperMenuComponent.GetTabView().GetOnChanged().Insert(OnTabChanged);

		// Create new tabs
		SCR_Faction scrFaction;
		foreach (Faction faction : m_aFactions)
		{
			if (!faction)
				continue;

			scrFaction = SCR_Faction.Cast(faction);
			if (scrFaction && !scrFaction.IsPlayable())
				continue; //--- ToDo: Refresh dynamically when a new faction is added/removed

			string name = faction.GetFactionName();
			m_SuperMenuComponent.GetTabView().AddTab(ResourceName.Empty,name);
			
			AddFactionPlayerCounter(faction);
		}

		//handle groups tab
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		Faction playerFaction;
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			playerFaction = factionManager.GetLocalPlayerFaction();
		
		
		if (!groupsManager || !playerFaction)
			m_SuperMenuComponent.GetTabView().SetTabVisible(EPlayerListTab.GROUPS, false);

		s_OnPlayerListMenu.Invoke(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		m_fTimeSkip = m_fTimeSkip + tDelta;

		if (m_fTimeSkip >= TIME_STEP)
		{
			DeleteDisconnectedEntries();
			m_fTimeSkip = 0.0;
		}

		GetGame().GetInputManager().ActivateContext("PlayerMenuContext");
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		GetGame().GetInputManager().AddActionListener("ShowScoreboard",	EActionTrigger.DOWN, Close);

		if (m_Header)
			m_Header.SetCurrentSortElement(DEFAULT_SORT_INDEX, ESortOrder.ASCENDING, useDefaultSortOrder: true);

		FocusFirstItem();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		GetGame().GetInputManager().RemoveActionListener("ShowScoreboard",	EActionTrigger.DOWN, Close);

		//--- Close when some other menu is opened on top
		Close();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();

		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.Cast(m_PlayerController.FindComponent(SCR_HUDManagerComponent));
		if (hudManager)
			hudManager.SetVisibleLayers(hudManager.GetVisibleLayers() | EHudLayers.HIGH);

		m_aAllPlayersInfo.Clear();
		m_aFactions.Clear();

		if (m_ScoringSystem)
		{
			m_ScoringSystem.GetOnPlayerAdded().Remove(OnPlayerAdded);
			m_ScoringSystem.GetOnPlayerRemoved().Remove(OnPlayerRemoved);
			m_ScoringSystem.GetOnPlayerScoreChanged().Remove(OnPlayerScoreChanged);
			m_ScoringSystem.GetOnFactionScoreChanged().Remove(OnFactionScoreChanged);
		}

		s_OnPlayerListMenu.Invoke(false);
	}

	// Call updates with delay, so name is synced properly
	//------------------------------------------------------------------------------------------------
	void OnPlayerConnected(int id)
	{
		GetGame().GetCallqueue().CallLater(UpdatePlayerList, 1000, false, true, id);
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePlayerList(bool addPlayer, int id)
	{
		if (addPlayer)
		{
			SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));
			CreateEntry(id, editorDelegateManager);

			// Get current sort method and re-apply sorting
			if (!m_Header)
				return;

			OnHeaderChanged(m_Header);
		}
		else
		{
			// Delete entry from the list
			LRS_PlayerStatsListEntry playerEntry;
			foreach (LRS_PlayerStatsListEntry entry : m_aEntries)
			{
				if (entry.m_iID != id)
					continue;

				playerEntry = entry;
				break;
			}

			if (!playerEntry)
				return;

			playerEntry.m_wRow.RemoveFromHierarchy();
			m_aEntries.RemoveItem(playerEntry);
		}
	}

	//------------------------------------------------------------------------------------------------
	void UpdateScore()
	{
		if (!m_aAllPlayersInfo || m_aAllPlayersInfo.Count() == 0 || !m_ScoringSystem)
			return;

		foreach (LRS_PlayerStatsListEntry entry : m_aEntries)
		{
			foreach (int playerId, SCR_ScoreInfo info : m_aAllPlayersInfo)
			{
				if (!info || playerId != entry.m_iID)
					continue;

				entry.m_Info = info;
				break;
			}

			if (!entry.m_Info)
				continue;

			if (entry.m_wKills)
				entry.m_wKills.SetText(entry.m_Info.m_iKills.ToString());
			if (entry.m_wDeaths)
				entry.m_wDeaths.SetText(entry.m_Info.m_iDeaths.ToString());
			if (entry.m_wSuppliesDelivered)
				entry.m_wSuppliesDelivered.SetText(entry.m_Info.m_iSuppliesDelivered.ToString());
			if (entry.m_wSuppliesConsumed)
				entry.m_wSuppliesConsumed.SetText(entry.m_Info.m_iSuppliesConsumed.ToString());
			if (entry.m_wScore)
			{
				// Use modifiers from scoring system where applicable!!!
				int score;
				if (m_ScoringSystem)
					score = m_ScoringSystem.GetPlayerScore(entry.m_iID);

				entry.m_wScore.SetText(score.ToString());
			}
		}

		// Get current sort method and re-apply sorting
		if (!m_Header)
			return;

		OnHeaderChanged(m_Header);
	}

	//------------------------------------------------------------------------------------------------
	void DeleteDisconnectedEntries()
	{
		for (int i = m_aEntries.Count() - 1; i >= 0; i--)
		{
			if (!m_aEntries[i])
				continue;
			
			if (!m_aEntries[i] || GetGame().GetPlayerManager().IsPlayerConnected(m_aEntries[i].m_iID))
				continue;
			
			m_aEntries[i].m_wRow.RemoveFromHierarchy();			
			m_aEntries.Remove(i);
		}
	}
	
	void OnSuppliesDelivered() {
		
	}
	
	//------------------------------------------------------------------------------------------------
	void AddFactionPlayerCounter(Faction faction)
	{
		SCR_Faction scriptedFaction = SCR_Faction.Cast(faction);
		if (!scriptedFaction)
			return;
		
		Widget contentLayout = GetRootWidget().FindAnyWidget("FactionPlayerNumbersLayout");
		if (!contentLayout)
			return;
		
		Widget factionTile = GetGame().GetWorkspace().CreateWidgets(FACTION_COUNTER_LAYOUT, contentLayout);
		if (!factionTile)
			return;
		
		RichTextWidget playerCount = RichTextWidget.Cast(factionTile.FindAnyWidget("PlayerCount"));
		if (!playerCount)
			return;
		
		ImageWidget factionFlag = ImageWidget.Cast(factionTile.FindAnyWidget("FactionFlag"));
		if (!factionFlag)
			return;

		int x, y;
		factionFlag.LoadImageTexture(0, scriptedFaction.GetFactionFlag());	
		factionFlag.GetImageSize(0, x, y);
		factionFlag.SetSize(x, y);
		
		playerCount.SetText(scriptedFaction.GetPlayerCount().ToString());
	}
};