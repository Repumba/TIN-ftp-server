# TIN-ftp-server

## Kompilacja oraz uruchomienie

Kompilacja w środowisku Linux/Unix (g++): <br>
  `make`

Uruchomienie w środowisku Linux/Unix (g++): <br>
  `make launch`
  
***Ważne*** Przed uruchomieniem polecenia `make` oraz `make launch` należy zedytować plik `Makefile` oraz wpisać lokalizację bibliotek SFML jako wartość zmiennej SFMLpath.

Kompilacja oraz uruchomienie w środowisku Windows bardzo zależy od używanego kompilatora. W celu skonfigorowania kompilacji oraz uruchomienia, polecam skorzystać z tutoriali dostępnych na stronie SFML (https://www.sfml-dev.org/).

## Połączenie z serwerem ftp
Na potrzeby projektu postawiłem własny serwer ftp na RaspberryPi. Dane serwera: <br>
`IP:` 94.75.112.77 <br>
`Port:` 20000 <br>

W celu zalogowania się konieczne są login oraz hasło (utworzenie danych logowania możliwe poprzez kontakt z administratorem).

## Poruszanie się po serwerze

#### Dostepne polecenia:
* `a` - wyswietl zawartosc folderu
* `b` - pobranie pliku z serwera
* `c` - wyslanie pliku na serwer
* `d` - usuniecie pliku
* `e` - utworzenie folderu
* `f` - zmienienie aktualnego folderu
* `g` - zablokowanie pliku
* `h` - odblokowanie pliku
* `x` - zakoncz polaczenie

## Instalacja SFML:
https://www.sfml-dev.org/tutorials/2.5/
