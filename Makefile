#定义变量保存gcc,arm-linux-gcc
CC=arm-linux-gcc

ELF=test_V1.1

CONFIG=-I./ -I./libjpeg -L./libjpeg -ljpeg -lapi_v4l2_arm

SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c, %.o, $(SRCS))

$(ELF):$(OBJS)
	$(CC) -o $@ $^ -lpthread $(CONFIG)
	# cp test_V1.1 /home/abc/nfs_server

%.o:%.c
	$(CC) -c $< -o $@ $(CONFIG)

clean:
	rm  test_V1.1 *.o $(TARGET)
