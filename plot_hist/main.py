import pandas as pd
import matplotlib.pyplot as plt

red = pd.read_csv("red.csv", header=None)
red.columns = ["intensity", "count"]

plt.figure()
plt.bar(red["intensity"], red["count"])
plt.xlabel("Intensity Value")
plt.ylabel("Frequency")
plt.title("Red Image Intensity Histogram")
plt.yscale("log")
plt.xlim(0, 255)
plt.savefig("graphs/red_hist_nonequal.png")

green = pd.read_csv("green.csv", header=None)
green.columns = ["intensity", "count"]

plt.figure()
plt.bar(green["intensity"], green["count"])
plt.xlabel("Intensity Value")
plt.ylabel("Frequency")
plt.title("Green Image Intensity Histogram")
plt.yscale("log")
plt.xlim(0, 255)
plt.savefig("graphs/green_hist_nonequal.png")

blue = pd.read_csv("blue.csv", header=None)
blue.columns = ["intensity", "count"]

plt.figure()
plt.bar(blue["intensity"], blue["count"])
plt.xlabel("Intensity Value")
plt.ylabel("Frequency")
plt.title("Blue Image Intensity Histogram")
plt.yscale("log")
plt.xlim(0, 255)
plt.savefig("graphs/blue_hist_nonequal.png")
