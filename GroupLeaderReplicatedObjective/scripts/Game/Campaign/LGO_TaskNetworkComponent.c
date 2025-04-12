//! Takes care of tasks-specific server <> client communication and requests
modded class SCR_TaskNetworkComponent
{
	//------------------------------------------------------------------------------------------------
	//! Allows the local player to request assignment to a task.
	//! \param[in] taskID
	override void RequestAssignment(int taskID)
	{
		super.RequestAssignment(taskID);
		if (m_RplComponent && m_RplComponent.Role() == RplRole.Authority)
		{
			// For each member of this group, also request assignment
			SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
			if (!groupManager) {
				return;
			}
			
			int thisPlayerID = m_PlayerController.GetLocalPlayerId();
			
			SCR_AIGroup playerGroup = groupManager.GetPlayerGroup(thisPlayerID);
			if (!playerGroup || playerGroup.GetLeaderID() != thisPlayerID) {
				return; 
			}
			
			array<int> groupMembers = playerGroup.GetPlayerIDs();
			for (int i = 0; i < groupMembers.Count(); i++) {
				int playerId = groupMembers[i];
				if (playerId == thisPlayerID) {
					continue;
				}
				SCR_PlayerController groupMemberPlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
				
				if (!groupMemberPlayerController) {
					continue;
				}
				
				SCR_TaskNetworkComponent groupMemberTaskComponent = SCR_TaskNetworkComponent.Cast(groupMemberPlayerController.FindComponent(SCR_TaskNetworkComponent));
				if (!groupMemberTaskComponent) {
					continue;
				}
				
				groupMemberTaskComponent.RPC_RequestAssignment(taskID, playerId);
			}
		}
	}
}
