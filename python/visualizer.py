import sys, os
import pandas as pd
import matplotlib.pyplot as plt

def load_csv(path):
    df = pd.read_csv(path)

    df["t_ms"]  = pd.to_numeric(df["t_ms"], errors="coerce")
    df["VA"]    = pd.to_numeric(df["VA"], errors="coerce")
    df["VB"]    = pd.to_numeric(df["VB"], errors="coerce")
    df["spA_s"] = pd.to_numeric(df["spA_s"], errors="coerce")
    df["spB_s"] = pd.to_numeric(df["spB_s"], errors="coerce")
    df["dir"]   = df["dir"].astype(str).str.strip()

    df = df.dropna(subset=["t_ms","VA","VB","spA_s","spB_s"])
    df["t_s"] = df["t_ms"] / 1000.0

    # map direction to numeric
    dir_map = {"L": -1, ".": 0, "R": 1}
    df["dir_num"] = df["dir"].map(dir_map).fillna(0)

    return df

def save_plots(df, path):
    base = os.path.splitext(path)[0]

    # Plot 1: voltages
    plt.figure()
    plt.plot(df["t_s"], df["VA"], label="VA")
    plt.plot(df["t_s"], df["VB"], label="VB")
    plt.title(f"Voltages VA/VB — {os.path.basename(path)}")
    plt.xlabel("time (s)")
    plt.ylabel("V (Volts)")
    plt.legend()
    plt.tight_layout()
    plt.savefig(base + "_voltages.png", dpi=160)
    plt.close()

    # Plot 2: spike rates
    plt.figure()
    plt.plot(df["t_s"], df["spA_s"], label="spA_s")
    plt.plot(df["t_s"], df["spB_s"], label="spB_s")
    plt.title(f"Spike rates — {os.path.basename(path)}")
    plt.xlabel("time (s)")
    plt.ylabel("spikes/s")
    plt.legend()
    plt.tight_layout()
    plt.savefig(base + "_spikes.png", dpi=160)
    plt.close()

    # Plot 3: direction
    plt.figure()
    plt.step(df["t_s"], df["dir_num"], where="post")
    plt.title(f"Direction — {os.path.basename(path)}")
    plt.xlabel("time (s)")
    plt.ylabel("direction")
    plt.yticks([-1, 0, 1], ["L", ".", "R"])
    plt.ylim(-1.5, 1.5)
    plt.tight_layout()
    plt.savefig(base + "_direction.png", dpi=160)
    plt.close()

def main(paths):
    if not paths:
        print("Usage: python plot_direction.py <csv1> [csv2 ...]")
        return

    for p in paths:
        df = load_csv(p)

        counts = df["dir"].value_counts().to_dict()
        print(f"\nFile: {p}")
        print("Direction counts:", {k: counts.get(k, 0) for k in ['L', 'R', '.']})

        save_plots(df, p)

        print("Saved:", os.path.splitext(p)[0] + "_voltages.png")
        print("Saved:", os.path.splitext(p)[0] + "_spikes.png")
        print("Saved:", os.path.splitext(p)[0] + "_direction.png")

if __name__ == "__main__":
    main(sys.argv[1:])
