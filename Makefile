SFMLpath = /home/pi/2.1

make:
	g++ -c main.cpp Servo.cpp Client.cpp sha256.cpp -I $(SFMLpath)/include
	g++ main.o Servo.o Client.o sha256.o -o app.e -L $(SFMLpath)/lib -lsfml-system -lsfml-network -lpthread

launch:
	export LD_LIBRARY_PATH=$(SFMLpath)/lib && ./app.e
