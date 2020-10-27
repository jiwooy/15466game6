#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "Mesh.hpp"
#include "Load.hpp"
#include "DrawLines.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "hex_dump.hpp"
#include <string>

#include <glm/gtc/type_ptr.hpp>

#include <random>

using namespace std;
PlayMode::~PlayMode() {
}

GLuint tetris_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > tetris_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("tetris.pnct"));
	tetris_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > tetris_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("tetris.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = tetris_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = tetris_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

PlayMode::PlayMode(Client &client_) : client(client_) {
	scene = Scene(*tetris_scene);
	for (auto drawable : scene.drawables) {
		if (drawable.transform->name == "Cylinder") {
			p = &drawable;
			pTrans = drawable.transform;
			pPipe = drawable.pipeline;
		} else if (drawable.transform->name == "Placer") {
			placer = drawable.transform;
		} else if (drawable.transform->name == "IBlock") {
			bdraw[0].d = &drawable;
			bdraw[0].t = drawable.transform;
			bdraw[0].p = drawable.pipeline;
		} else if (drawable.transform->name == "IPiece") {
			pdraw[0].d = &drawable;
			pdraw[0].t = drawable.transform;
			pdraw[0].p = drawable.pipeline;
		} else if (drawable.transform->name == "LBlock") {
			bdraw[1].d = &drawable;
			bdraw[1].t = drawable.transform;
			bdraw[1].p = drawable.pipeline;
		} else if (drawable.transform->name == "LPiece") {
			pdraw[1].d = &drawable;
			pdraw[1].t = drawable.transform;
			pdraw[1].p = drawable.pipeline;
		}  else if (drawable.transform->name == "JBlock") {
			bdraw[2].d = &drawable;
			bdraw[2].t = drawable.transform;
			bdraw[2].p = drawable.pipeline;
		} else if (drawable.transform->name == "JPiece") {
			pdraw[2].d = &drawable;
			pdraw[2].t = drawable.transform;
			pdraw[2].p = drawable.pipeline;
		}  else if (drawable.transform->name == "ZBlock") {
			bdraw[3].d = &drawable;
			bdraw[3].t = drawable.transform;
			bdraw[3].p = drawable.pipeline;
		} else if (drawable.transform->name == "ZPiece") {
			pdraw[3].d = &drawable;
			pdraw[3].t = drawable.transform;
			pdraw[3].p = drawable.pipeline;
		}  else if (drawable.transform->name == "SBlock") {
			bdraw[4].d = &drawable;
			bdraw[4].t = drawable.transform;
			bdraw[4].p = drawable.pipeline;
		} else if (drawable.transform->name == "SPiece") {
			pdraw[4].d = &drawable;
			pdraw[4].t = drawable.transform;
			pdraw[4].p = drawable.pipeline;
		}  else if (drawable.transform->name == "TBlock") {
			bdraw[5].d = &drawable;
			bdraw[5].t = drawable.transform;
			bdraw[5].p = drawable.pipeline;
		} else if (drawable.transform->name == "TPiece") {
			pdraw[5].d = &drawable;
			pdraw[5].t = drawable.transform;
			pdraw[5].p = drawable.pipeline;
		}  else if (drawable.transform->name == "OBlock") {
			bdraw[6].d = &drawable;
			bdraw[6].t = drawable.transform;
			bdraw[6].p = drawable.pipeline;
		} else if (drawable.transform->name == "OPiece") {
			pdraw[6].d = &drawable;
			pdraw[6].t = drawable.transform;
			pdraw[6].p = drawable.pipeline;
		}
	}

	camera = &scene.cameras.front();
	camera->transform->position = pTrans->position;
	camera->transform->position.z += 2.0f;
}


bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.repeat) {
			//ignore repeats
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
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
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
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		rotate_block();
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
		}
	}


	return false;
}

void PlayMode::update(float elapsed) {

	//queue data for sending to server:
	//TODO: send something that makes sense for your game
	constexpr float PlayerSpeed = 8.0f;
	glm::vec2 move = glm::vec2(0.0f);
	if (left.pressed && !right.pressed) move.x =-1.0f;
	if (!left.pressed && right.pressed) move.x = 1.0f;
	if (!down.pressed && up.pressed) move.y =-1.0f;
	if (down.pressed && !up.pressed) move.y = 1.0f;
	if (space.pressed) {
		if (held == nullptr) {
			if (pickup()) {
				client.connections.back().send('t'); //t for take 
				client.connections.back().send(held->name);
				client.connections.back().send('e');
			}
		}
		else {
			uint8_t ind = place_block();
			if (ind != 69) {
				client.connections.back().send('p'); //p for place 
				client.connections.back().send(held->btype); //type of block
				client.connections.back().send(ind); //index to place
				client.connections.back().send(held->rotation); //index to place
				client.connections.back().send('e');
				held = nullptr;
			}
		}
	}
	if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;
	glm::vec3 remain = camera->transform->make_local_to_world() * glm::vec4(move.x, 0.0f, move.y, 0.0f);
	camera->transform->position.x += remain.x;
	//printf("%f %f %f\n", remain.x, remain.y, remain.z);
	camera->transform->position.y += remain.y;
	if (held != nullptr) {
		held->t->position.x += remain.x;
		//printf("%f %f %f\n", remain.x, remain.y, remain.z);
		held->t->position.y += remain.y;
	}
	move_belt();
	

	//send/receive data:
	client.poll([this](Connection *c, Connection::Event event){
		if (event == Connection::OnOpen) {
			std::cout << "[" << c->socket << "] opened" << std::endl;
		} else if (event == Connection::OnClose) {
			std::cout << "[" << c->socket << "] closed (!)" << std::endl;
			throw std::runtime_error("Lost connection to server!");
		} else { assert(event == Connection::OnRecv);
			//std::cout << "[" << c->socket << "] recv'd data. Current buffer:\n" << hex_dump(c->recv_buffer); std::cout.flush();
			//expecting message(s) like 'm' + 3-byte length + length bytes of text:
			int total = 0;
			while (c->recv_buffer.size() >= 4) {
				//std::cout << "[" << c->socket << "] recv'd data. Current buffer:\n" << hex_dump(c->recv_buffer); std::cout.flush();
				char type = c->recv_buffer[0];
				char end = c->recv_buffer.back();
				//printf("%c %c %zd\n", type, end, c->recv_buffer.size());
				if (!(type == 'm' || type == 'b' || type == 't')) {
					throw std::runtime_error("Server sent unknown message type '" + std::to_string(type) + "'");
				}
				if (end != 'e') break;
				if (type == 't') {
					short name = (c->recv_buffer[3] << 8) | c->recv_buffer[2];
					char erase_type = c->recv_buffer[1];
					receive_erase(name, erase_type);
					total += 5; // 't' plus 2
					if (c->recv_buffer.size() > total) type = c->recv_buffer[total];
				}
				if (type == 'b') {
					printf("received bliock\n");
					uint8_t block_type = c->recv_buffer[total + 1] - '0' - 208;
					short name = (c->recv_buffer[total + 3] << 8) | c->recv_buffer[total + 2];
					//short name = (short)stoi(n);
					printf("name is %d\n", name);
					total += 5;
					gen_block(block_type, name);
					if (c->recv_buffer.size() > total) type = c->recv_buffer[total];
				}
				if (type == 'm') {
					//whole message *is* here, so set current server message:
					printf("received update\n");
					server_message = std::string(c->recv_buffer.begin() + 1 + total, c->recv_buffer.begin() + 121 + total);
					//printf("sizeee %s\n", server_message.c_str());
					//total += 121; // 'm' + 120
					//type = c->recv_buffer[total];
					get_board();
				} 
				
				c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.end());
			}
		}
	}, 0.0);
}

void PlayMode::rotate_block() {
	if (held == nullptr) return;
	//printf("rotating\n");
	held->rotation = (held->rotation + 1) % 4;
	size_t newcols; 
	size_t newrows;
	size_t oldcols; 
	size_t oldrows;
	newcols = held->rot.size();
	newrows = held->rot[0].size();
	oldcols = newrows;
	oldrows = newcols;
	std::vector<std::vector<bool>> newrot;
	for (size_t i = 0; i < newrows; i++) {
		vector<bool> vec(newcols, false);
		newrot.push_back(vec);
	}
	for (size_t row = 0; row < oldrows; row++) {
		for (size_t col = 0; col < oldcols; col++) {
			newrot[col][row] = held->rot[row][(oldcols-1) - col];
		}
	}
	held->rot = newrot;
	//printf("done\n");
}

void PlayMode::gen_block(uint8_t btype, short name) {
	Scene::Drawable *new_block = new Scene::Drawable();
	new_block->pipeline = bdraw[btype].p;

	Scene::Transform *t = new Scene::Transform;
	t->rotation = bdraw[btype].t->rotation;
	t->position = bdraw[btype].t->position;
	t->scale = bdraw[btype].t->scale;
	t->name = std::to_string(name);
	//t->parent = player.transform;
	scene.transforms.emplace_back(*t);

	new_block->transform = t;
	scene.drawables.emplace_back(*new_block);

	active_blocks *belt_block = new active_blocks;
	belt_block->btype = btype;
	belt_block->name = name;
	belt_block->t = t;
	belt.push_back(belt_block);
}

void PlayMode::gen_piece(uint8_t ptype, size_t row, size_t col) {
	//printf("gen piece\n");
	Scene::Drawable *new_piece = new Scene::Drawable();
	new_piece->pipeline = pdraw[ptype].p;

	Scene::Transform *t = new Scene::Transform;
	t->rotation = pdraw[ptype].t->rotation;
	t->position = pdraw[ptype].t->position;
	t->position.z = 22.0f - (row * 2);
	t->position.y = -9.0f + (col * 2);
	t->scale = pdraw[ptype].t->scale;
	//t->parent = player.transform;
	scene.transforms.emplace_back(*t);

	new_piece->transform = t;
	scene.drawables.emplace_back(*new_piece);
	pieces[row][col] = new_piece;
	//printf("done gen\n");
}

void PlayMode::move_belt() {
	for (size_t i = 0; i < belt.size(); i++) {
		belt[i]->t->position.y -= 0.1f;
	}
}

void PlayMode::delete_belt() {
	for (size_t i = 0; i < belt.size(); i++) {
		if (belt[i]->t->position.y < -14.5f) {
			belt[i]->t->scale = glm::vec3(0.0f, 0.0f, 0.0f);
			belt.erase(belt.begin() + i);
		}
	}
}

uint8_t PlayMode::place_block() {
	if (std::max(placer->position[0] - 3.0f, camera->transform->position[0] - 1.0f) <= std::min(placer->position[0] + 3.0f, camera->transform->position[0] + 1.0f) &&
		std::max(placer->position[1], camera->transform->position[1] - 1.0f) <= std::min(placer->position[1] + 18.0f, camera->transform->position[1] + 1.0f)) {
		size_t est = (int)((camera->transform->position.y + 10.0f) / 2.0f);
		size_t half = (size_t)((held->rot[0].size()) / 2);
		est -= half;
		if (est > (10 - held->rot[0].size())) est = 10 - held->rot[0].size();
		if (est < 0) est = 0;
		printf("place %d %zd\n", (uint8_t)est, est);
		return (uint8_t)est;
	}
	printf("fail place\n");
	return 69;
}

bool PlayMode::pickup() {
	for (size_t i = 0; i < belt.size(); i++) {
		if (std::max(belt[i]->t->position[0] - 6.0f, camera->transform->position[0] - 1.0f) <= std::min(belt[i]->t->position[0] + 6.0f, camera->transform->position[0] + 1.0f) &&
			std::max(belt[i]->t->position[1] - 4.0f, camera->transform->position[1] - 1.0f) <= std::min(belt[i]->t->position[1] + 4.0f, camera->transform->position[1] + 1.0f)) {
			//belt[i]->t->parent = camera->transform;
			holding *h = new holding;
			h->t = belt[i]->t;
			h->btype = belt[i]->btype;
			h->name = belt[i]->name;
			if (h->btype == 0) {
				printf("pick i");
				std::vector<std::vector<bool>> piece{{true, true, true, true}};
				h->rot = piece;
			}
			if (h->btype == 1) {
				printf("pick j");
				std::vector<std::vector<bool>> piece{{true, false, false}, {true, true, true}};
				h->rot = piece;
			}
			if (h->btype == 2) {
				printf("pick l");
				std::vector<std::vector<bool>> piece{{false, false, true}, { true, true, true}};
				h->rot = piece;
			}
			if (h->btype == 3) {
				printf("pick s");
				std::vector<std::vector<bool>> piece{{false, true, true}, {true, true, false}};
				h->rot = piece;
			}
			if (h->btype == 4) {
				printf("pick z");
				std::vector<std::vector<bool>> piece{{true, true, false}, {false, true, true}};
				h->rot = piece;
			}
			if (h->btype == 5) {
				printf("pick t");
				std::vector<std::vector<bool>> piece{{false, true, false}, {true, true, true}};
				h->rot = piece;
			}
			if (h->btype == 6) {
				printf("pick o");
				std::vector<std::vector<bool>> piece{{true, true}, {true, true}};
				h->rot = piece;
			}
			held = h;
			printf("done pickup %d\n", h->name);
			return true;
		}
	}
	return false;
}

void PlayMode::receive_erase(short name, char erase_type) {
	printf("erasing %d\n", name);
	for (size_t i = 0; i < belt.size(); i++) {
		if (belt[i]->name == name) {
			if (erase_type == 't') {
				belt[i]->t->position = camera->transform->position;
				belt[i]->t->position.z = camera->transform->position.z + 3.0f;
				belt[i]->t->scale.x *= 0.2f;
				belt[i]->t->scale.y *= 0.2f;
				belt[i]->t->scale.z *= 0.2f;
			} else if (erase_type == 'o') {
				belt[i]->t->scale = glm::vec3(0.0f, 0.0f, 0.0f);
			}
			belt.erase(belt.begin() + i);
			printf("erased\n");
			return;
		}
	}
}

void PlayMode::get_board() {
	size_t rowcount = 0;
	size_t colcount = 0;
	//printf("gettinb board\n");
	// for (size_t i = 0; i < server_message.size(); i++) {
	// 	uint8_t ptype = server_message[i] - '0' - 208;
	// 	printf("%d ", ptype);
	// 	colcount++;
	// 	if (colcount == 10) {
	// 		rowcount++;
	// 		printf("\n");
	// 		if (rowcount == 12) return;
	// 		colcount = 0;
	// 	}
	// }

	// rowcount = 0;
	// colcount = 0;
	for (size_t i = 0; i < server_message.size(); i++) {
		uint8_t ptype = server_message[i] - '0' - 208;
		//printf("done %zd %zd\n", rowcount, colcount);
		if (ptype >= 0 && ptype < 7) {
			if (pieces[rowcount][colcount] == 0) {
				gen_piece(ptype, rowcount, colcount);
			}
			else {
				//printf("replace %p\n", pieces[rowcount][colcount]);
				pieces[rowcount][colcount]->pipeline = pdraw[ptype].p;
			}
		}
		colcount++;
		if (colcount == 10) {
			rowcount++;
			if (rowcount == 12) return;
			colcount = 0;
		}
	}
	//printf("done %zd %zd\n", rowcount, colcount);
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	GL_ERRORS();
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	GL_ERRORS();
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	GL_ERRORS();
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

		auto draw_text = [&](glm::vec2 const &at, std::string const &text, float H) {
			lines.draw_text(text,
				glm::vec3(at.x, at.y, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0x00, 0x00, 0x00, 0x00));
			float ofs = 2.0f / drawable_size.y;
			lines.draw_text(text,
				glm::vec3(at.x + ofs, at.y + ofs, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		};

		//draw_text(glm::vec2(-aspect + 0.1f, 0.0f), server_message, 0.09f);

		if (held != nullptr) {
			
			std::string display = "Held:";
			draw_text(glm::vec2(-aspect + 0.1f,-0.5f), display, 0.09f);
			
			//printf("%zd, %zd\n", held->rot.size(),  held->rot[0].size());
			for (size_t i = 0; i < held->rot.size(); i++) {
				std::string show;
				for (size_t j = 0; j < held->rot[i].size(); j++) {
					//printf("%zd, %zd\n", i, j);
					if (held->rot[i][j]) show = show + "O";
					else show = show + " ";
				}
				draw_text(glm::vec2(-aspect + 0.1f,-0.5f - (i+1)*0.1 ), show, 0.09f);
			}
		}
		
	}
	GL_ERRORS();
}
