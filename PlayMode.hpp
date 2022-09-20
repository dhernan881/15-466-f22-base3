#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

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

	enum EnemyType {
		DEFAULT = 0,
		RED = 1,
		GREEN = 2,
		YELLOW = 3
	};
	struct Enemy {
		// Properties
		static constexpr float MoveSpeed = 10.0f;

		// Methods
		void update(float elapsed);

		Scene::Transform *transform = nullptr;
		std::deque< Scene::Transform* > dest_queue = {};
		EnemyType type = EnemyType::DEFAULT;
		uint enemy_list_idx = 0;
	};
	static constexpr uint MaxEnemies = 15;
	PlayMode::Enemy *active_enemies[PlayMode::MaxEnemies] = 
		{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
	std::deque< uint > enemies_to_delete;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//hexapod leg to wobble:
	// Scene::Transform *hip = nullptr;
	// Scene::Transform *upper_leg = nullptr;
	// Scene::Transform *lower_leg = nullptr;
	// glm::quat hip_base_rotation;
	// glm::quat upper_leg_base_rotation;
	// glm::quat lower_leg_base_rotation;
	// float wobble = 0.0f;
	// glm::vec3 get_leg_tip_position();

	Scene::Transform *amogus = nullptr;


	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > leg_tip_loop;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
