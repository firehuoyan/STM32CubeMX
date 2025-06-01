#include <stdio.h>
#include <stdlib.h>
#include <math.h>    // 需要链接 -lm
#include <string.h>  // 添加string.h用于memset函数
#include <time.h>    // 添加time.h用于计时


// --- 全局常量定义 ---
#ifndef M_PI
#define M_PI (3.14159265358979323846f) // 定义 PI (float)
#endif

// --- 输出模式控制 ---
typedef enum {
    OUTPUT_MODE_RECOVERED_SIGNAL = 1,
    OUTPUT_MODE_INTERNAL_REF_SIGNALS = 2,
    OUTPUT_MODE_REF_COS_AND_RECOVERED = 3
} OutputMode;

// 全局变量，用于控制输出模式
OutputMode g_output_mode = OUTPUT_MODE_REF_COS_AND_RECOVERED;

// --- 可配置参数 ---
// 信号参数
#define SAMPLING_RATE 10000.0f // 采样率 (Hz)，例如10kHz
#define SIGNAL_DURATION 50.0f   // 信号持续时间 (秒) - 用于生成待测信号的总长度
#define NUM_SAMPLES ((int)(SAMPLING_RATE * SIGNAL_DURATION)) // 总采样点数
#define AVERAGE_TIME 0.1f      // 平均时间窗口长度（秒）
#define BUFFER_SIZE ((int)(SAMPLING_RATE * AVERAGE_TIME)) // 缓冲区大小


// 待测信号1参数 (用于生成模拟输入)
#define SIGNAL1_DURATION 25.0f  // 第一段信号持续时间 (秒)
#define INPUT1_FREQ 50.0f       // 待测信号1中的目标频率 (Hz)
#define INPUT1_AMPLITUDE 1.3f   // 待测信号1中目标频率分量的幅值 (V)
#define INPUT1_PHASE_DEG 80.0f  // 待测信号1中目标频率分量的相位 (度)
#define INPUT1_DC_OFFSET 1.7f   // 待测信号1的直流偏置 (V)

// 待测信号2参数 (用于生成模拟输入)
#define SIGNAL2_DURATION 25.0f  // 第二段信号持续时间 (秒)
#define INPUT2_FREQ 100.0f       // 待测信号2中的目标频率 (Hz)
#define INPUT2_AMPLITUDE 1.5f   // 待测信号2中目标频率分量的幅值 (V)
#define INPUT2_PHASE_DEG 15.0f  // 待测信号2中目标频率分量的相位 (度)
#define INPUT2_DC_OFFSET 1.7f   // 待测信号2的直流偏置 (V)

// 可选：添加一些噪声/干扰信号
#define NOISE_FREQ_1 150.0f    // 第一个噪声频率 (Hz)
#define NOISE_AMPLITUDE_1 0.0f // 第一个噪声幅值 (V)
#define NOISE_PHASE_DEG_1 0.0f // 第一个噪声相位 (度)


// 参考信号参数
#define REF_FREQ_OFFSET 0.001f   // 参考信号相对于待测信号的频率偏移 (Hz)
float g_ref_freq = INPUT1_FREQ + REF_FREQ_OFFSET;  // 参考信号频率 (Hz)
#define LUT_SIZE 8192        // 参考信号查找表的大小 (一个周期的点数)

// 输出控制
#define PRINT_INTERVAL_SEC 0.1f   // 每隔多少秒打印一次结果
#define PRINT_INTERVAL_SAMPLES ((int)(PRINT_INTERVAL_SEC * SAMPLING_RATE)) // 转换为对应的采样点数

// --- 全局数组和变量 ---
float* input_signal_buffer = NULL; // 修改为指针
float ref_cos_lut[LUT_SIZE];            // 参考余弦查找表
float ref_sin_lut[LUT_SIZE];            // 参考正弦查找表

// 环形缓冲区用于存储过去一秒的数据
float* X_buffer = NULL;
float* Y_buffer = NULL;
int buffer_index = 0;
int buffer_filled = 0;

// 参考信号相位累加器
double ref_phase_accumulator = 0.0f;

// 在全局变量区域添加
#define MAX_FREQ_ADJUST 0.1f    // 最大频率调整范围 (Hz)
#define PHASE_DIFF_THRESHOLD 0.002f  // 相位差阈值 (度)
#define FREQ_ADJUST_STEP 0.001f   // 频率调整步长 (Hz)

// 用于频率跟踪的变量
float g_last_phase = 0.0f;      // 上一次的相位值
float g_last_freq_adjust_time = 0.0f;  // 上次频率调整的时间（秒）
int g_sample_count = 0;         // 总采样计数，用于时间计算

#define WINDOW_TIME 0.05f  // 窗口时间长度
#define WINDOW_SIZE ((int)(WINDOW_TIME * SAMPLING_RATE)) // 窗口大小 (采样点数)
float g_window1_sum = 0.0f;     // 第一个窗口的相位和
float g_window2_sum = 0.0f;     // 第二个窗口的相位和
int g_window1_count = 0;        // 第一个窗口的计数
int g_window2_count = 0;        // 第二个窗口的计数

// --- 函数声明 ---
void generate_input_signal_buffer(void);
void generate_reference_lut(void);
void get_reference_samples(float* cos_val, float* sin_val);
void calculate_amplitude_and_phase(float X, float Y, float* amplitude, float* phase_deg);
float get_simulated_adc_sample(int sample_index);
void update_reference_frequency(float new_freq);
int auto_frequency_tracking(float current_phase);


// --- 主函数 ---
int main() {
    clock_t start_time, end_time;
    double cpu_time_used;

    start_time = clock(); // 记录开始时间

        printf("锁相放大器算法模拟 (流式处理)\n");
    printf("---------------------------------\n");
    printf("参数设置:\n");
    printf("  采样率: %.1f Hz\n", SAMPLING_RATE);
    printf("  信号时长 (模拟): %.2f s\n", SIGNAL_DURATION);
    printf("  总采样点数 (模拟): %d\n", NUM_SAMPLES);
    printf("  信号1持续时间: %.1f s, 信号2持续时间: %.1f s\n", SIGNAL1_DURATION, SIGNAL2_DURATION);
    printf("  待测信号1: %.1f Hz, 幅值 %.2f V, 相位 %.1f 度, 直流偏置 %.2f V\n",
           INPUT1_FREQ, INPUT1_AMPLITUDE, INPUT1_PHASE_DEG, INPUT1_DC_OFFSET);
    printf("  待测信号2: %.1f Hz, 幅值 %.2f V, 相位 %.1f 度, 直流偏置 %.2f V\n",
           INPUT2_FREQ, INPUT2_AMPLITUDE, INPUT2_PHASE_DEG, INPUT2_DC_OFFSET);
    printf("  (信号1预期范围: %.2fV to %.2fV)\n", INPUT1_DC_OFFSET - INPUT1_AMPLITUDE, INPUT1_DC_OFFSET + INPUT1_AMPLITUDE);
    printf("  (信号2预期范围: %.2fV to %.2fV)\n", INPUT2_DC_OFFSET - INPUT2_AMPLITUDE, INPUT2_DC_OFFSET + INPUT2_AMPLITUDE);
    printf("  噪声信号1: %.1f Hz, 幅值 %.2f V, 相位 %.1f 度\n",
           NOISE_FREQ_1, NOISE_AMPLITUDE_1, NOISE_PHASE_DEG_1);
    printf("  参考信号频率: %.1f Hz, LUT大小: %d\n", g_ref_freq, LUT_SIZE);
    printf("  平均时间窗口: %.2f s\n", AVERAGE_TIME);
    printf("  每隔 %.2f 秒输出一次结果\n", PRINT_INTERVAL_SEC);
    printf("  当前输出模式: %d (1:重建信号, 2:内部参考, 3:参考余弦+重建)\n", g_output_mode);
    printf("---------------------------------\n\n");
    printf("时间(s)\t ADC值(V)\t X_filt\t Y_filt\t 幅值(V)\t 相位(deg)\n");


    // 1. 分配内存
    input_signal_buffer = (float*)malloc(NUM_SAMPLES * sizeof(float));
    X_buffer = (float*)malloc(BUFFER_SIZE * sizeof(float));
    Y_buffer = (float*)malloc(BUFFER_SIZE * sizeof(float));
    
    if (input_signal_buffer == NULL || X_buffer == NULL || Y_buffer == NULL) {
        printf("内存分配失败!\n");
        // 清理已分配的内存
        if (input_signal_buffer) free(input_signal_buffer);
        if (X_buffer) free(X_buffer);
        if (Y_buffer) free(Y_buffer);
        return -1;
    }

    // 初始化缓冲区
    memset(X_buffer, 0, BUFFER_SIZE * sizeof(float));
    memset(Y_buffer, 0, BUFFER_SIZE * sizeof(float));    // 2. 预生成待测信号序列 (模拟ADC的连续输入源)
    generate_input_signal_buffer();
    printf("待测信号缓存已生成.\n");

    // // 输出待测信号前100个样本 (调试用)
    // printf("前100个待测信号样本:\n");
    // for (int i = 0; i < 100 && i < NUM_SAMPLES; ++i) {
    //     printf("样本 %d: %.4f V\n", i, input_signal_buffer[i]);
    // }
    // 绘制待测信号图像
    // （PC端可选：输出CSV文件，便于用Excel/Matlab/Python绘图）
    FILE* fp_input = fopen("input_signal.csv", "w");
    if (fp_input) {
        fprintf(fp_input, "sample_index,time(s),voltage(V)\n");
        for (int i = 0; i < NUM_SAMPLES; ++i) {
            float t = (float)i / SAMPLING_RATE;
            fprintf(fp_input, "%d,%.6f,%.6f\n", i, t, input_signal_buffer[i]);
        }
        fclose(fp_input);
        printf("已导出 input_signal.csv，可用Excel/Matlab/Python绘图.\n");
    } else {
        printf("无法写入 input_signal.csv，跳过信号导出.\n");
    }


    // 3. 生成参考正余弦查找表
    generate_reference_lut();
    printf("参考信号查找表已生成.\n\n");



    // 在开始循环前清空并创建新文件
    FILE* fp = fopen("lock_in_output.csv", "w");
    if (!fp) {
        printf("无法创建 lock_in_output.csv 文件!\n");
        return -1;
    }
    // 写入CSV标题行
    fprintf(fp, "时间(s),输入值(V),X_filt,Y_filt,幅值(V),相位(deg),还原信号(V),参考余弦,参考正弦\n");

    // 存储两个通道的文件
    FILE* fp_2 = fopen("lock_in_output_2.csv", "w");
    if (!fp_2) {
        printf("无法创建 lock_in_output_2.csv 文件!\n");
        fclose(fp);  // 关闭已打开的文件
        return -1;
    }
    // 写入CSV标题行
    fprintf(fp_2, "channel1,channel2\n");

    printf("Time(s)\t ADC(V)\t X_filt\t Y_filt\t Amp(V)\t Phase(deg)\t Recovered(V)\t RefCos\t RefSin\n");

    // 4. 模拟连续处理
    int i = 0; // 初始化采样索引
    while (1) { 
        
        // 调试，在time为30s时，更新参考频率
        float time_30s = 25.0f;
        int sample_index_30s = (int)(time_30s * SAMPLING_RATE);
        if (i == sample_index_30s) {
            update_reference_frequency(101.0f);
        }


        // 4.1 模拟从ADC获取当前采样值
        float current_input_sample = get_simulated_adc_sample(i);

        // 4.2 从LUT获取当前参考信号样本
        float current_ref_cos, current_ref_sin; // 定义局部变量
        get_reference_samples(&current_ref_cos, &current_ref_sin); // 修正变量名

        // 4.3 混频 (Phase Sensitive Detection - PSD)
        float mixed_X = current_input_sample * current_ref_cos;
        float mixed_Y = current_input_sample * current_ref_sin;

        // 4.4 更新缓冲区并计算平均值
        float current_filtered_X, current_filtered_Y;
        X_buffer[buffer_index] = mixed_X;
        Y_buffer[buffer_index] = mixed_Y;
        
        if (buffer_index >= BUFFER_SIZE - 1) {
            buffer_filled = 1;
        }
        
        // 计算平均值
        float sum_X = 0.0f;
        float sum_Y = 0.0f;
        
        // 始终使用完整缓冲区大小进行平均，未填充的部分自动为0
        for (int j = 0; j < BUFFER_SIZE; j++) {
            if (j <= buffer_index || buffer_filled) {
                sum_X += X_buffer[j];
                sum_Y += Y_buffer[j];
            }
        }
        
        // 始终除以完整的缓冲区大小
        current_filtered_X = sum_X / BUFFER_SIZE;
        current_filtered_Y = sum_Y / BUFFER_SIZE;
        
        // 更新缓冲区索引
        buffer_index = (buffer_index + 1) % BUFFER_SIZE;

        // 4.5 计算幅值和相位
        float recovered_amplitude, recovered_phase_deg;
        calculate_amplitude_and_phase(current_filtered_X, current_filtered_Y, &recovered_amplitude, &recovered_phase_deg);

        // 4.5.1 执行自动频率跟踪
        int freq_adjusted = auto_frequency_tracking(recovered_phase_deg);
        if (freq_adjusted) {
            printf("时间: %.2f s, 参考频率已调整至: %.3f Hz\n", 
                   (float)i / SAMPLING_RATE, g_ref_freq);
        }

        // 4.6 还原信号
        float current_time = (float)i / SAMPLING_RATE;
        float recovered_phase_rad = recovered_phase_deg * M_PI / 180.0f;
        float recovered_signal = recovered_amplitude * cosf(2.0f * M_PI * g_ref_freq * current_time - recovered_phase_rad);

        // 4.7 定期输出结果
        // 调试内容
        if ((i + 1) % PRINT_INTERVAL_SAMPLES == 0) { 
            printf("time:%.4f\t input:%.4f\t X:%.4f\t Y:%.4f\t amp:%.4f\t phase:%.2f\t rec:%.4f\n",
                   current_time,
                   current_input_sample,
                   current_filtered_X,
                   current_filtered_Y,
                   recovered_amplitude,
                   recovered_phase_deg,
                   recovered_signal);
            fflush(stdout);
        }

        // // 第一部分输出：重建信号的频率、幅值和相位
        // printf("\n--- 当前重建信号参数 ---\n");
        // printf("频率: %.2f Hz\n", REF_FREQ);
        // printf("幅值: %.4f V\n", recovered_amplitude);
        // printf("相位: %.2f 度\n", recovered_phase_deg);
        // printf("---------------------------------\n");

        // // 根据输出模式添加新的输出
        // printf("--- 当前输出模式 %d 的额外信息 ---\n", g_output_mode);
        // switch (g_output_mode) {
        //     case OUTPUT_MODE_RECOVERED_SIGNAL:
        //         printf("待测信号: %.4f, 重建信号: %.4f V\n", current_input_sample, recovered_signal);
        //         fprintf(fp_2, "%.4f,%.4f\n", current_input_sample, recovered_signal);
        //         break;
        //     case OUTPUT_MODE_INTERNAL_REF_SIGNALS:
        //         printf("内部参考信号 - 余弦: %.4f, 正弦: %.4f\n", current_ref_cos, current_ref_sin);
        //         fprintf(fp_2, "%.4f,%.4f\n", current_ref_cos, current_ref_sin);
        //         break;
        //     case OUTPUT_MODE_REF_COS_AND_RECOVERED:
        //         printf("参考余弦: %.4f, 重建信号: %.4f V\n", current_ref_cos, recovered_signal);
        //         fprintf(fp_2, "%.4f,%.4f\n", current_ref_cos, recovered_signal);
        //         break;
        // }


        // // 输出最后100个样本 (调试用)
        // if (i >= NUM_SAMPLES - 100) {
        //     printf("样本 %d: 输入=%.4f V, X_filt=%.4f, Y_filt=%.4f, 幅值=%.4f V, 相位=%.2f 度\n",
        //            i, current_input_sample, current_filtered_X, current_filtered_Y,
        //            recovered_amplitude, recovered_phase_deg);
        // }

        // 将结果保存到文件
        if (fp) {
            fprintf(fp, "%.4f,%.4f,%.4f,%.4f,%.4f,%.2f,%.4f,%.4f,%.4f\n",
                    current_time,
                    current_input_sample,
                    current_filtered_X,
                    current_filtered_Y,
                    recovered_amplitude,
                    recovered_phase_deg,
                    recovered_signal,
                    current_ref_cos,
                    current_ref_sin);
        }

        i++; // 在循环末尾递增采样索引

        // 调试用的退出条件
        if (i >= NUM_SAMPLES) {
            break;
        }
    }
    
    // 关闭文件
    fclose(fp);
    fclose(fp_2);
    
    // 释放分配的内存
    free(input_signal_buffer);
    free(X_buffer);
    free(Y_buffer);
    
    printf("---------------------------------\n");
    printf("模拟处理完成.\n");

    end_time = clock(); // 记录结束时间
    cpu_time_used = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
    printf("程序运行时间: %.4f 秒\n", cpu_time_used);

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
 * 生成两段不同的信号：
 * - 前SIGNAL1_DURATION秒为信号1参数
 * - 后SIGNAL2_DURATION秒为信号2参数
 *
 * 信号模型: s(t) = dc_offset + A * cos(2*pi*f*t - phi) + Noise(t)
 */
void generate_input_signal_buffer(void) {
    float t;
    
    // 计算信号1和信号2的采样点边界
    int signal1_samples = (int)(SIGNAL1_DURATION * SAMPLING_RATE);
    int signal2_samples = (int)(SIGNAL2_DURATION * SAMPLING_RATE);
    
    // 确保不超过总采样点数
    if (signal1_samples + signal2_samples > NUM_SAMPLES) {
        signal1_samples = NUM_SAMPLES / 2;
        signal2_samples = NUM_SAMPLES - signal1_samples;
    }

    for (int i = 0; i < NUM_SAMPLES; ++i) {
        t = (float)i / SAMPLING_RATE;
        
        if (i < signal1_samples) {
            // 生成信号1
            float phase_rad1 = INPUT1_PHASE_DEG * M_PI / 180.0f;
            float noise_phase_rad1 = NOISE_PHASE_DEG_1 * M_PI / 180.0f;
            
            float target_component = INPUT1_AMPLITUDE * cosf(2.0f * M_PI * INPUT1_FREQ * t - phase_rad1);
            float noise_component = NOISE_AMPLITUDE_1 * cosf(2.0f * M_PI * NOISE_FREQ_1 * t - noise_phase_rad1);
            
            input_signal_buffer[i] = INPUT1_DC_OFFSET + target_component + noise_component;
        } else {
            // 生成信号2
            float phase_rad2 = INPUT2_PHASE_DEG * M_PI / 180.0f;
            float noise_phase_rad1 = NOISE_PHASE_DEG_1 * M_PI / 180.0f;
            
            float target_component = INPUT2_AMPLITUDE * cosf(2.0f * M_PI * INPUT2_FREQ * t - phase_rad2);
            float noise_component = NOISE_AMPLITUDE_1 * cosf(2.0f * M_PI * NOISE_FREQ_1 * t - noise_phase_rad1);
            
            input_signal_buffer[i] = INPUT2_DC_OFFSET + target_component + noise_component;
        }
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
 * @brief 更新参考信号频率
 * @param new_freq 新的参考信号频率 (Hz)
 */
void update_reference_frequency(float new_freq) {
    g_ref_freq = new_freq;
    // 重新生成查找表
    generate_reference_lut();
    // 重置相位累加器
    ref_phase_accumulator = 0.0f;
    // 记录本次频率调整的时间
    float current_time = (float)g_sample_count / SAMPLING_RATE;
    g_last_freq_adjust_time = current_time;
}

/**
 * @brief 从查找表获取当前周期的参考正余弦值，并更新相位累加器
 *
 * @param cos_val 指针，用于返回当前的余弦参考值
 * @param sin_val 指针，用于返回当前的正弦参考值
 */
void get_reference_samples(float* cos_val, float* sin_val) {
    
    float phase_increment = (g_ref_freq / SAMPLING_RATE) * (float)LUT_SIZE;

    // // 查表方法 (四舍五入)
    // unsigned int lut_index = (unsigned int)(ref_phase_accumulator + 0.5f) % LUT_SIZE;
    // *cos_val = ref_cos_lut[lut_index];
    // *sin_val = ref_sin_lut[lut_index];

    // 使用线性插值
    float current_exact_phase = ref_phase_accumulator; // 当前的精确相位
    // 计算插值所需的整数索引和小数部分
    // floorf确保向下取整, 例如 current_exact_phase = 3.7, index_floor = 3
    // current_exact_phase = 3.0, index_floor = 3
    unsigned int index_floor = (unsigned int)floorf(current_exact_phase);
    float fraction = current_exact_phase - (float)index_floor;
    unsigned int lut_idx0 = index_floor % LUT_SIZE;
    unsigned int lut_idx1 = (index_floor + 1) % LUT_SIZE;
    // 对余弦分量进行线性插值
    float cos_val0 = ref_cos_lut[lut_idx0];
    float cos_val1 = ref_cos_lut[lut_idx1];
    *cos_val = cos_val0 + fraction * (cos_val1 - cos_val0);
    // 对正弦分量进行线性插值
    float sin_val0 = ref_sin_lut[lut_idx0];
    float sin_val1 = ref_sin_lut[lut_idx1];
    *sin_val = sin_val0 + fraction * (sin_val1 - sin_val0);

    // 更新相位累加器
    ref_phase_accumulator += phase_increment;
    if (ref_phase_accumulator >= (float)LUT_SIZE) {
         ref_phase_accumulator = fmodf(ref_phase_accumulator, (float)LUT_SIZE);
    }
}
// void get_reference_samples(float* cos_val, float* sin_val) {
//     // 静态整数计数器，跟踪总的采样点数
//     static unsigned int n_sample_count = 0;
//     // 1. 计算从开始到现在的总相位（未进行周期缠绕）
//     float total_unwrapped_phase = (float)n_sample_count * (REF_FREQ / SAMPLING_RATE) * (float)LUT_SIZE;
//     // 2. 将总相位映射到当前 LUT 的一个周期内 [0, LUT_SIZE)
//     float current_phase_in_lut = fmodf(total_unwrapped_phase, (float)LUT_SIZE);
//     // 3. 计算 LUT 索引，使用 +0.5f 实现四舍五入到最近的整数索引
//     unsigned int lut_index = (unsigned int)(current_phase_in_lut + 0.5f);

//     // 4. 取模确保索引在 [0, LUT_SIZE-1] 范围内
//     lut_index %= LUT_SIZE;
//     // 5. 从查找表获取参考值
//     *cos_val = ref_cos_lut[lut_index];
//     *sin_val = ref_sin_lut[lut_index];

//     // 6. 增加采样计数
//     n_sample_count++;
// }

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

/**
 * @brief 自动频率跟踪算法
 * @param current_phase 当前测量的相位值 (度)
 * @return 是否需要调整频率
 */
int auto_frequency_tracking(float current_phase) {
    // 获取当前时间
    float current_time = (float)g_sample_count / SAMPLING_RATE;
    g_sample_count++;
    
    // 如果距离上次频率调整的时间小于AVERAGE_TIME，直接返回
    if (current_time - g_last_freq_adjust_time < AVERAGE_TIME*2) {
        // 重置所有窗口数据
        g_window1_sum = 0.0f;
        g_window2_sum = 0.0f;
        g_window1_count = 0;
        g_window2_count = 0;
        return 0;
    }
    
    // 根据当前窗口状态更新相应的累加器
    if (g_window1_count < WINDOW_SIZE) {
        g_window1_sum += current_phase;
        g_window1_count++;
    } else if (g_window2_count < WINDOW_SIZE) {
        g_window2_sum += current_phase;
        g_window2_count++;
    }
    
    // 当两个窗口都填满时，进行比较和判断
    if (g_window1_count == WINDOW_SIZE && g_window2_count == WINDOW_SIZE) {
        // 计算两个窗口的平均相位
        float avg_phase1 = g_window1_sum / WINDOW_SIZE;
        float avg_phase2 = g_window2_sum / WINDOW_SIZE;
        
        // 计算两个窗口的平均相位差
        float phase_diff = avg_phase2 - avg_phase1;
        // 处理相位跳变
        if (phase_diff > 180.0f) phase_diff -= 360.0f;
        if (phase_diff < -180.0f) phase_diff += 360.0f;
        
        // printf("时间: %.2f s, 窗口1平均相位: %.3f, 窗口2平均相位: %.3f, 相位差: %.3f\n",current_time, avg_phase1, avg_phase2, phase_diff);
        
        // 如果相位差超过阈值，调整频率
        if (fabsf(phase_diff) > PHASE_DIFF_THRESHOLD) {
            // 根据相位差方向调整频率
            // float freq_adjust = -90 * phase_diff * FREQ_ADJUST_STEP;
            // 理论精确补偿 (Δf = Δφ / (360 × T_window))
            float freq_adjust = - phase_diff / (360.0f * WINDOW_TIME);
            printf("时间: %.2f s, 窗口1平均相位: %.6f, 窗口2平均相位: %.6f, 相位差: %.6f\n",current_time, avg_phase1, avg_phase2, phase_diff);
            printf("freq_adjust: %.6f\n", freq_adjust);
            
            // 限制调整范围
            if (freq_adjust > MAX_FREQ_ADJUST) freq_adjust = MAX_FREQ_ADJUST;
            if (freq_adjust < -MAX_FREQ_ADJUST) freq_adjust = -MAX_FREQ_ADJUST;
            
            // 更新参考频率
            float new_freq = g_ref_freq + freq_adjust;
            printf("new_freq: %.6f\n", new_freq);
            update_reference_frequency(new_freq);
            
            
            // 重置所有窗口数据
            g_window1_sum = 0.0f;
            g_window2_sum = 0.0f;
            g_window1_count = 0;
            g_window2_count = 0;
            
            return 1;  // 表示频率已调整
        }
        
        // 准备下一轮比较：将窗口2的数据移到窗口1
        g_window1_sum = 0.0f;
        g_window2_sum = 0.0f;
        g_window1_count = 0;
        g_window2_count = 0;
    }
    
    g_last_phase = current_phase;
    return 0;  // 表示频率未调整
}
