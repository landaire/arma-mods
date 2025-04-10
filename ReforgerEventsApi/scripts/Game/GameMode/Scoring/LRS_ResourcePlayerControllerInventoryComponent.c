// Hijacks resource spend relating to inventory actions. For example, dragging supplies between a supply container
// and vehicle, or purchasing things from the armory.
//
// TODO: likely need to handle depositing items at the armory?
modded class SCR_ResourcePlayerControllerInventoryComponent {
	override bool TryPerformResourceConsumption(notnull SCR_ResourceActor actor, float resourceValue, bool ignoreOnEmptyBehavior = false) {
		if (!super.TryPerformResourceConsumption(actor, resourceValue, ignoreOnEmptyBehavior)) {
			return false;
		}
		
		SCR_ResourceComponent component = actor.GetComponent();
		IEntity providerEntity = component.GetOwner();
		SCR_EditableEntityComponent m_EditableEntity = null;
		SCR_EditableEntityComponent editableParent = null;
		FactionAffiliationComponent resourceFaction = null;
		IEntity rootParent = null;


		if (providerEntity != null) {
			rootParent = providerEntity.GetRootParent();
			m_EditableEntity = SCR_EditableEntityComponent.Cast(rootParent.FindComponent(SCR_EditableEntityComponent));
		}
		if (m_EditableEntity) {
			editableParent =  m_EditableEntity.GetParentEntity();
			if (editableParent) {
				resourceFaction = FactionAffiliationComponent.Cast(editableParent.GetOwner().FindComponent(FactionAffiliationComponent));
			} else {
				// Ignore transfers from vehicles
				Vehicle vehicle = Vehicle.Cast(rootParent);
				if (vehicle == null) {
					resourceFaction = FactionAffiliationComponent.Cast(rootParent.FindComponent(FactionAffiliationComponent))
				}
			}
		}
		
		SCR_PlayerController player = SCR_PlayerController.Cast(GetOwner());
		if (player != null && resourceFaction != null) {
			// Check if this faction matches the faction of the container we're taking resources from. 
			if (player.GetLocalControlledEntityFaction().IsFactionFriendly(resourceFaction.GetAffiliatedFaction())) {
				// Check flags indicating if we should notify in this case.
				//
				// If this is an editable parent, we're probably taking supplies from the supply storage
				// and loading it up into something.
				if (editableParent == null) {
					LRS_NotifySuppliesConsumed(player.GetPlayerId(), resourceValue);
				}
			}
		}
		
		return true;
	}
}