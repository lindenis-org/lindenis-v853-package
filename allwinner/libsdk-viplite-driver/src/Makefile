CC  = $(TINA_TOP)/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-musl/toolchain/bin/arm-openwrt-linux-muslgnueabi-gcc
CPP = $(TINA_TOP)/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-musl/toolchain/bin/arm-openwrt-linux-muslgnueabi-g++

all:
	${CPP} body_face.c -I./ -I./../sdk_release/sdk -I./../algo_aw -L./../algo_aw -L./../sdk_release -lawnn_det -lVIPlite -lVIPuser -lm -lc -lsupc++ -o body_face

clean:
	rm -fr *.o body_face
