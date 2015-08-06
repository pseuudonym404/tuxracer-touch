# This universal Makefile is prepared for different platforms. 
# Please comment out the related flags. The platforms are:
# 
# OS_Linux 
# OS_Win32_MINGW - Windows, for compiling with mingw (unix-like code)
# OS_WIN32_NATIVE - Windows, for native windows compilers like visual c++
# OS_MAC

#ARCHPATH = arm-linux-gnueabihf
#CC = arm-linux-gnueabihf-g++

ARCHPATH = i386-linux-gnu
INSTPATH = /home/laurie/Software/Touch/SDL/install.i386
CC = i686-linux-gnu-g++

CFLAGS = -Wall -Wextra -Wno-unused-parameter -O1 -g -DUSE_GLES1 -fsingle-precision-constant -I/usr/include/freetype2 -I/usr/include/dbus-1.0 -I/usr/lib/$(ARCHPATH)/dbus-1.0/include -I$(INSTPATH)/include -I./src
LDFLAGS = -L/usr/lib/$(ARCHPATH) -L$(INSTPATH)/lib -lGLESv1_CM -lSDL2 -lSDL2_image -lSDL2_mixer -lfreetype -lm -lstdc++ -ldl -lubuntu_application_api -ldbus-1

# ----------------- Linux ---------------------------------------------
#CFLAGS = -Wall -O2 -DOS_LINUX -I/usr/include/freetype2
#LDFLAGS = -lGL -lGLU -lSDL -lSDL_image -lSDL_mixer -lfreetype 

# ----------------- Windows with mingw --------------------------------
# CFLAGS = -Wall -O2 -DOS_WIN32_MINGW -mwindows -I/usr/include/freetype2
# LDFLAGS = -lfreeglut -lopengl32 -lGLU32 -lSDL -lSDL_image -lSDL_mixer -lfreetype

# ----------------- Windows, erins mingw environment ;-) --------------
# CFLAGS = -Wall -O2 -DOS_WIN32_MINGW -Ic:/mingw/include/freetype2
# LDFLAGS = -Lc:/mingw/lib/sdl -lmingw32 -mwindows -lSDLmain -lSDL -lopengl32 -lglu32 \
# -l:SDL_image.lib -l:SDL_mixer.lib -lfreetype

# ----------------- Windows native ------------------------------------
# CFLAGS = -Wall -O2 -DOS_WIN32_NATIVE .....
# LDFLAGS = .....

# ----------------- Windows with Microsoft compiler -------------------
# CFLAGS = -Wall -O2 -DOS_WIN32_MSC .....
# LDFLAGS = .....

# ----------------- MAC OS --------------------------------------------
# CFLAGS = -Wall -O2 -DOS_MAC .....
# LDFLAGS = .....

# CC = g++
BIN = tuxracer
OBJ = main.o game_config.o ogl.o tux.o audio.o winsys.o \
particles.o mathlib.o splash_screen.o intro.o racing.o \
game_over.o paused.o reset.o game_type_select.o event_select.o \
race_select.o credits.o loading.o course.o keyframe.o env.o event.o \
spx.o common.o course_render.o game_ctrl.o physics.o \
track_marks.o hud.o view.o gui.o translation.o tools.o \
quadtree.o font.o ft_font.o textures.o help.o regist.o tool_frame.o \
tool_char.o newplayer.o score.o ogl_test.o \
config_screen.o states.o vectors.o matrices.o \
opengles.o delplayer.o

$(BIN) : $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(LDFLAGS) $(CFLAGS)

clean:
	rm -f $(BIN) $(OBJ)

# use this template and rename it if you want to add a module

# mmmm.o : mmmm.cpp mmmm.h
#	$(CC) -c mmmm.cpp $(CFLAGS)

delplayer.o : src/delplayer.cpp src/delplayer.h
	$(CC) -c src/delplayer.cpp $(CFLAGS)

ogl_test.o : src/ogl_test.cpp src/ogl_test.h
	$(CC) -c src/ogl_test.cpp $(CFLAGS)

score.o : src/score.cpp src/score.h
	$(CC) -c src/score.cpp $(CFLAGS)

newplayer.o : src/newplayer.cpp src/newplayer.h
	$(CC) -c src/newplayer.cpp $(CFLAGS)

tool_char.o : src/tool_char.cpp src/tool_char.h
	$(CC) -c src/tool_char.cpp $(CFLAGS)

tool_frame.o : src/tool_frame.cpp src/tool_frame.h
	$(CC) -c src/tool_frame.cpp $(CFLAGS)

regist.o : src/regist.cpp src/regist.h
	$(CC) -c src/regist.cpp $(CFLAGS)

tools.o : src/tools.cpp src/tools.h
	$(CC) -c src/tools.cpp $(CFLAGS)

help.o : src/help.cpp src/help.h
	$(CC) -c src/help.cpp $(CFLAGS)

translation.o : src/translation.cpp src/translation.h
	$(CC) -c src/translation.cpp $(CFLAGS)

physics.o : src/physics.cpp src/physics.h
	$(CC) -c src/physics.cpp $(CFLAGS)

winsys.o : src/winsys.cpp src/winsys.h
	$(CC) -c src/winsys.cpp $(CFLAGS)

game_ctrl.o : src/game_ctrl.cpp src/game_ctrl.h
	$(CC) -c src/game_ctrl.cpp $(CFLAGS)

textures.o : src/textures.cpp src/textures.h
	$(CC) -c src/textures.cpp $(CFLAGS)

ft_font.o : src/ft_font.cpp src/ft_font.h
	$(CC) -c src/ft_font.cpp $(CFLAGS)

font.o : src/font.cpp src/font.h
	$(CC) -c src/font.cpp $(CFLAGS)

event.o : src/event.cpp src/event.h
	$(CC) -c src/event.cpp $(CFLAGS)

gui.o : src/gui.cpp src/gui.h
	$(CC) -c src/gui.cpp $(CFLAGS)

common.o : src/common.cpp src/common.h
	$(CC) -c src/common.cpp $(CFLAGS)

spx.o : src/spx.cpp src/spx.h
	$(CC) -c src/spx.cpp $(CFLAGS)

quadtree.o : src/quadtree.cpp src/quadtree.h
	$(CC) -c src/quadtree.cpp $(CFLAGS)

view.o : src/view.cpp src/view.h
	$(CC) -c src/view.cpp $(CFLAGS)

hud.o : src/hud.cpp src/hud.h
	$(CC) -c src/hud.cpp $(CFLAGS)

track_marks.o : src/track_marks.cpp src/track_marks.h
	$(CC) -c src/track_marks.cpp $(CFLAGS)

course_render.o : src/course_render.cpp src/course_render.h
	$(CC) -c src/course_render.cpp $(CFLAGS)

env.o : src/env.cpp src/env.h
	$(CC) -c src/env.cpp $(CFLAGS)

keyframe.o : src/keyframe.cpp src/keyframe.h
	$(CC) -c src/keyframe.cpp $(CFLAGS)

course.o : src/course.cpp src/course.h
	$(CC) -c src/course.cpp $(CFLAGS)

loading.o : src/loading.cpp src/loading.h
	$(CC) -c src/loading.cpp $(CFLAGS)

credits.o : src/credits.cpp src/credits.h
	$(CC) -c src/credits.cpp $(CFLAGS)

race_select.o : src/race_select.cpp src/race_select.h
	$(CC) -c src/race_select.cpp $(CFLAGS)

event_select.o : src/event_select.cpp src/event_select.h
	$(CC) -c src/event_select.cpp $(CFLAGS)

game_type_select.o : src/game_type_select.cpp src/game_type_select.h
	$(CC) -c src/game_type_select.cpp $(CFLAGS)

game_over.o : src/game_over.cpp src/game_over.h
	$(CC) -c src/game_over.cpp $(CFLAGS)

paused.o : src/paused.cpp src/paused.h
	$(CC) -c src/paused.cpp $(CFLAGS)

reset.o : src/reset.cpp src/reset.h
	$(CC) -c src/reset.cpp $(CFLAGS)

racing.o : src/racing.cpp src/racing.h
	$(CC) -c src/racing.cpp $(CFLAGS)

intro.o : src/intro.cpp src/intro.h
	$(CC) -c src/intro.cpp $(CFLAGS)

splash_screen.o : src/splash_screen.cpp src/splash_screen.h
	$(CC) -c src/splash_screen.cpp $(CFLAGS)

mathlib.o : src/mathlib.cpp src/mathlib.h
	$(CC) -c src/mathlib.cpp $(CFLAGS)

particles.o : src/particles.cpp src/particles.h
	$(CC) -c src/particles.cpp $(CFLAGS)

audio.o : src/audio.cpp src/audio.h
	$(CC) -c src/audio.cpp $(CFLAGS)

tux.o : src/tux.cpp src/tux.h
	$(CC) -c src/tux.cpp $(CFLAGS)

ogl.o : src/ogl.cpp src/ogl.h
	$(CC) -c src/ogl.cpp $(CFLAGS)

game_config.o : src/game_config.cpp src/game_config.h
	$(CC) -c src/game_config.cpp $(CFLAGS)

config_screen.o : src/config_screen.cpp src/config_screen.h
	$(CC) -c src/config_screen.cpp $(CFLAGS)

states.o : src/states.cpp src/states.h
	$(CC) -c src/states.cpp $(CFLAGS)

matrices.o : src/matrices.cpp src/matrices.h
	$(CC) -c src/matrices.cpp $(CFLAGS)

vectors.o : src/vectors.cpp src/vectors.h
	$(CC) -c src/vectors.cpp $(CFLAGS)

#eglport.o : src/eglport.cpp src/eglport.h
#	$(CC) -c src/eglport.cpp $(CFLAGS)

opengles.o : src/opengles.cpp 
	$(CC) -c src/opengles.cpp $(CFLAGS)


main.o : src/main.cpp src/bh.h src/etr_types.h
	$(CC) -c src/main.cpp $(CFLAGS)

