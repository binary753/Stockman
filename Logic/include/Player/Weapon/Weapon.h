#ifndef WEAPON_H
#define WEAPON_H

#include <Projectile\ProjectileManager.h>

#pragma region ClassDesc
		
		/*
			CLASS: Weapon
			AUTHOR:

			DESCRIPTION: This class defines a weapon of the system
		*/

#pragma endregion

namespace Logic
{
	class Weapon
	{
	private:
		ProjectileManager* m_projectileManager;
		ProjectileData m_projectileData;
		int m_weaponID;
		int m_ammoCap;
		int m_ammo;
		int m_magSize;
		int m_magAmmo;
		int m_ammoConsumption;
		float m_damage;
		float m_attackRate;		// Attacks per minute
		float m_freeze;
		float m_reloadTime;
	//	Animation m_animation;
	public:
		Weapon();
		Weapon(ProjectileManager* projectileManager, ProjectileData projectileData, int weaponID, int ammoCap, int ammo, int magSize, int magAmmo, int ammoConsumption, float damage, float attackRate, float freeze, float reloadTime);

		void use(btVector3 position, btVector3 forward);

		void update();

		ProjectileData* getProjectileData();
		int getAmmoCap();
		void setAmmoCap(int ammoCap);
		int getAmmo();
		void setAmmo(int ammo);
		int getMagSize();
		void setMagSize(int magSize);
		int getMagAmmo();
		void removeMagAmmo();
		void removeMagAmmo(int ammo);
		int getAmmoConsumption();
		float getAttackTimer();

		void fillMag();
	};
}

#endif