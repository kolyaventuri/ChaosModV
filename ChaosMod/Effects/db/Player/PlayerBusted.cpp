#include <stdafx.h>

enum ArrestState {
	start = 0,
	startBust,
	handsUp,
	cleanup
};

static const int stopTime = 300;
static const char* cops[3] = { "s_m_y_cop_01", "s_f_y_cop_01", "csb_cop" };
static int currentMode = ArrestState::start;
static int lastModeTime = 0;
static int nextModeTime = 0;

static void OnStart() {
	Player playerId = PLAYER_ID();
	Ped player = PLAYER_PED_ID();
	const char* hashKey = cops[g_random.GetRandomInt(0, 2)];

	currentMode = ArrestState::start;
	lastModeTime = 0;
	nextModeTime = 0;

	Ped cop = CreatePoolPed(6, GET_HASH_KEY(hashKey), 0.f, 0.f, 0.f, GET_ENTITY_HEADING(player));
	SET_PED_AS_COP(cop, true);
	GIVE_WEAPON_TO_PED(cop, GET_HASH_KEY("WEAPON_PISTOL"), 100, false, true); // (kolyaventuri): Cops must have weapon wielded for the arrest task to fire

	Vector3 minSize;
	Vector3 maxSize;
	Vehicle veh;
	boolean inVeh = false;
	if (IS_PED_IN_ANY_VEHICLE(player, false)) {
		inVeh = true;
		veh = GET_VEHICLE_PED_IS_IN(player, false);
		BRING_VEHICLE_TO_HALT(veh, 3.f, stopTime, 0);
		TASK_LEAVE_VEHICLE(player, veh, 256);
		WAIT(stopTime);
	}

	/*
	* (kolyaventuri): Ensure that the police cannot spawn inside of the car model.
	* It was causing issues where the stars would immediately bump to 2, given the cop was injured
	*/
	Vector3 pos = GET_ENTITY_COORDS(player, true);
	Hash model = GET_ENTITY_MODEL(inVeh ? veh : player);
	GET_MODEL_DIMENSIONS(model, &minSize, &maxSize);
	Vector3 modelSize = (maxSize - minSize) * 1.15f;

	Vector3 newPos = GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(inVeh ? veh : player, modelSize.x, modelSize.y, 0.0);
	SET_ENTITY_COORDS(cop, newPos.x, newPos.y, newPos.z, 1, 0, 0, 1);


	while (currentMode < ArrestState::cleanup) {
		// (kolyaventuri): Ensure player cannot move
		DISABLE_ALL_CONTROL_ACTIONS(0);
		WAIT(0);

		int currentTime = GetTickCount64();
		if (currentTime - lastModeTime > nextModeTime)
		{
			nextModeTime = 2000;
			lastModeTime = currentTime;
			currentMode++;
		}
		else {
			continue;
		}

		if (currentMode != ArrestState::cleanup) {
			// (kolyaventuri): Player must be wanted for effect to fire
			SET_PLAYER_WANTED_LEVEL(playerId, 1, false);
			SET_PLAYER_WANTED_LEVEL_NOW(playerId, false);
		}

		switch (currentMode) {
			case ArrestState::startBust:
				TASK_ARREST_PED(cop, player);
				lastModeTime = GetTickCount64();
				nextModeTime = 15000;
				currentMode++;
				break;
			case ArrestState::cleanup:
				SET_ENTITY_AS_MISSION_ENTITY(cop, true, true);
				DELETE_PED(&cop);
				break;
		}
	}
}

static RegisterEffect registerEffect(EFFECT_PLAYER_BUSTED, OnStart);