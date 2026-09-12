Nombre: Sebastián Abarza
Rol: 202473601-9
Repositorio Github: https://github.com/Fe26-00/Tarea1-ALGOCO.git

# Documentación

## Entrega

La entrega se realiza vía **aula.usm.cl** en formato `.zip`.

## Instrucciones de ejecución:

### Para Ordenamiento de Arreglos
En la carpeta code/sorting/scripts ejecutar el comando "python3 array_generator.py"
Luego, en la carpeta code/sorting ejecutar los comandos "make clean" y "make run"
Y para general los plots, en la carpeta code/sorting/scripts ejecutar el comando "python3 plot_generator.py"

### Para Multiplicación de Matrices
En la carpeta code/matrix_multiplication/scripts ejecutar el comando "python3 matrix_generator.py"
Luego, en la carpeta code/matrix_multiplication ejecutar los comandos "make clean" y "make run"
Y para general los plots, en la carpeta code/matrix_multiplication/scripts ejecutar el comando "python3 plot_generator.py"


## Multiplicación de matrices
Algoritmos: Naive, Strassen


### Programa principal
Lee cada par de matrices generado, ejecuta ambos algoritmos sobre ellas midiendo tiempo y memoria utilizada, y guarda tanto la matriz resultante como las mediciones obtenidas.


### Scripts
matrix_generator.py
Genera todas las Matrices que serán utilizadas como input para los algoritmos.

plot_generator.py
General los gráficos en la carpeta "plots".

## Ordenamiento de arreglo unidimensional
Algoritmos: MergeSort, QuickSort, PatienceSort, std::sort.

### Programa principal
Lee cada arreglo generado, ejecuta los cuatro algoritmos de ordenamiento sobre él midiendo tiempo y memoria utilizada, y guarda tanto el arreglo ordenado como las mediciones obtenidas.


### Scripts
matrix_generator.py
Genera todos los arreglos que serán utilizadas como input para los algoritmos.

plot_generator.py
General los gráficos en la carpeta "plots".
