#include "Game.h"
#include <iostream>
#include <Engine\Typing.h>

#include <GameType.h>
#include <DebugDefines.h>
#include <Engine\DebugWindow.h> 
#include <Player\Skill\SkillManager.h>

using namespace Logic;

const int Game::NUMBER_OF_UNIQUE_CARDS      = 13;

const GameState Game::STATE_START           = gameStateMenuMain;
const btVector3 Game::PLAYER_START_SCALE    = { 1.5f, 3.0f, 1.5f };
const btVector3 Game::PLAYER_START_ROTATION = { 0.0f, 0.0f, 0.0f };

Game::Game()
{
	m_physics			= nullptr;
	m_player			= nullptr;
	m_map				= nullptr;
	m_projectileManager = nullptr;
	m_cardManager		= nullptr;
	m_menu				= nullptr;
	m_highScoreManager	= nullptr;
}

Game::~Game() 
{ 
	clear();
}

void Game::init(LPWSTR *cmdLine, int args)
{
    m_gameType = GameType::NORMAL_MODE;

	// Initializing Sound
	Sound::NoiseMachine::Get().init();

	// Initializing Bullet physics
	btDefaultCollisionConfiguration* collisionConfiguration		= new btDefaultCollisionConfiguration();				// Configuration
	btCollisionDispatcher* dispatcher							= new btCollisionDispatcher(collisionConfiguration);	// The default collision dispatcher
	btBroadphaseInterface* overlappingPairCache					= new btDbvtBroadphase();								// Detecting aabb-overlapping object pairs
	btSequentialImpulseConstraintSolver* constraintSolver		= new btSequentialImpulseConstraintSolver();			// Default constraint solver
	m_physics = new Physics(dispatcher, overlappingPairCache, constraintSolver, collisionConfiguration);
	m_physics->init();

	// Initializing Projectile Manager
	m_projectileManager = newd ProjectileManager(m_physics);

	// Initializing Player
	m_player = new Player(Resources::Models::UnitCube, nullptr, Game::PLAYER_START_SCALE);
	m_player->init(m_physics, m_projectileManager);
	Sound::NoiseMachine::Get().update(m_player->getListenerData());

	// Initializing Highscore Manager
	m_highScoreManager = newd HighScoreManager();
	m_highScoreManager->setName("Stockman");

	for (int i = 0; i < 10; i++)
	{
		if (m_highScoreManager->gethighScore(i).score != -1)
		{
			highScore[i] = std::to_string(i + 1) + ". " + m_highScoreManager->gethighScore(i).name + ": " + std::to_string(m_highScoreManager->gethighScore(i).score);
		}
		else
		{
			highScore[i] = "";
		}
	}

	// Initializing Menu's
	m_menu = newd MenuMachine(m_highScoreManager->getName());
	m_menu->initialize(Game::STATE_START);

	// Initializing the Map
	m_map = newd Map();
	m_map->init(m_physics);

	// Initializing Card Manager
	m_cardManager = newd CardManager();
	m_cardManager->init();
	m_cardManager->createDeck(Game::NUMBER_OF_UNIQUE_CARDS);

	// Initializing Combo's
	ComboMachine::Get().ReadEnemyBoardFromFile("Nothin.");
	ComboMachine::Get().Reset();

    // Loading func
    m_entityManager.setSpawnFunctions(*m_projectileManager, *m_physics);

#ifdef _DEBUG
    DebugWindow *win = DebugWindow::getInstance();
    win->registerCommand("SETGAMESTATE", [&](std::vector<std::string> &para) -> std::string {
        try {
            this->m_menu->setStateToBe(static_cast<GameState> (stoi(para[0])));
            return "Menu State set to " + stoi(para[0]);
        } catch (std::exception e) {
            return "Chaos is a pit.";
        }
    });

    for (int i = 1; i < args; i++) // first arg is name of file
        for (std::wstring const &str : GameTypeStr)
            if (wcscmp(str.c_str(), cmdLine[i]) == 0)
                m_gameType = static_cast<GameType> (i - 1); // offset for filename

    for (std::string const &cmd : GameCommands[m_gameType])
        if (!cmd.empty())
            win->doCommand(cmd.c_str());
#endif
}

void Game::clear()
{
	m_menu->clear();
	m_projectileManager->clear();
    m_entityManager.deallocateData(); // Have to deallocate before deleting physics
	Sound::NoiseMachine::Get().clear();
    Typing::releaseInstance();

	delete m_physics;
	delete m_player;
	delete m_menu;
	delete m_map;
	delete m_cardManager;
	delete m_highScoreManager;
	delete m_projectileManager;
}

void Game::reset()
{
    m_entityManager.deallocateData();
    m_player->reset();

	ComboMachine::Get().Reset();
}

void Game::update(float deltaTime)
{
    // Updates FPS at all times
    m_fpsRenderer.updateFPS(deltaTime);
 
    if (updateMenu(deltaTime))
        updateGame(deltaTime);
}

// Returns true if game should be updated or not
bool Game::updateMenu(float deltaTime)
{
    // Specific Menu interactions
    switch (m_menu->currentState())
    {
    case gameStateGame:
        if (m_menu->getStateToBe() == GameState::gameStateGameUpgrade)
        {
            m_cardManager->pickThree(false); //get some trigger for injury
            m_menu->update(deltaTime);
            break;
        }
        else if (m_menu->getStateToBe() != GameState::gameStateDefault)
        {
            m_menu->update(deltaTime);
            break;
        }
        else
        {
            DirectX::Keyboard::State ks = DirectX::Keyboard::Get().GetState();
            DirectX::Mouse::Get().SetMode(ks.IsKeyDown(DirectX::Keyboard::LeftAlt) ? DirectX::Mouse::MODE_ABSOLUTE : DirectX::Mouse::MODE_RELATIVE); // !TEMP!
            if (ks.IsKeyDown(DirectX::Keyboard::U))
            {
                m_menu->setStateToBe(gameStateGameUpgrade);
                m_cardManager->pickThree(false);
            }
            return true;
        }

        break;
    case gameStateGameUpgrade:
        m_menu->update(deltaTime);
        {
            if (m_menu->getStateToBe() != GameState::gameStateDefault)
                break;

            Card temp = m_cardManager->pick(m_menu->getChoiceUpgrade());
            if (temp.getName().compare("") != 0 && temp.getDescription().compare("") != 0)
            {
                //add information to player
                m_menu->setStateToBe(gameStateGame); //change to gameStateGame

                for (auto const& ID : temp.getUpgradesID())
                {
                    if (temp.getIsEffect())
                    {
                        m_player->getStatusManager().addStatus(static_cast<StatusManager::EFFECT_ID>(ID), 1); //edit to how you feel it should be
                    }
                    else
                    {
                        m_player->getStatusManager().addUpgrade(static_cast<StatusManager::UPGRADE_ID>(ID));
                    }
                }
            }
        }
        return true;

        break;
    case gameStateSkillPick:
        m_menu->update(deltaTime);
        {
            std::pair<int, int>* selectedSkills = m_menu->getSkillPick();
            if (selectedSkills->first != -1 && selectedSkills->second != -1)
            {
                m_player->getSkillManager()->switchToSkill({
                    SkillManager::SKILL(selectedSkills->second),
                    SkillManager::SKILL(selectedSkills->first)
                });
                m_player->setCurrentSkills(selectedSkills->first, selectedSkills->second);
               
                // Reset menu stuff
                selectedSkills->first = -1;
                selectedSkills->second = -1;

                m_menu->setStateToBe(gameStateGame); //change to gameStateGame
            }
        }
        break;
    case gameStateLoading:
    case gameStateMenuMain:
    case gameStateMenuSettings:
    case gameStateGameOver:
	case gameStateHighscore:
    default:
        m_menu->update(deltaTime);
        break;
    }

    return false;
}

void Game::updateGame(float deltaTime)
{
   	ComboMachine::Get().Update(deltaTime);
    if (m_waveTimeManager.update(deltaTime, m_entityManager))
        m_projectileManager->removeAllProjectiles();

	PROFILE_BEGIN("Sound");
	Sound::NoiseMachine::Get().update(m_player->getListenerData());
	PROFILE_END();

	PROFILE_BEGIN("Player");
	m_player->updateSpecific(deltaTime);
	PROFILE_END();

	PROFILE_BEGIN("Physics");
	m_physics->update(deltaTime);
	PROFILE_END();

	PROFILE_BEGIN("AI & Triggers");
    if (m_gameType != GameType::TESTING_MODE)
	    m_entityManager.update(*m_player, deltaTime);
	PROFILE_END();

	PROFILE_BEGIN("Map");
	m_map->update(deltaTime);
	PROFILE_END();

	PROFILE_BEGIN("Projectiles");
	m_projectileManager->update(deltaTime);
	PROFILE_END();

    PROFILE_BEGIN("HUD");
    m_hudManager.update(*m_player, m_waveTimeManager, m_entityManager);
    PROFILE_END();

    if (DirectX::Keyboard::Get().GetState().IsKeyDown(DirectX::Keyboard::NumPad8))
        m_player->takeDamage(1, 0);

	if (m_player->getHP() <= 0)
		gameOver();
}

void Game::gameOver()
{
	m_highScoreManager->addNewHighScore(ComboMachine::Get().GetCurrentScore());
	m_menu->setStateToBe(GameState::gameStateGameOver);

	for (int i = 0; i < 10; i++)
	{
		if (m_highScoreManager->gethighScore(i).score != -1)
		{
			highScore[i] = std::to_string(i + 1) + ". " + m_highScoreManager->gethighScore(i).name + ": " + std::to_string(m_highScoreManager->gethighScore(i).score);
			break;
		}
	}
	reset();
}

void Game::render() const
{
	switch (m_menu->currentState())
	{
	case gameStateGame:
        renderGame();
		break;

	case gameStateGameUpgrade:
	case gameStateLoading:
	case gameStateMenuMain:
	case gameStateMenuSettings:
	case gameStateGameOver:
    case gameStateSkillPick:
	case gameStateHighscore:
    default: renderMenu();
		break;
	}

    m_fpsRenderer.render();
}

void Game::renderGame() const
{
    // Debug Draw physics
    if (DirectX::Keyboard::Get().GetState().IsKeyDown(DirectX::Keyboard::LeftShift))
        m_physics->render();

	PROFILE_BEGIN("Player Render");
	m_player->render();
	PROFILE_END();

	PROFILE_BEGIN("Render Map");
	m_map->render();
	PROFILE_END();

	PROFILE_BEGIN("Render Enemies & Triggers");
	m_entityManager.render();
	PROFILE_END();

	PROFILE_BEGIN("Render Projectiles");
	m_projectileManager->render();
	PROFILE_END();

    PROFILE_BEGIN("Render HUD");
    m_hudManager.render();
    PROFILE_END();
}

void Game::renderMenu() const
{
	m_menu->render();
}

DirectX::SimpleMath::Vector3 Game::getPlayerForward()
{
	return m_player->getForward();
}

DirectX::SimpleMath::Vector3 Game::getPlayerPosition()
{
	return m_player->getPosition();
}

int Game::getState() const
{
    return m_menu->currentState();
}
