#include <StateStart.h>
#include <StateMachine\StateBuffer.h>
#include <Misc\Sound\NoiseMachine.h>
#include <Engine\Typing.h>
#include <DebugDefines.h>
#include <Engine\DebugWindow.h> 
#include <Physics\Physics.h>
#include <Map.h>
#include <Graphics\include\MainCamera.h>
#include <Graphics\include\Device.h>

using namespace Logic;

StateStart::StateStart(StateBuffer* stateBuffer)
    : State(stateBuffer)
{
    Sound::NoiseMachine::Get().playMusic(Sound::MUSIC::AMBIENT_STORM, nullptr, true);
    Sound::NoiseMachine::Get().playMusic(Sound::MUSIC::MUSIC_MAIN_MENU, nullptr, true);

    // Making a small little campfire map to look at
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();			
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);	
    btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();								
    btSequentialImpulseConstraintSolver* constraintSolver = new btSequentialImpulseConstraintSolver();			
    m_physics = new Physics(dispatcher, overlappingPairCache, constraintSolver, collisionConfiguration);
    m_physics->init();
    m_map = newd Map();
    m_map->init(m_physics, "Campfire.txt");

    // Initializing Highscore Manager
    m_highScoreManager = newd HighScoreManager();
    m_highScoreManager->setName("Stockman");

    // Initializing Menu's
    m_menu = newd MenuMachine();
    m_menu->initialize(Logic::gameStateMenuMain);
}

StateStart::~StateStart()
{
    m_menu->clear();

    delete m_menu;
    delete m_highScoreManager;
    delete m_physics;
    delete m_map;
}

void StateStart::reset() { }

void StateStart::update(float deltaTime)
{
    DirectX::Mouse::Get().SetMode(DirectX::Mouse::Mode::MODE_ABSOLUTE);
    static DirectX::SimpleMath::Vector3 movingCameraPosition(0, 0, 0);
    static DirectX::SimpleMath::Vector3 movingCameraForward(0, 0, 1);
    movingCameraPosition.y += 0.001f * deltaTime;
    Global::mainCamera->update(movingCameraPosition, movingCameraForward, Global::context);

    PROFILE_BEGIN("Physics");
    m_physics->update(deltaTime);
    PROFILE_END();

    PROFILE_BEGIN("Map");
    m_map->update(deltaTime);
    PROFILE_END();

    m_fpsRenderer.updateFPS(deltaTime);
    m_menu->update(deltaTime);

    if (m_menu->currentState() == gameStateSkillPick)
    {
        SwitchPrimaryState(StateType::State_Playing);
        SwitchSecondaryState(StateType::State_InGame_Overlay);
    }
}

void StateStart::render() const
{
    PROFILE_BEGIN("Render Map");
    m_map->render();
    PROFILE_END();

    PROFILE_BEGIN("Render Menu's");
    m_menu->render();
    PROFILE_END();

    m_fpsRenderer.render();
}