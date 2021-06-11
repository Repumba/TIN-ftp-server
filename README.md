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
W przypadku chęci testów na innym komputerze, należy wprowadzić odpowiedni adres IP. Numer portu jest określony w kodzie (w pliku main.cpp), w przypadku chęci zmiany, konieczna jest lekka modyfikacja kodu. <br>

W celu zalogowania się konieczne są login, hasło oraz utworzenie folderu na serwerze o takiej samej nazwie, jak nazwa użytkownika.
Utworzenie konta możliwe poprzez kontakt z administratorem serwera.

## Poruszanie się po serwerze

#### Dostepne polecenia:
* `a` - wyświetl zawartość folderu
* `b` - pobranie pliku z serwera
* `c` - wysłanie pliku na serwer
* `d` - usunięcie pliku
* `e` - utworzenie folderu
* `f` - zmienienie aktualnego folderu
* `g` - zablokowanie pliku
* `h` - odblokowanie pliku
* `x` - zakończ połączenie

Zablokowanie oraz odblokowanie pliku są dostępne, aby dało się przetestować te funkcjonalności. Zakładamy, że w wersji release, zakładanie oraz zdejmowanie blokad, byłoby wykonywanie wyłącznie przez serwer.

## Instalacja SFML:
Nasz projekt napisaliśmy w oparciu o wersję SFML 2.1, dlatego też zalecamy użycie tej samej.
https://www.sfml-dev.org/tutorials/2.5/
https://www.sfml-dev.org/download/sfml/2.1/
