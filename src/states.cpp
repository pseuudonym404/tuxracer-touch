/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tuxracer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "states.h"
#include "ogl.h"
#include "winsys.h"
#include <ctime>
#include <ubuntu/application/sensors/accelerometer.h>

static UASensorsAccelerometer *accel = NULL;
static float accelx = 0;
static float accely = 0;
static float accelz = 0;

void tilt_cb(UASAccelerometerEvent *event, void*/*context*/) {
	float value = 0;
	if (uas_accelerometer_event_get_acceleration_x(event, &value) == U_STATUS_SUCCESS) accelx = value;
	if (uas_accelerometer_event_get_acceleration_y(event, &value) == U_STATUS_SUCCESS) accely = value;
	if (uas_accelerometer_event_get_acceleration_z(event, &value) == U_STATUS_SUCCESS) accelz = value;
}

State::Manager State::manager(Winsys);

State::Manager::~Manager() {
	if (accel)
		ua_sensors_accelerometer_disable(accel);
	if (current)
		current->Exit();
}

void State::Manager::Run(State& entranceState) {
	SDL_setenv("UBUNTU_PLATFORM_API_BACKEND", "touch_mirclient", 1);
	accel = ua_sensors_accelerometer_new();
	ua_sensors_accelerometer_set_reading_cb(accel, tilt_cb, 0);
	ua_sensors_accelerometer_enable(accel);

	current = &entranceState;
	current->Enter();
	//clock_t ticks = clock();
	while (!quit) {
		PollEvent();
		Tilt();
		if (next)
			EnterNextState();
		CallLoopFunction();
		if (param.framerate != 0) {
			/**clock_t nticks = clock();
			int32_t sleeptime = (CLOCKS_PER_SEC/param.framerate - (nticks-ticks)) / 1000;
			if (sleeptime > 0)
				SDL_Delay(sleeptime);*/
			SDL_Delay(1);
			//ticks = nticks;
		}
	}
	current->Exit();
	previous = current;
	current = NULL;
}

void State::Manager::EnterNextState() {
	current->Exit();
	previous = current;
	current = next;
	next = NULL;
	current->Enter();
}

void State::Manager::Tilt() {
	float x, y;

	if (!next) {
		switch (Winsys.orient) {
			default:
				x = accelx * -1;
				y = accely;
				break;
			case 1:
				x = accely;
				y = accelx;
				break;
			case 2:
				x = accelx;
				y = accely * -1;
				break;
			case 3:
				x = accely * -1;
				y = accelx * -1;
				break;
		}
		x = (x / 4) * (param.sensit / 10.0f);
		y = (y / 4) - 1.8f;
		if (y > 0.0) y *= 2;
		y *= (param.sensit / 10.0f);
		//printf("%f %f %f\n", x, y, accelz);
		current->Jaxis(0, x);
		current->Jaxis(1, y);
		if (accelz > 12) {
			current->Jbutt(0, 1);
			current->Jbutt(0, 0);
		}
	}
}

void State::Manager::PollEvent() {
	SDL_Event event;
	unsigned int key;
	TVector2i cur, old;

	while (!next && SDL_PollEvent (&event)) {
		switch (event.type) {
			case SDL_KEYDOWN:
				SDL_GetMouseState(&cur.x, &cur.y);
				key = event.key.keysym.sym;
				current->Keyb(key, key >= 256, false, cur.x, cur.y);
				current->Keyb_spec(event.key.keysym, false);
				break;

			case SDL_KEYUP:
				SDL_GetMouseState(&cur.x, &cur.y);
				key = event.key.keysym.sym;
				current->Keyb(key, key >= 256, true, cur.x, cur.y);
				current->Keyb_spec(event.key.keysym, true);
				break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				current->Mouse(event.button.button, event.button.state, event.button.x, event.button.y);
				break;

			case SDL_MOUSEMOTION:
				old = cursor_pos;
				cursor_pos.x = event.motion.x;
				cursor_pos.y = event.motion.y;
				current->Motion(event.motion.x-old.x, event.motion.y-old.y);
				break;

			case SDL_FINGERDOWN:
			case SDL_FINGERUP:
				if (Winsys.orient & ORIENT_ROTATE) {
					cur.x = event.tfinger.y * Winsys.resolution.width;
					cur.y = event.tfinger.x * Winsys.resolution.height;
					if (Winsys.orient & ORIENT_MIRROR) {
						cur.x = Winsys.resolution.width - cur.x;
					} else {
						cur.y = Winsys.resolution.height - cur.y;
					}
				} else {
					cur.x = event.tfinger.x * Winsys.resolution.width;
					cur.y = event.tfinger.y * Winsys.resolution.height;
					if (Winsys.orient & ORIENT_MIRROR) {
						cur.x = Winsys.resolution.width - cur.x;
						cur.y = Winsys.resolution.height - cur.y;
					}
				}

				cursor_pos.x = cur.x;
				cursor_pos.y = cur.y;

				current->Mouse(SDL_BUTTON_LEFT, event.type == SDL_FINGERDOWN ? SDL_PRESSED : SDL_RELEASED, cur.x, cur.y);
				break;

			case SDL_FINGERMOTION:
				old = cursor_pos;

				if (Winsys.orient & ORIENT_ROTATE) {
					cur.x = event.tfinger.y * Winsys.resolution.width;
					cur.y = event.tfinger.x * Winsys.resolution.height;
					if (Winsys.orient & ORIENT_MIRROR) {
						cur.x = Winsys.resolution.width - cur.x;
					} else {
						cur.y = Winsys.resolution.height - cur.y;
					}
				} else {
					cur.x = event.tfinger.x * Winsys.resolution.width;
					cur.y = event.tfinger.y * Winsys.resolution.height;
					if (Winsys.orient & ORIENT_MIRROR) {
						cur.x = Winsys.resolution.width - cur.x;
						cur.y = Winsys.resolution.height - cur.y;
					}
				}

				cursor_pos.x = cur.x;
				cursor_pos.y = cur.y;

				current->Motion(cur.x - old.x, cur.y - old.y);
				break;

			case SDL_JOYAXISMOTION:
				if (Winsys.joystick_isActive()) {
					unsigned int axis = event.jaxis.axis;
					if (axis < 2) {
						float val = (float)event.jaxis.value / 32768.f;
						current->Jaxis(axis, val);
					}
				}
				break;
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				if (Winsys.joystick_isActive()) {
					current->Jbutt(event.jbutton.button, event.jbutton.state);
				}
				break;

			/*case SDL_VIDEORESIZE:
				if (Winsys.resolution.width != event.resize.w || Winsys.resolution.height != event.resize.h) {
					Winsys.resolution.width = event.resize.w;
					Winsys.resolution.height = event.resize.h;
					Winsys.SetupVideoMode (param.res_type);
					Reshape(event.resize.w, event.resize.h);
				}
				break;*/

			case SDL_QUIT:
				quit = true;
				break;
		}
	}
}

void State::Manager::CallLoopFunction() {
	float cur_time = SDL_GetTicks() * 1.e-3;
	g_game.time_step = cur_time - clock_time;
	if (g_game.time_step < 0.0001) g_game.time_step = 0.0001;
	clock_time = cur_time;
	current->Loop();
}
