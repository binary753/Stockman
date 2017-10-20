#include "Player/Skill/SkillGrapplingHook.h"
#include <Player\Player.h>

using namespace Logic;

#define GRAPPLING_HOOK_CD			50.f		// Cooldown in ms
#define GRAPPLING_HOOK_RANGE		500.f		// Range in bulletphysics units (probably meters)
#define GRAPPLING_HOOK_POWER		0.0011f	// The amount of power to reach the max speed
#define GRAPPLING_HOOK_MAX_SPEED_XZ	0.0615f		// The max speed in x & z
#define GRAPPLING_HOOK_MAX_SPEED_Y	15.f		// The max speed in y

SkillGrapplingHook::SkillGrapplingHook(Physics* physics)
: Skill(GRAPPLING_HOOK_CD)
{
	// Default variables
	m_physicsPtr = physics;
	m_shooter = nullptr;
	m_point = { 0, 0, 0 };

	// Debug draw ray
	renderDebug.points = new std::vector<DirectX::SimpleMath::Vector3>;
	renderDebug.color = DirectX::SimpleMath::Color(1, 1, 1);
	renderDebug.topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	renderDebug.useDepth = true;
}

SkillGrapplingHook::~SkillGrapplingHook()
{
	// These are just borrowed
	m_physicsPtr = nullptr;
	m_shooter = nullptr;

	// Debug
	delete renderDebug.points;
}

// When the grappling hook is used, send out a ray to the targeted surface and save variables
void SkillGrapplingHook::onUse(btVector3 forward, Entity& shooter)
{
	// Ray testing to see if we're hitting a rigidbody
	const btRigidBody* intersection = m_physicsPtr->RayTestOnRigidBodies(Ray(shooter.getPositionBT(), forward, GRAPPLING_HOOK_RANGE));
	if (intersection)
	{
		if (Entity* target = static_cast<Entity*>(intersection->getUserPointer()))
		{
			// Saving the shooter's as an entity
			m_shooter = &shooter;

			// The entity is now pulling the grappling hook
			m_state = GrapplingHookStatePulling;

			// Saving ray to intersection surface
			Ray ray(shooter.getPositionBT(), forward, GRAPPLING_HOOK_RANGE);
			m_point = m_physicsPtr->RayTestGetPoint(ray);

			// Drawing the ray
			renderDebug.points->clear();
			renderDebug.color = DirectX::SimpleMath::Color(1, 1, 1);
			renderDebug.points->push_back(DirectX::SimpleMath::Vector3(ray.getStart()));
			renderDebug.points->push_back(DirectX::SimpleMath::Vector3(m_point));

		}
	}
}

// On button release
void SkillGrapplingHook::onRelease()
{
	// If unsuccesful hook, don't put full cooldown
	if (!m_shooter)
	{
		setCooldown(GRAPPLING_HOOK_CD);
		setCanUse(true);
	}
	else if (Player* player = dynamic_cast<Player*>(m_shooter))
	{
		player->getCharController()->setLinearVelocity({ 0.f, 0.f, 0.f });
	}


	// Set to defaults
	m_state = GrapplingHookStateNothing;
	m_shooter = nullptr;
	m_point = { 0, 0, 0 };
	renderDebug.color = DirectX::SimpleMath::Color( 1, 0, 0 );
}

// Moving the entity the grappling hook is active to the targeted point
void SkillGrapplingHook::onUpdate(float deltaTime)
{
	if (m_shooter && m_state == GrapplingHookStatePulling)
	{
		// Setting player movement specific grappling hook variables
		if (Player* player = dynamic_cast<Player*>(m_shooter))
		{
			btGhostObject* ghostObject = player->getGhostObject();
			btVector3 linearVelocity = player->getCharController()->getLinearVelocity();
			btVector3 dirToPoint = (m_point - player->getPositionBT()).normalize();

			// Checks if the player does a 90 degree turn around mid-air
			if (dirToPoint.dot(player->getMoveDirection()) > 0)
			{
				// Resets the vertical velocity to make a "stopping effect"
				player->getCharController()->setLinearVelocity({ 0, linearVelocity.y(), 0 });
			}
			else
			{
				// If the player targets hooks to something behind himself, reset the current speed to the ground-speed
				player->setMoveSpeed(PLAYER_MOVEMENT_MAX_SPEED);
			}

			// Sets the current movedirection to avoid breaking strafing
			player->setMoveDirection(btVector3(dirToPoint.x(), 0, dirToPoint.z()));

			// Easing to reach the targeted vertical speed
			//shooterBody->setLinearVelocity({ 0, linearVelocity.y() + (((dirToPoint.y()) * GRAPPLING_HOOK_MAX_SPEED_Y) - linearVelocity.y()) * GRAPPLING_HOOK_POWER * deltaTime, 0 });
			if(!(m_point.y() < player->getPositionBT().y()))
				player->getCharController()->setLinearVelocity({ 0.f, linearVelocity.y() + (((dirToPoint.y()) * GRAPPLING_HOOK_MAX_SPEED_Y) - linearVelocity.y()) * GRAPPLING_HOOK_POWER * deltaTime, 0.f });
				//player->getCharController()->jump({ 0.f, linearVelocity.y() + (((dirToPoint.y()) * GRAPPLING_HOOK_MAX_SPEED_Y) - linearVelocity.y()) * GRAPPLING_HOOK_POWER * 0.01f * deltaTime, 0.f });
				//player->getCharController()->setLinearVelocity({ 0.f, 1.f, 0.f });
			//else
				//player->getCharController()->setLinearVelocity({ 0.f, linearVelocity.y() + (((dirToPoint.y()) * GRAPPLING_HOOK_MAX_SPEED_Y) - linearVelocity.y()) * GRAPPLING_HOOK_POWER * deltaTime * 0.0000015f, 0.f });
				//player->getCharController()->jump({ 0.f, (((dirToPoint.y()) * GRAPPLING_HOOK_MAX_SPEED_Y) - linearVelocity.y()) * GRAPPLING_HOOK_POWER * deltaTime, 0.f });

			//printf("%f\n", player->getCharController()->getLinearVelocity().y());

			// Easing to reach the maximum vertical speed
			player->setMoveSpeed(player->getMoveSpeed() + ((GRAPPLING_HOOK_MAX_SPEED_XZ - player->getMoveSpeed()) * GRAPPLING_HOOK_POWER * deltaTime));
		}
		else
		{
			// Getting variables from entity
			btRigidBody* shooterBody = m_shooter->getRigidBody();
			btVector3 linearVelocity = shooterBody->getLinearVelocity();
			btVector3 dirToPoint = (m_point - m_shooter->getPositionBT()).normalize();
		}

		// Setting entity movement specific varialbes
		//
	}
}

void SkillGrapplingHook::render(Graphics::Renderer& renderer)
{
	// Drawing a ray of the grappling hook for debugging purposes
	renderer.queueRenderDebug(&renderDebug);
}

GrapplingHookState SkillGrapplingHook::getState() const
{
	return m_state;
}
