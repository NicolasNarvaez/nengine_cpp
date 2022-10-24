DEFFLAGS = -D NENGINE_LLRENDERER_VENDOR_VULKAN
CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

main_test: main.cpp
	g++ $(CFLAGS) -o main_test main.cpp Core/*.cpp $(LDFLAGS) ${DFLAGS}

.PHONY: test clean

test: main_test
	./main_test

clean:
	rm -f main_test

