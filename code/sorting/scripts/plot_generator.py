import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Rutas de los datos y de las gráficas
RUTA_MEDICIONES = "../data/measurements/measurements.txt"
DIR_SALIDA = "../data/plots"

os.makedirs(DIR_SALIDA, exist_ok=True)

# Colores para mantener los algoritmos consistentes
COLORES_ALGORITMOS = {
    "mergesort": "#1f77b4",
    "quicksort": "#ff7f0e",
    "patiencesort": "#2ca02c",
    "sort": "#d62728",
}


def cargar_datos():
    # Leemos el archivo separado por espacios
    df = pd.read_csv(RUTA_MEDICIONES, sep=r"\s+", engine="python")

    # Dejamos los tipos como corresponde
    df["tamano"] = df["tamano"].astype(int)
    df["tiempo_ms"] = df["tiempo_ms"].astype(float)
    df["memoria_bytes"] = df["memoria_bytes"].astype(float)

    return df


def promediar_por_tamano(df, columna_y):
    # Juntamos las mediciones del mismo algoritmo y tamaño
    resumen = (
        df.groupby(["algoritmo", "tamano"])[columna_y]
        .agg(media="mean", desvio="std")
        .reset_index()
        .sort_values("tamano")
    )

    # Por si hay un solo dato
    resumen["desvio"] = resumen["desvio"].fillna(0)

    return resumen


def graficar_lineas(df, columna_y, etiqueta_y, titulo, nombre_archivo):
    resumen = promediar_por_tamano(df, columna_y)

    fig, ax = plt.subplots(figsize=(9, 6))

    for algoritmo, grupo in resumen.groupby("algoritmo"):
        color = COLORES_ALGORITMOS.get(algoritmo, "gray")

        ax.plot(
            grupo["tamano"],
            grupo["media"],
            marker="o",
            label=algoritmo,
            color=color,
        )

        # Mostramos la variación de las mediciones
        limite_inferior = np.clip(
            grupo["media"] - grupo["desvio"],
            a_min=1e-9,
            a_max=None,
        )
        limite_superior = grupo["media"] + grupo["desvio"]

        ax.fill_between(
            grupo["tamano"],
            limite_inferior,
            limite_superior,
            color=color,
            alpha=0.15,
        )

    # Log porque los tamaños crecen bastante
    ax.set_xscale("log")
    ax.set_yscale("log")

    ax.set_xlabel("Tamaño del arreglo (n)")
    ax.set_ylabel(etiqueta_y)
    ax.set_title(titulo)
    ax.legend(title="Algoritmo")
    ax.grid(True, which="both", linestyle="--", alpha=0.3)

    fig.tight_layout()

    ruta = os.path.join(DIR_SALIDA, nombre_archivo)
    fig.savefig(ruta, dpi=150)
    plt.close(fig)

    print(f"Guardado: {ruta}")


def graficar_barras_por_dominio(df):
    # Promedio de tiempo para cada algoritmo y dominio
    resumen = (
        df.groupby(["algoritmo", "d"])["tiempo_ms"]
        .mean()
        .reset_index()
    )

    dominios = sorted(resumen["d"].unique())
    algoritmos = sorted(resumen["algoritmo"].unique())

    x = range(len(algoritmos))
    ancho = 0.8 / len(dominios)

    fig, ax = plt.subplots(figsize=(9, 6))

    for i, dominio in enumerate(dominios):
        valores = []

        for algoritmo in algoritmos:
            fila = resumen[
                (resumen["algoritmo"] == algoritmo)
                & (resumen["d"] == dominio)
            ]

            # Si falta alguna medición, dejamos la barra en 0
            valores.append(
                fila["tiempo_ms"].values[0]
                if not fila.empty
                else 0
            )

        posiciones = [xi + i * ancho for xi in x]

        ax.bar(
            posiciones,
            valores,
            width=ancho,
            label=dominio,
        )

    ax.set_xticks(
        [xi + ancho * (len(dominios) - 1) / 2 for xi in x]
    )
    ax.set_xticklabels(algoritmos)

    ax.set_ylabel("Tiempo de ejecución promedio (ms)")
    ax.set_title("Tiempo de ejecución promedio por dominio y algoritmo")
    ax.legend(title="Dominio")
    ax.grid(True, axis="y", linestyle="--", alpha=0.3)

    fig.tight_layout()

    ruta = os.path.join(DIR_SALIDA, "tiempo_por_dominio.png")
    fig.savefig(ruta, dpi=150)
    plt.close(fig)

    print(f"Guardado: {ruta}")


def graficar_complejidad_teorica(df):
    resumen = promediar_por_tamano(df, "tiempo_ms")

    fig, ax = plt.subplots(figsize=(9, 6))

    for algoritmo, grupo in resumen.groupby("algoritmo"):
        color = COLORES_ALGORITMOS.get(algoritmo, "gray")

        ax.plot(
            grupo["tamano"],
            grupo["media"],
            marker="o",
            label=f"{algoritmo} (empírico)",
            color=color,
        )

    # Tomamos el punto más grande para ajustar las curvas
    n_max = resumen["tamano"].max()

    tiempo_en_n_max = resumen.loc[
        resumen["tamano"] == n_max,
        "media"
    ].mean()

    n_valores = np.sort(resumen["tamano"].unique())

    referencias = {
        "O(n log n)": n_valores * np.log2(n_valores),
        "O(n^2)": n_valores.astype(float) ** 2,
    }

    for nombre_ref, valores in referencias.items():
        # Ajustamos la escala para que quede cerca de los datos
        factor = tiempo_en_n_max / valores[-1]

        ax.plot(
            n_valores,
            valores * factor,
            linestyle="--",
            color="gray",
            alpha=0.6,
            label=nombre_ref,
        )

    ax.set_xscale("log")
    ax.set_yscale("log")

    ax.set_xlabel("Tamaño del arreglo (n)")
    ax.set_ylabel("Tiempo de ejecución (ms)")
    ax.set_title(
        "Comparación de curvas empíricas con complejidad teórica"
    )
    ax.legend(title="Algoritmo / referencia", fontsize=8)
    ax.grid(True, which="both", linestyle="--", alpha=0.3)

    fig.tight_layout()

    ruta = os.path.join(DIR_SALIDA, "complejidad_teorica.png")
    fig.savefig(ruta, dpi=150)
    plt.close(fig)

    print(f"Guardado: {ruta}")


def main():
    df = cargar_datos()

    # Gráfica de memoria
    graficar_lineas(
        df,
        "memoria_bytes",
        "Memoria utilizada (bytes)",
        "Tamaño del arreglo vs. memoria utilizada promedio",
        "tamano_vs_memoria.png",
    )

    # Las otras dos comparaciones
    graficar_barras_por_dominio(df)
    graficar_complejidad_teorica(df)


if __name__ == "__main__":
    main()