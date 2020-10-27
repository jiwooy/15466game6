#include "Mode.hpp"

#include "Connection.hpp"
#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct block_draw {
	Scene::Drawable *d = nullptr;
	Scene::Transform *t = nullptr;
	Scene::Drawable::Pipeline p;
};

struct active_blocks {
	uint8_t btype;
	short name;
	Scene::Transform *t = nullptr;
};

struct holding {
	uint8_t btype;
	short name;
	Scene::Transform *t = nullptr;
	uint8_t rotation = 0; //default
	std::vector<std::vector<bool>> rot;
};

struct tetris {
	std::vector<std::vector<bool>> ipiece {{true, true, true, true}};
	std::vector<std::vector<bool>> jpiece {{true, false, false}, {true, true, true}};
	std::vector<std::vector<bool>> lpiece {{false, false, true}, { true, true, true}};
	std::vector<std::vector<bool>> opiece {{true, true}, {true, true}};
	std::vector<std::vector<bool>> spiece {{false, true, true}, {true, true, false}};
	std::vector<std::vector<bool>> zpiece {{true, true, false}, {false, true, true}};
	std::vector<std::vector<bool>> tpiece {{false, true, false}, {true, true, true}};
};


struct PlayMode : Mode {
	PlayMode(Client &client);
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;
	virtual void gen_block(uint8_t btype, short name);
	void move_belt();
	void delete_belt();
	bool pickup();
	uint8_t place_block();
	void rotate_block();
	void get_board();
	void gen_piece(uint8_t ptype, size_t row, size_t col);
	void receive_erase(short name, char erase_type);
	void boundaries();

	//----- game state -----
	
	block_draw bdraw[7]; //store block data
	block_draw pdraw[7]; //store piece data

	std::deque<active_blocks *> belt;

	Scene::Drawable *pieces[12][10] = {{0}};
	
	int mask = 0x000000ff;
	bool lost = false;
	short score;

	Scene::Drawable *p = nullptr;
	Scene::Transform *pTrans = nullptr;
	Scene::Drawable::Pipeline pPipe;

	Scene::Transform *placer = nullptr;

	holding *held = nullptr;
	tetris tet;

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space;

	//the tetris board
	std::string server_message;

	//connection to server:
	Client &client;
	Scene scene;
	Scene::Camera *camera = nullptr;

};
