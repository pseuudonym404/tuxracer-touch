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

#include "bh.h"
#include "textures.h"
#include "ogl.h"
#include "splash_screen.h"
#include "audio.h"
#include "font.h"
#include "tools.h"
#include "ogl_test.h"
#include "winsys.h"
#include <iostream>
#include <ctime>
#include <dbus/dbus.h>

TGameData g_game;

void InitGame (int argc, char **argv) {
	g_game.toolmode = NONE;
	g_game.argument = 0;
	if (argc == 4) {
		string group_arg = argv[1];
		if (group_arg == "--char") g_game.argument = 4;
		Tools.SetParameter(argv[2], argv[3]);
	} else if (argc == 2) {
		string group_arg = argv[1];
		if (group_arg == "9") g_game.argument = 9;
	}

	g_game.player = NULL;
	g_game.start_player = 0;
	g_game.course = NULL;
	g_game.mirrorred = false;
	g_game.character = NULL;
	g_game.location_id = 0;
	g_game.light_id = 0;
	g_game.snow_id = 0;
	g_game.cup = 0;
	g_game.theme_id = 0;
	g_game.force_treemap = 0;
	g_game.treesize = 3;
	g_game.treevar = 3;
}

// ====================================================================
// 					main
// ====================================================================

#if defined ( OS_WIN32_MINGW )
#undef main
#endif

int main( int argc, char **argv ) {
	//easier gdb attach
	usleep(2000000);
	
	// ****************************************************************
	cout << "\n----------- Extreme Tux Racer " ETR_VERSION_STRING " ----------------";
	cout << "\n----------- (C) 2010-2013 Extreme Tuxracer Team  --------\n\n";

	int dispreq = -1;

	DBusError err;
	dbus_error_init(&err);
	DBusConnection *conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
	if (dbus_error_is_set(&err)) {
		cerr << "Failed to get system bus\n";
		dbus_error_free(&err);
		if (conn) {
			dbus_connection_unref(conn);
			conn = NULL;
		}
	} else {
		dbus_connection_set_exit_on_disconnect(conn, 0);

		DBusMessage *msg = dbus_message_new_method_call("com.canonical.Unity.Screen", "/com/canonical/Unity/Screen", "com.canonical.Unity.Screen", "keepDisplayOn");
		if (msg != NULL) {
			DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, 300, NULL);
			if (reply) {
				if (!dbus_message_get_args(reply, NULL, DBUS_TYPE_INT32, &dispreq, DBUS_TYPE_INVALID)) dispreq = -1;
				dbus_message_unref(reply);
			}
			dbus_message_unref(msg);
		}

		if (dispreq == -1) cerr << "Failed to request backlight stay on\n";
	}

	srand (time (NULL));
	InitConfig (argv[0]);
	InitGame (argc, argv);
	Winsys.Init ();
	InitOpenglExtensions ();
	BuildGlobalVBO();

	// for checking the joystick and the OpgenGL version (the info is
	// written on the console):
	//	Winsys.PrintJoystickInfo ();
	//	PrintGLInfo ();

	// theses resources must or should be loaded before splashscreen starts
	Tex.LoadTextureList ();
	FT.LoadFontlist ();
	Winsys.SetFonttype ();
	Audio.Open ();
	Music.LoadMusicList ();
	Music.SetVolume (param.music_volume);

	switch (g_game.argument) {
		case 0:
			State::manager.Run(SplashScreen);
			break;
		case 4:
			g_game.toolmode = TUXSHAPE;
			State::manager.Run(Tools);
			break;
		case 9:
			State::manager.Run(OglTest);
			break;
	}

	if (conn) {
		if (dispreq != -1) {
			DBusMessage *msg = dbus_message_new_method_call("com.canonical.Unity.Screen", "/com/canonical/Unity/Screen", "com.canonical.Unity.Screen", "removeDisplayOnRequest");
			dbus_message_append_args(msg, DBUS_TYPE_INT32, &dispreq, DBUS_TYPE_INVALID);
			if (msg != NULL) {
				if (dbus_connection_send(conn, msg, NULL)) dbus_connection_flush(conn);
				dbus_message_unref(msg);
			}
		}
		dbus_connection_close(conn);
		dbus_connection_unref(conn);
		dbus_shutdown();
	}

	Winsys.Quit();
	DeleteGlobalVBO();

	return 0;
}
