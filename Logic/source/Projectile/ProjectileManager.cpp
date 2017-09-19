#include "../Projectile/ProjectileManager.h"

using namespace Logic;

ProjectileManager::ProjectileManager(Physics* physPtr)
{
	m_physPtr = physPtr;
}

ProjectileManager::~ProjectileManager() { }

void ProjectileManager::clear()
{
	for (Projectile* p : m_projectiles)
		delete p;
	m_projectiles.clear();
}

void ProjectileManager::addProjectile(ProjectileData& pData, btVector3 position, btVector3 forward)
{
	btRigidBody* body = m_physPtr->createBody(Sphere(position + forward, btVector3(), pData.scale), 10.f, false);
	Projectile* p = newd Projectile(body, pData);
	p->start(forward);
	m_projectiles.push_back(p);
	printf("projs: %d\n", m_projectiles.size());
}

void ProjectileManager::removeProjectile()
{

}

void Logic::ProjectileManager::update(float deltaTime)
{
	for (Projectile* p : m_projectiles)
		p->update(deltaTime);
}

void Logic::ProjectileManager::render(Graphics::Renderer& renderer)
{
	for (Projectile* p : m_projectiles)
		p->render(renderer);
}

std::vector<Projectile*>* ProjectileManager::getProjectiles()
{
	return &m_projectiles;
}
