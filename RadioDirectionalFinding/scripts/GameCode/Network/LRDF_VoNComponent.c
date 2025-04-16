//------------------------------------------------------------------------------------------------
class LRDF_RadioTransmission {
	int m_iPlayerId;
	int m_iFrequency;
	float m_fRange;
	string m_sEncryptionKey;
	
	static bool Extract(LRDF_RadioTransmission instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		// Fill a snapshot with values from an instance.
		snapshot.SerializeInt(instance.m_iPlayerId);
		snapshot.SerializeInt(instance.m_iFrequency);
		snapshot.SerializeFloat(instance.m_fRange);
		snapshot.SerializeString(instance.m_sEncryptionKey);
		return true;
	}

	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, LRDF_RadioTransmission instance)
	{
		// Fill an instance with values from snapshot.
		snapshot.SerializeInt(instance.m_iPlayerId);
		snapshot.SerializeInt(instance.m_iFrequency);
		snapshot.SerializeFloat(instance.m_fRange);
		snapshot.SerializeString(instance.m_sEncryptionKey);
		return true;
	}

	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		// Read values from snapshot, encode them into smaller representation, then
		// write them into packet.
		snapshot.EncodeInt(packet);		// m_iPlayerId
		snapshot.EncodeInt(packet); 	// m_iFrequency
		snapshot.EncodeFloat(packet); 	// m_fRange
		snapshot.EncodeString(packet); 	// m_sEncryptionKey
	}

	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		// Read values from packet, decode them into their original representation,
		// then write them into snapshot.
		snapshot.DecodeInt(packet);		// m_iPlayerId
		snapshot.DecodeInt(packet); 	// m_iFrequency
		snapshot.DecodeFloat(packet); 	// m_fRange
		snapshot.DecodeString(packet); 	// m_sEncryptionKey
		return true;
	}

	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		// Compare two snapshots and determine whether they are the same.
		return lhs.CompareSnapshots(rhs, 4 + 4 + 4) && lhs.CompareStringSnapshots(rhs);
	}

	static bool PropCompare(LRDF_RadioTransmission instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		// Determine whether current values in instance are sufficiently different from
		// an existing snapshot that it's worth creating new one.
		// For float or vector values, it is possible to use some threshold to avoid creating too
		// many snapshots due to tiny changes in these values.
		return snapshot.CompareInt(instance.m_iPlayerId)
			&& snapshot.CompareInt(instance.m_iFrequency)
			&& snapshot.CompareFloat(instance.m_fRange)
			&& snapshot.CompareString(instance.m_sEncryptionKey);
	}
}


modded class SCR_VoNComponent
{
	// Ping the server every 5 seconds during transmission
	const float TRANSMISSION_NOTIFICATION_INTERVAL = 5000;
	
	private float m_fNextMinTransmissionPing;
	
	private bool drawShape = false;
	protected ref array<ref Shape> m_aDebugShapes = {};
	protected ref array<ref SCR_MapMarkerBase> m_aMapMarkers = {};
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnCapture(BaseTransceiver transmitter)
	{
		super.OnCapture(transmitter);
		
		if (transmitter && GetGame().GetWorld().GetWorldTime() > m_fNextMinTransmissionPing) {
			LRDF_RadioTransmission transmission = new LRDF_RadioTransmission();
			transmission.m_iPlayerId = GetGame().GetPlayerController().GetPlayerId();
			transmission.m_iFrequency = transmitter.GetFrequency();
			transmission.m_fRange = transmitter.GetRange();
			transmission.m_sEncryptionKey = transmitter.GetRadio().GetEncryptionKey();
			
			
			RplComponent rpl = RplComponent.Cast(GetGame().GetPlayerController().FindComponent(RplComponent));
			if (rpl.IsProxy()) {
				Rpc(OnCaptureDetected, transmission);
			} else {
				OnCaptureDetected(transmission);
			}
			
			m_fNextMinTransmissionPing = GetGame().GetWorld().GetWorldTime() + TRANSMISSION_NOTIFICATION_INTERVAL;
		}
	}
	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void OnCaptureDetected(LRDF_RadioTransmission transmission) {
		PrintFormat("Voice capture detected on transmitter %1!", transmission);
		
		
		SCR_MapMarkerManagerComponent m_MarkerMgr = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		if (m_MarkerMgr) {
			float wX, wY;
			IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(transmission.m_iPlayerId);
			if (!player) {
				return;			
			}
			
			vector playerPosition = player.GetOrigin();
			
			wX = playerPosition[0];
			wY = playerPosition[2];
						
			SCR_MapMarkerBase marker = new SCR_MapMarkerBase();
			
			{
				marker.SetType(SCR_EMapMarkerType.PLACED_CUSTOM);
			}
			
			marker.SetCustomText("RADIO POSITION");
			marker.SetWorldPos(wX, wY);
			
				FactionManager factionManager = GetGame().GetFactionManager();
				if (factionManager)
				{
					Faction markerOwnerFaction = SCR_FactionManager.SGetPlayerFaction(GetGame().GetPlayerController().GetPlayerId());
					if (markerOwnerFaction)
						marker.AddMarkerFactionFlags(factionManager.GetFactionIndex(markerOwnerFaction));
				}
			
			m_MarkerMgr.InsertStaticMarker(marker, false);
			
			
			// Find the nearest radio tranceiver
			array<SCR_CoverageRadioComponent> radios = {};
			SCR_RadioCoverageSystem.GetInstance().GetRadioCoverageComponents(radios);
			
			
			SCR_SortedArray<SCR_CoverageRadioComponent> radiosInRange = new SCR_SortedArray<SCR_CoverageRadioComponent>();
			
			// Check if any of these are within our radio's range
			for (int i = 0; i < radios.Count(); i++) {
				SCR_CoverageRadioComponent radio = radios[i];
				vector position = radio.GetOwner().GetOrigin();
				float radioX, radioY;
				radioX = position[0];
				radioY = position[2];
				
				// Calculate the distance between the player and the radio relay
				float distance = vector.DistanceSqXZ(playerPosition, position);
				if (distance < Math.Pow(transmission.m_fRange, 2)) {
					radiosInRange.Insert(distance, radio);				
				}
			}
			
			// We need at least 3 radios to triangulate
			//if (radiosInRange.Count() < 3) {
			//	return;
			//}
			
			PrintFormat("Found %1 radios in range", radiosInRange.Count());
			for (int i = 0; i < radiosInRange.Count(); i++)  {
				SCR_CoverageRadioComponent radio = radiosInRange.Get(i);

				vector directionVector = vector.Direction(player.GetOrigin(), radio.GetOwner().GetOrigin());
				PrintFormat("Raw direction vector: %1", directionVector);
				float length = directionVector.NormalizeSize();
				
				PrintFormat("Radio %1 direction: %2, length %3", i, directionVector, length);
			}
			
			
			m_aDebugShapes.Clear();
			
			array<ref array<vector>> lines = {};
			for (int i = 0; i < Math.Min(radiosInRange.Count(), 3); i++) {
				// Grab the radio relay's position
				vector source = radiosInRange[i].GetOwner().GetOrigin();
				// Share Z coordinate with the player
				source[1] = playerPosition[1];
				
				// Grab a line segment 15 degrees offset from the antenna position
				vector targetRight = GetRotatedPoint(source, playerPosition, 15, transmission.m_fRange * 2);
				targetRight[1] = playerPosition[1];
				
				// Grab a line segment -15 degrees offset
				vector targetLeft = GetRotatedPoint(source, playerPosition, -15, transmission.m_fRange * 2);
				targetLeft[1] = playerPosition[1];
				
				// Create the triangle to draw (only 2 lines)
				vector rdfTriangle[4] = {
					source,
					targetRight,
					source,
					targetLeft,
				};

				// Create a copy of the triangle for persisting
				array<vector> segment = {
					source,
					targetRight,
					source,
					targetLeft,
				};
				lines.Insert(segment);
				
				int color = Color.Blue.PackToInt();
				if (i == 1) {
					color = Color.Red.PackToInt();
				}
				if (i == 2) {
					color = Color.Green.PackToInt();
				}
				ShapeFlags shapeFlags = ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOOUTLINE;
		
				// Bounding Box markers
				m_aDebugShapes.Insert(Shape.CreateLines(color, shapeFlags, rdfTriangle, 4));
			}
			
			// Check if any of these lines intersect
			array<vector> intersections = {
				// Always put the closest radio tower's points here
				lines[0][0],
				lines[0][1],
				// Intentionally skip 2 -- these are just the key points that make
				// up the triangle
				lines[0][3],
			};
			
			// Last line doesn't need to be checked
			for (int i = 0; i < lines.Count() - 1; i++)  {
				array<vector> firstLine = lines[i];
				
				// Two segments per loop index
				for (int j = 0; j < firstLine.Count(); j += 2) {
					vector firstLineOrigin = firstLine[j];
					vector firstLineEnd = firstLine[j+ 1];
				
					// Iterate over lines ahead in the list
					for (int k = i + 1; k < lines.Count(); k++) {
						array<vector> secondLine = lines[k];

						// Two segments per loop index
						for (int l = 0; l < secondLine.Count(); l += 2) {
							vector intersection;
							
							vector secondLineOrigin = secondLine[l];
							vector secondLineEnd = secondLine[l + 1];
							
							PrintFormat("Does the game think they intersect? %1", Math3D.IntersectionLineSegments(firstLineOrigin, firstLineEnd, secondLineOrigin, secondLineEnd));
							if (GetLineSegmentIntersection(firstLineOrigin, firstLineEnd, secondLineOrigin, secondLineEnd, intersection)) {
								intersection[1] = secondLineOrigin[1];
								intersections.Insert(intersection);
							}
						}
					}
				}
			}
			
			set<int> toRemove = new set<int>();
			
			for (int i = 0; i < lines.Count(); i++) {
				vector a = lines[i][0];
				vector b = lines[i][1];
				vector c = lines[i][3];
				
				for (int j = 0; j < intersections.Count(); j++) {
					if (!PointInTriangle(intersections[j], a, b, c)) {
						PrintFormat("Triangle %1: Removing intersection at %2", i, j);
						toRemove.Insert(j);
					}
				}
			}
			
			// Filter out intersections that don't fall within the smallest sector
			array<vector> filtered = {};
			for (int i = 0; i < intersections.Count(); i++) {
				if (!toRemove.Contains(i)) {
					filtered.Insert(intersections[i]);
				}
			}
				
			// Draw yellow spheres for all general intersections	
			for (int i = 0; i < intersections.Count(); i++) {
				if (!toRemove.Contains(i)) {
					continue;
				}
				
				vector intersection = intersections[i];
				PrintFormat("Intersection detected at %1", intersection);
				ShapeFlags shapeFlags = ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOOUTLINE;
				m_aDebugShapes.Insert(Shape.CreateSphere(Color.Yellow.PackToInt(), shapeFlags, intersection, 10));		
			}
			
			// Draw orange spheres for intersections that make up the minimum
			// polygon describing where the RDF antennas all meet
			for (int i = 0; i < filtered.Count(); i++) {
				vector intersection = filtered[i];
				PrintFormat("Intersection detected at %1", intersection);
				ShapeFlags shapeFlags = ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOOUTLINE;
				m_aDebugShapes.Insert(Shape.CreateSphere(Color.Orange.PackToInt(), shapeFlags, intersection, 10));		
			}
		}
	}
};

bool PointInTriangle(vector p, vector a, vector b, vector c, float epsilon = 1) {
	vector ab = b - a;
	vector bc = c - b;
	vector ca = a - c;
	
	vector ap = p - a;
	vector bp = p - b;
	vector cp = p - c;
	
	// 2D cross product for each edge vs point vector
	float cross1 = ab[0] * ap[2] - ab[2] * ap[0];
	float cross2 = bc[0] * bp[2] - bc[2] * bp[0];
	float cross3 = ca[0] * cp[2] - ca[2] * cp[0];
	
	bool allPos = (cross1 >= -epsilon) && (cross2 >= -epsilon) && (cross3 >= -epsilon);
	bool allNeg = (cross1 <= epsilon) && (cross2 <= epsilon) && (cross3 <= epsilon);
	
	return allPos || allNeg;
}

bool GetLineSegmentIntersection(vector a1, vector a2, vector b1, vector b2, out vector intersection) {
	float x1 = a1[0], z1 = a1[2];
	float x2 = a2[0], z2 = a2[2];
	float x3 = b1[0], z3 = b1[2];
	float x4 = b2[0], z4 = b2[2];
	
	float denominator = (x1 - x2) * (z3 - z4) - (z1 - z2) * (x3 - x4);
	
	if (Math.AbsFloat(denominator) < 0.0001)  {
		return false; // Lines are parallel or coincident
	}
	
	float t = ((x1 - x3) * (z3 - z4) - (z1 - z3) * (x3 - x4)) / denominator;
	float u = ((x1 - x3) * (z1 - z2) - (z1 - z3) * (x1 - x2)) / denominator;
	
	if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
		float ix = x1 + t * (x2 - x1);
		float iz = z1 + t * (z2 - z1);
		
		intersection = {ix, 0, iz};
		return true;
	}
	
	return false;
}

vector GetRotatedPoint(vector origin, vector target, float angleDeg, float distance) {
	vector dir = vector.Direction(origin, target);
	dir.Normalize();
	dir[1] = 0;
	
	float rad = Math.DEG2RAD * angleDeg;
	float x = dir[0];
	float z = dir[2];
	
	float newX = x * Math.Cos(rad) - z * Math.Sin(rad);
	float newZ = x * Math.Sin(rad) + z * Math.Cos(rad);
	
	vector rotated = {newX, origin[1], newZ};
	
	return origin + rotated * distance;
}
