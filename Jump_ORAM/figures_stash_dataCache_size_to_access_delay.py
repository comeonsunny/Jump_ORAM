# 1.process the raw data to get the access delay for each stash size and dataCache size pair ranging from 2^1 to 2^9
import numpy as np
s_size = [2**i for i in range(1, 10)]
d_size = [2**i for i in range(1, 10)]
access_delay = np.zeros((len(s_size), len(d_size)))
for i in range(len(s_size)):
    for j in range(len(d_size)):
        # log/stash_size_2_dataCache_size_2.txt
        log_path = 'log/stash_size_' + str(s_size[i]) + '_dataCache_size_' + str(d_size[j]) + '.txt'
        with open(log_path, 'r') as f:
            data = [int(x) for x in f.read().split()]
            access_delay[i][j] = np.mean(data)

import matplotlib.pyplot as plt
# 2.plot the figure
color = "#0976b5"
color_bg_line = '#8c8c8c'
# 2.1 plot the line chart with varying s_size (independent variable) and d_size 2^9; and the access_delay as the dependent variable
plt.figure()
plt.grid(True)
s_size_str = [str(x) for x in s_size]
plt.plot(s_size_str, access_delay[:, -1], 'o-', color=color)
plt.xlabel('Stash size (blocks)')
plt.ylabel('Access delay (ns)')
plt.legend()
# save the figure to the "log/figures" with the name of accessDelay_varying_stashSize_lineChart.png
plt.savefig("log/figures/accessDelay_varying_stashSize_lineChart.png", dpi=300)

# 2.2 plot the bar chart with varying d_size (independent variable) and s_size 2^9; and the access_delay as the dependent variable
plt.figure()
# just keep the horizontal grid lines
# plt.grid(axis='y')
d_size_str = [str(x) for x in d_size] 
plt.bar(d_size_str, access_delay[-1, :], color=color)
plt.xlabel('Data Cache size (blocks)')
plt.ylabel('Access delay (ns)')
plt.legend()
# add a horizontal line at the mean of the last column of access_delay
plt.axhline(y=np.mean(access_delay[-1, :]), color=color_bg_line, linestyle='--')
# save the figure to the "log/figures" with the name of accessDelay_varying_dataCacheSize_barChart.png
plt.savefig("log/figures/accessDelay_varying_dataCacheSize_barChart.png", dpi=300)

# 2.3 draw the 3d scatter with varying s_size and d_size; and the access_delay as the dependent variable
# s_size_extension = np.tile(s_size, (len(d_size), 1)).T
# d_size_extension = np.tile(d_size, (len(s_size), 1))
# access_delay_extension = access_delay.flatten()
# # save the data s_size_extension, d_size_extension, access_delay_extension to file namely "data.csv" the title of each column is "stash_size", "dataCache_size", "access_delay"
# np.savetxt("log/figures/data.csv", np.column_stack((s_size_extension.flatten(), d_size_extension.flatten(), access_delay_extension)), delimiter=",", header="stash_size,dataCache_size,access_delay", comments='')




