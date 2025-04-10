//------------------------------------------------------------------------------------------------
modded class SCR_PlayerListMenu
{
	 //------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		// Skip the direct parent function, call its base
		super.OnMenuOpen();
		
		if (m_ScoringSystem) {
			m_ScoringSystem.GetOnPlayerAdded().Remove(OnPlayerAdded);
			m_ScoringSystem.GetOnPlayerRemoved().Remove(OnPlayerRemoved);
			m_ScoringSystem.GetOnPlayerScoreChanged().Remove(OnPlayerScoreChanged);
			m_ScoringSystem.GetOnFactionScoreChanged().Remove(OnFactionScoreChanged);
			
			m_aAllPlayersInfo.Clear();
			
			// Hide K/D/S sorting headers
			Widget w = GetRootWidget().FindAnyWidget("SortHeader");
			if (w) {
				ButtonWidget sortKills = ButtonWidget.Cast(w.FindAnyWidget("sortKills"));
				ButtonWidget sortDeaths = ButtonWidget.Cast(w.FindAnyWidget("sortDeaths"));
				ButtonWidget sortScore = ButtonWidget.Cast(w.FindAnyWidget("sortScore"));
		
				if (sortKills)
					sortKills.SetOpacity(0);
				if (sortDeaths)
					sortDeaths.SetOpacity(0);
				if (sortScore)
					sortScore.SetOpacity(0);
				
				m_ScoringSystem = null;
			}
			
			foreach (SCR_PlayerListEntry entry : m_aEntries)
			{
				entry.m_Info = null;
				if (entry.m_wKills)
					entry.m_wKills.SetText("");
				if (entry.m_wDeaths)
					entry.m_wDeaths.SetText("");
				if (entry.m_wScore)
					entry.m_wScore.SetText("");
			}
		}
	}
};