make:
	g++ -c main.cpp Servo.cpp Klient.cpp -I /home/pi/2.1/include
	g++ main.o Servo.o Klient.o -o app.e -L /home/pi/2.1/lib -lsfml-system -lsfml-network

launch:
	export LD_LIBRARY_PATH=/home/pi/2.1/lib && ./app.e
