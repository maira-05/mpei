"""
revisar_wavs.py - Revisa en el PC que las canciones de la microSD esten en
el formato que va a usar el reproductor.

Uso (con la microSD conectada al PC):
    python revisar_wavs.py E:\\          (Windows, E: = letra de la tarjeta)
    python revisar_wavs.py /media/sd    (Linux / Mac)

Para cada .WAV muestra: formato, bits, canales, frecuencia y duracion, y
avisa si algo no coincide con lo recomendado:
    PCM, 16 bits, mono, 22050 Hz, nombre corto 8.3 sin tildes ni espacios.
Solo usa la libreria estandar de Python.
"""
import os
import re
import struct
import sys

RECOMENDADO = {"formato": 1, "bits": 16, "canales": 1, "fs": 22050}
NOMBRE_83 = re.compile(r"^[A-Z0-9_]{1,8}\.WAV$")


def leer_encabezado(ruta):
    with open(ruta, "rb") as f:
        if f.read(4) != b"RIFF":
            return None, "no empieza con RIFF"
        f.read(4)
        if f.read(4) != b"WAVE":
            return None, "no es WAVE"
        info, datos = None, None
        while True:
            cab = f.read(8)
            if len(cab) < 8:
                break
            nombre, tam = cab[:4], struct.unpack("<I", cab[4:])[0]
            if nombre == b"fmt ":
                fmt = f.read(tam)
                formato, canales, fs = struct.unpack("<HHI", fmt[:8])
                bits = struct.unpack("<H", fmt[14:16])[0]
                info = {"formato": formato, "canales": canales, "fs": fs, "bits": bits}
            elif nombre == b"data":
                datos = tam
                break
            else:
                f.seek(tam + (tam & 1), 1)
        if info is None or datos is None:
            return None, "falta el chunk fmt o data"
        info["bytes_audio"] = datos
        return info, None


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    carpeta = sys.argv[1]
    wavs = sorted(n for n in os.listdir(carpeta) if n.lower().endswith(".wav"))
    if not wavs:
        sys.exit("No se encontraron .wav en " + carpeta)

    problemas = 0
    for n in wavs:
        info, err = leer_encabezado(os.path.join(carpeta, n))
        if err:
            print("[X] %-20s %s" % (n, err))
            problemas += 1
            continue
        bps = info["fs"] * info["canales"] * info["bits"] // 8
        dur = info["bytes_audio"] / bps if bps else 0
        avisos = []
        if info["formato"] != 1:
            avisos.append("no es PCM (formato %d)" % info["formato"])
        if info["bits"] != RECOMENDADO["bits"]:
            avisos.append("%d bits" % info["bits"])
        if info["canales"] != RECOMENDADO["canales"]:
            avisos.append("%d canales" % info["canales"])
        if info["fs"] != RECOMENDADO["fs"]:
            avisos.append("%d Hz" % info["fs"])
        if not NOMBRE_83.match(n.upper()) or n != n.upper():
            avisos.append("nombre no es 8.3 en mayusculas")
        marca = "[OK]" if not avisos else "[!] "
        if avisos:
            problemas += 1
        print("%s %-20s %5d Hz %2d bits %d can. %5.1f s  %s"
              % (marca, n, info["fs"], info["bits"], info["canales"], dur, "; ".join(avisos)))

    print()
    print("Todo en el formato recomendado." if problemas == 0
          else "%d archivo(s) con avisos: conviertanlos en Audacity "
               "(mono, 22050 Hz, WAV 16 bits PCM)." % problemas)


if __name__ == "__main__":
    main()
