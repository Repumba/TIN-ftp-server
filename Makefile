SFMLpath = /home/pi/2.1

make:
	g++ -c main.cpp Servo.cpp Klient.cpp -I $(SFMLpath)/include
	g++ main.o Servo.o Klient.o -o app.e -L $(SFMLpath)/lib -lsfml-system -lsfml-network -lpthread

launch:
	export LD_LIBRARY_PATH=$(SFMLpath)/lib && ./app.e
