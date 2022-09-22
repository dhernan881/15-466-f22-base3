#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <queue>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, one, two, three;

	// Enemy Stuff:
	enum EnemyType {
		DEFAULT = 0,
		RED = 1,
		GREEN = 2,
		YELLOW = 3
	};
	static constexpr uint NumEnemyTypes = 3;
	struct Enemy {
		// Properties
		static constexpr float MoveSpeed = 5.0f;

		// Methods
		void update(float elapsed);

		Scene::Transform *transform = nullptr;
		std::queue< glm::vec3 > dest_queue = {}; // assumes destinations don't change
		EnemyType enemy_type = EnemyType::DEFAULT;
		uint enemy_list_idx = 0;
		uint drawables_idx = 0;
	};
	static constexpr float EnemySpawnDelay = 1.0f;
	static constexpr uint MaxEnemies = 1;
	void try_spawn_enemy();
	Enemy *get_closest_enemy();
	float enemy_spawn_timer = 0.0f;
	PlayMode::Enemy *active_enemies[PlayMode::MaxEnemies] = 
		{nullptr};
	std::queue< uint > enemies_to_delete;

	// Shooting Stuff:
	static constexpr float ShootDelay = 0.5f;
	float shoot_timer = 0.0f;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	Scene::Transform *amogus = nullptr;


	//music coming from the tip of the leg (as a demonstration):
	// std::shared_ptr< Sound::PlayingSample > leg_tip_loop;

	// Game music:
	std::shared_ptr< Sound::PlayingSample > music_loop;
	
	//camera:
	Scene::Camera *camera = nullptr;

	// screen dimensions (is there a better way to do this???)
	glm::uvec2 screen_dims;

};
