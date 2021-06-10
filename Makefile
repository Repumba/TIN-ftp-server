make:
	g++ -c main.cpp Servo.cpp Klient.cpp
	g++ main.o Servo.o Klient.o -o app.e -lsfml-system -lsfml-network -lpthread

launch:
	./app.e
