#ifndef DEL_PLAYER_H
#define DEL_PLAYER_H

#include "bh.h"
#include "states.h"

class CDelPlayer : public State {
	void Enter();
	void Loop();
	void Keyb_spec(SDL_Keysym sym, bool release);
	void Mouse(int button, int state, int x, int y);
	void Motion(int x, int y);
public:
	int player;
};

extern CDelPlayer DelPlayer;

#endif
