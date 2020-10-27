
#include "Connection.hpp"

#include "hex_dump.hpp"

#include <stdexcept>
#include <iostream>
#include <cassert>
#include <unordered_map>
#include <random>
#include <deque>
#include <chrono>
#include <glm/gtc/type_ptr.hpp>

//Tetris board logic is my 112 code translated to cpp

struct tetris {
	std::vector<std::vector<bool>> ipiece {{true, true, true, true}};
	std::vector<std::vector<bool>> lpiece {{false, false, true}, { true, true, true}};
	std::vector<std::vector<bool>> jpiece {{true, false, false}, {true, true, true}};
	std::vector<std::vector<bool>> zpiece {{true, true, false}, {false, true, true}};
	std::vector<std::vector<bool>> spiece {{false, true, true}, {true, true, false}};
	std::vector<std::vector<bool>> tpiece {{false, true, false}, {true, true, true}};
	std::vector<std::vector<bool>> opiece {{true, true}, {true, true}};
	
	std::vector<std::vector<uint8_t>> board;
};
tetris *t = new tetris;
struct block {
	short name;
	float age = 0.0f;
};

void print_board() {
	printf("board");
	for (int i = 0; i < 12; i++) {
		for (int j = 0; j < 10; j++) {
			printf("%d ", t->board[i][j]);
		}
		printf("\n");
	}
}

void print_piece(std::vector<std::vector<bool>> rot) {
	for (size_t i = 0; i < rot.size(); i++) {
		for (size_t j = 0; j < rot[i].size(); j++) {
			if (rot[i][j]) printf("O");
			else printf(" ");
		}
		printf("\n");
	}
}

bool drop_piece(std::vector<std::vector<bool>> piece, uint8_t ind, uint8_t btype) {
	std::vector<int> rows;
    std::vector<int> cols;
    std::vector<int> rowSpaces;
	//printf("row cols:\n");
	int prow = (int)piece.size();
	int pcol = (int)piece[0].size();
	for (int i = 0; i < prow; i++) {
		for (int j = 0; j < pcol; j++) {
			if (piece[i][j]) {
				rows.push_back(i);
                cols.push_back(ind + j);
				//printf("%d %d\n", i, ind + j);
			}
		}
	}
	//printf("rowloops: ");
    for (int i = 0; i < (int)cols.size(); i++) {
		int rowLoop = 0;
		while (rowLoop < 12 && t->board[rowLoop][cols[i]] == 7) {
			rowLoop++;
		}
		//printf("%d ", rowLoop);
		rowSpaces.push_back((rowLoop));
	}
        
    std::vector<int> newList;
    for (int i = 0; i < (int)rows.size(); i++) {
		newList.push_back(rowSpaces[i] - rows[i] - 1);
	}
    int row_place = *(std::min_element(newList.begin(), newList.end()));
	//printf("row place %d\n", row_place);
	//print_piece(piece);
	for (int i = 0; i < prow; i++){
		for (int j = 0; j < pcol; j++) {
			if (row_place + i < 0) {
				return true;
			}
			if (piece[i][j]) t->board[row_place + i][j + ind] = btype;
		}
	}
	return false;
}

short remove_rows() {
	short ret = 0;
	for (int i = 0; i < 12; i++) {
		bool found7 = false;
		for (int j = 0; j < 10; j++) {
			if (t->board[i][j] == 7) {
				found7 = true; //not complete
			}
		}
		if (!found7) { //row complete
			ret++;
			for (int k = i; k > 0; k--) {
				t->board[k] = t->board[k - 1];
			}
		}
	}
	return ret;
}
    
std::vector<std::vector<bool>> rotate_block(std::vector<std::vector<bool>> rot) {
	size_t newcols; 
	size_t newrows;
	size_t oldcols; 
	size_t oldrows;
	newcols = rot.size();
	newrows = rot[0].size();
	oldcols = newrows;
	oldrows = newcols;
	std::vector<std::vector<bool>> newrot;
	for (size_t i = 0; i < newrows; i++) {
		std::vector<bool> vec(newcols, false);
		newrot.push_back(vec);
	}
	for (size_t row = 0; row < oldrows; row++) {
		for (size_t col = 0; col < oldcols; col++) {
			newrot[col][row] = rot[row][(oldcols-1) - col];
		}
	}
	return newrot;
}

std::vector<std::vector<bool>> rotate(uint8_t times, uint8_t type) {
	std::vector<std::vector<bool>> rot;
	if (type == 0) std::copy(t->ipiece.begin(), t->ipiece.end(), std::back_inserter(rot));
	if (type == 1) std::copy(t->jpiece.begin(), t->jpiece.end(), std::back_inserter(rot));
	if (type == 2) std::copy(t->lpiece.begin(), t->lpiece.end(), std::back_inserter(rot));
	if (type == 3) std::copy(t->spiece.begin(), t->spiece.end(), std::back_inserter(rot));
	if (type == 4) std::copy(t->zpiece.begin(), t->zpiece.end(), std::back_inserter(rot));
	if (type == 5) std::copy(t->tpiece.begin(), t->tpiece.end(), std::back_inserter(rot));
	if (type == 6) std::copy(t->opiece.begin(), t->opiece.end(), std::back_inserter(rot));
	//printf("type %d before rotate\n", type);
	//print_piece(rot);
	for (int i = 0; i < times; i++) {
		rot = rotate_block(rot);
	}
	//print_piece(rot);
	return rot;
}

int main(int argc, char **argv) {
#ifdef _WIN32
	//when compiled on windows, unhandled exceptions don't have their message printed, which can make debugging simple issues difficult.
	try {
#endif

	//------------ argument parsing ------------

	if (argc != 2) {
		std::cerr << "Usage:\n\t./server <port>" << std::endl;
		return 1;
	}

	//------------ initialization ------------

	Server server(argv[1]);


	//------------ main loop ------------
	constexpr float ServerTick = 1.0f / 10.0f; //TODO: set a server tick that makes sense for your game

	//server state:
	
	for (uint8_t i = 0; i < 12; i++) {
		std::vector<uint8_t> row(10, 7);
		t->board.push_back(row);
	}
	double gen_block = 0.0;
	double block_timer = 3.0;
	double timer_add = 3.0;
	short num_sent = 10000;
	short block_name;
	bool block_erase = false;
	bool update_board = false;
	float max_age = 6.0f;
	short score = 0;
	int trash = 0;
	int mask = 0x000000ff;
	bool lose = false;
	char erase_type;
	std::deque<block *> blocks;

	//per-client state:
	struct PlayerInfo {
		PlayerInfo() {
			static uint32_t next_player_id = 1;
			name = "Player" + std::to_string(next_player_id);
			next_player_id += 1;
		}
		std::string name;
		int x;
		int y;
	};
	std::unordered_map< Connection *, PlayerInfo > players;

	while (true) {
		static auto next_tick = std::chrono::steady_clock::now() + std::chrono::duration< double >(ServerTick);
		//process incoming data from clients until a tick has elapsed:
		block_erase = false;
		update_board = false;
		auto current_time = std::chrono::high_resolution_clock::now();
		static auto previous_time = current_time;
		float elapsed = std::chrono::duration< float >(current_time - previous_time).count();
		previous_time = current_time;
		elapsed = std::min(0.1f, elapsed);
		gen_block += (double)elapsed;
		for (size_t i = 0; i < blocks.size(); i++) {
			blocks[i]->age += elapsed;
		}
		if (!block_erase && blocks.size() > 0) {
			if (blocks[0]->age > max_age) {
				block_name = blocks[0]->name;
				erase_type = 'o';
				blocks.pop_front();
				block_erase = true;
				if (players.size() > 0) trash++;
				if (trash > 15) {
					lose = true;
				}
			}
		}
		while (true) {
			auto now = std::chrono::steady_clock::now();
			
			double remain = std::chrono::duration< double >(next_tick - now).count();
			//printf("%lf\n",remain);
			//elapsed = std::min(0.1f, elapsed);
			if (remain < 0.0) {
				next_tick += std::chrono::duration< double >(ServerTick);
				break;
			}
			

			server.poll([&](Connection *c, Connection::Event evt){
				if (evt == Connection::OnOpen) {
					//client connected:
					players.emplace(c, PlayerInfo());
					printf("someone connected\n");
					if (players.size() > 2) {
						printf("this is a 2 player game\n");
						c->close();
					}
					update_board = true;
				} else if (evt == Connection::OnClose) {
					//client disconnected:

					//remove them from the players list:
					auto f = players.find(c);
					assert(f != players.end());
					players.erase(f);


				} else {assert(evt == Connection::OnRecv);
					//got data from client:
					//std::cout << "got bytes:\n" << hex_dump(c->recv_buffer); std::cout.flush();

					//look up in players list:
					auto f = players.find(c);
					assert(f != players.end());
					PlayerInfo &player = f->second;
					
					//handle messages from client:
					//TODO: update for the sorts of messages your clients send
					while (c->recv_buffer.size() >= 4) {
						if (lose) break;
						//expecting five-byte messages 'b' (left count) (right count) (down count) (up count)
						char type = c->recv_buffer[0];
						char end = c->recv_buffer.back();
						int total = 0;
						if (end != 'e') {
							std::cout << "receiveed " << end << " didn't receive e" << std::endl;
							return;
						}
						
						if (!(type == 't' || type == 'p' || type == 'm')) {
							std::cout << " message of non-'t' type received from client!" << std::endl;
							//shut down client connection:
							c->close();
							return;
						}
						if (type == 't') {
							block_name = (c->recv_buffer[2] << 8) | c->recv_buffer[1];
							//printf("got block %x %x\n", c->recv_buffer[2], c->recv_buffer[1]);
							for (size_t i = 0; i < blocks.size(); i++) {
								if (blocks[i]->name == block_name) {
									//printf("erasing block\n");
									blocks.erase(blocks.begin() + i);
									block_erase = true;
									erase_type = 't';
									break;
								}
							}
							total += 4;
							if (c->recv_buffer.size() > 4) type = c->recv_buffer[4];
							
						} else if (type == 'p') {
							//printf("dropping piece\n");
							uint8_t block_type = c->recv_buffer[1];
							uint8_t block_ind = c->recv_buffer[2];
							uint8_t block_rot = c->recv_buffer[3];
							auto new_block = rotate(block_rot, block_type);
							lose = drop_piece(new_block, block_ind, block_type);
							//print_board();
							score += remove_rows();
							//print_board();
							update_board = true;
							total += 5;
							if (c->recv_buffer.size() > 4) type = c->recv_buffer[5];
						}
						if (type == 'm') {
							//printf("total is %d\n", total);

							player.x = (((c->recv_buffer[total+4] & mask) << 24) | ((c->recv_buffer[total+3] & mask)  << 16) | ((c->recv_buffer[total+2] & mask)  << 8) | (c->recv_buffer[total+1] & mask));
							player.y = (((c->recv_buffer[total+8] & mask) << 24) | ((c->recv_buffer[total+7] & mask)  << 16) | ((c->recv_buffer[total+6] & mask)  << 8) | (c->recv_buffer[total+5] & mask));
							//printf("x %x %x %x %x\n", c->recv_buffer[total+4], c->recv_buffer[total+3], c->recv_buffer[total+2], c->recv_buffer[total+1]);
							//printf("y %x %x %x %x\n", c->recv_buffer[total+8], c->recv_buffer[total+7], c->recv_buffer[total+6], c->recv_buffer[total+5]);
							//printf("got coords %x %x\n", player.x, player.y);
						}
						c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.end());
					}
				}
			}, remain);
		}

		//update current game state
		//TODO: replace with *your* game state update

		if (lose) {
			for (auto &[c, player] : players) {
				(void)player; 
				c->send('f'); //to pay respects
				c->send(score);
				c->send('e');
			}
			continue;
		}

		for (auto &[c, player] : players) {
			for (auto &[c2, player2] : players) {
				(void)c2;
				if (player.name != player2.name) {
					//printf("send playe coord\n");
					c->send('p');
					c->send(player2.x);
					c->send(player2.y);
					c->send('e');
				}
			}
		}
		if (block_erase) {
			//printf("send erase %d\n", block_name);
			for (auto &[c, player] : players) {
				(void)player; //work around "unused variable" warning on whatever g++ github actions uses
				//send an update starting with 'm', a 24-bit size, and a blob of text:
				c->send('t');
				c->send(erase_type);
				c->send(block_name);
				c->send('e');
			}
			block_erase = false;
		}
		if (gen_block > block_timer) {
			block *b = new block;
			b->name = num_sent;
			//printf("send %d\n", b->name);
			blocks.push_back(b);
			uint8_t btype = rand() % 7;
			for (auto &[c, player] : players) {
				
				(void)player; //work around "unused variable" warning on whatever g++ github actions uses
				//send an update starting with 'm', a 24-bit size, and a blob of text:
				c->send('b');
				
				//printf("send block %d %s\n", btype, b->name.c_str());
				c->send(btype);
				c->send(b->name);
				c->send('e');
			}
			
			block_timer = gen_block + timer_add;
			timer_add -= 0.03;
			num_sent += 1;
		} 
		if (update_board) {
			//printf("send update\n");
			for (auto &[c, player] : players) {
			(void)player; //work around "unused variable" warning on whatever g++ github actions uses
			//send an update starting with 'm', a 24-bit size, and a blob of text:
				c->send('m');
				for (uint8_t i = 0; i < 12; i++) {
					for (uint8_t j = 0; j < 10; j++) {
						c->send(t->board[i][j]);
					}
				}
				c->send('e');
			}
			update_board = false;
		}
	}


	return 0;

#ifdef _WIN32
	} catch (std::exception const &e) {
		std::cerr << "Unhandled exception:\n" << e.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "Unhandled exception (unknown type)." << std::endl;
		throw;
	}
#endif
}
