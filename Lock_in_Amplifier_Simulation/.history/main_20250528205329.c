#include <stdio.h>
#include <stdlib.h>
#include <math.h> // 需要链接 -lm

// --- 全局常量定义 ---
#ifndef M_PI
#define M_PI (3.14159265358979323846f) // 定义 PI (float)
#endif

// --- 可配置参数 ---
// 信号参数
#define SAMPLING_RATE 10000.0f // 采样率 (Hz)，例如10kHz
#define SIGNAL_DURATION 20.0f   // 信号持续时间 (秒) - 用于生成待测信号的总长度
#define NUM_SAMPLES ((int)(SAMPLING_RATE * SIGNAL_DURATION)) // 总采样点数

// 待测信号参数 (用于生成模拟输入)
#define INPUT_FREQ 50.0f       // 待测信号中的目标频率 (Hz)
#define INPUT_AMPLITUDE 1.3f   // 待测信号中目标频率分量的幅值 (V)
#define INPUT_PHASE_DEG 30.0f  // 待测信号中目标频率分量的相位 (度)
#define INPUT_DC_OFFSET 1.7f   // 待测信号的直流偏置 (V)
                               // 使得信号范围如 0.4V (1.7-1.3) 到 3.0V (1.7+1.3)

// 可选：添加一些噪声/干扰信号
#define NOISE_FREQ_1 150.0f    // 第一个噪声频率 (Hz)
#define NOISE_AMPLITUDE_1 0.5f // 第一个噪声幅值 (V)
#define NOISE_PHASE_DEG_1 0.0f // 第一个噪声相位 (度)

// 参考信号参数
#define REF_FREQ INPUT_FREQ  // 参考信号频率，应与待测信号中的目标频率一致
#define LUT_SIZE 1024        // 参考信号查找表的大小 (一个周期的点数)

// 低通滤波器参数
#define LPF_CUTOFF_FREQ 0.05f // 低通滤波器的截止频率 (Hz)
                             // 应远小于 REF_FREQ，也远小于 REF_FREQ 的两倍频
                             // 也应小于任何不希望通过的噪声频率

// 输出控制
#define PRINT_INTERVAL 99   // 每隔多少个采样点打印一次结果

// --- 全局数组和变量 ---
float* input_signal_buffer = NULL; // 修改为指针
float ref_cos_lut[LUT_SIZE];            // 参考余弦查找表
float ref_sin_lut[LUT_SIZE];            // 参考正弦查找表

// 低通滤波器状态变量 (前一个输出值)
float filtered_X_prev = 0.0f;
float filtered_Y_prev = 0.0f;

// 参考信号相位累加器
float ref_phase_accumulator = 0.0f;

// --- 函数声明 ---
void generate_input_signal_buffer(float freq, float amplitude, float phase_deg, float dc_offset,
                                  float noise_freq1, float noise_amp1, float noise_phase_deg1);
void generate_reference_lut(void);
void get_reference_samples(float* cos_val, float* sin_val); // 参数是指针
void low_pass_filter(float raw_X, float raw_Y, float* filtered_X, float* filtered_Y, float alpha); // 参数是指针
void calculate_amplitude_and_phase(float X, float Y, float* amplitude, float* phase_deg); // 参数是指针
float get_simulated_adc_sample(int sample_index); // 模拟从ADC读取一个样本

// --- 主函数 ---
int main() {
    printf("锁相放大器算法模拟 (流式处理)\n");
    printf("---------------------------------\n");
    printf("参数设置:\n");
    printf("  采样率: %.1f Hz\n", SAMPLING_RATE);
    printf("  信号时长 (模拟): %.2f s\n", SIGNAL_DURATION);
    printf("  总采样点数 (模拟): %d\n", NUM_SAMPLES);
    printf("  待测信号 (目标): %.1f Hz, 幅值 %.2f V, 相位 %.1f 度, 直流偏置 %.2f V\n",
           INPUT_FREQ, INPUT_AMPLITUDE, INPUT_PHASE_DEG, INPUT_DC_OFFSET);
    printf("  (预期信号范围: %.2fV to %.2fV)\n", INPUT_DC_OFFSET - INPUT_AMPLITUDE, INPUT_DC_OFFSET + INPUT_AMPLITUDE);
    printf("  噪声信号1: %.1f Hz, 幅值 %.2f V, 相位 %.1f 度\n",
           NOISE_FREQ_1, NOISE_AMPLITUDE_1, NOISE_PHASE_DEG_1);
    printf("  参考信号频率: %.1f Hz, LUT大小: %d\n", REF_FREQ, LUT_SIZE);
    printf("  低通滤波器截止频率: %.1f Hz\n", LPF_CUTOFF_FREQ);
    printf("  每隔 %d 个采样点输出结果\n", PRINT_INTERVAL);
    printf("---------------------------------\n\n");
    printf("时间(s)\t ADC值(V)\t X_filt\t Y_filt\t 幅值(V)\t 相位(deg)\n");


    // 1. 分配内存
    input_signal_buffer = (float*)malloc(NUM_SAMPLES * sizeof(float));
    if (input_signal_buffer == NULL) {
        printf("内存分配失败!\n");
        return -1;
    }

    // 2. 预生成待测信号序列 (模拟ADC的连续输入源)
    generate_input_signal_buffer(INPUT_FREQ, INPUT_AMPLITUDE, INPUT_PHASE_DEG, INPUT_DC_OFFSET,
                               NOISE_FREQ_1, NOISE_AMPLITUDE_1, NOISE_PHASE_DEG_1);
    printf("待测信号缓存已生成.\n");

    // 输出待测信号前100个样本 (调试用)
    printf("前100个待测信号样本:\n");
    for (int i = 0; i < 100 && i < NUM_SAMPLES; ++i) {
        printf("样本 %d: %.4f V\n", i, input_signal_buffer[i]);
    }
    // 绘制待测信号图像
    // （PC端可选：输出CSV文件，便于用Excel/Matlab/Python绘图）
    FILE* fp = fopen("input_signal.csv", "w");
    if (fp) {
        fprintf(fp, "sample_index,time(s),voltage(V)\n");
        for (int i = 0; i < NUM_SAMPLES; ++i) {
            float t = (float)i / SAMPLING_RATE;
            fprintf(fp, "%d,%.6f,%.6f\n", i, t, input_signal_buffer[i]);
        }
        fclose(fp);
        printf("已导出 input_signal.csv，可用Excel/Matlab/Python绘图.\n");
    } else {
        printf("无法写入 input_signal.csv，跳过信号导出.\n");
    }


    // 3. 生成参考正余弦查找表
    generate_reference_lut();
    printf("参考信号查找表已生成.\n\n");

    // 计算低通滤波器系数 alpha
    float lpf_alpha;
    if (LPF_CUTOFF_FREQ > 0 && SAMPLING_RATE > 0) { // 添加检查 SAMPLING_RATE > 0
        lpf_alpha = (2.0f * M_PI * LPF_CUTOFF_FREQ) / (SAMPLING_RATE + 2.0f * M_PI * LPF_CUTOFF_FREQ);
    } else {
        lpf_alpha = 1.0f; // 如果截止频率为0或采样率为0，则alpha为1意味着直接通过，无滤波
                          // 或者可以设置为0，意味着完全阻塞，取决于期望行为
        if (LPF_CUTOFF_FREQ > 0) printf("警告: 采样率为0，LPF alpha 设置为1\n");

    }
    printf("LPF alpha: %f\n", lpf_alpha);
    printf("---------------------------------\n");

    // 在开始循环前清空并创建新文件
    FILE* header_fp = fopen("lock_in_output.csv", "w");
    if (header_fp) {
        // 写入CSV标题行
        fprintf(header_fp, "时间(s),输入值(V),X_filt,Y_filt,幅值(V),相位(deg)\n");
        fclose(header_fp);
    } else {
        printf("无法创建 lock_in_output.csv 文件!\n");
    }

    // 4. 模拟连续处理
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        // 4.1 模拟从ADC获取当前采样值
        float current_input_sample = get_simulated_adc_sample(i);

        // 4.2 从LUT获取当前参考信号样本
        float current_ref_cos, current_ref_sin; // 定义局部变量
        get_reference_samples(&current_ref_cos, &current_ref_sin); // 修正变量名

        // 4.3 混频 (Phase Sensitive Detection - PSD)
        float mixed_X = current_input_sample * current_ref_cos;
        float mixed_Y = current_input_sample * current_ref_sin;

        // 4.4 低通滤波
        float current_filtered_X, current_filtered_Y; // 定义局部变量
        low_pass_filter(mixed_X, mixed_Y, &current_filtered_X, &current_filtered_Y, lpf_alpha); // 修正变量名

        // 4.5 计算幅值和相位
        float recovered_amplitude, recovered_phase_deg; // 定义局部变量
        calculate_amplitude_and_phase(current_filtered_X, current_filtered_Y, &recovered_amplitude, &recovered_phase_deg);

        // 4.6 定期输出结果
        if ((i + 1) % PRINT_INTERVAL == 0 || i == NUM_SAMPLES -1) {
            float current_time = (float)i / SAMPLING_RATE;
            printf("%.4f\t %.4f\t %.4f\t %.4f\t %.4f\t %.2f\n",
                   current_time,
                   current_input_sample,
                   current_filtered_X,
                   current_filtered_Y,
                   recovered_amplitude,
                   recovered_phase_deg);
        }
        // 输出最后100个样本 (调试用)
        if (i >= NUM_SAMPLES - 100) {
            printf("样本 %d: 输入=%.4f V, X_filt=%.4f, Y_filt=%.4f, 幅值=%.4f V, 相位=%.2f 度\n",
                   i, current_input_sample, current_filtered_X, current_filtered_Y,
                   recovered_amplitude, recovered_phase_deg);
        }
        // 将结果保存到文件 (可选)
        FILE* fp = fopen("lock_in_output.csv", "a");
        float current_time = (float)i / SAMPLING_RATE;
        if (fp) {
            fprintf(fp, "%.4f,%.4f,%.4f,%.4f,%.4f,%.2f\n",
                    current_time,
                    current_input_sample,
                    current_filtered_X,
                    current_filtered_Y,
                    recovered_amplitude,
                    recovered_phase_deg);
            fclose(fp);
        } else {
            printf("无法写入 lock_in_output.csv，跳过结果保存.\n");
        }
    }
    
    // 释放分配的内存
    free(input_signal_buffer);
    
    printf("---------------------------------\n");
    printf("模拟处理完成.\n");

    return 0;
}

// --- 函数定义 ---

/**
 * @brief 从预生成的缓冲区获取一个模拟的ADC样本
 * @param sample_index 当前的样本索引
 * @return 该样本的电压值
 */
float get_simulated_adc_sample(int sample_index) {
    if (sample_index >= 0 && sample_index < NUM_SAMPLES) {
        return input_signal_buffer[sample_index];
    }
    return 0.0f; // 超出范围则返回0
}

/**
 * @brief 生成待测信号并存储到全局缓冲区 input_signal_buffer
 * 
 * @param freq 待测信号的目标频率 (Hz)
 * @param amplitude 待测信号的目标幅值 (V)
 * @param phase_deg 待测信号的目标相位 (度)
 * @param dc_offset 待测信号的直流偏置 (V)
 * @param noise_freq1 第一个噪声信号的频率 (Hz)
 * @param noise_amp1 第一个噪声信号的幅值 (V)
 * @param noise_phase_deg1 第一个噪声信号的相位 (度)
 *
 * 信号模型: s(t) = dc_offset + A * cos(2*pi*f*t - phi) + Noise(t)
 */
void generate_input_signal_buffer(float freq, float amplitude, float phase_deg, float dc_offset,
                                  float noise_freq1, float noise_amp1, float noise_phase_deg1) {
    float t;
    float phase_rad = phase_deg * M_PI / 180.0f;
    float noise_phase_rad1 = noise_phase_deg1 * M_PI / 180.0f;

    for (int i = 0; i < NUM_SAMPLES; ++i) {
        t = (float)i / SAMPLING_RATE;

        float target_component = amplitude * cosf(2.0f * M_PI * freq * t - phase_rad);
        float noise_component = noise_amp1 * cosf(2.0f * M_PI * noise_freq1 * t - noise_phase_rad1);

        input_signal_buffer[i] = dc_offset + target_component + noise_component;
    }
}

/**
 * @brief 生成参考正余弦查找表 (LUT)
 * LUT覆盖一个完整的 2*PI 周期
 */
void generate_reference_lut(void) {
    for (int i = 0; i < LUT_SIZE; ++i) {
        float angle = 2.0f * M_PI * (float)i / (float)LUT_SIZE;
        ref_cos_lut[i] = cosf(angle);
        ref_sin_lut[i] = sinf(angle);
    }
}

/**
 * @brief 从查找表获取当前周期的参考正余弦值，并更新相位累加器
 *
 * @param cos_val 指针，用于返回当前的余弦参考值
 * @param sin_val 指针，用于返回当前的正弦参考值
 */
void get_reference_samples(float* cos_val, float* sin_val) {
    float phase_increment = (REF_FREQ / SAMPLING_RATE) * (float)LUT_SIZE;
    ref_phase_accumulator += phase_increment;

    if (ref_phase_accumulator >= (float)LUT_SIZE) {
         ref_phase_accumulator = fmodf(ref_phase_accumulator, (float)LUT_SIZE);
    }

    unsigned int lut_index = (unsigned int)ref_phase_accumulator;
    // 在 STM32 上，如果 LUT_SIZE 是 2 的幂，可以用位运算优化取模: lut_index = ((unsigned int)ref_phase_accumulator) & (LUT_SIZE - 1);
    if (lut_index >= LUT_SIZE) lut_index = LUT_SIZE - 1; // 防御性编程，虽然fmodf后应该不会超

    *cos_val = ref_cos_lut[lut_index];
    *sin_val = ref_sin_lut[lut_index];
}


/**
 * @brief 一阶IIR低通滤波器
 * y[n] = (1-alpha)*y[n-1] + alpha*x[n]
 *
 * @param raw_X 当前混频后的X通道值 (滤波器输入)
 * @param raw_Y 当前混频后的Y通道值 (滤波器输入)
 * @param filtered_X 指针，用于返回滤波后的X值 (滤波器输出)
 * @param filtered_Y 指针，用于返回滤波后的Y值 (滤波器输出)
 * @param alpha 滤波器系数
 */
void low_pass_filter(float raw_X, float raw_Y, float* filtered_X, float* filtered_Y, float alpha) {
    filtered_X_prev = (1.0f - alpha) * filtered_X_prev + alpha * raw_X;
    *filtered_X = filtered_X_prev;

    filtered_Y_prev = (1.0f - alpha) * filtered_Y_prev + alpha * raw_Y;
    *filtered_Y = filtered_Y_prev;
}

/**
 * @brief 根据 X 和 Y 计算原始信号的幅值和相位
 * 输入信号模型: A_in * cos(2*pi*f*t - phi_in)
 * X = (A_in/2) * cos(phi_in)
 * Y = (A_in/2) * sin(phi_in)
 *
 * @param X 相关滤波得到的 X 分量 (已滤波)
 * @param Y 相关滤波得到的 Y 分量 (已滤波)
 * @param amplitude 输出参数，存储计算得到的幅值 (A_in)
 * @param phase_deg 输出参数，存储计算得到的相位 (phi_in, 度)
 */
void calculate_amplitude_and_phase(float X, float Y, float* amplitude, float* phase_deg) {
    *amplitude = 2.0f * sqrtf(X * X + Y * Y);
    float phase_rad = atan2f(Y, X);
    *phase_deg = phase_rad * 180.0f / M_PI;
}