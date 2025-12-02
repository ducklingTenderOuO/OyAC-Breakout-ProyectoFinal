# 🧱 Breakout - Proyecto Final (OyAC)

Un clon clásico del juego **Breakout** desarrollado en **C** puro utilizando la librería gráfica **SDL3**, con una característica única: **toda la lógica crítica del juego (física, colisiones, máquina de estados y ordenamiento) está implementada en Lenguaje Ensamblador x86 (Inline ASM).**

Este proyecto fue desarrollado como parte de la materia de Organización y Arquitectura de Computadoras.

## 📸 Capturas de Pantalla

### Menú Principal
<img width="1400" height="936" alt="image" src="https://github.com/user-attachments/assets/d775fcd7-6d9b-4f67-a6c3-1df760f834e3" />

*Vista del menú con opciones de Jugar, Puntuaciones y Salir*

### Gameplay - Nivel 1
<img width="1395" height="934" alt="image" src="https://github.com/user-attachments/assets/417fdcf2-a4c6-4e23-9b21-334744a326d8" />

*Vista de la paleta, la pelota y los ladrillos en el primer nivel*

### Tabla de Puntuaciones (High Scores)
<img width="1399" height="936" alt="image" src="https://github.com/user-attachments/assets/8cb7fb3f-4ffb-4f6c-912c-c692fdab4af6" />

*Pantalla mostrando los mejores jugadores ordenados*

---

## 🚀 Características Principales

* **Motor Híbrido C/ASM:** Integración de bloques `__asm` dentro de C para el manejo de alto rendimiento.
* **Física en Ensamblador:** Cálculos de rebote, velocidad y trayectoria de la pelota realizados a bajo nivel.
* **Máquina de Estados en ASM:** Control de flujo del juego (Menú -> Juego -> Pausa -> Game Over) gestionado mediante registros y saltos.
* **Algoritmo de Ordenamiento en ASM:** Implementación de *Bubble Sort* en ensamblador para ordenar la tabla de puntuaciones al guardar un nuevo récord.
* **Sistema de Niveles:** 10 niveles con patrones de mapas únicos definidos por matrices.
* **Mecánicas Avanzadas:**
    * Resistencia de ladrillos (golpes múltiples) a partir del nivel 6.
    * Vidas extra al alcanzar puntuaciones específicas.
    * Rebote dinámico aleatorio ("Factor Caos") para evitar patrones repetitivos.
* **Estética Retro:** Uso de fuentes tipográficas arcade y renderizado de corazones mediante primitivas geométricas (Pixel Art).
* **Persistencia de Datos:** Guardado y lectura de los mejores puntajes en archivo binario (`scores.dat`).

## 🛠️ Tecnologías Utilizadas

* **Lenguaje:** C (Estándar C11/C17).
* **Arquitectura:** x86 (32-bits). *Nota: El proyecto debe compilarse en x86 debido al uso de Inline ASM de MSVC.*
* **Gráficos/Input:** [SDL3](https://libsdl.org/) (Simple DirectMedia Layer 3).
* **Texto:** [SDL3_ttf](https://github.com/libsdl-org/SDL_ttf).
* **IDE Recomendado:** Visual Studio 2022 (con herramientas de compilación C++).

## 🎮 Controles

| Tecla | Acción |
| :--- | :--- |
| **Flecha Izquierda** | Mover Paddle a la izquierda |
| **Flecha Derecha** | Mover Paddle a la derecha |
| **Enter** | Seleccionar / Pausar / Guardar Récord |
| **Esc** | Salir / Regresar al Menú / Cancelar |
| **Tab** | Ver Mejores Puntuaciones (Solo en Menú) |

## 🔧 Instalación y Compilación

Para ejecutar este proyecto en tu entorno local:

1.  **Clonar el repositorio:**
    ```bash
    git clone [https://github.com/TU_USUARIO/oyac-breakout-proyectofinal.git](https://github.com/TU_USUARIO/oyac-breakout-proyectofinal.git)
    ```
2.  **Configurar Visual Studio:**
    * Abre el archivo `.sln`.
    * Asegúrate de que la plataforma de solución esté configurada en **x86** (Debug o Release). El ensamblador *inline* de MSVC no funciona en x64.
3.  **Dependencias SDL3:**
    * Asegúrate de que las carpetas `include` y `lib` de SDL3 y SDL3_ttf estén correctamente vinculadas en las propiedades del proyecto:
        * *C/C++ -> General -> Directorios de inclusión adicionales.*
        * *Vinculador -> General -> Directorios de bibliotecas adicionales.*
        * *Vinculador -> Entrada -> Dependencias adicionales:* `SDL3.lib`, `SDL3_ttf.lib`.
4.  **Archivos Runtime:**
    * Copia los archivos `SDL3.dll`, `SDL3_ttf.dll` y `RETRO.TTF` (o la fuente que estés usando) en la misma carpeta donde se genera el ejecutable (`/x86/Debug` o `/x86/Release`).
5.  **Compilar y Ejecutar.**

## 🧩 Estructura del Código Relevante

El núcleo del proyecto reside en `BreakOut.c`. Aquí algunos puntos de interés para revisión de código:

* `OrdenamientoPuntajesASM()`: Implementación manual de ordenamiento de estructuras en memoria.
* `bucle principal -> __asm`: Máquina de estados que gestiona `ESTADO_MENU`, `ESTADO_JUGANDO`, etc.
* `bucle principal -> física`: Bloques de movimiento y detección de colisiones AABB (Axis-Aligned Bounding Box) utilizando registros de la FPU (`fld`, `fcomp`, `fstp`).

## 👥 Autores

* **Astrid Jimenez**
* **Erick Moya**

---
*Proyecto realizado con fines académicos para demostrar la integración de lenguajes de alto y bajo nivel.*
