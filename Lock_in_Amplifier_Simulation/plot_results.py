import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# 读取数据文件
input_signal = pd.read_csv('input_signal.csv')
lock_in_output = pd.read_csv('lock_in_output.csv')

# 设置中文字体
plt.rcParams['font.sans-serif'] = ['SimHei']  # 用来正常显示中文标签
plt.rcParams['axes.unicode_minus'] = False     # 用来正常显示负号

# 创建一个包含所有子图的图形
plt.figure(figsize=(15, 20))

# 1. 原始输入信号
plt.subplot(611)
plt.plot(input_signal['time(s)'], input_signal['voltage(V)'], 'b-', label='输入信号')
plt.grid(True)
plt.legend()
plt.ylabel('电压 (V)')
plt.title('锁相放大器完整分析结果')

# 2. X和Y分量
plt.subplot(612)
plt.plot(lock_in_output['时间(s)'], lock_in_output['X_filt'], 'b-', label='X分量')
plt.plot(lock_in_output['时间(s)'], lock_in_output['Y_filt'], 'r-', label='Y分量')
plt.grid(True)
plt.legend()
plt.ylabel('幅值')

# 3. 幅值和相位
ax1 = plt.subplot(613)
ax1.plot(lock_in_output['时间(s)'], lock_in_output['幅值(V)'], 'b-', label='幅值')
ax1.set_ylabel('幅值 (V)')
ax1.grid(True)
ax1.legend(loc='upper left')
plt.xlim(0,10.5)

ax2 = ax1.twinx()
ax2.plot(lock_in_output['时间(s)'], lock_in_output['相位(deg)'], 'r-', label='相位')
ax2.set_ylabel('相位 (度)')
ax2.legend(loc='upper right')

# 4. 原始信号与还原信号对比（放大显示）
plt.subplot(614)
plt.plot(input_signal['time(s)'], input_signal['voltage(V)'], 'b-', label='原始信号', alpha=0.5)
plt.plot(lock_in_output['时间(s)'], lock_in_output['还原信号(V)'], 'r--', label='还原信号')
plt.grid(True)
plt.legend()
plt.ylabel('电压 (V)')
plt.xlim(10, 10.1)  # 显示一小段时间范围以便观察细节

# 5. 输入信号、还原信号和幅值包络
plt.subplot(615)
plt.plot(lock_in_output['时间(s)'], lock_in_output['输入值(V)'], label='输入信号', alpha=0.7)
plt.plot(lock_in_output['时间(s)'], lock_in_output['还原信号(V)'], label='还原信号', alpha=0.7)
# plt.plot(lock_in_output['时间(s)'], lock_in_output['幅值(V)'], label='幅值包络', linewidth=2)
plt.grid(True)
plt.ylabel('电压 (V)')
plt.legend()
plt.xlim(10, 10.1)

# 6. 输入信号与参考正弦信号对比
plt.subplot(616)
plt.plot(lock_in_output['时间(s)'], lock_in_output['输入值(V)'], label='输入信号', alpha=0.7)
plt.plot(lock_in_output['时间(s)'], lock_in_output['参考正弦'], label='参考正弦信号', alpha=0.7)
plt.grid(True)
plt.xlabel('时间 (秒)')
plt.ylabel('幅值')
plt.legend()
plt.xlim(10, 10.3)

# 调整子图之间的间距
plt.tight_layout()
plt.show()
# 保存图像
plt.savefig('lock_in_analysis.png', dpi=300, bbox_inches='tight')
plt.close()

print("完整分析结果已保存为 lock_in_analysis.png") 