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

# 创建一个2x2的子图布局
fig = plt.figure(figsize=(15, 10))

# 1. 绘制待测信号时域图
ax1 = plt.subplot(2, 2, 1)
ax1.plot(input_signal['time(s)'], input_signal['voltage(V)'])
ax1.set_title('待测信号时域图')
ax1.set_xlabel('时间 (s)')
ax1.set_ylabel('电压 (V)')
ax1.grid(True)

# 2. 绘制待测信号的FFT
# 计算FFT
voltage = input_signal['voltage(V)'].values
N = len(voltage)
T = input_signal['time(s)'].iloc[1] - input_signal['time(s)'].iloc[0]  # 采样周期
yf = fft(voltage)
xf = fftfreq(N, T)  # 频率轴

# 只显示正频率部分
ax2 = plt.subplot(2, 2, 2)
ax2.plot(xf[:N//2], 2.0/N * np.abs(yf[:N//2]))
ax2.set_title('待测信号频谱')
ax2.set_xlabel('频率 (Hz)')
ax2.set_ylabel('幅值')
ax2.grid(True)
# 限制x轴范围到有意义的频率范围（例如0-500Hz）
ax2.set_xlim(0, 500)

# 3. 绘制锁相放大器输出的幅值随时间变化
ax3 = plt.subplot(2, 2, 3)
ax3.plot(lock_in_output['时间(s)'], lock_in_output['幅值(V)'])
ax3.set_title('锁相放大器幅值输出')
ax3.set_xlabel('时间 (s)')
ax3.set_ylabel('幅值 (V)')
ax3.set_xlim(1.7, 2)
ax3.grid(True)

# 4. 绘制锁相放大器输出的相位随时间变化
ax4 = plt.subplot(2, 2, 4)
ax4.plot(lock_in_output['时间(s)'], lock_in_output['相位(deg)'])
ax4.set_title('锁相放大器相位输出')
ax4.set_xlabel('时间 (s)')
ax4.set_ylabel('相位 (度)')
ax4.grid(True)

# 调整子图布局
plt.tight_layout()

# 保存图像
plt.savefig('lock_in_analysis.png', dpi=300, bbox_inches='tight')
# print("分析结果已保存为 'lock_in_analysis.png'")

# 显示图像
plt.show() 