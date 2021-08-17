PKGS=libpng libwebp
CFLAGS=`pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)`

drop: src/main.c
	$(CC) ./src/main.c $(CFLAGS) $(LIBS) -o drop
