//------------------------------------------------------------------------------------------------
modded class SCR_VehicleTagData
{
	//------------------------------------------------------------------------------------------------
	//! Update current tracked passangers within the vehicle
	override void UpdatePassenger(SCR_NameTagData tag, bool IsEntering, bool isControlledEntity = false)
	{
		super.UpdatePassenger(tag, IsEntering, isControlledEntity);
		if (m_Flags & ENameTagFlags.NAME_UPDATE) { 
			m_Flags |= ENameTagFlags.GROUP_UPDATE;
		}
	}
	
	override protected bool UpdateEntityStateFlags()
	{
		bool result = super.UpdateEntityStateFlags();
		m_Flags = ENameTagFlags.DISABLED | ENameTagFlags.NAME_UPDATE;	// this has a default flag because if tag never reaches a visibile state, it needs a disable flag for clean up
		
		if (m_Flags & ENameTagFlags.NAME_UPDATE) { 
			m_Flags |= ENameTagFlags.GROUP_UPDATE;
		}
		
		return result;
	}
	
	override protected void UpdateMainTag() {
		super.UpdateMainTag();
		if (m_MainTag.m_eEntityStateFlags & ENameTagEntityState.GROUP_LEADER)
			ActivateEntityState(ENameTagEntityState.GROUP_LEADER);
		else if (m_eEntityStateFlags & ENameTagEntityState.GROUP_LEADER)
			DeactivateEntityState(ENameTagEntityState.GROUP_LEADER);
	}
};
