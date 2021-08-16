PKGS=libpng libwebp
CFLAGS=`pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)`

drop: src/main.cpp
	$(CXX) ./src/main.cpp $(CFLAGS) $(LIBS) -o drop
