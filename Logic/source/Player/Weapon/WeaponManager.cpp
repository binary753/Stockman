#include "Player/Weapon/WeaponManager.h"

using namespace Logic;

WeaponManager::WeaponManager() { }

WeaponManager::~WeaponManager() { clear(); }

void WeaponManager::init(ProjectileManager* projectileManager)
{
	m_projectileManager = projectileManager;
	initializeWeapons();
	makeWeaponLoadout();
	m_currentWeapon = m_weaponsLoadouts[0];

	// Timers
	m_swapWeaponTimerMax = 1000.f;
	m_swapWeaponTimer = 0.f;
	m_attackTimer = 0.f;

	m_reloadTimer = 0.f;
	m_reloadState = ReloadingWeapon::IDLE;
}

void WeaponManager::clear()
{
	m_weaponsLoadouts.clear();
	m_allWeapons.clear();
}

void Logic::WeaponManager::reset()
{
	for (int i = 0; i < m_allWeapons.size(); i++)
	{
		m_allWeapons[i].reset();
	}
}

void WeaponManager::setWeaponModel(DirectX::SimpleMath::Matrix playerTranslation, DirectX::SimpleMath::Vector3 playerForward)
{
	// Updating weapon model
	m_currentWeapon.first->setWeaponModelFrontOfPlayer(playerTranslation, playerForward);
}

void WeaponManager::update(float deltaTime)
{
	// Timers / cooldowns

	// Reload
	if (m_reloadTimer > 0.f)
	{
		m_reloadTimer -= deltaTime;
	}
	else if (m_reloadState == ReloadingWeapon::ACTIVE)
	{
		m_reloadState = ReloadingWeapon::DONE;
	}
	if (m_reloadState == ReloadingWeapon::DONE)
	{
		m_currentWeapon.first->fillMag();
		m_reloadState = ReloadingWeapon::IDLE;
		printf("adding ammo\n");
		printf("ammo: %d\n", m_currentWeapon.first->getAmmo());
		printf("mag: %d\n", m_currentWeapon.first->getMagAmmo());
	}

	// Attack
	if (m_attackTimer > 0.f)
		m_attackTimer -= deltaTime;
	// Weapon swap
	if (m_swapWeaponTimer > 0.f)
	{
		m_swapWeaponTimer -= deltaTime;
		m_attackTimer = m_swapWeaponTimer;
	}
}

void WeaponManager::render(Graphics::Renderer& renderer)
{
	m_currentWeapon.first->render(renderer);
}

void WeaponManager::initializeWeapons()
{
	// Adding all weapons
	m_allWeapons =
	{
		{ Weapon(Graphics::ModelID::CROSSBOW, m_projectileManager, ProjectileData(1, 0.2, 1, 100, 0.f, 3000, Graphics::ModelID::CUTTLERY, 1), 0, 60, 60, 30, 30, 1, 1, 0, 0, 450, 0, 2000) },		                                    // Gattling prim
		{ Weapon(Graphics::ModelID::CROSSBOW, m_projectileManager, ProjectileData(1, 0.1f, 1, 100, 0.f, 500, Graphics::ModelID::CUTTLERY, 1), 1, 60, 60, 8, 8, 6, 18, 15, 10, 100, 0, 2000) },		                                    // Gattling sec
		{ Weapon(Graphics::ModelID::STAFF, m_projectileManager, ProjectileData(1, 1, 1, 20, 0, 675, Graphics::ModelID::CUBE, 1, ProjectileType::ProjectileTypeIce, true), 2, 300, 200, 100, 100, 1, 1, 17, 5, 750, 1, 3000) },		// Freeze prim
		{ Weapon(Graphics::ModelID::STAFF, m_projectileManager, ProjectileData(1, 1, 1, 50, 5, 5000, Graphics::ModelID::CUBE, 1), 3, 100, 100, 50, 50, 10, 1, 0, 0, 100, 1, 3000) },		                                            // Freeze sec
		{ Weapon(Graphics::ModelID::STAFF, m_projectileManager, ProjectileData(1, 2, 1, 0, 0, 300, Graphics::ModelID::CUBE, 1, ProjectileType::ProjectileTypeMelee, true), 4, 0, 0, 0, 0, 0, 1, 0, 0, 50, 0, 2000) },				                                            // Sledge prim
		{ Weapon(Graphics::ModelID::STAFF, m_projectileManager, ProjectileData(1, 2, 1, 0, 0, 300, Graphics::ModelID::CUBE, 1, ProjectileType::ProjectileTypeMelee, true), 5, 0, 0, 0, 0, 0, 1, 0, 0, 50, 0, 3000) }					                                        // Sledge sec
	};
}

void WeaponManager::makeWeaponLoadout()
{
	// Adding current weapon pair
	m_weaponsLoadouts =
	{ 
		{ &m_allWeapons[0], &m_allWeapons[1] },
		{ &m_allWeapons[2], &m_allWeapons[3] },
		{ &m_allWeapons[4], &m_allWeapons[5] }
	};
}

void WeaponManager::switchWeapon(int index)
{
	// Check if swapTimer is active or swapping to same weapon 
	if (m_swapWeaponTimer <= 0.f && !(m_currentWeapon == m_weaponsLoadouts[index]))
	{
		m_currentWeapon = m_weaponsLoadouts[index];
		m_swapWeaponTimer = m_swapWeaponTimerMax;

		m_reloadTimer = 0.f;
		m_reloadState = ReloadingWeapon::IDLE;

		printf("switch weapon %d\n", index);
	}
}

void WeaponManager::usePrimary(btVector3 position, float yaw, float pitch, Entity& shooter)
{
	if(m_attackTimer <= 0.f)
	{
		if (m_currentWeapon.first->getMagAmmo() > 0 || m_currentWeapon.first->getAmmoConsumption() == 0)
		{
			m_currentWeapon.first->use(position, yaw, pitch, shooter);
			m_currentWeapon.first->removeMagAmmo(m_currentWeapon.first->getAmmoConsumption());
		}
		else
			printf("out of ammo\n");

		m_attackTimer = m_currentWeapon.first->getAttackTimer();
	}
}

void WeaponManager::useSecondary(btVector3 position, float yaw, float pitch, Entity& shooter)
{
	if (m_attackTimer <= 0.f)
	{
		if (m_currentWeapon.first->getMagAmmo() > 0 || m_currentWeapon.second->getAmmoConsumption() == 0)
		{
			m_currentWeapon.second->use(position, yaw, pitch, shooter);
			m_currentWeapon.first->removeMagAmmo(m_currentWeapon.second->getAmmoConsumption());
		}
		else
			printf("out of ammo\n");

		m_attackTimer = m_currentWeapon.second->getAttackTimer();
	}
}

void Logic::WeaponManager::reloadWeapon()
{
	if (m_reloadTimer <= 0.f && m_currentWeapon.first->getAmmo() > 0 && m_currentWeapon.first->getMagAmmo() < m_currentWeapon.first->getMagSize())
	{
		m_reloadTimer = m_currentWeapon.first->getRealoadTime();
		m_reloadState = ReloadingWeapon::ACTIVE;
		printf("reloading weapon\n");
	}
}

bool Logic::WeaponManager::isSwitching()
{
	return m_swapWeaponTimer > 0.f;
}

bool Logic::WeaponManager::isAttacking()
{
	return m_attackTimer > 0.f;
}

bool Logic::WeaponManager::isReloading()
{
	return m_reloadState != ReloadingWeapon::IDLE;
}

Weapon* Logic::WeaponManager::getCurrentWeaponPrimary()
{
	return m_currentWeapon.first;
}

Weapon* Logic::WeaponManager::getCurrentWeaponSecondary()
{
	return m_currentWeapon.second;
}

Weapon * Logic::WeaponManager::getfirstWeapon()
{
    if (m_weaponsLoadouts[0] == m_currentWeapon)
    {
        return m_weaponsLoadouts[0].first;
    }
    else
    {
        return m_weaponsLoadouts[1].first;
    } 
}

Weapon * Logic::WeaponManager::getSecondWeapon()
{
    if (m_weaponsLoadouts[0] != m_currentWeapon)
    {
        return m_weaponsLoadouts[0].first;
    }
    else
    {
        return m_weaponsLoadouts[1].first;
    };
}
