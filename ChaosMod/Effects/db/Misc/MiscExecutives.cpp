#include <stdafx.h>

static Hash highEndVehs[48] = {
	2983812512, 3003014393, 418536135, 2598821281, 2672523198, 3078201489, 338562499, 819197656, 2067820283, 3062131285,
	1987142870, 1034187331, 1093792632, 2246633323, 3812247419, 272929391, 989294410, 2536829930, 408192225, 234062309,
	1426219628, 2465164804, 2123327359, 2891838741, 1663218586, 3999278268, 633712403, 1234311532, 917809321, 1939284556,
	3052358707, 1392481335, 3296789504, 1352136073, 3981782132, 2174267100, 3918533058, 1031562256, 3160260734, 3656405053,
	1591739866, 3630826055, 3970348707, 1044193113, 3612858749, 1323778901, 960812448, 2936769864
};

static void replaceVehicle(Vehicle veh) {
	if (IS_ENTITY_A_MISSION_ENTITY(veh)) return; // (kolyaventuri): Don't replace critial entities

	float heading;
	std::vector<Ped> vehPeds;
	Vector3 coords;
	Vector3 velocity;
	bool isEngineRunning = true;
	float forwardSpeed = 0;

	// (kolyaventuri): Store data for original vehicle
	int numSeats = GET_VEHICLE_MODEL_NUMBER_OF_SEATS(GET_ENTITY_MODEL(veh));
	isEngineRunning = GET_IS_VEHICLE_ENGINE_RUNNING(veh);
	coords = GET_ENTITY_COORDS(veh, false);
	heading = GET_ENTITY_HEADING(veh);
	velocity = GET_ENTITY_VELOCITY(veh);
	forwardSpeed = GET_ENTITY_SPEED(veh);

	// (kolyaventuri): Store peds
	for (int i = -1; i < numSeats - 1; i++) {
		if (!IS_VEHICLE_SEAT_FREE(veh, i, false)) {
			Ped ped = GET_PED_IN_VEHICLE_SEAT(veh, i, false);
			SET_ENTITY_AS_MISSION_ENTITY(ped, true, true);
			vehPeds.push_back(ped);
		}
	}

	// (kolyaventuri): Remove original vehicle
	SET_ENTITY_AS_MISSION_ENTITY(veh, true, true);
	DELETE_VEHICLE(&veh);

	// (kolyaventuri): Get a new vehicle
	Hash randomVeh = highEndVehs[g_random.GetRandomInt(0, 48)];
	LoadModel(randomVeh);
	Vehicle newVeh = CREATE_VEHICLE(randomVeh, coords.x, coords.y, coords.z + 1.f, heading, true, true, true);
	int newSeats = GET_VEHICLE_MODEL_NUMBER_OF_SEATS(GET_ENTITY_MODEL(newVeh));

	// (kolyaventuri): Refill the vehicle
	for (int i = 0; i < vehPeds.size(); i++) {
		int ped = vehPeds[i];
		if (i > numSeats - 1) {
			// (kolyaventuri): Ragdoll everyone that doesn't fit in the car. Not their lucky day.
			CLEAR_PED_TASKS_IMMEDIATELY(ped);
			SET_PED_TO_RAGDOLL(ped, 10000, 10000, 0, true, true, false);
			continue;
		}

		int seatIdx = i == 0 ? -1 : -2;
		SET_PED_INTO_VEHICLE(ped, newVeh, seatIdx);
	}

	// Restore
	if (isEngineRunning) {
		SET_VEHICLE_ENGINE_ON(newVeh, true, true, false);
	}
	SET_ENTITY_VELOCITY(newVeh, velocity.x, velocity.y, velocity.z);
	SET_VEHICLE_FORWARD_SPEED(newVeh, forwardSpeed);

	SET_MODEL_AS_NO_LONGER_NEEDED(randomVeh);
}

static void OnStart() {
	for (Vehicle veh : GetAllVehs()) {
		if (!IS_PED_IN_VEHICLE(PLAYER_PED_ID(), veh, false)) {
			replaceVehicle(veh);
		}
	}
}

static void OnTick() {}

static RegisterEffect registerEffect(EFFECT_MISC_EXECUTIVES, OnStart, nullptr, OnTick);