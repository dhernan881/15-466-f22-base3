#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdlib>
#include <iostream>

GLuint amogus_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > amogus_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("amogus.pnct"));
	amogus_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > amogus_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("amogus.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name) {
		Mesh const &mesh = amogus_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = amogus_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;
	});
});

Load< Sound::Sample > rap_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("among_us_rap.opus"));
});

Load< Sound::Sample > fart_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("fart_sound_effect.wav"));
});

Load< Sound::Sample > laser_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("laser_sound_effect.wav"));
});

Load< Sound::Sample > pew_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("pew_sound_effect.wav"));
});

PlayMode::PlayMode() : scene(*amogus_scene) {
	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	//start music loop playing:
	// (note: position will be over-ridden in update())
	// leg_tip_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, glm::vec3(0.0f), 10.0f);
	music_loop = Sound::loop(*rap_sample);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_1) {
			one.downs += 1;
			one.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_2) {
			two.downs += 1;
			two.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_3) {
			three.downs += 1;
			three.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_1) {
			one.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_2) {
			two.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_3) {
			three.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

// Quaternion lookat function from https://stackoverflow.com/a/49824672
glm::quat safe_quat_lookat(glm::vec3 const &fromPos, glm::vec3 const &toPos, 
		glm::vec3 const &rotateAround = glm::vec3(0.0f, 0.0f, 1.0f)) {
	glm::vec3 dir = toPos - fromPos;
	float dir_len = glm::length(dir);

	// Make sure dir is valid
	if (!(dir_len > 0.0001f)) {
		return glm::quat(1, 0, 0, 0); // identity
	}

	dir /= dir_len;
	
	// quatLookAt requires that dir is *not* parallel to the axis to rotate around.
	if (glm::abs(glm::dot(dir, rotateAround)) > 0.9999f) {
		return glm::quat(1, 0, 0, 0); // identity (I don't expect this to happen tbh)
	}
	return glm::quatLookAt(dir, rotateAround);
}

void PlayMode::try_spawn_enemy() {
	for (uint i = 0; i < PlayMode::MaxEnemies; i++) {
		if (active_enemies[i] != nullptr)
			continue;
		
		// From https://en.cppreference.com/w/cpp/numeric/random/rand
		std::srand(std::time(nullptr));
		uint type_int = (std::rand() % NumEnemyTypes);
		uint dir_int = std::rand() % 2;
		std::cout << "type_int: " << type_int << "dir_int: " << dir_int << std::endl;

		Enemy *new_enemy = new Enemy {};
		Scene::Transform *new_trans = new Scene::Transform();
		new_trans->name = "Astronaut" + std::to_string(i);
		scene.drawables.emplace_back(new_trans);
		Scene::Drawable &drawable = scene.drawables.back();
		drawable.pipeline = lit_color_texture_program_pipeline;
		drawable.pipeline.vao = amogus_meshes_for_lit_color_texture_program;

		// Duplicate code cuz idk the proper way to do this otherwise
		switch (type_int) {
			case 0: {
				const Mesh& mesh = amogus_meshes->lookup("Amogus.Red");
				new_enemy->enemy_type = EnemyType::RED;
				drawable.pipeline.type = mesh.type;
				drawable.pipeline.start = mesh.start;
				drawable.pipeline.count = mesh.count;
				break; }
			case 1: {
				const Mesh& mesh = amogus_meshes->lookup("Amogus.Green");
				new_enemy->enemy_type = EnemyType::GREEN;
				drawable.pipeline.type = mesh.type;
				drawable.pipeline.start = mesh.start;
				drawable.pipeline.count = mesh.count;
				break; }
			case 2: {
				const Mesh& mesh = amogus_meshes->lookup("Amogus.Yellow");
				new_enemy->enemy_type = EnemyType::YELLOW;
				drawable.pipeline.type = mesh.type;
				drawable.pipeline.start = mesh.start;
				drawable.pipeline.count = mesh.count;
				break; }
			default:
				std::cerr << "Invalid random enemy int: " << type_int << std::endl;
				return;
		}
		new_enemy->transform = new_trans;
		new_enemy->transform->position = glm::vec3(0.0f, 12.0f, 2.12f);
		switch (dir_int) {
			case 0:
				new_enemy->dest_queue.push(glm::vec3(10.0f,15.0f,2.12f));
				break;
			case 1:
				new_enemy->dest_queue.push(glm::vec3(-10.0f,15.0f,2.12f));
				break;
			default:
				std::cerr << "Invalid random enemy direction int: " << dir_int << std::endl;
				return;
		}
		new_enemy->dest_queue.push(camera->transform->position);
		new_enemy->enemy_list_idx = i;
		active_enemies[i] = new_enemy;
		return;
	}
}

void PlayMode::Enemy::update(float elapsed) {
	// update position
	// TODO: add popping from dest queue and updating dest and looking at next dest

	glm::vec3 dest = dest_queue.front();
	glm::vec3 move_dir = dest - transform->position;
	move_dir = glm::normalize(move_dir);
	transform->position = transform->position + (move_dir * Enemy::MoveSpeed * elapsed);
	// do this every frame for now
	// TODO: don't do this every frame if necessary
	transform->rotation = safe_quat_lookat(transform->position, dest);

	if (glm::distance(transform->position, dest) < 0.5f &&
			dest_queue.size() > 1) {
		dest_queue.pop();
	}
}

PlayMode::Enemy *PlayMode::get_closest_enemy() {
	float closest_dist = 100000000.0f;
	Enemy *closest_enemy = nullptr;
	for (uint i = 0; i < PlayMode::MaxEnemies; i++) {
		Enemy *cur_enemy = active_enemies[i];
		if (cur_enemy == nullptr)
			continue;
		float cur_dist = glm::distance(cur_enemy->transform->position,
			camera->transform->position);
		if (cur_dist < closest_dist) {
			closest_dist = cur_dist;
			closest_enemy = cur_enemy;
		}
	}
	return closest_enemy;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	// wobble += elapsed / 10.0f;
	// wobble -= std::floor(wobble);

	// hip->rotation = hip_base_rotation * glm::angleAxis(
	// 	glm::radians(5.0f * std::sin(wobble * 2.0f * float(M_PI))),
	// 	glm::vec3(0.0f, 1.0f, 0.0f)
	// );
	// upper_leg->rotation = upper_leg_base_rotation * glm::angleAxis(
	// 	glm::radians(7.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))),
	// 	glm::vec3(0.0f, 0.0f, 1.0f)
	// );
	// lower_leg->rotation = lower_leg_base_rotation * glm::angleAxis(
	// 	glm::radians(10.0f * std::sin(wobble * 3.0f * 2.0f * float(M_PI))),
	// 	glm::vec3(0.0f, 0.0f, 1.0f)
	// );

	// //move sound to follow leg tip position:
	// leg_tip_loop->set_position(get_leg_tip_position(), 1.0f / 60.0f);

	//move camera:
	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 frame_forward = -frame[2];

		camera->transform->position += move.x * frame_right + move.y * frame_forward;
	}

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_at = frame[3];
		Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
	}

	// update enemies:
	{
		enemy_spawn_timer += elapsed;
		if (enemy_spawn_timer >= PlayMode::EnemySpawnDelay) {
			try_spawn_enemy();
			enemy_spawn_timer -= PlayMode::EnemySpawnDelay;
		}
		for (auto &enemy : active_enemies) {
			if (enemy != nullptr)
				enemy->update(elapsed);
		}
	}

	// check if shooting:
	// Fortunately, my enemies are color-coded so it's pretty easy to implement
	// a VERY VERY VERY BASIC shooting system.
	// From: https://en.m.wikibooks.org/wiki/OpenGL_Programming/Object_selection
	{
		shoot_timer += elapsed;
		if (shoot_timer >= PlayMode::ShootDelay && (one.pressed ||
				two.pressed || three.pressed)) {
			if (one.pressed) {
				Sound::play(*laser_sample);
			} else if (two.pressed) {
				Sound::play(*pew_sample);
			} else if (three.pressed) {
				Sound::play(*fart_sample);
			}
			shoot_timer = 0.0f;
			float pixel[4];
			EnemyType hit_enemy_type;
			glReadPixels(screen_dims.x / 2, screen_dims.y / 2, 
				1, 1, GL_RGBA, GL_FLOAT, &pixel);
			
			// Check for the color of the closest guy.
			if (pixel[0] >= 4 * pixel[1] && pixel[0] >= 4 * pixel[2]) {
				std::cout << "red guy detected" << std::endl;
				hit_enemy_type = RED;
			} else if (pixel[1] >= 4 * pixel[0] && pixel[1] >= 4 * pixel[2]) {
				std::cout << "green guy detected" << std::endl;
				hit_enemy_type = GREEN;
			} else if (pixel[0] >= 4 * pixel[2] && pixel[1] >= 4 * pixel[2]) {
				std::cout << "yellow guy detected" << std::endl;
				hit_enemy_type = YELLOW;
			} else {
				std::cout << "unable to determine color of guy at center of screen" << std::endl;
				hit_enemy_type = DEFAULT;
			}
			Enemy *closest_enemy = get_closest_enemy();
			if (closest_enemy != nullptr && 
					closest_enemy->enemy_type == hit_enemy_type) {
				std::cout << "The color of the closest guy == color that the camera is looking at" << std::endl;
				// The color of the closest guy is equal to the color that
				// the camera is looking at.
				if (one.pressed && !two.pressed && !three.pressed &&
						hit_enemy_type == RED) {
					// Correctly hit a red guy while pressing the red key.
					enemies_to_delete.push(closest_enemy->enemy_list_idx);
				} else if (!one.pressed && two.pressed && !three.pressed &&
						hit_enemy_type == GREEN) {
					// Correctly hit a green guy while pressing the green key.
					enemies_to_delete.push(closest_enemy->enemy_list_idx);
				} else if (!one.pressed && !two.pressed && three.pressed &&
						hit_enemy_type == YELLOW) {
					// Correctly hit a yellow guy while pressing the yellow key.
					enemies_to_delete.push(closest_enemy->enemy_list_idx);
				}
			}
		}
		// TODO: delete enemies that need to be deleted
		for (; !enemies_to_delete.empty(); enemies_to_delete.pop()) {
			uint enemy_idx = enemies_to_delete.front();
			// Very bad idea: just move the mesh out of sight.
			active_enemies[enemy_idx]->transform->position = glm::vec3(0.0f, 0.0f, -10.0f);
			delete active_enemies[enemy_idx];
			active_enemies[enemy_idx] = nullptr;
		}
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	one.downs = 0;
	two.downs = 0;
	three.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	screen_dims = drawable_size; // is there a better way to do this?
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		// draw crosshair? ask later.
		// lines.draw_text("+",
		// 	glm::vec3(float(drawable_size.x) / 2.0f, float(drawable_size.y) / 2.0f, 0.0),
		// 	glm::vec3(0.0f), glm::vec3(0.0f),
		// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00));
	}
	GL_ERRORS();
}
