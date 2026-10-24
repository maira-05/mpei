"""
wav_a_c.py - Convierte un fragmento de una cancion WAV en un arreglo de C
para la prueba de audio (prueba_cancion).

Uso:
    python wav_a_c.py CANCION.WAV            (primeros 18 s)
    python wav_a_c.py CANCION.WAV 30         (empieza en el segundo 30)
    python wav_a_c.py CANCION.WAV 30 15      (empieza en 30 s, dura 15 s)

Genera src/cancion.c con las muestras en 8 bits sin signo, mono, a
22050 Hz (lo que espera main.c). Acepta WAV PCM de 8 o 16 bits, mono o
estereo, a cualquier frecuencia de muestreo.

Solo usa la libreria estandar de Python (no hace falta instalar nada).
"""
import os
import struct
import sys
import wave

FS_SALIDA = 22050          # Hz, debe coincidir con CANCION_FS en main.c
DURACION_MAX = 18.0        # s, ~400 KB: cabe en los 512 KB de FLASH


def leer_wav(ruta):
    with wave.open(ruta, "rb") as w:
        canales = w.getnchannels()
        ancho = w.getsampwidth()
        fs = w.getframerate()
        if w.getcomptype() != "NONE":
            sys.exit("Error: el WAV debe ser PCM sin compresion.")
        datos = w.readframes(w.getnframes())

    if ancho == 1:                                   # 8 bits sin signo
        muestras = [b - 128 for b in datos]
        escala = 128.0
    elif ancho == 2:                                 # 16 bits con signo
        muestras = list(struct.unpack("<%dh" % (len(datos) // 2), datos))
        escala = 32768.0
    else:
        sys.exit("Error: solo se aceptan WAV de 8 o 16 bits.")

    # Pasar a mono promediando canales y normalizar a -1..1
    mono = []
    for i in range(0, len(muestras) - canales + 1, canales):
        mono.append(sum(muestras[i:i + canales]) / (canales * escala))
    return mono, fs


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    ruta = sys.argv[1]
    inicio = float(sys.argv[2]) if len(sys.argv) > 2 else 0.0
    duracion = float(sys.argv[3]) if len(sys.argv) > 3 else DURACION_MAX
    duracion = min(duracion, DURACION_MAX)

    mono, fs = leer_wav(ruta)
    print("WAV leido: %d Hz, %.1f s" % (fs, len(mono) / fs))

    # Remuestrear a 22050 Hz con interpolacion lineal
    n_salida = int(duracion * FS_SALIDA)
    paso = fs / FS_SALIDA
    pos = inicio * fs
    salida = []
    for _ in range(n_salida):
        i = int(pos)
        if i + 1 >= len(mono):
            break
        f = pos - i
        salida.append(mono[i] * (1 - f) + mono[i + 1] * f)
        pos += paso
    if not salida:
        sys.exit("Error: el segundo de inicio esta despues del final de la cancion.")

    # Normalizar el volumen al 90 % y pasar a 8 bits sin signo (0..255)
    pico = max(abs(x) for x in salida) or 1.0
    bytes_u8 = [max(0, min(255, int(round(128 + 127 * 0.9 * x / pico)))) for x in salida]

    destino = os.path.join(os.path.dirname(os.path.abspath(__file__)), "src", "cancion.c")
    with open(destino, "w") as f:
        f.write("/* Generado por wav_a_c.py a partir de %s */\n" % os.path.basename(ruta))
        f.write("/* %d muestras, 8 bits sin signo, mono, %d Hz (%.1f s) */\n"
                % (len(bytes_u8), FS_SALIDA, len(bytes_u8) / FS_SALIDA))
        f.write("#include <stdint.h>\n\n")
        f.write("const uint32_t cancion_len = %d;\n\n" % len(bytes_u8))
        f.write("const uint8_t cancion[] = {\n")
        for i in range(0, len(bytes_u8), 16):
            f.write("  " + ",".join("%3d" % b for b in bytes_u8[i:i + 16]) + ",\n")
        f.write("};\n")
    print("Listo: %s (%.1f s, %d bytes)" % (destino, len(bytes_u8) / FS_SALIDA, len(bytes_u8)))


if __name__ == "__main__":
    main()
