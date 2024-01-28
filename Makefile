all: nclient nserver

nserver:
	g++ Server/server.cpp -o server -lsqlite3
nclient:
	g++ Client/client1.cpp -o client 
nview:
	g++ Test_database/viewtable.cpp -o view -lsqlite3
ndtb:
	g++ Test_database/dtb.cpp -o dtb -lsqlite3

