import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy.fft import fft, fftfreq

# 设置中文显示
plt.rcParams['font.sans-serif'] = ['SimHei']  # 用来正常显示中文标签
plt.rcParams['axes.unicode_minus'] = False     # 用来正常显示负号

# 读取数据文件
input_signal = pd.read_csv('input_signal.csv')
lock_in_output = pd.read_csv('lock_in_output.csv')

# 创建一个包含4个子图的图形
plt.figure(figsize=(15, 10))

# 1. 原始输入信号
plt.subplot(411)
plt.plot(input_signal['time(s)'], input_signal['voltage(V)'], 'b-', label='Input Signal')
plt.grid(True)
plt.legend()
plt.ylabel('Voltage (V)')
plt.title('Lock-in Amplifier Results')

# 2. X和Y分量
plt.subplot(412)
plt.plot(input_signal['time(s)'], input_signal['voltage(V)'], 'b-', label='Input Signal')
plt.grid(True)
plt.legend()
plt.ylabel('Voltage (V)')

# 3. 幅值和相位
ax1 = plt.subplot(413)
ax1.plot(lock_in_output['时间(s)'], lock_in_output['幅值(V)'], 'b-', label='Amplitude')
ax1.set_ylabel('Amplitude (V)')
ax1.grid(True)
ax1.legend(loc='upper left')

ax2 = ax1.twinx()
ax2.plot(lock_in_output['时间(s)'], lock_in_output['相位(deg)'], 'r-', label='Phase')
ax2.set_ylabel('Phase (deg)')
ax2.legend(loc='upper right')

# 4. 原始信号与还原信号对比
plt.subplot(414)
plt.plot(input_signal['time(s)'], input_signal['voltage(V)'], 'b-', label='Original', alpha=0.5)
plt.plot(lock_in_output['时间(s)'], lock_in_output['还原信号(V)'], 'r--', label='Recovered')
plt.grid(True)
plt.legend()
plt.xlabel('Time (s)')
plt.ylabel('Voltage (V)')
plt.xlim(10, 10.1)

# 调整子图间距
plt.tight_layout()
plt.show()

# 保存图像
plt.savefig('lock_in_results.png', dpi=300, bbox_inches='tight')
plt.close()

print("图像已保存为 lock_in_results.png") 