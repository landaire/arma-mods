modded class SCR_RadioCoverageSystem {
	void GetRadioCoverageComponents(array<SCR_CoverageRadioComponent> aRadioComponents) {
		aRadioComponents.Copy(m_aRadioComponents);
	}
}

//class LRDF_RadioDirectionFindingSystem : GameSystem  {
//	override static void InitInfo(WorldSystemInfo outInfo)
//	{
//		outInfo.SetAbstract(false);
//	}
//}