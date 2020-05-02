# Cartography

Second LAP practical project (P02).
<br>The aim of this project is to write a C program to work with data from the [CAOP](http://www.dgterritorio.pt/cartografia_e_geodesia/cartografia/carta_administrativa_oficial_de_portugal_caop/) database

### Compiling
```
$ mkdir build, lib
$ gcc -c src/Cartography.c -o build/Cartography.o
$ ar rcs lib/libcartography.a build/Cartography.o
$ gcc src/Main.c -L -lcartography -o bin/Main
```

### Running

```
$ ./bin/Main
```

## Authors

* **João Bordalo** - *Initial work* - [jbordalo](https://github.com/jbordalo)
* **Jacinta Sousa** - *Initial work* - [jacintinha](https://github.com/jacintinha)

