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


int CurrentWeaponKills[16];
int firstPass = 0;
int oldXP = 0;

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
	switch (a) {
		case 0: id = 4; break;
		case 1: id = 3; break;
		case 2: id = 2; break;
		case 3: id = 7; break;
		case 4: id = 5; break;
		case 5: id = 9; break;
		case 6: id = 1; break;
		case 7: id = 6; break;
		case 8: id = 8; break;
		case 9: id = 10; break;
	}
	return id;
}

int checkWeaponKill(Player *player)
{
	// Get current held weapon
	int weapon = player->WeaponHeldId;
	// if Weapon equals: Nothing, Wrench, Grind Cable, EMP, Hacker Ray or Swingshot return
	if (weapon == 0 || weapon == 1 || weapon == 11 || weapon == 12 || weapon == 13 || weapon == 14)
		return;
	
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
	player->ChangeWeaponHeldId = getRandomNum(player->timers.timeAlive);
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

	// Constantly set weapon ammo to 200
	player->GadgetBox->Gadgets[player->WeaponHeldId].Ammo = 200;

	player->timers.allowQuickSelect = 0x7fff;

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
