// Hijacks supply spend when in the building manager. For instance, placing buildings from building service
// (E_BuildingService_USSR.et) or vehicle service (E_VehicleService_USSR_Small.et)
//
// Currently, we do not consider placing compositions as a resource spend.
modded class SCR_CampaignBuildingManagerComponent {
	override void OnEntityCoreBudgetUpdated(EEditableEntityBudget entityBudget, int originalBudgetValue, int budgetChange, int updatedBudgetValue, SCR_EditableEntityComponent entity) {
		if (IsProxy())
			return;

		if (entityBudget != m_BudgetType)
			return;

		// Continue with compositions placed in WB only when refund is about to happen.
		if (entity.GetOwner().IsLoaded() && budgetChange > 0)
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (campaign && campaign.IsSessionLoadInProgress())
			return;

		int propBudgetValue;
		array<ref SCR_EntityBudgetValue> budgets = {};
		entity.GetEntityAndChildrenBudgetCost(budgets);

		//get props budget value
		foreach (SCR_EntityBudgetValue budget : budgets)
		{
			if (budget.GetBudgetType() != EEditableEntityBudget.PROPS)
				continue;

			propBudgetValue = budget.GetBudgetValue();
			break;
		}

		IEntity entityOwner = entity.GetOwnerScripted();
		SCR_ResourceComponent resourceComponent;
		bool wasContainerSpawned;

		// If resource component was not found on deconstruction, spawn a custom one , find again the resource component at this spawned box and fill it with refund supply.
		if (!GetResourceComponent(entityOwner, resourceComponent))
		{
			//Spawn a resource holder only when the refunded object is a composition.
			SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(entityOwner.FindComponent(SCR_CampaignBuildingCompositionComponent));
			if (compositionComponent && budgetChange < 0)
			{
				SpawnCustomResourceHolder(entityOwner, resourceComponent);
				wasContainerSpawned = true;
			}
		}

		if (!resourceComponent)
			return;

		//~ Supplies not enabled so no need to remove any
		if (!resourceComponent.IsResourceTypeEnabled())
			return;

		IEntity providerEntity = resourceComponent.GetOwner();

		if (!providerEntity)
			return;
		
		SCR_EditableEntityAuthor author = entity.GetAuthor();
		int playerId = -1;
		if (author != null) {
			playerId = author.m_iAuthorID;
		}
		
		// Check if the providerEntity has a catalog spawner. If it does not, we do not care about reporting these supplies.
		// It's likely the BuildingService entity at the command post.
		SCR_CatalogEntitySpawnerComponent catalogEntitySpawnerComponent = SCR_CatalogEntitySpawnerComponent.Cast(providerEntity.FindComponent(SCR_CatalogEntitySpawnerComponent));

		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(providerEntity.FindComponent(SCR_CampaignBuildingProviderComponent));

		// We need to send out a notification that the budget changed
		if (budgetChange < 0)
		{
			budgetChange = Math.Round(budgetChange * m_iCompositionRefundPercentage * 0.01);

			if (providerComponent)
				providerComponent.AddPropValue(-propBudgetValue);

			// Something that can hold supplies?
			if (wasContainerSpawned)
			{
				SCR_ResourceContainer container = resourceComponent.GetContainer(EResourceType.SUPPLIES);

				if (container)
					container.SetResourceValue(-budgetChange);
			}
			else
			{
				// We gained back supplies?
				SCR_ResourceGenerator generator = resourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

				if (generator) {
					if (catalogEntitySpawnerComponent) {
						// TODO: this will reflect upon whoever originally placed an item.
						// e.g. if I place an item and someone else disassembles it, I will have `-budgetChange` supplies
						// attributed to myself.
						LRS_NotifySuppliesConsumed(playerId, budgetChange);
					}
					generator.RequestGeneration(-budgetChange);
				}
			}
		}
		else
		{
			if (providerComponent)
				providerComponent.AddPropValue(propBudgetValue);

			SCR_ResourceConsumer consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

			// We burned supplies
			if (consumer) {
				if (catalogEntitySpawnerComponent) {
					LRS_NotifySuppliesConsumed(playerId, budgetChange);
				}
				consumer.RequestConsumtion(budgetChange);
			}
		}
	}
}