# Projekt Webserver - Woche 4


**Abgabe: 10.12.19 04:00**
Die Abgabe ist der letzte Commit vor der Abgabe der master branch.

## Überblick
Diese Woche entfernen wir die Limitierungen des Webservers indem wir dynamischen
Speicher nutzen, anstatt der bisherigen konstanten Puffer.


## String Append
Eine naive Methode wäre einen Buffer zu allozieren und wenn der String größer
wird als der Buffer, einen neuen, größeren zu erstellen und den bisherigen String
zu kopieren. Oft ist dies auch optimal, allerdings benutzt unser Webserver
Strings in einer besonderen Art und Weise.
Der Webserver baut immer wieder Strings, indem er neue Strings anhängt.
So wird z.B. nach und nach der Responseheader gebaut. Anfangs wissen wir
jedoch nicht wie groß der Responseheader werden kann, er muss also dynamisch
vergrößert werden. (In unserem Beispiel allerdings kaum verkleinert.)
Gelöscht oder innerhalb des Strings geändert wird allerdings nicht, wir brauchen
also nur eine append Operation.
Eine geeignete Datenstruktur ist eine einfach verkettete Liste von Strings, da
der komplette String lediglich am Ende gebraucht wird.
Implementieren Sie diese in `stringlist.c` nach den Anforderungen von `stringlist.h`.
Es sollen alle Tests in `test_stringlist.c` bestanden werden, ohne Fehler oder
Warnungen von `valgrind`. (Ignorieren Sie eventuelle Ausgaben durch getaddrinfo.)

Um die Unittests in `test_stringlist.c` zu kompilieren, nutzen Sie
`make unittest` und rufen `test` auf.
Da wir zwei verschiedene Programme durch die gleiche Makefile erstellen, passen
Sie die Makefile ggf. an, falls Sie `.c` Dateien hinzufügen.

## Dynamische Speichergrößen
Der Webserver soll außerdem keine Grenze in den übertragenden Dateien haben.
Auch die Anzahl der Header-Felder und deren Inhaltsgröße soll variabel sein.
Passen Sie den Server dementsprechend an. Die dazugehörigen Tests sind in `test04.sh`.
