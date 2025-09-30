# Asteroids en C con SDL3

Este es un clon del clásico juego de arcade "Asteroids", desarrollado en C utilizando la biblioteca SDL3.

## Características

*   Movimiento clásico de la nave con inercia.
*   Asteroides que se dividen en fragmentos más pequeños al ser destruidos.
*   OVNIs enemigos (grandes y pequeños) que disparan al jugador.
*   Power-ups: Escudo y Disparo Triple.
*   Sistema de puntuación y guardado de la puntuación más alta (`highscore.txt`).
*   Efectos visuales como partículas para explosiones y fondo de estrellas con paralaje.
*   Dificultad que aumenta progresivamente.
*   Efecto de "sacudida de pantalla" (Screen Shake) al ser destruido.
*   Hiperespacio para escapar de situaciones peligrosas.

## Requisitos

Para compilar y ejecutar este proyecto, necesitarás:

*   Un compilador de C (como GCC o Clang).
*   La biblioteca **SDL3**.
*   La biblioteca **SDL3_ttf** (para renderizar texto).

## Compilación

El proyecto incluye un `Makefile` para facilitar la compilación en sistemas tipo Unix (Linux, macOS).

1.  **Clona el repositorio:**
    ```bash
    git clone https://github.com/TU_USUARIO/TU_REPOSITORIO.git
    cd TU_REPOSITORIO
    ```

2.  **Compila el juego:**
    Asegúrate de que las bibliotecas SDL3 y SDL3_ttf estén instaladas en tu sistema. Luego, simplemente ejecuta `make`:
    ```bash
    make
    ```
    Esto generará un ejecutable llamado `asteroids`.

## Ejecución

Una vez compilado, puedes ejecutar el juego desde la terminal:

```bash
./asteroids
```

Asegúrate de que el archivo de fuente `Press_Start_2P.ttf` esté en el mismo directorio que el ejecutable.

## Controles

*   **Flechas Arriba/Izquierda/Derecha** o **W/A/D**: Acelerar y girar la nave.
*   **Espacio**: Disparar.
*   **Shift Izquierdo**: Activar Hiperespacio.
*   **P** o **Escape**: Pausar el juego.
*   **F11**: Activar/Desactivar pantalla completa.