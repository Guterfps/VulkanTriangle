CFLAGS = -std=c++17 -pedantic-errors -Wall -Wextra
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

STB_INCLUDE_PATH = ../

.PHONY: clean debug release test

debug: CFLAGS += -g
debug: app
test:
	./VulkanTest.out

release: CFLAGS += -DNDEBUG -O3
release: app

app: main.cpp
	g++ $(CFLAGS) -o VulkanTest.out main.cpp $(LDFLAGS) -I$(STB_INCLUDE_PATH)


clean:
	rm -f VulkanTest.out