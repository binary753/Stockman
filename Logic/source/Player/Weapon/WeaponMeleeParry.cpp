#include <Player\Weapon\WeaponMeleeParry.h>
#include <Projectile\Projectile.h>
#include <AI\Enemy.h>

using namespace Logic;

WeaponMeleeParry::WeaponMeleeParry()
{
}

WeaponMeleeParry::WeaponMeleeParry(ProjectileManager * projectileManager, ProjectileData & projectileData, WeaponInfo wInfo, float knockbackPower)
    : Weapon(projectileManager, projectileData, wInfo)
{
    m_knockbackPower = knockbackPower;
}

WeaponMeleeParry::~WeaponMeleeParry()
{
}

void WeaponMeleeParry::onUse(std::vector<Projectile*>& projectiles, Entity& shooter)
{
    static btVector3 pPosition;
    for (Projectile* p : projectiles)
    {
        pPosition = shooter.getPositionBT();
        p->addCallback(Entity::ON_COLLISION, [&](Entity::CallbackData &data) -> void {
            
            PhysicsObject* obj = reinterpret_cast<PhysicsObject*>(data.dataPtr);

            btVector3 knockbackDir = (obj->getPositionBT() - pPosition).normalize();

            if (Enemy* enemy = dynamic_cast<Enemy*>(obj))
            {
                obj->getRigidBody()->setLinearVelocity(knockbackDir * m_knockbackPower);
                enemy->getStatusManager().addStatus(StatusManager::EFFECT_ID::STUN, 1);
            }

        });
    }
}
