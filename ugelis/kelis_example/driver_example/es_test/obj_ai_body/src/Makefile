obj-y += module/streamfifo.o
obj-y += main_code_tinyyolo.o
obj-y += shell/ai_shell.o
obj-y += lcd/lcd.o
obj-y += lcd/draw.o
obj-y += math/math.o
obj-y += math/tan_atan.o
obj-y += camera/camera.o
obj-y += main.o
allincs = $(shell ls $(srctree)/src/inc)
all_inc = $(addprefix -I$(srctree)/src/inc/, $(allincs))
ccflags-y += $(all_inc)
