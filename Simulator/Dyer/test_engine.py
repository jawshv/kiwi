"""
    This script validates the lumpedHybrid model against hot fire data.
    Data is derived from 'Comparing Hydroxyl Terminated Polybutadiene and Acrylonitrile Butadiene Styrene as Hybrid Rocket Fuels' by Whitmore et. al.
"""

from cfp.mach import machFromAreaRatio, areaRatioFromMach
import numpy as np
from matplotlib import pyplot as plt
from timeit import timeit
from cea import hybridlookup as cea
from ballistics.lumpedhybrid import LumpedHybrid

table = cea.HybridCEALookup()
# table.flushCache()
table.open()
table.generate(pcmin=101325, pcmax=6.8e6, pcint=50000, ofmin=0.1, ofmax=30, ofint=0.1)
cea_lookup = lambda Pc, OF : table.get(pc=Pc, of=OF)

model = LumpedHybrid(rp=0.0127, L=0.5715, rt=0.0085, rho_f=968, mode='whitmore')

# Oxidizer feed initial conditions
rho_ox = 749 # density kg/m3
Pox = 5.200e6 # feed pressure (Pa)
Pci = 3.44e6 # chamber pressure (Pa)
OF = 5 # OF ratio
Cd = 0.5 # injector discharge coefficient
Ainj = 1.115e-5 # injector area (m2) 

model.initialize(Pc=Pci, OF=OF)
f_mdot_ox = lambda Pf, Pc : Cd * Ainj * np.sqrt(2 * rho_ox * (Pf - Pc))
mdot_ox = f_mdot_ox(Pox, Pci)

tf = 10000
dt = 1/1000

output = np.zeros(shape=(tf, 9))

for i in range(0, tf):
    Pc, Tc, OF = model.chamberProps()
    mdot_ox = f_mdot_ox(Pox, Pc)
    model.step(cea_lookup, mdot_ox, rho_ox, dt)

    output[i] = [
        i * dt,
        model.x["w"],
        model.x["Pc"],
        model.x["Tc"],
        model.x["OF"],
        model.x["cstar"],
        model.x["mdot_ox"],
        model.x["mdot_f"],
        model.x["dPc"]
    ]

model.logState()

# print(output[:,6])

print(np.trapz(output[:,6], dx=dt))
print(np.trapz(output[:,7], dx=dt))

plt.clf()
plt.plot(output[:,0], output[:,1])
plt.xlabel('t')
plt.ylabel('w (m)')
plt.savefig('tempout_w.png')

plt.clf()
plt.plot(output[:,0], output[:,5])
plt.xlabel('t')
plt.ylabel('c* (m/s)')
plt.savefig('tempout_cstar.png')

plt.clf()
plt.plot(output[:,0], output[:,2])
plt.xlabel('t')
plt.ylabel('pc (Pa)')
plt.savefig('tempout_pc.png')
# plt.show()
