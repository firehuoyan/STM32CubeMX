#include <stdio.h>
#include <stdlib.h>
#include <math.h>    // 需要链接 -lm
#include <string.h>  // 添加string.h用于memset函数
#include <time.h>    // 添加time.h用于计时


#define ATAN2_LUT_SIZE 1024         // 查找表大小
float atan2_lut[ATAN2_LUT_SIZE];    // atan2查找表，存储预计算的反正切值

#ifndef M_PI
#define M_PI 3.14159265358979323846f // 定义圆周率常量
#endif


// void init_atan2_lut(void);
float fast_atan2(float Y, float X);





// 初始化atan2快速查找表
void init_atan2_lut(void) {
    for (int j = 0; j < ATAN2_LUT_SIZE; j++) {
        float ratio = (float)j / (ATAN2_LUT_SIZE - 1) * 2.0f - 1.0f; // -1 到 1
        atan2_lut[j] = atan2f(ratio, 1.0f);
    }
}

/**
 * @brief 快速计算 atan2 的函数（带线性插值）
 * @param Y Y 分量
 * @param X X 分量
 * @return 计算得到的角度（弧度）
 */
float fast_atan2(float Y, float X) {
    // 特殊情况：X接近0
    if (fabsf(X) < 1e-10f) {
        return (Y > 0) ? M_PI/2 : -M_PI/2;
    }
    
    float abs_Y = fabsf(Y);
    float abs_X = fabsf(X);
    
    // 当Y的绝对值大于X时，切换坐标系计算
    if (abs_Y > abs_X) {
        float ratio_swapped = X / Y;  // 保证 |ratio_swapped| <= 1
        
        // // 线性插值计算
        // float float_index = (ratio_swapped + 1.0f) * (ATAN2_LUT_SIZE - 1) / 2.0f;
        // int index1 = (int)float_index;
        // int index2 = index1 + 1;
        // // 边界检查
        // if (index1 < 0) index1 = 0;
        // if (index2 >= ATAN2_LUT_SIZE) {
        //     index1 = ATAN2_LUT_SIZE - 1;
        //     index2 = ATAN2_LUT_SIZE - 1;
        // }
        // // 线性插值
        // float fraction = float_index - index1;
        // float atan_val = atan2_lut[index1] + fraction * (atan2_lut[index2] - atan2_lut[index1]);

        // 不线性插值，直接四舍五入
        float float_index = (ratio_swapped + 1.0f) * (ATAN2_LUT_SIZE - 1) / 2.0f;
        int index1 = (int)(float_index + 0.5f); // 四舍五入到最近的整数索引
        // 边界检查
        if (index1 < 0) index1 = 0;
        if (index1 >= ATAN2_LUT_SIZE) {
            index1 = ATAN2_LUT_SIZE - 1;
        }
        float atan_val = atan2_lut[index1];
        
        return (Y > 0) ? M_PI/2 - atan_val : -M_PI/2 - atan_val;
    }
    
    // 常规情况：abs_Y <= abs_X，保证 |Y/X| <= 1
    float ratio = Y / X;
    
    // // 线性插值计算
    // float float_index = (ratio + 1.0f) * (ATAN2_LUT_SIZE - 1) / 2.0f;
    // int index1 = (int)float_index;
    // int index2 = index1 + 1;
    // // 边界检查
    // if (index1 < 0) index1 = 0;
    // if (index2 >= ATAN2_LUT_SIZE) {
    //     index1 = ATAN2_LUT_SIZE - 1;
    //     index2 = ATAN2_LUT_SIZE - 1;
    // }
    // // 线性插值
    // float fraction = float_index - index1;
    // float base_angle = atan2_lut[index1] + fraction * (atan2_lut[index2] - atan2_lut[index1]);

    // 不线性插值，直接四舍五入
    float float_index = (ratio + 1.0f) * (ATAN2_LUT_SIZE - 1) / 2.0f;
    int index1 = (int)(float_index + 0.5f); // 四舍五入到最近的整数索引
    // 边界检查
    if (index1 < 0) index1 = 0;
    if (index1 >= ATAN2_LUT_SIZE) {
        index1 = ATAN2_LUT_SIZE - 1;
    }
    float base_angle = atan2_lut[index1];
    
    // 象限修正
    if (X < 0) {
        base_angle += (Y >= 0) ? M_PI : -M_PI;
    }
    
    return base_angle;
}



void calculate_amplitude_and_phase(float X, float Y, float* amplitude, float* phase_deg) {
    *amplitude = 2.0f * sqrtf(X * X + Y * Y);
    float phase_rad = fast_atan2(Y, X);  // 使用快速atan2
    *phase_deg = phase_rad * 180.0f / M_PI;
}


// generate_reference_lut(); // 生成参考信号查找表

// init_atan2_lut();       // 初始化 atan2 查找表





void calculate_amplitude_and_phase_std(float X, float Y, float* amplitude, float* phase_deg) {
    *amplitude = 2.0f * sqrtf(X * X + Y * Y);
    float phase_rad = atan2f(Y, X);
    *phase_deg = phase_rad * 180.0f / M_PI;
}

// 提前生成角度、X、Y值
#define num_samples 3600  // 360000个角度，每个角度0.1度

// 定义三个数组
float angles[num_samples];  // 存储角度
float X[num_samples];       // 存储X值
float Y[num_samples];       // 存储Y值

// 测试函数
int main() {
    init_atan2_lut();  // 初始化查找表


    // float X = 0.3434;  // 示例X值
    // float Y = -0.5512;  // 示例Y值

    // float amplitude, phase_deg;
    // calculate_amplitude_and_phase(X, Y, &amplitude, &phase_deg);
    // printf("Amplitude: %.2f, Phase: %.2f degrees\n", amplitude, phase_deg);

    for (int i = 0; i < num_samples; i += 1) {
        angles[i] = i * 0.1f;  // 每个角度0.1度
        float angle_rad = angles[i] * M_PI / 180.0f;  // 转换为弧度
        X[i] = cosf(angle_rad);  // 计算X值
        Y[i] = sinf(angle_rad);  // 计算Y值
    }


    // 记录开始时间
    clock_t start_time = clock();
    //计算0-360角度对应的X和Y值，然后反解，判断效果
    for (int i = 0; i < num_samples; i += 1) {
        float amplitude, phase_deg;
        calculate_amplitude_and_phase(X[i], Y[i], &amplitude, &phase_deg);
        printf("Angle: %.1f degrees -> Amplitude: %.2f, Phase: %.2f degrees\n", angles[i], amplitude, phase_deg);
    }

    // 记录结束时间
    clock_t end_time = clock();
    double cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Total time taken: %.16f seconds\n", cpu_time_used);


    // 对比使用标准库的 atan2 函数
    clock_t start_time_std = clock();
    for (int i = 0; i < num_samples; i += 1) {
        float amplitude, phase_deg;
        calculate_amplitude_and_phase_std(X[i], Y[i], &amplitude, &phase_deg);
        printf("Standard Angle: %.1f degrees -> Amplitude: %.2f, Phase: %.2f degrees\n", angles[i], amplitude, phase_deg);
    }
    clock_t end_time_std = clock();
    double cpu_time_used_std = ((double)(end_time_std - start_time_std)) / CLOCKS_PER_SEC;
    printf("Total time taken (standard): %.16f seconds\n", cpu_time_used_std);
    printf("Total time taken: %.16f seconds\n", cpu_time_used);
    // 输出性能对比结果
    printf("Speedup: %.2f\n", cpu_time_used_std / cpu_time_used);
    // printf("测试完成.\n");
    // printf("");
    // printf("");
    // printf("");
    // printf("");
    // printf("测试完成.\n");

    return 0;
}