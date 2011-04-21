PROJECT_NAME := rush2
PACKAGES += rhizome
LDFLAGS += -lfreenect
SRC += \
	kinect.c \
	kinect_display.c \
	main.c \
	main_scene.c \
	noise.c \
	obj.c \
	player.c \
	player_camera.c \
	ship.c \
	terrain_manager.c \
	terrain_renderer.c
