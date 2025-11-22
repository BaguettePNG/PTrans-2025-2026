import numpy as np
import matplotlib.pyplot as plt

# Équations approximatives issues de la datasheet (en kΩ)
def Rmax(L):
    return 100 * L**(-0.76)

def Rmin(L):
    return 30 * L**(-0.74)

# Domaine : 1 → 100 lux
L = np.logspace(0, 2, 500)

R_max = Rmax(L)
R_min = Rmin(L)

plt.figure(figsize=(7,5))

# Tracé des courbes
plt.loglog(L, R_max, label="Rmax")
plt.loglog(L, R_min, label="Rmin")

# Zone grisée
plt.fill_between(L, R_min, R_max, alpha=0.3)

# 📌 Équations affichées directement sur le graphique
plt.text(2, Rmax(2), r"$R_{\max}(L) = 100\,L^{-0.76}$", 
         fontsize=11, va="bottom")

plt.text(2, Rmin(2), r"$R_{\min}(L) = 30\,L^{-0.74}$", 
         fontsize=11, va="top")

plt.xlabel("Luminosité (lux)")
plt.ylabel("Résistance (kΩ)")
plt.title("Courbes LDR – équations affichées")
plt.grid(True, which="both")

plt.show()
