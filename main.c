#include <tamtypes.h>
#include <libdl/stdio.h>
#include <libdl/game.h>
#include <libdl/player.h>
#include <libdl/math.h>
#include <libdl/weapon.h>
#include <libdl/hud.h>

#define CURRENT_LEVEL				(*(int*)0x0021de10)
#define CURRENT_XP					(*(int*)0x00171B48)
#define WEAPON_KILLS				(0x0020b53c)
#define WEAPON_EQUIPSLOT			((int*)0x0020c690)


int CurrentWeaponKills[16];
int firstPass = 0;
int oldXP = 0;

void runConstants(Player *player)
{
	// Give player 200 ammo always
	player->GadgetBox->Gadgets[player->WeaponHeldId].Ammo = 200;
	// hide slots 2 and 3
	int * quickselect = WEAPON_EQUIPSLOT;
	quickselect[1] = WEAPON_ID_EMPTY;
	quickselect[2] = WEAPON_ID_EMPTY;
	// don't let player access quick select.
	player->timers.allowQuickSelect = 0xffff;
}

void giveWeapons(Player *player)
{
	int i;
	for (i = 0; i < 16; ++i) {
		// Gives the player each weapon at level 1, if not already given.
		//if (player->GadgetBox->Gadgets[i].Level < 0)
			player->GadgetBox->Gadgets[i].Level = 0;
	}
}

void getWeaponKills(void)
{
	int i;
	for (i = 0; i < 14; ++i) {
		int kills = *(int*)(WEAPON_KILLS + (i * 4));
		CurrentWeaponKills[i] = kills;
		DPRINTF("\nCurrent Kills (%d): %d", i, kills);
	}
}

int getRandomNum(int seed)
{
	int a = (seed % 9);
	int id;
	// Set random number random weapon ID.
	switch (a) {
		case 0: id = 4; break;
		case 1: id = 3; break;
		case 2: id = 2; break;
		case 3: id = 7; break;
		case 4: id = 5; break;
		case 5: id = 9; break;
		case 6: id = 15; break;
		case 7: id = 6; break;
		case 8: id = 8; break;
		case 9: id = 10; break;
	}
	// return weapon ID
	return id;
}

void checkWeaponKill(Player *player)
{
	// Get current held weapon from quick select slot 0
	int * quickselect = WEAPON_EQUIPSLOT;
	int weapon = quickselect[0];

	// Weapon kill list starts with dual vieprs, which is ID 2.
	int multiply = weapon - 2;
	int kills = *(int*)(WEAPON_KILLS + (multiply * 4));
	// if current held weapon hasn't changed, then exit.
	if (CurrentWeaponKills[multiply] == kills)
		return;
	
	// If we got here, we must of got a kill!
	DPRINTF("\nGot Kill on (%d): %d -> %d", weapon, CurrentWeaponKills[multiply], kills);
	// Update current Weapon Kills
	CurrentWeaponKills[multiply] = kills;

	// Cycle to next random weapon
	int getNewWeapon = getRandomNum(player->timers.timeAlive);
	quickselect[0] = getNewWeapon;
	player->ChangeWeaponHeldId = getNewWeapon;
}

int main(void)
{
	// Get pointer for Player Struct
	void* PlayerPointer = (void*)(*(u32*)0x001eeb70);
	if (PlayerPointer == 0 || PlayerPointer == 3451532)
		return -1;

	dlPreUpdate();

	// Subtract offset to get to begining of player struct so we can use libdl's offsets.
	Player* player = (Player*)((u32)PlayerPointer - 0x2FEC);
	PadButtonStatus* pad = playerGetPad(player);

	// Do Constants
	runConstants(player);

	// if FirstPass is false, set all current weapon kills.
	if (!firstPass) {
		giveWeapons(player);
		getWeaponKills();
	}
	// if Current XP doesn't match stored XP, did we get a kill?
	if (CURRENT_XP != oldXP) {
		// Check to see if we got a kill with a weapon
		checkWeaponKill(player);

		// Update stored XP with current XP
		oldXP = CURRENT_XP;
		DPRINTF("\nXP: %d", CURRENT_XP);
		DPRINTF("\nCurrent Held ID: %d", player->WeaponHeldId);
	}

	firstPass = 1;

	dlPostUpdate();
	return 0;
}
