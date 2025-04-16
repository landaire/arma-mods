class LRDF_MapMarkerComponent : SCR_MapMarkerDynamicWComponent
{
	bool m_bIsHovered;
	protected bool m_bIsInit;
	protected Widget m_wOwnSquadBackground;
	protected Widget m_wOwnSquadIcon;
	protected Widget m_wOwnSquadIconGlow;
	protected Widget m_wGroupInfo;
	protected TextWidget m_wGroupName;
	
	[Attribute("{FA4A877B7048F1FA}UI/layouts/LRDF/LRDF_Marker.layout", desc: "radio transmission layout")]
	protected ResourceName m_sPlayerInfoLayout;
	
	[Attribute("40", desc: "pixels, group info offset")]
	protected int m_iGroupInfoOffset;
		
	//------------------------------------------------------------------------------------------------
	//! Differentiates visuals between our group and the others
	//! \param[in] state
	void SetGroupActive(bool state, string factionName = string.Empty)
	{
		Color iconColor = Color.FromInt(Color.BLACK);
		
		if (state)
		{
			/*m_wOwnSquadBackground.SetVisible(true);
			m_wOwnSquadIcon.SetVisible(true);
			m_wOwnSquadIconGlow.SetVisible(true);
			
			// TODO temp set until these are added to configs
			if (factionName == "US")
				iconColor = new Color(0.0, 0.18, 0.61, 1.0);
			else if (factionName == "USSR")
				iconColor = new Color(0.51, 0.02, 0.02, 1.0);*/
		}
		else 
		{
			m_wOwnSquadBackground.SetVisible(false);
			m_wOwnSquadIcon.SetVisible(false);
			m_wOwnSquadIconGlow.SetVisible(false);
		}
		
		//m_wOwnSquadIcon.SetColor(iconColor);
		//m_wOwnSquadIconGlow.SetColor(iconColor);
	}
			
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] screenX
	//! \param[in] screenY
	void UpdateGroupInfoPosition(int screenX, int screenY)
	{
		if (m_wGroupInfo)
			FrameSlot.SetPos(m_wGroupInfo, GetGame().GetWorkspace().DPIUnscale(screenX), GetGame().GetWorkspace().DPIUnscale(screenY) - m_iGroupInfoOffset);	// needs unscaled coords
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (button != 0)	// LMB only
			return true;

		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		SCR_MapCursorModule cursorModule;
		if (mapEntity)
			cursorModule = SCR_MapCursorModule.Cast(mapEntity.GetMapModule(SCR_MapCursorModule));

		if (cursorModule && cursorModule.GetCursorState() & SCR_MapCursorModule.STATE_POPUP_RESTRICTED)
			return true;

		GetGame().OpenGroupMenu();
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{		
		m_MarkerEnt.LayerChangeLogic(0);
		
		SCR_AIGroup group = LRDF_MapMarker.Cast(m_MarkerEnt).GetGroup();
		if (group)
		{
			if (!m_bIsInit)
			{
				m_wGroupInfo = GetGame().GetWorkspace().CreateWidgets(m_sPlayerInfoLayout, m_wRoot.GetParent());
				if (!m_wGroupInfo)
					return false;
				
				m_wGroupName = TextWidget.Cast(m_wGroupInfo.FindAnyWidget("GroupName"));
				
				string groupName = "";
				string customName = group.GetCustomName();
				if (customName == string.Empty) {
					string company, platoon, squad, character, format;
					group.GetCallsigns(company, platoon, squad, character, format);
						
					groupName = WidgetManager.Translate(format, company, platoon, squad, character);
				}
				m_wGroupName.SetText(groupName);
				
				m_bIsInit = true;
			}
			
			m_wGroupInfo.SetVisible(true);
		}
		
		m_bIsHovered = true;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		m_wGroupInfo.SetVisible(false);
		m_bIsHovered = false;
		
		m_MarkerEnt.LayerChangeLogic(m_iLayerID);
		
		return true;
	}
	
	void SetVisible(bool visible) {
		if (m_wGroupInfo) {
			m_wGroupInfo.SetVisible(false);
		}
		
		m_MarkerEnt.LayerChangeLogic(m_iLayerID);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wOwnSquadBackground = m_wRoot.FindAnyWidget("OwnSquadBackground");
		m_wOwnSquadIcon = m_wRoot.FindAnyWidget("OwnSquadIcon");
		m_wOwnSquadIconGlow = m_wRoot.FindAnyWidget("OwnSquadIconGlow");
	}
}
