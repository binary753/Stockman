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

#define EDIT_CAMERA_POS false

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
    m_menu = newd iMenuMachine();
    m_menu->swapMenu(iMenu::MenuGroup::Start);

    // ! Reminder !  
    // Gives a small mem leak as for right now, but it's too cool to remove ^.^
    // Initializing campfire
    m_campfire = Graphics::FXSystem->getEffect("FireSmoke");
}

StateStart::~StateStart()
{
    delete m_menu;
    delete m_highScoreManager;
    delete m_physics;
    delete m_map;
}

void StateStart::reset() { }

void StateStart::update(float deltaTime)
{
    Graphics::FXSystem->processEffect(&m_campfire, DirectX::XMMatrixTranslation(0, 0, 0), deltaTime / 1000.f);

    DirectX::Mouse::Get().SetMode(DirectX::Mouse::Mode::MODE_ABSOLUTE);
    static DirectX::SimpleMath::Vector3 movingCameraPosition(6.941, 5.6, -4.141);
    static DirectX::SimpleMath::Vector3 movingCameraForward(-0.2, -0.153, 0.258);

#if EDIT_CAMERA_POS
    // Debugging purposes
    ImGui::Begin("Camera");
    ImGui::SliderFloat3("Position", reinterpret_cast<float*>(&movingCameraPosition), -10.f, 10.f, "%.3f");
    ImGui::SliderFloat3("Forward", reinterpret_cast<float*>(&movingCameraForward), -1.f, 1.f, "%.3f");
    ImGui::End();
#else
    // Moving the camera slightly
    movingCameraForward.x += (-0.3f - movingCameraForward.x) * 0.00001f * deltaTime;
#endif

    Global::mainCamera->update(movingCameraPosition, movingCameraForward, Global::context);

    PROFILE_BEGIN("Physics");
    m_physics->update(deltaTime);
    PROFILE_END();

    PROFILE_BEGIN("Map");
    m_map->update(deltaTime);
    PROFILE_END();

    m_fpsRenderer.updateFPS(deltaTime);
    m_menu->update(deltaTime);
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